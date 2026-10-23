// Actual file chooser/download interchange between the generated site and Windows.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {promisify}=require('node:util'),run=promisify(require('node:child_process').execFile);
(async()=>{
 const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/m4/interchange');await fs.mkdir(out,{recursive:true});
 const input=path.resolve(process.env.PAWS_NATIVE_SAVE||'build/evidence/m4/windows-final-resume/afternoon.save');
 const exe=path.resolve(process.env.PAWS_NATIVE||'build/m4-windows/package/Release/paws_windows.exe');
 const report={runs:[]},stamp=Date.now();
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576','--media-cache-size=1048576']});
 try {for(const backend of ['webgpu','webgl2']){
  const context=await browser.newContext({viewport:{width:1440,height:1000},acceptDownloads:true});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  const page=await context.newPage(),errors=[];page.on('pageerror',e=>errors.push(e.message));
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(200);};
  const state=()=>page.evaluate(()=>pawsState);
  await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8148/')+'?devBeach=1');await ready();
  assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
  await key('Save and recovery');const chooser=page.waitForEvent('filechooser');await key('Import save');await(await chooser).setFiles(input);
  await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsState.production&&pawsState.save===2,null,{timeout:90000});
  const imported=await state(),expectedCoins=Number(process.env.PAWS_EXPECT_COINS||20);assert.equal(imported.coins,expectedCoins);assert.equal(imported.completed,1);
  if(process.env.PAWS_RICH_SAVE){assert.equal(imported.items.filter(i=>i.owned&&i.spot).length,8);assert.equal(imported.picnicSeen,1);assert.equal(imported.beachEndingSeen,1);}
  await page.screenshot({path:path.join(out,backend+'-garden.png')});
  await key('Menu');await key('Save and recovery');const download=page.waitForEvent('download');await key('Export save');
  const exported=path.join(out,backend+'.save');await(await download).saveAs(exported);
  assert.match(await fs.readFile(exported,'utf8'),/^PAWS_AFTERNOON 14/);
  const returned=path.join(out,backend+'-native');await run(exe,['--dev-beach','--user-directory',path.join(out,`profile-${backend}-${stamp}`),'--import',exported,'--smoke','import',returned],{windowsHide:true,timeout:180000});
  const native=JSON.parse(await fs.readFile(path.join(returned,'state.json'),'utf8'));assert.equal(native.coins,imported.coins);assert.equal(native.completed,imported.completed);
  assert.deepEqual(native.cats.map(c=>[c.id,c.friendship]),imported.cats.map(c=>[c.id,c.friendship]));
  assert.deepEqual(native.items,imported.items);assert.equal(native.picnicSeen,imported.picnicSeen);assert.equal(native.beachEndingSeen,imported.beachEndingSeen);
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.production,null,{timeout:90000});assert.equal((await state()).coins,expectedCoins);
  assert.deepEqual(errors,[]);report.runs.push({backend,build:await page.evaluate(()=>pawsBuild),imported,native,errors});await context.close();
 }}finally{await browser.close();}
 await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log('Paws web/Windows file interchange passed on both browser renderers');
})().catch(e=>{console.error(e);process.exitCode=1;});
