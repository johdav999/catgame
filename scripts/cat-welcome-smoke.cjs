// Actual staged game, isolated saves, both render backends, exact HTTP asset hashes.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),{createHash}=require('node:crypto');
(async()=>{
 const backend=process.env.WEBGL==='1'?'webgl2':'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/cat-welcome/${backend}`;
 await fs.mkdir(out,{recursive:true});const report={backend,errors:[],loaded:[],checks:[]};
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1600,height:1000},...(process.env.GAIT_VIDEO?{recordVideo:{dir:out,size:{width:1280,height:800}}}:{})});page.setDefaultTimeout(60000);
 page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const assets=JSON.parse(await fs.readFile('docs/release1/fragments/cats.json','utf8')).assets;const pending=[];let capture=true;
 page.on('response',r=>{if(!capture)return;const a=assets.find(a=>r.url().endsWith(a.url));if(a)pending.push((async()=>{assert.equal(r.status(),200);const sha=createHash('sha256').update(await r.body()).digest('hex');assert.equal(sha,a.sha256);report.loaded.push({cat:a.cat,url:a.url,sha256:sha});})());});
 if(backend==='webgl2')await page.addInitScript(()=>Object.defineProperty(navigator.gpu,'requestAdapter',{value:async()=>null}));
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(200);}
 try{
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8120/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  await key('Save and recovery');const chooser=page.waitForEvent('filechooser');await key('Import save');await(await chooser).setFiles(path.resolve('build/evidence/toon-cats/fixtures/picnic.save'));await key('Use imported afternoon');await key('Play');
  await page.waitForFunction(()=>window.pawsLoadError||pawsState.garden&&pawsState.production);assert.equal(await page.evaluate(()=>window.pawsLoadError),undefined);
  if(await page.getByRole('button',{name:'Continue',exact:true}).count())await key('Continue');
  await page.waitForFunction(()=>pawsGarden.some(c=>c.phase===17));
  await page.waitForTimeout(1200);await page.screenshot({path:out+'/four-cats-garden.png'});
  assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);await Promise.all(pending);assert.equal(new Set(report.loaded.map(a=>a.cat)).size,4);
  report.checks.push('All four selected GLBs loaded with exact catalogue hashes','Resident begins welcoming the queued newcomers');
  const seen=new Set();let finished=false,paused=false;
  for(let n=0;n<600&&!finished;++n){
   const cats=await page.evaluate(()=>pawsGarden);
   for(const c of cats)if([17,18,19].includes(c.phase)&&!seen.has(c.phase)){seen.add(c.phase);await page.screenshot({path:out+'/phase-'+c.phase+'.png'});}
   if(!paused&&cats.some(c=>c.phase===18)){
    await key('Menu');const frozen=await page.evaluate(()=>pawsGarden);await page.waitForTimeout(600);assert.deepEqual(await page.evaluate(()=>pawsGarden),frozen);await key('Continue');paused=true;
   }
   finished=seen.has(19)&&cats[2].phase===0&&cats[3].phase===0;
   if(finished){for(const i of [2,3])assert(Math.hypot(cats[i].x-(-1.8+1.2*i),cats[i].z+3.6)<.02,'Cat did not reach its own home');}
   await page.waitForTimeout(500);
  }
  assert.deepEqual([...seen].sort(),[17,18,19]);assert(finished&&paused,'Welcome did not finish');
  await page.waitForFunction(()=>pawsState.cats[3].introduced&&pawsState.save===2);
  await page.screenshot({path:out+'/home.png'});
  capture=false;await Promise.all(pending);await page.reload();await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady);await key('Continue');
  await page.waitForFunction(()=>window.pawsState?.garden&&window.pawsGarden?.length===4&&window.pawsUi?.screen===1&&window.pawsRenderState?.state==='ready');await page.waitForTimeout(1200);
  assert(!(await page.evaluate(()=>pawsGarden)).some(c=>c.phase>=17));
  report.checks.push('Resident walks out to meet newcomer','Greeting poses and paired return observed','Pause freezes greeting','Both cats return home; introduction saves and does not replay on reload');
  assert.deepEqual(report.errors,[]);report.build=await page.evaluate(()=>pawsBuild);report.passed=true;
 }catch(e){report.failure=e.stack;report.diagnostics=await page.evaluate(()=>({load:window.pawsLoadError,state:window.pawsState,garden:window.pawsGarden,text:document.body.innerText}));await page.screenshot({path:out+'/failure.png'});process.exitCode=1;}
 finally{await browser.close();await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}
})().catch(e=>{console.error(e);process.exitCode=1});
