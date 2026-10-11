const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const url=process.env.PAWS_URL||'http://127.0.0.1:8080';
const out=process.env.EVIDENCE_DIR||'build/evidence/direct-controls';
const report={build:'direct-controls-v1',url,checks:[],screenshots:[],errors:[]};
let browser,page,mode='keyboard';
const settle=()=>page.waitForTimeout(160);
async function focus(name){for(let i=0;i<110;i++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name)return;await page.keyboard.press('Tab');await page.waitForTimeout(8);}throw Error('Unreachable '+name);}
async function action(name){
 await settle();await focus(name);await settle();
 if(mode==='keyboard')await page.keyboard.press('Enter');
 else {const b=await page.evaluate(name=>{const el=[...document.querySelectorAll('[data-widget-id]')].find(e=>e.getAttribute('aria-label')===name);return EngineGui.snapshot().widgets.find(w=>w.id===+el.dataset.widgetId).bounds;},name);if(mode==='touch')await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);else await page.mouse.click(b.x+b.width/2,b.y+b.height/2);}
 await settle();
}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function state(){return page.evaluate(()=>pawsState);}
async function open(){await page.goto(url);await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await settle();}
async function help(){await action('Help Pip');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>['Objects & actions','Actions'].includes(e.getAttribute('aria-label'))),{timeout:60000});await settle();}
async function actions(){await action((await page.viewportSize()).height<500?'Actions':'Objects & actions');}
async function next(title){await action('Back to garden');await action('Meet Pip');await action(title);await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Help Pip'));await help();await shot(mode+"-"+title.replace(/[^a-z0-9]/gi,"-")+"-board");await actions();}
async function saved(){await page.waitForFunction(()=>pawsState.save===2);}
let touch=false,cdp;
async function point(id,slot=false){return page.evaluate(([id,slot])=>{Module._paws_interaction_snapshot();return pawsInteraction[slot?'slots':'objects'][id];},[id,slot]);}
async function clickObject(id){const p=await point(id);if(touch)await page.touchscreen.tap(p.x,p.y);else await page.mouse.click(p.x,p.y);await page.waitForTimeout(220);}
async function dragObject(id,cell,cancel=false){const a=await point(id),b=await point(cell,cell.includes(','));
 if(touch){await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:a.x,y:a.y}]});for(let i=1;i<=8;i++)await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:a.x+(b.x-a.x)*i/8,y:a.y+(b.y-a.y)*i/8}]});await cdp.send('Input.dispatchTouchEvent',{type:cancel?'touchCancel':'touchEnd',touchPoints:[]});}
 else {await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:8});if(cancel)await page.evaluate(()=>dispatchEvent(new Event('blur')));await page.mouse.up();}
 await page.waitForTimeout(350);
}
const object=async id=>(await state()).objects.find(o=>o.id===id);
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 for(const input of ['mouse','touch']){
 touch=input==='touch';const context=await browser.newContext({viewport:touch?{width:390,height:844}:{width:1440,height:900},hasTouch:touch});page=await context.newPage();cdp=await context.newCDPSession(page);page.on('pageerror',e=>report.errors.push(e.message));
 await open();await action('Play');await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");await action('Meet Pip');await help();await page.waitForTimeout(500);
 if(!touch){await clickObject('pot_a');await page.keyboard.press('ArrowUp');await settle();assert.equal((await object('pot_a')).y,1,'keyboard movement alternative');await actions();await action('Undo');await action('Back');}await clickObject('source_ball');assert.equal((await state()).completed,0,'blocked roll stays incomplete');
 await dragObject('pot_a','1,1');await dragObject('pot_b','1,1');assert.equal((await object('pot_b')).x,2,'occupied slot rejected');
 await dragObject('pot_a','1,0',true);assert.equal((await object('pot_a')).y,1,'cancel rolls back');await actions();await action('Undo');await action('Back');
 await dragObject('pot_a','1,0');assert.equal((await object('pot_a')).y,0,'two-step drag committed');
 await dragObject('pot_b','1,1');assert.equal((await object('pot_b')).x,1,'middle pot moves after clearance');
 for(const [id,n]of[['channel_a',1],['channel_b',3],['channel_c',1],['channel_d',1],['channel_e',1]]){await clickObject(id);assert((await object(id)).selected);for(let i=0;i<n;i++)await clickObject(id);}
 await shot(input+'-direct-route');await clickObject('source_ball');assert.equal((await state()).completed,1,'tap ball commits solved route');
 const startRoll=await page.evaluate(()=>pawsBall);
 assert(startRoll.rolling&&startRoll.completionPending,'tap starts traversal before reward card');
 assert(!await page.evaluate(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Back to garden')),'completion card waits for arrival');
 await shot(input+'-rolling');
 const travel=[];
 for(let i=0;i<45;i++){travel.push(await page.evaluate(()=>pawsBall));if(!travel.at(-1).rolling)break;await page.waitForTimeout(120);}
 assert(travel.length>10,'journey stays visible');
 assert(travel.some(p=>Math.abs(p.x)<.03&&p.z<.4&&p.z>-.4),'ball travels through middle straight');
 const arrived=travel.at(-1);assert(!arrived.rolling&&!arrived.completionPending,'arrival finishes traversal');
 assert(Math.abs(arrived.x-2)<.001&&Math.abs(arrived.z+1)<.001,'ball reaches Pip destination');
 await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(e=>e.getAttribute('aria-label')==='Back to garden'));
 report.checks.push(input+': visible sampled travel through track, reward card after arrival');
 await shot(input+'-complete');
 await action('Back to garden');await action('Meet Pip');await action('Tidy the toy basket');await help();await page.waitForTimeout(400);
 await dragObject('ball','ball_recess');assert.equal((await object('ball')).location,0,'blocked toy drag rejected');
 await dragObject('cushion','1,0');assert.equal((await object('cushion')).y,0);
 await dragObject('ball','mouse_recess');assert.equal((await object('ball')).location,0,'wrong recess rejected');
 for(const id of ['ball','mouse','fish']){await dragObject(id,id+'_recess');assert.equal((await object(id)).location,2,'matching toy drop');}
 assert.equal((await state()).completed,1,'direct tidy completion');await shot(input+'-tidy-complete');
  await action('Back to garden');await action('Meet Pip');await action('The stuck little gate');await help();await page.waitForTimeout(400);
 await dragObject('pot','1,1');
 for(const [id,n]of[['channel_a',1],['channel_b',3],['channel_c',1]]){await clickObject(id);for(let i=0;i<n;i++)await clickObject(id);}
 await clickObject('source_ball');assert.equal((await state()).switchActive,1);await page.waitForFunction(()=>!pawsBall.rolling);
 await dragObject('mouse','0,0');assert.equal((await state()).completed,1,'direct gate retrieval');
 report.checks.push(input+': selection, selected-piece rotation, blocked/cancelled/legal multi-slot drag, ball roll and completion');await context.close();
 }
 assert.equal(report.errors.length,0);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.state=await state().catch(()=>null);if(page)await shot('failure').catch(()=>{});process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();});





