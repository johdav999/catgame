const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/automatic-quality';
const expectMsaa=process.env.EXPECT_MSAA==='1';
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={checks:[],errors:[]};
 try {
  for(const [name,width,height,dpr] of [['desktop',1485,965,1],['retina',1485,965,2],['portrait',390,844,3],['landscape',844,390,2]]) {
   const page=await browser.newPage({viewport:{width,height},deviceScaleFactor:dpr});
   page.on('pageerror',e=>report.errors.push(e.message));
   page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
   async function ready(){await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});}
   async function key(label){
    for(let n=0;n<150;n++){
     if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===label){await page.keyboard.press('Enter');await page.waitForTimeout(200);return;}
     await page.keyboard.press('Tab');
    }throw Error('Unreachable '+label);
   }
   async function sample(){return page.evaluate(()=>({quality:pawsQuality,render:pawsRenderState,canvas:[Module.canvas.width,Module.canvas.height],widgets:EngineGui.snapshot().widgets.map(w=>({...w,name:document.querySelector(`[data-widget-id="${w.id}"]`)?.getAttribute('aria-label')}))}));}
   async function msaa(samples){
    if(!expectMsaa)return;
    await page.waitForFunction(n=>pawsQuality.msaaSamples===n&&pawsRenderState.msaaSamples===n&&pawsRenderState.effectiveMsaaSamples===n,samples);
    assert.equal((await sample()).render.msaa4x,true);
   }
   async function fps(){
    await page.waitForFunction(()=>EngineGui.snapshot().widgets.some(w=>/^FPS: \d+$/.test(w.text)),null,{timeout:20000});
    const state=await sample(),label=state.widgets.find(w=>/^FPS: \d+$/.test(w.text));
    const menu=state.widgets.find(w=>w.type==='button'&&w.name==='Menu');
    assert(label&&menu,'Menu and FPS are visible');
    assert(label.bounds.y>=menu.bounds.y+menu.bounds.height,'FPS below Menu');
    assert(label.bounds.x+label.bounds.width<=width&&label.bounds.y+label.bounds.height<=height,'FPS fits viewport');
    assert.equal(label.lineCount,1);
    assert(label.clip.height>=label.bounds.height,'FPS line is not clipped');
    await msaa(state.quality.profile==='desktop'?4:1);
    return {quality:state.quality,render:state.render,canvas:state.canvas,fps:label};
   }
   await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8118');await ready();
   assert.deepEqual((await sample()).canvas,[width,height]);
   assert.equal((await sample()).quality.maxShadowMaps,1);
   await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
   const automatic=await fps();
   await page.screenshot({path:path.join(out,name+'.png'),scale:'css'});
   if(name==='desktop'||name==='retina') {
    await key('Menu');await key('Settings');await key('Quality: Automatic');
    await page.waitForFunction(()=>pawsQuality.profile==='mobile');
    await msaa(1);
    assert.equal((await sample()).quality.maxShadowMaps,0);
    await key('Quality: Mobile');await page.waitForFunction(()=>pawsQuality.profile==='desktop');
    await page.waitForFunction(()=>Module.canvas.width===2970&&Module.canvas.height===1930);
    await msaa(4);
    const high=await sample();assert.equal(high.quality.maxShadowMaps,1);
    assert.equal(automatic.canvas[0]*automatic.canvas[1]/(high.canvas[0]*high.canvas[1]),0.25);
    await key('Back');await key('Continue');await fps();
    await page.reload();await ready();await page.waitForFunction(()=>pawsQuality.profile==='desktop');
    await key('Settings');await key('Quality: High (2×)');
    await page.waitForFunction(()=>pawsQuality.profile==='automatic'&&Module.canvas.width===1485);
    await key('Back');await key('Continue');await fps();
    await key('Menu');await key('Requests & friends');
    for(let i=0;i<4;i++){
     const welcome=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')).find(n=>n?.startsWith('Welcome ')));
     if(!welcome)break;await key(welcome);
    }
    await key("Pip's missing ball · Available");await key('Help Pip');
    await page.waitForFunction(()=>pawsUi.screen===2);await fps();
    await page.screenshot({path:path.join(out,name+'-puzzle.png'),scale:'css'});
    await page.setViewportSize({width:1280,height:800});
    await page.waitForFunction(()=>Module.canvas.width===1280&&Module.canvas.height===800);await msaa(1);
    report.checks.push({name,automatic,high:{quality:high.quality,render:high.render,canvas:high.canvas},savedHigh:true,resized:true});
   } else {
    await key('Menu');await key('Settings');await key('Text size: Standard');await key('Back');await key('Continue');
    const largerText=await fps();await page.screenshot({path:path.join(out,name+'-large-text.png'),scale:'css'});
    report.checks.push({name,automatic,largerText});
   }
   await page.close();
  }
  assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{const json=JSON.stringify(report,(_,v)=>typeof v==='bigint'?Number(v):v,2);await fs.writeFile(path.join(out,'report.json'),json);console.log(json);await browser.close();}
})();
