// Focused furnishing playback after an accepted Pip asset update. The fixture is
// the real earned/bought/placed save retained by the full phase-7 browser journey.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase7-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/phase7-use');
const save=path.resolve(process.env.SAVE_FIXTURE||'build/evidence/phase7/played-afternoon.save');
const port=Number(process.env.PAWS_TEST_PORT||19488);
let browser,page,server;const report={site,save,checks:[],errors:[],screenshots:[]};
async function action(label){
    for(let i=0;i<120;i++){
        if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===label){await page.keyboard.press('Enter');await page.waitForTimeout(180);return;}
        await page.keyboard.press('Tab');await page.waitForTimeout(8);
    }
    throw Error('Unreachable '+label);
}
async function shot(name){await page.screenshot({path:path.join(out,name+'.png')});report.screenshots.push(name+'.png');}
async function loaded(){await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');}
async function phase(n){await page.waitForFunction(n=>pawsPip.usePhase===n,n,{timeout:30000});}
(async()=>{
    await fs.mkdir(out,{recursive:true});
    server=http.createServer(async(req,res)=>{try{
        const pathname=decodeURIComponent(new URL(req.url,'http://localhost').pathname);
        const file=path.resolve(site,'.'+(pathname==='/'?'/index.html':pathname));assert(file.startsWith(site+path.sep));
        res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.png':'image/png'})[path.extname(file)]||'application/octet-stream');
        res.end(await fs.readFile(file));
    }catch{res.statusCode=404;res.end();}});
    await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
    const hash=async file=>createHash('sha256').update(await fs.readFile(path.join(site,file))).digest('hex');
    report.wasmSha256=await hash('paws.wasm');report.pipSha256=await hash('assets/models/pip/pip.glb');
    browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});report.browser=browser.version();
    page=await browser.newPage({viewport:{width:1440,height:900}});
    page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
    await page.goto(`http://127.0.0.1:${port}`);await loaded();
    await page.evaluate(async bytes=>new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'}).put('current',new TextEncoder().encode(bytes)),await fs.readFile(save,'utf8'));
    await page.reload();await loaded();await action('Continue');await page.waitForFunction(()=>pawsState.scene==="Pip's courtyard");
    assert.equal(await page.getByRole('button',{name:'Meet Pip',exact:true}).count(),1);
    await action('Menu');await action('Settings');await action('Motion: Reduced');await action('Back');await action('Continue');
    await action('Menu');await action('Shop');await action('Garden cushion · Owned');await action('Invite Pip to rest');
    await phase(2);await shot('settling');await phase(3);await shot('rest-on-cushion');await phase(0);await shot('returned-idle');
    report.checks.push('Accepted merged Pip: real authored approach, settle/rest on cushion and bounded return in the production courtyard');
    await action('Menu');await action('Shop');await action('Drinking bowl · Owned');await action('Invite Pip to drink');await phase(3);await shot('drink');
    await action('Menu');await action('Settings');await action('Motion: Full');await action('Back');await action('Continue');await phase(3);
    assert(await page.evaluate(()=>pawsPip.staticPose));await shot('reduced-drink');
    await action('Menu');await action('Replay afternoon moment');await shot('replay-reduced-rest');await action('Continue in garden');
    const state=await page.evaluate(()=>pawsState);assert.equal(state.coins,5);assert.equal(state.cushionOwned,1);assert.equal(state.bowlOwned,1);assert.equal(state.endingSeen,1);
    assert.equal(state.cushionSpot,'bench_corner');assert.equal(state.bowlSpot,'quiet_corner');
    report.checks.push('Reduced drinking/rest replay, no automatic ending on restore and unchanged five-coin ownership/placement ledger');
    assert.equal(await hash('paws.wasm'),report.wasmSha256);assert.equal(await hash('assets/models/pip/pip.glb'),report.pipSha256);assert.equal(report.errors.length,0);
    report.passed=true;
})().catch(e=>{report.failure=e.stack;process.exitCode=1;}).finally(async()=>{
    await fs.mkdir(out,{recursive:true});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));
    await browser?.close();if(server)await new Promise(resolve=>server.close(resolve));
});
