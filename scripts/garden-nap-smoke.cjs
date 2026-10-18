const {chromium}=require('playwright');
const fs=require('node:fs/promises');
(async()=>{
 const out='build/evidence/garden-nap-cycle';await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1600,height:1000}});const errors=[];page.on('pageerror',e=>errors.push(e.message));
 try{
  await page.goto('http://127.0.0.1:8112/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  for(let i=0;i<100;i++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))==='Play'){await page.keyboard.press('Enter');break;}await page.keyboard.press('Tab');}
  await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
  const started=await page.evaluate(()=>pawsLifecycle.simulationElapsed);console.log('Cycle start',started);
  await page.screenshot({path:out+'/sleeping.png'});
  await page.waitForFunction(t=>pawsLifecycle.simulationElapsed>=t+34,started,{timeout:180000});console.log('Awake simulation time',await page.evaluate(()=>pawsLifecycle));await page.screenshot({path:out+'/awake.png'});
  await page.waitForFunction(t=>pawsLifecycle.simulationElapsed>=t+47,started,{timeout:180000});await page.screenshot({path:out+'/sleeping-again.png'});
  if(errors.length)throw Error(errors.join('\n'));
  await fs.writeFile(out+'/report.json',JSON.stringify({errors,build:await page.evaluate(()=>pawsBuild),captures:['sleeping','awake after 34 seconds','sleeping again after 47 seconds']},null,2));
  console.log('Nap cycle runtime captures complete; no browser errors.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
