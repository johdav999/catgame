const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/garden-greenery');
const report={errors:[],screenshots:[]};let browser;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});const page=await context.newPage();
 const foliageResponses=[];
 page.on('response',response=>{if(/\/(?:exterior_foliage|paving)(?:_lod[12])?\.glb$/.test(response.url()))foliageResponses.push((async()=>({url:response.url(),status:response.status(),sha256:crypto.createHash('sha256').update(await response.body()).digest('hex')}))());});
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const settle=()=>page.waitForTimeout(300);
 async function click(name){const b=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);assert(b,name);await page.mouse.click(b.x+b.width/2,b.y+b.height/2);await settle();}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await settle();await shot('garden-desktop');
 await page.waitForTimeout(1500);report.performance=await page.evaluate(()=>pawsPerformance);
 assert(report.performance.current.cameraLodParts>0,'Foliage LOD was not selected in the real garden');
 await page.evaluate(()=>Module._paws_orbit(300,0));await settle();await shot('garden-side');
 await page.evaluate(()=>Module._paws_reset_view());await settle();
 await click('Menu');await click('Requests & friends');await click('Pip\'s missing ball · Available');await click('Help Pip');
 await page.waitForFunction(()=>!pawsState.garden&&pawsState.production);await settle();await shot('puzzle-desktop');
 for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await shot('puzzle-'+width+'x'+height);}
 report.foliage=await Promise.all(foliageResponses);
 for(const name of ['exterior_foliage','exterior_foliage_lod1','exterior_foliage_lod2','paving','paving_lod1','paving_lod2']){
  const expected=crypto.createHash('sha256').update(await fs.readFile(path.resolve('assets/runtime/models/garden/'+name+'.glb'))).digest('hex');
  const responses=report.foliage.filter(r=>r.url.endsWith('/'+name+'.glb'));
  assert(responses.length>0,name+' was not requested');
  assert(responses.every(r=>r.status===200&&r.sha256===expected),'Served '+name+' differs from accepted replacement');
 }
 report.build=await page.evaluate(()=>pawsBuild);report.success=report.errors.length===0;assert(report.success,JSON.stringify(report.errors));
})().catch(e=>{report.failure=e.stack;process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,(_,v)=>typeof v==='bigint'?Number(v):v,2));await browser?.close();});
