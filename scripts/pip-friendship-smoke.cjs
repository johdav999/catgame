const {chromium}=require('playwright');const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/pip-friendship/${backend}`,report={backend,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 for(const q of (process.env.QUEST_ONLY?[Number(process.env.QUEST_ONLY)]:[12,13,14])){
 const context=await browser.newContext({viewport:{width:1600,height:1000}});if(backend==='webgl2')await context.addInitScript(()=>{if(navigator.gpu)navigator.gpu.requestAdapter=async()=>null;});page=await context.newPage();page.setDefaultTimeout(60000);page.on('pageerror',e=>report.errors.push(e.message));
 const state=()=>page.evaluate(()=>pawsState),settle=()=>page.waitForTimeout(300),ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){const w=await page.evaluate(name=>{const e=[...document.querySelectorAll('button[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return e&&EngineGui.snapshot().widgets.find(w=>w.id===+e.dataset.widgetId);},name);assert(w,'Button '+name);const b=w.bounds,c=w.clip,x=b.x+b.width/2,y=b.y+b.height/2;assert(x>=c.x&&x<=c.x+c.width&&y>=c.y&&y<=c.y+c.height,'Visible button '+name);await page.mouse.click(x,y);await settle();}
 async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
 async function click(id){const p=(await points()).objects[id];assert(p,'Missing '+id);await page.mouse.click(p.x,p.y);await settle();}
 async function drag(id,target,cancel=false){const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to,JSON.stringify({id,target,p}));const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];const b=/^\d+,\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});if(cancel)await page.evaluate(()=>window.dispatchEvent(new Event('blur')));await page.mouse.up();await settle();}
 await page.goto('http://127.0.0.1:8160/');await ready();report.build=await page.evaluate(()=>pawsBuild);
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},await fs.readFile(`build/evidence/pip-friendship/${q}.save`,'utf8'));
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);await settle();
 const initial=await state();await page.screenshot({path:path.join(out,`${q}-start.png`)});
 const delta=[[0,-1],[1,0],[0,1],[-1,0]];
 for(let n=0;n<initial.solution.length;n++){
 const s=initial.solution[n];if(s.verb===4)continue;const before=await state();if(before.completed)break;console.log(q,n,s.object,s.target);
 if(s.verb===1){const o=before.objects.find(o=>o.id===s.object),d=delta[s.direction];await drag(s.object,`${o.x+d[0]},${o.y+d[1]}`);const moved=(await state()).objects.find(o=>o.id===s.object);assert.equal(moved.x,o.x+d[0]);assert.equal(moved.y,o.y+d[1]);}
 else if(s.verb===2)await click(s.object);else if(s.verb===3){await click('source_ball');await page.waitForFunction(()=>!pawsBall.rolling);}
 else if(s.verb===5)await drag(s.object,s.target);else if(s.verb===21)await key('Test balance');else if(s.verb===6){assert((await state()).completed);continue;}else throw Error('Unmapped '+JSON.stringify(s));
 assert((await state()).revision>before.revision,JSON.stringify({q,s,message:(await state()).message}));
 if(q===12&&n===7){assert(!(await state()).completed);await page.waitForTimeout(1500);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);report.checks.push('Ball checkpoint reload');}
 }
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});assert.equal((await state()).coins,initial.coins+20);await page.screenshot({path:path.join(out,`${q}-complete.png`)});report.checks.push('Real gestures completed '+q);
 if(q===14){await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await click('lookout_seat');await page.waitForFunction(()=>pawsGarden[0].item==='pip_lookout'&&pawsGarden[0].phase===4,null,{timeout:60000});await page.waitForTimeout(6500);assert.equal((await page.evaluate(()=>pawsGarden))[0].phase,4);await page.screenshot({path:path.join(out,'quiet-lookout.png')});await key('Leave lookout');await page.waitForTimeout(1500);await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await page.setViewportSize({width:390,height:844});await settle();await key('Sit with Pip');await page.waitForFunction(()=>pawsGarden[0].phase===4,null,{timeout:60000});await page.screenshot({path:path.join(out,'quiet-portrait.png')});report.checks.push('Persistent lookout, untimed rest, leave, reload and portrait');}
 await context.close();
 }assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,garden:window.pawsGarden,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({passed:report.passed,checks:report.checks,failure:report.failure}));}})();
