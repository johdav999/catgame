// Real-site FPS and visual comparison. Uses isolated copies of a saved Chapter 3.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {execFileSync}=require('node:child_process');
const {createHash}=require('node:crypto');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/foliage-paving-20261015/comparison');
const before=process.env.BEFORE_URL||'http://127.0.0.1:19256';const after=process.env.AFTER_URL||'http://127.0.0.1:8156';
const json=x=>JSON.stringify(x,(_,v)=>typeof v==='bigint'?Number(v):v,2);
const summary=v=>{const s=[...v].sort((a,b)=>a-b);return {frames:v.length,fps:1000*v.length/v.reduce((a,b)=>a+b,0),meanMs:v.reduce((a,b)=>a+b,0)/v.length,p95Ms:s[Math.floor(s.length*.95)]};};
function unrealClosed(){const raw=execFileSync('powershell.exe',['-NoProfile','-Command',"@(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id) | ConvertTo-Json -Compress"],{encoding:'utf8'}).trim();assert(!raw||raw==='[]','Close Unreal before profiling: '+raw);return {checkedAt:new Date().toISOString(),processes:[]};}
(async()=>{
 await fs.mkdir(out,{recursive:true});let save=await fs.readFile(process.env.SAVE_FILE||'build/evidence/performance-20261015/chapter3.save','utf8');
 const staticMotion=process.env.LIVE!=='1';
 // The copy uses a static cat pose, for reproducible pixels and timing.
 if(staticMotion)save=save.replace(/(LEGACY "PAWS_AFTERNOON 5 \d+ [01] )([01]) /,'$11 ');
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={scope:'Desktop Chrome, Intel GPU, Chapter 3 copy, 2187x1244 DPR 1, Automatic 1x / MSAA off / shadows on. No GPU command interception. Unreal process absence checked before and after each run.',staticMotion,browser:browser.version(),warmupMs:14000,sampleMs:12000,runs:[],errors:[]};
 try{
  const modes=(process.env.MODES||'before,after,after,before').split(',');
  for(const [index,mode] of modes.entries()){
   const closedBefore=unrealClosed();const context=await browser.newContext({viewport:{width:2187,height:1244},deviceScaleFactor:1,reducedMotion:staticMotion?'reduce':'no-preference'});
   const assets=[];
   await context.addInitScript(()=>{const request=GPUAdapter.prototype.requestDevice;GPUAdapter.prototype.requestDevice=function(...a){window.benchmarkAdapter={vendor:this.info.vendor,architecture:this.info.architecture,description:this.info.description};return request.apply(this,a);};});
   const page=await context.newPage();page.on('response',response=>{if(/\/(?:paving(?:_lod[12])?|exterior_foliage(?:_lod[12])?)\.glb$/.test(response.url()))assets.push((async()=>({url:response.url(),status:response.status(),sha256:createHash('sha256').update(await response.body()).digest('hex')}))());});page.on('pageerror' ,e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
   const url=(mode==='before'?before:after)+'/?profile=1';await page.goto(url);
   const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});await ready();
   await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon',maxBytes:16*1024*1024});await s.put('current',new TextEncoder().encode(text));s.dispose();},save);
   await page.reload();await ready();
   async function click(name){const b=await page.evaluate(n=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===n);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;},name);assert(b,name);await page.mouse.click(b.x+b.width/2,b.y+b.height/2);}
   await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});await page.waitForTimeout(report.warmupMs);
   const intervals=await page.evaluate(ms=>new Promise(resolve=>{const v=[];let first,last;function frame(t){if(first===undefined){first=last=t;}else{v.push(t-last);last=t;}if(t-first>=ms)resolve(v);else requestAnimationFrame(frame);}requestAnimationFrame(frame);}),report.sampleMs);
   const state=await page.evaluate(()=>({build:pawsBuild,quality:pawsQuality,render:pawsRenderState,performance:pawsPerformance,scene:pawsState,adapter:window.benchmarkAdapter,canvas:[Module.canvas.width,Module.canvas.height]}));
   assert.deepEqual(state.canvas,[2187,1244]);assert.equal(state.render.effectiveMsaaSamples,1);assert.equal(state.render.maxShadowMaps,1);assert.equal(state.render.resolutionScale,1);assert.equal(state.render.backend,'webgpu');
   const run={index:index+1,mode,...summary(intervals),...state,intervals,assets:await Promise.all(assets),unreal:{before:closedBefore,after:unrealClosed()}};
   await page.screenshot({path:path.join(out,`${mode}-${index+1}.png`)});
   report.runs.push(run);console.log(json({index:index+1,mode,fps:run.fps,cpuMs:state.performance.timings.cpuMs.mean,gpuMs:state.performance.gpu.totalMs?.mean,worldMs:state.performance.gpu.layersMs[1]?.mean,triangles:state.performance.current.triangles,shadowTriangles:state.performance.current.shadowTriangles,drawCalls:state.performance.current.drawCalls}));await fs.writeFile(path.join(out,'report.json'),json(report));
   if(process.env.VISUALS==='1'&&!report.runs.slice(0,-1).some(r=>r.mode===mode)){
    for(const [name,operation] of [['side',()=>page.evaluate(()=>Module._paws_orbit(300,0))],['close',()=>page.evaluate(()=>{Module._paws_reset_view();Module._paws_zoom(-480);})],['portrait',()=>page.setViewportSize({width:390,height:844})],['landscape',()=>page.setViewportSize({width:844,height:390})]]){await operation();await page.waitForTimeout(900);await page.screenshot({path:path.join(out,mode+'-'+name+'.png')});}
   }
   await context.close();
  }
  report.summary=['before','after'].map(mode=>({mode,...summary(report.runs.filter(r=>r.mode===mode).flatMap(r=>r.intervals))}));report.improvementPercent=(report.summary[1].fps/report.summary[0].fps-1)*100;
  for(const mode of ['before','after']){const builds=report.runs.filter(r=>r.mode===mode).map(r=>r.build.buildId);assert.equal(new Set(builds).size,1,'Build changed during '+mode+' measurements');}assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}finally{await fs.writeFile(path.join(out,'report.json'),json(report));console.log(json({summary:report.summary,improvementPercent:report.improvementPercent,failure:report.failure}));await browser.close();}
})();
