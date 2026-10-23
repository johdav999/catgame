// Decode and play the user's real WAVs through the generated game's audio device.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const {createHash}=require('node:crypto');
const base=process.env.PAWS_URL||'http://127.0.0.1:8081/';
const out=process.env.EVIDENCE_DIR||'build/evidence/beach-audio';
const report={errors:[],waveRequests:[],musicRequests:[],checks:[]};
(async()=>{let browser,page;try{
 await fs.mkdir(out,{recursive:true});
 const catalog=JSON.parse(await fs.readFile('assets/runtime/audio/catalog.json','utf8'));
 const clips=catalog.clips.filter(c=>c.family==='beach_wave');assert.equal(clips.length,4);
 const music=catalog.clips.find(c=>c.id==='music_beachside_puzzle_01');assert(music);
 for(const c of [...clips,music]){const bytes=await fs.readFile(path.join('assets/runtime/audio',c.file));assert.equal(createHash('sha256').update(bytes).digest('hex'),c.sha256);}
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 page=await browser.newPage({viewport:{width:1280,height:800}});
 if(process.env.WEBGL==='1')await page.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page.on('pageerror',e=>report.errors.push(e.message));
 page.on('request',r=>{if(/\/beach\/waves\d\.wav/.test(r.url()))report.waveRequests.push(r.url());});
 page.on('request',r=>{if(/\/beach\/.*\.(mp3|wav)/.test(r.url())&&!/waves\d\.wav/.test(r.url()))report.musicRequests.push(r.url());});
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 await page.goto(base+'?devBeach=1');
 await page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready'&&window.pawsAudio,null,{timeout:90000});
 async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(200);}
 async function resume(){await page.mouse.click(950,700);await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);}
 async function waveState(){return page.evaluate(()=>{
  const device=EngineAudioDevices.get(pawsAudio.device),voices=[...device.voices.values()].filter(v=>Math.abs(v.source.buffer.duration-2.55)<.001&&v.source.buffer.numberOfChannels===2);
  return {events:PawsAudioHost.stats.events.filter(e=>/^beach_wave_/.test(e)),starts:window.__waveStarts,voices:voices.map(v=>({gain:v.gainNode.gain.value,loop:v.source.loop})),time:device.context.currentTime};
 });}
 async function musicState(){return page.evaluate(()=>{
  const d=EngineAudioDevices.get(pawsAudio.device),voices=[...d.voices].filter(([,v])=>v.source.buffer.duration>100);
  return voices.map(([id,v])=>({id,duration:v.source.buffer.duration,gain:v.gainNode.gain.value,loop:v.source.loop}));
 });}
 assert.equal(report.waveRequests.length,0);await key('Play');
 await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});await resume();
 await page.waitForFunction(()=>PawsAudioHost.stats.events.includes('music_garden_afternoon_01'),null,{timeout:90000});
 assert.equal(report.waveRequests.length,0,'Garden must not request beach-only WAVs');
 assert.equal(report.musicRequests.length,0,'Garden must not fetch beach music');
 await page.evaluate(()=>{window.__waveStarts=[];window.__surfFrames=[];const event=PawsAudioHost.event;PawsAudioHost.event=name=>{event(name);if(/^beach_wave_/.test(name))__waveStarts.push({name,time:EngineAudioDevices.get(pawsAudio.device).context.currentTime});};
  function sample(){if(window.pawsSurf?.starts&&__surfFrames.length<8000)__surfFrames.push({...pawsSurf,time:EngineAudioDevices.get(pawsAudio.device).context.currentTime});requestAnimationFrame(sample);}requestAnimationFrame(sample);
 });
 await key('Adventures');await key('Start / Continue beach');await page.waitForFunction(()=>pawsUi.screen===3,null,{timeout:90000});await key('Help Pip');await resume();
 await page.waitForFunction(()=>PawsAudioHost.stats.events.includes('music_beachside_puzzle_01'),null,{timeout:90000});
 const firstMusic=await musicState();assert.equal(firstMusic.length,1);assert(Math.abs(firstMusic[0].duration-music.pcm.duration)<.1);assert.equal(firstMusic[0].loop,false);
 assert(report.musicRequests.every(url=>url.endsWith('music_beachside_puzzle_01.mp3')),'Only compressed beach music ships/loads');
 await page.waitForFunction(()=>PawsAudioHost.stats.events.some(e=>/^beach_wave_/.test(e)),null,{timeout:90000});
 report.build=await page.evaluate(()=>pawsBuild);
 // Observe real normal-speed completion and independently timed starts.
 const deadline=Date.now()+65000;let last;
 while(Date.now()<deadline){last=await waveState();assert(last.voices.length<=1,'Waves cannot overlap');assert(last.voices.every(v=>!v.loop));if(last.events.length>=8)break;await page.waitForTimeout(100);}
 assert(last.events.length>=8,'Eight random waves must play');
 assert(last.events.every((e,i)=>/^beach_wave_0[1-4]$/.test(e)&&(!i||e!==last.events[i-1])),'Consecutive variants must differ');
 report.gaps=last.starts.slice(1).map((s,i)=>s.time-last.starts[i].time-2.55);
 assert(report.gaps.every(g=>g>=3.4&&g<5),'Wave gaps must stay near the authored 3.55–4.35 second range');
 assert(Math.max(...report.gaps)-Math.min(...report.gaps)>.03,'Wave gaps must vary');
 report.sequence=last.events;report.checks.push('Real WAVs play randomly with varied pauses, no consecutive repeats, overlap or sample looping');
 report.musicOutputPeak=await page.evaluate(async()=>{
  const d=EngineAudioDevices.get(pawsAudio.device),voice=[...d.voices.values()].find(v=>v.source.buffer.duration>100);
  const a=d.context.createAnalyser();a.fftSize=2048;voice.gainNode.connect(a);
  await new Promise(r=>setTimeout(r,250));const samples=new Float32Array(2048);a.getFloatTimeDomainData(samples);voice.gainNode.disconnect(a);return Math.max(...samples.map(Math.abs));
 });assert(report.musicOutputPeak>0&&report.musicOutputPeak<1,'Actual beach music reaches mixer alongside waves');
 await page.evaluate(()=>{const d=EngineAudioDevices.get(pawsAudio.device);[...d.voices.values()].find(v=>v.source.buffer.duration>100).source.playbackRate.value=64;});
 await page.waitForFunction(()=>PawsAudioHost.stats.events.filter(e=>e==='music_beachside_puzzle_01').length>=2,null,{timeout:15000});
 const repeatedMusic=await musicState();assert.equal(repeatedMusic.length,1);assert.notEqual(repeatedMusic[0].id,firstMusic[0].id);
 report.checks.push('Beachside Puzzle replaces garden music, plays alongside waves and repeats on real source completion (64x completion check)');
 await page.waitForFunction(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].some(v=>Math.abs(v.source.buffer.duration-2.55)<.001));
 const frames=await page.evaluate(()=>__surfFrames);
 report.surfSync=last.starts.slice(1).map((s,i)=>{
  const frame=frames.find(f=>f.starts===i+2);
  assert(frame&&frame.age<.15,'Every actual wave start restarts visible surf within a frame');
  assert(Math.abs(frame.time-s.time)<.15,'Foam and real device start must agree');
  const wash=frames.filter(f=>f.starts===i+2&&f.age<1.6);
  if(wash.length>4)assert(wash.at(-1).travel>wash[0].travel,'Foam travels toward shore');
  return {wave:s.name,delay:frame.time-s.time};
 });
 report.checks.push('Each real wave recording starts the incoming foam wash on the same browser frame');
 await page.waitForFunction(()=>{
  const d=EngineAudioDevices.get(pawsAudio.device),last=__waveStarts.at(-1);
  return last&&d.context.currentTime-last.time<.2&&[...d.voices.values()].some(v=>Math.abs(v.source.buffer.duration-2.55)<.001);
 });
 report.outputPeak=await page.evaluate(async()=>{
  const d=EngineAudioDevices.get(pawsAudio.device),voice=[...d.voices.values()].find(v=>Math.abs(v.source.buffer.duration-2.55)<.001&&v.source.buffer.numberOfChannels===2);
  const a=d.context.createAnalyser();a.fftSize=2048;voice.gainNode.connect(a);
  // The cleaned recording intentionally has no opening impact; sample the surf.
  const decoded=voice.source.buffer.getChannelData(0),rate=voice.source.buffer.sampleRate;
  const assertOpening=decoded.slice(0,Math.floor(.299*rate)).every(v=>v===0);
  if(!assertOpening)throw Error('Wave opening transient remains in decoded browser PCM');
  await new Promise(r=>setTimeout(r,800));const samples=new Float32Array(2048);a.getFloatTimeDomainData(samples);
  if([...d.voices.values()].includes(voice))voice.gainNode.disconnect(a);return Math.max(...samples.map(Math.abs));
 });assert(report.outputPeak>0&&report.outputPeak<1,'Real wave audio reaches mixer without output clipping');
 await key('Menu');await page.waitForFunction(()=>!EngineAudioDevices.get(pawsAudio.device).active);
 const paused=await waveState(),pausedSurf=await page.evaluate(()=>pawsSurf);await page.waitForTimeout(900);const still=await waveState();assert.equal(still.time,paused.time);assert.deepEqual(still.events,paused.events);assert.deepEqual(await page.evaluate(()=>pawsSurf),pausedSurf,'Pause freezes surf too');
 await key('Settings');await key('Sound: On');await key('Back');await key('Continue');await resume();await page.waitForTimeout(500);
 let muted=await waveState();assert(muted.voices.every(v=>v.gain===0));const mutedCount=muted.events.length;await page.waitForTimeout(8000);assert.equal((await waveState()).events.length,mutedCount);
 assert((await musicState()).every(v=>v.gain===0),'Master mute silences beach music');
 await key('Menu');await key('Settings');await key('Sound: Muted');await key('Effects: 80%');await key('Music: 70%');await key('Back');await key('Continue');await resume();
 await page.waitForFunction(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].some(v=>Math.abs(v.source.buffer.duration-2.55)<.001&&Math.abs(v.gainNode.gain.value-.12*.9)<.0001));
 report.checks.push('Pause freezes device clock and wave sequence; mute prevents queued waves; Effects volume controls wave gain');
 await page.waitForTimeout(300);const controlledMusic=await musicState();assert.equal(controlledMusic[0].id,repeatedMusic[0].id);assert(Math.abs(controlledMusic[0].gain-.18*.8)<.0001);
 report.checks.push('Music volume controls the beach track; pause/mute/resume retain its current source');
 await page.waitForFunction(()=>pawsSurf.age>.7&&pawsSurf.age<1.5&&pawsSurf.opacity>.4);
 await page.screenshot({path:path.join(out,'beach-waves-playing.png')});
 await key('Menu');await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});await resume();
 const left=await waveState();assert.equal(left.voices.length,0);await page.waitForTimeout(8000);assert.deepEqual((await waveState()).events,left.events);
 await page.waitForFunction(()=>{const d=EngineAudioDevices.get(pawsAudio.device);return [...d.voices.values()].some(v=>Math.abs(v.source.buffer.duration-178.4)<.1);},null,{timeout:90000});
 const gardenMusic=await musicState();assert.equal(gardenMusic.length,1);assert(Math.abs(gardenMusic[0].duration-178.4)<.1);assert.notEqual(gardenMusic[0].id,repeatedMusic[0].id);
 report.checks.push('Returning to garden stops the current wave and prevents subsequent wave playback');
 report.checks.push('Leaving beach replaces Beachside Puzzle with the garden playlist without music overlap');
 report.audio=await page.evaluate(()=>({state:pawsAudio,host:PawsAudioHost.stats}));assert.deepEqual(report.audio.host.failed,[]);
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;process.exitCode=1;}finally{
 if(page)try{report.finalState=await page.evaluate(()=>({state:window.pawsState,audio:window.pawsAudio,host:window.PawsAudioHost?.stats}));}catch{}
 await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report));
}})();
