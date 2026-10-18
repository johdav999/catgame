const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const out=path.resolve('build/evidence/new-game/picnic-replay');let browser,page;const report={errors:[]};
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1440,height:900}});page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(150);}
 await page.goto(process.env.PAWS_URL||'http://localhost:8080/');await ready();
 const fixture=await fs.readFile('build/evidence/release1/prompt-09/browser-final/complete.save','utf8');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},fixture);
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&[1,5].includes(pawsUi.screen));
 if(await page.evaluate(()=>pawsUi.screen===5)){await key('Continue');await page.waitForFunction(()=>pawsUi.screen===1);}
 assert(await page.evaluate(()=>pawsState.picnicSeen),'Fixture has a legally recorded picnic');const coins=await page.evaluate(()=>pawsState.coins);
 await key('Menu');await key('Requests & friends');await key('The garden bells · Practice');await key('Help Pepper');await page.waitForFunction(()=>!pawsState.garden&&pawsUi.screen===2);
 await key('Menu');assert(await page.getByRole('button',{name:'Replay picnic celebration',exact:true}).isEnabled());
 await page.screenshot({path:path.join(out,'puzzle-menu.png')});await key('Replay picnic celebration');await page.waitForFunction(()=>pawsUi.screen===13&&pawsState.garden);
 assert.equal(await page.evaluate(()=>pawsState.coins),coins);await page.screenshot({path:path.join(out,'replayed-celebration.png')});
 await key('Continue in garden');await page.waitForFunction(()=>pawsUi.screen===1&&pawsState.garden);assert.equal(await page.evaluate(()=>pawsState.coins),coins);
 assert.deepEqual(report.errors,[]);report.passed=true;report.build=await page.evaluate(()=>pawsBuild);
})().catch(async e=>{report.failure=e.stack;process.exitCode=1;console.error(e);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();});
