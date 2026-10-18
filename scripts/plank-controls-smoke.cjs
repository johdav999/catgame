const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=process.env.EVIDENCE_DIR||'build/evidence/plank-controls';
const report={checks:[],errors:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 let page;
 try{
 page=await browser.newPage({viewport:{width:1600,height:1000}});
 page.on('pageerror',e=>report.errors.push(e.message));
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(160);
 async function key(name){for(let n=0;n<600;n++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await settle();return;}await page.keyboard.press('Tab');}throw Error('Unreachable '+name);}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b);await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 async function click(id){const a=(await points()).objects[id];await page.mouse.click(a.x,a.y);await settle();}
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8137/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 await key('Play');await page.waitForFunction(()=>pawsState.garden);await key('Menu');await key('Save and recovery');
 const chooser=page.waitForEvent('filechooser');await key('Import save');await (await chooser).setFiles('tests/fixtures/phase7-completed.save');await settle();await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsState.garden);
 await key('Menu');await key('Requests & friends');
 let names=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 while(names.some(n=>n.startsWith('Welcome '))){await key(names.find(n=>n.startsWith('Welcome ')));names=await page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));}
 await key('The sunny spot · Available');await key('Help Mabel');await page.waitForFunction(()=>pawsState.quest==='mabel_sunny_spot'&&pawsUi.screen===2);await settle();
 let before=await state();await drag('long_plank','long_socket',true);assert.equal((await state()).revision,before.revision);report.checks.push('Blur cancels drag without changing the attempt');
 await drag('long_plank','long_socket');assert.equal((await state()).revision,before.revision);assert.match((await state()).message,/Rotate/);report.checks.push('Misaligned drop is rejected without picking up or changing state');
 await click('long_plank');assert.equal(await page.evaluate(()=>pawsUi.screen),2);assert.match((await state()).message,/turned clockwise/);report.checks.push('First mouse-up click rotates without opening a menu');
 before=await state();await drag('long_plank','unsupported_socket');assert.equal((await state()).revision,before.revision);report.checks.push('Invalid crossing preserves plank and revision');
 await drag('long_plank','long_socket');assert.match((await state()).message,/connected to both supports/);
 // Dragging away and returning still counts as a drag, never a click.
 const a=(await points()).objects.long_plank;before=await state();await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(a.x+35,a.y,{steps:4});await page.mouse.move(a.x,a.y,{steps:4});await page.mouse.up();await settle();assert.doesNotMatch((await state()).message,/turned clockwise/);report.checks.push('A drag returning to its origin does not rotate');
 await click('step_plank');await drag('step_plank','step_socket');assert.match((await state()).message,/connected to both supports/);
 await click('finish_plank');await drag('finish_plank','finish_socket');assert((await state()).completed);
 await page.screenshot({path:path.join(out,'route.png')});
 report.checks.push('All three planks placed with mouse; route completes automatically');
 assert.deepEqual(report.errors,[]);report.build=await page.evaluate(()=>pawsBuild);report.passed=true;
 }catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({text:document.body.innerText,state:window.pawsState,render:window.pawsRenderState,storage:window.pawsStorageReady,gui:window.pawsGuiReady}));await page?.screenshot({path:path.join(out,'failure.png')});process.exitCode=1;}finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}
})();
