const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/fruit-trees');
const report={errors:[],screenshots:[]};let browser;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});const page=await context.newPage();
 const foliageResponses=[];
 page.on('response',response=>{if(/\/(apple|pear)_tree\.glb$/.test(response.url()))foliageResponses.push((async()=>({url:response.url(),status:response.status(),sha256:crypto.createHash('sha256').update(await response.body()).digest('hex')}))());});
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const settle=()=>page.waitForTimeout(300);
 async function click(name){const b=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);assert(b,name);await page.mouse.click(b.x+b.width/2,b.y+b.height/2);await settle();}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await settle();await shot('garden-desktop');
 for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await shot('garden-'+width+'x'+height);}
 await page.setViewportSize({width:1440,height:900});await settle();
 await click('Menu');await click('Requests & friends');
 let labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 while(labels.some(n=>n.startsWith('Welcome '))){await click(labels.find(n=>n.startsWith('Welcome ')));labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
 await click('Pip\'s missing ball · Available');await click('Help Pip');
 await page.waitForFunction(()=>!pawsState.garden&&pawsState.production);await settle();await shot('puzzle-desktop');
 for(const [width,height] of [[1440,900],[390,844],[844,390]]){
 await page.setViewportSize({width,height});await settle();await shot('puzzle-'+width+'x'+height);
 const pts=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const a=pts.objects.pot_a,from=pts.slots['1,2'],to=pts.slots['1,1'];
 await page.mouse.move(a.x,a.y);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.move(a.x+to.x-from.x,a.y+to.y-from.y,{steps:8});await page.waitForTimeout(80);await page.mouse.up();
 await page.waitForFunction(()=>pawsState.objects.find(o=>o.id==='pot_a').y===1);
 async function key(name){const control=page.locator('button[data-widget-id][aria-label="'+name+'"]');await control.focus();await page.keyboard.press('Enter');await settle();}
 await key('Menu');await key('Puzzle controls');await key('Undo');await key('Back');await key('Continue');
 assert.equal(await page.evaluate(()=>pawsState.objects.find(o=>o.id==='pot_a').y),2);
 }
 report.interaction='Projected pot drag and undo passed at desktop, portrait and landscape';
 report.foliage=await Promise.all(foliageResponses);
 for(const name of ['apple_tree','pear_tree']){
 const expected=crypto.createHash('sha256').update(await fs.readFile(path.resolve('assets/runtime/models/garden/'+name+'.glb'))).digest('hex');
 const responses=report.foliage.filter(r=>r.url.endsWith('/'+name+'.glb'));
 assert(responses.length>0,name+' was not requested');
 assert(responses.every(r=>r.status===200&&r.sha256===expected),'Served tree differs from reviewed export');
 }
 report.build=await page.evaluate(()=>pawsBuild);report.success=report.errors.length===0;assert(report.success,JSON.stringify(report.errors));
})().catch(e=>{report.failure=e.stack;process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});
