// Isolated generated Paws preview; no production registration or save mutation.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const sha=data=>createHash('sha256').update(data).digest('hex');
const {PNG}=require('pngjs');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-cats/site');
const evidence=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-02/browser');
const port=Number(process.env.PAWS_TEST_PORT||19612);
const cats=['pip','mabel','button','pepper'];
const labels=['Relaxed idle / blink','Attentive request','Happy pounce','Garden walk','Settle on cushion','Curled rest','Approach bowl','Drink','Return to idle','Scratch','Perch step','Perch settle','Playhouse enter','Playhouse lookout','Picnic sit','Personality reaction'];
function difference(a,b){a=PNG.sync.read(a);b=PNG.sync.read(b);let count=0;for(let i=0;i<a.data.length;i+=4)if(Math.abs(a.data[i]-b.data[i])+Math.abs(a.data[i+1]-b.data[i+1])+Math.abs(a.data[i+2]-b.data[i+2])>20)count++;return count;}
(async()=>{
 await fs.mkdir(evidence,{recursive:true});
 const server=http.createServer(async(req,res)=>{
   const url=new URL(req.url,'http://localhost');const target=path.resolve(site,'.'+decodeURIComponent(url.pathname==='/'?'/index.html':url.pathname));
   if(!target.startsWith(site+path.sep)){res.writeHead(403).end();return;}
   try{const data=await fs.readFile(target);res.setHeader('Content-Type',({'.js':'text/javascript','.wasm':'application/wasm','.html':'text/html','.css':'text/css','.glb':'model/gltf-binary','.png':'image/png','.svg':'image/svg+xml'})[path.extname(target)]||'application/octet-stream');res.setHeader('Cache-Control','no-store');res.end(data);}catch{res.writeHead(404).end();}
 });
 await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const report={browser:browser.version(),site,wasmSha256:sha(await fs.readFile(path.join(site,'release1_preview.wasm'))),errors:[],cats:[],scope:'Automated desktop WebGPU; responsive emulation is not physical mobile acceptance'};
 try{
  for(const cat of cats){
   const page=await browser.newPage({viewport:{width:1280,height:1100},deviceScaleFactor:1});
   const record={cat,glbSha256:sha(await fs.readFile(path.join(site,'candidates',cat+'.glb'))),requests:[],actions:[]};report.cats.push(record);
   page.on('pageerror',e=>report.errors.push(cat+': '+String(e)));
   page.on('console',m=>{if(m.type()==='error')report.errors.push(cat+': '+m.text());});
   page.on('response',r=>{if(r.url().includes('/candidates/'))record.requests.push({url:r.url(),status:r.status()});});
   await page.goto(`http://127.0.0.1:${port}/?cat=${cat}`);
   await page.waitForFunction(()=>window.pipState==='ready'&&window.pawsRenderState?.state==='ready',null,{timeout:60000});
   await page.getByLabel('Reduced motion',{exact:true}).check();
   const canvas=page.locator('#viewport');
   for(let i=0;i<labels.length;i++){
    await page.getByRole('button',{name:labels[i],exact:true}).click();await page.waitForTimeout(60);
    assert.equal(await page.evaluate(()=>window.pipAction),i);
    const before=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(50);assert.equal(await page.evaluate(()=>Module._pip_time()),before);
    await canvas.screenshot({path:path.join(evidence,`${cat}-static-${i}.png`)});record.actions.push(labels[i]);
   }
   await page.getByRole('button',{name:labels[3],exact:true}).click();await page.getByLabel('Reduced motion',{exact:true}).uncheck();
   await page.waitForTimeout(250);const a=await canvas.screenshot();await page.waitForTimeout(330);const b=await canvas.screenshot();
   record.walkChangedPixels=difference(a,b);assert(record.walkChangedPixels>100,'Animated walk must change rendered pixels');
   await page.evaluate(()=>Module._pip_suspend(1));const suspended=await page.evaluate(()=>Module._pip_time());await page.waitForTimeout(150);assert.equal(await page.evaluate(()=>Module._pip_time()),suspended);await page.evaluate(()=>Module._pip_suspend(0));
   for(const [action,seconds,label] of [[0,2.533333,'closed-blink'],[5,0,'rest'],[9,.5,'scratch'],[10,.6,'step']]){
    await page.evaluate(([a,t])=>Module._cat_sample(a,t),[action,seconds]);await page.waitForTimeout(80);
    await canvas.screenshot({path:path.join(evidence,`${cat}-${label}.png`)});
   }
   const eyes=[];
   for(const [seconds,label] of [[2.25,'eye-open'],[2.533333,'eye-closed']]){
    await page.evaluate(t=>Module._pip_blink_review(t,0),seconds);await page.waitForTimeout(80);
    eyes.push(await canvas.screenshot({path:path.join(evidence,`${cat}-${label}.png`)}));
   }
   record.blinkChangedPixels=difference(eyes[0],eyes[1]);assert(record.blinkChangedPixels>100,'Blink must change visible eye pixels');
   await page.evaluate(()=>Module._cat_sample(0,0));
   await page.getByLabel('Desktop shadows',{exact:true}).uncheck();await page.getByRole('button',{name:'Puzzle distance',exact:true}).click();await page.waitForTimeout(100);
   await canvas.screenshot({path:path.join(evidence,`${cat}-shadow-free-distance.png`)});
   await page.setViewportSize({width:390,height:844});await page.waitForTimeout(100);await page.screenshot({path:path.join(evidence,`${cat}-portrait-layout.png`),fullPage:true});
   assert(record.requests.some(r=>r.status===200&&r.url.endsWith('/'+cat+'.glb')),'Missing actual exported model HTTP request');
   await page.close();
  }
  assert.equal(report.errors.length,0,report.errors.join('\n'));report.passed=true;
 }finally{await fs.writeFile(path.join(evidence,'report.json'),JSON.stringify(report,null,2));await browser.close();await new Promise(resolve=>server.close(resolve));}
 console.log(JSON.stringify(report,null,2));
})().catch(e=>{console.error(e);process.exitCode=1;});
