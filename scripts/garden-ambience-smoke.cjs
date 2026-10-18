// Actual generated Paws site and provider WAVs; no audio routes or synthetic fixtures.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p09-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/garden-ambience');
const port=Number(process.env.PAWS_TEST_PORT||19641),base=`http://127.0.0.1:${port}`;
const report={site,checks:[],errors:[],wavRequests:[],listening:'Automated real-backend signal checks; no human or physical-device audition claimed'};
const lifecycleOnly=process.argv.includes('--lifecycle-only');
report.scope=lifecycleOnly?'Actual-file decoding and pause/mute/gesture resume':'Full loop, variant and lifecycle regression';
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
(async()=>{
  await fs.mkdir(out,{recursive:true});
  server=http.createServer(async(req,res)=>{
    try{
      const pathname=decodeURIComponent(new URL(req.url,base).pathname);
      const file=path.resolve(site,'.'+pathname+(pathname.endsWith('/')?'index.html':''));
      if(!file.startsWith(site+path.sep))throw Error('Outside site');
      const bytes=await fs.readFile(file);
      res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.wav':'audio/wav','.json':'application/json'})[path.extname(file)]||'application/octet-stream');
      res.end(bytes);
    }catch{res.statusCode=404;res.end();}
  });
  await new Promise(resolve=>server.listen(port,'127.0.0.1',resolve));
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  page=await browser.newPage({viewport:{width:1440,height:900}});
  page.on('pageerror',e=>report.errors.push(e.message));
  page.on('request',r=>{if(r.url().endsWith('.wav'))report.wavRequests.push(r.url());});
  await page.goto(base+'/');
  await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready'&&window.pawsAudio);
  assert.equal(await page.evaluate(()=>pawsAudio.voices),0);
  assert.equal(report.wavRequests.length,0);
  await page.evaluate(()=>{
    const device=EngineAudioDevices.get(pawsAudio.device),original=device.start.bind(device);
    window.ambienceStarts=[];
    device.start=(...args)=>{
      const result=original(...args);
      if(result)ambienceStarts.push({id:args[0],soundId:args[1],duration:args[4].length/args[3]/args[2],loop:args[5].loop,time:device.context.currentTime});
      return result;
    };
  });
  await click('Play');
  await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
  // Asset preparation can suspend the device; exercise the game's real gesture resume.
  await page.mouse.click(700,450);
  await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1&&PawsAudioHost.stats.decoded>=4&&pawsAudio.voices>0);
  assert(await page.evaluate(()=>PawsAudioHost.stats.playable>=4));
  assert.equal(new Set(report.wavRequests.filter(url=>url.includes('/amb_garden_'))).size,4);
  await page.evaluate(()=>{
    const device=EngineAudioDevices.get(pawsAudio.device);
    window.ambienceAnalyser=device.context.createAnalyser();ambienceAnalyser.fftSize=2048;
    const air=[...device.voices.values()].find(v=>v.options.loop&&v.source.buffer.duration>20&&v.source.buffer.duration<30);
    if(!air)throw Error('Garden breeze voice missing');
    air.gainNode.connect(ambienceAnalyser);
    window.ambienceStartTime=device.context.currentTime;
  });
  if(!lifecycleOnly){
  let peak=0;
  // Five full 24.75-second loops at the real AudioContext clock, plus natural bird intervals.
  for(let second=0;second<130;second++){
    await page.waitForTimeout(1000);
    peak=Math.max(peak,await page.evaluate(()=>{
      const samples=new Float32Array(ambienceAnalyser.fftSize);ambienceAnalyser.getFloatTimeDomainData(samples);
      return Math.max(...samples.map(Math.abs));
    }));
    if(second%25===24)console.log('Observed real garden playback:',second+1,'seconds');
  }
  const playback=await page.evaluate(()=>({starts:ambienceStarts,elapsed:EngineAudioDevices.get(pawsAudio.device).context.currentTime-ambienceStartTime,audio:pawsAudio,host:PawsAudioHost.stats}));
  assert(peak>0.00001&&peak<1,'Real mix is non-silent with headroom');
  const air=playback.starts.find(s=>s.loop&&s.duration>20&&s.duration<30);assert(air&&playback.elapsed>=air.duration*5);
  assert.equal(new Set(playback.starts.filter(s=>!s.loop&&Math.abs(s.duration-2.48)<.01).map(s=>s.soundId)).size,3,'All three real bird variants play naturally');
  assert.equal(playback.starts.filter(s=>s.loop&&s.duration>20&&s.duration<30).length,1,'Breeze retains one continuous voice');
  assert.equal(playback.host.failed.filter(id=>id.startsWith('amb_garden_')).length,0,'Ambience has no decode failures');
  // Other authored music may have a different budget; measure this task's four buffers.
  const catalogue=await page.evaluate(()=>fetch('assets/audio/catalog.json').then(r=>r.json()));
  report.ambiencePcmBytes=catalogue.clips.filter(c=>c.id.startsWith('amb_garden_')).reduce((sum,c)=>sum+Math.ceil(c.pcm.duration*44100)*c.pcm.channels*4,0);
  assert(report.ambiencePcmBytes<12*1024*1024&&playback.audio.peakVoices<=14);
  report.playback=playback;report.mixPeak=peak;
  report.checks.push('Four actual WAV requests and successful hash/decode/publication', 'Five continuous real-backend breeze loops', 'Three naturally scheduled bird variants', 'Nonzero mix, headroom and bounded memory/voices');
  }else report.checks.push('Four actual provider WAVs decode and publish in the generated game');
  await click('Menu');
  await page.waitForFunction(()=>pawsLifecycle.userPaused);
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).active),false);
  const before=await page.evaluate(()=>ambienceStarts.length);
  await page.waitForTimeout(1200);assert.equal(await page.evaluate(()=>ambienceStarts.length),before);
  await click('Settings');await click('Sound: On');
  assert(await page.evaluate(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].every(v=>v.gainNode.gain.value===0)));
  await page.screenshot({path:path.join(out,'ambience-settings.png')});
  await click('Sound: Muted');await click('Back');await click('Continue');
  await page.waitForFunction(()=>!pawsLifecycle.userPaused&&pawsUi.screen===1&&pawsRenderState.state==='ready');
  await page.mouse.click(700,450);
  await page.waitForFunction(()=>!pawsLifecycle.userPaused&&EngineAudioDevices.get(pawsAudio.device).active);
  await page.evaluate(()=>document.dispatchEvent(new Event('freeze')));
  await page.waitForFunction(()=>pawsLifecycle.suspended);
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).active),false);
  await page.evaluate(()=>document.dispatchEvent(new Event('resume')));
  await page.waitForFunction(()=>!pawsLifecycle.suspended);
  await page.mouse.click(700,450);
  await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);
  report.checks.push('Pause freezes birds and device; mute silences live voices; gesture resumes after host suspension');
  await page.screenshot({path:path.join(out,'garden-playing.png')});
  assert.equal(report.errors.length,0,report.errors.join('\n'));
  report.success=true;
})().catch(error=>{report.failure=error.stack;console.error(error);process.exitCode=1;}).finally(async()=>{
  if(page){try{report.finalState=await page.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,audio:window.pawsAudio,host:window.PawsAudioHost?.stats,lifecycle:window.pawsLifecycle,starts:window.ambienceStarts,device:window.pawsAudio?{active:EngineAudioDevices.get(pawsAudio.device).active,suspended:EngineAudioDevices.get(pawsAudio.device).suspended,state:EngineAudioDevices.get(pawsAudio.device).context?.state,error:EngineAudioDevices.get(pawsAudio.device).error}:null}));await page.screenshot({path:path.join(out,'final-state.png')});}catch{}}
  await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));
  await browser?.close();await new Promise(resolve=>server?server.close(resolve):resolve());
});
