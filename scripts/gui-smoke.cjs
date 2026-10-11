// Actual pinned Wasm/WebGPU acceptance. Use a PAWS_GUI_GALLERY=ON development build.
const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const url=process.env.PAWS_URL||'http://127.0.0.1:19474';
const out=process.env.EVIDENCE_DIR||'build/evidence/gui-phase2';
const result={url,build:'gui-phase2-v1',checks:[],errors:[],screenshots:[],layouts:[]};
let browser,page;
const settle=()=>page.waitForTimeout(140);
async function focus(name) {
  for(let n=0;n<80;n++) {
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name) return;
    await page.keyboard.press('Tab');await page.waitForTimeout(12);
  }
  throw Error('Keyboard target not reachable: '+name);
}
async function key(name) {await focus(name);await page.keyboard.press('Enter');await settle();}
async function shot(name) {const file=name+'.png';await page.screenshot({path:path.join(out,file)});result.screenshots.push(file);}
async function open(gallery=false) {
  await page.goto(url+(gallery?'/?gallery=1':'/'));
  await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsRenderState?.state==='ready'&&window.EngineGui?.snapshot().fontReady);
  await settle();
}
async function geometry() {
  const info=await page.evaluate(()=>({widgets:EngineGui.snapshot().widgets,names:[...document.querySelectorAll('[data-widget-id]')].map(e=>({id:+e.dataset.widgetId,name:e.getAttribute('aria-label')}))}));
  const buttons=info.widgets.filter(w=>w.type==='button'&&w.active);
  for(const b of buttons) assert(b.bounds.height>=44&&b.bounds.width>=44,'Minimum touch size');
  for(const n of info.names) {
    const b=buttons.find(b=>b.id===n.id);if(!b)continue;
    await focus(n.name);
    const focused=await page.evaluate(()=>EngineGui.snapshot().widgets.find(w=>w.focused));
    assert(focused.bounds.x>=focused.clip.x-1&&focused.bounds.y>=focused.clip.y-1,'Focused action starts inside clip: '+n.name);
    assert(focused.bounds.x+focused.bounds.width<=focused.clip.x+focused.clip.width+1,'Focused action fits width: '+n.name);
    assert(focused.bounds.y+focused.bounds.height<=focused.clip.y+focused.clip.height+1,'Focused action fits height: '+n.name);
  }
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),'No horizontal page overflow');
}
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:1440,height:900},deviceScaleFactor:1});
 result.browser=browser.version();
 page.on('pageerror',e=>result.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')result.errors.push(m.text());});
 await open();await shot('title-desktop');
 result.wasmSha256=createHash('sha256').update(await (await page.request.get(url+'/paws.wasm')).body()).digest('hex');
 result.contentId=await page.evaluate(()=>pawsState.contentId);
 await focus('Settings');
 const playBounds=await page.evaluate(()=>{const el=[...document.querySelectorAll('[data-widget-id]')].find(e=>e.getAttribute('aria-label')==='Play');return EngineGui.snapshot().widgets.find(w=>w.id===+el.dataset.widgetId).bounds;});
 await page.mouse.move(playBounds.x+30,playBounds.y+25);await settle();await shot('button-hover');
 await page.mouse.down();await settle();await shot('button-pressed');await page.mouse.move(20,20);await page.mouse.up();
 await key('Play');await key('Meet Pip');await key('Help Pip');await shot('puzzle-desktop');
 await key('Objects & actions');
 await key('Hint');await key('Next hint');await key('Next hint');await shot('all-hints');await key('Back');
 await key('Roll ball');assert.match(await page.evaluate(()=>pawsState.message),/blocks|source/);await shot('blocked-roll');
 await key('Left pot');await key('Move north');
 const moved=await page.evaluate(()=>pawsState);assert.equal(moved.objects[0].y,1);
 await key('Undo');assert.equal(await page.evaluate(()=>pawsState.objects[0].y),2);
 await key('Left pot');await key('Move north');await key('Move north');
 await key('Middle pot');await key('Move west');
 for(const [name,turns] of [['Source straight',1],['Lower bend',3],['Middle straight',1],['Upper bend',1],['Finish straight',1]]) {
   await key(name);for(let i=0;i<turns;i++)await key('Rotate');
 }
 await key('Roll ball');assert.equal(await page.evaluate(()=>!!pawsState.completed),true);await shot('completed-keyboard');
 result.checks.push('Known solution completed using only Tab/Enter; blocked roll; undo; semantic object/action selection');
 await key('Back to garden');await key('Meet Pip');await key('Help Pip');await key('Menu');await key('Restart puzzle');
 const attempt=await page.evaluate(()=>pawsState.attempt);await key('Keep playing');assert.equal(await page.evaluate(()=>pawsState.attempt),attempt);
 await key('Restart puzzle');await key('Restart puzzle');assert.equal(await page.evaluate(()=>pawsState.attempt),attempt+1);
 result.checks.push('Restart cancellation preserves attempt; confirmed restart replaces attempt');
 await key('Menu');await key('Settings');await key('Text size: Standard');await shot('settings-larger-desktop');
 await page.keyboard.press('Escape');await settle();assert.equal(await page.evaluate(()=>document.activeElement.getAttribute('aria-label')),'Settings');
 await page.keyboard.press('Escape');await settle();assert.equal(await page.evaluate(()=>document.activeElement.getAttribute('aria-label')),'Menu');
 result.checks.push('Nested modal capture and focus return, larger text');
 // A GUI drag owns its contact through release. Compare only unobstructed world pixels.
 await page.setViewportSize({width:1440,height:900});await open();await key('Play');
 const world=()=>page.screenshot({clip:{x:550,y:200,width:500,height:400}});
 const base=await world();
 const menuBounds=await page.evaluate(()=>{const id=+[...document.querySelectorAll('[data-widget-id]')].find(e=>e.getAttribute('aria-label')==='Menu').dataset.widgetId;return EngineGui.snapshot().widgets.find(w=>w.id===id).bounds;});
 await page.mouse.move(menuBounds.x+20,menuBounds.y+20);await page.mouse.down();
 await page.mouse.move(650,400,{steps:8});assert.equal(await page.evaluate(()=>pawsHostState().owner),'gui');
 await page.mouse.up();await settle();assert(base.equals(await world()),'GUI drag must not orbit');
 const rev=await page.evaluate(()=>pawsState.revision);
 await page.mouse.move(800,450);await page.mouse.down();await page.mouse.move(880,480,{steps:8});await page.mouse.up();await settle();
 assert(!base.equals(await world()),'Background drag does orbit');assert.equal(await page.evaluate(()=>pawsState.revision),rev,'Camera does not commit a puzzle move');
 result.checks.push('GUI drag never orbits; background drag orbits without a puzzle command');
 const samples=['Title / continue','Garden HUD','Pip dialogue','Puzzle actions','Puzzle HUD','Hints','Pause','Settings','Restart confirmation','Completion / rewards','Shop / details','Placement','Ending','Save / recovery'];
 for(const [width,height] of [[1440,900],[390,844],[844,390]]) for(const large of [false,true]) {
   await page.setViewportSize({width,height});await open(true);
   if(large) {await key('Settings');await key('Text size: Standard');await key('Back');}
   for(const name of samples) {
     await key(name);await shot(`${width}x${height}-${large?'large':'normal'}-${name.replaceAll(/[^a-z0-9]/gi,'-')}`);
     await geometry();
     // Escape closes overlay previews; HUD/title are inspected in a fresh gallery next.
     await open(true);
     if(large) {await key('Settings');await key('Text size: Standard');await key('Back');}
   }
   result.layouts.push({width,height,largerText:large,surfaces:samples.length,minimumTarget:44});
   await open();if(large){await key('Settings');await key('Text size: Standard');await key('Back');}
   await key('Play');await key('Meet Pip');await key('Help Pip');await shot(`${width}x${height}-${large?'large':'normal'}-actual-puzzle`);
 }
 await page.setViewportSize({width:390,height:844});await open(true);await key('Save / recovery');
 for(let n=0;n<6;n++){await shot(`save-state-${n}`);await key('Next save state');}
 await open(true);await key('Shop / details');await key('Garden cushion · 30 coins');
 const before=await page.evaluate(()=>JSON.stringify(pawsState));await key('Buy');assert.equal(await page.evaluate(()=>JSON.stringify(pawsState)),before);await shot('shop-preview-no-purchase');
 result.checks.push('All gallery surfaces at three viewports and both text scales; focused controls fit; six save states; no fake purchase');
 await open(true);await key('Placement');await key('Preview valid / blocked');await shot('placement-valid');
 await key('Preview valid / blocked');await shot('placement-blocked');
 await open(true);await key('Completion / rewards');await key('Preview reward / practice');await shot('completion-practice');
 await open(true);await key('Shop / details');
 for(let n=0;n<3;n++){await key('Preview shop state');await shot(`shop-state-${n}`);await key('Garden cushion · '+(n===2?'Owned':'30 coins'));await shot(`shop-details-state-${n}`);await key('Back');}
 // Native touch events on the canvas at a named engine widget's bounds.
 await page.close();page=await browser.newPage({viewport:{width:390,height:844},hasTouch:true,deviceScaleFactor:2});
 page.on('pageerror',e=>result.errors.push(e.message));
 let expectedNetworkFailure=false;result.expectedFailureMessages=[];
 page.on('console',m=>{if(m.type()==='error')(expectedNetworkFailure?result.expectedFailureMessages:result.errors).push(m.text());});
 await page.emulateMedia({reducedMotion:'reduce'});
 await open();
 async function tap(name) {
   const b=await page.evaluate(name=>{const el=[...document.querySelectorAll('[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);const b=EngineGui.snapshot().widgets.find(w=>w.id===+el.dataset.widgetId).bounds;const canvas=document.querySelector('canvas').getBoundingClientRect();return {...b,x:b.x+canvas.x,y:b.y+canvas.y};},name);
   await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);await settle();
 }
 await tap('Play');await tap('Meet Pip');await tap('Help Pip');await tap('Objects & actions');
 const session=await page.context().newCDPSession(page);const state=await page.evaluate(()=>JSON.stringify(pawsState));
 await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:190,y:600}]});
 await session.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:190,y:250}]});
 await session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await settle();
 assert.equal(await page.evaluate(()=>JSON.stringify(pawsState)),state,'Touch scroll does not commit');
 assert(await page.evaluate(()=>EngineGui.snapshot().widgets.some(w=>w.scrollOffset>0)),'Touch scroll moves the sheet');
 await shot('touch-scrolling-2x');result.checks.push('Chromium touch tap and drag-scroll at 2x DPI; no accidental command');
 await key('Back');await key('Menu');await key('Settings');await focus('Motion: Reduced');await shot('reduced-motion-preference');
 await key('Back');await key('Continue');
 await page.evaluate(()=>document.querySelector('main').style.inset='18px 12px 26px 12px');await settle();
 await tap('Menu');await shot('safe-area-modal');await tap('Back');
 result.checks.push('Reduced-motion system preference and inset canvas pointer coordinates');
 // Measured opaque pixel palette: UI pass preserves authored sRGB tokens.
 const p=PNG.sync.read(await page.screenshot());let cream=0,teal=0;
 for(let i=0;i<p.data.length;i+=4){if(p.data[i]===255&&p.data[i+1]===248&&p.data[i+2]===233)cream++;if(p.data[i]===40&&p.data[i+1]===102&&p.data[i+2]===93)teal++;}
 assert(cream>1000&&teal>100,'Authored cream and teal in actual rendered pixels');
 const lum=rgb=>rgb.map(v=>v/255).map(v=>v<=.04045?v/12.92:((v+.055)/1.055)**2.4).reduce((s,v,i)=>s+v*[.2126,.7152,.0722][i],0);
 const contrast=(a,b)=>(Math.max(lum(a),lum(b))+.05)/(Math.min(lum(a),lum(b))+.05);
 result.contrast={inkCream:contrast([37,62,55],[255,248,233]),creamTeal:contrast([255,248,233],[40,102,93]),disabledInkLinen:contrast([37,62,55],[232,221,199]),errorCream:contrast([169,67,53],[255,248,233])};
 Object.values(result.contrast).forEach(v=>assert(v>=4.5));
 assert.equal(createHash('sha256').update(await (await page.request.get(url+'/paws.wasm')).body()).digest('hex'),result.wasmSha256,'Build remains unchanged during acceptance');
 expectedNetworkFailure=true;
 await page.route('**/assets/fonts/NunitoSans-Regular.ttf',route=>route.fulfill({status:503,body:'Unavailable'}));
 await page.goto(url);await page.waitForFunction(()=>document.querySelector('#status').dataset.error==='true');
 assert.match(await page.locator('#status').textContent(),/lettering could not load/);assert(await page.locator('#retry').isVisible());await shot('font-load-recovery');
 result.checks.push('Real startup font-fetch failure shows retry and never enters the game');
 assert.deepEqual(result.errors,[]);result.passed=true;
})().catch(async e=>{result.failure=String(e.stack);if(page)await shot('failure');process.exitCode=1;}).finally(async()=>{
 await fs.writeFile(path.join(out,'report.json'),JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));if(browser)await browser.close();
});
