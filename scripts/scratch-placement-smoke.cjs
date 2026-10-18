// Real production shop/placement controls; isolated browser saves and server.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const site=path.resolve(process.env.SITE_DIR||'build/web-scratch-placement-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/scratch-placement');
const port=+(process.env.PAWS_TEST_PORT||19742),url=`http://127.0.0.1:${port}/`;
const report={site,url,checks:[],errors:[]};let browser,server,page;
const settle=()=>page.waitForTimeout(140);
const control=name=>page.locator(`button[data-widget-id][aria-label="${name}"]`);
async function key(name){await control(name).focus();await page.keyboard.press('Enter');await settle();}
async function widget(name){const id=+(await control(name).getAttribute('data-widget-id'));return page.evaluate(id=>EngineGui.snapshot().widgets.find(w=>w.id===id),id);}
async function pointer(name,touch=false){const w=await widget(name);assert(w.bounds.y>=w.clip.y&&w.bounds.y+w.bounds.height<=w.clip.y+w.clip.height,`Visible ${name}`);const x=w.bounds.x+w.bounds.width/2,y=w.bounds.y+w.bounds.height/2;if(touch)await page.touchscreen.tap(x,y);else await page.mouse.click(x,y);await settle();}
async function open(){await page.goto(url);await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready');}
async function home(){await page.waitForFunction(()=>pawsState.garden&&[1,5].includes(pawsUi.screen));if(await page.evaluate(()=>pawsUi.screen===5))await key('Continue');}
async function state(){return page.evaluate(()=>({coins:pawsState.coins,item:pawsState.items.find(i=>i.id==='scratching_post')}));}
(async()=>{
    await fs.mkdir(out,{recursive:true});
    server=http.createServer(async(req,res)=>{try{const u=decodeURIComponent(new URL(req.url,url).pathname);const file=path.resolve(site,'.'+(u==='/'?'/index.html':u));assert(file.startsWith(site+path.sep));res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.svg':'image/svg+xml','.png':'image/png'})[path.extname(file)]||'application/octet-stream');res.end(await fs.readFile(file));}catch(e){res.statusCode=404;res.end();}});
    await new Promise(r=>server.listen(port,'127.0.0.1',r));
    browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
    for(const [width,height] of [[585,791],[390,844],[844,390]])for(const large of [false,true]){
        const context=await browser.newContext({viewport:{width,height},hasTouch:true});page=await context.newPage();
        page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
        await open();assert.match(await page.title(),/little garden/);
        await key('Save and recovery');const chooser=page.waitForEvent('filechooser');await key('Import save');await (await chooser).setFiles(path.resolve('tests/release1/progression/fixtures/mvp-v2.save'));await key('Use imported afternoon');await key('Continue');await home();
        await key('Menu');await key('Settings');const current=await control('Text size: Larger').count();if(Boolean(current)!==large)await key(current?'Text size: Larger':'Text size: Standard');await key('Back');await key('Continue');
        await key('Menu');await key('Shop');await key('Scratching post · 20 coins');await key('Buy');assert.equal((await state()).coins,40);
        await key('Choose a spot');assert.equal(await control('Place').isDisabled(),false);assert.equal(await control('scratch corner · Fits').count(),1);assert.equal(await page.locator('button[aria-label$=" · Blocked"]').count(),0);
        const scratch=await widget('scratch corner · Fits'),place=await widget('Place');assert(scratch.bounds.y+scratch.bounds.height<=place.bounds.y,'Scratch corner above footer');
        await pointer('scratch corner · Fits');assert.equal(await control('Place').isDisabled(),false);
        await pointer('Cancel',true);assert.deepEqual(await state(),{coins:40,item:{id:'scratching_post',owned:1,spot:'',available:1}});
        await key('Choose a spot');await pointer('scratch corner · Fits',true);await page.screenshot({path:path.join(out,`${width}x${height}-${large?'large':'standard'}.png`)});await pointer('Place',true);await home();assert.equal((await state()).item.spot,'scratch_corner');assert.equal((await state()).coins,40);
        await page.waitForFunction(()=>pawsState.save===2);await open();await key('Continue');await home();assert.equal((await state()).item.spot,'scratch_corner');assert.equal((await state()).coins,40);
        // Reopening a placed furnishing previews its current spot; cancellation
        // must retain both the saved move and the already-paid purchase.
        await key('Menu');await key('Shop');await key('Scratching post · Owned');await key('Choose a spot');assert.equal(await control('Place').isDisabled(),false);await key('Cancel');assert.equal((await state()).item.spot,'scratch_corner');assert.equal((await state()).coins,40);
        report.checks.push({width,height,largerText:large,mouseSelection:true,touchPlace:true,cancelPreservesState:true,reload:true});await context.close();
    }
    assert.equal(report.errors.length,0,JSON.stringify(report.errors));report.success=true;
})().catch(e=>{report.failure=e.stack;console.error(e);process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();await new Promise(r=>server?server.close(r):r());console.log(JSON.stringify(report));});
