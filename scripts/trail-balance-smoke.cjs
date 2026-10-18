// Isolated saved attempts; all puzzle commits use real mouse/keyboard events.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/trail-balance/${backend}`;
const report={backend,checks:[],quests:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 for(const kind of ['scent','balance']){
  const context=await browser.newContext({viewport:{width:1600,height:1000}});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(250);
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
  async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
  async function click(id){const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();}
  async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b,JSON.stringify({id,target,p}));await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8168/');await ready();report.build=await page.evaluate(()=>pawsBuild);
  const fixture=await fs.readFile(`build/evidence/trail-balance/fixtures/${kind}.save`,'utf8');
  await page.evaluate(async text=>{const store=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await store.put('current',new TextEncoder().encode(text));}finally{store.dispose();}},fixture);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});await settle();
  const initial=await state();assert.equal(initial.coins,180);const solution=initial.solution;
  await page.screenshot({path:path.join(out,kind+'-start.png')});
  if(kind==='scent'){
   const before=await state();await click('trail_0');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/three clues/);
   for(let n=0;n<3;n++)await click('clue_'+n);
   const inspected=await state();await click('trail_1');assert.equal((await state()).revision,inspected.revision);assert.match((await state()).message,/does not fit/);
   await key('Menu');await key('Undo');await key('Continue');
   assert((await state()).objects.find(o=>o.id==='clue_2').active===0);
   await key('Inspect scent');
   report.checks.push('Inspection gate, wrong trail and Undo preserve clue state');
  }else{
   await key('Test balance');assert.match((await state()).message,/four numbered/);
   const before=await state();await drag('weight_1','beam_1',true);assert.deepEqual((await state()).objects,before.objects);
   await drag('weight_1','beam_1');const placed=await state();await drag('weight_2','beam_1');assert.deepEqual((await state()).objects,placed.objects);
   await key('Menu');await key('Undo');await key('Continue');
   report.checks.push('Incomplete balance, cancelled drag and occupied slot rejected; Undo restores cushion');
  }
  await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);report.checks.push(kind+' restart resets the attempt');
  await page.setViewportSize({width:390,height:844});await settle();
  const portraitPoints=await points(),board=await page.evaluate(()=>pawsUi.board);
  for(const id of (kind==='scent'?['clue_0','clue_1','clue_2','trail_0','trail_1','trail_2']:['weight_1','weight_2','weight_3','weight_4'])){const p=portraitPoints.objects[id];assert(p.x>=board.x&&p.x<=board.x+board.width&&p.y>=board.y&&p.y<=board.y+board.height,JSON.stringify({id,p,board}));}
  if(kind==='balance'){await drag('weight_1','beam_1');assert.equal((await state()).objects.find(o=>o.id==='weight_1').y,2);await key('Menu');await key('Undo');await key('Continue');}
  await page.screenshot({path:path.join(out,kind+'-portrait-active.png')});await page.setViewportSize({width:1600,height:1000});await settle();
  let midway=false;
  for(let n=0;n<solution.length;n++){
   const s=solution[n];console.log(kind,n,s.object,s.target);
   if(s.verb===19)await click(s.object); // Inspect
   else if(s.verb===20)await click(s.object);
   else if(s.verb===21)await key('Test balance');
   else if(s.verb===5)await drag(s.object,s.target);
   else throw Error('Unmapped action '+JSON.stringify(s));
   if(n===Math.floor(solution.length/2)&&!midway){
    const before=await state();await page.waitForTimeout(1400);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);assert.deepEqual((await state()).objects,before.objects);midway=true;report.checks.push(kind+' midway reload restores every piece and clue');
   }
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:15000});
  assert.equal((await state()).coins,200);await page.screenshot({path:path.join(out,kind+'-complete.png')});
  await page.waitForTimeout(1400);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.completed);assert.equal((await state()).coins,200);
  report.quests.push({quest:initial.quest,completed:true,commands:solution.length,coins:200});
  if(kind==='balance'){await page.setViewportSize({width:390,height:844});await settle();await page.screenshot({path:path.join(out,'balance-portrait.png')});}
  await context.close();
 }
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}})();
