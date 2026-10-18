const {chromium}=require('playwright'),fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const out='build/evidence/cat-quests',report={errors:[],checks:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 try{
 const context=await browser.newContext({viewport:{width:1600,height:1000},hasTouch:true});const page=await context.newPage();
 page.on('pageerror',e=>report.errors.push(e.message));
 let markerResponse;page.on('response',r=>{if(r.url().endsWith('/quest_marker.glb'))markerResponse=r;});
 const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 async function key(name){for(let n=0;n<600;n++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(100);return;}await page.keyboard.press('Tab');}throw Error('Unreachable '+name+' '+JSON.stringify(await names()));}
 async function snapshot(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return {points:pawsInteraction.objects,markers:pawsQuestMarkers};});}
 async function garden(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await page.waitForTimeout(250);}
 async function solve(){for(const step of await page.evaluate(()=>pawsState.solution)){
  if(await page.evaluate(()=>pawsUi.screen)!==7)await key('Objects & actions');
  let s=await page.evaluate(()=>pawsState);if(step.object){const n=s.objects.findIndex(o=>o.id===step.object);await key(`${n+1}. ${s.objects[n].name}`);}
  s=await page.evaluate(()=>pawsState);const a=s.actions.find(a=>a.verb===step.verb&&a.object===step.object&&a.target===step.target&&a.direction===step.direction&&(step.verb!==5||(a.x===step.x&&a.y===step.y)));assert(a&&a.enabled,JSON.stringify(step));await key(a.label);
 }await page.waitForFunction(()=>pawsUi.screen===9);await key('Back to garden');await garden();}
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8125/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await key('Play');await garden();
 let snap=await snapshot();assert(snap.markers[0].visible);let yaw=snap.markers[0].yaw;await page.waitForTimeout(500);assert.notEqual((await snapshot()).markers[0].yaw,yaw);
 const p=snap.points.pip;await page.mouse.move(p.x,p.y);await page.mouse.down();await page.mouse.move(p.x+35,p.y,{steps:4});await page.mouse.move(p.x,p.y,{steps:4});await page.mouse.up();await garden();
 await page.mouse.move(p.x,p.y);await page.mouse.down();await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await garden();report.checks.push('Dragged and cancelled cat taps do not start requests');
 await key('Menu');await key('Settings');await key('Motion: Full');await key('Back');await key('Continue');await garden();assert.equal((await snapshot()).markers[0].yaw,0);await page.waitForTimeout(250);assert.equal((await snapshot()).markers[0].yaw,0);
 await key('Menu');await key('Settings');await key('Motion: Reduced');await key('Back');await key('Continue');await garden();report.checks.push('Reduced motion stops marker spin');
 const requests=[['pip',0,1600,1000,false],['pip',1,390,844,true],['pip',2,844,390,true],['button',3,1600,1000,false],['button',5,390,844,true],['mabel',4,844,390,true],['mabel',6,1600,1000,false],['pepper',7,390,844,true],['pip',8,844,390,true],['mabel',9,1600,1000,false]];
 for(const [cat,q,width,height,touch] of requests){
  await page.setViewportSize({width,height});await page.waitForTimeout(250);
  await key('Requests & friends');let options=await names();while(options.some(n=>n.startsWith('Welcome '))){await key(options.find(n=>n.startsWith('Welcome ')));options=await names();}await key('Back');await garden();
  snap=await snapshot();const i=['pip','mabel','button','pepper'].indexOf(cat);assert(snap.markers.find(m=>m.cat===i).visible);
  await page.screenshot({path:path.join(out,`garden-${q}-${width}x${height}.png`)});const at=snap.points[cat];assert(at);
  if(touch)await page.touchscreen.tap(at.x,at.y);else await page.mouse.click(at.x,at.y);
  await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);assert.equal(await page.evaluate(()=>pawsState.quest),['pip_missing_ball','pip_tidy_toys','pip_stuck_gate','button_mouse_bench','mabel_sunny_spot','button_cushion_bed','mabel_drinking_route','pepper_garden_bells','pip_pepper_playhouse','garden_picnic'][q]);
  await solve();report.checks.push(`${touch?'Touch':'Mouse'} ${cat} starts quest ${q} directly at ${width}x${height}; completion and garden return pass`);
 }
 assert((await snapshot()).markers.every(m=>!m.visible),'All completed requests hide every marker');report.checks.push('All ten requests completed; every quest marker is hidden');
 assert.equal(markerResponse.status(),200);report.markerSha256=crypto.createHash('sha256').update(await markerResponse.body()).digest('hex');assert.equal(report.markerSha256,crypto.createHash('sha256').update(await fs.readFile('assets/runtime/models/garden/quest_marker.glb')).digest('hex'));
 report.build=await page.evaluate(()=>pawsBuild);assert.deepEqual(report.errors,[]);report.passed=true;
 }catch(e){report.failure=e.stack;process.exitCode=1;}finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}
})();
