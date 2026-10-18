const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/msaa-fps';
const base=process.env.PAWS_URL||'http://127.0.0.1:8121';
const json=v=>JSON.stringify(v,(_,x)=>typeof x==='bigint'?Number(x):x,2);
const summarize=values=>{const sorted=[...values].sort((a,b)=>a-b);return {frames:values.length,fps:1000*values.length/values.reduce((a,b)=>a+b,0),meanMs:values.reduce((a,b)=>a+b,0)/values.length,p50Ms:sorted[Math.floor(sorted.length*.5)],p95Ms:sorted[Math.floor(sorted.length*.95)]};};
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={browser:browser.version(),scope:'Headless Chrome on this PC, WebGPU, live garden, default camera, reduced motion',viewport:{width:1485,height:965,dpr:1},warmupSeconds:15,sampleSeconds:20,runs:[],errors:[]};
 try {
  // Counterbalance order so warming and gradual system load do not favor one mode.
  for(const [index,samples] of [1,4,4,1,1,4].entries()) {
   const context=await browser.newContext({viewport:report.viewport,deviceScaleFactor:1,reducedMotion:'reduce'});
   await context.addInitScript(()=>{
    const request=GPUAdapter.prototype.requestDevice;
    GPUAdapter.prototype.requestDevice=function(...args){window.benchmarkAdapter={vendor:this.info.vendor,architecture:this.info.architecture,device:this.info.device,description:this.info.description};return request.apply(this,args);};
   });
   const page=await context.newPage();
   page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
   await page.goto(`${base}/?msaa=${samples===1?'off':'4'}`);
   await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
   for(let i=0;i<100;i++){
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))==='Play'){await page.keyboard.press('Enter');break;}
    await page.keyboard.press('Tab');
   }
   await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
   await page.waitForTimeout(report.warmupSeconds*1000);
   const capture=()=>page.evaluate(()=>({build:pawsBuild,quality:pawsQuality,render:pawsRenderState,performance:pawsPerformance,adapter:window.benchmarkAdapter,scene:pawsState.scene,canvas:[Module.canvas.width,Module.canvas.height],lifecycle:pawsLifecycle}));
   const before=await capture();
   assert.equal(before.render.effectiveMsaaSamples,samples);assert.equal(before.render.resolutionScale,1);
   assert.equal(before.render.maxShadowMaps,1);assert.equal(before.render.shadowResolution,1024);
   assert.equal(before.render.backend,'webgpu');assert.deepEqual(before.canvas,[1485,965]);
   assert(before.performance.current.shadowDrawCalls>0,'Shadows must actually render');
   const intervals=await page.evaluate(ms=>new Promise(resolve=>{
    const values=[];let first,last;
    function frame(now){if(first===undefined){first=last=now;}else {values.push(now-last);last=now;}
     if(now-first>=ms)resolve(values);else requestAnimationFrame(frame);}
    requestAnimationFrame(frame);
   }),report.sampleSeconds*1000);
   const after=await capture();assert.equal(after.render.effectiveMsaaSamples,samples);
   const summary=summarize(intervals);report.runs.push({index:index+1,samples,...summary,before,after,intervals});
   await page.screenshot({path:path.join(out,`run-${index+1}-${samples}x.png`),scale:'css'});
   console.log(json({run:index+1,samples,...summary}));
   await fs.writeFile(path.join(out,'report.json'),json(report));await context.close();
  }
  report.summary=[1,4].map(samples=>({samples,...summarize(report.runs.filter(r=>r.samples===samples).flatMap(r=>r.intervals)),runFps:report.runs.filter(r=>r.samples===samples).map(r=>r.fps)}));
  report.fpsChangePercent=(report.summary[1].fps/report.summary[0].fps-1)*100;
  assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{await fs.writeFile(path.join(out,'report.json'),json(report));console.log(json({summary:report.summary,fpsChangePercent:report.fpsChangePercent,errors:report.errors,failure:report.failure}));await browser.close();}
})();
