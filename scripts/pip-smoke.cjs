// Run against the generated pinned Paws site. Never an engine demo fixture.
const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises');const path=require('node:path');const assert=require('node:assert/strict');
const base=process.env.PAWS_URL||'http://127.0.0.1:8093';
const evidence=process.env.EVIDENCE_DIR||'build/evidence/pip';
const labels=['Relaxed idle / blink','Attentive request','Happy pounce','Garden walk','Settle on cushion','Curled rest','Approach bowl','Drink','Return to idle'];
function changed(a,b){a=PNG.sync.read(a);b=PNG.sync.read(b);let n=0;for(let i=0;i<a.data.length;i+=4)if(Math.abs(a.data[i]-b.data[i])+Math.abs(a.data[i+1]-b.data[i+1])+Math.abs(a.data[i+2]-b.data[i+2])>18)n++;return n;}
(async()=>{
 await fs.mkdir(evidence,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE});
 const result={url:base+'/pip-preview.html',checks:[],errors:[],requests:[],browser:browser.version()};
 const page=await browser.newPage({viewport:{width:1440,height:1000},deviceScaleFactor:1});
 page.on('pageerror',e=>result.errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')result.errors.push(m.text());});
 page.on('response',r=>{if(r.url().includes('/assets/models/pip/'))result.requests.push({url:r.url(),status:r.status()});});
 try{
  await page.goto(result.url);await page.waitForFunction(()=>window.pipState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});
  const canvas=page.locator('#viewport');
  await page.screenshot({path:path.join(evidence,'desktop.png'),fullPage:true});
  await page.getByLabel('Reduced motion').check();
  const still=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(180);assert.equal(await page.evaluate(()=>Module._pip_time()),still);
  const idle=await canvas.screenshot();
  for(let i=0;i<labels.length;i++){
   await page.getByRole('button',{name:labels[i],exact:true}).click();await page.waitForTimeout(80);
   assert.equal(await page.evaluate(()=>window.pipAction),i);
   await canvas.screenshot({path:path.join(evidence,`static-${i}.png`)});
  }
  result.checks.push('All nine actions select through visible controls; reduced-motion time remains fixed');
  await page.getByRole('button',{name:labels[0],exact:true}).click();await page.getByLabel('Reduced motion').uncheck();
  await page.getByRole('button',{name:labels[3],exact:true}).click();await page.waitForTimeout(200);const walkA=await canvas.screenshot();await page.waitForTimeout(350);const walkB=await canvas.screenshot();
  result.walkChangedPixels=changed(walkA,walkB);assert(result.walkChangedPixels>100,'Actual skinned playback must change canvas pixels');
  await page.waitForTimeout(2500);result.skinnedPerformance=await page.evaluate(()=>window.pawsPerformance);
  await page.evaluate(()=>Module._pip_suspend(1));const t=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>Module._pip_time()),t);await page.evaluate(()=>Module._pip_suspend(0));
  result.checks.push('Walk changes rendered pixels; host suspension boundary freezes animation');
  for(let i=0;i<labels.length;i++){await page.getByRole('button',{name:labels[i],exact:true}).click();await page.waitForTimeout(330);await canvas.screenshot({path:path.join(evidence,`playing-${i}.png`)});}
  await page.getByLabel('Reduced motion').check();await page.getByRole('button',{name:labels[0],exact:true}).click();
  await page.getByLabel('Desktop shadows').uncheck();await canvas.screenshot({path:path.join(evidence,'shadow-free-close.png')});
  await page.getByRole('button',{name:'Puzzle distance',exact:true}).click();await canvas.screenshot({path:path.join(evidence,'shadow-free-puzzle.png')});
  await page.getByLabel('Desktop shadows').check();await canvas.screenshot({path:path.join(evidence,'desktop-puzzle.png')});
  await page.getByRole('button',{name:'Close view',exact:true}).click();
  for(const [w,h] of [[390,844],[844,390]]){await page.setViewportSize({width:w,height:h});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));await page.screenshot({path:path.join(evidence,`layout-${w}x${h}.png`),fullPage:true});}
  result.checks.push('Close and ordinary puzzle distance under desktop and shadow-free lighting; portrait and short landscape layout');
  // Rapid control changes must leave the latest selected pose in charge.
  await page.evaluate(()=>{for(let i=0;i<24;i++)document.querySelectorAll('#clips button')[i%9].click();});assert.equal(await page.evaluate(()=>window.pipAction),5);
  result.performance=await page.evaluate(()=>window.pawsPerformance);result.renderer=await page.evaluate(()=>window.pawsRenderState);
  assert.deepEqual(result.errors,[]);
  result.checks.push('Rapid action changes retain final selection; no browser console errors');
  // A separate fresh page tests a failed HTTP fetch and an actual retry.
  const failure=await browser.newPage();await failure.route('**/assets/models/pip/pip.glb',r=>r.fulfill({status:503,body:'Unavailable for acceptance test'}));
  await failure.goto(result.url);await failure.waitForFunction(()=>window.pipState==='failed');await failure.unroute('**/assets/models/pip/pip.glb');await failure.getByRole('button',{name:'Retry loading Pip'}).click();await failure.waitForFunction(()=>window.pipState==='ready',null,{timeout:30000});await failure.close();
  result.checks.push('Fresh HTTP 503 is visible; Retry loads the actual GLB successfully');
 }catch(e){result.failure=String(e);result.state=await page.evaluate(()=>({pip:window.pipState,render:window.pawsRenderState,status:document.querySelector('#status').textContent}));await page.screenshot({path:path.join(evidence,'failure.png'),fullPage:true});throw e;}
 finally{await fs.writeFile(path.join(evidence,'pip-smoke.json'),JSON.stringify(result,null,2));await browser.close();}
 console.log(JSON.stringify(result,null,2));
})().catch(e=>{console.error(e);process.exit(1);});
