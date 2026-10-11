// Acceptance against the generated site, via visible controls rather than direct model mutation.
const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const assert=require('node:assert/strict');
const fs=require('node:fs/promises');
const path=require('node:path');
const crypto=require('node:crypto');
const url=process.env.PAWS_URL||'http://localhost:8080';
const evidence=process.env.EVIDENCE_DIR||'build/evidence/phase1';
(async()=>{
 await fs.mkdir(evidence,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE});
 const result={url,checks:[],errors:[],browser:browser.version(),contentId:'missing-ball-v1'};
 let page;
 try {
  result.buildId=crypto.createHash('sha256').update(await fs.readFile('build/web-pinned-release/site/paws.wasm')).digest('hex');
  page=await browser.newPage({viewport:{width:1440,height:900},deviceScaleFactor:1});
  page.on('pageerror',error=>result.errors.push(String(error)));
  page.on('console',m=>{if(m.type()==='error')result.errors.push(m.text());});
  await page.goto(url);
  await page.waitForFunction(()=>window.pawsRenderState?.state==='ready'&&window.pawsState,{timeout:30000});
  await page.waitForFunction(()=>window.pawsSceneStats?.meshes>30);
  const click=id=>page.locator('#'+id).click();
  const feedback=()=>page.locator('#feedback').textContent();
  const state=()=>page.evaluate(()=>window.pawsState);
  const shot=name=>page.screenshot({path:path.join(evidence,name+'.png'),fullPage:true});
  const canvas=page.locator('#viewport');
  const baseline=await canvas.screenshot();
  const pixels=PNG.sync.read(baseline),colors=new Set();
  for(let i=0;i<pixels.data.length;i+=4)colors.add(pixels.data.readUInt32BE(i));
  assert(colors.size>30,'actual rendered board');
  await shot('initial-desktop');
  await click('roll');assert.match(await feedback(),/blocks the ball/);assert.equal((await state()).completed,0);
  await click('object-0');await click('east');assert.match(await feedback(),/No sliding slot/);
  await click('north');await click('object-1');await click('west');assert.match(await feedback(),/other pot/);
  await shot('blocked-occupancy');
  await click('undo');assert.equal((await state()).objects[0].y,2);
  result.checks.push('Failed roll, off-slot move, shared-slot occupancy rejection and undo via visible controls');
  async function solve(keyboard=false){
   const activate=async id=>{if(keyboard){await page.locator('#'+id).focus();await page.keyboard.press('Enter');}else await click(id);};
   await activate('object-0');await activate('north');await activate('north');
   await activate('object-1');await activate('west');
   for(const [id,count] of [[2,1],[3,3],[4,1],[5,1],[6,1]]){await activate('object-'+id);for(let n=0;n<count;n++)await activate('rotate');}
   await activate('roll');assert.equal((await state()).completed,1);assert.match(await feedback(),/Puzzle complete/);
  }
  await click('restart');await solve();await shot('solved-desktop');
  assert(!baseline.equals(await canvas.screenshot()),'committed puzzle changes rendered pixels');
  result.checks.push('Full known solution through mouse controls reaches the destination in WebGPU');
  await click('undo');assert.equal((await state()).completed,0);await click('roll');
  const attempt=(await state()).attempt;await click('restart');assert.equal((await state()).attempt,attempt+1);
  assert.equal((await state()).objects[0].y,2);assert.equal((await state()).completed,0);
  await click('pause');assert(await page.locator('#roll').isDisabled());
  await page.evaluate(()=>{window.dispatchEvent(new Event('pagehide'));window.dispatchEvent(new Event('pageshow'));});
  assert.equal((await state()).paused,1);await click('resume');
  await click('object-0');await click('north');await click('garden');await click('resume');assert.equal((await state()).objects[0].y,1);
  result.checks.push('Undo completion, restart, pause across host page lifecycle events and garden/continue retention');
  await click('hint');assert.match(await feedback(),/slide both pots/i);
  await click('restart');await solve(true);
  result.checks.push('Full puzzle solved with focused buttons and keyboard Enter');
  for(const [width,height,name] of [[390,844,'portrait'],[844,390,'landscape']]){
   await page.setViewportSize({width,height});await click('restart');
   assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
   await shot(name);
  }
  const touch=await browser.newContext({viewport:{width:390,height:844},hasTouch:true,isMobile:true});
  const touchPage=await touch.newPage();await touchPage.goto(url);await touchPage.waitForFunction(()=>window.pawsRenderState?.state==='ready');
  await touchPage.locator('#object-0').tap();await touchPage.locator('#north').tap();
  assert.equal(await touchPage.evaluate(()=>window.pawsState.objects[0].y),1);
  await touchPage.close();await touch.close();
  result.checks.push('390x844 and 844x390 layouts without horizontal overflow; emulated touch select/move');
  const unsupported=await browser.newPage();
  await unsupported.addInitScript(()=>Object.defineProperty(navigator,'gpu',{get:()=>undefined}));
  await unsupported.goto(url);await unsupported.getByText(/Paws needs WebAssembly and WebGPU/).waitFor();
  assert(await unsupported.locator('#retry').isVisible());await unsupported.screenshot({path:path.join(evidence,'unsupported.png')});await unsupported.close();
  result.checks.push('Readable missing-WebGPU startup failure with Retry');
  result.renderState=await page.evaluate(()=>window.pawsRenderState);
  assert.deepEqual(result.errors,[]);
 }catch(error){result.failure=String(error);if(page)await page.screenshot({path:path.join(evidence,'failure.png'),fullPage:true});throw error;}
 finally{await fs.writeFile(path.join(evidence,'browser-smoke.json'),JSON.stringify(result,null,2));await browser.close();console.log(JSON.stringify(result,null,2));}
})().catch(error=>{console.error(error);process.exitCode=1;});
