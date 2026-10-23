// Generated-site acceptance: fixture only seeds the save; every move uses visible input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const backend=process.env.QUEST_BACKEND||'webgpu';
const base=process.env.PAWS_URL||'http://127.0.0.1:8198/?devBeach=1';
const access=process.env.PEPPER_ACCESS==='1';
const root=process.env.EVIDENCE_DIR||`build/evidence/pepper-world/${backend}${access?'-access':''}`;
const versions=process.env.PEPPER_VERSION?[process.env.PEPPER_VERSION]:['v1','v2'];
const report={backend,checks:[],errors:[],httpErrors:[],failedRequests:[],versions:[]};let browser,page;
function observePage(page){
 page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 page.on('response',r=>{if(r.status()>=400)report.httpErrors.push({url:r.url(),status:r.status()});});
 page.on('requestfailed',r=>report.failedRequests.push({url:r.url(),error:r.failure()?.errorText}));
}
(async()=>{try{
 await fs.mkdir(root,{recursive:true});
 for(const version of versions){
  // Keep each asset-heavy acceptance run in a fresh browser process.
  browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  const desktop=backend==='webgl2'?{width:1280,height:800}:{width:1600,height:1000};
  let context=await browser.newContext({viewport:desktop});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  page=await context.newPage();observePage(page);
  const landmarks=[];
  page.on('response',r=>{
   if(/\/assets\/models\/beach\/(shelter|branch)-v1\.glb$/.test(new URL(r.url()).pathname))
    landmarks.push(r.body().then(b=>({file:path.basename(new URL(r.url()).pathname),status:r.status(),bytes:b.length,sha256:crypto.createHash('sha256').update(b).digest('hex')})));
  });
  const state=()=>page.evaluate(()=>pawsState),pepper=()=>page.evaluate(()=>pawsPepper);
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const saved=()=>page.waitForFunction(()=>pawsState.save===2,null,{timeout:15000});
  const idle=()=>page.waitForFunction(()=>window.pawsPepper&&!pawsPepper.active,null,{timeout:90000});
  const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction.objects;});
  async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(150);}
  async function click(id){const p=(await points())[id];assert(p&&p.x>=0&&p.x<=page.viewportSize().width&&p.y>=0&&p.y<=page.viewportSize().height,JSON.stringify({id,p}));await page.mouse.click(p.x,p.y);await page.waitForTimeout(180);}
  async function shot(name){await page.screenshot({path:path.join(root,`${version}-${name}.png`)});}
  await page.goto(base);await ready();
  await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},await fs.readFile(`build/evidence/pepper-world/fixtures/pepper-${version}.save`,'utf8'));
  await page.reload();await ready();await key('Continue');
  await page.waitForFunction(()=>pawsState.quest==='beach_pepper_basket'&&pawsUi.screen===2&&window.pawsPepper,null,{timeout:90000});
  assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);report.build=await page.evaluate(()=>pawsBuild);
  const manifest=JSON.parse(await fs.readFile('assets/runtime/models/beach/manifest.json','utf8'));
  const fetched=await Promise.all(landmarks);
  for(const name of ['beach_shelter','beach_branch']){
   const asset=manifest.assets.find(a=>a.name===name),download=fetched.find(a=>a.file===path.basename(asset.url));
   assert(download&&download.status===200&&download.sha256===asset.sha256&&download.bytes===asset.bytes,'Reviewed landmark bytes loaded: '+name);
  }
  report.landmarks=fetched;
  if(access){
   await click('clue_0');await key('Menu');const frozen=(await pepper()).time;
   await page.waitForTimeout(400);assert.equal((await pepper()).time,frozen,'Menu freezes Pepper');
   await key('Settings');await key('Text size: Standard');await key('Motion: Full');await key('Back');
   await key('Undo');await key('Continue');await idle();
   report.checks.push('Pause freezes an active investigation; larger text and reduced motion retain world completion');
  }
  const initial=await state(),solution=initial.solution,stops=solution.length/4;
  assert.equal(stops,version==='v1'?4:6);
  assert.equal(await page.getByRole('button',{name:'Clues & paths',exact:true}).count(),1,'Notebook is directly available from the HUD');
  assert.match(await page.locator('body').innerText(),/Step 1: tap clues 1, 2 and 3/);
  await key('Clues & paths');
  await page.getByRole('button',{name:'Examine: Paw prints',exact:true}).waitFor();
  assert.match(await page.locator('body').innerText(),/Paw prints · Not examined/);
  assert(!(await page.locator('body').innerText()).includes('Fresh prints turn toward a wooden post.'),'Notebook hides unexamined evidence');
  await shot('notebook-unexamined');await key('Back');
  for(const viewport of [{width:390,height:844},{width:844,height:390},desktop]){
   const previous=await page.evaluate(()=>pawsUi.board.width);
   await page.setViewportSize(viewport);await page.waitForFunction(width=>pawsUi.board.width!==width,previous);await page.waitForTimeout(250);
   const p=await points();for(const id of ['clue_0','clue_1','clue_2','trail_0','trail_1','trail_2'])assert(p[id].x>=0&&p[id].x<=viewport.width&&p[id].y>=0&&p[id].y<=viewport.height,JSON.stringify({viewport,id,p:p[id]}));
   assert.equal(await page.getByRole('button',{name:/^Inspect |^Follow:/}).count(),0,'Bottom choice tray must be absent');
   const notebook=await page.getByRole('button',{name:'Clues & paths',exact:true}).boundingBox();
   const account=await page.getByRole('button',{name:'Account & cloud',exact:true}).boundingBox();
   assert(notebook&&notebook.width>=44&&notebook.height>=44,'Notebook keeps a touch-sized target');
   if(account)assert(notebook.x+notebook.width<=account.x||account.x+account.width<=notebook.x||notebook.y+notebook.height<=account.y||account.y+account.height<=notebook.y,'Notebook must not overlap Account & cloud');
   await shot(`start-${viewport.width}x${viewport.height}`);
  }
  // A premature destination remains explainable and leaves every clue available.
  await click('trail_0');assert.match((await state()).message,/Inspect all three/);assert.deepEqual((await state()).objects,initial.objects);
  await click('clue_0');await key('Menu');await key('Undo');await key('Continue');
  assert(!(await state()).objects.find(o=>o.id==='clue_0').active,'Undo cancels inspection');
  await key('Menu');await key('Clues & paths');
  await key('Examine: Paw prints');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
  assert((await state()).objects.find(o=>o.id==='clue_0').active,'Keyboard equivalent examines real clue');
  for(let stage=0;stage<stops;stage++){
   console.log(backend,version,'stop',stage+1,'/',stops);
   const startPoints=await points();
   await click('clue_0');await page.waitForFunction(()=>pawsPepper.arrived||!pawsPepper.active,null,{timeout:90000});await shot(`stop-${stage+1}-pepper-inspects`);
   await click('clue_1');await page.waitForFunction(()=>pawsPepper.arrived||!pawsPepper.active,null,{timeout:90000});
   await click('clue_2');await page.waitForFunction(()=>pawsPepper.arrived||!pawsPepper.active,null,{timeout:90000});
   assert((await state()).objects.filter(o=>o.id.startsWith('clue_')).every(o=>o.active));
   await page.waitForFunction(()=>document.body.innerText.includes('Step 2: compare your clues'));
   assert.match(await page.locator('body').innerText(),/Clue 1 Paw prints: examined/);
   assert.match(await page.locator('body').innerText(),/Clue 2 Blue threads: examined/);
   assert.match(await page.locator('body').innerText(),/Clue 3 Picnic scraps: examined/);
   if(stage===0){
    await key('Clues & paths');
    await page.getByRole('button',{name:'Examine again: Paw prints',exact:true}).waitFor();
    const routes=page.getByRole('button',{name:/^Path [ABC]: /});assert.equal(await routes.count(),3);
    for(const route of await routes.all())assert(await route.isEnabled(),'All routes become equally available');
    await shot('notebook-examined');await key('Back');
   }
   const afterPoints=await points();for(const id of ['clue_0','clue_1','clue_2','trail_0','trail_1','trail_2'])assert(Math.hypot(startPoints[id].x-afterPoints[id].x,startPoints[id].y-afterPoints[id].y)<1,'Clue feedback moved the next target');
   if(stage===0){
    const correct=Number(solution[3].object.at(-1)),before=await state();await click(`trail_${(correct+1)%3}`);
    assert.equal((await state()).revision,before.revision);assert.deepEqual((await state()).objects,before.objects);assert.equal((await pepper()).reaction,2);
    await page.waitForFunction(()=>pawsPepper.arrived||!pawsPepper.active,null,{timeout:90000});await shot('wrong-trail');
   }
   if(stage===1){
    await saved();const before=(await state()).objects;await page.reload();await ready();await key('Continue');
    await page.waitForFunction(()=>pawsUi.screen===2&&window.pawsPepper,null,{timeout:90000});assert.deepEqual((await state()).objects,before);assert.equal((await pepper()).stage,stage);
   }
   await shot(`stop-${stage+1}-ready`);await click(solution[stage*4+3].object);
   await idle();assert.equal((await pepper()).stage,Math.min(stage+1,stops-1));
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:30000});
  const complete=await state();assert.equal(complete.coins,initial.coins);await shot('basket-found');await saved();
  // Reopen the completed save with a fresh Chrome network/GPU process. Repeated
  // full asset loads in one local headless process can exhaust Chrome's transport
  // buffers (ERR_NO_BUFFER_SPACE). The mid-trail reload above stays in-process.
  const storageState=await context.storageState({indexedDB:true});
  await context.close();await browser.close();
  browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  context=await browser.newContext({viewport:desktop,storageState});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  page=await context.newPage();observePage(page);await page.goto(base);await ready();await key('Continue');
  // Completed requests intentionally resume in the home garden. Revisit the
  // saved beach request through its existing entry point, without restarting it.
  await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
  await key('Adventures');await key("Pepper's missing picnic basket · Revisit");
  await page.waitForFunction(()=>pawsState.quest==='beach_pepper_basket'&&pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});
  assert.equal((await state()).coins,initial.coins);assert(!(await pepper()).active);await shot('completed-reload');
  report.versions.push({version,stops,completed:true,undo:true,wrongTrail:true,reload:true,keyboard:true});await context.close();await browser.close();browser=null;
 }
 assert.deepEqual(report.errors,[]);report.passed=true;
 report.checks.push('World-only completion, wrong-path recovery, Pepper movement and sniff reactions','Undo cancels inspection; keyboard menu performs the same command','All six targets visible at desktop, portrait and short landscape sizes','Mid-trail reload and fresh-process completed-save restoration preserve progress and rewards');
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,pepper:window.pawsPepper,ui:window.pawsUi,loadError:window.pawsLoadError,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(root,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(root,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({backend,versions:report.versions,passed:report.passed,failure:report.failure,errors:report.errors},null,2));}})();
