// Real generated game, isolated saves, trusted mouse drags. Never edits the user's save.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const backend=process.env.TIDY_BACKEND||'webgpu';
const out=path.resolve(process.env.EVIDENCE_DIR||`build/evidence/tidy-side-access/${backend}`);
const report={backend,checks:[],cases:[],errors:[]};let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const fixture=await fs.readFile('build/evidence/tidy-side-access/fixtures/ball-first.save','utf8');
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 for(const order of [['fish','mouse'],['mouse','fish']]){
  const context=await browser.newContext({viewport:{width:1600,height:1000}});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  page=await context.newPage();page.setDefaultTimeout(90000);
  page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  const ready=()=>page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready');
  const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(350);
  async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
  async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
  async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to,JSON.stringify({id,target,p}));const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];const b=/^\d+,\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
  async function shot(name){const file=path.join(out,`${order.join('-')}-${name}.png`);await page.screenshot({path:file});return file;}
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8156/');await ready();
  await page.evaluate(async text=>{const store=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await store.put('current',new TextEncoder().encode(text));}finally{store.dispose();}},fixture);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);await settle();
  assert.equal((await state()).coins,20);assert.equal((await state()).objects.find(o=>o.id==='ball').x,4);
  // The saved state matches the reports: ball packed, cushion at the front.
  // Fish is still covered, but mouse is exposed.
  await drag('fish','fish_recess');assert(!(await state()).completed);assert.match((await state()).message,/pickup position/);
  report.checks.push('Covered fish remains blocked by the cushion');
  const screenshots=[];
  for(let n=0;n<order.length;n++){
   const toy=order[n],parking=toy==='fish'?'1,0':'1,2';
   await drag('cushion',parking);await settle();
   if(n===0){
    const before=await state();await drag(toy,toy==='fish'?'mouse_recess':'fish_recess');assert.match((await state()).message,/shape/);assert.deepEqual((await state()).objects,before.objects);
    await drag(toy,toy+'_recess',true);assert.deepEqual((await state()).objects,before.objects);
    report.checks.push('Wrong recess and cancelled drag leave all committed toy positions intact');
   }
   screenshots.push(await shot(toy+'-exposed'));
   await drag(toy,toy+'_recess');const after=await state();assert.equal(after.objects.find(o=>o.id===toy).x,4,JSON.stringify(after));
   assert.equal(after.objects.find(o=>o.id==='ball').x,4,'Previously packed ball stays in its recess');
   screenshots.push(await shot(toy+'-packed'));
   if(n===0){
    await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);await settle();
    assert.equal((await state()).objects.find(o=>o.id===toy).x,4);assert.equal((await state()).coins,20);
    report.checks.push('Ball-first save reload preserves the packed toy and grants no premature coins');
   }
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);assert.equal((await state()).coins,40);
  await page.waitForFunction(()=>pawsRewards.phase===1&&pawsRewards.time>.15);
  const contact=await page.evaluate(()=>pawsEnding);assert(contact.finished&&contact.x<.95,'Basket contact must stand off from the visible rim');
  screenshots.push(await shot('coin-clearance'));report.checks.push('Cat stays outside the basket while the 3D coin reward rises');
  await page.waitForFunction(()=>pawsRewards.phase===3);
  screenshots.push(await shot('settled-clearance'));
  await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsState.save===2);await page.reload();await ready();assert.equal((await state()).coins,40);
  report.cases.push({order,passed:true,contact,screenshots,build:await page.evaluate(()=>pawsBuild)});console.log('Passed ball then '+order.join(' then '));await context.close();
 }
 report.build=report.cases[0].build;report.wasmSha256=crypto.createHash('sha256').update(await fs.readFile('build/web-local-release/site/paws.wasm')).digest('hex');assert.equal(report.build.wasmSha256,report.wasmSha256);assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();console.log(JSON.stringify({passed:report.passed,cases:report.cases.length,failure:report.failure},null,2));});
