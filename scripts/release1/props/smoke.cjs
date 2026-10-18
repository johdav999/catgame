// Isolated pinned Paws browser inspection; owns and closes its local server/profile.
const {chromium}=require('playwright'),{PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p03-pinned-release/site');
const evidence=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-03');
const port=Number(process.env.PAWS_TEST_PORT||19613),url=`http://127.0.0.1:${port}/`;
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
function changed(a,b){a=PNG.sync.read(a);b=PNG.sync.read(b);let n=0;for(let i=0;i<a.data.length;i+=4)if(Math.abs(a.data[i]-b.data[i])+Math.abs(a.data[i+1]-b.data[i+1])+Math.abs(a.data[i+2]-b.data[i+2])>30)n++;return n;}
(async()=>{
 await fs.mkdir(evidence,{recursive:true});
 const server=http.createServer(async(req,res)=>{try{const u=new URL(req.url,url),p=path.resolve(site,'.'+decodeURIComponent(u.pathname==='/'?'/index.html':u.pathname));if(!p.startsWith(site+path.sep))throw Error('outside site');const b=await fs.readFile(p);res.setHeader('Content-Type',({'.js':'text/javascript','.wasm':'application/wasm','.html':'text/html','.css':'text/css','.json':'application/json','.svg':'image/svg+xml','.png':'image/png'})[path.extname(p)]||'application/octet-stream');res.setHeader('Cache-Control','no-store');res.end(b);}catch{res.statusCode=404;res.end('Missing');}});
 await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
 let browser,page;const report={url,site,checks:[],errors:[],assets:[],views:[]};const pending=[];
 try{
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
  page=await browser.newPage({viewport:{width:1440,height:1000},deviceScaleFactor:1});
  page.on('pageerror',e=>report.errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  page.on('response',r=>{if(r.url().endsWith('.glb'))pending.push((async()=>{const relative=new URL(r.url()).pathname.slice(1),data=await r.body();report.assets.push({url:relative,status:r.status(),sha256:hash(data),expected:hash(await fs.readFile(path.join(site,relative)))});})());});
  await page.goto(url);await page.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:60000});
  await page.waitForTimeout(700);const canvas=page.locator('#viewport');
  const names=JSON.parse(await fs.readFile(path.join(site,'views.json'),'utf8'));
  for(let i=0;i<names.length;i++){
   await page.locator('#view').selectOption(String(i));await page.waitForTimeout(100);
   const file=names[i].replaceAll(' ','-')+'.png';await canvas.screenshot({path:path.join(evidence,file)});report.views.push({name:names[i],screenshot:file});
  }
  for(const name of ['bells','playhouse','cushion_basket_fitted']){
   await page.locator('#view').selectOption(String(names.indexOf(name)));await page.getByRole('button',{name:'Rest / fitted',exact:true}).click();await page.waitForTimeout(100);const before=await canvas.screenshot();
   await page.getByRole('button',{name:'Open / pressed / lifted',exact:true}).click();await page.waitForTimeout(100);const after=await canvas.screenshot({path:path.join(evidence,name+'-active.png')});assert(changed(before,after)>100,name+' pose does not change geometry');
  }
  report.checks.push('All individual exports and explicit assemblies rendered; door, plates, bells and cushion lifts change pixels');
  await page.getByRole('button',{name:'Rest / fitted',exact:true}).click();await page.getByLabel('Desktop shadows').uncheck();
  for(const name of ['Whole new kit','bells','cushion_basket_fitted','water_route','playhouse','picnic']){await page.locator('#view').selectOption(String(names.indexOf(name)));await page.waitForTimeout(100);await canvas.screenshot({path:path.join(evidence,name.replaceAll(' ','-')+'-shadow-free.png')});}
  for(const [w,h] of [[390,844],[844,390]]){await page.setViewportSize({width:w,height:h});await page.locator('#view').selectOption(String(names.indexOf('bells')));await page.waitForTimeout(150);assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));await page.screenshot({path:path.join(evidence,`layout-${w}x${h}.png`),fullPage:true});}
  await page.getByRole('button',{name:'Open / pressed / lifted',exact:true}).focus();await page.keyboard.press('Enter');assert.equal(await page.evaluate(()=>Module._garden_mode()),1);
  await page.evaluate(()=>{for(let i=0;i<20;i++)Module._garden_pose(i%2);Module._garden_suspend(1);});await page.waitForTimeout(100);await page.evaluate(()=>Module._garden_suspend(0));assert.equal(await page.evaluate(()=>Module._garden_mode()),1);
  report.checks.push('Shadow-free, portrait/short landscape, keyboard activation, rapid latest-pose reset and host suspension');
  await page.setViewportSize({width:1440,height:1000});await page.locator('#view').selectOption('0');await page.waitForTimeout(1200);report.performance=await page.evaluate(()=>window.pawsPerformance);report.renderer=await page.evaluate(()=>window.pawsRenderState);
  await Promise.all(pending);assert.equal(new Set(report.assets.map(a=>a.url)).size,51);for(const a of report.assets){assert.equal(a.status,200);assert.equal(a.sha256,a.expected);}assert.deepEqual(report.errors,[]);
  const fail=await browser.newPage();await fail.route('**/props/crate.glb',r=>r.fulfill({status:503,body:'Test failure'}));await fail.goto(url);await fail.waitForFunction(()=>window.gardenState==='failed',null,{timeout:30000});await fail.unroute('**/props/crate.glb');await fail.getByRole('button',{name:'Retry loading garden'}).click();await fail.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});await fail.close();
  report.checks.push('51 HTTP GLB identities match staged bytes; explicit load failure and retry; zero errors in normal run');
 }catch(e){report.failure=String(e);if(page){report.state=await page.evaluate(()=>({garden:window.gardenState,renderer:window.pawsRenderState,status:document.querySelector('#status')?.textContent}));await page.screenshot({path:path.join(evidence,'failure.png'),fullPage:true});}throw e;}
 finally{await fs.writeFile(path.join(evidence,'report.json'),JSON.stringify(report,null,2));if(browser)await browser.close();await new Promise(resolve=>server.close(resolve));}
 console.log(JSON.stringify({checks:report.checks,assets:report.assets.length,views:report.views.length}));
})().catch(e=>{console.error(e);process.exitCode=1;});
