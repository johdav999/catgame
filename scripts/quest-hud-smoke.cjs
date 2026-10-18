const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/quest-hud');
const report={errors:[],screenshots:[]};let browser;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});const page=await context.newPage();
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const settle=()=>page.waitForTimeout(300);
 async function click(name){const b=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);assert(b,name);await page.mouse.click(b.x+b.width/2,b.y+b.height/2);await settle();}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 async function key(name){const control=page.locator('button[data-widget-id][aria-label="'+name+'"]');await control.focus();await page.keyboard.press('Enter');await settle();report.lastControl=name;await fs.writeFile(path.join(out,'progress.json'),JSON.stringify(report,null,2));}
 async function checkHud(){
 const controls=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 assert.deepEqual(controls,['Menu'],'HUD has only Menu');
 const menu=await page.evaluate(()=>{const id=+document.querySelector('button[aria-label="Menu"]').dataset.widgetId;return EngineGui.snapshot().widgets.find(w=>w.id===id);});
 assert(menu.bounds.x>=(await page.viewportSize()).width*.65,'Menu is on the right');
 assert(menu.bounds.y<70,'Menu is at the top');
 report.layouts??=[];report.layouts.push({viewport:page.viewportSize(),controls,menu:menu.bounds});
 }
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await settle();await checkHud();await shot('garden-desktop');
 for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await checkHud();await shot('garden-'+width+'x'+height);}
 await page.setViewportSize({width:1440,height:900});await settle();
 await key('Menu');await key('Requests & friends');
 let labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 while(labels.some(n=>n.startsWith('Welcome '))){await click(labels.find(n=>n.startsWith('Welcome ')));labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
 await click('Pip\'s missing ball · Available');await click('Help Pip');
 await page.waitForFunction(()=>!pawsState.garden&&pawsState.production);await settle();await shot('puzzle-desktop');
 for(const [width,height] of [[1440,900],[390,844],[844,390]]){
 await page.setViewportSize({width,height});await settle();await checkHud();await shot('puzzle-'+width+'x'+height);
 const pts=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const a=pts.objects.pot_a,from=pts.slots['1,2'],to=pts.slots['1,1'];
 await page.mouse.move(a.x,a.y);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.move(a.x+to.x-from.x,a.y+to.y-from.y,{steps:8});await page.waitForTimeout(80);await page.mouse.up();
 await page.waitForFunction(()=>pawsState.objects.find(o=>o.id==='pot_a').y===1);
 await key('Menu');await key('Puzzle controls');await key('Undo');await key('Back');await key('Continue');
 assert.equal(await page.evaluate(()=>pawsState.objects.find(o=>o.id==='pot_a').y),2);
 }
 report.interaction='Projected pot drag and undo passed at desktop, portrait and landscape';
 await page.setViewportSize({width:2048,height:900});await settle();await checkHud();await shot('puzzle-wide');
 await key('Menu');await key('Settings');await key('Text size: Standard');await key('Back');await key('Continue');
 for(const [width,height] of [[2048,900],[390,844],[844,390]]){await page.setViewportSize({width,height});await settle();await checkHud();await shot('puzzle-large-'+width+'x'+height);}
 await key('Menu');await key('Settings');await key('Text size: Larger');await key('Back');await key('Continue');
 await page.setViewportSize({width:1715,height:900});await key('Menu');await key('Save and recovery');
 const chooser=page.waitForEvent('filechooser',{timeout:15000});await key('Import save');await (await chooser).setFiles(path.resolve('tests/release1/progression/fixtures/mvp-v2.save'));await key('Use imported afternoon');await key('Continue');
 await key('Menu');await key('Requests & friends');
 let friends=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 while(friends.some(n=>n.startsWith('Welcome '))){await key(friends.find(n=>n.startsWith('Welcome ')));friends=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
 report.questChoices=friends;
 const gate=friends.find(n=>n.includes('gate')&&!n.endsWith(' · Locked'));
 assert(gate,'Gate quest is available in the acceptance save: '+JSON.stringify(friends));
 await key(gate);await key('Help Pip');await settle();await checkHud();await shot('gate-quest');
 report.build=await page.evaluate(()=>pawsBuild);report.success=report.errors.length===0;assert(report.success,JSON.stringify(report.errors));
})().catch(e=>{report.failure=e.stack;process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});
