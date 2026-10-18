// Actual generated game, original runtime MP3 bytes and real Web Audio completion.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const site=path.resolve(process.env.SITE_DIR||'build/web-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/garden-music');
const port=Number(process.env.PAWS_TEST_PORT||19643),base=`http://127.0.0.1:${port}`;
const tracks=['music_garden_afternoon_01','music_garden_puzzles_01'];
const report={site,checks:[],errors:[],musicRequests:[],listening:'Real backend/mix validation; physical speaker output and human audition unverified'};
let server,browser,page;
async function click(name){
  const bounds=await page.evaluate(name=>{
    const button=[...document.querySelectorAll('button[data-widget-id]')].find(b=>b.getAttribute('aria-label')===name);
    return button&&EngineGui.snapshot().widgets.find(w=>w.id===Number(button.dataset.widgetId))?.bounds;
  },name);
  assert(bounds,`Missing action ${name}`);
  await page.mouse.click(bounds.x+bounds.width/2,bounds.y+bounds.height/2);
  await page.waitForTimeout(200);
}
async function resume(){await page.mouse.click(700,450);await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);}
async function musicVoice(){return page.evaluate(()=>{
  const device=EngineAudioDevices.get(pawsAudio.device);
  const entry=[...device.voices].find(([,v])=>v.source.buffer.duration>100);
  return entry?{id:entry[0],duration:entry[1].source.buffer.duration,loop:entry[1].source.loop,gain:entry[1].gainNode.gain.value}:null;
});}
async function accelerateEnd(){
  await page.evaluate(()=>{
    const device=EngineAudioDevices.get(pawsAudio.device);
    const voice=[...device.voices.values()].find(v=>v.source.buffer.duration>100);
    if(!voice)throw Error('No music voice');
    voice.source.playbackRate.value=64;
  });
}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  const catalog=JSON.parse(await fs.readFile(path.join(site,'assets/audio/catalog.json'),'utf8'));
  assert.deepEqual(catalog.playlists[0].tracks,tracks);
  for(const id of tracks){
    const clip=catalog.clips.find(c=>c.id===id),bytes=await fs.readFile(path.join(site,'assets/audio',clip.file));
    assert.equal(createHash('sha256').update(bytes).digest('hex'),clip.sha256);
  }
  report.wasmSha256=createHash('sha256').update(await fs.readFile(path.join(site,'paws.wasm'))).digest('hex');
  server=http.createServer(async(req,res)=>{
    try{
      const pathname=decodeURIComponent(new URL(req.url,base).pathname);
      const file=path.resolve(site,'.'+pathname+(pathname.endsWith('/')?'index.html':''));
      if(!file.startsWith(site+path.sep))throw Error('Outside site');
      const bytes=await fs.readFile(file);
      res.setHeader('Cache-Control','no-store');
      res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.wav':'audio/wav','.mp3':'audio/mpeg','.json':'application/json'})[path.extname(file)]||'application/octet-stream');
      res.end(bytes);
    }catch{res.statusCode=404;res.end();}
  });
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  page=await browser.newPage({viewport:{width:1440,height:900}});
  page.on('pageerror',e=>report.errors.push(e.message));
  page.on('request',r=>{if(r.url().endsWith('.mp3'))report.musicRequests.push(r.url());});
  await page.goto(base+'/');
  await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready'&&window.pawsAudio);
  assert.equal((await page.evaluate(()=>pawsAudio.voices)),0);assert.equal(report.musicRequests.length,0);
  await click('Play');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await resume();
  await page.waitForFunction(id=>PawsAudioHost.stats.events.includes(id),tracks[0],{timeout:60000});
  const first=await musicVoice();assert(first&&!first.loop&&Math.abs(first.duration-178.4)<.1);
  // Isolate music from ambience at the analyser without changing content or playback.
  report.musicMixPeak=await page.evaluate(async()=>{
    const device=EngineAudioDevices.get(pawsAudio.device);
    const voice=[...device.voices.values()].find(v=>v.source.buffer.duration>100);
    const analyser=device.context.createAnalyser();analyser.fftSize=2048;voice.gainNode.connect(analyser);
    await new Promise(r=>setTimeout(r,1000));const samples=new Float32Array(2048);analyser.getFloatTimeDomainData(samples);
    voice.gainNode.disconnect(analyser);return Math.max(...samples.map(Math.abs));
  });assert(report.musicMixPeak>0&&report.musicMixPeak<1,'Actual music reaches mix');
  report.checks.push('No playback/fetch before gesture; full stereo track one decoded with hash checks; nonzero music mix');
  await click('Requests & friends');await click("Pip's missing ball · Available");
  await page.waitForFunction(()=>!pawsState.garden&&pawsState.production);await click('Help Pip');await resume();
  assert.equal((await musicVoice()).id,first.id,'Garden-to-puzzle keeps the same music source');
  await click('Menu');await click('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await resume();
  assert.equal((await musicVoice()).id,first.id,'Puzzle-to-garden keeps music');
  report.checks.push('Both scene transitions preserve the same live music voice');
  await accelerateEnd();
  await page.waitForFunction(id=>PawsAudioHost.stats.events.includes(id),tracks[1],{timeout:60000});
  assert(Math.abs((await musicVoice()).duration-182.36)<.1);
  assert.equal((await musicVoice()).loop,false);
  await accelerateEnd();
  await page.waitForFunction(id=>PawsAudioHost.stats.events.filter(e=>e===id).length===2,tracks[0],{timeout:60000});
  const repeated=await musicVoice();assert.notEqual(repeated.id,first.id);
  report.checks.push('Real AudioBufferSource completion advances 1 → 2 → 1 (64x playback rate for bounded automation)');
  await click('Menu');await page.waitForFunction(()=>!EngineAudioDevices.get(pawsAudio.device).active);
  const frozen=await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).context.currentTime);
  await page.waitForTimeout(400);
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).context.currentTime),frozen);
  await click('Settings');await click('Sound: On');
  await click('Back');await click('Continue');await resume();await page.waitForTimeout(300);
  // Web Audio applies gain automation on its device clock, which was frozen in Settings.
  assert.equal((await musicVoice()).gain,0);
  await click('Menu');await click('Settings');await click('Sound: Muted');await click('Music: 70%');
  await page.screenshot({path:path.join(out,'music-settings.png')});
  await click('Back');await click('Continue');await resume();
  await page.waitForTimeout(300);
  assert(Math.abs((await musicVoice()).gain-.18*.8)<.0001,'Saved Music volume controls existing source');
  assert.equal((await musicVoice()).id,repeated.id,'Pause/resume keeps current source');
  await page.evaluate(()=>document.dispatchEvent(new Event('freeze')));
  await page.waitForFunction(()=>pawsLifecycle.suspended&&!EngineAudioDevices.get(pawsAudio.device).active);
  await page.evaluate(()=>document.dispatchEvent(new Event('resume')));
  await page.waitForFunction(()=>!pawsLifecycle.suspended);
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).active),false);
  await resume();assert.equal((await musicVoice()).id,repeated.id);
  report.checks.push('Pause freezes device clock, mute silences music, volume changes live source, host resume needs gesture');
  report.audio=await page.evaluate(()=>({audio:pawsAudio,host:PawsAudioHost.stats}));
  assert.equal(report.audio.host.failed.length,0);assert(report.audio.audio.peakPcmBytes<=128*1024*1024);
  assert.equal(await page.evaluate(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].filter(v=>v.source.buffer.duration>100).length),1);
  await page.screenshot({path:path.join(out,'garden-playing.png')});
  assert.equal(report.errors.length,0,report.errors.join('\n'));report.success=true;
})().catch(error=>{report.failure=error.stack;console.error(error);process.exitCode=1;}).finally(async()=>{
  if(page)try{report.finalState=await page.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,audio:window.pawsAudio,host:window.PawsAudioHost?.stats}));}catch{}
  await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));
  await browser?.close();await new Promise(resolve=>server?server.close(resolve):resolve());
});
