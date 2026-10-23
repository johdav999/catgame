// Real ground-shadow pixels, quality toggles and WebGL context restoration.
const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu';
const out=process.env.EVIDENCE_DIR||`build/evidence/beach-shadows/${backend}`;
const report={backend,errors:[],checks:[]};
(async()=>{let browser;try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1280,height:720}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8080/')+'?devBeach=1');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(250);}
 await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
 await key('Adventures');await key('Start / Continue beach');await page.waitForFunction(()=>pawsUi.screen===3,null,{timeout:90000});await key('Help Pip');
 assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 await key('Menu');await key('Settings');await key('Motion: Full');await key('Back');await key('Continue');
 await page.evaluate(()=>Module._paws_orbit(0,(.5-.88)/.006));
 const capture=async name=>{await page.waitForTimeout(600);return page.screenshot({path:path.join(out,name+'.png')});};
 await page.waitForFunction(()=>pawsPerformance?.current.shadowPasses===1);
 const on=await capture('shadows-on');report.render=await page.evaluate(()=>pawsRenderState);report.build=await page.evaluate(()=>pawsBuild);
 assert.equal(report.render.shadowFilterTaps,9);assert.equal(report.render.shadowResolution,512);
 await key('Menu');await key('Settings');await key('Quality: Automatic');await key('Back');await key('Continue');
 await page.waitForFunction(()=>pawsPerformance?.current.shadowPasses===0);
 const off=await capture('shadows-off');
 const a=PNG.sync.read(on),b=PNG.sync.read(off);assert.equal(a.width,b.width);assert.equal(a.height,b.height);
 let darker=0,soft=0;
 // Only the central sand/caster region; excludes sky, water and GUI changes.
 for(let y=290;y<590;y++)for(let x=190;x<1050;x++){
  const i=(y*a.width+x)*4;const delta=(b.data[i]+b.data[i+1]+b.data[i+2]-a.data[i]-a.data[i+1]-a.data[i+2])/3;
  if(delta>8)darker++;if(delta>3&&delta<25)soft++;
 }
 report.pixels={darker,soft};assert(darker>800,'Enabled shadows must visibly darken ground/casters');assert(soft>300,'Filtered shadow transitions must be visible');
 report.checks.push('Shadow on/off changes visible beach pixels and removes the depth pass when disabled');
 await key('Menu');await key('Settings');await key('Quality: Mobile');await key('Back');await key('Continue');
 await page.waitForFunction(()=>pawsRenderState.effectiveMsaaSamples===4&&pawsPerformance?.current.shadowPasses===1);
 await capture('shadows-msaa4');report.checks.push('Soft shadows render with 4x MSAA');
 await key('Menu');await key('Settings');await key('Quality: High (2×)');await key('Back');await key('Continue');
 await page.waitForFunction(()=>pawsRenderState.effectiveMsaaSamples===1&&pawsPerformance?.current.shadowPasses===1);
 if(backend==='webgl2'){
  const generation=await page.evaluate(()=>pawsRenderState.generation);
  await page.evaluate(()=>{const ext=document.querySelector('#viewport').getContext('webgl2').getExtension('WEBGL_lose_context');ext.loseContext();setTimeout(()=>ext.restoreContext(),200);});
  await page.waitForFunction(g=>pawsRenderState.state==='ready'&&pawsRenderState.generation>g&&pawsPerformance?.current.shadowPasses===1,generation,{timeout:90000});
  await capture('shadows-restored');report.checks.push('WebGL context restoration recreates the shadow map');
 }
 await page.setViewportSize({width:980,height:720});await capture('shadows-resized');
 await key('Menu');await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
 await page.waitForFunction(()=>pawsRenderState.shadowResolution===1024);report.checks.push('Resize and return to garden restore the garden shadow resolution');
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;process.exitCode=1;}finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report));}})();
