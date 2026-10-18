// Prompt 7 acceptance through the actual generated game's semantic/canvas input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase7-pinned-release/site');
const out=process.env.EVIDENCE_DIR||'build/evidence/phase7';
const port=Number(process.env.PAWS_TEST_PORT||19487);
const report={build:'first-afternoon-phase7-v1',site,checks:[],screenshots:[],errors:[]};
const audioHarness=process.env.PAWS_AUDIO_TEST==='1'?require('./phase9-audio-harness.cjs'):null;
let browser,page,server,mode='keyboard';
const settle=()=>page.waitForTimeout(140);
async function focus(name){for(let n=0;n<120;n++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name)return;await page.keyboard.press('Tab');await page.waitForTimeout(8);}throw Error('Unreachable '+name);}
async function action(name){
    console.log('Action:',name);
    // Logical completion commits before the authored ball travel/reaction ends.
    // Wait for the actionable reward card rather than racing its publication.
    if(name==='Back to garden'&&await page.evaluate(()=>!!window.pawsBall?.completionPending))
        await page.waitForFunction(()=>!window.pawsBall?.completionPending,null,{timeout:30000});
    await settle();await focus(name);await settle();
    if(mode==='keyboard')await page.keyboard.press('Enter');
    else {const b=await page.evaluate(()=>EngineGui.snapshot().widgets.find(w=>w.focused).bounds);if(mode==='touch')await page.touchscreen.tap(b.x+b.width/2,b.y+b.height/2);else await page.mouse.click(b.x+b.width/2,b.y+b.height/2);}
    await settle();
    if(audioHarness)await audioHarness.observe(page);
}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
const state=()=>page?page.evaluate(()=>pawsState):Promise.resolve(null);
async function saved(){await page.waitForFunction(()=>pawsState.save===2);}
async function open(url){await page.goto(url);await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');await settle();}
async function home(){await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");await settle();}
async function help(){await action('Help Pip');await page.waitForFunction(()=>[...document.querySelectorAll('[data-widget-id]')].some(w=>['Objects & actions','Actions'].includes(w.getAttribute('aria-label'))));await settle();}
async function actions(){await action((await page.viewportSize()).height<500?'Actions':'Objects & actions');}
async function rotate(name,count){await action(name);for(let n=0;n<count;n++)await action('Rotate');}
async function shop(){await action('Menu');await action('Shop');}
async function next(title){await action('Meet Pip');await action(title);await help();await actions();}
async function disabled(label){assert(await page.getByRole('button',{name:label,exact:true}).isDisabled());}
async function usePhase(phase){await page.waitForFunction(n=>pawsPip?.usePhase===n,phase,{timeout:30000});}
(async()=>{
    await fs.mkdir(out,{recursive:true});
    server=http.createServer(async(req,res)=>{try{
        const pathname=decodeURIComponent(new URL(req.url,'http://localhost').pathname);
        const file=path.resolve(site,'.'+(pathname==='/'?'/index.html':pathname));if(!file.startsWith(site+path.sep))throw Error('Outside site');
        const bytes=await fs.readFile(file);res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png'})[path.extname(file)]||'application/octet-stream');res.end(bytes);
    }catch{res.statusCode=404;res.end('Missing file');}});
    await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
    const url=`http://127.0.0.1:${port}`;report.url=url;
    report.wasmSha256=createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex');
    report.assets={};for(const file of ['assets/models/pip/pip.glb','assets/models/pip/portrait-ending.png','assets/models/garden/cushion.glb','assets/models/garden/bowl.glb','assets/models/garden/cushion-preview.png','assets/models/garden/bowl-preview.png'])
        report.assets[file]=createHash('sha256').update(await fs.readFile(path.join(site,file))).digest('hex');
    browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
    const context=await browser.newContext({viewport:{width:1440,height:900},hasTouch:true});
    if(audioHarness)await audioHarness.attach(context);
    page=await context.newPage();
    page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
    await open(url);report.runtimeBuild=await page.evaluate(()=>window.pawsBuild||null);if(report.runtimeBuild)report.build=report.runtimeBuild.buildId;
    await action('Play');await home();await action('Menu');await disabled('Shop');await shot('shop-locked');await action('Continue');
    await action('Meet Pip');await help();await actions();
    if(audioHarness){await action('Roll ball');await action('Hint');await action('Back');await action('Left pot');await action('Move north');await action('Undo');}
    if(process.env.PAWS_UAT==='1'){
        await action('Roll ball');assert(!(await state()).completed);await shot('blocked-roll');
        await action('Hint');await shot('first-hint');await action('Next hint');await action('Next hint');await shot('all-hints');await action('Back');
        await action('Left pot');const potBefore=(await state()).objects[0].y;await action('Move north');await action('Undo');assert.equal((await state()).objects[0].y,potBefore);
        report.checks.push('Blocked roll preserves source, three free hints progressively reveal, undo restores the moved pot');
    }
    await action('Left pot');await action('Move north');await action('Move north');await action('Middle pot');await action('Move west');
    for(const [name,n]of[['Source straight',1],['Lower bend',3],['Middle straight',1],['Upper bend',1],['Finish straight',1]])await rotate(name,n);
    await action('Roll ball');await saved();assert.equal((await state()).coins,20);await action('Back to garden');await home();
    await shop();await shot('shop-first-reward');await action('Garden cushion · 30 coins');await disabled('Buy');await shot('insufficient-funds');await action('Back');await action('Back');
    await next('Tidy the toy basket');await action('Blocking cushion');await action('Move north');
    for(const [toy,socket]of[['Coral ball','Round ball recess'],['Toy mouse','Mouse-shaped recess'],['Toy fish','Fish-shaped recess']]){await action(toy);await action('Pick up '+toy);await action('Place in '+socket);}
    await action('Back to garden');await home();await next('The stuck little gate');await action('Sliding pot');await action('Move north');
    await rotate('Source straight',1);await rotate('Lower bend',3);await rotate('Switch bend',1);await action('Roll ball');
    if(audioHarness){await action('Return ball to source');await action('Roll ball');}
    await action('Mouse behind gate');await action('Pick up Mouse behind gate');await saved();assert.equal((await state()).coins,60);assert.equal((await state()).friendship,3);
    await action('Back to garden');await home();assert.equal((await state()).endingSeen,0);
    report.checks.push('All three quests earned 60 coins/3 friendship through keyboard; shop locked before quest one and purchases disabled at 20 coins');
    mode='mouse';await shop();await shot('shop-available');await action('Garden cushion · 30 coins');await action('Buy');await saved();assert.equal((await state()).coins,30);await disabled('Owned');
    await action('Choose a spot');await action('Sunny corner');await shot('cushion-preview');await action('Cancel');assert.equal((await state()).cushionSpot,'');assert.equal((await state()).cushionOwned,1);assert.equal((await state()).coins,30);
    await action('Back');await action('Drinking bowl · 25 coins');await action('Buy');await saved();await disabled('Owned');assert.equal((await state()).coins,5);await shot('bowl-owned');await action('Back');
    await action('Garden cushion · Owned');await action('Choose a spot');await action('Sunny corner');await action('Place');await saved();assert.equal((await state()).cushionSpot,'sunny_corner');assert.equal((await state()).endingSeen,1);
    await shot('ending-card');await usePhase(3);await shot('ending-cushion-rest');await action('Continue in garden');await home();
    report.checks.push('Mouse earn/buy/cancel/place; unique cushion and bowl cost 55 coins, leaving five; first placement records ending once');
    await shop();await action('Drinking bowl · Owned');await action('Choose a spot');await action('Sunny corner');await disabled('Place');await shot('occupied-spot');await action('Cancel');assert.equal((await state()).bowlSpot,'');
    await action('Choose a spot');await action('Quiet nook');await action('Place');await saved();assert.equal((await state()).bowlSpot,'quiet_corner');
    if(process.env.WALK_EVIDENCE){
        const folder=path.resolve(process.env.WALK_EVIDENCE);await fs.mkdir(folder,{recursive:true});
        const samples=[];
        for(let frame=0;frame<48;frame++){
            const pip=await page.evaluate(()=>pawsPip);
            if(frame&&pip.usePhase!==1)break;
            samples.push({frame,wall:Date.now(),pip});
            await page.screenshot({path:path.join(folder,`garden-walk-${String(frame).padStart(3,'0')}.png`)});
            await page.waitForTimeout(60);
        }
        assert(samples.some(s=>s.pip.usePhase===1),'Expected moving Pip in the actual garden');
        await fs.writeFile(path.join(folder,'garden-walk-samples.json'),JSON.stringify(samples,null,2));
        report.checks.push('Captured actual garden traversal while the reference walk plays');
    }
    await usePhase(3);await shot('pip-drinking');const t=await page.evaluate(()=>pawsPip.time);await page.waitForTimeout(300);assert.notEqual(await page.evaluate(()=>pawsPip.time),t);
    await action('Menu');await action('Settings');await action('Effects: 80%');await action('Back');await action('Continue');
    assert.equal(await page.evaluate(()=>pawsPip.usePhase),3,'Volume preference must preserve the paused use phase');
    await page.waitForFunction(()=>pawsPip.usePhase===0,{timeout:30000});await shot('bounded-use-idle');
    report.checks.push('Occupied spot rejects; bowl placement uses authored approach/lower/drink/return with actual animation and bounded idle return; volume preference preserves use');
    await shop();await action('Garden cushion · Owned');await action('Choose a spot');await action('Beside the bench');await action('Cancel');assert.equal((await state()).cushionSpot,'sunny_corner');
    await action('Choose a spot');await action('Beside the bench');await action('Place');await saved();assert.equal((await state()).cushionSpot,'bench_corner');assert.equal((await state()).coins,5);
    await usePhase(3);await shot('pip-cushion-rest');await page.waitForFunction(()=>pawsPip.usePhase===0,{timeout:30000});
    await shop();await action('Garden cushion · Owned');await action('Choose a spot');await action('Sunny corner');await page.keyboard.press('Escape');await settle();assert.equal((await state()).cushionSpot,'bench_corner');await action('Back');await action('Back');
    report.checks.push('Cancel and Escape preserve old placement; committed move costs zero coins, cushion settle/rest and free movement verified');
    await saved();const before=await state();await open(url);await action('Continue');await home();assert.equal((await state()).coins,5);assert.equal((await state()).cushionSpot,before.cushionSpot);assert.equal((await state()).bowlSpot,before.bowlSpot);assert.equal((await state()).endingSeen,1);
    assert(await page.getByRole('button',{name:'Meet Pip',exact:true}).count());await shot('restored-garden-no-ending');
    await action('Menu');await action('Replay afternoon moment');await shot('replayed-ending');await action('Continue in garden');await home();
    report.checks.push('Reload restores coherent wallet/ownership/placements/ending and does not automatically repeat ending; menu replay works');
    for(const [width,height]of[[390,844],[844,390]]){
        mode='touch';await page.setViewportSize({width,height});await settle();await action('Menu');await action('Settings');
        if(!await page.getByRole('button',{name:'Text size: Larger',exact:true}).count())await action('Text size: Standard');
        if(!await page.getByRole('button',{name:'Motion: Reduced',exact:true}).count())await action('Motion: Full');
        await action('Back');await action('Continue');await shop();await shot(`${width}x${height}-shop-larger`);
        await action('Drinking bowl · Owned');await action('Choose a spot');await action('Sunny corner');await shot(`${width}x${height}-placement-larger`);
        for(const b of await page.evaluate(()=>EngineGui.snapshot().widgets.filter(w=>w.type==='button'&&w.active)))assert(b.bounds.width>=44&&b.bounds.height>=44);
        await action('Cancel');await action('Invite Pip to drink');await usePhase(3);await shot(`${width}x${height}-reduced-drink`);
        assert(await page.evaluate(()=>pawsPip.staticPose));const poseTime=await page.evaluate(()=>pawsPip.time);await page.waitForTimeout(300);assert.equal(await page.evaluate(()=>pawsPip.time),poseTime);assert.equal((await state()).coins,5);
    }
    report.checks.push('390x844 and 844x390 with larger text, touch controls >=44px, real placement previews and reduced-motion static drinking pose');
    assert.equal(createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex'),report.wasmSha256);assert.equal(report.errors.length,0,report.errors.join('\n'));
    for(const [file,hash]of Object.entries(report.assets))assert.equal(createHash('sha256').update(await fs.readFile(path.join(site,file))).digest('hex'),hash,'Asset changed during acceptance: '+file);
    if(audioHarness)await audioHarness.finish(page,report,action,out);
    report.final=await state();report.passed=true;
    await saved();const snapshot=await page.evaluate(async()=>new TextDecoder().decode(await new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'}).get('current')));
    await fs.writeFile(path.join(out,'played-afternoon.save'),snapshot);
})().catch(async error=>{report.failure=error.stack;report.state=await state().catch(()=>null);process.exitCode=1;}).finally(async()=>{
    if(page)await shot('final-state').catch(()=>{});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));await browser?.close();await new Promise(resolve=>server?server.close(resolve):resolve());
});
