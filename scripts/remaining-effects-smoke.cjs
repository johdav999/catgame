// Real provider WAVs, generated production site, trusted retained-control input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),http=require('node:http'),assert=require('node:assert/strict');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p09-pinned-release/site');
const out=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/remaining-effects');
const port=+(process.env.PAWS_TEST_PORT||19644),base=`http://127.0.0.1:${port}`;
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
async function buyPlace(item,spot){await details(item);await key('Buy');await key('Choose a spot');await key(spot+' · Fits');await key('Place');if(await screen()===13){await key('Continue in garden');await home();await details(item,'Use furnishing');}await home();}
async function count(id){return page.evaluate(id=>effectStarts.filter(s=>s.clip===id).length,id);}
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
  page.on('pageerror',e=>report.errors.push(e.message));page.on('request',r=>{if(r.url().endsWith('.wav'))report.wavRequests.push(r.url());});
  await page.goto(base+'/');await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready'&&window.pawsAudio);
  assert.equal(report.wavRequests.length,0,'No effects download before consent');
  const jobs=JSON.parse(await fs.readFile('artifacts/audio-production/remaining-effects-jobs.json','utf8')).jobs;
  await page.evaluate(ids=>{
    const wanted=new Set(ids),device=EngineAudioDevices.get(pawsAudio.device),decode=device.decode.bind(device),fingerprints=new Map();
    const fingerprint=(rate,channels,pcm)=>[rate,channels,pcm.length,...Array.from({length:64},(_,i)=>pcm[Math.floor((pcm.length-1)*i/63)])].join('|');
    window.effectStarts=[];window.effectPeaks={};window.effectDecoded=[];
    device.decode=async(file,...args)=>{const d=await decode(file,...args),clip=file.name.replace(/^.*[/:]/,'').replace('.wav','');fingerprints.set(fingerprint(d.sampleRate,d.channels,d.samples),clip);effectDecoded.push(clip);return d;};
    for(const id of [pawsAudio.device,pawsAudio.uiDevice]){
      const d=EngineAudioDevices.get(id),start=d.start.bind(d);
      d.start=(...args)=>{
        const result=start(...args),clip=fingerprints.get(fingerprint(args[2],args[3],args[4]));
        if(result&&wanted.has(clip)){
          effectStarts.push({clip,device:id,soundId:args[1],duration:args[4].length/args[3]/args[2],loop:args[5].loop,gain:args[6],time:d.context.currentTime,generation:pawsAudio.generation});
          const analyser=d.context.createAnalyser();analyser.fftSize=512;d.voices.get(args[0]).gainNode.connect(analyser);
          const samples=new Float32Array(512),timer=setInterval(()=>{analyser.getFloatTimeDomainData(samples);effectPeaks[clip]=Math.max(effectPeaks[clip]||0,...samples.map(Math.abs));if(!d.voices.has(args[0])){clearInterval(timer);analyser.disconnect();}},10);
        }return result;
      };
    }
  },jobs.map(j=>j.id));
  await key('Play');await home();await page.waitForFunction(()=>PawsAudioHost.stats.decoded>=64);
  assert.equal(await page.evaluate(ids=>ids.filter(id=>effectDecoded.includes(id)).length,jobs.map(j=>j.id)),49,'All 49 provider WAVs decode in the real host');
  const quests=[["Pip's missing ball",'Pip'],['Tidy the toy basket','Pip'],['The stuck little gate','Pip'],['A drink after playing','Mabel'],['A bed for Button','Button'],['The sunny spot','Mabel'],['Mouse under the bench','Button'],['Two kittens, one doorway','Pip and Pepper'],['The garden bells','Pepper'],['The garden picnic','All four cats']];
  async function select(title,speaker,suffix='Available'){
    await key('Requests & friends');for(let opts=await names();opts.some(n=>n.startsWith('Welcome '));opts=await names())await key(opts.find(n=>n.startsWith('Welcome ')));
    await key(title+' · '+suffix);await page.waitForFunction(()=>pawsUi.screen===3);await key('Help '+speaker);await page.waitForFunction(()=>pawsUi.screen===2);
  }
  for(const [title,speaker] of quests){
    await select(title,speaker);
    if(title==="Pip's missing ball"){
      // A real keyboard roll on the invalid initial route; no direct audio calls.
      await page.locator('#viewport').focus();await page.keyboard.press('Space');
      await page.waitForFunction(()=>effectStarts.some(s=>s.clip==='ui_blocked_01'));
      await page.waitForTimeout(350);
      await key('Objects & actions');await key('Hint');await key('Back');await key('Back');
    }
    if(title==='The garden bells'){
      await key('Objects & actions');await key('3. Paw bell');await key('Tap Paw bell');await page.waitForTimeout(800);await key('Undo');
    }
    if(title==='The stuck little gate'){
      // Hold the pressure switch, then undo the roll to exercise both release edges.
      const steps=(await state()).solution;
      for(const step of steps){
        if(step.verb===4)break;
        if(await screen()!==7)await key('Objects & actions');let s=await state();
        if(step.object){const n=s.objects.findIndex(o=>o.id===step.object);await key(`${n+1}. ${s.objects[n].name}`);}
        const a=(await state()).actions.find(a=>a.verb===step.verb&&a.object===step.object&&a.target===step.target&&a.direction===step.direction);assert(a?.enabled);await key(a.label);
      }
      await key('Undo');await key('Back');await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2);
    }
    await solve();
    if(title==='A drink after playing')await page.waitForFunction(()=>effectStarts.some(s=>s.clip==='bowl_fill_01'));
    await page.waitForTimeout(1100);await key('Back to garden');await home();
    report.checks.push('Actual quest effects: '+title);console.log('Verified effects:',title);
  }
  // Three actual arrivals are needed to exercise all ball contact variants.
  for(let n=0;n<2;n++){await select("Pip's missing ball",'Pip','Practice');await solve();await page.waitForTimeout(400);await key('Back to garden');await home();}
  await key('Menu');await key('Settings');
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).active),false);
  assert.equal(await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.uiDevice).active),true);
  assert(await count('ui_panel_01')>0,'Panels play with world paused');
  await key('Sound: On');await page.waitForTimeout(200);
  assert(await page.evaluate(()=>[pawsAudio.device,pawsAudio.uiDevice].every(id=>[...EngineAudioDevices.get(id).voices.values()].every(v=>v.gainNode.gain.value===0)||!EngineAudioDevices.get(id).active)),'Mute reaches active menu and suspended world');
  await key('Sound: Muted');await key('Back');await key('Continue');await home();
  for(const [item,spot,cat,cue] of [['Scratching post','scratch corner',0,'cat_scratch_01'],['Playhouse','playhouse corner',0,'playhouse_use_01']]){
    await buyPlace(item,spot);if(await screen()===13){await key('Continue in garden');await home();await details(item,'Use furnishing');await home();}
    await page.waitForFunction(({cat,cue})=>pawsGarden[cat].phase===4&&effectStarts.some(s=>s.clip===cue),{cat,cue});
    await page.waitForTimeout(1400);await page.screenshot({path:path.join(out,item.replaceAll(' ','-')+'.png')});await resetGarden();
  }
  await page.evaluate(()=>document.dispatchEvent(new Event('freeze')));await page.waitForFunction(()=>pawsLifecycle.suspended);
  assert(await page.evaluate(()=>[pawsAudio.device,pawsAudio.uiDevice].every(id=>!EngineAudioDevices.get(id).active)),'Host suspension stops both devices');
  await page.evaluate(()=>document.dispatchEvent(new Event('resume')));await page.mouse.click(700,450);await home();
  report.playback=await page.evaluate(()=>({starts:effectStarts,peaks:effectPeaks,decoded:effectDecoded,audio:pawsAudio,host:PawsAudioHost.stats}));
  report.missingPlayback=jobs.map(j=>j.id).filter(id=>!report.playback.starts.some(s=>s.clip===id));
  assert.deepEqual(report.missingPlayback,[],'Each of the 49 files must start through a real game/UI event');
  for(const id of jobs.map(j=>j.id))assert(report.playback.peaks[id]>0.000001&&report.playback.peaks[id]<1,'Non-silent unclipped real signal: '+id);
  assert(report.playback.audio.peakVoices<=14);assert.equal(report.playback.host.failed.length,0);assert.equal(report.errors.length,0);
  report.checks.push('All 49 actual effects start with nonzero unclipped signals','Paused UI feedback, mute, resume, both-device host suspension and shared voice budget');report.success=true;
})().catch(e=>{report.failure=e.stack;console.error(e);process.exitCode=1;}).finally(async()=>{
  if(page)report.finalState=await page.evaluate(()=>({state:window.pawsState,garden:window.pawsGarden,audio:window.pawsAudio,host:window.PawsAudioHost?.stats,starts:window.effectStarts,peaks:window.effectPeaks})).catch(()=>null);
  await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();await new Promise(r=>server?server.close(r):r());
});
