// Real backend plumbing with explicitly synthetic signals; no listening claim.
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http');
const assert=require('node:assert/strict'),{createHash}=require('node:crypto'),{chromium}=require('playwright');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-audio/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-08/browser');
function tone(seconds,frequency){
  const n=Math.floor(seconds*48000),b=Buffer.alloc(44+n*2);
  b.write('RIFF');b.writeUInt32LE(b.length-8,4);b.write('WAVEfmt ',8);b.writeUInt32LE(16,16);b.writeUInt16LE(1,20);b.writeUInt16LE(1,22);b.writeUInt32LE(48000,24);b.writeUInt32LE(96000,28);b.writeUInt16LE(2,32);b.writeUInt16LE(16,34);b.write('data',36);b.writeUInt32LE(n*2,40);
  for(let i=0;i<n;i++)b.writeInt16LE(Math.round(Math.sin(i*2*Math.PI*frequency/48000)*1600),44+i*2);
  return b;
}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  const errors=[],report={fixture:'Synthetic non-silent signals in Playwright routes only. No production files, audition or physical-output acceptance.'};
  const server=http.createServer(async(req,res)=>{try{
    const url=new URL(req.url,'http://local'),p=path.resolve(site,'.'+decodeURIComponent(url.pathname.replace(/^\/review/,'')));
    if(p!==site&&!p.startsWith(site+path.sep)){res.writeHead(403).end();return;}
    const file=p===site?path.join(site,'index.html'):p;
    const mime={'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.json':'application/json'};
    res.setHeader('Content-Type',mime[path.extname(file)]||'application/octet-stream');res.end(await fs.readFile(file));
  }catch{res.writeHead(404).end();}});
  await new Promise(r=>server.listen(19618,'127.0.0.1',r));
  let browser;
  try{
    browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
    report.browser=browser.version();
    const context=await browser.newContext(),page=await context.newPage();
    page.on('pageerror',e=>errors.push(e.message));
    const catalog=JSON.parse(await fs.readFile(path.join(site,'catalog.json'),'utf8')),tones=new Map();
    for(const [i,c] of catalog.clips.entries()){
      const bytes=tone(c.loop?1:.5,180+i*15);tones.set(c.file,bytes);
      Object.assign(c,{accepted:true,sha256:createHash('sha256').update(bytes).digest('hex'),source:{rightsRecord:'TEST FIXTURE ONLY'},review:{sourceAuditioned:true},pcm:{duration:c.loop?1:.5,channels:1}});
    }
    await context.route('**/catalog.json',r=>r.fulfill({json:catalog}));
    await context.route('**/assets/audio/**/*.wav',r=>{
      const file=new URL(r.request().url()).pathname.split('/assets/audio/')[1];return r.fulfill({contentType:'audio/wav',body:tones.get(file)});
    });
    await page.goto('http://127.0.0.1:19618/review/index.html');
    await page.waitForFunction(()=>window.audioPreview);
    assert.equal(await page.evaluate(()=>audioPreview.voices),0);
    await page.getByRole('button',{name:'Enable sound',exact:true}).click();
    await page.waitForFunction(()=>PawsAudioHost.stats.decoded===63); // 66 slots minus 3 on-demand music files.
    const index=family=>catalog.clips.findIndex(c=>c.family===family);
    const commit=async(family,stale=false)=>page.evaluate(([i,s])=>Module._audio_cue(i,s),[index(family),stale?1:0]);
    assert.equal(await commit('bell_leaf',true),0);
    assert.equal(await commit('bell_leaf'),1);
    await page.waitForFunction(()=>audioPreview.voices>0);
    for(const family of ['crate_slide','plank_place','shape_fit','bell_star','bell_paw','bell_reset','bowl_fill','mabel_request','mabel_happy','button_request','button_happy','pepper_request','pepper_happy','cat_scratch','playhouse_use']) {
      assert.equal(await commit(family),1);await page.waitForTimeout(140);
    }
    await page.getByRole('button',{name:'Replace scene'}).click();
    await page.waitForFunction(()=>audioPreview.voices===0);
    await commit('water_flow');await page.waitForFunction(()=>audioPreview.voices===1);
    await page.waitForTimeout(4350);assert.equal(await page.evaluate(()=>audioPreview.voices),0);
    await commit('water_flow');await page.waitForFunction(()=>audioPreview.voices===1);
    await page.getByLabel('Pause',{exact:true}).check();
    await page.waitForFunction(()=>audioPreview.voices===0);
    await page.getByLabel('Pause',{exact:true}).uncheck();
    assert.equal(await page.evaluate(()=>EngineAudioDevices.get(audioPreview.device).active),false);
    await page.getByRole('button',{name:'Enable sound',exact:true}).click();
    await page.getByRole('button',{name:'Garden: chapters 1–3'}).click();
    await page.waitForFunction(()=>audioPreview.voices===2);
    await page.getByLabel('Mute',{exact:true}).check();
    // AudioParam scheduling becomes observable on the next device render quantum.
    await page.waitForFunction(()=>[...EngineAudioDevices.get(audioPreview.device).voices.values()].every(v=>v.gainNode.gain.value===0));
    await page.getByLabel('Mute',{exact:true}).uncheck();
    const peak=await page.evaluate(async()=>{
      const d=EngineAudioDevices.get(audioPreview.device),a=d.context.createAnalyser();a.fftSize=2048;d.output.connect(a);
      await new Promise(r=>setTimeout(r,150));const samples=new Float32Array(2048);a.getFloatTimeDomainData(samples);d.output.disconnect(a);
      return Math.max(...samples.map(Math.abs));
    });assert(peak>0&&peak<1);
    for(let i=0;i<10;i++) {await page.evaluate(i=>Module._audio_music(i%2),i);await page.waitForTimeout(40);}
    await page.waitForTimeout(700);
    report.runtime=await page.evaluate(()=>({...audioPreview,stats:PawsAudioHost.stats}));report.mixPeak=peak;
    assert(report.runtime.peakVoices<=14);assert(report.runtime.peakPcmBytes<=48*1024*1024);
    await page.screenshot({path:path.join(out,'synthetic-backend.png')});
    await context.close();
    const actual=await browser.newContext(),quiet=await actual.newPage();let wavRequests=0;
    quiet.on('request',r=>{if(r.url().endsWith('.wav'))++wavRequests;});quiet.on('pageerror',e=>errors.push(e.message));
    await quiet.goto('http://127.0.0.1:19618/review/index.html');await quiet.waitForFunction(()=>window.audioPreview);
    await quiet.getByRole('button',{name:'Enable sound',exact:true}).click();await quiet.waitForTimeout(800);
    report.actual=await quiet.evaluate(()=>({imported:PawsAudioHost.stats.imported,missing:PawsAudioHost.stats.missing,voices:audioPreview.voices}));
    assert.deepEqual(report.actual,{imported:0,missing:66,voices:0});assert.equal(wavRequests,0);
    await quiet.screenshot({path:path.join(out,'actual-missing.png')});
    report.errors=errors;assert.deepEqual(errors,[]);
    report.wasmSha256=createHash('sha256').update(await fs.readFile(path.join(site,'release1_preview.wasm'))).digest('hex');
    report.checks=['66 catalog slots; 63 serial effect/ambience decodes','all new effect families','stale committed event rejection','four-second water bound and pause cancellation','gesture resume','real backend nonzero mix and mute','rapid music changes','nested URL','actual catalog: no audio requests or voices'];
    await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));
  } catch(e) {await fs.writeFile(path.join(out,'failure.json'),JSON.stringify({error:String(e),errors},null,2));throw e;}
  finally {await browser?.close();await new Promise(r=>server.close(r));}
})().catch(e=>{console.error(e);process.exitCode=1;});
