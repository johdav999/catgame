const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/bell-click/browser');
let browser,page;const report={errors:[],checks:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:2187,height:964}});page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(150);}
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction.objects;});
 const count=()=>page.evaluate(()=>Number(document.body.innerText.match(/(\d+) \/ 5 correct/)?.[1]));
 async function click(id){const p=(await points())[id];assert(p,id);await page.mouse.click(p.x,p.y);await page.waitForTimeout(150);}
 async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});}
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/?debugSequence=1');await ready();
 const fixture=await fs.readFile('build/evidence/picture-sequence/chapter3.save','utf8');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},fixture);
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsSequence?.watching);await page.waitForFunction(()=>!pawsSequence.watching);
 await click('paw');assert.equal(await count(),0);
 // A player retries with a camera angle chosen after the error, like the reported view.
 await page.mouse.move(1900,750);await page.mouse.down();await page.mouse.move(1750,700,{steps:10});await page.mouse.up();
 for(let n=0;n<8;n++)await page.mouse.wheel(0,-80);await page.waitForTimeout(200);
 const before=await points();await shot('before-retry');
 await page.mouse.click(before['leaf.picture']?.x||before.leaf.x,before['leaf.picture']?.y||before.leaf.y);await page.waitForTimeout(150);
 assert.equal(await count(),1);const after=await points();await shot('after-leaf');
 report.cameraMovement={before,after,starDisplacement:Math.hypot(before.star.x-after.star.x,before.star.y-after.star.y)};
 await page.mouse.click(before['star.picture'].x,before['star.picture'].y);await page.waitForTimeout(150);report.nextClickCount=await count();
 assert(report.cameraMovement.starDisplacement<1,'Correct bell tap moved the next target by '+report.cameraMovement.starDisplacement.toFixed(1)+' pixels');
 assert.equal(report.nextClickCount,2);for(const [id,n] of [['leaf.picture',3],['paw.picture',4],['star.picture',5]]){await click(id);if(n<5)assert.equal(await count(),n);if(n===4)await shot('four-correct');}
 await page.waitForFunction(()=>pawsState.completed);assert.equal(await page.evaluate(()=>pawsState.coins),160);await shot('complete');
 report.checks.push('Retry from an orbited error view preserves bell positions and completes with visible glyph clicks');
 assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;process.exitCode=1;console.error(e);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});}).finally(async()=>{report.debug=await page?.evaluate(()=>window.pawsSequenceDebug).catch(()=>null);await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});
