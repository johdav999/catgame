// Real provider WAVs, generated production site, trusted retained-control input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p09-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/pip-sounds');
const port=+(process.env.PAWS_TEST_PORT||19643),base=`http://127.0.0.1:${port}`;
const report={site,checks:[],errors:[],wavRequests:[],listening:'Real WebAudio signal and lifecycle checks; no human or physical-device audition claimed'};
let server,browser,page;
const names=()=>page.locator('button[data-widget-id]').evaluateAll(es=>es.map(e=>e.getAttribute('aria-label')));
const screen=()=>page.evaluate(()=>pawsUi.screen);
const state=()=>page.evaluate(()=>pawsState);
async function key(name){
  assert((await names()).includes(name),'Missing control '+name);
  for(let n=0;n<600;n++){
    if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(100);return;}
    await page.keyboard.press('Tab');
  }throw Error('Unreachable control '+name);
}
async function home(){await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await page.mouse.click(700,450);await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);}
async function resetGarden(){await key('Menu');await key('Back to garden');await home();}
async function solve(){
  for(const step of (await state()).solution){
    if(await screen()!==7)await key('Objects & actions');let s=await state();
    if(step.object){const n=s.objects.findIndex(o=>o.id===step.object);assert(n>=0);await key(`${n+1}. ${s.objects[n].name}`);}
    s=await state();const a=s.actions.find(a=>a.verb===step.verb&&a.object===step.object&&a.target===step.target&&a.direction===step.direction&&(step.verb!==5||(a.x===step.x&&a.y===step.y)));
    assert(a?.enabled,JSON.stringify(step));await key(a.label);
  }await page.waitForFunction(()=>pawsUi.screen===9);
}
async function details(item,label){await key('Menu');await key('Shop');const option=(await names()).find(n=>n.startsWith(item+' · '));assert(option);await key(option);if(label)await key(label);}
async function buyPlace(item,spot){await details(item);await key('Buy');await key('Choose a spot');await key(spot+' · Fits');await key('Place');await home();}
async function count(id){return page.evaluate(id=>pipStarts.filter(s=>s.clip===id).length,id);}
(async()=>{
  await fs.mkdir(out,{recursive:true});
  server=http.createServer(async(req,res)=>{try{
    const pathname=decodeURIComponent(new URL(req.url,base).pathname),file=path.resolve(site,'.'+pathname+(pathname.endsWith('/')?'index.html':''));
    if(!file.startsWith(site+path.sep))throw Error('Outside site');const bytes=await fs.readFile(file);
    res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.wav':'audio/wav','.json':'application/json'})[path.extname(file)]||'application/octet-stream');res.end(bytes);
  }catch{res.statusCode=404;res.end();}});
  await new Promise(r=>server.listen(port,'127.0.0.1',r));
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  page=await browser.newPage({viewport:{width:1440,height:900}});page.setDefaultTimeout(60000);
  page.on('pageerror',e=>report.errors.push(e.message));page.on('request',r=>{if(r.url().includes('/pip_')&&r.url().endsWith('.wav'))report.wavRequests.push(r.url());});
  await page.goto(base+'/');await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready'&&window.pawsAudio);
  assert.equal(report.wavRequests.length,0,'No audio downloads before gesture');
  await page.evaluate(()=>{
    const device=EngineAudioDevices.get(pawsAudio.device),decode=device.decode.bind(device),start=device.start.bind(device),fingerprints=new Map();
    const fingerprint=(rate,channels,pcm)=>[rate,channels,pcm.length,...Array.from({length:64},(_,i)=>pcm[Math.floor((pcm.length-1)*i/63)])].join('|');
    window.pipStarts=[];window.pipPeaks={};
    device.decode=async(file,...args)=>{const d=await decode(file,...args);fingerprints.set(fingerprint(d.sampleRate,d.channels,d.samples),file.name.replace('.wav',''));return d;};
    device.start=(...args)=>{
      const result=start(...args),clip=fingerprints.get(fingerprint(args[2],args[3],args[4]));
      if(result&&clip?.startsWith('pip_')){
        pipStarts.push({clip,soundId:args[1],duration:args[4].length/args[3]/args[2],loop:args[5].loop,gain:args[6],time:device.context.currentTime,generation:pawsAudio.generation});
        const analyser=device.context.createAnalyser();analyser.fftSize=512;device.voices.get(args[0]).gainNode.connect(analyser);
        const samples=new Float32Array(512),timer=setInterval(()=>{analyser.getFloatTimeDomainData(samples);pipPeaks[clip]=Math.max(pipPeaks[clip]||0,...samples.map(Math.abs));if(!device.voices.has(args[0])){clearInterval(timer);analyser.disconnect();}},20);
      }return result;
    };
  });
  await key('Play');await home();
  await page.waitForFunction(()=>PawsAudioHost.stats.decoded>=14);
  assert.equal(new Set(report.wavRequests).size,10,'All ten real Pip WAVs downloaded');
  for(const title of ["Pip's missing ball",'Tidy the toy basket','The stuck little gate']){
    await key('Requests & friends');await key(title+' · Available');await page.waitForFunction(()=>pawsUi.screen===3);await key('Help Pip');await page.waitForFunction(()=>pawsUi.screen===2);
    await solve();await key('Back to garden');await home();console.log('Verified Pip request and completion:',title);
  }
  assert(await count('pip_request_01')>=1&&await count('pip_request_02')>=1,'Both request variants actually start');
  assert(await count('pip_happy_01')>=1&&await count('pip_happy_02')>=1,'Both happy variants actually start');
  report.checks.push('Two requesting meows and two happy trills play through three real Pip quests');
  await buyPlace('Garden cushion','bench corner');
  await page.waitForFunction(()=>pipStarts.some(s=>s.clip.startsWith('pip_paws_stone_')));
  await key('Menu');const paused=await page.evaluate(()=>pipStarts.length);await page.waitForTimeout(1200);
  assert.equal(await page.evaluate(()=>pipStarts.length),paused,'Pause has no footsteps or contact catch-up');assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).active),false);
  await key('Continue');await home();
  await page.waitForFunction(()=>pawsGarden[0].phase===4&&pawsGarden[0].item==='garden_cushion');
  await page.waitForFunction(()=>pipStarts.some(s=>s.clip==='pip_purr_01'));
  const settleCount=await count('pip_settle_01');assert.equal(settleCount,1);await page.waitForTimeout(900);assert.equal(await count('pip_settle_01'),settleCount);
  await page.screenshot({path:path.join(out,'pip-resting.png')});
  assert.equal(new Set(await page.evaluate(()=>pipStarts.filter(s=>s.clip.startsWith('pip_paws_stone_')).map(s=>s.clip))).size,3,'All three padded paw variants actually play');
  await page.waitForTimeout(5500);assert(await page.evaluate(()=>![...EngineAudioDevices.get(pawsAudio.device).voices.values()].some(v=>v.options.loop&&Math.abs(v.source.buffer.duration-9.75)<.01)),'Purr ends after bounded rest');
  report.checks.push('Three step variants follow walking; pause freezes sound; cushion rustles once and purr ends with bounded rest');
  await resetGarden();await details('Garden cushion');assert((await names()).includes('Invite Pip to rest'));await key('Invite Pip to rest');await home();
  await page.waitForFunction(()=>pawsGarden[0].item==='garden_cushion'&&pawsGarden[0].phase===2);
  await resetGarden();await buyPlace('Drinking bowl','sunny corner');
  await page.waitForFunction(()=>pawsGarden[0].phase===4&&pawsGarden[0].item==='drinking_bowl');
  await page.waitForFunction(()=>pipStarts.some(s=>s.clip==='pip_drink_01'));const drinks=await count('pip_drink_01');await page.waitForTimeout(900);assert.equal(await count('pip_drink_01'),drinks);
  await page.screenshot({path:path.join(out,'pip-drinking.png')});
  await key('Menu');await key('Settings');await key('Sound: On');
  assert((await names()).includes('Sound: Muted'),'Mute setting committed');
  // A suspended AudioContext does not render newly scheduled AudioParam values.
  // Resume while still muted to verify the actual rendered gains.
  await key('Back');await key('Continue');await home();
  await page.waitForFunction(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].every(v=>v.gainNode.gain.value===0));
  await key('Menu');await key('Settings');await key('Sound: Muted');await key('Back');await key('Continue');await home();
  await resetGarden();await details('Drinking bowl');assert((await names()).includes('Invite Pip to drink'));await key('Invite Pip to drink');await home();
  await page.waitForFunction(()=>pawsGarden[0].item==='drinking_bowl'&&pawsGarden[0].phase===2);
  report.checks.push('Explicit Pip rest/drink controls, one lapping cue at bowl contact, mute and gesture resume');
  // Other cats keep their own voice. Welcome Mabel/Button and use the defaults.
  await resetGarden();await key('Requests & friends');for(let opts=await names();opts.some(n=>n.startsWith('Welcome '));opts=await names())await key(opts.find(n=>n.startsWith('Welcome ')));await key('Back');
  for(const [item,cat] of [['Drinking bowl',1],['Garden cushion',2]]){
    const before=await page.evaluate(()=>pipStarts.filter(s=>['pip_purr_01','pip_settle_01','pip_drink_01'].includes(s.clip)).length);
    await details(item,'Use furnishing');await home();await page.waitForFunction(cat=>pawsGarden[cat].phase===4,cat);
    assert.equal(await page.evaluate(()=>pipStarts.filter(s=>['pip_purr_01','pip_settle_01','pip_drink_01'].includes(s.clip)).length),before,'Other cats do not use Pip voice');await resetGarden();
  }
  report.checks.push('Mabel drinks and Button rests without Pip vocal/contact cues');
  report.playback=await page.evaluate(()=>({starts:pipStarts,peaks:pipPeaks,audio:pawsAudio,host:PawsAudioHost.stats}));
  const catalog=await fs.readFile(path.join(site,'assets/audio/catalog.json'),'utf8').then(JSON.parse),clips=catalog.clips.filter(c=>c.id.startsWith('pip_'));
  report.pcmBytesAt48k=clips.reduce((n,c)=>n+c.pcm.pcmBytesAtDeviceRate,0);assert(report.pcmBytesAt48k<4*1024*1024);assert(report.playback.audio.peakVoices<=14);
  assert.equal(report.playback.host.failed.filter(c=>c.startsWith('pip_')).length,0);
  for(const clip of clips)assert(report.playback.peaks[clip.id]>0.000001&&report.playback.peaks[clip.id]<1,'Real non-silent unclipped signal: '+clip.id);
  assert.equal(report.errors.length,0,report.errors.join('\n'));report.success=true;
})().catch(e=>{report.failure=e.stack;console.error(e);process.exitCode=1;}).finally(async()=>{
  if(page)report.finalState=await page.evaluate(()=>({state:window.pawsState,garden:window.pawsGarden,audio:window.pawsAudio,host:window.PawsAudioHost?.stats})).catch(()=>null);
  await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();await new Promise(r=>server?server.close(r):r());
});
