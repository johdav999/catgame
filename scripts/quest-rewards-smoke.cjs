// Generated production site, real model downloads, trusted puzzle gestures and WebAudio.
const {chromium}=require('playwright'),fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const backend=process.env.REWARD_BACKEND||'webgpu',url=process.env.PAWS_URL||'http://127.0.0.1:8156/';
const out=path.resolve(process.env.EVIDENCE_DIR||`build/evidence/quest-rewards/${backend}`);
const report={backend,checks:[],errors:[],models:[],screenshots:[],quests:[],listening:'Actual WebAudio output measured; no human listening or physical-device acceptance claimed.'};
let browser,page;
(async()=>{
 await fs.mkdir(out,{recursive:true});browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 for(const [quest,recipients,width,height,reduced] of [[0,1,1440,900,false],[8,2,1440,900,false],[9,4,390,844,true]]){
  const context=await browser.newContext({viewport:{width,height}});if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  page=await context.newPage();page.setDefaultTimeout(90000);page.on('pageerror',e=>report.errors.push(e.message));page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
  page.on('response',r=>{if(/\/models\/rewards\/.*\.glb$/.test(r.url()))report.models.push({url:r.url(),status:r.status()});});
  const ready=()=>page.waitForFunction(()=>window.pawsGuiReady&&window.pawsStorageReady&&window.pawsRenderState?.state==='ready');
  const state=()=>page.evaluate(()=>pawsState);
  const snapshot=()=>page.evaluate(()=>{Module._paws_reward_snapshot();return pawsRewards;});
  async function key(name){await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(140);}
  async function points(){return page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});}
  async function click(id){const p=(await points()).objects[id];assert(p,'Clickable '+id);await page.mouse.click(p.x,p.y);await page.waitForTimeout(130);}
  async function drag(id,target){const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to,JSON.stringify({id,target}));const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];const b=/^\d+,\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});await page.mouse.up();await page.waitForTimeout(130);}
  async function shot(name){const file=path.join(out,`quest-${quest}-${name}.png`);await page.screenshot({path:file});report.screenshots.push(file);}
  await page.goto(url);await ready();
  const fixture=await fs.readFile(`build/evidence/quest-rewards/fixtures/quest-${quest}.save`,'utf8');
  await page.evaluate(async text=>{const storage=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await storage.put('current',new TextEncoder().encode(text));}finally{storage.dispose();}},fixture);
  await page.reload();await ready();
  await page.evaluate(()=>{
   const d=EngineAudioDevices.get(pawsAudio.device),decode=d.decode.bind(d),start=d.start.bind(d),event=PawsAudioHost.event.bind(PawsAudioHost),fingerprints=new Map();
   const fingerprint=(rate,channels,pcm)=>[rate,channels,pcm.length,...Array.from({length:32},(_,i)=>pcm[Math.floor((pcm.length-1)*i/31)])].join('|');
   window.rewardEvents=[];window.rewardStarts=[];window.rewardPeaks={};window.rewardDecoded=[];
   PawsAudioHost.event=name=>{rewardEvents.push({name,time:performance.now()});event(name);};
   d.decode=async(file,...args)=>{const r=await decode(file,...args),id=file.name.replace(/^.*[/:]/,'').replace('.wav','');fingerprints.set(fingerprint(r.sampleRate,r.channels,r.samples),id);rewardDecoded.push(id);return r;};
   d.start=(...args)=>{const ok=start(...args),id=fingerprints.get(fingerprint(args[2],args[3],args[4]));if(ok&&(id==='reward_coins_01'||id==='reward_friendship_01')){
    rewardStarts.push({id,time:d.context.currentTime,loop:args[5].loop,generation:pawsAudio.generation});
    const analyser=d.context.createAnalyser();analyser.fftSize=512;d.voices.get(args[0]).gainNode.connect(analyser);const pcm=new Float32Array(512);
    const timer=setInterval(()=>{analyser.getFloatTimeDomainData(pcm);rewardPeaks[id]=Math.max(rewardPeaks[id]||0,...pcm.map(Math.abs));if(!d.voices.has(args[0])){clearInterval(timer);analyser.disconnect();}},10);
   }return ok;};
  });
  await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden);const initial=await state();
  if(reduced){await key('Menu');await key('Settings');if(await page.getByRole('button',{name:'Motion: Full',exact:true}).count())await key('Motion: Full');await key('Back');await key('Continue');}
  await page.waitForFunction(()=>rewardDecoded.includes('reward_coins_01')&&rewardDecoded.includes('reward_friendship_01'));
  // Settings pauses the world device; the existing host requires a fresh
  // trusted input after Continue, before a resumed context can produce audio.
  await page.mouse.click(20,Math.min(height-180,300));
  await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);
  const audioTime=await page.evaluate(()=>EngineAudioDevices.get(pawsAudio.device).context.currentTime);
  await page.waitForFunction(t=>EngineAudioDevices.get(pawsAudio.device).context.currentTime>t+.05,audioTime);
  await page.waitForTimeout(500);
  const delta=[[0,-1],[1,0],[0,1],[-1,0]];
  for(const step of (await state()).solution){
   if((await state()).completed)break;
   const before=await state();
   if(step.verb===1){const o=before.objects.find(o=>o.id===step.object),d=delta[step.direction];await drag(step.object,`${o.x+d[0]},${o.y+d[1]}`);}
   else if(step.verb===2)await click(step.object);
   else if(step.verb===3){await click('source_ball');await page.waitForFunction(()=>pawsState.completed||!pawsBall.rolling);}
   else if(step.verb===4)continue;
   else if(step.verb===5)await drag(step.object,step.target);
   else if(step.verb===7||step.verb===8)await click(step.object);
   else if(step.verb===9)await drag(step.object,step.target);
   else if(step.verb===6)assert((await state()).completed);
   else throw Error('Unhandled solution step '+JSON.stringify(step));
   await page.waitForFunction(revision=>pawsState.revision>revision||pawsState.completed,before.revision,{timeout:5000});
   assert((await state()).revision>before.revision||(await state()).completed,'Gesture did not commit '+JSON.stringify(step));
  }
  await page.waitForFunction(()=>pawsUi.screen===9&&pawsRewards?.phase===1&&pawsRewards.time>.15);
  assert(!await page.getByText('Thank you!',{exact:true}).count(),'Old modal removed');
  let coin=await snapshot();assert.equal(coin.recipients,recipients);assert.equal(coin.tokens.length,recipients);assert(coin.tokens.every(t=>t.name==='Coin reward'&&t.pixelX>0&&t.pixelX<width&&t.pixelY>0&&t.pixelY<height));await shot('coin');
  if(quest===0){await page.evaluate(()=>Module._paws_suspend(1));const held=await snapshot();await page.waitForTimeout(500);assert.equal((await snapshot()).time,held.time);await page.evaluate(()=>Module._paws_suspend(0));await page.mouse.click(20,300);await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);report.checks.push('Host suspension holds the timeline; trusted input resumes sound');}
  const before=(await snapshot()).tokens.map(t=>t.y);await page.waitForTimeout(200);const after=await snapshot();if(after.phase===1)for(let n=0;n<recipients;n++)assert(reduced?Math.abs(after.tokens[n].y-before[n])<.0001:after.tokens[n].y>before[n]);
  await page.waitForFunction(()=>pawsRewards.phase===2&&pawsRewards.time>1.4);const heart=await snapshot();assert.equal(heart.tokens.length,recipients);assert(heart.tokens.every(t=>t.name==='Friendship heart reward'&&t.pixelX>0&&t.pixelX<width&&t.pixelY>0&&t.pixelY<height));await shot('heart');
  await page.waitForFunction(()=>pawsRewards.phase===3);assert.equal((await snapshot()).tokens.length,0);await page.waitForTimeout(150);
  const sound=await page.evaluate(()=>({events:rewardEvents.filter(e=>e.name==='reward_coins'||e.name==='reward_friendship'),starts:rewardStarts,peaks:rewardPeaks,failed:PawsAudioHost.stats.failed,device:EngineAudioDevices.get(pawsAudio.device).snapshot()}));
  report.currentSound=sound;
  assert.deepEqual(sound.events.map(e=>e.name),['reward_coins','reward_friendship']);assert.equal(sound.starts.filter(s=>s.id==='reward_coins_01').length,1);assert.equal(sound.starts.filter(s=>s.id==='reward_friendship_01').length,1);assert(sound.starts.every(s=>!s.loop));assert(sound.peaks.reward_coins_01>0&&sound.peaks.reward_friendship_01>0,'Both generated effects reach real WebAudio output');assert.deepEqual(sound.failed,[]);
  assert.equal((await state()).coins,initial.coins+20);await shot('finished');await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);await page.waitForFunction(()=>pawsState.save===2);
  await page.reload();await ready();assert.notEqual((await snapshot()).phase,1);assert.notEqual((await snapshot()).phase,2);assert.equal((await state()).coins,initial.coins+20);
  report.quests.push({quest,recipients,reduced,coin,heart,sound,build:await page.evaluate(()=>pawsBuild),render:await page.evaluate(()=>pawsRenderState)});console.log('Passed reward quest '+quest);await context.close();
 }
 report.build=report.quests[0].build;const wasm=await fs.readFile('build/web-local-release/site/paws.wasm');assert.equal(report.build.wasmSha256,crypto.createHash('sha256').update(wasm).digest('hex'));assert(report.models.length>=6&&report.models.every(m=>m.status===200));assert.deepEqual(report.errors,[]);report.passed=true;
})().catch(async e=>{report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,rewards:window.pawsRewards,ending:window.pawsEnding,audio:window.pawsAudio,host:window.PawsAudioHost?.stats,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}).finally(async()=>{await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));await browser?.close();console.log(JSON.stringify({passed:report.passed,checks:report.checks,quests:report.quests.length,failure:report.failure},null,2));});
