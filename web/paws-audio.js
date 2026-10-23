/* Paws loading policy only. Device, decode and playback belong to WebEngine. */
'use strict';
window.PawsAudioHost = (() => {
  let deviceId=0, catalog=null, catalogPromise=null, chain=Promise.resolve();
  let epoch=0, controller=new AbortController();
  const playable=clip=>clip?.accepted===true||
    (clip?.playable===true&&clip.review?.technicalVerified===true);
  const stats={imported:0,playable:0,missing:72,failed:[],decoded:0,peakDecodePcmBytes:0,events:[]};
  const manifest=()=>catalogPromise||(catalogPromise=fetch('assets/audio/catalog.json').then(r=>{
    if(!r.ok)throw Error('Audio catalog unavailable');return r.json();
  }).then(m=>{
    if(m.schema!==1||m.clips.length!==72)throw Error('Audio catalog version mismatch');
    catalog=m;stats.imported=m.clips.filter(c=>c.accepted).length;stats.playable=m.clips.filter(playable).length;stats.missing=m.clips.length-stats.playable;return m;
  }).catch(e=>{catalogPromise=null;throw e;}));
  const failed=(index,generation,error)=>{
    if(error?.name==='AbortError'||generation!==epoch)return;
    const name=catalog?.clips[index]?.id||String(index);
    if(!stats.failed.includes(name))stats.failed.push(name);
    Module._paws_audio_failed(index,generation);
  };
  return {
    stats,
    init(id) {deviceId=id;void manifest().catch(()=>{});},
    epoch(value) {epoch=value;controller.abort();controller=new AbortController();},
    retry() {catalogPromise=null;stats.failed=[];void manifest().catch(()=>{});},
    event(name) {stats.events.push(name);if(stats.events.length>512)stats.events.shift();},
    request(index,generation) {
      const signal=controller.signal;
      // Exactly one fetch/decode/PCM transfer at a time, including cancelled work.
      chain=chain.then(async()=>{
        if(signal.aborted||generation!==epoch)return;
        const m=await manifest(),clip=m.clips[index];
        if(!playable(clip)){Module._paws_audio_failed(index,generation);return;}
        if(!clip.source?.rightsRecord||
          (clip.accepted===true&&!clip.review?.sourceAuditioned))
          throw Error('Final audio missing');
        const music=clip.id.startsWith('music_'),pcmBudget=(music?96:24)*1024*1024;
        if(!/^(?:(?:release1|beach)\/)?[a-z0-9_]+\.(?:wav|mp3)$/.test(clip.file)||!clip.pcm||clip.pcm.duration>(music?300:90)||clip.pcm.channels>2)
          throw Error('Invalid audio delivery');
        const response=await fetch('assets/audio/'+clip.file,{signal});
        if(!response.ok)throw Error('Audio file unavailable');
        const bytes=await response.arrayBuffer();
        if(bytes.byteLength>24*1024*1024)throw Error('Encoded audio budget');
        const hash=[...new Uint8Array(await crypto.subtle.digest('SHA-256',bytes))].map(x=>x.toString(16).padStart(2,'0')).join('');
        if(hash!==clip.sha256)throw Error('Audio content mismatch');
        const device=EngineAudioDevices.get(deviceId);
        // decodeAudioData resamples to device rate; budget that size, not the WAV rate.
        if(clip.pcm.duration*clip.pcm.channels*(device.context?.sampleRate||48000)*4>pcmBudget)
          throw Error('Decoded audio budget');
        const decoded=await device.decode(new File([bytes],clip.file),{signal});
        if(signal.aborted||generation!==epoch)return;
        const size=decoded.samples.byteLength;
        stats.peakDecodePcmBytes=Math.max(stats.peakDecodePcmBytes,size);
        if(size>pcmBudget)throw Error('Decoded audio budget');
        const pointer=Module._malloc(size);if(!pointer)throw Error('Audio allocation failed');
        try {
          Module.HEAPU8.set(new Uint8Array(decoded.samples.buffer),pointer);
          if(!Module._paws_audio_publish(index,generation,decoded.sampleRate,decoded.channels,pointer,decoded.samples.length))
            throw Error('Audio publication rejected');
          ++stats.decoded;
        } finally {Module._free(pointer);}
      }).catch(e=>failed(index,generation,e));
    },
    status(active,muted) {
      if(!catalog)return 'Sound files could not be checked. Enable sound to retry.';
      if(!stats.playable)return 'Final sound files are not installed. You can play quietly.';
      const partial=stats.playable<catalog.clips.length?' Some sound files are not installed.':'';
      if(muted)return 'Sound is muted.'+partial;
      if(!active)return 'Choose Enable sound to start or resume sound.'+partial;
      if(stats.failed.length)return 'Some sounds could not load. Enable sound to retry.'+partial;
      return 'Sound is on.'+partial;
    }
  };
})();
// Capture a real input event before engine GUI dispatch. Initial activation also
// occurs synchronously in Play/Continue/Enable sound, never after an awaited load.
for(const type of ['pointerup','keydown'])document.addEventListener(type,event=>{
  if(event.isTrusted&&(type!=='keydown'||['Enter',' '].includes(event.key)))
    window.Module?._paws_audio_gesture?.();
},true);
