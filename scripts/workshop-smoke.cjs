// Real-input acceptance for the irregular eight-panel workshop, in an isolated profile.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=`build/evidence/workshop/${backend}`;
const report={backend,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
 const context=await browser.newContext({viewport:{width:1705,height:964},hasTouch:true});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const settle=()=>page.waitForTimeout(220),state=()=>page.evaluate(()=>pawsState);
 const saved=()=>page.waitForFunction(()=>pawsState.save===2);
 const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();};
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const tap=async(id,touch=false)=>{const p=(await points()).objects[id];assert(p,id);if(touch)await page.touchscreen.tap(p.x,p.y);else await page.mouse.click(p.x,p.y);await settle();};
 const drag=async(id,target)=>{const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b,target);await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:14});await settle();const preview=await points();assert.equal(preview.guides.length,1);assert.equal(preview.rotationGuides.length,0);assert(preview.workshopPreview.visible);assert.equal(preview.workshopPreview.target,target);if(id==='panel_2')await page.screenshot({path:out+'/drag-preview.png'});await page.mouse.up();await settle();};
 const rotate=async id=>{const before=(await state()).objects.find(o=>o.id===id).orientation;await tap(id);await key('Rotate');assert.equal((await state()).objects.find(o=>o.id===id).orientation,(before+1)%4);};
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8080/?devBeach=1');await ready();await key('Play');await saved();
 const fixture=await fs.readFile('build/evidence/beach-outings-start.save.workshop','utf8');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});await s.put('current',new TextEncoder().encode(text));s.dispose();},fixture);
 await page.reload();await ready();report.build=await page.evaluate(()=>pawsBuild);assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 await key('Adventures');await key('Start / Continue beach');await key('Help Mabel and Button');await page.waitForFunction(()=>pawsState.quest==='beach_windbreak'&&pawsUi.screen===2);
 assert.equal((await state()).objects.filter(o=>o.role==='shade_panel').length|| (await state()).objects.filter(o=>/^panel_/.test(o.id)).length,8);
 const initial=(await state()).objects,solution=(await state()).solution;
 // One real double-click rotates exactly once; separated clicks only select.
 const beforeDouble=(await state()).objects.find(o=>o.id==='panel_0').orientation;
 let dp=(await points()).objects.panel_0;await page.mouse.dblclick(dp.x,dp.y,{delay:70});await settle();
 assert.equal((await state()).objects.find(o=>o.id==='panel_0').orientation,(beforeDouble+1)%4);await page.screenshot({path:out+'/double-click-rotated.png'});
 assert.equal((await points()).rotationGuides.length,0);assert((await points()).guides.length>0);
 await key('Undo');await page.waitForTimeout(500);
 await tap('panel_0');await page.waitForTimeout(500);await tap('panel_0');
 assert.equal((await state()).objects.find(o=>o.id==='panel_0').orientation,beforeDouble);
 await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await settle();
 report.checks.push('Double-click rotates once; separated single clicks only select, and Undo restores orientation.');

 const framed=async label=>{
  const pts=await points(),board=await page.evaluate(()=>pawsUi.board);
  const corners=Object.entries(pts.objects).filter(([id])=>/^panel_\d+\.corner_/.test(id));assert.equal(corners.length,64);
  for(const [id,p] of corners)assert(p.x>=board.x&&p.x<=board.x+board.width&&p.y>=board.y&&p.y<=board.y+board.height,`${label}: ${id} outside free HUD area: ${JSON.stringify({p,board})}`);
  report.checks.push(`${label}: all eight complete panel meshes fit inside the free HUD area.`);
 };
 assert.equal((await points()).guides.length,0);assert.equal((await points()).rotationGuides.length,0);assert.equal((await points()).uncoveredCells,16);assert(!(await page.getByRole('button',{name:'Rotate',exact:true}).isEnabled()));assert((await page.locator('body').innerText()).includes('0 of 16 cells shaded'));
 await framed('1705x964 initial');
 await page.screenshot({path:out+'/initial.png'});
 for(const viewport of [{width:2071,height:964},{width:844,height:390},{width:390,height:844}]){
  await page.setViewportSize(viewport);await settle();await framed(`${viewport.width}x${viewport.height}`);
  await page.screenshot({path:out+`/framing-${viewport.width}x${viewport.height}.png`});
 }

 // Real touch selection and rotation, plus a plausible alternative fit.
 await page.setViewportSize({width:390,height:844});await settle();
 await tap('panel_2',true);assert.equal((await points()).selectedPlank,'panel_2');
 assert((await points()).guides.length>0);assert.equal((await points()).rotationGuides.length,0);
 await page.screenshot({path:out+'/selected-portrait.png'});
 await page.waitForTimeout(500);await tap('panel_2',true);assert.equal((await state()).objects.find(o=>o.id==='panel_2').orientation,0);await key('Rotate');assert.equal((await state()).objects.find(o=>o.id==='panel_2').orientation,1);await framed('Portrait rotated panel');
 await drag('panel_2','alternative_0');assert.equal((await state()).objects.find(o=>o.id==='panel_2').x,2);
 assert(!(await state()).completed);assert.equal((await points()).guides.length,0);assert.equal((await points()).uncoveredCells,14);assert((await page.locator('body').innerText()).includes('2 of 16 cells shaded'));await key('Undo');assert.equal((await points()).uncoveredCells,16);
 await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
 assert.deepEqual((await state()).objects,initial);
 report.checks.push('Quiet initial board, selection-only white fits, explicit Rotate, one drag preview, and coverage count pass; Undo and Restart restore uncovered cells.');
 await page.setViewportSize({width:1600,height:1000});await settle();
 await tap('panel_0');const beforeCancel=(await state()).objects;
 const cancelPoints=await points(),ca=cancelPoints.objects.panel_0;
 // Move away from every support, then cancel the active scene pointer.
 await page.mouse.move(ca.x,ca.y);await page.mouse.down();await page.mouse.move(ca.x,ca.y+55,{steps:8});await settle();
 assert.equal((await points()).guides.length,0);assert(!(await points()).workshopPreview.visible);
 await page.locator('#viewport').dispatchEvent('pointercancel',{pointerId:1,pointerType:'mouse',isPrimary:true});await page.mouse.up();await settle();
 assert.deepEqual((await state()).objects,beforeCancel);assert(!(await points()).workshopPreview.visible);
 report.checks.push('An invalid drag and pointer cancellation leave the puzzle unchanged and remove the preview.');

 let reloaded=false;
 for(const step of solution){
  if(step.verb===2)await rotate(step.object);
  if(step.verb===5){await drag(step.object,step.target);assert((await state()).revision>0);const target=(await points()).slots[step.target];assert(target,'Target remains addressable');}
  if(!reloaded&&step.verb===5&&step.object==='panel_3'){
   const midway=(await state()).objects;await saved();await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);
   assert.deepEqual((await state()).objects,midway);await framed('Mid-puzzle reload');reloaded=true;
  }
 }
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:30000});await saved();
 assert.equal((await state()).beachProgress,511);assert.equal((await state()).coins,0);
 await page.screenshot({path:out+'/completed.png'});
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 await key('Adventures');await key('A sheltered shell workshop · Revisit');await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);
 assert.equal((await state()).beachProgress,511);assert(reloaded);assert.deepEqual(report.errors,[]);
 await key('Replay workshop');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.completed);
 assert.equal((await state()).beachProgress,511);assert.equal((await state()).objects.filter(o=>/^panel_/.test(o.id)).length,8);
 await framed('Revisited workshop replay');
 report.checks.push('Completion offers replay through the existing restart confirmation without losing earned progress.');
 report.checks.push('All eight panels placed with real scene gestures; mid-puzzle and completion reload preserve state and rewards.');report.ok=true;
}catch(error){report.ok=false;report.failure=error.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,loadError:window.pawsLoadError,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:out+'/failure.png'}).catch(()=>{});process.exitCode=1;}
finally{await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify({ok:report.ok,checks:report.checks,failure:report.failure},null,2));await browser?.close();}})();
