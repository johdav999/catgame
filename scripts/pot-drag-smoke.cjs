// Real pointer regression: keep the grabbed part of the pot under the pointer.
// A centre-to-ground-slot drag masks the legacy drop/preview mismatch.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const url=process.env.PAWS_URL||'http://localhost:8080/';
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/pot-drag/browser');
const report={url,checks:[],moves:[],errors:[],screenshots:[]};let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:Number(process.env.VIEWPORT_WIDTH||2187),height:Number(process.env.VIEWPORT_HEIGHT||964)}});page.setDefaultTimeout(60000);
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(300);
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 async function restart(){await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);await settle();}
 async function at(id,x,y){const o=(await state()).objects.find(o=>o.id===id);assert.deepEqual([o.x,o.y],[x,y],JSON.stringify({expected:[x,y],actual:o,message:(await state()).message}));}
 async function drag(id,target,offset={x:0,y:0},cancel=false){
  const p=await points(),s=await state(),o=s.objects.find(o=>o.id===id),a=p.objects[id],from=p.slots[`${o.x},${o.y}`],to=p.slots[target];
  assert(a&&from&&to,JSON.stringify({id,target,p}));const start={x:a.x+offset.x,y:a.y+offset.y};
  const end={x:start.x+to.x-from.x,y:start.y+to.y-from.y};
  await page.mouse.move(start.x,start.y);await page.mouse.down();assert.equal(await page.evaluate(()=>pawsHostState().owner),'object','Pot did not capture the pointer');
  await page.mouse.move(end.x,end.y,{steps:16});await settle();
  const preview=(await points()).objects[id];
  if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));
  await page.mouse.up();await settle();const after=await state();
  report.moves.push({id,target,offset,start,end,preview,before:o,after:after.objects.find(o=>o.id===id),message:after.message,cancel});
 }
 await page.goto(url);await ready();report.build=await page.evaluate(()=>pawsBuild);await key('Play');await page.waitForFunction(()=>pawsState.garden);await restart();await shot('start');
 for(const [name,offset] of [['body',{x:0,y:0}],['rim',{x:10,y:-16}],['side',{x:-16,y:8}]]){
  if(name!=='body')await restart();
  await drag('pot_a','1,1',offset);await shot(name+'-north');await at('pot_a',1,1);
  await drag('pot_a','1,0',offset);await at('pot_a',1,0);
  report.checks.push(name+' grab moves exactly one authored slot on each north drag');
 }
 if(process.env.PROBE_ONLY){report.passed=true;return;}
 await restart();const initial=(await state()).objects;
 await drag('pot_a','1,1',{x:10,y:-16},true);assert.deepEqual((await state()).objects,initial);report.checks.push('Blur cancellation restores the visible pot and leaves all cells unchanged');
 await drag('pot_a','1,1',{x:-16,y:8});await at('pot_a',1,1);
 const blocked=(await state()).objects;await drag('pot_b','1,1');assert.deepEqual((await state()).objects,blocked);report.checks.push('Occupied shared slot rejects the complete drag without moving either pot');
 await key('Menu');await key('Undo');assert((await state()).paused);await at('pot_a',1,2);await key('Continue');
 await drag('pot_a','1,0',{x:10,y:-16});await at('pot_a',1,0);report.checks.push('Paused Undo restores the pot; a two-slot drag follows the whole legal track');
 await drag('pot_b','1,1',{x:-12,y:6});await at('pot_b',1,1);
 await drag('pot_a','0,0',{x:10,y:-16});await at('pot_a',0,0);
 await drag('pot_b','1,0',{x:-12,y:6});await at('pot_b',1,0);await shot('pots-parked');
 await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);await at('pot_a',0,0);await at('pot_b',1,0);report.checks.push('Reload preserves both correctly parked pots');
 const solution=(await state()).solution;
 for(const step of solution){if(step.verb===2){const p=(await points()).objects[step.object];await page.mouse.click(p.x,p.y);await settle();}else if(step.verb===3){const p=(await points()).objects.source_ball;await page.mouse.click(p.x,p.y);}}
 await page.waitForFunction(()=>pawsState.completed,null,{timeout:60000});assert.equal((await state()).coins,20);await shot('complete');report.checks.push('Parked pots leave a playable channel route; real clicks complete Pip\'s ball and award 20 coins');
 assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,host:window.pawsHostState?.()})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({passed:report.passed,checks:report.checks,moves:report.moves.length,failure:report.failure},null,2));await browser?.close();});
