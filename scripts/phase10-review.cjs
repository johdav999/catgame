// Final visual/performance review against the packaged payload under a URL prefix.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto'),zlib=require('node:zlib');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase10-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/phase10/review');
const port=Number(process.env.PAWS_TEST_PORT||19521),prefix='/review/paws/',url=`http://127.0.0.1:${port}${prefix}`;
const soakSeconds=Number(process.env.PAWS_SOAK_SECONDS||900);
const json=value=>JSON.stringify(value,(_key,v)=>typeof v==='bigint'?(v<=BigInt(Number.MAX_SAFE_INTEGER)?Number(v):String(v)):v,2);
const report={site,url,kind:'scripted desktop review; not an independent first-player playtest',soakSeconds,checks:[],screenshots:[],errors:[],network:[]};
let browser,page,server;
async function action(name){
  console.log('Action:',name);
  await page.waitForTimeout(150);
  for(let n=0;n<150;n++){
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(150);return;}
    await page.keyboard.press('Tab');await page.waitForTimeout(8);
  }throw Error('Unreachable '+name);
}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function ready(){await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');}
async function home(){await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");}
async function settings(){await action('Menu');await action('Settings');}
async function closeSettings(){await action('Back');await action('Continue');}
async function help(title){await action('Meet Pip');await action(title);await action('Help Pip');await page.waitForFunction(()=>pawsState.production&&!pawsState.garden);}
async function targets(){
  for(const b of await page.evaluate(()=>EngineGui.snapshot().widgets.filter(w=>w.type==='button'&&w.active)))assert(b.bounds.width>=44&&b.bounds.height>=44,'Small touch target');
}
async function captureMetrics(label,ms=6000){
  await page.waitForTimeout(ms);
  report.metrics??=[];report.metrics.push({label,...await page.evaluate(()=>({render:pawsRenderState,performance:pawsPerformance,quality:pawsQuality,audio:pawsAudio,wasmBytes:Module.HEAPU8.buffer.byteLength,jsHeap:performance.memory?{used:performance.memory.usedJSHeapSize,total:performance.memory.totalJSHeapSize}:null}))});
}
function distribution(values){values=values.filter(v=>Number.isFinite(v)&&v>0).sort((a,b)=>a-b);return {samples:values.length,mean:values.reduce((a,b)=>a+b,0)/Math.max(1,values.length),p50:values[Math.floor(values.length*.5)],p95:values[Math.floor(values.length*.95)],p99:values[Math.floor(values.length*.99)],max:values.at(-1)};}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  const mime={'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png','.json':'application/json','.ttf':'font/ttf','.svg':'image/svg+xml'};
  server=http.createServer(async(req,res)=>{try{
    const pathname=decodeURIComponent(new URL(req.url,url).pathname);
    if(!pathname.startsWith(prefix))throw Error('Unexpected prefix');
    const relative=pathname.slice(prefix.length)||'index.html',file=path.resolve(site,relative);
    if(!file.startsWith(site+path.sep))throw Error('Outside site');
    const bytes=await fs.readFile(file);res.setHeader('Content-Type',mime[path.extname(file)]||'application/octet-stream');
    let body=bytes;if(/\.(wasm|js|json|css|html|ttf|glb)$/.test(file)&&req.headers['accept-encoding']?.includes('gzip')){body=zlib.gzipSync(bytes);res.setHeader('Content-Encoding','gzip');res.setHeader('Vary','Accept-Encoding');}
    report.network.push({path:relative,rawBytes:bytes.length,bodyBytes:body.length});res.end(body);
  }catch{res.statusCode=404;res.end('Missing');}});
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
  report.build=JSON.parse(await fs.readFile(path.join(site,'build-info.json')));
  assert(!report.build.gallery&&report.build.configuration==='Release');
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  report.browser=browser.version();report.os=process.platform;report.device='host PC, headless Chrome; physical GPU model unavailable';
  const context=await browser.newContext({viewport:{width:1440,height:900},deviceScaleFactor:2,hasTouch:true});
  await context.addInitScript(()=>{
    const acquire=GPUAdapter.prototype.requestDevice;
    GPUAdapter.prototype.requestDevice=function(...args){const info=this.info;window.reviewAdapter={vendor:info.vendor,architecture:info.architecture,device:info.device,description:info.description};return acquire.apply(this,args);};
    window.reviewFrames=[];window.reviewSamples=[];let last=0,lastSample=0;
    function tick(now){if(last&&reviewFrames.length<100000)reviewFrames.push({ms:now-last,scene:window.pawsState?.scene||'startup'});last=now;
      if(now-lastSample>1000&&window.pawsPerformance){lastSample=now;reviewSamples.push({time:now,scene:pawsState.scene,perf:pawsPerformance,wasm:Module.HEAPU8.buffer.byteLength,audio:window.pawsAudio});}requestAnimationFrame(tick);}
    requestAnimationFrame(tick);
  });
  page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  const start=Date.now();await page.goto(url);await ready();const titleReadyMs=Date.now()-start;await shot('title-wide');const playStart=Date.now();await action('Play');await home();
  report.adapter=await page.evaluate(()=>window.reviewAdapter);
  report.startup={titleReadyMs,gardenReadyMsFromPlay:Date.now()-playStart,elapsedMs:Date.now()-start,requests:report.network.slice(),note:'Local gzip transfer, no bandwidth throttle; gardenReady includes 300ms input settling'};
  await captureMetrics('cold-garden');
  const unique=new Map(report.startup.requests.map(r=>[r.path,r]));report.startup.rawBytes=[...unique.values()].reduce((s,r)=>s+r.rawBytes,0);report.startup.gzipBodyBytes=[...unique.values()].reduce((s,r)=>s+r.bodyBytes,0);
  assert.equal(await page.evaluate(()=>pawsHostState().build),report.build.buildId);
  // Import the afternoon played by the separate fresh journey; then use only visible navigation.
  await action('Menu');await action('Save and recovery');const chooser=page.waitForEvent('filechooser');await action('Import save');
  await(await chooser).setFiles(path.resolve(process.env.PAWS_PLAYED_SAVE||'build/evidence/phase10/journey/played-afternoon.save'));
  await action('Use imported afternoon');await action('Continue');await home();
  await settings();
  if(await page.getByRole('button',{name:'Sound: On',exact:true}).count())await action('Sound: On');
  if(await page.getByRole('button',{name:'Motion: Full',exact:true}).count())await action('Motion: Full');
  if(await page.getByRole('button',{name:'Text size: Standard',exact:true}).count())await action('Text size: Standard');
  await action('Quality: Automatic');await page.waitForFunction(()=>pawsRenderState.maxShadowMaps===0);await shot('settings-larger-mobile');await closeSettings();
  await captureMetrics('mobile-shadow-free');const before=await page.evaluate(()=>({coins:pawsState.coins,attempt:pawsState.attempt,revision:pawsState.revision}));
  await settings();await action('Quality: Mobile');await page.waitForFunction(()=>pawsRenderState.maxPixels===2073600);await closeSettings();await captureMetrics('desktop');
  assert.deepEqual(await page.evaluate(()=>({coins:pawsState.coins,attempt:pawsState.attempt,revision:pawsState.revision})),before,'Quality changed rules');
  await page.reload();await ready();await action('Continue');await home();await page.waitForFunction(()=>pawsRenderState.maxPixels===2073600);report.checks.push('Quality switches preserve rule identities and explicit Desktop persists through reload');
  await settings();await action('Quality: High (2×)');await action('Quality: Automatic');await closeSettings();
  for(const [width,height]of[[1440,900],[390,844],[844,390]]){
    await page.setViewportSize({width,height});await page.waitForTimeout(500);await shot(`garden-${width}x${height}`);
    for(const title of ["Pip's missing ball · Practice",'Tidy the toy basket · Practice','The stuck little gate · Practice']){
      await help(title);await action('Menu');await action('Restart puzzle');await action('Restart puzzle');
      const tag=title.split(' ·')[0].replace(/[^a-z0-9]/gi,'-');await shot(`${tag}-${width}x${height}`);
      await page.locator('#viewport').evaluate(el=>el.style.filter='grayscale(1)');await shot(`${tag}-${width}x${height}-grayscale`);await page.locator('#viewport').evaluate(el=>el.style.filter='');
      await action(height<500?'Actions':'Objects & actions');await targets();await shot(`${tag}-${width}x${height}-actions`);
      // Modal gestures must not reach the orbit controller or authoritative state.
      const revision=await page.evaluate(()=>pawsState.revision);await page.mouse.move(width*.45,height*.4);await page.mouse.down();await page.mouse.move(width*.55,height*.5);await page.mouse.up();assert.equal(await page.evaluate(()=>pawsState.revision),revision);
      await action('Hint');await shot(`${tag}-${width}x${height}-hint`);await action('Back');await action('Back');
      await captureMetrics(`${tag}-${width}x${height}`,2000);await action('Menu');await action('Back to garden');await home();
    }
  }
  report.checks.push('Three boards, wide/portrait/short landscape, larger text, reduced motion, muted, grayscale, mobile/shadow-free profile, named actions/hints, >=44px controls, modal gesture isolation');
  const reviewed=page;
  for(const failure of ['asset','font','unsupported']){
    const isolated=await browser.newContext({viewport:{width:390,height:844}});page=await isolated.newPage();
    page.on('pageerror',e=>report.errors.push(e.message));
    page.on('console',m=>{if(m.type()==='error'){if(/503/.test(m.text()))(report.expectedErrors??=[]).push(m.text());else report.errors.push(m.text());}});
    if(failure==='unsupported')await page.addInitScript(()=>Object.defineProperty(navigator,'gpu',{value:undefined}));
    if(failure==='font')await page.route('**/assets/fonts/NunitoSans-Regular.ttf',r=>r.fulfill({status:503,body:'Injected missing font'}));
    if(failure==='asset')await page.route('**/assets/models/garden/paving.glb',r=>r.fulfill({status:503,body:'Injected missing model'}));
    await page.goto(url);
    if(failure==='asset'){
      await ready();const before=await page.evaluate(()=>({scene:pawsState.scene,attempt:pawsState.attempt,coins:pawsState.coins}));await action('Play');
      await page.waitForFunction(()=>!!window.pawsLoadError);assert.deepEqual(await page.evaluate(()=>({scene:pawsState.scene,attempt:pawsState.attempt,coins:pawsState.coins})),before);
      await shot('asset-failure-retains-progress');await page.unroute('**/assets/models/garden/paving.glb');await action('Retry loading');await home();
    } else {
      await page.waitForFunction(()=>!document.getElementById('retry').hidden);await shot(failure+'-startup-retry');assert(await page.locator('#startup').isVisible());
      const text=await page.locator('#status').innerText();assert(text.includes(failure==='font'?'lettering':'WebGPU'));
    }
    await isolated.close();page=reviewed;
  }
  report.checks.push('Packaged nested path: model HTTP failure preserves scene/attempt/coins and Retry succeeds; font failure and missing WebGPU expose readable startup Retry');
  await page.setViewportSize({width:1440,height:900});await settings();await shot('settings-persisted');await closeSettings();
  const soakStart=Date.now();const beforeSoak=await page.evaluate(()=>({wasmBytes:Module.HEAPU8.buffer.byteLength,counters:pawsPerformance.current}));let transitions=0;
  while((Date.now()-soakStart)/1000<soakSeconds){
    await help("Pip's missing ball · Practice");await action('Menu');await action('Back to garden');await home();transitions++;
    await captureMetrics(`soak-${transitions}`,Math.min(20000,Math.max(0,soakSeconds*1000-(Date.now()-soakStart))));
    console.log('Soak seconds:',Math.round((Date.now()-soakStart)/1000),'transitions:',transitions);
  }
  report.soak={elapsedSeconds:(Date.now()-soakStart)/1000,transitions,before:beforeSoak,after:await page.evaluate(()=>({wasmBytes:Module.HEAPU8.buffer.byteLength,counters:pawsPerformance.current})),scope:'Automated desktop transitions; no physical mobile or first-player pacing claim'};
  const samples=await page.evaluate(()=>({frames:reviewFrames,samples:reviewSamples}));await fs.writeFile(path.join(out,'performance-samples.json'),JSON.stringify(samples));
  report.frameIntervals=distribution(samples.frames.map(f=>f.ms));report.byScene={};
  for(const scene of new Set(samples.frames.map(f=>f.scene)))report.byScene[scene]=distribution(samples.frames.filter(f=>f.scene===scene).map(f=>f.ms));
  const counters=samples.samples.map(s=>s.perf.current);report.peaks={wasmBytes:Math.max(...samples.samples.map(s=>s.wasm)),trackedGpuBytes:Math.max(...counters.map(c=>c.gpuPayloadBytes+c.guiTextureBytes+c.guiBufferBytes)),cpuAssetBytes:Math.max(...counters.map(c=>c.cpuAssetBytes)),colorDraws:Math.max(...counters.map(c=>c.drawCalls-c.shadowDrawCalls-c.guiDrawCalls)),colorTriangles:Math.max(...counters.map(c=>c.triangles-c.shadowTriangles-c.guiTriangles)),audioPcmBytes:Math.max(...samples.samples.map(s=>s.audio?.peakPcmBytes||0))};
  report.budgets={startup15MiB:report.startup.gzipBodyBytes<=15*1024*1024,wasmCapacity192MiB:report.peaks.wasmBytes<=192*1024*1024,trackedGpu96MiB:report.peaks.trackedGpuBytes<=96*1024*1024,colorDraws100:report.peaks.colorDraws<=100,colorTriangles150000:report.peaks.colorTriangles<=150000,scope:'Desktop measurements compared to provisional mobile allocations; capacity is not working set, GPU payload is not driver/process memory'};
  report.limits=['RAF includes automation, font/GUI rebuilds and capture stalls; CPU updateMs includes Paws application, GUI and audio preparation; no full-process memory measurement','Final audio is absent: decode peak zero is not full-length audio acceptance','Independent first-player timing and physical devices are unavailable'];
  assert.equal(createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex'),report.build.wasmSha256);
  assert.equal(report.errors.length,0,report.errors.join('\n'));report.passed=true;await shot('final-garden');
})().catch(error=>{report.failure=error.stack;process.exitCode=1;}).finally(async()=>{try{await fs.writeFile(path.join(out,'report.json'),json(report));console.log(json(report));}finally{await browser?.close();await new Promise(resolve=>server?server.close(resolve):resolve());}});
