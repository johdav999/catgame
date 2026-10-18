// Generated production site; isolated storage; all puzzle moves are real drags.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/playhouse/browser');
const fixtureDir=path.resolve(process.env.FIXTURE_DIR||'build/evidence/playhouse');
const url=process.env.PAWS_URL||'http://127.0.0.1:19647/';
const report={url,checks:[],errors:[],screenshots:[]};let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:1600,height:1000}});page.setDefaultTimeout(60000);
 if(process.env.RENDER_BACKEND==='webgl2')await page.addInitScript(()=>{if(navigator.gpu)navigator.gpu.requestAdapter=async()=>null;});
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(220);
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function drag(id,target,cancel=false){
  const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b,JSON.stringify({id,target,p}));
  // Crate destinations describe the actor origin; retain the grabbed offset.
  const o=(await state()).objects.find(o=>o.id===id),from=p.slots[`${o.x},${o.y}`];
  const x=id==='crate'?a.x+b.x-from.x:b.x,y=id==='crate'?a.y+b.y-from.y:b.y;
  await page.mouse.move(a.x,a.y);await page.mouse.down();
  if(id==='crate'){
   const s=await state(),blocked=!s.actions.some(a=>a.object==='crate'&&a.verb===1&&a.enabled);
   const guides=(await points()).guides;
   assert.equal(guides.length===0,blocked,'Crate outlines must show exactly when it can move');
   if(!blocked)await shot('crate-destinations-'+s.revision);
  }
  await page.mouse.move(x,y,{steps:16});
  if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));
  await page.mouse.up();await settle();
 }
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 async function expectAt(id,x,y){const o=(await state()).objects.find(o=>o.id===id);assert.deepEqual([o.x,o.y],[x,y],JSON.stringify(await state()));}
 const active=async id=>(await state()).objects.find(o=>o.id===id).active;
 async function unchanged(id,target,reason){const s=await state();await drag(id,target);const after=await state();assert.equal(after.revision,s.revision);assert.deepEqual(after.objects,s.objects);assert.match(after.message,reason);}
 async function load(name){
  await page.goto(url);await ready();const text=await fs.readFile(path.join(fixtureDir,name),'utf8');
  await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},text);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);await settle();
 }
 await load('playhouse.save');report.build=await page.evaluate(()=>pawsBuild);report.renderer=await page.evaluate(()=>pawsRenderState);if(process.env.RENDER_BACKEND)assert.equal(report.renderer.backend,process.env.RENDER_BACKEND);await shot('start');
 await unchanged('pepper','storage',/side gate is closed/);
 await unchanged('crate','5,2',/side gate is closed/);
 await unchanged('pip','inside_latch',/through the doorway/);
 await drag('pip','pip_switch');assert(await active('side_gate'));
 await unchanged('crate','5,2',/Move Pepper/);
 await unchanged('crate','5,3',/Move Pepper/);
 {const before=await state(),p=(await points()).objects.crate;await page.mouse.click(p.x,p.y);await settle();assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/Move Pepper/);}
 report.checks.push('Tapping the blocked crate and dragging beyond its track both explain the missing helper');
 await drag('pepper','pepper_switch');assert(await active('playhouse_door'));assert(!(await state()).completed);
 await unchanged('pip','inside_entry',/would close/);
 report.checks.push('Closed gate, missing helper, early latch and two-switch shortcut are rejected without changing state');
 await drag('pepper','storage');await expectAt('pepper',6,1);assert(!await active('playhouse_door'));await shot('pepper-storage');
 const before=await state();await drag('crate','5,2',true);assert.equal((await state()).revision,before.revision);await expectAt('crate',5,1);
 await drag('crate','5,2');await expectAt('crate',5,2);await drag('pip','waiting');assert(!await active('side_gate'));await shot('crate-outside');
 await unchanged('pepper','pepper_switch',/side gate is closed/);
 await key('Menu');await key('Undo');assert((await state()).paused);assert(await active('side_gate'));await expectAt('pip',1,3);await key('Continue');
 await drag('pip','waiting');await drag('crate','1,3');await expectAt('crate',1,3);assert(await active('pip_switch'));assert(await active('side_gate'));
 report.checks.push('Crate traverses the whole track in one drag; pause Undo restores Pip and gate; cancellation preserves the crate');await shot('crate-on-switch');
 await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);await expectAt('crate',1,3);assert(await active('side_gate'));
 await drag('pepper','pepper_switch');await drag('pip','inside_entry');await expectAt('pip',2,1);await unchanged('pepper','pepper_inside',/would close/);await shot('pip-inside');
 await drag('pip','inside_latch');assert(await active('inside_latch'));
 await key('Menu');await key('Undo');assert(!await active('inside_latch'));await expectAt('pip',2,1);await key('Continue');await drag('pip','inside_latch');
 await drag('pepper','pepper_waiting');assert(await active('playhouse_door'));assert(!await active('pepper_switch'));assert(!(await state()).completed);await shot('door-latched');
 await drag('pepper','pepper_inside');await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);assert.equal((await state()).coins,160);await shot('both-inside');
 report.checks.push('Mid-quest reload restores crate and switches; Undo unlatches; latch holds after Pepper leaves; completion requires both kittens inside');
 await key('Back to garden');await page.waitForFunction(()=>pawsState.garden);await key('Menu');await key('Requests & friends');
 for(;;){const name=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')).find(n=>n?.startsWith('Welcome ')));if(!name)break;await key(name);}
 await key('Two kittens, one doorway · Practice');await key('Help Pip and Pepper');await page.waitForFunction(()=>pawsUi.screen===2);assert(!await active('inside_latch'));assert.equal((await state()).coins,160);
 for(const [width,height] of [[390,844],[844,390]]){
  await page.setViewportSize({width,height});await settle();await shot(`layout-${width}x${height}`);
  const p=await points(),board=await page.evaluate(()=>pawsUi.board);
  for(const id of ['pip_switch','pepper_switch','storage','inside_entry','inside_latch','pepper_inside','waiting']){
   const s=p.slots[id];assert(s.x>=board.x&&s.x<=board.x+board.width&&s.y>=board.y&&s.y<=board.y+board.height,JSON.stringify({id,s,board,width,height}));
  }
 }
 await page.setViewportSize({width:1600,height:1000});
 await load('playhouse-v1-complete.save');assert((await state()).objects.some(o=>o.id==='crate'));assert.equal((await state()).coins,160);assert(!(await state()).completed);
 report.checks.push('Old completed save opens the new layout on practice, preserving coins; all destinations fit portrait and landscape HUD bounds');
 await drag('pip','pip_switch');await drag('pepper','storage');await drag('crate','5,2');await drag('pip','waiting');await drag('crate','1,3');await drag('pepper','pepper_switch');await drag('pip','inside_entry');await drag('pip','inside_latch');await drag('pepper','pepper_inside');
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);assert.equal((await state()).coins,160);await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();
 assert.equal((await state()).coins,160);report.checks.push('Nine-drag practice from a version-one save completes and reloads without a second reward');
 assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({passed:report.passed,checks:report.checks,failure:report.failure},null,2));await browser?.close();});
