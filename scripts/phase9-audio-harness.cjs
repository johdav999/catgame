// Test-only non-silent signals served by Playwright routes. Never staged as assets.
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict'),{createHash}=require('node:crypto');
let catalog,tones=new Map(),events=new Set(),maxVoices=0,maxPcm=0,maxDecode=0;
function tone(seconds,frequency){
    const rate=48000,n=Math.floor(seconds*rate),b=Buffer.alloc(44+n*2);
    b.write('RIFF');b.writeUInt32LE(b.length-8,4);b.write('WAVEfmt ',8);b.writeUInt32LE(16,16);b.writeUInt16LE(1,20);b.writeUInt16LE(1,22);b.writeUInt32LE(rate,24);b.writeUInt32LE(rate*2,28);b.writeUInt16LE(2,32);b.writeUInt16LE(16,34);b.write('data',36);b.writeUInt32LE(n*2,40);
    for(let i=0;i<n;i++)b.writeInt16LE(Math.round(Math.sin(i*2*Math.PI*frequency/rate)*2000),44+i*2);
    return b;
}
exports.attach=async context=>{
    catalog=JSON.parse(await fs.readFile('assets/runtime/audio/catalog.json','utf8'));
    for(const [i,c]of catalog.clips.entries()){
        const duration=c.loop?1:.3,bytes=tone(duration,220+i*20);tones.set(c.file,bytes);
        Object.assign(c,{accepted:true,sha256:createHash('sha256').update(bytes).digest('hex'),source:{rightsRecord:'AUTOMATION FIXTURE ONLY'},review:{sourceAuditioned:true},pcm:{duration,channels:1}});
    }
    await context.route('**/assets/audio/catalog.json',r=>r.fulfill({json:catalog}));
    await context.route('**/assets/audio/*.wav',r=>r.fulfill({contentType:'audio/wav',body:tones.get(new URL(r.request().url()).pathname.split('/').pop())}));
};
exports.observe=async page=>{
    const s=await page.evaluate(()=>({audio:window.pawsAudio,stats:window.PawsAudioHost?.stats}));
    if(!s.audio)return;
    s.stats.events.forEach(e=>events.add(e));maxVoices=Math.max(maxVoices,s.audio.peakVoices);maxPcm=Math.max(maxPcm,s.audio.peakPcmBytes);maxDecode=Math.max(maxDecode,s.stats.peakDecodePcmBytes);
    assert(s.audio.voices<=14);assert(s.audio.pcmBytes<=48*1024*1024);
};
exports.finish=async(page,report,action,out)=>{
    await page.setViewportSize({width:1440,height:900});await page.waitForTimeout(250);
    await action('Menu');await action('Settings');
    // Same retained controls; preference mutation is persisted by normal UI.
    await action('Sound: On');await page.waitForTimeout(150);
    assert(await page.evaluate(()=>[...EngineAudioDevices.get(pawsAudio.device).voices.values()].every(v=>v.gainNode.gain.value===0)),'Master mute reaches every real backend voice');
    await action('Sound: Muted');
    for(const name of ['Music: 70%','Ambience: 60%'])await action(name);
    await page.screenshot({path:path.join(out,'audio-settings.png')});
    await page.waitForTimeout(600);
    const peak=await page.evaluate(async()=>{
        const d=EngineAudioDevices.get(pawsAudio.device),analyser=d.context.createAnalyser();analyser.fftSize=2048;d.output.connect(analyser);
        await new Promise(r=>setTimeout(r,150));const samples=new Float32Array(2048);analyser.getFloatTimeDomainData(samples);d.output.disconnect(analyser);
        return Math.max(...samples.map(Math.abs));
    });assert(peak>0&&peak<1,'Nonzero bounded test signal reaches mix');
    await page.evaluate(()=>Module._paws_suspend(1));await page.waitForTimeout(100);
    assert(await page.evaluate(()=>!EngineAudioDevices.get(pawsAudio.device).active));
    await page.evaluate(()=>Module._paws_suspend(0));await page.waitForTimeout(100);
    assert(await page.evaluate(()=>!EngineAudioDevices.get(pawsAudio.device).active),'Visibility return requires gesture');
    await action('Enable sound');await page.waitForFunction(()=>EngineAudioDevices.get(pawsAudio.device).active);
    await exports.observe(page);
    for(const name of ['pot_slide','channel_rotate','toy_pickup','basket_place','cushion_move','switch_down','switch_up','gate_open','gate_close','ui_blocked','ui_undo','ui_hint','reward_coins','shop_purchase','furnishing_place','bowl_place','pip_settle','pip_purr','pip_drink','music_pip_at_home'])assert(events.has(name),'Missing journey cue '+name);
    report.audio={fixture:'synthetic non-silent automation signals; no production audio, audition or physical output claim',events:[...events].sort(),maxVoices,maxPcmBytes:maxPcm,maxDecodePcmBytes:maxDecode,mixPeak:peak,generated:0,imported:0,audiblyVerified:0};
    report.checks.push('Audio: full three-quest/shop cue routing, real backend decode/play, mute and volume buses, visibility suspension and gesture resume, bounded voices/PCM with test-only signals');
    // A second context uses the actual missing-file catalog, with no fixtures.
    const actual=await page.context().browser().newContext({viewport:{width:390,height:844}}),quiet=await actual.newPage();
    let audioRequests=0;quiet.on('request',r=>{if(r.url().endsWith('.wav'))++audioRequests;});
    await quiet.goto(page.url());await quiet.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready');
    async function quietAction(name){
        for(let i=0;i<120;i++){
            if(await quiet.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await quiet.keyboard.press('Enter');await quiet.waitForTimeout(160);return;}
            await quiet.keyboard.press('Tab');await quiet.waitForTimeout(8);
        }throw Error('Quiet control unreachable: '+name);
    }
    await quietAction('Play');await quiet.waitForFunction(()=>pawsState.scene==="Pip's courtyard");
    await quietAction('Menu');await quietAction('Settings');
    assert.equal(await quiet.evaluate(()=>PawsAudioHost.stats.imported),0);
    assert.equal(await quiet.evaluate(()=>pawsAudio.voices),0);assert.equal(audioRequests,0,'Absent final files are never fetched or replaced');
    await quiet.screenshot({path:path.join(out,'actual-missing-audio-390x844.png')});
    report.checks.push('Unmodified production catalog: 0 imported clips, readable missing-audio settings, no WAV requests, no placeholder voices');
    await actual.close();
};
