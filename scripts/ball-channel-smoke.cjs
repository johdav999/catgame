const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const site=path.resolve(process.env.SITE_DIR||'build/web-pinned-release/site');
const out=path.resolve('build/evidence/ball-channel'),port=19642,base=`http://127.0.0.1:${port}`;
const report={site,checks:[],errors:[],traversals:[]};let server,browser,page;
async function action(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(100);}
async function select(title){await action('Requests & friends');await action(title+' · Available');await action('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2);}
async function solve(){
 for(const step of await page.evaluate(()=>pawsState.solution)){
  if(await page.evaluate(()=>pawsUi.screen)!==7)await action('Objects & actions');
  let state=await page.evaluate(()=>pawsState);
  if(step.object){const i=state.objects.findIndex(o=>o.id===step.object);await action(`${i+1}. ${state.objects[i].name}`);}
  state=await page.evaluate(()=>pawsState);
  const choice=state.actions.find(a=>a.verb===step.verb&&a.object===step.object&&a.target===step.target&&a.direction===step.direction&&(step.verb!==5||(a.x===step.x&&a.y===step.y)));
  assert(choice?.enabled,JSON.stringify(step));await action(choice.label);
 }
}
(async()=>{
 await fs.mkdir(out,{recursive:true});
 server=http.createServer(async(req,res)=>{try{const file=path.resolve(site,'.'+decodeURIComponent(new URL(req.url,base).pathname));assert(file.startsWith(site+path.sep));const bytes=await fs.readFile(file.endsWith(path.sep)?file+'index.html':file);res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css'})[path.extname(file)]||'application/octet-stream');res.end(bytes);}catch{res.statusCode=404;res.end();}});
 await new Promise(r=>server.listen(port,'127.0.0.1',r));
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:1440,height:900}});
 page.on('pageerror',e=>report.errors.push(e.message));
 await page.goto(base+'/index.html');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
 await action('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 for(const title of ["Pip's missing ball",'Tidy the toy basket','The stuck little gate']){
  await select(title);await solve();
  if(title!=='Tidy the toy basket'){
   const samples=[];
   for(let i=0;i<180;i++){
    const sample=await page.evaluate(()=>({...pawsBall,screen:pawsUi.screen}));samples.push(sample);
    assert(Math.abs(sample.centreY-sample.y-.105)<.00001,'centre stays one radius above authored surface');
    if(sample.rolling)assert.notEqual(sample.screen,9,'reward card waits for ball arrival');
    if(!sample.rolling)break;
    if(i===12)await page.screenshot({path:path.join(out,title.replaceAll(' ','-')+'-rolling.png')});
    await page.waitForTimeout(50);
   }
   assert(samples.length>20,'full animated journey sampled');assert(!samples.at(-1).rolling,'ball arrived');
   report.traversals.push({title,samples});report.checks.push(title+': stable channel clearance and completion after arrival');
  }
  await page.waitForFunction(()=>pawsUi.screen===9);await action('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 }
 assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(e=>{report.failure=e.stack;process.exitCode=1;}).finally(async()=>{if(page)await page.screenshot({path:path.join(out,'final.png')}).catch(()=>{});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({...report,traversals:report.traversals.map(t=>({title:t.title,samples:t.samples.length}))},null,2));await browser?.close();server?.close();});
