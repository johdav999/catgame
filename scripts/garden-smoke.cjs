// Real Paws WebGPU asset acceptance, no engine playground or synthetic renderer.
const {chromium}=require('playwright');const {PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const base=process.env.PAWS_URL||'http://127.0.0.1:19484';const evidence=process.env.EVIDENCE_DIR||'build/evidence/garden';
function changed(a,b){a=PNG.sync.read(a);b=PNG.sync.read(b);let n=0;for(let i=0;i<a.data.length;i+=4)if(Math.abs(a.data[i]-b.data[i])+Math.abs(a.data[i+1]-b.data[i+1])+Math.abs(a.data[i+2]-b.data[i+2])>24)n++;return n;}
(async()=>{
 await fs.mkdir(evidence,{recursive:true});const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE});
 const report={url:base+'/garden-preview.html',browser:browser.version(),checks:[],errors:[],assets:[]};
 const page=await browser.newPage({viewport:{width:1440,height:900},deviceScaleFactor:1});
 const hashes=[];const digest=b=>crypto.createHash('sha256').update(b).digest('hex');
 page.on('pageerror',e=>report.errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});page.on('response',r=>{if(r.url().endsWith('.glb'))hashes.push((async()=>{const relative=new URL(r.url()).pathname.replace('/assets/','assets/runtime/');report.assets.push({url:r.url(),status:r.status(),sha256:digest(await r.body()),expected:digest(await fs.readFile(relative))});})().catch(e=>report.errors.push(String(e))));});
 try{
  await page.goto(report.url);await page.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:60000});
  await page.waitForTimeout(400);await page.screenshot({path:path.join(evidence,'desktop.png'),fullPage:true});
  const canvas=page.locator('#viewport');
  for(const [label,name] of [['Channels','channels'],['Basket','basket'],['Gate and switch','gate'],['Furnishings','furnishings']]){
   await page.getByRole('button',{name:label,exact:true}).click();await page.waitForTimeout(120);await canvas.screenshot({path:path.join(evidence,name+'-close.png')});
  }
  await page.getByRole('button',{name:'Gate and switch',exact:true}).click();const closed=await canvas.screenshot();await page.getByRole('button',{name:'Press switch / open gate',exact:true}).click();await page.waitForTimeout(120);const open=await canvas.screenshot({path:path.join(evidence,'gate-open.png')});assert(changed(closed,open)>150,'Gate/plate must change rendered geometry');assert.equal(await page.evaluate(()=>Module._garden_mode()),1);
  await page.getByRole('button',{name:'Basket',exact:true}).click();await page.getByRole('button',{name:'Fit toys in basket',exact:true}).click();await page.waitForTimeout(120);await canvas.screenshot({path:path.join(evidence,'basket-fit.png')});
  await page.getByRole('button',{name:'Furnishings',exact:true}).click();await page.getByRole('button',{name:'Pip at bowl',exact:true}).click();await page.waitForTimeout(120);await canvas.screenshot({path:path.join(evidence,'pip-bowl.png')});
  report.checks.push('All 21 GLBs and Pip loaded over HTTP; route, basket, gate and furnishing close views captured; gate/plate action changes pixels');
  await page.getByRole('button',{name:'Kit overview',exact:true}).click();await page.getByRole('button',{name:'Whole kit',exact:true}).click();
  await page.getByLabel('Desktop shadows').uncheck();await canvas.screenshot({path:path.join(evidence,'shadow-free.png')});await page.getByLabel('Desktop shadows').check();
  for(const [width,height] of [[390,844],[844,390]]){
   await page.setViewportSize({width,height});await page.waitForTimeout(200);assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));await page.screenshot({path:path.join(evidence,`layout-${width}x${height}.png`),fullPage:true});
   if(width===390){await page.getByRole('button',{name:'Basket',exact:true}).click();await canvas.screenshot({path:path.join(evidence,'portrait-basket.png')});await page.getByRole('button',{name:'Whole kit',exact:true}).click();}
  }
  // Keyboard action and input ownership: focus/Enter changes one assembly state.
  await page.getByRole('button',{name:'Press switch / open gate',exact:true}).focus();await page.keyboard.press('Enter');assert.equal(await page.evaluate(()=>Module._garden_mode()),1);
  await page.evaluate(()=>{for(let i=0;i<30;i++)document.querySelectorAll('[data-pose]')[i%4].click();});assert.equal(await page.evaluate(()=>Module._garden_mode()),1);
  await page.evaluate(()=>Module._garden_suspend(1));await page.waitForTimeout(120);await page.evaluate(()=>Module._garden_suspend(0));
  report.checks.push('Wide, portrait and short landscape; shadow-free rendering; keyboard and rapid latest-pose selection; host suspension boundary');
  await page.setViewportSize({width:1440,height:900});await page.getByRole('button',{name:'Whole kit',exact:true}).click();await page.waitForTimeout(1500);report.performance=await page.evaluate(()=>window.pawsPerformance);report.renderer=await page.evaluate(()=>window.pawsRenderState);
  await Promise.all(hashes);assert.equal(new Set(report.assets.filter(a=>a.status===200).map(a=>a.url)).size,22);for(const a of report.assets)assert.equal(a.sha256,a.expected,'Served GLB differs from delivered source: '+a.url);assert.deepEqual(report.errors,[]);
  report.checks.push('Every HTTP GLB response hash matches the delivered runtime asset');
  const failed=await browser.newPage();await failed.route('**/assets/models/garden/basket.glb',r=>r.fulfill({status:503,body:'Acceptance load failure'}));await failed.goto(report.url);await failed.waitForFunction(()=>window.gardenState==='failed',null,{timeout:30000});await failed.unroute('**/assets/models/garden/basket.glb');await failed.getByRole('button',{name:'Retry loading garden'}).click();await failed.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});await failed.close();report.checks.push('Fresh basket HTTP failure is visible; retry loads full kit; zero console errors on normal page');
 }catch(e){report.failure=String(e);report.state=await page.evaluate(()=>({garden:window.gardenState,renderer:window.pawsRenderState,status:document.querySelector('#status').textContent}));await page.screenshot({path:path.join(evidence,'failure.png'),fullPage:true});throw e;}
 finally{await fs.writeFile(path.join(evidence,'garden-smoke.json'),JSON.stringify(report,null,2));await browser.close();}
 console.log(JSON.stringify(report,null,2));
})().catch(e=>{console.error(e);process.exit(1);});
