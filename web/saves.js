/* Paws save scheduling, session ownership and file UI. EngineStorage owns bytes
   and transactions; Afternoon validates every candidate and owns publication. */
(() => {
  'use strict';
  const limit=16*1024*1024, session=crypto.randomUUID();
  let storage, namespace='paws-afternoon', mode=0, notice='', original=null;
  let initialized=false, initializing, loaded=false, loadGeneration=0, fileGeneration=0;
  let pending=null, timer=null, writes=Promise.resolve(), historyIndex=0;
  let latestSequence=0, lastWriteError=null, retrying=null;
  let migrationOriginal=null,migrationBackedUp=false;
  const metrics={requested:0,committed:0,failed:0};
  const encoder=new TextEncoder();
  function withBytes(bytes,callback) {
    const p=Module._malloc(bytes.length+1);if(!p)throw Error('Allocation failed');
    try {Module.HEAPU8.set(bytes,p);Module.HEAPU8[p+bytes.length]=0;return callback(p,bytes.length);}
    finally {Module._free(p);}
  }
  const withText=(text,callback)=>withBytes(encoder.encode(text),callback);
  const valid=bytes=>withBytes(bytes,(p,n)=>Module._paws_validate_save(p,n));
  const tell=text=>withText(text,p=>Module._paws_notice(p));
  function publishMode() {
    const message=[notice,lastWriteError?.help].filter(Boolean).join(' ');
    withText(session,s=>withText(message,n=>Module._paws_storage_mode(mode,s,n,original?1:0)));
  }
  function writeError(error) {
    const code=error?.code||'io', name=error?.cause?.name||error?.name||'Error';
    const help=code==='quota'||name==='QuotaExceededError'
      ? 'Browser storage is full. Free some disk space, then choose Retry save. Export a copy before closing this tab.'
      : code==='unavailable'
      ? 'The browser is blocking storage. Allow site storage, then choose Retry save. Export a copy before closing this tab.'
      : 'The browser could not write the save. Retry save will reconnect to storage. Export a copy before closing this tab.';
    return {code,name,message:String(error?.message||'Browser storage failed.').slice(0,500),help};
  }
  function backend() {return storage||(storage=new EngineStorage.IndexedDBStorage({namespace,maxBytes:limit}));}
  function isolate(reason) {
    namespace='paws-isolated-'+session;storage=undefined;mode=0;
    notice=reason+' This is an independent save. Export it before leaving; the usual garden stays unchanged.';
  }
  async function initialize() {
    if(initialized)return;
    if(initializing)return initializing;
    initializing=(async()=>{
      if(navigator.locks?.request) {
        await new Promise(resolve=>{
          navigator.locks.request('paws-afternoon-writer',{mode:'exclusive',ifAvailable:true},lock=>{
            if(!lock){mode=1;notice='Another tab is keeping the usual save. This tab is temporary; export to keep its progress.';resolve();return;}
            resolve();
            // Hold through hidden/background/BFCache. The browser releases on termination.
            return new Promise(()=>{});
          }).catch(()=>{isolate('Browser locking is unavailable.');resolve();});
        });
      } else isolate('Browser locking is unavailable.');
      initialized=true;publishMode();
    })();
    return initializing;
  }
  window.pawsLoadSave=async()=>{
    if(loaded)return;
    await initialize();const generation=++loadGeneration;
    window.pawsStorageReady=false;
    try {
      const candidates=[];let damaged=false;
      // Fixed bounded slots: never scan attacker-controlled/unbounded key sets.
      for(const key of ['current',...Array.from({length:5},(_,i)=>'history-'+i)]) {
        let bytes;
        try {bytes=await backend().get(key);}catch(error){if(error.code!=='corrupt')throw error;damaged=true;continue;}
        if(generation!==loadGeneration)return;
        if(!bytes)continue;
        const revision=valid(bytes);
        if(revision<0){damaged=true;if(!original||key==='current')original=bytes;continue;}
        candidates.push({bytes,revision,key});
      }
      candidates.sort((a,b)=>b.revision-a.revision);
      const best=candidates[0];
      if(best&&!new TextDecoder().decode(best.bytes.subarray(0,32)).startsWith('PAWS_AFTERNOON 14')) {
        // Keep the exact pre-migration bytes outside the rotating history slots.
        migrationOriginal=best.bytes;if(!original)original=best.bytes;
      }
      // Retain access to the migration original after later page reloads and
      // history rotation. Damage to an optional backup cannot replace current.
      if(!original)try{original=await backend().get('pre-quest-m6')||await backend().get('pre-quest-m5');}catch{}
      if(damaged) {
        if(mode!==1)isolate(best?'An earlier valid afternoon was recovered. The damaged or newer-format data is preserved.':'The saved data could not be read and has been preserved.');
        else notice+=' Some saved data could not be read and has been preserved.';
      }
      if(best)historyIndex=(best.revision+1)%5;
      publishMode();
      const ok=best?withBytes(best.bytes,(p,n)=>Module._paws_restore(p,n,1)):Module._paws_restore(0,0,0);
      loaded=window.pawsStorageReady=!!ok;
      if(best&&damaged)tell('Recovered a complete earlier afternoon. Export a copy from Save and recovery.');
    } catch {
      if(generation!==loadGeneration)return;
      mode=2;notice='Storage is unavailable. Play temporarily and export a file to keep your afternoon.';publishMode();
      loaded=window.pawsStorageReady=!!Module._paws_restore(0,0,0);
    }
  };
  function flush() {
    clearTimeout(timer);timer=null;
    if(!pending)return writes;
    const snapshot=pending;pending=null;
    const slot='history-'+(historyIndex++%5);
    writes=writes.then(async()=>{
      try {
        // Resolve after queued recovery so new writes never borrow a disposed adapter.
        const destination=backend();
        if(migrationOriginal&&!migrationBackedUp) {
          if(!await destination.get('pre-quest-m6'))await destination.put('pre-quest-m6',migrationOriginal);
          migrationBackedUp=true;
        }
        // A full history record commits before replacing current. Failed writes
        // retain earlier complete records; no wallet/ledger fields are split.
        await destination.put(slot,snapshot.bytes);
        await destination.put('current',snapshot.bytes);
        ++metrics.committed;
        if(snapshot.sequence===latestSequence&&lastWriteError){lastWriteError=null;publishMode();}
        Module._paws_saved(snapshot.sequence,1);
      } catch(error) {
        ++metrics.failed;
        if(snapshot.sequence===latestSequence){lastWriteError=writeError(error);publishMode();}
        Module._paws_saved(snapshot.sequence,0);
      }
    });
    return writes;
  }
  window.pawsStoreSave=(text,sequence,immediate=true,revision=0)=>{
    ++loadGeneration;++metrics.requested;
    if(mode!==0)return;
    latestSequence=sequence;
    pending={bytes:encoder.encode(text),sequence,revision};
    if(immediate)flush();
    else {clearTimeout(timer);timer=setTimeout(flush,500);}
  };
  window.pawsFlushSave=flush;
  window.pawsCloudSaveText=async()=>{
    if(!loaded||mode!==0)throw Error('Cloud saves need the main writable game tab.');
    await flush();Module._paws_cloud_snapshot();
    const text=window.pawsCloudSnapshot;delete window.pawsCloudSnapshot;
    if(typeof text!=='string'||valid(encoder.encode(text))<0)throw Error('Your afternoon is not ready to save.');
    return text;
  };
  window.pawsCloudSaveStamp=()=>{
    if(!loaded||mode!==0)throw Error('Cloud saves need the main writable game tab.');
    Module._paws_cloud_stamp();return {...window.pawsCloudStamp};
  };
  window.pawsStageCloudSave=(bytes,stamp)=>{
    if(!loaded||mode!==0||valid(bytes)<0)return false;
    return withBytes(bytes,(p,n)=>Module._paws_stage_import(p,n,stamp.attempt,stamp.revision));
  };
  window.pawsSetCloudEntitlements=items=>{
    if(window.pawsRuntimeReady)Module._paws_cloud_beach_access(items.includes('beach_afternoon')?1:0);
  };
  window.pawsRetrySave=async()=>{
    if(!loaded){await window.pawsLoadSave();return;}
    if(mode===1){tell('The other tab owns the usual save. Export this temporary afternoon to keep it.');return;}
    if(retrying)return retrying;
    if(mode===2){isolate('Storage is being retried in an independent save.');publishMode();}
    // Finish queued transactions before reconnecting. A browser may have closed
    // the cached database after an I/O failure; repeating put cannot reopen it.
    // Keep the namespace, Web Lock and records, then snapshot the latest game.
    flush();
    writes=writes.then(()=>{storage?.dispose();storage=undefined;});
    retrying=writes.then(()=>Module._paws_retry_save()).finally(()=>{retrying=null;});
    return retrying;
  };
  function download(bytes,name) {
    const url=URL.createObjectURL(new Blob([bytes],{type:'application/octet-stream'}));
    const link=document.createElement('a');link.href=url;link.download=name;link.click();
    setTimeout(()=>URL.revokeObjectURL(url),30000);
  }
  window.pawsDownloadSave=text=>download(encoder.encode(text),'paws-afternoon.save');
  window.pawsExportOriginal=()=>{if(original)download(original,'paws-original.save');};
  window.pawsChooseImport=(attempt,revision)=>{
    const generation=++fileGeneration;
    const input=document.createElement('input');input.type='file';input.accept='.save,.txt,application/octet-stream';
    input.setAttribute('aria-label','Choose a Paws saved afternoon');input.hidden=true;document.body.append(input);
    input.addEventListener('cancel',()=>input.remove(),{once:true});
    input.addEventListener('change',async()=>{
      const file=input.files?.[0];input.remove();if(!file)return;
      try {
        if(file.size>limit)throw Error('size');
        const bytes=new Uint8Array(await file.arrayBuffer());
        if(generation!==fileGeneration)return;
        const ok=withBytes(bytes,(p,n)=>Module._paws_stage_import(p,n,attempt,revision));
        if(!ok)tell('That file could not be imported. Your current afternoon is unchanged.');
      } catch {tell('That file could not be read or is too large. Your current afternoon is unchanged.');}
    },{once:true});
    input.click();
  };
  window.pawsSaveDiagnostics=()=>({mode,namespace,session,notice,lastWriteError,pending:!!pending,...metrics});
})();
