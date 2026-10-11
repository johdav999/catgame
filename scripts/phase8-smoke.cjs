// Generated-site acceptance. New contexts isolate all injected failures from user saves.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase8-pinned-release/site');
const out=process.env.EVIDENCE_DIR||'build/evidence/phase8';
const port=Number(process.env.PAWS_TEST_PORT||19489),url=`http://127.0.0.1:${port}`;
const report={site,url,checks:[],screenshots:[],errors:[]};let browser,page,server;
const settle=()=>page.waitForTimeout(140);
async function action(name){
  console.log('Action:',name);await settle();
  for(let n=0;n<140;n++){
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await settle();return;}
    await page.keyboard.press('Tab');await page.waitForTimeout(8);
  }throw Error('Unreachable '+name);
}
const state=()=>page.evaluate(()=>pawsState);
const saved=()=>page.waitForFunction(()=>pawsState.save===2);
const home=()=>page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");
async function open(){await page.goto(url);await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await settle();}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function bytes(namespace='paws-afternoon',key='current'){return page.evaluate(async({namespace,key})=>{const s=new EngineStorage.IndexedDBStorage({namespace});try{return new TextDecoder().decode(await s.get(key));}finally{s.dispose();}},{namespace,key});}
async function importFile(buffer,name='afternoon.save') {const chooser=page.waitForEvent('filechooser');await action('Import save');await (await chooser).setFiles({name,mimeType:'application/octet-stream',buffer:Buffer.from(buffer)});await settle();}
async function download(label,name) {const event=page.waitForEvent('download');await action(label);const d=await event;await d.saveAs(path.join(out,name));return fs.readFile(path.join(out,name),'utf8');}
async function newPage(context){page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error'&&!m.text().includes('503'))report.errors.push(m.text());});return page;}
async function recovery(){await action('Menu');await action('Save and recovery');}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  server=http.createServer(async(req,res)=>{try {const pathname=decodeURIComponent(new URL(req.url,url).pathname);const file=path.resolve(site,'.'+(pathname==='/'?'/index.html':pathname));if(!file.startsWith(site+path.sep))throw Error();const b=await fs.readFile(file);res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png'})[path.extname(file)]||'application/octet-stream');res.end(b);}catch{res.statusCode=404;res.end('Missing');}});
  await new Promise(resolve=>server.listen(port,'127.0.0.1',resolve));
  report.hashes={};for(const file of ['paws.wasm','host.js','saves.js','assets/models/pip/pip.glb'])report.hashes[file]=createHash('sha256').update(await fs.readFile(path.join(site,file))).digest('hex');
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
  const opening=await browser.newContext();
  await opening.addInitScript(()=>Object.defineProperty(navigator,'locks',{value:{request:()=>new Promise(()=>{})}}));
  await newPage(opening);await page.goto(url);await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await action('Save and recovery');
  assert(await page.getByRole('button',{name:'Import save',exact:true}).isDisabled());assert(await page.getByRole('button',{name:'Export save',exact:true}).isDisabled());await opening.close();
  report.checks.push('File import remains unavailable until session ownership and initial save loading resolve');
  const context=await browser.newContext({viewport:{width:1440,height:900},hasTouch:true});
  await context.addInitScript(()=>{const old=GPUAdapter.prototype.requestDevice;GPUAdapter.prototype.requestDevice=async function(...args){const d=await old.apply(this,args);window.phase8Device=d;return d;};});
  await newPage(context);await open();await action('Play');await home();await action('Meet Pip');await action('Help Pip');await action('Objects & actions');await action('Left pot');await action('Move north');await saved();
  const attempt=(await state()).attempt;await open();await action('Continue');await page.waitForFunction(()=>pawsState.production&&!pawsState.garden);assert.equal((await state()).objects[0].y,1);assert.equal((await state()).attempt,attempt);assert((await state()).canUndo);
  await saved();const initial=await page.evaluate(()=>pawsSaveDiagnostics().committed);
  await page.evaluate(()=>{for(let i=0;i<12;i++){const s=pawsState;Module._paws_command(2,2,0,s.attempt,s.revision);}});
  assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().committed),initial);assert(await page.evaluate(()=>pawsSaveDiagnostics().pending));await saved();assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().committed),initial+1);
  report.checks.push('Mid-attempt reload preserves attempt, moved pot and undo; 12 ordinary moves coalesce to one coherent commit');
  await recovery();await saved();const exported=await download('Export save','mid-attempt.save');assert(exported.startsWith('PAWS_AFTERNOON 5'));
  await importFile(exported.replace('PAWS_AFTERNOON 5','PAWS_AFTERNOON 99'));assert((await state()).message.includes('unchanged'));assert.equal((await state()).objects[0].y,1);
  await importFile('damaged');assert((await state()).message.includes('unchanged'));
  const completed=await fs.readFile('tests/fixtures/phase7-completed.save','utf8');
  await importFile(completed);await shot('import-confirm');await action('Keep current afternoon');assert.equal((await state()).coins,0);
  await importFile(completed);await action('Use imported afternoon');await saved();assert.equal((await state()).coins,5);assert.equal((await state()).friendship,3);assert((await state()).endingSeen);
  await action('Continue');await home();await saved();await open();await action('Continue');await home();assert.equal((await state()).coins,5);assert((await state()).cushionOwned&&(await state()).bowlOwned);await shot('completed-reload');
  report.checks.push('Actual download/file picker, invalid/future import rejection, explicit cancel/replace, v3 migration and completed 5-coin/3-friendship/placement/ending reload');
  // The second tab reads a coherent snapshot, but cannot overwrite the writer.
  await saved();const primary=page,beforeSecond=await bytes();await newPage(context);await open();assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().mode),1);await action('Continue');await home();await recovery();await shot('second-tab');const secondExport=await download('Export save','second-tab.save');assert(secondExport.includes('PAWS_AFTERNOON 5'));assert.equal(await bytes(),beforeSecond);await page.close();page=primary;
  report.checks.push('Two tabs in one origin: Web Lock grants one writer, second tab is readable temporary play with export and no shared writes');
  await recovery();await saved();
  // Persistent failure, then explicit retry from the shipped recovery UI.
  await page.evaluate(()=>{window.phase8Put=EngineStorage.IndexedDBStorage.prototype.put;EngineStorage.IndexedDBStorage.prototype.put=()=>Promise.reject(Error('Injected quota failure'));Module._paws_retry_save();});
  await page.waitForFunction(()=>pawsState.save===3);await shot('write-failed');await page.evaluate(()=>EngineStorage.IndexedDBStorage.prototype.put=phase8Put);await action('Retry save');await saved();
  // Delay receipts independently of the actual adapter transactions.
  await page.evaluate(()=>{window.phase8Saved=Module._paws_saved;window.phase8Receipts=[];Module._paws_saved=(...args)=>phase8Receipts.push(args);Module._paws_retry_save();Module._paws_retry_save();});
  await page.waitForFunction(()=>phase8Receipts.length===2);await page.evaluate(()=>phase8Saved(...phase8Receipts[0]));assert.equal((await state()).save,1);await page.evaluate(()=>phase8Saved(phase8Receipts[1][0],0));assert.equal((await state()).save,3);await page.evaluate(()=>{phase8Saved(...phase8Receipts[0]);Module._paws_saved=phase8Saved;});assert.equal((await state()).save,3);await action('Retry save');await saved();
  const keys=await page.evaluate(async()=>new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'}).list());assert.equal(keys.filter(k=>k.startsWith('history-')).length,5);assert.equal(keys.length,6);
  report.checks.push('Failed transaction stays Failed until Retry; reordered receipts cannot mark a newer revision durable; history bounded to five snapshots plus current');
  // Pause and composed suspension reasons survive background/return and GPU loss.
  const clock=await page.evaluate(()=>pawsLifecycle.simulationElapsed);
  await page.evaluate(()=>{document.dispatchEvent(new Event('freeze'));window.dispatchEvent(new PageTransitionEvent('pagehide',{persisted:true}));document.dispatchEvent(new Event('resume'));});
  await page.waitForTimeout(250);assert(await page.evaluate(()=>pawsLifecycle.suspended));assert.equal(await page.evaluate(()=>pawsLifecycle.simulationElapsed),clock);
  await page.evaluate(()=>window.dispatchEvent(new PageTransitionEvent('pageshow',{persisted:true})));await settle();assert(!await page.evaluate(()=>pawsLifecycle.suspended));assert(await page.evaluate(()=>pawsLifecycle.paused));assert.equal(await page.evaluate(()=>pawsLifecycle.simulationElapsed),clock);
  const generation=await page.evaluate(()=>pawsRenderState.generation);await page.evaluate(()=>phase8Device.destroy());await page.waitForFunction(g=>pawsRenderState.state==='ready'&&pawsRenderState.generation>g,generation);assert(await page.evaluate(()=>pawsLifecycle.paused));assert.equal(await page.evaluate(()=>pawsLifecycle.simulationElapsed),clock);await shot('gpu-recovered-paused');
  await saved();await open();await action('Continue');await home();assert(await page.evaluate(()=>pawsLifecycle.paused));await action('Save and recovery');
  report.checks.push('Composed freeze/pagehide/resume/pageshow boundaries preserve explicit pause and clock; actual WebGPU device destruction recovers scene/GUI; reload preserves user pause');
  for(const [width,height]of [[390,844],[844,390]]) {await page.setViewportSize({width,height});await settle();await shot(`recovery-${width}x${height}`);for(const b of await page.evaluate(()=>EngineGui.snapshot().widgets.filter(w=>w.type==='button'&&w.active)))assert(b.bounds.width>=44&&b.bounds.height>=44);}
  await page.setViewportSize({width:1440,height:900});
  // Preserve the original bytes while recovering a validated earlier snapshot.
  const future='PAWS_AFTERNOON 99 preserved-future-bytes';await page.evaluate(async text=>{await new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'}).put('current',new TextEncoder().encode(text));},future);
  await open();assert((await page.evaluate(()=>pawsSaveDiagnostics().namespace)).startsWith('paws-isolated-'));assert.equal((await state()).coins,5);await action('Save and recovery');assert.equal(await download('Export original save','future-original.save'),future);assert.equal(await bytes(),future);await shot('recovered-history');
  await action('Back');await action('Continue');await home();await saved();assert.equal(await bytes(),future);
  report.checks.push('Future current save is preserved byte-for-byte and exportable; newest compatible history restores full economy; subsequent writes use an independent slot');
  await context.close();
  // Unavailable storage and missing browser locking are separate fallback policies.
  const temporary=await browser.newContext({viewport:{width:390,height:844}});
  await temporary.addInitScript(()=>{window.phase8IDB=indexedDB;Object.defineProperty(window,'indexedDB',{configurable:true,value:undefined});});await newPage(temporary);await open();assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().mode),2);await action('Play');await home();await recovery();await shot('storage-unavailable');await download('Export save','temporary.save');
  await page.evaluate(()=>Object.defineProperty(window,'indexedDB',{configurable:true,value:phase8IDB}));await action('Retry save');await saved();assert((await page.evaluate(()=>pawsSaveDiagnostics().namespace)).startsWith('paws-isolated-'));await temporary.close();
  const noLocks=await browser.newContext();await noLocks.addInitScript(()=>Object.defineProperty(navigator,'locks',{value:undefined}));await newPage(noLocks);await open();const slot=await page.evaluate(()=>pawsSaveDiagnostics().namespace);await action('Play');await home();await saved();assert(slot.startsWith('paws-isolated-'));await noLocks.close();
  report.checks.push('Unavailable IndexedDB permits identified temporary play/export; Retry uses an independent slot; absent Web Locks never writes the shared slot');
  const rapid=await browser.newContext({viewport:{width:1440,height:900}});await newPage(rapid);await open();await action('Play');await home();await action('Meet Pip');await action('Help Pip');await saved();
  const oldAttempt=(await state()).attempt;
  await page.evaluate(()=>{window.phase8Old={...pawsState};for(let n=0;n<8;n++){const s=pawsState;Module._paws_command(5,-1,0,s.attempt,s.revision);}window.phase8Revision=pawsState.revision;Module._paws_command(1,0,0,phase8Old.attempt,phase8Old.revision);});
  assert.equal((await state()).attempt,oldAttempt+8);assert.equal((await state()).revision,await page.evaluate(()=>phase8Revision));await saved();
  // Hidden-page boundary flushes the debounce immediately and clears the gesture owner.
  await page.mouse.move(1000,400);await page.mouse.down();assert(await page.evaluate(()=>pawsHostState().owner));
  await page.evaluate(()=>{const s=pawsState;Module._paws_command(1,0,0,s.attempt,s.revision);Object.defineProperty(document,'hidden',{configurable:true,value:true});document.dispatchEvent(new Event('visibilitychange'));});
  assert.equal(await page.evaluate(()=>pawsHostState().owner),null);assert(!await page.evaluate(()=>pawsSaveDiagnostics().pending));await saved();
  const hiddenTime=await page.evaluate(()=>pawsLifecycle.simulationElapsed);await page.waitForTimeout(300);assert.equal(await page.evaluate(()=>pawsLifecycle.simulationElapsed),hiddenTime);
  await page.evaluate(()=>{delete document.hidden;document.dispatchEvent(new Event('visibilitychange'));});await page.mouse.up();await settle();assert((await page.evaluate(()=>pawsLifecycle.simulationElapsed))-hiddenTime<.2);
  for(let n=0;n<3;n++){await action('Menu');await action('Back to garden');await home();await action('Meet Pip');await action('Help Pip');}
  await saved();await open();await action('Continue');await page.waitForFunction(()=>pawsState.production&&!pawsState.garden);assert.equal((await state()).attempt,oldAttempt+8);assert.equal((await state()).objects[0].y,1);assert.equal((await state()).coins,0);
  // Seed a broken file with no valid backups, then verify fresh independent play.
  const damaged='broken saved afternoon';await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});for(const key of await s.list())await s.remove(key);await s.put('current',new TextEncoder().encode(text));},damaged);
  await open();assert.equal((await state()).coins,0);assert((await page.evaluate(()=>pawsSaveDiagnostics().namespace)).startsWith('paws-isolated-'));await action('Save and recovery');assert.equal(await download('Export original save','damaged-original.save'),damaged);assert.equal(await bytes(),damaged);await shot('damaged-no-history');await rapid.close();
  report.checks.push('Eight rapid restarts reject old commands; three garden/board replacements preserve attempt; synthetic hidden boundary flushes/clears held input without catch-up; damaged save without history remains exportable and unchanged');
  const cancelled=await browser.newContext();await newPage(cancelled);let release;const held=new Promise(resolve=>release=resolve);
  await page.route('**/models/garden/pot_round.glb',async route=>{await held;await route.continue().catch(()=>{});});await open();const prior=(await state()).scene;await action('Play');await action('Back to garden');release();await page.waitForTimeout(700);assert.equal((await state()).scene,prior);assert.equal((await state()).coins,0);await cancelled.close();
  report.checks.push('Cancelled asset preparation cannot publish a late response or grant progress');
  assert.deepEqual(report.errors,[]);for(const [file,hash]of Object.entries(report.hashes))assert.equal(createHash('sha256').update(await fs.readFile(path.join(site,file))).digest('hex'),hash,'Tested build changed');report.passed=true;
})().catch(async error=>{report.failure=error.stack;report.state=await state().catch(()=>null);process.exitCode=1;}).finally(async()=>{if(page&&!page.isClosed())await shot('final').catch(()=>{});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();await new Promise(resolve=>server?server.close(resolve):resolve());});

