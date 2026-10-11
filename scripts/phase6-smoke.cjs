const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const url=process.env.PAWS_URL||'http://127.0.0.1:8096';
const out=process.env.EVIDENCE_DIR||'build/evidence/phase6';
const report={build:'puzzles-phase6-v1',url,checks:[],screenshots:[],errors:[]};
let browser,page,mode='keyboard';
const settle=()=>page.waitForTimeout(160);
async function focus(name){for(let i=0;i<110;i++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name)return;await page.keyboard.press('Tab');await page.waitForTimeout(8);}throw Error('Unreachable '+name);}
async function action(name){
 await settle();await focus(name);await settle();
 if(mode==='keyboard')await page.keyboard.press('Enter');
 else {const b=await page.evaluate(name=>{const el=[...document.querySelectorAll('[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return EngineGui.snapshot().widgets.find(w=>w.id===+el.dataset.widgetId).bounds;},name);if(mode==='touch')await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);else await page.mouse.click(b.x+b.width/2,b.y+b.height/2);}
 await settle();
}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function state(){return page.evaluate(()=>pawsState);}
async function open(){await page.goto(url);await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await settle();}
async function help(){await action('Help Pip');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>['Objects & actions','Actions'].includes(e.getAttribute('aria-label'))),{timeout:60000});await settle();}
async function actions(){await action((await page.viewportSize()).height<500?'Actions':'Objects & actions');}
async function next(title){await action('Back to garden');await action('Meet Pip');await action(title);await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Help Pip'));await help();await shot(mode+"-"+title.replace(/[^a-z0-9]/gi,"-")+"-board");await actions();}
async function saved(){await page.waitForFunction(()=>pawsState.save===2);}
async function rotate(name,n){await action(name);for(let i=0;i<n;i++)await action('Rotate');}
async function first(expected=20){
 await action('Left pot');await action('Move north');await action('Move north');await action('Middle pot');await action('Move west');
 for(const [n,k]of[['Source straight',1],['Lower bend',3],['Middle straight',1],['Upper bend',1],['Finish straight',1]])await rotate(n,k);
 await action('Roll ball');assert.equal((await state()).coins,expected);await saved();await shot(mode+'-ball-complete');
}
async function toys(reload,expected=40){
 await action('Coral ball');assert(await page.getByRole('button',{name:'Pick up Coral ball',exact:true}).isDisabled());
 await shot(mode+'-toys-blocked');await action('Blocking cushion');await action('Move north');await action('Coral ball');await action('Pick up Coral ball');
 assert(await page.getByRole('button',{name:'Place in Mouse-shaped recess',exact:true}).isDisabled());
 await shot(mode+'-toys-targets');await saved();
 if(reload){await open();await action('Continue');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Objects & actions'));await actions();assert.equal((await state()).objects.find(o=>o.id==='ball').location,1);report.checks.push('Reload while holding a toy preserves independent attempt');}
 await action('Place in Round ball recess');await action('Toy mouse');await action('Pick up Toy mouse');await action('Place in Mouse-shaped recess');
 await action('Toy fish');await action('Pick up Toy fish');await action('Place in Fish-shaped recess');
 assert.equal((await state()).coins,expected);await saved();await shot(mode+'-toys-complete');
}
async function gate(reload){
 await action('Mouse behind gate');await page.mouse.move(700,250);await page.mouse.wheel(0,-1800);await settle();assert(await page.getByRole('button',{name:'Pick up Mouse behind gate',exact:true}).isDisabled());
 await action('Sliding pot');await action('Move north');await rotate('Source straight',1);await rotate('Lower bend',3);await rotate('Switch bend',1);await action('Roll ball');
 assert.equal((await state()).switchActive,1);await action('Back');await shot(mode+'-gate-open');await actions();
 await action('Return ball to source');assert.equal((await state()).switchActive,0);await action('Undo');assert.equal((await state()).switchActive,1);
 await saved();
 if(reload){await open();await action('Continue');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>['Objects & actions','Actions'].includes(e.getAttribute('aria-label'))));await actions();assert.equal((await state()).switchActive,1);report.checks.push('Reload with occupied switch preserves gate and collectible; undo toggles gate coherently');}
 await action('Mouse behind gate');await action('Pick up Mouse behind gate');await saved();assert.equal((await state()).coins,60);assert.equal((await state()).friendship,3);await shot(mode+'-gate-complete');
}
(async()=>{await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();report.layouts=['1440x900','390x844','844x390'];
 const context=await browser.newContext({viewport:{width:1440,height:900},hasTouch:true});page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 await open();report.gardenManifest=await(await page.request.get(url+'/assets/models/garden/asset-manifest.json')).json();report.gardenRevision=report.gardenManifest.revision;delete report.gardenManifest;report.wasmSha256=createHash('sha256').update(await(await page.request.get(url+'/paws.wasm')).body()).digest('hex');
 if(!process.env.PAWS_FAILURE_ONLY){
 await action('Play');await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");await action('Meet Pip');await help();await shot('production-ball-desktop');await actions();await first();await next('Tidy the toy basket');await toys(true);await next('The stuck little gate');await gate(true);
 report.checks.push('All three quests completed with Tab/Enter, ordered unlocks, 60 coins and 3 friendship');
 await action('Back to garden');await action('Meet Pip');await action("Pip's missing ball · Practice");await help();await actions();
 mode='mouse';await first(60);
 assert.equal((await state()).coins,60);await saved();report.checks.push('Mouse practice completion grants no duplicate reward');
 await next('Tidy the toy basket · Practice');await toys(false,60);
 await next('The stuck little gate · Practice');await gate(false);report.checks.push('All three puzzles also completed through canvas mouse actions');
 for(const [width,height]of[[390,844],[844,390]]){
  await page.setViewportSize({width,height});await action('Back to garden');await action('Meet Pip');await action('Tidy the toy basket · Practice');await help();await action('Menu');await action('Restart puzzle');await action('Restart puzzle');await shot(`${width}x${height}-tidy-board`);await actions();
  mode='touch';await action('Blocking cushion');await action('Move north');await action('Coral ball');await action('Pick up Coral ball');await shot(`${width}x${height}-legal-targets`);
  for(const b of await page.evaluate(()=>EngineGui.snapshot().widgets.filter(w=>w.type==='button'&&w.active)))assert(b.bounds.width>=44&&b.bounds.height>=44);
  await action('Place in Round ball recess');await action('Back');await action('Menu');await action('Settings');await action('Text size: Standard');await shot(`${width}x${height}-larger-settings`);await action('Text size: Larger');await action('Back');await action('Back to garden');await action('Meet Pip');await action('The stuck little gate · Practice');await help();await actions();await gate(false);
 }
 report.checks.push('390x844 and 844x390 touch-oriented controls, target previews, gate completion, larger text; all targets >=44 pixels');
 await open();assert.equal((await state()).coins,60);assert.equal((await state()).friendship,3);await action('Continue');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Meet Pip'));assert.equal((await state()).completed,1);report.checks.push('Reload and Continue preserve final wallet, friendship and completed attempt without implicit restart');

 }
 // Separate storage isolates failure/cancellation from the earned afternoon.
 const acceptancePage=page, acceptanceMode=mode;
 const isolated=await browser.newContext({viewport:{width:1440,height:900}});page=await isolated.newPage();mode='keyboard';
 page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error'){if(/503|ERR_ABORTED/.test(m.text()))(report.expectedErrors??=[]).push(m.text());else report.errors.push(m.text());}});
 let release;const held=new Promise(resolve=>release=resolve);
 await page.route('**/assets/models/garden/paving.glb',async route=>{await held;await route.continue().catch(()=>{});});
 await open();await action('Play');
 await page.waitForFunction(()=>EngineGui.snapshot().widgets.some(w=>w.text.includes('Loading the garden')));
 const beforeCancel=await state();await page.keyboard.press('Escape');await settle();release();await page.waitForTimeout(1000);
 assert.equal((await state()).quest,beforeCancel.quest);assert.equal((await state()).attempt,beforeCancel.attempt);assert.equal((await state()).scene,beforeCancel.scene);
 await shot('cancelled-preparation');report.checks.push('Cancelled asset preparation cannot publish its late completion');
 await page.unroute('**/assets/models/garden/paving.glb');
 await page.route('**/assets/models/garden/paving.glb',route=>route.fulfill({status:503,body:'Acceptance failure'}));
 await action('Play');await page.waitForFunction(()=>!!window.pawsLoadError);
 assert.equal((await state()).attempt,beforeCancel.attempt);await shot('asset-failure-keeps-attempt');
 await page.unroute('**/assets/models/garden/paving.glb');await action('Retry loading');await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");await action('Meet Pip');await help();
 report.checks.push('Asset HTTP failure retains active scene/attempt and Retry loading succeeds');
 // Hold a storage completion, commit two commands, and reject the older completion.
 await page.evaluate(()=>{window.phase6Writes=[];window.phase6OriginalStore=window.pawsStoreSave;window.pawsStoreSave=(text,seq)=>phase6Writes.push({text,seq});return true;});
 await actions();await action('Left pot');await action('Move north');
 const sequences=await page.evaluate(()=>phase6Writes.map(w=>w.seq));assert(sequences.length>=2);
 await page.evaluate(seq=>Module._paws_saved(seq,1),sequences[0]);assert.equal((await state()).save,1);
 await page.evaluate(seq=>Module._paws_saved(seq,0),sequences.at(-1));assert.equal((await state()).save,3);
 await action('Back');await action('Menu');await action('Save and recovery');await shot('save-failure');
 await page.evaluate(()=>window.pawsStoreSave=window.phase6OriginalStore);await action('Retry save');await saved();
 report.checks.push('Stale save completion is ignored; failed newest save stays visible and Retry persists coherent state');
 await isolated.close();page=acceptancePage;mode=acceptanceMode;
 const finalHash=createHash('sha256').update(await(await page.request.get(url+'/paws.wasm')).body()).digest('hex');assert.equal(finalHash,report.wasmSha256,'Build unchanged throughout acceptance');
 assert.equal(report.errors.length,0,report.errors.join('\n'));report.passed=true;
})().catch(async e=>{report.failure=e.stack;if(page){report.state=await state().catch(()=>null);report.loadError=await page.evaluate(()=>window.pawsLoadError).catch(()=>null);}process.exitCode=1;}).finally(async()=>{if(page)await shot('final-state').catch(()=>{});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();});

