const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p04-pinned/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-04/browser');
const port=Number(process.env.PAWS_TEST_PORT||19614);
const report={site,checks:[],errors:[],screenshots:[],payload:{}};
let browser,server;
const key=x=>JSON.stringify(x);
async function state(page){return page.evaluate(()=>window.rulesView);}
async function button(page,locator,mode){
  if(mode==='keyboard'){
    const index=await locator.getAttribute('data-action-index');
    if(index!==null){
      for(let n=0;n<150;n++){
        if(await page.evaluate(i=>document.activeElement?.dataset.actionIndex===i,index))break;
        await page.keyboard.press('Tab');
      }
      assert.equal(await page.evaluate(()=>document.activeElement?.dataset.actionIndex),index,'action reachable with Tab');
    }else await locator.focus();
    await page.keyboard.press('Enter');
  }else if(mode==='touch')await locator.tap();else await locator.click();
}
async function command(page,s,mode='mouse'){
  let v=await state(page);const index=v.actions.findIndex(a=>key(a.intent)===key(s));
  assert(index>=0,`Action exists: ${key(s)}`);assert(v.actions[index].enabled,v.actions[index].reason);
  if(s.object){
    if(mode==='keyboard'){
      const objects=page.locator('#objects');await objects.focus();await page.keyboard.press('Home');
      const position=await objects.evaluate((select,id)=>Array.from(select.options).findIndex(o=>o.value===id),s.object);
      assert(position>=0,'named object keyboard option');for(let i=0;i<position;i++)await page.keyboard.press('ArrowDown');
      await page.keyboard.press('Tab');
    }else await page.locator('#objects').selectOption(s.object);
  }
  const prior=v.revision;
  await button(page,page.locator(`[data-action-index="${index}"]`),mode);
  v=await state(page);assert(v.revision>prior,'command commits');assert(v.accepted,v.feedback);return v;
}
async function named(page,name,mode='mouse'){
  const v=await state(page);const a=v.actions.find(a=>a.label===name);assert(a,`missing ${name}`);return command(page,a.intent,mode);
}
async function solve(page,mode){const v=await state(page);for(const s of v.trace)await command(page,s,mode);assert((await state(page)).completed);}
async function shot(page,name){await page.screenshot({path:path.join(out,name+'.png'),fullPage:true});report.screenshots.push(name+'.png');}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  for(const name of ['release1_preview.wasm','release1_preview.js','index.html','debug.js','debug.css']){
    const bytes=await fs.readFile(path.join(site,name));report.payload[name]={bytes:bytes.length,sha256:createHash('sha256').update(bytes).digest('hex')};
  }
  server=http.createServer(async(req,res)=>{try{
    const url=new URL(req.url,'http://localhost');const file=path.resolve(site,'.'+decodeURIComponent(url.pathname==='/'?'/index.html':url.pathname));
    if(!file.startsWith(site+path.sep))throw Error('outside site');const bytes=await fs.readFile(file);
    res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css'})[path.extname(file)]||'application/octet-stream');res.end(bytes);
  }catch{res.statusCode=404;res.end('missing');}});
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
  for(const [width,height,mode] of [[1440,900,'keyboard'],[390,844,'touch'],[844,390,'mouse']]){
    const context=await browser.newContext({viewport:{width,height},hasTouch:mode==='touch',reducedMotion:'reduce'});
    const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
    await page.goto(`http://127.0.0.1:${port}`);await page.waitForFunction(()=>window.rulesView&&window.rulesRender?.state==='ready');
    await page.locator('#large').check();
    for(let n=0;n<10;n++){
      if(mode==='touch'&&n<3)continue;
      await page.locator('#quests').selectOption(String(n));let v=await state(page);const quest=v.quest;
      await named(page,'Free hint',mode);await named(page,'Free hint',mode);await named(page,'Free hint',mode);assert.equal((await state(page)).hints.length,3);
      await solve(page,mode);await named(page,'Undo',mode);assert(!(await state(page)).completed);
      await command(page,v.trace.at(-1),mode);assert((await state(page)).completed);
      await shot(page,`${width}x${height}-${quest}`);
      await named(page,'Restart / clear sequence',mode);assert(!(await state(page)).completed);await solve(page,mode);
      assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),'no horizontal clipping');
      report.checks.push(`${width}x${height}: ${quest} ${mode} completion, three free hints, undo, restart, replay`);
      console.log(report.checks.at(-1));
    }
    // Real wrong-bell commit clears the row and is undoable.
    await page.locator('#quests').selectOption('7');await named(page,'Restart / clear sequence',mode);
    let v=await state(page);await command(page,v.trace[0],mode);
    v=await state(page);await command(page,v.actions.find(a=>a.intent.object==='paw'&&a.label==='Tap Paw bell').intent,mode);
    assert.deepEqual((await state(page)).entered,[]);await named(page,'Undo',mode);assert.deepEqual((await state(page)).entered,['leaf']);
    const stale=await state(page);await named(page,'Free hint',mode);
    const hi=stale.actions.findIndex(a=>a.label==='Free hint');const rev=(await state(page)).revision;
    await page.evaluate(({hi,stale})=>Module._rules_action(hi,stale.revision,stale.attempt,stale.generation),{hi,stale});
    assert.equal((await state(page)).revision,rev);assert(!(await state(page)).accepted);
    await button(page,page.locator('#pause'),mode);assert((await state(page)).paused);assert((await state(page)).actions.every(a=>!a.enabled));
    await page.evaluate(()=>{Module._rules_suspend(1);Module._rules_suspend(0);});assert((await state(page)).paused,'hidden resume preserves explicit pause');
    await button(page,page.locator('#pause'),mode);assert(!(await state(page)).paused);
    report.checks.push(`${width}x${height}: visible sequence reset/undo, stale command, pause and suspension`);
    await shot(page,`${width}x${height}-bell-reset`);
    // Repeated projections replace temporary actors rather than retaining meshes.
    for(let i=0;i<40;i++)await page.locator('#quests').selectOption(String(i%10));
    await page.waitForTimeout(300);assert((await page.evaluate(()=>window.rulesGraphics.meshes))<160,'temporary scene resources remain bounded');
    await context.close();
  }
  assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(e=>{report.failure=e.stack;console.error(e);process.exitCode=1;}).finally(async()=>{
  await fs.mkdir(out,{recursive:true});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));
  if(browser)await browser.close();if(server)await new Promise(resolve=>server.close(resolve));
});
