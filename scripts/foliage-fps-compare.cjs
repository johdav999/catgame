// Compare complete Release sites on the same browser, camera and quality.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/foliage-performance');
const before=process.env.BEFORE_URL||'http://127.0.0.1:8137';
const after=process.env.AFTER_URL||'http://127.0.0.1:8138';
const json=v=>JSON.stringify(v,(_,x)=>typeof x==='bigint'?Number(x):x,2);
const summary=v=>{const s=[...v].sort((a,b)=>a-b);return {frames:v.length,fps:1000*v.length/v.reduce((a,b)=>a+b,0),meanMs:v.reduce((a,b)=>a+b,0)/v.length,p50Ms:s[Math.floor(s.length*.5)],p95Ms:s[Math.floor(s.length*.95)]};};
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={scope:'Desktop headless Chrome on this PC; matching garden camera, Automatic quality, 1x MSAA, reduced motion',browser:browser.version(),viewport:{width:1440,height:900},warmupMs:6000,sampleMs:10000,runs:[],errors:[]};
 try{
  const modes=process.env.BASELINE_ONLY?['before']:['before','after','after','before'];
  for(const [i,mode] of modes.entries()){
   const context=await browser.newContext({viewport:report.viewport,deviceScaleFactor:1,reducedMotion:'reduce'});
   const page=await context.newPage();
   page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
   await page.goto(`${mode==='before'?before:after}/?msaa=off`);
   await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
   const box=await page.evaluate(()=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')==='Play');return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId).bounds;});
   assert(box,'Play button');await page.mouse.click(box.x+box.width/2,box.y+box.height/2);
   await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});
   await page.waitForTimeout(report.warmupMs);
   const capture=()=>page.evaluate(()=>({build:pawsBuild,quality:pawsQuality,render:pawsRenderState,performance:pawsPerformance,scene:pawsState.scene,canvas:[Module.canvas.width,Module.canvas.height]}));
   const start=await capture();assert.deepEqual(start.canvas,[1440,900]);assert.equal(start.render.backend,'webgpu');assert.equal(start.render.effectiveMsaaSamples,1);
   const values=await page.evaluate(ms=>new Promise(resolve=>{let first,last;const v=[];function frame(t){if(first===undefined){first=last=t;}else{v.push(t-last);last=t;}if(t-first>=ms)resolve(v);else requestAnimationFrame(frame);}requestAnimationFrame(frame);}),report.sampleMs);
   const end=await capture();report.runs.push({index:i+1,mode,...summary(values),start,end,intervals:values});
   await page.screenshot({path:path.join(out,`${mode}-${i+1}.png`)});
   console.log(JSON.stringify({mode,index:i+1,...summary(values),counters:end.performance.current,gpu:end.performance.gpu}));
   await fs.writeFile(path.join(out,'report.json'),json(report));await context.close();
  }
  report.summary=['before','after'].filter(mode=>report.runs.some(r=>r.mode===mode)).map(mode=>({mode,...summary(report.runs.filter(r=>r.mode===mode).flatMap(r=>r.intervals))}));
  if(report.summary.length===2)report.fpsChangePercent=(report.summary[1].fps/report.summary[0].fps-1)*100;
  assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{await fs.writeFile(path.join(out,'report.json'),json(report));console.log(JSON.stringify({summary:report.summary,fpsChangePercent:report.fpsChangePercent,failure:report.failure}));await browser.close();}
})();
