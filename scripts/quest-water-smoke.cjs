// Actual generated Paws site; fixtures come from the portable production owner.
const {chromium}=require('playwright'),{PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.WATER_BACKEND||'webgpu';
const out=path.resolve(process.env.EVIDENCE_DIR||`build/evidence/quest-water/${backend}`);
const url=process.env.PAWS_URL||'http://127.0.0.1:8156/';
const report={backend,checks:[],screenshots:[],errors:[]};
const record=report.checks.push.bind(report.checks);report.checks.push=(...checks)=>{console.log(...checks);return record(...checks);};
let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 else await context.addInitScript(()=>{const original=GPUAdapter.prototype.requestDevice;GPUAdapter.prototype.requestDevice=async function(...args){const device=await original.apply(this,args);window.waterTestDevice=device;return device;};});
 page=await context.newPage();page.setDefaultTimeout(90000);
 page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 const state=()=>page.evaluate(()=>pawsState);
 const water=()=>page.evaluate(()=>{Module._paws_water_snapshot();return pawsWater;});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(140);}
 async function click(id){const p=await page.evaluate(id=>{Module._paws_interaction_snapshot();return pawsInteraction.objects[id];},id);assert(p,id);await page.mouse.click(p.x,p.y);await page.waitForTimeout(150);}
 async function shot(name){const file=path.join(out,name+'.png');await page.screenshot({path:file});report.screenshots.push(file);return PNG.sync.read(await fs.readFile(file));}
 async function fixture(name){
  if(name==='picnic')await page.waitForFunction(()=>pawsState.save===2&&!pawsSaveDiagnostics().pending);
  const text=await fs.readFile(`build/evidence/quest-water/fixtures/${name}.save`,'utf8');
  await page.evaluate(async text=>{const storage=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{
   // Recovery chooses the newest valid revision across current and history.
   for(let n=0;n<5;n++)await storage.remove('history-'+n);
   await storage.put('current',new TextEncoder().encode(text));
  }finally{storage.dispose();}},text);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>!pawsState.garden&&pawsUi.screen===2);
 }
 function changed(a,b,p){let n=0;for(let y=Math.round(p.y)-8;y<=Math.round(p.y)+8;y++)for(let x=Math.round(p.x)-8;x<=Math.round(p.x)+8;x++){if(x<0||x>=a.width||y<0||y>=a.height)continue;const i=(y*a.width+x)*4;if(Math.abs(a.data[i]-b.data[i])+Math.abs(a.data[i+1]-b.data[i+1])+Math.abs(a.data[i+2]-b.data[i+2])>3)n++;}return n;}
 await page.goto(url);await ready();
 for(const file of ['index.html','paws.js','paws.wasm'])assert((await page.request.get(new URL(file,url).href)).ok(),file+' is served');
 await fixture('water');assert.equal((await state()).quest,'mabel_drinking_route');assert((await water()).shader);assert.equal((await water()).surfaces.length,0);
 await click('rain_barrel');assert(!(await state()).completed);assert.equal((await water()).surfaces.length,0);
 report.checks.push('Invalid route stays dry');
 const dry=await shot('drinking-dry');
 const solve=async()=>{for(const step of (await state()).solution)if(step.verb===2)await click(step.object);else if(step.verb===3)await click('rain_barrel');};
 await solve();await page.waitForFunction(()=>pawsState.completed&&pawsEnding.time>=1.8);
 const wetState=await water();assert.equal(wetState.surfaces.filter(s=>s.name==='Validated water surface').length,5);assert(wetState.surfaces.some(s=>s.name==='Filled bowl'));
 const wet=await shot('drinking-filled');report.fillPixels=wetState.surfaces.filter(s=>s.name==='Validated water surface').map(p=>changed(dry,wet,p));assert(report.fillPixels.some(n=>n>5),'Filled water changes actual gutter pixels');
 await key('Menu');const held=await water();await page.waitForTimeout(500);assert.equal((await water()).time,held.time);
 await page.evaluate(()=>Module._paws_suspend(1));await page.waitForTimeout(200);assert.equal((await water()).time,held.time);await page.evaluate(()=>Module._paws_suspend(0));
 await page.waitForTimeout(200);assert.equal((await water()).time,held.time);
 if(backend==='webgpu'){const generation=await page.evaluate(()=>pawsRenderState.generation);await page.evaluate(()=>waterTestDevice.destroy());await page.waitForFunction(g=>pawsRenderState.state==='ready'&&pawsRenderState.generation>g,generation);assert.equal((await water()).time,held.time);assert((await water()).surfaces.some(s=>s.name==='Filled bowl'));}
 await key('Undo');assert(!(await state()).completed);assert.equal((await water()).surfaces.length,0);await key('Continue');
 report.checks.push(backend==='webgpu'?'Pause, suspension, GPU recovery and paused Undo preserve water ownership':'Pause, suspension and paused Undo preserve water ownership');
 await click('rain_barrel');await page.waitForFunction(()=>pawsState.completed&&pawsEnding.time>=1.8);
 const p=(await water()).surfaces.find(s=>s.name==='Filled bowl');const a=await shot('ripples-a');await page.waitForTimeout(700);const b=await shot('ripples-b');report.ripplePixels=changed(a,b,p);assert(report.ripplePixels>0,'Shader ripples animate actual water pixels');
 await key('Menu');await key('Restart puzzle');await key('Restart puzzle');assert(!(await state()).completed);assert.equal((await water()).surfaces.length,0);
 if(await page.getByRole('button',{name:'Continue',exact:true}).count())await key('Continue');
 await solve();await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9);await page.waitForFunction(()=>pawsState.save===2);
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 await key('Menu');await key('Requests & friends');
 for(;;){const welcome=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')).find(n=>n.startsWith('Welcome ')));if(!welcome)break;await key(welcome);}
 await key('A drink after playing · Practice');await page.waitForFunction(()=>!pawsState.garden&&pawsState.quest==='mabel_drinking_route'&&pawsUi.screen===3);await key('Help Mabel');
 await page.waitForFunction(()=>pawsUi.screen===2);assert(!(await state()).completed);assert.equal((await water()).surfaces.length,0);assert.equal((await state()).coins,140);await shot('drinking-restored-practice');
 report.checks.push('Animated water pixels, restart, saved reward and fresh dry practice after reload');
 await fixture('picnic');assert.equal((await state()).quest,'garden_picnic');assert.equal((await water()).surfaces.filter(s=>s.name==='Filled bowl').length,1);
 for(const [width,height] of [[1440,900],[390,844],[844,390]]){await page.setViewportSize({width,height});await page.waitForTimeout(500);assert.equal((await water()).surfaces.length,1);await shot(`picnic-${width}x${height}`);}
 await key('Menu');await key('Settings');const reduced=page.getByRole('button',{name:'Motion: Full',exact:true});if(await reduced.count())await key('Motion: Full');await key('Back');await key('Continue');assert.equal((await water()).time,0);await page.waitForTimeout(350);assert.equal((await water()).time,0);
 report.checks.push('Picnic bowl, portrait/landscape and reduced-motion still water');
 report.render=await page.evaluate(()=>pawsRenderState);report.build=await page.evaluate(()=>pawsBuild);assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,render:window.pawsRenderState,water:window.pawsWater,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();console.log(JSON.stringify({passed:report.passed,checks:report.checks,fillPixels:report.fillPixels,ripplePixels:report.ripplePixels,failure:report.failure},null,2));});
