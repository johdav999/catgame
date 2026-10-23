// Isolated browser profile: real IndexedDB connection failure and shipped recovery UI.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
const base=process.env.PAWS_URL||'http://127.0.0.1:8080/';
const backend=process.env.QUEST_BACKEND||'webgpu';
const out=process.env.EVIDENCE_DIR||`build/evidence/save-recovery/${backend}`;
const report={checks:[],errors:[]};let browser;
(async()=>{try{
  await fs.mkdir(out,{recursive:true});
  browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
  const context=await browser.newContext({viewport:{width:1440,height:1000}});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const saved=()=>page.waitForFunction(()=>pawsState.save===2);
  const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(180);};
  const read=()=>page.evaluate(async()=>{const s=new EngineStorage.IndexedDBStorage({namespace:pawsSaveDiagnostics().namespace});try{return new TextDecoder().decode(await s.get('current'));}finally{s.dispose();}});
  await page.goto(base);await ready();
  report.build=await page.evaluate(()=>pawsBuild);report.renderer=await page.evaluate(()=>pawsRenderState.backend);assert.equal(report.renderer,backend);
  // Capture only this isolated game's next adapter, using the public constructor.
  await page.evaluate(()=>{const Base=EngineStorage.IndexedDBStorage;window.testAdapters=[];EngineStorage.IndexedDBStorage=class extends Base{constructor(options){super(options);testAdapters.push(this);}};});
  await key('Play');await saved();await page.evaluate(()=>pawsRetrySave());await saved();
  if(process.env.SAVE_FIXTURE){
    const fixture=await fs.readFile(process.env.SAVE_FIXTURE,'utf8');
    await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:pawsSaveDiagnostics().namespace});await s.put('current',new TextEncoder().encode(text));s.dispose();},fixture);
    await page.reload();await ready();await key('Continue');await saved();
    await page.evaluate(()=>{const Base=EngineStorage.IndexedDBStorage;window.testAdapters=[];EngineStorage.IndexedDBStorage=class extends Base{constructor(o){super(o);testAdapters.push(this);}};return pawsRetrySave();});await saved();
  }
  const progress=()=>page.evaluate(()=>({coins:pawsState.coins,friendship:pawsState.cats.map(c=>({id:c.id,friendship:c.friendship})),beachProgress:pawsState.beachProgress}));
  const before=await progress();
  await page.getByRole('button',{name:'Play',exact:true}).waitFor({state:'hidden',timeout:90000});
  await page.getByRole('button',{name:/^(Menu|Save and recovery)$/}).first().waitFor({state:'visible',timeout:90000});
  if(await page.getByRole('button',{name:'Menu',exact:true}).count())await key('Menu');
  await key('Save and recovery');
  await saved();const durable=await read();
  // Force the cached game's database closed; a second put on it cannot reconnect.
  await page.evaluate(()=>{testAdapters[0].database.close();Module._paws_retry_save();});
  await page.waitForFunction(()=>pawsState.save===3);
  assert.equal(await read(),durable);
  report.closedError=await page.evaluate(()=>pawsSaveDiagnostics().lastWriteError);
  assert.equal(report.closedError.name,'InvalidStateError');
  await page.waitForFunction(()=>document.querySelector('#engine-gui-accessibility').innerText.includes('reconnect'));
  await page.screenshot({path:out+'/closed-connection.png'});
  await key('Retry save');await saved();
  assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().lastWriteError),null);
  report.checks.push('Closed cached IndexedDB connection fails without changing current; Retry reconnects and commits through the UI.');
  // Quota failures remain honest until writes succeed, and keep last complete bytes.
  const recovered=await read();
  await page.evaluate(()=>{window.testPut=EngineStorage.IndexedDBStorage.prototype.put;EngineStorage.IndexedDBStorage.prototype.put=()=>Promise.reject(new EngineStorage.StorageError('quota','Injected disk quota',new DOMException('full','QuotaExceededError')));Module._paws_retry_save();});
  await page.waitForFunction(()=>pawsState.save===3);
  assert.equal(await read(),recovered);
  await page.waitForFunction(()=>document.querySelector('#engine-gui-accessibility').innerText.includes('Free some disk space'));
  await key('Retry save');await page.waitForFunction(()=>pawsState.save===3);
  await page.evaluate(()=>EngineStorage.IndexedDBStorage.prototype.put=testPut);
  await key('Retry save');await saved();
  report.checks.push('Persistent quota failure reports disk-space guidance; successful retry clears the error without deleting records.');
  // Debounced work is drained before disposal and writes arriving during retry
  // use the new adapter. The final snapshot remains authoritative.
  await page.evaluate(async()=>{Module._paws_cloud_snapshot();const text=pawsCloudSnapshot;delete window.pawsCloudSnapshot;pawsStoreSave(text,900001,false);const retry=pawsRetrySave();pawsStoreSave(text,900002,true);await retry;await pawsFlushSave();});
  assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().lastWriteError),null);
  await page.evaluate(()=>Module._paws_retry_save());await saved();
  report.checks.push('Pending debounce and writes arriving during recovery drain without using a disposed adapter.');
  await page.reload();await ready();await key('Continue');
  assert.deepEqual(await progress(),before);
  await saved();await page.screenshot({path:out+'/restored.png'});
  report.progress=before;report.checks.push('Reload retains coins, friendship and completed beach quests.');
  // Second tab cannot use Retry to take ownership or overwrite the main save.
  const second=await context.newPage();await second.goto(base);await second.waitForFunction(()=>window.pawsStorageReady);
  assert.equal(await second.evaluate(()=>pawsSaveDiagnostics().mode),1);
  const primary=await read();await second.evaluate(()=>pawsRetrySave());assert.equal(await read(),primary);
  report.checks.push('Retry in a second tab preserves the primary writer and its bytes.');
  assert.deepEqual(report.errors,[]);report.ok=true;
}catch(error){report.ok=false;report.failure=error.stack;process.exitCode=1;}finally{
  await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();
}})();
