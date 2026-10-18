const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/grass-lawn');
const report={errors:[],screenshots:[]};let browser;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});const page=await context.newPage();
 const grassResponses=[];
 page.on('response',response=>{if(response.url().endsWith('/grass-lawn-v1.png'))grassResponses.push((async()=>({url:response.url(),status:response.status(),sha256:crypto.createHash('sha256').update(await response.body()).digest('hex')}))());});
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const settle=()=>page.waitForTimeout(300);
 async function click(name){const b=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);assert(b,name);await page.mouse.click(b.x+b.width/2,b.y+b.height/2);await settle();}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await settle();await shot('garden-desktop'); for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await shot('garden-'+width+'x'+height);} await page.setViewportSize({width:1440,height:900});await settle();
 await click('Requests & friends');await click('Pip\'s missing ball · Available');await click('Help Pip');
 await page.waitForFunction(()=>!pawsState.garden&&pawsState.production);await settle();await shot('puzzle-desktop');
 for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await shot('puzzle-'+width+'x'+height);}
 report.grass=await Promise.all(grassResponses);
 const expected=crypto.createHash('sha256').update(await fs.readFile(path.resolve('assets/runtime/textures/garden/grass-lawn-v1.png'))).digest('hex');
 assert(report.grass.length>0,'Grass texture was not requested');
 assert(report.grass.every(r=>r.status===200&&r.sha256===expected),'Served grass texture differs from accepted asset');
 report.build=await page.evaluate(()=>pawsBuild);report.success=report.errors.length===0;assert(report.success,JSON.stringify(report.errors));
})().catch(e=>{report.failure=e.stack;process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});


