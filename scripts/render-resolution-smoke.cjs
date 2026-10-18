const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/render-resolution';
const baseline=process.env.BASELINE==='1';
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={baseline,checks:[],errors:[]};
 try{
  for(const [name,width,height,dpr] of [['desktop',1920,1080,1],['large',2560,1440,1],['retina',1440,900,2],['portrait',390,844,3],['landscape',844,390,2]]){
   const page=await browser.newPage({viewport:{width,height},deviceScaleFactor:dpr});
   page.on('pageerror',e=>report.errors.push(e.message));
   page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
   await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8107');
   await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
   async function key(name){
    for(let n=0;n<250;n++){
     if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(150);return;}
     await page.keyboard.press('Tab');
    }throw Error('Unreachable '+name);
   }
   await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
   await page.waitForTimeout(800);
   const state=await page.evaluate(()=>({quality:pawsQuality,render:pawsRenderState,canvas:[Module.canvas.width,Module.canvas.height],css:[Module.canvas.clientWidth,Module.canvas.clientHeight]}));
   const mobile=baseline&&(width<700||height<500);
   const scale=1,maxDpr=mobile?1:baseline?1.5:1,pixels=mobile?921600:baseline?2073600:8294400;
   const ratio=Math.min(Math.min(dpr,maxDpr)*scale,(mobile?4096:baseline?4096:8192)/width,(mobile?4096:baseline?4096:8192)/height,Math.sqrt(pixels/width/height));
   assert.deepEqual(state.canvas,[Math.floor(width*ratio),Math.floor(height*ratio)]);
   assert.deepEqual(state.css,[width,height]);
   await page.screenshot({path:path.join(out,name+'.png'),scale:'css'});
   report.checks.push({name,dpr,...state});
   if(name==='desktop'){
    await page.mouse.move(width/2,height/2);await page.mouse.wheel(0,-500);await page.waitForTimeout(500);
    await page.screenshot({path:path.join(out,'bench-zoom.png'),scale:'css'});
    await key('Menu');await key('Settings');
    const labels=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
    report.settingsLabels=labels;
    await key('Quality: Automatic');
    await page.waitForFunction(()=>pawsQuality.profile==='mobile');
    assert(await page.evaluate(()=>Module.canvas.width*Module.canvas.height<=921600));
    await key('Quality: Mobile');
    await page.waitForFunction(()=>pawsQuality.profile==='desktop');
    await key('Back');await key('Continue');
    await key('Requests & friends');
    let names=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
    while(names.some(n=>n.startsWith('Welcome '))){await key(names.find(n=>n.startsWith('Welcome ')));names=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
    await key("Pip's missing ball · Available");await key('Help Pip');
    await page.waitForFunction(()=>pawsUi.screen===2);
    await page.waitForTimeout(800);
    const points=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
    // Drag keeps the grabbed offset. Translate by one ground-cell vector,
    // rather than dropping the raised model's centre onto the ground centre.
    const a=points.objects.pot_a,from=points.slots['1,2'],to=points.slots['1,1'];
    const b={x:a.x+to.x-from.x,y:a.y+to.y-from.y};
    report.drag={a,b};
    await page.screenshot({path:path.join(out,'puzzle-before-drag.png'),scale:'css'});
    await page.mouse.move(a.x,a.y);await page.mouse.down();await page.waitForTimeout(80);await page.mouse.move(b.x,b.y,{steps:8});await page.waitForTimeout(80);await page.mouse.up();
    report.drag.state=await page.evaluate(()=>pawsState);
    await page.waitForFunction(()=>pawsState.objects.find(o=>o.id==='pot_a').y===1);
    await key('Objects & actions');await key('Undo');await key('Back');
    assert.equal(await page.evaluate(()=>pawsState.objects.find(o=>o.id==='pot_a').y),2);
    report.interaction='Quality switches, projected drag and undo passed';
   }
   await page.close();
  }
  assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,(_,v)=>typeof v==='bigint'?Number(v):v,2));console.log(JSON.stringify(report,(_,v)=>typeof v==='bigint'?Number(v):v,2));await browser.close();}
})();


