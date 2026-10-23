// Migration backup must commit before any schema-14 write replaces old bytes.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
(async()=>{
 const out=process.env.EVIDENCE_DIR||'build/evidence/m6/migration-storage';await fs.mkdir(out,{recursive:true});
 const report={runs:[]};
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
 try{for(const backend of ['webgpu','webgl2'])for(const version of [12,13]){
  const original=await fs.readFile(`tests/fixtures/quest-migration-${version===12?'m0':'m5'}/pip_missing_ball-partial.save`,'utf8');
  const context=await browser.newContext({acceptDownloads:true,viewport:{width:1440,height:1000}});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  const page=await context.newPage(),errors=[];page.on('pageerror',e=>errors.push(e.message));
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(180);};
  const read=key=>page.evaluate(async key=>{const db=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon',maxBytes:16*1024*1024});try{const bytes=await db.get(key);return bytes?new TextDecoder().decode(bytes):null;}finally{db.dispose();}},key);
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8151/');await ready();
  await page.evaluate(async text=>{const db=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon',maxBytes:16*1024*1024});await db.put('current',new TextEncoder().encode(text));db.dispose();},original);
  // Install before startup: slow renderer preparation may schedule its first save
  // before the ready marker is observed by the test runner.
  await page.addInitScript(()=>{let value;Object.defineProperty(globalThis,'EngineStorage',{configurable:true,get:()=>value,set:v=>{value=v;const prototype=v.IndexedDBStorage.prototype;window.m5OriginalPut=prototype.put;prototype.put=function(key,bytes){if(key==='pre-quest-m6')return Promise.reject(Object.assign(new Error('Controlled migration backup failure'),{code:'quota'}));return m5OriginalPut.call(this,key,bytes);};}});});
  await page.reload();await ready();assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
  await key('Continue');await page.waitForFunction(()=>pawsSaveDiagnostics().failed>0&&pawsState.save===3,null,{timeout:90000});
  assert.equal(await read('current'),original);assert.equal(await read('pre-quest-m6'),null);
  await page.evaluate(()=>{EngineStorage.IndexedDBStorage.prototype.put=m5OriginalPut;});
  await key('Menu');await key('Save and recovery');await key('Retry save');await page.waitForFunction(()=>pawsState.save===2);
  assert.equal(await read('pre-quest-m6'),original);assert.match(await read('current'),/^PAWS_AFTERNOON 14/);
  await page.reload();await ready();await key('Save and recovery');const download=page.waitForEvent('download');await key('Export original save');const file=path.join(out,`${backend}-v${version}-original.save`);await(await download).saveAs(file);assert.equal(await fs.readFile(file,'utf8'),original);
  assert.deepEqual(errors,[]);report.runs.push({backend,version,build:await page.evaluate(()=>pawsBuild),backupFailurePreservesCurrent:true,retryPreservesOriginal:true,originalExportAfterReload:true,errors});await context.close();
 }}finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));}
 console.log('M6 migration backup failure, retry and original export passed for v12/v13 on both renderers');
})().catch(e=>{console.error(e);process.exitCode=1;});
