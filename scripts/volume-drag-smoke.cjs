const {chromium}=require('playwright');
const {PNG}=require('pngjs');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu';
const out=`build/evidence/audio-controls/${backend}`,report={backend,checks:[],errors:[]};
(async()=>{let browser,page;try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1280,height:900},hasTouch:true});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const settle=()=>page.waitForTimeout(250);
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();}
 async function value(name,expected){await page.getByRole('button',{name:`${name}: ${expected}%`,exact:true}).waitFor();}
 async function bars(){
  const png=PNG.sync.read(await page.screenshot()),x=png.width<600?36:404,groups=[];
  for(let y=0;y<png.height;y++){const i=(y*png.width+x)*4,hit=png.data[i]===183&&png.data[i+1]===214&&png.data[i+2]===193;
   if(hit){const last=groups.at(-1);if(last&&last.end===y-1)last.end=y;else groups.push({start:y,end:y});}}
  return groups.filter(g=>g.end-g.start>20).map(g=>({x:png.width/2,y:(g.start+g.end)/2}));
 }
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8080/');await ready();
 report.build=await page.evaluate(()=>pawsBuild);assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 await key('Settings');let b=await bars();assert.equal(b.length,3,'Three visible fill bars inside the percentage rectangles');
 await page.mouse.move(b[0].x,b[0].y);await page.mouse.down();await page.mouse.move(b[0].x,b[0].y-40,{steps:12});await settle();
 await value('Music',90);assert.equal(await page.evaluate(()=>pawsHostState().owner),'volume');await page.mouse.up();await settle();
 b=await bars();await page.mouse.move(b[0].x,b[0].y);await page.mouse.down();await page.mouse.move(b[0].x,b[0].y+80,{steps:12});await page.mouse.up();await settle();await value('Music',50);
 await page.getByRole('button',{name:'Music: 50%',exact:true}).focus();await page.keyboard.press('ArrowDown');await settle();await value('Music',49);
 await page.keyboard.press('Home');await settle();await value('Music',0);await page.keyboard.press('End');await settle();await value('Music',100);
 await page.keyboard.press('PageDown');await settle();await value('Music',90);
 await page.screenshot({path:`${out}/desktop.png`});
 report.checks.push('Mouse up/down drag, live percent/fill, keyboard endpoints and fine adjustment');
 await page.waitForFunction(()=>pawsState.save===2);await page.reload();await ready();await key('Settings');await value('Music',90);
 report.checks.push('Chosen percentage survives full reload');
 await page.setViewportSize({width:390,height:844});await settle();
 await page.getByRole('button',{name:'Music: 90%',exact:true}).focus();await settle();b=await bars();assert(b.length>=1);
 const cdp=await context.newCDPSession(page),p=b[0];
 await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:p.x,y:p.y}]});
 for(let n=1;n<=10;n++)await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:p.x,y:p.y+n*8}]});
 await settle();await value('Music',50);assert.equal(await page.evaluate(()=>pawsHostState().owner),'volume');
 await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await settle();
 b=await bars();assert(Math.abs(b[0].y-p.y)<2,'Volume drag does not scroll its containing settings panel');
 await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:p.x,y:p.y}]});
 await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:p.x,y:p.y-20}]});await settle();await value('Music',60);
 await cdp.send('Input.dispatchTouchEvent',{type:'touchCancel',touchPoints:[]});await settle();
 assert.equal(await page.evaluate(()=>pawsHostState().owner),null);await value('Music',60);
 await page.screenshot({path:`${out}/mobile.png`});report.checks.push('Emulated finger drag owns input without scrolling; touch cancellation keeps chosen value and releases capture');
 // The text above/between controls still scrolls on narrow displays.
 await page.mouse.move(190,300);await page.mouse.wheel(0,500);await settle();
 await page.screenshot({path:`${out}/mobile-scrolled.png`});
 await page.getByRole('button',{name:'Effects: 80%',exact:true}).focus();await page.keyboard.press('ArrowUp');await settle();await value('Effects',81);
 await page.getByRole('button',{name:'Ambience: 60%',exact:true}).focus();await page.keyboard.press('ArrowDown');await settle();await value('Ambience',59);
 report.checks.push('All three percentage controls remain reachable after scrolling');
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;await page?.screenshot({path:`${out}/failure.png`}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(`${out}/report.json`,JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));}})();
