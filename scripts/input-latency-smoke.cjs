const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={errors:[],clicks:[]};const out=process.env.EVIDENCE_DIR||'build/evidence/input-latency';
 try{
  const page=await browser.newPage({viewport:{width:1600,height:1000},hasTouch:true});page.setDefaultTimeout(60000);
  page.on('pageerror',e=>report.errors.push(e.message));
  if(process.env.RENDER_BACKEND==='webgl2')await page.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  await page.goto(process.env.PAWS_URL||'http://localhost:8098/');
  await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&pawsRenderState?.state==='ready');
  async function key(name){const control=page.getByRole('button',{name,exact:true});await control.focus();await page.keyboard.press('Enter');await page.waitForTimeout(300);}
  const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
  await key('Play');await page.waitForFunction(()=>pawsState.garden);await key('Menu');await key('Requests & friends');
  let labels=await names();while(labels.some(n=>n.startsWith('Welcome '))){await key(labels.find(n=>n.startsWith('Welcome ')));labels=await names();}
  await key("Pip's missing ball · Available");await key('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);await page.waitForTimeout(500);
  await page.evaluate(()=>{window.inputSamples=[];const original=Module._paws_object_pointer;Module._paws_object_pointer=(...args)=>{const start=performance.now();try{return original(...args);}finally{inputSamples.push({phase:args[0],ms:performance.now()-start});}};});
  for(let n=0;n<8;n++){
   // These channels are exposed initially; other channels share cells with pots.
   const id=n%2?'channel_e':'channel_d';
   const before=await page.evaluate(id=>{Module._paws_interaction_snapshot();return {point:pawsInteraction.objects[id],orientation:pawsState.objects.find(o=>o.id===id).orientation};},id);
   if(n>=4)await page.touchscreen.tap(before.point.x,before.point.y);
   else {await page.mouse.move(before.point.x,before.point.y);await page.mouse.down();await page.waitForTimeout(30);await page.mouse.up();}
   await page.waitForFunction(({id,orientation})=>pawsState.objects.find(o=>o.id===id).orientation===(orientation+1)%4,{id,orientation:before.orientation});
   report.clicks.push({id,before:before.orientation,input:n>=4?'touch':'mouse'});await page.waitForTimeout(300);
  }
  report.samples=await page.evaluate(()=>inputSamples);report.build=await page.evaluate(()=>pawsBuild);report.renderer=await page.evaluate(()=>pawsRenderState.backend);
  assert.equal(report.errors.length,0);assert(report.samples.filter(s=>s.phase===0||s.phase===2).every(s=>s.ms<Number(process.env.MAX_INPUT_MS||100)),'Input handler exceeded latency budget');
  report.passed=true;
 }finally{await fs.mkdir(out,{recursive:true});await fs.writeFile(out+'/latency.json',JSON.stringify(report,null,2));await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
