// Verify real adapter selection and full-resolution Automatic on both backends.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
(async()=>{
 const out=process.env.EVIDENCE_DIR||'build/evidence/gpu-path-20261018';await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={runs:[],errors:[]};
 try{
  for(const fallback of [false,true]){
   const context=await browser.newContext({viewport:{width:2187,height:963},deviceScaleFactor:1});
   if(fallback)await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
   const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
   await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8194/')+'?profile=1');
   await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
   const key=async(name)=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');};
   await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
   await page.waitForTimeout(4000);
   const data=await page.evaluate(()=>({build:pawsBuild,quality:pawsQuality,render:pawsRenderState,perf:pawsPerformance,graphics:JSON.parse(document.querySelector('#paws-profile-data').textContent).graphics}));
   assert.equal(data.render.backend,fallback?'webgl2':'webgpu');
   assert.equal(Number(data.quality.maxPixels),8294400);
   assert.equal(data.render.width,2187);assert.equal(data.render.height,963);
   await key('Menu');await key('Settings');await key('Quality: Automatic');await key('Quality: Mobile');
   await page.waitForFunction(()=>pawsQuality.profile==='desktop'&&Number(pawsRenderState.maxPixels)===8294400);
   await key('Quality: High (2×)');
   await page.waitForFunction(()=>pawsQuality.profile==='automatic'&&Number(pawsRenderState.maxPixels)===8294400&&pawsRenderState.width===2187&&pawsRenderState.height===963);
   report.runs.push({fallback,...data});await context.close();
  }
  assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}
 finally{const json=JSON.stringify(report,(_,v)=>typeof v==='bigint'?Number(v):v,2);await fs.writeFile(out+'/report.json',json);await browser.close();console.log(json);}
})();
