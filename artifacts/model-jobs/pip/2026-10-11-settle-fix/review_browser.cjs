const {chromium}=require('playwright');
const fs=require('node:fs/promises');const path=require('node:path');const assert=require('node:assert/strict');
(async()=>{
 const evidence=path.join(__dirname,'browser');await fs.mkdir(evidence,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1280,height:920}});
 const page=await context.newPage();const errors=[];page.on('pageerror',e=>errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});
 await page.goto('http://127.0.0.1:8099/pip-preview.html');
 try {await page.waitForFunction(()=>window.pipState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:20000});}
 catch(e){console.log(errors,await page.evaluate(()=>({state:window.pipState,render:window.pawsRenderState,body:document.body.innerText})));await page.screenshot({path:path.join(evidence,'failure.png')});await browser.close();throw e;}
 await page.getByRole('button',{name:'Settle on cushion',exact:true}).click();
 const samples=[];
 for(const t of [.2,.6,1,1.5,2,2.4]){
  await page.waitForFunction(t=>Module._pip_time()>=t,t);await page.evaluate(()=>Module._pip_suspend(1));
  samples.push(await page.evaluate(()=>Module._pip_time()));
  await page.locator('#viewport').screenshot({path:path.join(evidence,`settle-${t}.png`)});
  await page.evaluate(()=>Module._pip_suspend(0));
 }
 await page.getByLabel('Reduced motion').check();
 for(const [label,angle] of [['front',0],['side',-180],['rear',-255],['other',-255]]){
  if(angle)await page.evaluate(a=>Module._pip_orbit(a,0),angle);
  await page.waitForTimeout(100);await page.locator('#viewport').screenshot({path:path.join(evidence,`end-${label}.png`)});
 }
 await page.getByRole('button',{name:'Curled rest',exact:true}).click();await page.waitForTimeout(150);
 await page.locator('#viewport').screenshot({path:path.join(evidence,'matching-rest.png')});
 await page.goto('http://127.0.0.1:8099/garden-preview.html');
 await page.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});
 await page.getByRole('button',{name:'Furnishings',exact:true}).click();await page.waitForTimeout(200);
 await page.locator('#viewport').screenshot({path:path.join(evidence,'cushion-fit.png')});
 assert.deepEqual(errors,[]);await fs.writeFile(path.join(evidence,'review.json'),JSON.stringify({url:page.url(),browser:browser.version(),samples,errors},null,2));
 await context.close();await browser.close();
})().catch(e=>{console.error(e);process.exit(1)});
