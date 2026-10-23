// Complete the five additional beach outings using retained GUI and real scene input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const backend=process.env.QUEST_BACKEND||'webgpu',base=process.env.PAWS_URL||'http://127.0.0.1:8194/';
const reducedMotion=process.env.BEACH_REDUCED_MOTION==='1';
const tidepoolOnly=process.env.TIDEPOOL_ONLY==='1';let dismissIntroduction=tidepoolOnly;
const out=process.env.EVIDENCE_DIR||`build/evidence/beach-outings/${backend}`;
const report={backend,url:base,reducedMotion,quests:[],checks:[],errors:[],failedRequests:[]};let browser,page;
const ids=['beach_pip_ball','beach_button_bridge','beach_mabel_shade','beach_pepper_basket','beach_shell_chimes','beach_picnic','beach_tidepool_crossing','beach_fresh_water','beach_windbreak','beach_shell_encore','beach_lantern_balance'];
const speakers=['Pip','Button','Mabel','Pepper','Pepper','All four cats','Button','Mabel','Mabel and Button','Pepper','Pip'];
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576','--media-cache-size=1048576']});
 const context=await browser.newContext({viewport:{width:1600,height:1000},hasTouch:true,reducedMotion:reducedMotion?'reduce':'no-preference'});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('requestfailed',r=>report.failedRequests.push({url:r.url(),error:r.failure()?.errorText}));page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const coastal=[];page.on('response',r=>{
  if(/\/assets\/models\/beach\/[^/]+\.glb$/.test(new URL(r.url()).pathname))
   { const capture=r.body().then(bytes=>({file:path.basename(new URL(r.url()).pathname),status:r.status(),bytes:bytes.length,sha256:crypto.createHash('sha256').update(bytes).digest('hex')})); capture.catch(()=>{}); coastal.push(capture); }
 });
 const coastalManifest=JSON.parse(await fs.readFile('assets/runtime/models/beach/manifest.json','utf8'));
 const propPass=new Set(coastalManifest.assets.filter(a=>a.source?.path?.includes('/beach-complete-20261020/')).map(a=>a.name));
 report.coastalScenes=[];
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(190);
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const saved=()=>page.waitForFunction(()=>pawsState.save===2,null,{timeout:15000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function click(id){const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();}
 async function rotate(id){const before=(await state()).objects.find(o=>o.id===id).orientation;await click(id);if((await state()).objects.find(o=>o.id===id).orientation===before&&(await points()).selectedPlank===id)await click(id);}
 async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b,JSON.stringify({id,target,p}));await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 async function garden(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});}
 async function active(stage){await page.waitForFunction(id=>pawsState.quest===id&&pawsUi.screen===3,ids[stage],{timeout:90000});if(dismissIntroduction){dismissIntroduction=false;await page.keyboard.press('Escape');}else await key('Help '+speakers[stage]);await page.waitForFunction(()=>pawsUi.screen===2);if(stage===4||stage===9)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:60000});}
 async function start(stage){await saved();await settle();await key('Adventures');await key('Start / Continue beach');await active(stage);}
 async function home(){await key('Menu');await key('Back to garden');await garden();}
 async function reload(stage){await saved();const before=(await state()).objects;await page.reload();await ready();await key('Continue');await page.waitForFunction(id=>pawsState.quest===id&&!pawsState.garden&&pawsUi.screen===2,ids[stage],{timeout:90000});if(stage===4||stage===9)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:60000});assert.deepEqual((await state()).objects,before);}
 async function doStep(s){
  if(s.verb===4||s.verb===6)return; // Drag owns pickup; valid arrangements auto-check.
  const before=await state();
  if(s.verb===1){const o=before.objects.find(o=>o.id===s.object);const dx=[0,1,0,-1][s.direction],dy=[-1,0,1,0][s.direction];await drag(s.object,`${o.x+dx},${o.y+dy}`);const after=(await state()).objects.find(o=>o.id===s.object);assert.equal(after.x,o.x+dx);assert.equal(after.y,o.y+dy);if(tidepoolOnly){const p=await points();assert.equal(p.selectedBasket,s.object,'Moving a basket retains selection');assert(p.guides.length>0,'Legal return moves stay highlighted');}}
  else if(s.verb===5){await drag(s.object,s.target);assert((await state()).revision>before.revision,'Fit did not commit: '+s.object+' to '+s.target);}
  else if(s.verb===2){await rotate(s.object);assert.equal((await state()).objects.find(o=>o.id===s.object).orientation,(before.objects.find(o=>o.id===s.object).orientation+1)%4);}
  else if(s.verb===21){await key('Test balance');assert((await state()).revision>before.revision,'Balance test must advance');}
  else if([3,8,19,20].includes(s.verb)){await click(s.verb===3?(before.objects.find(o=>o.id==='catch_tray'&&o.active)?.id||'rain_barrel'):s.object);assert((await state()).revision>before.revision,'Click did not commit: '+JSON.stringify(s));if(s.verb===20&&before.quest==='beach_pepper_basket')await page.waitForFunction(()=>!window.pawsPepper||!pawsPepper.active||pawsPepper.reaction<3,null,{timeout:90000});}
  else throw Error('Unhandled real-input step '+JSON.stringify(s));
 }
 const url=new URL(base);url.searchParams.set('devBeach','1');
 await page.goto(url.href);await ready();report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);
 assert.equal(report.render.backend,backend,'Must use the requested renderer');
 await key('Play');await garden();
 await saved();
 const fixture=await fs.readFile('build/evidence/beach-outings-start.save','utf8');
 assert(await page.evaluate(text=>{Module._paws_cloud_stamp();return pawsStageCloudSave(new TextEncoder().encode(text),pawsCloudStamp);},fixture),'Fixture import validates');
 await key('Use imported afternoon');await saved();await key('Continue');
 await garden();assert.equal((await state()).beachProgress,63,'Original six-quest progress restored');await start(6);
 report.coastal=await Promise.all(coastal);
 for(const asset of coastalManifest.assets){
  const fetched=report.coastal.find(r=>r.file===path.basename(asset.url));
  assert(fetched&&fetched.status===200&&fetched.sha256===asset.sha256&&fetched.bytes===asset.bytes,'Reviewed coastal bytes: '+asset.name);
 }

 for(let stage=6;stage<11;++stage){
  console.log('Beach stage',stage,ids[stage]);assert.equal((await state()).quest,ids[stage]);
  await page.setViewportSize({width:390,height:844});await settle();
  const solution=(await state()).solution,first=solution.find(s=>![4,6].includes(s.verb));
  const initial=(await state()).objects;
  if(tidepoolOnly&&stage===6){
   assert.match(await page.locator('body').innerText(),/Help Button reach the cushion/);
   await click('supply');let guidance=await points();
   assert.equal(guidance.selectedBasket,'supply');assert.equal(guidance.guides.length,2,'Tap retains both reachable spaces for the outer basket');
   assert.deepEqual((await state()).objects,initial,'Selection does not move a basket');
   await page.screenshot({path:path.join(out,'basket-selected-portrait.png')});
   await key('Menu');assert.equal((await points()).guides.length,0,'Menu hides basket destinations');await key('Continue');
   assert.equal((await points()).guides.length,2,'Resume restores selected basket destinations');
   await click('parking');guidance=await points();assert.equal(guidance.selectedBasket,'parking');assert.equal(guidance.guides.length,0,'Blocked basket has no legal destination highlights');
   assert.match(await page.locator('body').innerText(),/Middle basket cannot move yet.*Outer basket/);
   await page.screenshot({path:path.join(out,'basket-blocked-portrait.png')});
   const touchBasket=(await points()).objects.supply;await page.touchscreen.tap(touchBasket.x,touchBasket.y);await settle();
   assert.equal((await points()).selectedBasket,'supply');assert.equal((await points()).guides.length,2,'Touch selection retains legal moves after finger release');
   await click('west');guidance=await points();
   assert.equal(guidance.selectedPlank,'west');assert.equal(guidance.guides.length,0);assert.equal(guidance.rotationGuides.length,2,'Only unblocked matching gaps are marked for rotation');
   assert.deepEqual((await state()).objects,initial,'First plank tap selects without rotating or changing the save');
   assert.match(await page.locator('body').innerText(),/Needs a quarter-turn/);
   await page.screenshot({path:path.join(out,'plank-needs-rotation-portrait.png')});
   await key('Menu');assert.equal((await points()).rotationGuides.length,0);await key('Continue');assert.equal((await points()).rotationGuides.length,2,'Rotation guidance survives menu/resume');
   const touchPlank=(await points()).objects.west;await page.touchscreen.tap(touchPlank.x,touchPlank.y);await settle();
   guidance=await points();assert.equal(guidance.guides.length,2);assert.equal(guidance.rotationGuides.length,0,'Second tap rotates and exchanges amber gaps for white fits');
   await page.screenshot({path:path.join(out,'plank-fits-now-portrait.png')});
   const blocked=(await state()).objects;
   await drag('west','west_socket');
   assert.deepEqual((await state()).objects,blocked,'Blocked landing preserves pieces');
   assert.match(await page.locator('body').innerText(),/Pool-side basket blocks this landing/);
   await page.screenshot({path:path.join(out,'blocked-landing-portrait.png')});
   await key('Menu');await key('Undo');await key('Continue');
   assert.deepEqual((await state()).objects,initial,'Undo recovers the trial rotation');
   report.checks.push('Escape from introduction retains quest objective; blocked plank names the basket and preserves state');
   report.checks.push('Tap retains only selected basket destinations; blocked basket explains its named obstruction; menu/resume preserves selection');
   report.checks.push('First plank tap selects; touch tap rotates; authoritative white and broken amber fits exclude blocked/occupied/unsupported gaps');
  }
  if([6,7,8,10].includes(stage)){
   const o=initial.find(o=>o.id===first.object);const dx=[0,1,0,-1][first.direction],dy=[-1,0,1,0][first.direction];const target=first.verb===1?`${o.x+dx},${o.y+dy}`:solution.find(s=>s.verb===5).target;
   await drag(first.object,target,true);assert.deepEqual((await state()).objects,initial,'Cancelled drag changed board');
  }
  await doStep(first);await key('Menu');await key('Undo');await key('Continue');
  assert.deepEqual((await state()).objects,initial,'Undo failed to restore first move');
  await key('Menu');await key('Hint');await key('Next hint');await key('Back');await key('Continue');
  await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
  assert.deepEqual((await state()).objects,initial,'Restart failed to reset board');
  if(stage===4||stage===9)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:60000});
  await page.screenshot({path:path.join(out,`${stage}-portrait.png`)});
  if(process.env.BEACH_LAYOUT_ONLY==='1'){
   const account=await page.locator('#cloud-open').boundingBox();
   assert(account&&account.y+account.height<=756,'Account button clears the bottom quest controls');
   await page.mouse.click(300,795);await settle(); // Rendered Hint, previously covered by Account.
   await key('Next hint');await key('Back');
   report.checks.push('390x844 account launcher clears quest controls and rendered Hint accepts pointer input');
   report.passed=true;return;
  }
  await page.setViewportSize({width:844,height:390});await settle();
  await page.screenshot({path:path.join(out,`${stage}-short-landscape.png`)});
  await page.setViewportSize({width:1600,height:1000});await settle();
  const terrain=await page.evaluate(()=>{Module._paws_terrain_snapshot();return pawsTerrain;});
  assert(terrain.enabled&&terrain.chunks===64&&terrain.frustumCovered,JSON.stringify(terrain));
  report.coastalScenes.push({quest:ids[stage],assets:terrain.coastal});
  for(const asset of coastalManifest.assets)if(propPass.has(asset.name)&&(!asset.quests||asset.quests.includes(ids[stage]))){
   const used=terrain.coastal.find(c=>c.name===asset.name);
   assert(used&&used.instances>0,'Coastal model appears in its quest: '+asset.name+' / '+ids[stage]);
  }
  await page.screenshot({path:path.join(out,`${stage}-landscape.png`)});
  let resumed=false;
  for(let n=0;n<solution.length;++n){await doStep(solution[n]);
   if(!resumed&&n>=Math.floor(solution.length/2)&&!(await state()).completed){
    await reload(stage);const midway=(await state()).objects;
    await home();assert.equal((await state()).quest,'pip_missing_ball');await saved();await page.reload();await ready();await key('Continue');await garden();await start(stage);assert.deepEqual((await state()).objects,midway);
    resumed=true;
   }
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:25000});
  const complete=await state();assert.equal(complete.coins,0);assert.equal(complete.chapter,1);assert.equal(complete.beachProgress,(1<<(stage+1))-1);assert.equal(complete.sceneCount,1);
  await page.screenshot({path:path.join(out,`${stage}-complete.png`)});await saved();
  report.quests.push({id:ids[stage],completed:true,resumed,undo:true,restart:true,steps:solution.length,coins:complete.coins});
  if(tidepoolOnly){assert.deepEqual(report.errors,[]);report.passed=true;return;}
  if(stage<10){await key('Continue beach story');await active(stage+1);}
 }
 await key('Return to the beach picnic');await page.waitForFunction(()=>pawsUi.screen===9);
 await key('Replay beach ending');await page.waitForFunction(()=>pawsState.beachEndingSeen===1);await saved();
 await page.screenshot({path:path.join(out,'ending-landscape.png')});
 await page.setViewportSize({width:390,height:844});await settle();
 const framed=await points(),frame=await page.evaluate(()=>pawsUi.board);
 for(let n=0;n<4;n++)for(let c=0;c<8;c++){const p=framed.objects[`beach_cat_${n}_${c}`];assert(p,'Missing cat bounds');assert(p.x>=frame.x-2&&p.x<=frame.x+frame.width+2&&p.y>=frame.y-2&&p.y<=frame.y+frame.height+2,JSON.stringify({n,c,p,frame}));}
 await page.screenshot({path:path.join(out,'ending-portrait.png')});
 await key('Back to garden');await garden();await saved();await page.reload();await ready();await key('Continue');await garden();await saved();await settle();
 await key('Adventures');assert.match(await page.locator('body').innerText(),/Postcard saved/);await key('Visit beach picnic');
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});await key('Replay beach ending');await saved();
 assert.equal((await state()).coins,0);assert.equal((await state()).picnicSeen,0);assert.equal((await state()).beachProgress,2047);
 report.checks.push('All five new quests completed with real scene gestures and retained controls','Every new quest supports undo, restart, reload and garden travel','Sequential story continuation and persistent cove milestones','Four-cat picnic ending saved and replayed after reload without base rewards');
 const lantern=report.coastalScenes.find(s=>s.quest==='beach_lantern_balance')?.assets.find(a=>a.name==='beach_lantern');assert(lantern&&lantern.instances>=4,'Four imported interactive lantern weights');
 assert(report.quests.every(q=>q.resumed));assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,sequence:window.pawsSequence,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({backend,passed:report.passed,quests:report.quests,checks:report.checks,errors:report.errors,failure:report.failure},null,2));}})();
