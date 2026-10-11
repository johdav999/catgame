const {chromium}=require('playwright');
const fs=require('node:fs/promises');const path=require('node:path');
(async()=>{
 const job=__dirname;await fs.mkdir(path.join(job,'browser-evidence'),{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1440,height:1000}});const errors=[];
 page.on('pageerror',e=>errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});
 if(!process.env.PIP_LIVE)await page.route('**/assets/models/pip/pip.glb',r=>r.fulfill({status:200,contentType:'model/gltf-binary',path:path.join(job,'exports/pip-merged.glb')}));
 await page.goto('http://127.0.0.1:19484/pip-preview.html');
 await page.waitForFunction(()=>window.pipState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});
 await page.evaluate(()=>Module._pip_suspend(1));
 await page.evaluate(()=>Module._pip_zoom(-350));
 const samples=[];
 for(const [label,time] of [['open',.1],['half',2.45],['closed',2.53],['reopening',2.67],['open-again',2.85]]){
  await page.evaluate(target=>new Promise(resolve=>{
   Module._pip_suspend(0);const tick=()=>{const t=Module._pip_time();if(t>=target&&t<3.2){Module._pip_suspend(1);resolve();}else requestAnimationFrame(tick);};requestAnimationFrame(tick);
  }),time);
  const actual=await page.evaluate(()=>Module._pip_time());samples.push({label,time:actual});
  await page.locator('#viewport').screenshot({path:path.join(job,'browser-evidence',label+'.png')});
  if(label==='closed'){
   await page.evaluate(()=>Module._pip_orbit(90,0));await page.locator('#viewport').screenshot({path:path.join(job,'browser-evidence','closed-side-a.png')});
   await page.evaluate(()=>Module._pip_orbit(-180,0));await page.locator('#viewport').screenshot({path:path.join(job,'browser-evidence','closed-side-b.png')});
   await page.evaluate(()=>Module._pip_orbit(90,0));
  }
 }
 await fs.writeFile(path.join(job,'browser-evidence/blink-review.json'),JSON.stringify({samples,errors,renderer:await page.evaluate(()=>window.pawsRenderState)},null,2));
 await browser.close();if(errors.length)throw Error(errors.join('\n'));console.log(JSON.stringify({samples,errors}));
})().catch(e=>{console.error(e);process.exit(1)});
