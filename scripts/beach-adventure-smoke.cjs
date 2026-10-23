// Real retained GUI controls, scene gestures and IndexedDB restoration.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const crypto=require('node:crypto');
const {PNG}=require('pngjs');
const backend=process.env.QUEST_BACKEND||'webgpu';
const base=process.env.PAWS_URL||'http://127.0.0.1:8193/';
const out=process.env.EVIDENCE_DIR||`build/evidence/beach-adventure/${backend}`;
const report={backend,url:base,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const wasm=[],coastal=[];page.on('response',r=>{
  const url=new URL(r.url());
  if(url.pathname.endsWith('/paws.wasm'))wasm.push(r.body().then(b=>crypto.createHash('sha256').update(b).digest('hex')));
  if(/\/assets\/models\/beach\/[^/]+\.glb$/.test(url.pathname))coastal.push(r.body().then(b=>({file:path.basename(url.pathname),status:r.status(),bytes:b.length,sha256:crypto.createHash('sha256').update(b).digest('hex')})));
 });
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(220);
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const saved=()=>page.waitForFunction(()=>pawsState.save===2,null,{timeout:15000});
 const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function click(id){const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();}
 async function drag(id,cell,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[cell];assert(a&&b,'Missing drag points');await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:14});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 async function garden(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});}
 async function beach(){await key('Adventures');await key('Start / Continue beach');await page.waitForFunction(()=>pawsUi.screen===3&&!pawsState.garden,null,{timeout:90000});await key('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2);}
 const dev=new URL(base);dev.searchParams.set('devBeach','1');
 const disabled=new URL(base);disabled.searchParams.set('devBeach','0');
 await page.goto(disabled.href);await ready();await key('Play');await garden();assert(!(await names()).includes('Adventures'));
 report.checks.push('Explicitly disabled development access has no adventure entry');
 await saved();await page.goto(dev.href);await ready();await key('Continue');await garden();await beach();
 report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);
 assert.equal((await state()).quest,'beach_pip_ball');assert.match((await state()).scene,/beach cove/);
 const initial=await state();assert.equal(initial.coins,0);assert.equal(initial.sceneCount,1);
 async function grounding(completed=false){
  const p=await points();
  assert(Math.abs(p.objects.source_ball.worldY-(completed?.22:.06))<.001,'Ball base rests on sand or the picnic mat');
  for(const id of ['channel_a','channel_b','channel_c'])assert(p.objects[id].worldY<.101,'Channel geometry rests on sand: '+id);
 }
 await grounding();
 async function environment(){
  return page.evaluate(()=>{Module._paws_terrain_snapshot();Module._paws_water_snapshot();return {terrain:pawsTerrain,water:pawsWater};});
 }
 report.environment=await environment();
 report.coastal=await Promise.all(coastal);
 const coastManifest=JSON.parse(await fs.readFile('assets/runtime/models/beach/manifest.json','utf8'));
 for(const asset of coastManifest.assets){
  const fetched=report.coastal.find(r=>r.file===path.basename(asset.url));
  assert(fetched&&fetched.status===200&&fetched.sha256===asset.sha256&&fetched.bytes===asset.bytes,'Exact coastal GLB bytes loaded: '+asset.name);
  const instance=report.environment.terrain.coastal.find(c=>c.name===asset.name);
  if(!asset.quests||asset.quests.includes((await state()).quest))
   assert(instance&&instance.instances>0,'Imported coastal asset used in active beach scene: '+asset.name);
 }
 assert(report.environment.terrain.enabled&&report.environment.terrain.chunks===64&&report.environment.terrain.layered&&report.environment.terrain.frustumCovered,JSON.stringify(report.environment));
 assert(report.environment.terrain.layerTextures.length===4&&report.environment.terrain.layerTextures.every(t=>t.width===1254&&t.height===1254&&t.repeat),'Generated coastal sand is bound to every terrain layer');
 const sand=await page.request.get(new URL('assets/textures/beach/sand-v1.png',page.url()).href);assert(sand.ok());
 report.sandSha256=crypto.createHash('sha256').update(await sand.body()).digest('hex');
 assert.equal(report.sandSha256,'bc0482553f976df8b8011f221ea5926efd43584cec074420a9279f8d5e5fa126');
 const sea=report.environment.water.surfaces.find(s=>s.name==='Beach sea');assert(sea&&sea.amplitude>0&&sea.foam>0&&sea.refraction>0&&sea.vertices>1000);
 assert.equal(report.render.backend,backend,'Must exercise the requested actual renderer');
 const beforeWater=PNG.sync.read(await page.screenshot());
 const time=report.environment.water.time;await page.waitForTimeout(550);assert((await environment()).water.time>time,'Waves advance during play');
 const afterWater=PNG.sync.read(await page.screenshot());let waterPixels=0,movingWaterPixels=0;
 // Inspect the open upper-middle sea, excluding both HUD corners and puzzle.
 for(let y=70;y<350;y++)for(let x=520;x<1250;x++){
  const i=(y*beforeWater.width+x)*4,a=beforeWater.data,b=afterWater.data;
  if(a[i+1]>a[i]+8&&a[i+2]>a[i]+6){waterPixels++;if(Math.abs(a[i]-b[i])+Math.abs(a[i+1]-b[i+1])+Math.abs(a[i+2]-b[i+2])>3)movingWaterPixels++;}
 }
 report.waterPixels={visible:waterPixels,animated:movingWaterPixels};assert(waterPixels>2000&&movingWaterPixels>100,JSON.stringify(report.waterPixels));
 await key('Menu');const paused=(await environment()).water.time;await page.waitForTimeout(350);assert.equal((await environment()).water.time,paused,'Pause freezes water');await key('Continue');
 report.checks.push('Actual terrain and subdivided water shader, depth foam/refraction, playing and paused water clock');
 await page.setViewportSize({width:844,height:390});await settle();assert((await environment()).terrain.frustumCovered);await page.screenshot({path:path.join(out,'beach-short-landscape.png')});
 await page.setViewportSize({width:1600,height:1000});await settle();
 await page.screenshot({path:path.join(out,'beach-start.png')});
 await page.evaluate(()=>Module._paws_orbit(0,(.18-.88)/.006));await settle();
 await page.screenshot({path:path.join(out,'beach-ground-contact-low.png')});
 await page.evaluate(()=>Module._paws_orbit(0,(.88-.18)/.006));await settle();
 await drag('basket_b','1,0',true);assert.deepEqual((await state()).objects,initial.objects);
 await drag('basket_b','1,0');assert.equal((await state()).objects.find(o=>o.id==='basket_b').y,0);
 await key('Menu');await key('Undo');await key('Continue');assert.deepEqual((await state()).objects,initial.objects);
 report.checks.push('Cancelled scene drag leaves pieces unchanged; Undo restores a committed slide');
 await grounding();
 await drag('basket_b','1,0');await drag('basket_c','3,0');await drag('basket_b','2,0');await drag('basket_a','1,1');
 assert.equal((await state()).objects.find(o=>o.id==='basket_a').y,1,'Front basket is draggable above its covered channel');
 const halfway=(await state()).objects;await saved();await page.reload();await ready();await key('Continue');
 await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Reload while on the beach restores the exact board');
 await grounding();
 await key('Menu');await key('Back to garden');await garden();assert.equal((await state()).quest,'pip_missing_ball');
 await saved();await page.reload();await ready();await key('Continue');await garden();await beach();assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Leave, reload in the garden and return resumes the beach board and original base request');
 // Removing development access preserves the paid-content-shaped progress,
 // returns safely home, and never treats an imported/local save as ownership.
 await saved();await page.goto(disabled.href);await ready();await key('Continue');await garden();assert(!(await names()).includes('Adventures'));
 await saved();await page.goto(dev.href);await ready();await key('Continue');await garden();await beach();assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Removing and restoring development access preserves progress without granting ownership');
 await page.setViewportSize({width:390,height:844});await settle();
 assert((await environment()).terrain.frustumCovered);
 const p=await points(),board=await page.evaluate(()=>pawsUi.board);
 for(const id of ['basket_a','basket_b','channel_a','channel_b','channel_c','source_ball']){const a=p.objects[id];assert(a.x>=board.x&&a.x<=board.x+board.width&&a.y>=board.y&&a.y<=board.y+board.height,JSON.stringify({id,a,board}));}
 await page.screenshot({path:path.join(out,'beach-portrait.png')});
 await drag('basket_a','1,0');assert.equal((await state()).objects.find(o=>o.id==='basket_a').y,0);
 await drag('basket_d','4,0');
 await page.setViewportSize({width:1600,height:1000});await settle();
 for(const id of ['channel_a','channel_b','channel_b','channel_b','channel_c','channel_d','channel_e'])await click(id);
 await page.screenshot({path:path.join(out,'beach-channels-connected.png')});
 await click('source_ball');
 report.rollHeights=[];
 for(let n=0;n<120;n++){
  const p=(await points()).objects.source_ball;report.rollHeights.push(p.worldY-.06);
  assert(p.worldY>=.059&&p.worldY<=.221,'Rolling ball stays at the supporting surface height');
  if(await page.evaluate(()=>pawsUi.screen===9))break;
  await page.waitForTimeout(40);
 }
 assert(report.rollHeights.some(y=>y>.015&&y<.145),'Observed ball travelling at grounded channel height');
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:20000});
 await grounding(true);report.checks.push('Ball and channels stay grounded after slides, Undo, reload and completion');
 assert.equal((await state()).coins,0);assert.equal((await state()).chapter,1);assert(!(await page.locator('body').innerText()).includes('20 coins'));
 await page.screenshot({path:path.join(out,'beach-complete.png')});await saved();
 await page.reload();await ready();await key('Continue');await garden();await key('Adventures');await key("Pip's runaway beach ball · Revisit");
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});assert.equal((await state()).coins,0);
 report.checks.push('Portrait drag and scene clicks solve the quest; completed revisit retains progress and grants no base rewards');
 await key('Back to garden');await garden();await saved();
 report.final=await state();report.servedWasm=await Promise.all(wasm);assert.equal(report.final.sceneCount,1);assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}})();
