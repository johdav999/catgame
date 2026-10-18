// Real retained GUI controls, scene gestures and IndexedDB restoration.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu';
const base=process.env.PAWS_URL||'http://127.0.0.1:8193/';
const out=process.env.EVIDENCE_DIR||`build/evidence/beach-adventure/${backend}`;
const report={backend,url:base,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(220);
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const saved=()=>page.waitForFunction(()=>pawsState.save===2,null,{timeout:15000});
 const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function click(id){const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();}
 async function drag(id,cell,cancel=false){const p=await points(),a=p.objects[id],b=p.slots[cell];assert(a&&b,'Missing drag points');await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:14});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 async function garden(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});}
 async function beach(){await key('Adventures');await key('Start / Continue beach');await page.waitForFunction(()=>pawsUi.screen===3&&!pawsState.garden,null,{timeout:90000});await key('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2);}
 const dev=new URL(base);dev.searchParams.set('devBeach','1');
 await page.goto(base);await ready();await key('Play');await garden();assert(!(await names()).includes('Adventures'));
 report.checks.push('Default game has no development adventure entry');
 await saved();await page.goto(dev.href);await ready();await key('Continue');await garden();await beach();
 report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);
 assert.equal((await state()).quest,'beach_pip_ball');assert.match((await state()).scene,/beach greybox/);
 const initial=await state();assert.equal(initial.coins,0);assert.equal(initial.sceneCount,1);
 await page.screenshot({path:path.join(out,'beach-start.png')});
 await drag('basket_b','1,0',true);assert.deepEqual((await state()).objects,initial.objects);
 await drag('basket_b','1,0');assert.equal((await state()).objects.find(o=>o.id==='basket_b').y,0);
 await key('Menu');await key('Undo');await key('Continue');assert.deepEqual((await state()).objects,initial.objects);
 report.checks.push('Cancelled scene drag leaves pieces unchanged; Undo restores a committed slide');
 await drag('basket_b','1,0');await drag('basket_b','2,0');await drag('basket_a','1,1');
 assert.equal((await state()).objects.find(o=>o.id==='basket_a').y,1,'Front basket is draggable above its covered channel');
 const halfway=(await state()).objects;await saved();await page.reload();await ready();await key('Continue');
 await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Reload while on the beach restores the exact board');
 await key('Menu');await key('Back to garden');await garden();assert.equal((await state()).quest,'pip_missing_ball');
 await saved();await page.reload();await ready();await key('Continue');await garden();await beach();assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Leave, reload in the garden and return resumes the beach board and original base request');
 // Removing development access preserves the paid-content-shaped progress,
 // returns safely home, and never treats an imported/local save as ownership.
 await saved();await page.goto(base);await ready();await key('Continue');await garden();assert(!(await names()).includes('Adventures'));
 await saved();await page.goto(dev.href);await ready();await key('Continue');await garden();await beach();assert.deepEqual((await state()).objects,halfway);
 report.checks.push('Removing and restoring development access preserves progress without granting ownership');
 await page.setViewportSize({width:390,height:844});await settle();
 const p=await points(),board=await page.evaluate(()=>pawsUi.board);
 for(const id of ['basket_a','basket_b','channel_a','channel_b','channel_c','source_ball']){const a=p.objects[id];assert(a.x>=board.x&&a.x<=board.x+board.width&&a.y>=board.y&&a.y<=board.y+board.height,JSON.stringify({id,a,board}));}
 await page.screenshot({path:path.join(out,'beach-portrait.png')});
 await drag('basket_a','1,0');assert.equal((await state()).objects.find(o=>o.id==='basket_a').y,0);
 await page.setViewportSize({width:1600,height:1000});await settle();
 for(const id of ['channel_a','channel_b','channel_b','channel_b','channel_c'])await click(id);
 await click('source_ball');await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:20000});
 assert.equal((await state()).coins,0);assert.equal((await state()).chapter,1);assert(!(await page.locator('body').innerText()).includes('20 coins'));
 await page.screenshot({path:path.join(out,'beach-complete.png')});await saved();
 await page.reload();await ready();await key('Continue');await garden();await key('Adventures');await key("Pip's runaway beach ball · Revisit");
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});assert.equal((await state()).coins,0);
 report.checks.push('Portrait drag and scene clicks solve the quest; completed revisit retains progress and grants no base rewards');
 await key('Back to garden');await garden();await saved();
 report.final=await state();assert.equal(report.final.sceneCount,1);assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}})();
