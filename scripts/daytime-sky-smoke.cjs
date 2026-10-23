// Exercise the generated game, including sky pixels and scene replacement.
const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/daytime-sky-final';
const report={checks:[],screenshots:[],errors:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1440,height:900}});
 page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const settle=()=>page.waitForTimeout(800);
 const state=()=>page.evaluate(()=>pawsState);
 const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 async function action(name){
  await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();
 }
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function drag(id,target){const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to);const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];const b=/^\d+,\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});await page.mouse.up();await settle();}
 async function rotate(id){const before=(await state()).objects.find(o=>o.id===id).orientation;await click(id);if((await state()).objects.find(o=>o.id===id).orientation===before&&(await points()).selectedPlank===id)await click(id);}
 async function click(id){const p=(await points()).objects[id];assert(p);await page.mouse.click(p.x,p.y);await settle();}
 async function shot(name){
  await settle();const buffer=await page.screenshot({path:path.join(out,name+'.png')});
  const png=PNG.sync.read(buffer);let blue=0;
  for(let i=0;i<png.data.length;i+=4){const [r,g,b]=png.data.subarray(i,i+3);if(r>90&&b>r+35&&g>r+15)blue++;}
  assert(blue>png.width*png.height*.005,'Daytime blue sky missing from '+name);
  report.screenshots.push({name,bluePixels:blue});
 }
 try{
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8104/');
  await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  await action('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await shot('garden');
  // A real background drag exposes the upper hemisphere and clouds.
  await page.mouse.move(1240,220);await page.mouse.down();await page.mouse.move(1240,60,{steps:12});await page.mouse.up();await shot('garden-horizon');
  await page.evaluate(()=>Module._paws_reset_view());
  await action('Menu');await action('Requests & friends');let options=await names();
  while(options.some(n=>n.startsWith('Welcome '))){await action(options.find(n=>n.startsWith('Welcome ')));options=await names();}
  await action("Pip's missing ball · Available");await action('Help Pip');
  await page.waitForFunction(()=>!pawsState.garden&&pawsUi.screen===2);await shot('puzzle');
  await drag('pot_a','1,1');assert.equal((await state()).objects.find(o=>o.id==='pot_a').y,1);
  await action('Menu');await action('Undo');await action('Continue');assert.equal((await state()).objects.find(o=>o.id==='pot_a').y,2);
  await action('Menu');await action('Restart puzzle');await action('Restart puzzle');await shot('puzzle-restarted');
  const solution=(await state()).solution,delta=[[0,-1],[1,0],[0,1],[-1,0]];
  for(const step of solution){
   if(step.verb===1){const o=(await state()).objects.find(o=>o.id===step.object),d=delta[step.direction];await drag(step.object,`${o.x+d[0]},${o.y+d[1]}`);const moved=(await state()).objects.find(o=>o.id===step.object);assert.equal(moved.x,o.x+d[0]);assert.equal(moved.y,o.y+d[1]);}
   else if(step.verb===2)await rotate(step.object);
   else if(step.verb===3)await click('source_ball');
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);await action('Back to garden');await shot('garden-return');
  report.checks.push('mouse drag, undo, restart, completion and return retain sky');
  for(const [width,height] of [[390,844],[844,390]]){await page.setViewportSize({width,height});await shot(`garden-${width}x${height}`);}
  await page.setViewportSize({width:1440,height:900});await page.waitForFunction(()=>pawsState.save===2);
  await page.reload();await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await settle();await action('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production);await shot('garden-reloaded');
  report.checks.push('portrait, landscape and saved reload retain sky');
  report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;report.state=await state().catch(()=>null);await page.screenshot({path:path.join(out,'failure.png')});process.exitCode=1;}
 finally{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser.close();console.log(JSON.stringify(report,null,2));}
})();
