// Real-input acceptance for the eight-picture shell-song upgrade, in an isolated profile.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=`build/evidence/encore-eight/${backend}`;
const report={backend,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
 const context=await browser.newContext({viewport:{width:1705,height:964},hasTouch:true});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const settle=()=>page.waitForTimeout(220),state=()=>page.evaluate(()=>pawsState);
 const saved=()=>page.waitForFunction(()=>pawsState.save===2);
 const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();};
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const tap=async(id,touch=false)=>{const p=(await points()).objects[id];assert(p,id);if(touch)await page.touchscreen.tap(p.x,p.y);else await page.mouse.click(p.x,p.y);await settle();};
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8080/?devBeach=1');await ready();await key('Play');await saved();
 const fixture=await fs.readFile('build/evidence/beach-outings-start.save.encore-v1','utf8');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});await s.put('current',new TextEncoder().encode(text));s.dispose();},fixture);
 await page.reload();await ready();report.build=await page.evaluate(()=>pawsBuild);assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 await key('Adventures');await key('Start / Continue beach');await key('Help Pepper');await page.waitForFunction(()=>pawsState.quest==='beach_shell_encore'&&pawsUi.screen===2);

 await page.waitForFunction(()=>pawsSequence?.count===12);
 await key('Menu');await key('Restart puzzle');await key('Restart puzzle');
 await page.waitForFunction(()=>pawsSequence?.count===8&&pawsUi.screen===2);
 assert.equal((await state()).solution.length,8);assert.equal((await state()).beachProgress,511);
 await page.screenshot({path:out+'/watch-eight.png'});
 await page.waitForFunction(()=>!pawsSequence.watching,null,{timeout:60000});
 await page.waitForFunction(()=>document.body.innerText.includes('0 / 8 correct'));
 const solution=(await state()).solution;
 for(const step of solution.slice(0,4)){
  await tap(step.object);
  if(step.object==='paw'){
   // Verify a real shell tap starts the delivered paw recording, not only a cue event.
   const sound=await page.evaluate(async()=>{
    const device=EngineAudioDevices.get(pawsAudio.device);
    const voice=[...device.voices.values()].find(v=>Math.abs(v.source.buffer.duration-.881927)<.002);
    if(!voice)return {events:PawsAudioHost.stats.events.slice(-8),failed:PawsAudioHost.stats.failed};
    const analyser=device.context.createAnalyser();analyser.fftSize=2048;voice.gainNode.connect(analyser);
    await new Promise(r=>setTimeout(r,80));const signal=new Float32Array(2048);analyser.getFloatTimeDomainData(signal);voice.gainNode.disconnect(analyser);
    return {duration:voice.source.buffer.duration,started:voice.started,gain:voice.gainNode.gain.value,
      outputRms:Math.sqrt(signal.reduce((sum,x)=>sum+x*x,0)/signal.length)};
   });
   report.pawAudio=sound;assert(sound.started&&sound.gain>0&&sound.outputRms>.003,JSON.stringify(sound));
   report.checks.push('Real paw-shell input starts the repaired 0.88-second recording; its sustained output is non-silent after Effects gain.');
  }
 }
 await page.waitForFunction(()=>document.body.innerText.includes('4 / 8 correct'));await saved();
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);
 assert.equal(await page.evaluate(()=>pawsSequence.count),8);
 await page.waitForFunction(()=>!pawsSequence.watching,null,{timeout:60000});
 await page.waitForFunction(()=>document.body.innerText.includes('4 / 8 correct'));
 for(const step of solution.slice(4))await tap(step.object);
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:30000});await saved();
 assert.equal((await state()).beachProgress,1023);assert.equal((await state()).coins,0);assert.deepEqual(report.errors,[]);
 await page.screenshot({path:out+'/completed-eight.png'});
 report.checks.push('Legacy twelve-picture save upgrades on Restart; exactly eight pictures and responses complete the quest.','Four-answer prefix survives reload; previous milestones and rewards are preserved.');report.ok=true;
}catch(error){report.ok=false;report.failure=error.stack;report.state=await page?.evaluate(()=>({state:window.pawsState,sequence:window.pawsSequence,loadError:window.pawsLoadError,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:out+'/failure.png'}).catch(()=>{});process.exitCode=1;}
finally{await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify({ok:report.ok,checks:report.checks,failure:report.failure},null,2));await browser?.close();}})();
