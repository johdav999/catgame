// Runs the actual generated game. Quest commands use mouse events only.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/movement-guidance';
const report={checks:[],quests:[],errors:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 let page,validateGuides=false;
 try{
 page=await browser.newPage({viewport:{width:Number(process.env.VIEWPORT_WIDTH||1600),height:Number(process.env.VIEWPORT_HEIGHT||1000)}});page.setDefaultTimeout(60000);page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 if(process.env.RENDER_BACKEND==='webgl2')await page.addInitScript(()=>{if(navigator.gpu)navigator.gpu.requestAdapter=async()=>null;});
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(300);
 const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 async function key(name){
  for(let n=0;n<30;++n){
   const result=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);const widgets=EngineGui.snapshot().widgets,w=e&&widgets.find(w=>w.id===+e.dataset.widgetId);return {w,scroll:widgets.find(p=>p.maxScrollOffset>0&&w&&w.bounds.x>=p.bounds.x&&w.bounds.x<p.bounds.x+p.bounds.width)};},name);
   const {w,scroll}=result;
   assert(w,'Mouse-accessible control '+name);const b=w.bounds,c=w.clip,x=b.x+b.width/2,y=b.y+b.height/2;
   if(x>=c.x&&x<=c.x+c.width&&y>=c.y&&y<=c.y+c.height){await page.mouse.click(x,y);await settle();return;}
   const area=scroll?.bounds||c;await page.mouse.move(area.x+area.width/2,area.y+area.height/2);await page.mouse.wheel(0,y<c.y?-180:180);await settle();
  }
  throw Error('Could not scroll to '+name);
 }
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function click(id){const p=(await points()).objects[id];assert(p,'Clickable '+id);await page.mouse.click(p.x,p.y);await settle();assert.notEqual(await page.evaluate(()=>pawsUi.screen),7,'Click opened object action menu');}
 async function drag(id,target,cancel=false){
  const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to,JSON.stringify({id,target,p}));
  const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];
  // Sliding props keep the original grab offset, matching the visible preview.
  const b=/^\d+,\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;
  await page.mouse.move(a.x,a.y);await page.mouse.down();
  if(validateGuides){
   const guides=(await points()).guides;assert(guides.length,'No destination rectangles for '+id);
   if(id==='crate_b'||id==='crate_c')await page.screenshot({path:path.join(out,`guidance-${id}-${(await state()).revision}.png`)});
   report.checks.push('Visible legal destinations for '+id);
  }
  await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();assert.notEqual(await page.evaluate(()=>pawsUi.screen),7,'Drag opened action menu');
 }
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8146/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await page.waitForTimeout(1200);
 report.renderer=await page.evaluate(()=>pawsRenderState);
 if(process.env.RENDER_BACKEND)assert.equal(report.renderer.backend,process.env.RENDER_BACKEND);
 const quests=[["Pip's missing ball",'Pip'],['Tidy the toy basket','Pip'],['The stuck little gate','Pip'],['Mouse under the bench','Button'],['The sunny spot','Mabel'],['A bed for Button','Button'],['A drink after playing','Mabel'],['The garden bells','Pepper'],['Two kittens, one doorway','Pip'],['The garden picnic','Mabel']];
 const delta=[[0,-1],[1,0],[0,1],[-1,0]];
 for(let q=0;q<Number(process.env.QUEST_COUNT||10);q++){
  validateGuides=false;
  await key('Menu');await key('Requests & friends');let opts=await names();while(opts.some(n=>n.startsWith('Welcome '))){await key(opts.find(n=>n.startsWith('Welcome ')));opts=await names();}
  await key(quests[q][0]+' · Available');await page.waitForFunction(()=>[...document.querySelectorAll('button[data-widget-id]')].some(e=>e.getAttribute('aria-label')?.startsWith('Help ')));await key((await names()).find(n=>n.startsWith('Help ')));await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);await settle();
  await key('Menu');assert(!(await names()).some(n=>/Puzzle controls|Objects & actions|Choose an object/.test(n)));await key('Continue');
  const initial=await state(),solution=initial.solution,events=[];
  await page.screenshot({path:path.join(out,`quest-${q}-start.png`)});
  if(q===3){
   const before=await state(),p=await points();assert.equal(p.guides.length,1,'Only the rear crate has a free initial destination');
   await page.mouse.move(p.objects.crate_a.x,p.objects.crate_a.y);await page.mouse.down();assert.equal((await points()).guides.length,0,'Blocked front crate must have no destinations');await page.mouse.up();
   await page.mouse.move(p.objects.crate_b.x,p.objects.crate_b.y);await page.mouse.down();assert.equal((await points()).guides.length,1,'Holding rear crate isolates its destination');await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();
   assert.equal((await state()).revision,before.revision,'Guidance and cancellation must not mutate the quest');
   await key('Menu');assert.equal((await points()).guides.length,0,'Menu hides guidance');await key('Continue');assert.equal((await points()).guides.length,1,'Resume restores guidance');
   report.checks.push('Button: blocked crate, available rear crate, cancellation and pause guidance');
  }
  async function fresh(){await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);await settle();}
  if(q===1){
   await drag('cushion','1,0');const before=await state();await drag('ball','ball_recess');assert.deepEqual((await state()).objects,before.objects);assert.match((await state()).message,/pickup position/);
   await drag('fish','fish_recess');await drag('cushion','1,2');await drag('ball','ball_recess');const packed=await state();await drag('mouse','fish_recess');assert.deepEqual((await state()).objects,packed.objects);assert.match((await state()).message,/occupied/);
   await drag('ball','ball_home');assert.equal((await state()).objects.find(o=>o.id==='ball').x,2);report.checks.push('Basket requires different cushion parking, rejects an occupied recess, and lets a packed toy return home through the open side');await fresh();
  }
  if(q===4){
   await click('approach_plank');const before=await state();await drag('approach_plank','approach_socket');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/planter/);
   await click('long_plank');await drag('long_plank','direct_first');await click('step_plank');const direct=await state();await drag('step_plank','direct_second');assert.equal((await state()).revision,direct.revision);assert.match((await state()).message,/length/);assert(!(await state()).completed);
   report.checks.push('Planter blocks landing and direct bridge route cannot be completed with the available plank lengths');await fresh();
  }
  if(q===3){const before=await state();await click('mouse');assert.equal((await state()).revision,before.revision,'Blocked mouse retrieval changed state');assert.match((await state()).message,/crates/);report.checks.push('Bench retrieval rejects a blocked lane');}
  if(q===3){await key('Menu');await key('Hint');await key('Next hint');assert((await state()).paused);await key('Back');await key('Continue');report.checks.push('Hints remain available from Menu without resuming the scene');}
  if(q===4){const before=await state();await drag('long_plank','long_socket',true);assert.equal((await state()).revision,before.revision);await drag('long_plank','long_socket');assert.equal((await state()).revision,before.revision);report.checks.push('Plank drag cancellation and misalignment preserve the attempt');}
  if(q===5){const before=await state();await drag('square','square@4,2');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/outside/);report.checks.push('Cushion outside the basket is rejected without a partial pickup');}
  if(q===7){await page.waitForFunction(()=>!window.pawsSequence?.watching);await click('paw');assert(!(await state()).completed);report.checks.push('Wrong bell clears the sequence for retry');}
  if(q===8){const before=await state();await drag('pip','pepper_switch');assert.equal((await state()).revision,before.revision);report.checks.push('Wrong kitten switch is rejected');}
  for(let n=0;n<solution.length;n++){
   validateGuides=true;
   const step=solution[n];if((await state()).completed)break;
   if(step.verb===4)continue; // Pick-up is part of the drag transaction.
   const before=await state();
   if(step.verb===1){const o=before.objects.find(o=>o.id===step.object),d=delta[step.direction];await drag(step.object,`${o.x+d[0]},${o.y+d[1]}`);const moved=(await state()).objects.find(o=>o.id===step.object);assert.equal(moved.x,o.x+d[0]);assert.equal(moved.y,o.y+d[1]);events.push('drag '+step.object);}
   else if(step.verb===2){await click(step.object);events.push('rotate '+step.object);}
   else if(step.verb===3){await click(q===6?'rain_barrel':'source_ball');events.push('release source');if(q<3)await page.waitForFunction(()=>pawsState.completed||!pawsBall.rolling);}
   else if(step.verb===5){await drag(step.object,q===5?`${step.object}@${step.x},${step.y}`:step.target);events.push('fit '+step.object);}
   else if(step.verb===7){await click(step.object);events.push('retrieve '+step.object);}
   else if(step.verb===8){await click(step.object);events.push('tap '+step.object);}
   else if(step.verb===9){await drag(step.object,step.target);events.push('assign '+step.object);}
   else if(step.verb===6)assert((await state()).completed,'Arrangement did not automatically complete');
   else throw Error('Unmapped mouse action '+JSON.stringify(step));
   const after=await state();assert(after.revision>before.revision||after.completed,JSON.stringify({q,step,message:after.message}));
   if(q===0&&n===4){await click('channel_a');await click('channel_a');const before=await state();await click('source_ball');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/catch tray/);await key('Menu');await key('Undo');await key('Undo');await key('Continue');report.checks.push('Wrong ball branch ends at catch tray without changing the attempt');}
   if(q===2&&step.verb===3){assert(!(await state()).completed,'Gate opening skipped the second stage');await click('mouse');assert(!(await state()).completed);assert.match((await state()).message,/crate/);report.checks.push('Ball opens gate without finishing; crate path remains blocked');}
   if(q===3&&n===1){const before=await state();await click('mouse');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/crates/);report.checks.push('Clearing only the first crate does not retrieve the mouse');}
   if(q===0&&n===0){await key('Menu');await key('Undo');assert((await state()).paused);assert.equal((await state()).objects.find(o=>o.id==='pot_a').y,2);await key('Continue');await drag('pot_a','1,1');report.checks.push('Menu Undo restores a drag while retaining pause');}
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);await page.screenshot({path:path.join(out,`quest-${q}-complete.png`)});report.quests.push({quest:initial.quest,events,completed:true});console.log('Passed '+initial.quest);
  await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await settle();
 }
 assert.equal((await state()).coins,20*Number(process.env.QUEST_COUNT||10));assert.deepEqual(report.errors,[]);report.build=await page.evaluate(()=>pawsBuild);report.viewport=page.viewportSize();
  await page.reload();await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady);await key('Continue');await page.waitForFunction(()=>pawsState.garden);assert.equal((await state()).coins,20*Number(process.env.QUEST_COUNT||10));report.checks.push('Reload preserves completed quests and the once-only reward ledger');report.passed=true;
 }catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({text:document.body.innerText,state:window.pawsState,render:window.pawsRenderState,screen:window.pawsUi?.screen,gui:window.EngineGui?.snapshot(),ending:window.pawsEnding}));await page?.screenshot({path:path.join(out,'failure.png')});process.exitCode=1;}
 finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({passed:report.passed,checks:report.checks,quests:report.quests.length,failure:report.failure},null,2));}
})();
