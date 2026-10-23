// Fixed low-angle views expose the opaque-depth cutoff across distant water.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {PNG}=require('pngjs');
const backend=process.env.QUEST_BACKEND||'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/beach-water-seam/${backend}`;
const report={backend,errors:[],views:[]};
(async()=>{let browser;try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8080/')+'?devBeach=1');
 await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(250);}
 await key('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
 await key('Adventures');await key('Start / Continue beach');await page.waitForFunction(()=>pawsUi.screen===3,null,{timeout:90000});await key('Help Pip');
 report.build=await page.evaluate(()=>pawsBuild);assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 let previous=.88;
 for(const elevation of [.3,.2,.1,.5]){
  await page.evaluate(dy=>Module._paws_orbit(0,dy),(elevation-previous)/.006);previous=elevation;
  await page.waitForTimeout(400);
  const pixels=await page.screenshot({path:path.join(out,`elevation-${elevation}.png`)});report.views.push(elevation);
  if(elevation===.3){
   const png=PNG.sync.read(pixels),rows=[];
   // Open-water strip at the reproduced receiver cutoff; average wave detail
   // across columns so a full-width colour discontinuity dominates the metric.
   for(let y=172;y<=185;++y){const rgb=[0,0,0];for(let x=700;x<1250;++x)for(let c=0;c<3;++c)rgb[c]+=png.data[(y*png.width+x)*4+c]/550;rows.push(rgb);}
   report.cutoffStep=Math.max(...rows.slice(1).flatMap((r,i)=>r.map((v,c)=>Math.abs(v-rows[i][c]))));
   if(process.env.ASSERT_SEAM_FIXED==='1')assert(report.cutoffStep<8,`Distant-water seam: ${report.cutoffStep}`);
  }
 }
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;process.exitCode=1;}finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report));}})();
