const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/picture-sequence/browser');
const url=process.env.PAWS_URL||'http://127.0.0.1:8141/';
let browser,page;const report={errors:[],checks:[],screenshots:[],assets:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900},reducedMotion:'reduce'});page=await context.newPage();
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const assetTasks=[];page.on('response',r=>{if(/\/(?:sequence_(?:blackboard|picture_\w+)|bell_(?:support|leaf|star|paw))\.glb$/.test(r.url()))assetTasks.push((async()=>({url:r.url(),status:r.status(),sha256:crypto.createHash('sha256').update(await r.body()).digest('hex')}))());});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(120);}
 const state=()=>page.evaluate(()=>pawsState);
 async function point(id){return page.evaluate(id=>{Module._paws_interaction_snapshot();return pawsInteraction.objects[id];},id);}
 async function click(id){const p=await point(id);assert(p,id);await page.mouse.click(p.x,p.y);await page.waitForTimeout(100);}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
 await page.goto(url);await ready();
 const fixture=await fs.readFile('build/evidence/picture-sequence/chapter3.save','utf8');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},fixture);
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&pawsState.quest==='pepper_garden_bells'&&pawsSequence?.watching);
 assert(!await page.evaluate(()=>document.body.innerText.includes('leaf, star, leaf, paw, star')),'Answer leaked in instructions');
 await key('Replay pictures');const before=await state();await click('leaf');assert.equal((await state()).revision,before.revision,'Early bell tap changed answer');
 await key('Menu');const held=await page.evaluate(()=>pawsSequence.elapsed);await page.waitForTimeout(1300);assert.equal(await page.evaluate(()=>pawsSequence.elapsed),held,'Pause advanced picture');await key('Continue');
 report.checks.push('Early taps rejected and pause freezes clue playback');
 await key('Replay pictures');
 const timeline=await page.evaluate(()=>new Promise(resolve=>{const events=[];let old=-1;function poll(){const s=pawsSequence;if(s.index!==old){events.push({index:s.index,picture:s.picture,elapsed:s.elapsed,wall:performance.now()});old=s.index;}if(s.watching)requestAnimationFrame(poll);else resolve(events);}poll();}));
 assert.deepEqual(timeline.map(x=>x.picture),['leaf','star','leaf','paw','star','']);
 for(let i=1;i<timeline.length;i++){const dt=timeline[i].elapsed-timeline[i-1].elapsed;assert(dt>.75&&dt<1.25,'One-second picture interval '+dt);}
 report.timeline=timeline;report.checks.push('Five pictures shown in order for one second each; board blank afterwards');await shot('answer-desktop');
 await key('Replay pictures');await page.waitForFunction(()=>pawsSequence.picture==='star'&&pawsSequence.watching);await shot('tripo-models-picture');await page.waitForFunction(()=>!pawsSequence.watching);
 await click('leaf');await click('paw');assert(!(await state()).completed);await click('sequence_board');await page.waitForFunction(()=>pawsSequence.watching);assert.match((await state()).message,/Watch/);
 await page.waitForFunction(()=>!pawsSequence.watching);assert((await state()).canUndo);report.checks.push('Wrong answer clears prefix and clicking the standing board replays');
 await key('Replay pictures');await key('Menu');await shot('paused-picture');await key('Continue');await page.waitForFunction(()=>!pawsSequence.watching);
 await click('leaf');await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsSequence?.watching&&pawsState.quest==='pepper_garden_bells');await page.waitForFunction(()=>!pawsSequence.watching);
 assert.equal((await state()).coins,140);await click('star');await click('leaf');await click('paw');await click('star');await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);
 assert.equal((await state()).coins,160);report.checks.push('Save/reload retains entered prefix, replays clue, and rewards completion once');await shot('complete');
 // Restart through the existing confirmation flow, retaining earned rewards.
 await page.evaluate(()=>Module._paws_reset_view());
 await key('Back to garden');await page.waitForFunction(()=>pawsState.garden);
 await key('Menu');await key('Requests & friends');await key('The garden bells · Practice');await key('Help Pepper');
 for(const [w,h] of [[390,844],[844,390]]){await page.setViewportSize({width:w,height:h});await key('Replay pictures');
  const p=await point('sequence_board');const rect=await page.evaluate(()=>pawsUi.board);assert(p.x>=rect.x&&p.x<=rect.x+rect.width&&p.y>=rect.y&&p.y<=rect.y+rect.height,'Blackboard picture is obscured by HUD');
  await shot('watch-'+w+'x'+h);await page.waitForFunction(()=>!pawsSequence.watching);await shot('answer-'+w+'x'+h);}
 assert.equal((await state()).coins,160);
 report.assets=await Promise.all(assetTasks);
 for(const name of ['sequence_blackboard','sequence_picture_leaf','sequence_picture_star','sequence_picture_paw']){
  const sha=crypto.createHash('sha256').update(await fs.readFile('assets/runtime/models/garden/'+name+'.glb')).digest('hex');assert(report.assets.some(r=>r.url.endsWith('/'+name+'.glb')&&r.status===200&&r.sha256===sha),name+' not loaded exactly');
 }
 for(const name of ['bell_support','bell_leaf','bell_star','bell_paw']){
  const sha=crypto.createHash('sha256').update(await fs.readFile('assets/runtime/models/release1/props/'+name+'.glb')).digest('hex');assert(report.assets.some(r=>r.url.endsWith('/'+name+'.glb')&&r.status===200&&r.sha256===sha),name+' replacement not loaded exactly');
 }
 assert.deepEqual(report.errors,[]);report.build=await page.evaluate(()=>pawsBuild);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,sequence:window.pawsSequence,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;console.error(e);}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});
