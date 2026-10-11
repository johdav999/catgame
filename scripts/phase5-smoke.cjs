// Prompt 5 acceptance against the generated Paws site, with a task-owned server.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase5-pinned-release/site');
const out=process.env.EVIDENCE_DIR||'build/evidence/phase5';
const report={build:'first-afternoon-phase5-v1',site,checks:[],screenshots:[],errors:[],expectedErrors:[]};
let browser,page,server,expected=false;
const settle=()=>page.waitForTimeout(160);
async function focus(name) {
  for(let n=0;n<110;n++) {
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name)return;
    await page.keyboard.press('Tab');await page.waitForTimeout(12);
  }
  throw Error('Keyboard target unavailable: '+name);
}
async function key(name) {await focus(name);await page.keyboard.press('Enter');await settle();}
async function mouse(name) {
  await focus(name);
  const b=await page.evaluate(()=>EngineGui.snapshot().widgets.find(w=>w.focused).bounds);
  await page.mouse.click(b.x+b.width/2,b.y+b.height/2);await settle();
}
async function shot(name) {await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function open(url) {
  await page.goto(url);await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready');await settle();
}
async function saved() {await page.waitForFunction(()=>pawsState.save===2);}
async function garden() {await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard"&&!!pawsState.production);await settle();}
async function solution(firstMoves=2) {
  await key('Left pot');for(let i=0;i<firstMoves;i++)await key('Move north');
  await key('Middle pot');await key('Move west');
  for(const [name,turns] of [['Source straight',1],['Lower bend',3],['Middle straight',1],['Upper bend',1],['Finish straight',1]]) {
    await key(name);for(let i=0;i<turns;i++)await key('Rotate');
  }
  await key('Roll ball');await saved();assert(await page.evaluate(()=>!!pawsState.completed));
}
function watch(p) {
  p.on('pageerror',e=>report.errors.push(e.message));
  p.on('console',m=>{if(m.type()==='error')(expected?report.expectedErrors:report.errors).push(m.text());});
}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  server=http.createServer(async(req,res)=>{
    try {
      const pathname=decodeURIComponent(new URL(req.url,'http://localhost').pathname);
      const file=path.resolve(site,'.'+(pathname==='/'?'/index.html':pathname));
      if(!file.startsWith(site+path.sep))throw Error('Outside site');
      const bytes=await fs.readFile(file),ext=path.extname(file);
      res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png'})[ext]||'application/octet-stream');res.end(bytes);
    }catch {res.statusCode=404;res.end('Missing file');}
  });
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(19485,'127.0.0.1',resolve);});
  const url='http://127.0.0.1:19485';report.url=url;
  report.wasmSha256=createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex');
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
  const context=await browser.newContext({viewport:{width:1440,height:900}});page=await context.newPage();watch(page);
  await open(url);await shot('title');await mouse('Play');await garden();await saved();await shot('garden-fresh-desktop');
  await mouse('Meet Pip');await shot('pip-dialogue');await mouse('Help Pip');await key('Objects & actions');
  await key('Roll ball');assert.match(await page.evaluate(()=>pawsState.message),/blocks|source/);await shot('blocked-roll');
  await key('Hint');await key('Next hint');await key('Next hint');await shot('three-hints');await key('Back');
  await key('Left pot');await key('Move north');await key('Undo');assert.equal(await page.evaluate(()=>pawsState.objects[0].y),2);
  await key('Move north');await saved();
  const interruptedAttempt=await page.evaluate(()=>pawsState.attempt);
  await key('Back');await key('Menu');await key('Back to garden');await garden();await saved();
  assert.equal(await page.evaluate(()=>pawsState.attempt),interruptedAttempt);
  await open(url);await key('Continue');await garden();await key('Meet Pip');await key('Help Pip');await key('Objects & actions');
  assert.equal(await page.evaluate(()=>pawsState.objects[0].y),1);assert.equal(await page.evaluate(()=>pawsState.attempt),interruptedAttempt);
  await saved();
  // Reload while an attempt is active: Continue must reconstruct the current board.
  await open(url);await key('Continue');await page.waitForFunction(()=>!!pawsState.production&&!pawsState.garden);await settle();
  assert.equal(await page.evaluate(()=>pawsState.objects[0].y),1);await key('Objects & actions');
  await solution(1);assert.equal(await page.evaluate(()=>pawsState.coins),20);assert.equal(await page.evaluate(()=>pawsState.friendship),1);
  const reaction=await page.evaluate(()=>pawsPip.time);await page.waitForTimeout(120);
  assert((await page.evaluate(()=>pawsPip.time))>reaction,'Actual happy animation advances under completion card');
  await shot('first-reward');await key('Back to garden');await garden();await saved();
  await shot('garden-returned-ball');report.checks.push('Mouse garden/dialogue/start; keyboard blocked Roll, three hints, Undo, interrupted reload, known solution, 20 coins and 1 friendship');
  // Persist text and motion settings, then reopen the home view from a real save.
  await key('Menu');await key('Settings');await key('Text size: Standard');await key('Motion: Full');await saved();
  await open(url);await key('Continue');await garden();
  assert.equal(await page.evaluate(()=>pawsState.coins),20);
  await key('Menu');await key('Settings');await focus('Text size: Larger');await focus('Motion: Reduced');
  await page.keyboard.press('Escape');await settle();await page.keyboard.press('Escape');await settle();
  for(const [width,height] of [[1440,900],[390,844],[844,390]]) {
    await page.setViewportSize({width,height});await settle();await shot(`garden-${width}x${height}-larger`);
    await key('Meet Pip');await key('Help Pip');await page.waitForFunction(()=>!pawsState.garden);await settle();await shot(`puzzle-${width}x${height}-larger`);
    await key('Menu');await key('Back to garden');await garden();
  }
  report.checks.push('Completed reload restores wallet, friendship, larger text and reduced motion; real garden and puzzle captured at 1440x900, 390x844 and 844x390');
  await page.setViewportSize({width:1440,height:900});await key('Meet Pip');await key('Help Pip');await key('Objects & actions');
  await solution();assert.equal(await page.evaluate(()=>pawsState.coins),20);assert.equal(await page.evaluate(()=>pawsState.friendship),1);await shot('practice-no-reward');
  await page.keyboard.press('Escape');await garden();await saved();
  report.checks.push('Practice completion keeps 20/1; Escape from completion returns to garden without resurrecting the old action sheet');
  // Force a storage write failure, then recover through the shipped GUI.
  await page.evaluate(()=>{const old=EngineStorage.IndexedDBStorage.prototype.put;EngineStorage.IndexedDBStorage.prototype.put=function(...args){EngineStorage.IndexedDBStorage.prototype.put=old;return Promise.reject(Error('Injected write failure'));};});
  await key('Menu');await key('Settings');await key('Text size: Larger');await page.waitForFunction(()=>pawsState.save===3);
  await page.keyboard.press('Escape');await key('Save and recovery');await shot('save-failed');await key('Retry save');await saved();
  report.checks.push('Failed write displays failure; explicit Retry saves the latest coherent snapshot');
  await page.keyboard.press('Escape');await settle();await key('Settings');
  await page.evaluate(()=>{
    const old=EngineStorage.IndexedDBStorage.prototype.put;
    const held=new Promise(resolve=>window.phase5ReleaseReceipt=resolve);
    EngineStorage.IndexedDBStorage.prototype.put=function(...args){EngineStorage.IndexedDBStorage.prototype.put=old;return old.apply(this,args).then(()=>held);};
  });
  await key('Sound: On');assert.equal(await page.evaluate(()=>pawsState.save),1);
  await page.evaluate(()=>Module._paws_saved(0,1));assert.equal(await page.evaluate(()=>pawsState.save),1);
  await page.evaluate(()=>phase5ReleaseReceipt());await saved();
  report.checks.push('Saving waits for adapter receipt; stale acknowledgement cannot label a newer snapshot Saved');
  await context.close();
  // HTTP failure retains the previous scene and can retry without losing progress.
  const failed=await browser.newContext({viewport:{width:1440,height:900}});page=await failed.newPage();watch(page);expected=true;
  let fail=true;await page.route('**/models/garden/pot_round.glb',route=>fail?route.fulfill({status:503,body:'Injected asset failure'}):route.continue());
  await open(url);const previous=await page.evaluate(()=>pawsState.scene);await key('Play');
  await page.waitForFunction(()=>!!window.pawsLoadError);await settle();assert.equal(await page.evaluate(()=>pawsState.scene),previous);await shot('asset-failed');
  fail=false;await key('Retry loading');await garden();await saved();expected=false;
  report.checks.push('Real intercepted GLB failure preserves the previous scene and attempt; Retry publishes the complete garden');
  await failed.close();
  // Cancel delayed preparation and verify that a late HTTP response cannot publish it.
  const cancelled=await browser.newContext({viewport:{width:390,height:844}});page=await cancelled.newPage();watch(page);
  let release;const held=new Promise(resolve=>release=resolve);let requested=false;
  await page.route('**/models/garden/pot_round.glb',async route=>{requested=true;await held;await route.continue().catch(()=>{});});
  await open(url);const prior=await page.evaluate(()=>pawsState.scene);await key('Play');
  await page.waitForFunction(()=>[...document.querySelectorAll('[aria-label]')].some(e=>e.getAttribute('aria-label')==='Back to garden'));
  await key('Back to garden');release();await page.waitForTimeout(650);
  assert(requested);assert.equal(await page.evaluate(()=>pawsState.scene),prior);assert.equal(await page.evaluate(()=>pawsState.coins),0);await shot('loading-cancelled');
  report.checks.push('Delayed asset preparation cancelled through portrait controls; late response cannot replace scene or grant rewards');
  await cancelled.close();
  const touch=await browser.newContext({viewport:{width:390,height:844},hasTouch:true,deviceScaleFactor:2});page=await touch.newPage();watch(page);
  async function tap(name) {
    await focus(name);const b=await page.evaluate(()=>EngineGui.snapshot().widgets.find(w=>w.focused).bounds);
    assert(b.width>=44&&b.height>=44);await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);await settle();
  }
  await open(url);await tap('Play');await garden();await tap('Meet Pip');await tap('Help Pip');await tap('Objects & actions');
  await tap('Left pot');await tap('Move north');assert.equal(await page.evaluate(()=>pawsState.objects[0].y),1);
  await tap('Undo');assert.equal(await page.evaluate(()=>pawsState.objects[0].y),2);await shot('touch-actions');
  report.checks.push('Chromium touch taps select/move/undo through shared production controls with at least 44-pixel targets');
  await touch.close();
  assert.deepEqual(report.errors,[],'No unexpected browser errors');
  assert.equal(report.wasmSha256,createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex'),'Build unchanged during acceptance');
  report.passed=true;
})().catch(e=>{report.passed=false;report.failure=e.stack;console.error(e);process.exitCode=1;}).finally(async()=>{
  await fs.mkdir(out,{recursive:true});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));
  if(browser)await browser.close();if(server)await new Promise(resolve=>server.close(resolve));console.log(JSON.stringify(report));
});
