// Replay the original garden picnic and later beach ending from separate validated saves.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
(async()=>{
 const out=process.env.EVIDENCE_DIR||'build/evidence/m6/endings';await fs.mkdir(out,{recursive:true});
 const fixtures=process.env.PAWS_QUEST_FIXTURES||'build/evidence/m6/native-rules';
 const original=await fs.readFile(path.join(fixtures,'garden-ending.save'),'utf8'),beach=await fs.readFile(path.join(fixtures,'ending.save'),'utf8'),report={runs:[]};
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});let activePage;
 try{for(const backend of ['webgpu','webgl2']){
  const context=await browser.newContext({viewport:{width:1440,height:1000}});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  const page=await context.newPage(),errors=[];activePage=page;page.on('pageerror',e=>errors.push(e.message));
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(250);};
  await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8151/')+'?devBeach=1');await ready();
  assert(await page.evaluate(text=>{Module._paws_cloud_stamp();return pawsStageCloudSave(new TextEncoder().encode(text),pawsCloudStamp);},original));
  await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});
  await page.waitForFunction(()=>pawsState.cats.every(c=>c.introduced),null,{timeout:90000});
  const gardenBefore=await page.evaluate(()=>pawsState);assert.equal(gardenBefore.coins,170);assert.equal(gardenBefore.items.filter(i=>i.owned&&i.spot).length,1);
  await key('Menu');await page.waitForFunction(()=>pawsUi.screen===5);await key('Replay picnic celebration');await page.waitForFunction(()=>pawsUi.screen===13,null,{timeout:90000});
  await page.screenshot({path:path.join(out,backend+'-garden-picnic.png')});await key('Continue in garden');
  const gardenAfter=await page.evaluate(()=>pawsState);assert.equal(gardenAfter.coins,gardenBefore.coins);assert.deepEqual(gardenAfter.items,gardenBefore.items);assert.equal(gardenAfter.picnicSeen,1);
  assert(await page.evaluate(text=>{Module._paws_cloud_stamp();return pawsStageCloudSave(new TextEncoder().encode(text),pawsCloudStamp);},beach));await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});const before=await page.evaluate(()=>pawsState);
  await key('Adventures');await key('Visit beach picnic');await page.waitForFunction(()=>pawsUi.screen===9,null,{timeout:90000});
  await key('Replay beach ending');await page.waitForFunction(()=>pawsUi.screen===21,null,{timeout:90000});
  await page.screenshot({path:path.join(out,backend+'-beach-ending.png')});await key('Back to garden');
  await page.evaluate(()=>pawsFlushSave());await page.waitForFunction(()=>pawsState.save===2);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});
  const after=await page.evaluate(()=>pawsState);assert.equal(after.coins,before.coins);assert.deepEqual(after.items,before.items);assert.deepEqual(after.cats,before.cats);assert.equal(after.picnicSeen,1);assert.equal(after.beachEndingSeen,1);assert.equal(after.sceneCount,1);assert.deepEqual(errors,[]);
  report.runs.push({backend,build:await page.evaluate(()=>pawsBuild),gardenReplay:true,beachReplay:true,gardenAfter,after,errors});await context.close();
 }}catch(error){report.failure=error.stack;report.diagnostics=await activePage?.evaluate(()=>({state:pawsState,ui:pawsUi,garden:window.pawsGarden,render:pawsRenderState,text:document.body.innerText}));await activePage?.screenshot({path:path.join(out,'failure.png')});throw error;}finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));}
 console.log('M6 garden and beach ending replay, retained furnishings and reload passed on both renderers');
})().catch(e=>{console.error(e);process.exitCode=1;});
