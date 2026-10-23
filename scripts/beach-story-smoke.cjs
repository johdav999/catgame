// Complete beach adventure using retained GUI and real scene input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const backend=process.env.QUEST_BACKEND||'webgpu',base=process.env.PAWS_URL||'http://127.0.0.1:8194/';
const legacyUpgrade=process.env.BEACH_LEGACY_UPGRADE==='1';
const reducedMotion=process.env.BEACH_REDUCED_MOTION==='1';
const out=process.env.EVIDENCE_DIR||`build/evidence/beach-story/${backend}`;
const report={backend,url:base,reducedMotion,quests:[],checks:[],errors:[],failedRequests:[]};let browser,page;
const ids=['beach_pip_ball','beach_button_bridge','beach_mabel_shade','beach_pepper_basket','beach_shell_chimes','beach_picnic'];
const titles=["Pip's runaway beach ball","A little bridge for Button","Mabel's shady corner","Pepper's missing picnic basket","The shell-chime melody","Everyone has a place"];
const speakers=['Pip','Button','Mabel','Pepper','Pepper','All four cats'];
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000},reducedMotion:reducedMotion?'reduce':'no-preference'});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('requestfailed',r=>report.failedRequests.push({url:r.url(),error:r.failure()?.errorText}));page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const coastal=[];page.on('response',r=>{
  if(/\/assets\/models\/beach\/[^/]+\.glb$/.test(new URL(r.url()).pathname))
   coastal.push(r.body().then(bytes=>({file:path.basename(new URL(r.url()).pathname),status:r.status(),bytes:bytes.length,sha256:crypto.createHash('sha256').update(bytes).digest('hex')})));
 });
 const coastalManifest=JSON.parse(await fs.readFile('assets/runtime/models/beach/manifest.json','utf8'));
 const propPass=new Set(coastalManifest.assets.filter(a=>a.source?.path?.includes('/beach-complete-20261020/')).map(a=>a.name));
 report.coastalScenes=[];
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(190);
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const saved=()=>page.waitForFunction(()=>pawsState.save===2,null,{timeout:15000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function rotate(id){const before=(await state()).objects.find(o=>o.id===id).orientation;await click(id);if((await state()).objects.find(o=>o.id===id).orientation===before&&(await points()).selectedPlank===id)await click(id);}
 async function click(id){const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();}
 async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b,JSON.stringify({id,target,p}));await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 async function garden(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});}
 async function active(stage){await page.waitForFunction(id=>pawsState.quest===id&&pawsUi.screen===3,ids[stage],{timeout:90000});await key('Help '+speakers[stage]);await page.waitForFunction(()=>pawsUi.screen===2);if(stage===4)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:20000});}
 async function start(stage){await saved();await settle();await key('Adventures');await key('Start / Continue beach');await active(stage);}
 async function home(){await key('Menu');await key('Back to garden');await garden();}
 async function reload(stage){await saved();const before=(await state()).objects;await page.reload();await ready();await key('Continue');await page.waitForFunction(id=>pawsState.quest===id&&!pawsState.garden&&pawsUi.screen===2,ids[stage],{timeout:90000});if(stage===4)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:20000});assert.deepEqual((await state()).objects,before);}
 async function doStep(s){
  if(s.verb===4||s.verb===6)return; // Drag owns pickup; valid arrangements auto-check.
  const before=await state();
  if(s.verb===1){const o=before.objects.find(o=>o.id===s.object);const dx=[0,1,0,-1][s.direction],dy=[-1,0,1,0][s.direction];await drag(s.object,`${o.x+dx},${o.y+dy}`);const after=(await state()).objects.find(o=>o.id===s.object);assert.equal(after.x,o.x+dx);assert.equal(after.y,o.y+dy);}
  else if(s.verb===5){await drag(s.object,s.target);assert((await state()).revision>before.revision,'Fit did not commit: '+s.object+' to '+s.target);}
  else if(s.verb===2){await rotate(s.object);assert.equal((await state()).objects.find(o=>o.id===s.object).orientation,(before.objects.find(o=>o.id===s.object).orientation+1)%4);}
  else if([3,8,19,20].includes(s.verb)){await click(s.verb===3?'source_ball':s.object);assert((await state()).revision>before.revision,'Click did not commit: '+JSON.stringify(s));if(s.verb===20&&before.quest==='beach_pepper_basket')await page.waitForFunction(()=>!window.pawsPepper||!pawsPepper.active||pawsPepper.reaction<3,null,{timeout:90000});}
  else throw Error('Unhandled real-input step '+JSON.stringify(s));
 }
 const url=new URL(base);url.searchParams.set('devBeach','1');
 await page.goto(url.href);await ready();report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);
 assert.equal(report.render.backend,backend,'Must use the requested renderer');
 await key('Play');await garden();
 if(legacyUpgrade){
  await saved();
  const count=await page.evaluate(async()=>{
   const storage=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});
   try{const bytes=await storage.get('current');let count=0;
    const text=new TextDecoder().decode(bytes).replace(/beach-([a-z-]+)-v2/g,(_,id)=>{++count;return `beach-${id}-v1`;});
    await storage.put('current',new TextEncoder().encode(text));return count;
   }finally{storage.dispose();}
  });
  assert.equal(count,6,'Fixture contains all six original beach layouts');
  await page.reload();await ready();await key('Continue');await garden();
 }
 await start(0);
 report.coastal=await Promise.all(coastal);
 for(const asset of coastalManifest.assets){
  const fetched=report.coastal.find(r=>r.file===path.basename(asset.url));
  assert(fetched&&fetched.status===200&&fetched.sha256===asset.sha256&&fetched.bytes===asset.bytes,'Reviewed coastal bytes: '+asset.name);
 }

 for(let stage=0;stage<6;++stage){
  console.log('Beach stage',stage,ids[stage]);assert.equal((await state()).quest,ids[stage]);
  if(legacyUpgrade){
   const oldLength=(await state()).solution.length;
   assert.equal(oldLength,[10,10,6,16,6,5][stage],'Original saved layout must remain intact before Restart');
   await key('Menu');await key('Restart puzzle');await key('Restart puzzle');
   await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
   assert((await state()).solution.length>oldLength,'Restart must activate the longer layout');
   assert.equal((await state()).sceneCount,1,'Upgrade replaces the original scene');
   if(stage===4)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:20000});
  }
  await page.setViewportSize({width:390,height:844});await settle();
  const solution=(await state()).solution,first=solution.find(s=>![4,6].includes(s.verb));
  const initial=(await state()).objects;
  if([0,1,2,5].includes(stage)){
   const target=first.verb===1?(stage===0?'1,0':'1,1'):solution.find(s=>s.verb===5).target;
   await drag(first.object,target,true);assert.deepEqual((await state()).objects,initial,'Cancelled drag changed board');
  }
  await doStep(first);await key('Menu');await key('Undo');await key('Continue');
  assert.deepEqual((await state()).objects,initial,'Undo failed to restore first move');
  await key('Menu');await key('Hint');await key('Next hint');await key('Back');await key('Continue');
  await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
  assert.deepEqual((await state()).objects,initial,'Restart failed to reset board');
  if(stage===4)await page.waitForFunction(()=>pawsSequence&&!pawsSequence.watching,null,{timeout:20000});
  await page.screenshot({path:path.join(out,`${stage}-portrait.png`)});
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
  if(stage===1&&process.env.BEACH_LAYOUT_VIEWS==='1'){
   // Inspect the extended pier and landward planting from real orbit/zoom input.
   await page.mouse.move(1100,250);
   for(let n=0;n<5;++n){await page.mouse.wheel(0,80);await settle();}
   await page.mouse.move(1100,250);await page.mouse.down();await page.mouse.move(960,230,{steps:20});await page.mouse.up();await settle();
   await page.screenshot({path:path.join(out,'1-coast-overview.png')});
   await page.mouse.move(1100,250);await page.mouse.down();await page.mouse.move(700,230,{steps:30});await page.mouse.up();await settle();
   await page.screenshot({path:path.join(out,'1-landward-overview.png')});
   await reload(stage); // The shipped pause menu has no Reset view control.
  }
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
  report.quests.push({id:ids[stage],completed:true,resumed,undo:true,restart:true,legacyUpgrade,steps:solution.length,coins:complete.coins});
  if(stage<5){await key('Continue beach story');await active(stage+1);}
 }
 await key('Watch beach ending');await page.waitForFunction(()=>pawsState.beachEndingSeen===1);await saved();
 await page.screenshot({path:path.join(out,'ending-landscape.png')});
 await page.setViewportSize({width:390,height:844});await settle();
 const framed=await points(),frame=await page.evaluate(()=>pawsUi.board);
 for(let n=0;n<4;n++)for(let c=0;c<8;c++){const p=framed.objects[`beach_cat_${n}_${c}`];assert(p,'Missing cat bounds');assert(p.x>=frame.x-2&&p.x<=frame.x+frame.width+2&&p.y>=frame.y-2&&p.y<=frame.y+frame.height+2,JSON.stringify({n,c,p,frame}));}
 await page.screenshot({path:path.join(out,'ending-portrait.png')});
 await key('Back to garden');await garden();await saved();await page.reload();await ready();await key('Continue');await garden();await saved();await settle();
 await key('Adventures');assert.match(await page.locator('body').innerText(),/Postcard saved/);await key('Everyone has a place · Revisit');
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});await key('Replay beach ending');await saved();
 assert.equal((await state()).coins,0);assert.equal((await state()).picnicSeen,0);assert.equal((await state()).beachProgress,63);
 report.checks.push('All six quests completed with real scene gestures and retained controls','Every quest supports undo, restart, reload and garden travel','Sequential story continuation and persistent cove milestones','Four-cat picnic ending saved and replayed after reload without base rewards');
 assert(report.quests.every(q=>q.resumed));assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,sequence:window.pawsSequence,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({backend,passed:report.passed,quests:report.quests,checks:report.checks,errors:report.errors,failure:report.failure},null,2));}})();
