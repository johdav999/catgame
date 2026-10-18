// Actual staged game, isolated saves, both render backends, exact HTTP asset hashes.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),{createHash}=require('node:crypto');
(async()=>{
 const backend=process.env.WEBGL==='1'?'webgl2':'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/toon-cats/${backend}`;
 await fs.mkdir(out,{recursive:true});const report={backend,errors:[],loaded:[],checks:[]};
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1600,height:1000},...(process.env.GAIT_VIDEO?{recordVideo:{dir:out,size:{width:1280,height:800}}}:{})});page.setDefaultTimeout(60000);
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const assets=JSON.parse(await fs.readFile('docs/release1/fragments/cats.json','utf8')).assets;const pending=[];
 page.on('response',r=>{const a=assets.find(a=>r.url().endsWith(a.url));if(a)pending.push((async()=>{assert.equal(r.status(),200);const sha=createHash('sha256').update(await r.body()).digest('hex');assert.equal(sha,a.sha256);report.loaded.push({cat:a.cat,url:a.url,sha256:sha});})());});
 if(backend==='webgl2')await page.addInitScript(()=>Object.defineProperty(navigator.gpu,'requestAdapter',{value:async()=>null}));
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(200);}
 try{
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8120/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  await key('Save and recovery');const chooser=page.waitForEvent('filechooser');await key('Import save');await(await chooser).setFiles(path.resolve('build/evidence/toon-cats/fixtures/picnic.save'));await key('Use imported afternoon');await key('Play');
  await page.waitForFunction(()=>window.pawsLoadError||pawsState.garden&&pawsState.production);assert.equal(await page.evaluate(()=>window.pawsLoadError),undefined);
  if(await page.getByRole('button',{name:'Continue',exact:true}).count())await key('Continue');
  await page.waitForFunction(()=>pawsGarden.every(c=>c.visible));
  await page.waitForTimeout(1200);await page.screenshot({path:out+'/four-cats-garden.png'});
  assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);await Promise.all(pending);assert.equal(new Set(report.loaded.map(a=>a.cat)).size,4);
  report.checks.push('All four selected GLBs loaded with exact catalogue hashes','Four cats visible in the actual garden');
  await key('Menu');await key('Shop');
  await key('Garden cushion · 30 coins');await key('Buy');await key('Choose a spot');await key('bench corner · Fits');await key('Place');
  if(process.env.GAIT_CHECK){
   await page.waitForFunction(()=>pawsGarden.some(c=>c.item==='garden_cushion'&&c.phase===2));
   const samples=await page.evaluate(()=>new Promise(resolve=>{
    const index=pawsGarden.findIndex(c=>c.item==='garden_cushion'),samples=[],start=pawsLifecycle.simulationElapsed;
    function frame(){const c=pawsGarden[index];samples.push({time:pawsLifecycle.simulationElapsed,phase:c.phase,x:c.x,z:c.z});
     if(pawsLifecycle.simulationElapsed-start>=2.3||c.phase!==2)resolve(samples);else requestAnimationFrame(frame);}
    frame();
   }));
   const speeds=samples.slice(1).flatMap((s,i)=>{const p=samples[i],dt=s.time-p.time,d=Math.hypot(s.x-p.x,s.z-p.z);return dt>0&&d>.001&&s.phase===2?[d/dt]:[];}).sort((a,b)=>a-b);
   assert(speeds.length>10,'No sustained cat travel observed');const median=speeds[Math.floor(speeds.length/2)];assert(median>.43&&median<.47,`Unexpected travel speed ${median}`);
   report.gait={medianWorldSpeed:median,samples};report.checks.push('Actual walking remains 0.45 m/s with retimed assets');await page.screenshot({path:out+'/walking.png'});
  }
  await page.waitForFunction(()=>pawsGarden.some(c=>c.item==='garden_cushion'&&c.phase===4),undefined,{timeout:120000});await page.screenshot({path:out+'/cushion-use.png'});
  report.checks.push('New cat reaches cushion and enters its mapped resting animation');
  assert.deepEqual(report.errors,[]);report.build=await page.evaluate(()=>pawsBuild);report.passed=true;
 }catch(e){report.failure=e.stack;report.diagnostics=await page.evaluate(()=>({load:window.pawsLoadError,state:window.pawsState,garden:window.pawsGarden,text:document.body.innerText}));await page.screenshot({path:out+'/failure.png'});process.exitCode=1;}
 finally{await browser.close();await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}
})().catch(e=>{console.error(e);process.exitCode=1});
