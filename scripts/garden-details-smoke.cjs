const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const out=process.env.EVIDENCE_DIR||'build/evidence/garden-details';
const names=['watering_can','lantern','book_stack','garden_sign','bench_cushion','herb_pot','paw_mat','empty_pot','climbing_ivy','moss_flowers','stone_border'];
(async()=>{
 await fs.mkdir(out,{recursive:true});const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={errors:[],assets:[],screenshots:[]};const responses=[];
 try{
  const page=await browser.newPage({viewport:{width:1600,height:1000}});
  page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  page.on('response',r=>{const name=names.find(n=>r.url().endsWith('/'+n+'.glb'));if(name)responses.push((async()=>({name,status:r.status(),hash:crypto.createHash('sha256').update(await r.body()).digest('hex')}))());});
  async function key(name){for(let n=0;n<250;n++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(180);return;}await page.keyboard.press('Tab');}throw Error('Unreachable '+name);}
  async function shot(name){await page.waitForTimeout(700);await page.screenshot({path:path.join(out,name+'.png'),scale:'css'});report.screenshots.push(name+'.png');}
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8112/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await shot('garden-desktop');
  await page.mouse.move(800,500);await page.mouse.wheel(0,-160);await shot('garden-details-close');
  await key('Requests & friends');let labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
  while(labels.some(n=>n.startsWith('Welcome '))){await key(labels.find(n=>n.startsWith('Welcome ')));labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
  await key("Pip's missing ball · Available");await key('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2);await shot('puzzle-desktop');
  for(const [label,width,height] of [['desktop',1600,1000],['portrait',390,844],['landscape',844,390]]){
   await page.setViewportSize({width,height});await shot('puzzle-'+label);
   const pts=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
   const a=pts.objects.pot_a,from=pts.slots['1,2'],to=pts.slots['1,1'];
   await page.mouse.move(a.x,a.y);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.move(a.x+to.x-from.x,a.y+to.y-from.y,{steps:8});await page.waitForTimeout(80);await page.mouse.up();
   await page.waitForFunction(()=>pawsState.objects.find(o=>o.id==='pot_a').y===1);
   await key('Objects & actions');await key('Undo');await key('Back');assert.equal(await page.evaluate(()=>pawsState.objects.find(o=>o.id==='pot_a').y),2);
  }
  report.assets=await Promise.all(responses);
  for(const name of names){const expected=crypto.createHash('sha256').update(await fs.readFile('assets/runtime/models/garden/'+name+'.glb')).digest('hex');assert(report.assets.some(r=>r.name===name&&r.status===200&&r.hash===expected),'Missing or stale '+name);}
  report.build=await page.evaluate(()=>pawsBuild);report.interaction='Projected drag and undo passed in desktop, portrait and landscape';assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser.close();}
})();
