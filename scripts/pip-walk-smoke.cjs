// Full-cycle exported-skin review in the generated Paws WebGPU renderer.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-walk-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'artifacts/model-jobs/pip/2026-10-11-reference-walk/browser-evidence');
const report={site,errors:[],checks:[],views:['side','front','rear','three-quarter','puzzle'],laneSamples:[]};
let server,browser;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 server=http.createServer(async(req,res)=>{try{const p=decodeURIComponent(new URL(req.url,'http://localhost').pathname);const file=path.resolve(site,'.'+(p==='/'?'/index.html':p));assert(file.startsWith(site+path.sep));const bytes=await fs.readFile(file);res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png'})[path.extname(file)]||'application/octet-stream');res.end(bytes);}catch{res.statusCode=404;res.end();}});
 await new Promise(r=>server.listen(19511,'127.0.0.1',r));
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 report.browser=browser.version();report.assetSha256=createHash('sha256').update(await fs.readFile(path.join(site,'assets/models/pip/pip.glb'))).digest('hex');
 const page=await browser.newPage({viewport:{width:1280,height:1080}});
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const loaded=page.waitForResponse(r=>r.url().endsWith('/assets/models/pip/pip.glb')&&r.ok());
 await page.goto('http://127.0.0.1:19511/pip-preview.html');await loaded;
 await page.waitForFunction(()=>window.pipState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});
 await page.getByLabel('Reduced motion').uncheck();
 const canvas=page.locator('#viewport');
 for(let view=0;view<5;view++){
  await page.selectOption('#walk-view',String(view));
  for(let frame=0;frame<36;frame++){
   await page.locator('#walk-phase').evaluate((el,t)=>{el.value=t;el.dispatchEvent(new Event('input',{bubbles:true}));},String(frame/30));
   await page.evaluate(()=>new Promise(r=>requestAnimationFrame(()=>requestAnimationFrame(r))));
   assert(Math.abs(await page.evaluate(()=>Module._pip_time())-frame/30)<.001);
   await canvas.screenshot({path:path.join(out,`cycle-${report.views[view]}-${String(frame).padStart(2,'0')}.png`)});
  }
 }
 report.checks.push('All 36 samples of the 1.2-second cycle inspected from five runtime views through visible scrub/view controls');
 await page.getByRole('button',{name:'Walk along lane',exact:true}).click();await page.selectOption('#walk-view','0');
 for(let i=0;i<45;i++){
  report.laneSamples.push({frame:i,time:await page.evaluate(()=>Module._pip_time()),wall:Date.now()});
  await canvas.screenshot({path:path.join(out,`lane-${String(i).padStart(3,'0')}.png`)});
  await page.waitForTimeout(40);
 }
 assert(report.laneSamples.some((s,i,a)=>i&&Math.abs(s.time-a[i-1].time)>.01));
 await page.evaluate(()=>Module._pip_suspend(1));const paused=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>Module._pip_time()),paused);await page.evaluate(()=>Module._pip_suspend(0));
 await page.getByLabel('Reduced motion').check();await page.getByRole('button',{name:'Walk along lane',exact:true}).click();const still=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>Module._pip_time()),still);
 await page.screenshot({path:path.join(out,'workshop.png'),fullPage:true});
 report.checks.push('Moving lane playback advances, host suspension freezes it, reduced motion remains static');
 assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(e=>{report.failure=e.stack;process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'walk-report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();if(server)await new Promise(r=>server.close(r));});
