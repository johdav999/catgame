// Production controls with isolated browser storage. Serve the rebuilt site first.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const url=process.env.PAWS_URL||'http://localhost:19645/';
const out=process.env.EVIDENCE_DIR||'build/evidence/restart-puzzle';
const report={url,checks:[],errors:[]};let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:1280,height:720},hasTouch:true});
 page.on('pageerror',e=>report.errors.push(e.message));
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(300);
 const control=name=>page.getByRole('button',{name,exact:true});
 async function key(name){await control(name).focus();await page.keyboard.press('Enter');await settle();}
 async function tap(name){await control(name).focus();await settle();const b=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);await settle();}
 async function ready(){await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');}
 async function restart(){await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden&&!pawsState.paused);}
 await page.goto(url);await ready();report.build=await page.evaluate(()=>pawsBuild);
 await key('Play');await page.waitForFunction(()=>pawsUi.screen===1&&pawsState.garden);
 const initial=await state();await key('Menu');assert(await control('Restart puzzle').isEnabled());await key('Restart puzzle');await key('Keep playing');assert.equal((await state()).attempt,initial.attempt);assert((await state()).garden);
 await restart();assert.equal((await state()).attempt,initial.attempt+1);assert.deepEqual((await state()).objects,initial.objects);assert.equal((await state()).coins,initial.coins);report.checks.push('Fresh garden: restart enabled, cancellation preserves attempt, confirmation opens a new puzzle attempt');
 const points=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});const pot=initial.objects.find(o=>o.id==='pot_a');const a=points.objects.pot_a,b=points.slots[`${pot.x},${pot.y-1}`];assert(a&&b);await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});await page.mouse.up();await settle();assert((await state()).canUndo);assert.notDeepEqual((await state()).objects,initial.objects);
 await key('Menu');await key('Back to garden');await page.waitForFunction(()=>pawsUi.screen===1&&pawsState.garden);
 const moved=await state();await page.setViewportSize({width:390,height:844});await settle();await key('Menu');assert(await control('Restart puzzle').isEnabled());await tap('Restart puzzle');await tap('Keep playing');assert.deepEqual((await state()).objects,moved.objects);assert.equal((await state()).attempt,moved.attempt);
 await tap('Restart puzzle');await page.screenshot({path:path.join(out,'garden-confirmation.png')});await tap('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);
 assert.equal((await state()).attempt,moved.attempt+1);assert.deepEqual((await state()).objects,initial.objects);assert(!(await state()).canUndo);assert.equal((await state()).coins,initial.coins);report.checks.push('Unfinished moved request: garden touch restart clears moves and rebuilds the puzzle scene; cancel preserves moves');
 const attempt=(await state()).attempt;await key('Menu');await restart();assert.equal((await state()).attempt,attempt+1);report.checks.push('Paused puzzle restart still resumes and resets normally');
 await page.waitForFunction(()=>pawsState.save===2);const persisted=await state();await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);assert.equal((await state()).attempt,persisted.attempt);assert.deepEqual((await state()).objects,initial.objects);assert.equal((await state()).coins,initial.coins);report.checks.push('Restarted attempt and initial layout survive IndexedDB reload');
 await page.screenshot({path:path.join(out,'restarted-puzzle.png')});assert.deepEqual(report.errors,[]);report.success=true;
})().catch(async e=>{report.failure=e.stack;process.exitCode=1;console.error(e);if(page){report.diagnostics=await page.evaluate(()=>({loadError:window.pawsLoadError,state:window.pawsState,ui:window.pawsUi}));await page.screenshot({path:path.join(out,'failure.png')});}}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();console.log(JSON.stringify(report));});


