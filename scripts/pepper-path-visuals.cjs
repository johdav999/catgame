// Scene-only visual acceptance. Fixtures establish preconditions; all quest input
// uses world pointer targets. No notebook route descriptions are opened.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu';
const out=process.env.EVIDENCE_DIR||`build/evidence/pepper-paths/${backend}`;
const url=process.env.PAWS_URL||'http://127.0.0.1:8198/?devBeach=1';
const version=process.env.PEPPER_VERSION||'v2';
const fullMotion=process.env.PEPPER_MOTION==='full';
let browser,page;
const report={backend,version,motion:fullMotion?'full':'reduced',errors:[],httpErrors:[],stops:[]};
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 page.on('response',r=>{if(r.status()>=400)report.httpErrors.push({url:r.url(),status:r.status()});});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:120000});
 const state=()=>page.evaluate(()=>pawsState);
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction.objects;});
 const key=async(name)=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(160);};
 const click=async(id)=>{const p=(await points())[id];assert(p&&p.x>=0&&p.y>=0&&p.x<page.viewportSize().width&&p.y<page.viewportSize().height);await page.mouse.click(p.x,p.y);await page.waitForTimeout(200);};
 await page.goto(url);await ready();
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},await fs.readFile(`build/evidence/pepper-world/fixtures/pepper-${version}.save`,'utf8'));
 await page.reload();await ready();await key('Continue');
 await page.waitForFunction(()=>pawsState.quest==='beach_pepper_basket'&&pawsUi.screen===2&&window.pawsPepper,null,{timeout:120000});
 assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);
 if(!fullMotion){await key('Menu');await key('Settings');await key('Motion: Full');await key('Back');await key('Continue');}
 const initial=await state(),solution=initial.solution;
 for(let stage=0;stage<solution.length/4;stage++){
  const body=await page.locator('body').innerText();assert(!body.includes('Follow:'));
  assert((await state()).objects.filter(o=>o.id.startsWith('clue_')).every(o=>!o.active),'New stop requires new inspections');
  for(const clue of ['clue_0','clue_1','clue_2']){
   await click(clue);
   const evidence=(await state()).objects.find(o=>o.id===clue).name;
   if(fullMotion){
    await page.waitForFunction(()=>pawsPepper.active&&!pawsPepper.arrived,null,{timeout:15000});
    const travelling=await page.evaluate(()=>pawsPepper);
    assert(travelling.active&&!travelling.arrived,'Pepper must be walking before evidence appears');
    assert(!(await page.locator('body').innerText()).includes(evidence),'Clue text stays hidden during travel');
    if(stage===0&&clue==='clue_0')await page.screenshot({path:path.join(out,'clue-before-arrival.png')});
    await page.waitForFunction(()=>pawsPepper.arrived,null,{timeout:90000});
   }
   await page.waitForFunction(text=>document.body.innerText.includes(text),evidence);
   if(stage===0&&clue==='clue_0')await page.screenshot({path:path.join(out,'clue-after-arrival.png')});
  }
  assert((await state()).objects.filter(o=>o.id.startsWith('clue_')).every(o=>o.active),'All world clues selectable');
  const correct=solution[stage*4+3].object;
  const before=await state();await click(`trail_${(Number(correct.at(-1))+1)%3}`);
  assert.deepEqual((await state()).objects,before.objects,'Wrong path preserves evidence');
  const evidence=before.objects.filter(o=>o.id.startsWith('clue_')).map(o=>o.name);
  for(const viewport of [{width:1600,height:1000},{width:390,height:844},{width:844,height:390}]){
   await page.setViewportSize(viewport);await page.waitForTimeout(350);
   const p=await points();
   for(const id of ['trail_0','trail_1','trail_2','clue_0','clue_1','clue_2'])assert(p[id].x>0&&p[id].y>0&&p[id].x<viewport.width&&p[id].y<viewport.height,JSON.stringify({id,viewport,p:p[id]}));
   assert(p.clue_0.y>p.clue_1.y&&p.clue_1.y>p.clue_2.y&&p.clue_2.y>p.trail_1.y,'Clues lead vertically from the foreground toward the path fork');
   const hud=await page.locator('body').innerText();
   for(const clue of evidence)assert(hud.includes(clue),'Examined evidence stays visible after wrong choice: '+clue);
   await page.screenshot({path:path.join(out,`stop-${stage+1}-${viewport.width}.png`)});
  }
  await page.setViewportSize({width:1600,height:1000});await page.waitForTimeout(200);
  report.stops.push({stage:stage+1,choices:before.objects.filter(o=>o.id.startsWith('trail_')),worldInput:true});
  await click(correct);await page.waitForFunction(()=>!pawsPepper.active,null,{timeout:90000});
  if(stage+1<solution.length/4){const hud=await page.locator('body').innerText();for(const clue of evidence)assert(!hud.includes(clue),'Previous stop evidence clears');}
  console.log(backend,'stop',stage+1,'complete');
 }
 assert((await state()).completed);assert.equal((await state()).coins,initial.coins);
 assert.deepEqual(report.errors,[]);assert.deepEqual(report.httpErrors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,pepper:window.pawsPepper,loadError:window.pawsLoadError,render:window.pawsRenderState})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({backend,passed:report.passed,failure:report.failure,errors:report.errors}));}})();

