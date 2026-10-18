const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises');
const assert=require('node:assert/strict');
(async()=>{
 const out=process.env.EVIDENCE_DIR||'build/evidence/garden-sun';
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1440,height:900}});
 page.setDefaultTimeout(45000);
 // Keep the legacy host capability gate satisfied, then exercise the renderer's
 // real adapter-failure fallback to WebGL 2.
 if(process.env.WEBGL==='1')await page.addInitScript(()=>Object.defineProperty(navigator.gpu,'requestAdapter',{value:async()=>null}));
 const errors=[];page.on('pageerror',e=>errors.push(e.message));
 try {
  await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8116/');
  await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
  await page.getByRole('button',{name:'Play',exact:true}).focus();await page.keyboard.press('Enter');
  await page.waitForFunction(()=>pawsState.garden&&pawsState.production);
  await page.waitForTimeout(1500);
  await page.screenshot({path:out+'/garden.png'});
  // Real background drag continues above the former horizon clamp.
  await page.mouse.move(1240,270);await page.mouse.down();await page.mouse.move(1240,40,{steps:30});await page.mouse.up();
  await page.locator('canvas').first().focus();
  for(let i=0;i<34;i++)await page.keyboard.press('ArrowRight');
  await page.waitForTimeout(800);
  function countSun(buffer){const pixels=PNG.sync.read(buffer);
  let sunPixels=0;
  for(let y=300;y<600;y++)for(let x=500;x<940;x++){
   const i=(y*pixels.width+x)*4;
   if(pixels.data[i]>245&&pixels.data[i+1]>240&&pixels.data[i+2]>220)sunPixels++;
  }
  return sunPixels;}
  const sunPixels=countSun(await page.screenshot({path:out+'/sun.png'}));
  assert(sunPixels>10,'Sun disc is not visible in the center of the sky');
  await page.keyboard.press('-');await page.waitForTimeout(250);
  assert(countSun(await page.screenshot({path:out+'/sun-zoom.png'}))>10,'Zoom lost the upward gaze');
  // Reverse the same gesture and restore ordinary garden framing.
  await page.mouse.move(1240,200);await page.mouse.down();await page.mouse.move(1240,500,{steps:30});await page.mouse.up();
  await page.evaluate(()=>Module._paws_reset_view());await page.waitForTimeout(300);
  await page.screenshot({path:out+'/reset.png'});
  assert.deepEqual(errors,[]);
  const report={passed:true,sunPixels,errors,build:await page.evaluate(()=>pawsBuild),render:await page.evaluate(()=>pawsRenderState)};
  await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(report);
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
