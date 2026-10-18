// Actual pinned generated-site inspection. Isolated server, profile and save slot.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p06-pinned-release/site'),evidence=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-06/browser');
const port=Number(process.env.PAWS_TEST_PORT||19616),url=`http://127.0.0.1:${port}/`,hash=b=>crypto.createHash('sha256').update(b).digest('hex');
(async()=>{
 await fs.mkdir(evidence,{recursive:true});const report={site,url,checks:[],errors:[],assets:[],screenshots:[],gates:[]},pending=[];
 const server=http.createServer(async(req,res)=>{try{const u=new URL(req.url,url);if(u.pathname==='/favicon.ico'){res.statusCode=204;res.end();return;}const file=path.resolve(site,'.'+decodeURIComponent(u.pathname==='/'?'/index.html':u.pathname));if(!file.startsWith(site+path.sep))throw Error('outside site');const bytes=await fs.readFile(file);res.setHeader('Content-Type',({'.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.html':'text/html','.json':'application/json','.svg':'image/svg+xml'})[path.extname(file)]||'application/octet-stream');res.setHeader('Cache-Control','no-store');res.end(bytes);}catch{res.statusCode=404;res.end('missing');}});
 await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});let browser,page;
 try{
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
  page=await browser.newPage({viewport:{width:1440,height:1000}});page.on('pageerror',e=>report.errors.push(String(e)));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  page.on('response',r=>{if(r.url().endsWith('.glb'))pending.push((async()=>{const relative=new URL(r.url()).pathname.slice(1),data=await r.body();report.assets.push({url:relative,status:r.status(),sha256:hash(data),expected:hash(await fs.readFile(path.join(site,relative)))});})());});
  const ready=()=>page.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});
  const snap=()=>page.evaluate(()=>window.gardenSnapshot);
  const capture=async name=>{await page.locator('#viewport').screenshot({path:path.join(evidence,name+'.png')});report.screenshots.push(name+'.png');};
  await page.goto(url);await ready();assert.match(await page.title(),/garden harness/);await page.waitForTimeout(500);await capture('chapter1');
  assert.equal((await snap()).cats.filter(c=>c.introduced).length,1);
  for(let i=0;i<10;i++){await page.locator('#trace').click();await page.waitForFunction(coins=>window.gardenSnapshot.coins===coins,(i+1)*20);}
  for(const cat of [1,2,3]){await page.locator('#cat').selectOption(String(cat));await page.locator('#meet').click();await page.waitForFunction(c=>window.gardenSnapshot.cats[c].introduced,cat);}
  assert.equal((await snap()).chapter,3);report.checks.push('All ten real owner traces earn 200 coins; chapter introductions acknowledged');
  const placements=[0,1,3,4,5,6,7,8];for(let i=0;i<8;i++){await page.locator('#item').selectOption(String(i));await page.locator('#buy').click();await page.waitForFunction(n=>window.gardenSnapshot.items[n].owned,i);await page.locator('#spot').selectOption(String(placements[i]));await page.locator('#place').click();await page.waitForFunction(n=>!!window.gardenSnapshot.items[n].spot,i);
   if(i===0){await page.locator('#celebrate').click();await page.waitForFunction(()=>window.gardenSnapshot.message.startsWith('Supplied picnic'));let v=await snap();assert.equal(v.coins,170);assert.equal(v.items[6].owned,false);assert.equal(v.items[7].owned,false);await capture('supplied-picnic-without-purchases');}
  }
  assert.equal((await snap()).coins,10);await capture('all-eight-furnishings');
  await page.locator('#item').selectOption('6');await page.locator('#gather').click();assert.match((await snap()).message,/pending|missing|arrived/);assert.equal((await snap()).coins,10);report.checks.push('Supplied celebration props render without picnic purchases; absent contact assets reject group use without changing progress');
  for(let i=0;i<8;i++){await page.locator('#view').selectOption(String(placements[i]+5));await page.waitForTimeout(150);await capture('furnishing-'+i);}
  report.checks.push('Eight accepted prop furnishings bought and rendered at authored compatible placements; one 200/190 economy');
  await page.locator('#cat').selectOption('0');await page.locator('#item').selectOption('0');await page.locator('#view').selectOption('5');
  await page.locator('#reduced').check();await page.locator('#use').click();await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===4);await capture('pip-supported-low-rest');
  let before=(await snap()).cats[0];await page.waitForTimeout(800);let after=(await snap()).cats[0];assert.equal(before.time,after.time);assert.equal(before.y,after.y);
  await page.locator('#spot').selectOption('2');await page.locator('#preview').click();await page.locator('#cancel').click();assert.equal((await snap()).cats[0].phase,4);
  await page.locator('#place').click();await page.waitForFunction(()=>window.gardenSnapshot.items[0].spot==='quiet_corner'&&window.gardenSnapshot.cats[0].phase===0);
  await page.locator('#spot').selectOption('0');await page.locator('#place').click();await page.waitForFunction(()=>window.gardenSnapshot.items[0].spot==='bench_corner');
  await page.locator('#item').selectOption('1');await page.locator('#view').selectOption('6');await page.locator('#use').click();await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===4);await capture('pip-drink-static');
  report.checks.push('Reduced contact poses remain static; preview/cancel preserves use; committed move invalidates use');
  await page.locator('#cancel-use').click();await page.locator('#reduced').uncheck();await page.locator('#use').click();await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===2);
  await page.locator('#pause').click();await page.waitForFunction(()=>window.gardenSnapshot.paused);before=(await snap()).cats[0];await page.waitForTimeout(600);after=(await snap()).cats[0];assert.deepEqual([before.x,before.z,before.time],[after.x,after.z,after.time]);
  await page.evaluate(()=>{Module._garden_suspend(1);Module._garden_suspend(0);});assert.equal((await snap()).paused,true);await page.locator('#pause').click();
  console.log('Inspecting real in-place walk, approach, drink and return…');
  await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===4,null,{timeout:45000});await capture('pip-drink-playback');await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===6,null,{timeout:15000});
  await page.locator('#cancel-use').click();report.checks.push('Real walk/approach/drink/return playback; pause freezes movement and skin; visibility roundtrip preserves explicit pause');
  await page.locator('#shadows').uncheck();await page.locator('#view').selectOption('0');await page.waitForTimeout(200);await capture('shadow-free');
  const baseline=await snap();await page.evaluate(()=>Module._garden_fail_candidate());await page.waitForFunction(()=>window.gardenState==='failed');assert.equal((await snap()).sceneCount,1);assert.equal((await snap()).coins,baseline.coins);assert.equal(await page.evaluate(()=>window.pawsRenderState.state),'ready');
  await page.locator('#retry').click();await ready();await page.evaluate(()=>{for(let i=0;i<12;i++)Module._garden_rebuild();Module._garden_cancel_loading();});assert.equal((await snap()).sceneCount,1);await page.locator('#rebuild').click();await ready();assert.equal((await snap()).sceneCount,1);
  await page.evaluate(()=>window.pawsFlushSave());await page.reload();await ready();assert.equal((await snap()).coins,10);assert.equal((await snap()).items.filter(i=>i.owned).length,8);assert.equal((await snap()).cats.filter(c=>c.introduced).length,4);
  report.checks.push('Failed candidate retains valid outgoing renderer; rapid latest-wins preparation/cancel; one live scene; version-6 reload retains owner rewards, introductions and placements');
  report.gates=(await snap()).cats.map(c=>({name:c.name,modelLoaded:c.available,uses:c.uses}));
  for(const [w,h] of [[390,844],[844,390]]){await page.setViewportSize({width:w,height:h});await page.waitForTimeout(250);assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));await page.screenshot({path:path.join(evidence,`layout-${w}x${h}.png`),fullPage:true});}
  await page.locator('#attention').focus();await page.keyboard.press('Enter');await page.waitForFunction(()=>window.gardenSnapshot.cats[0].phase===7);report.checks.push('Keyboard controls and responsive portrait/short landscape inspection');
  await Promise.all(pending);for(const a of report.assets){assert.equal(a.status,200);assert.equal(a.sha256,a.expected);}assert.deepEqual(report.errors,[]);
  report.uniqueAssets=new Set(report.assets.map(a=>a.url)).size;report.performance=await page.evaluate(()=>window.pawsPerformance);report.finalSnapshot=await snap();
  const fail=await browser.newPage();await fail.route('**/props/hammock.glb',r=>r.fulfill({status:503,body:'Injected load failure'}));await fail.goto(url);await fail.waitForFunction(()=>window.gardenState==='failed',null,{timeout:45000});await fail.unroute('**/props/hammock.glb');await fail.locator('#retry').click();await fail.waitForFunction(()=>window.gardenState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:45000});await fail.close();report.checks.push('Real HTTP preparation failure retries successfully; every loaded GLB matches staged bytes');
  report.wasmSha256=hash(await fs.readFile(path.join(site,'release1_preview.wasm')));console.log(JSON.stringify({checks:report.checks,gates:report.gates.map(c=>({name:c.name,modelLoaded:c.modelLoaded})),wasmSha256:report.wasmSha256}));
 }catch(error){report.failure=String(error);if(page){report.state=await page.evaluate(()=>({garden:window.gardenState,renderer:window.pawsRenderState,snapshot:window.gardenSnapshot,status:document.querySelector('#status').textContent}));await page.screenshot({path:path.join(evidence,'failure.png'),fullPage:true});}throw error;}
 finally{await fs.writeFile(path.join(evidence,'report.json'),JSON.stringify(report,null,2));if(browser)await browser.close();await new Promise(resolve=>server.close(resolve));}
})().catch(e=>{console.error(e);process.exitCode=1;});
