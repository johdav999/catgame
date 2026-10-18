/* Browser lifecycle/input adapter. Visible game composition belongs to PawsUi. */
const canvas = document.getElementById('viewport');
const status = document.getElementById('status');
const startup = document.getElementById('startup');
const retry = document.getElementById('retry');
let loaded = false, runtimeFailed = false, pointer = null;
const setStatus = (text, error = false) => {
  status.textContent = text; status.dataset.error = String(error);
  if(error) { startup.hidden = false; retry.hidden = false; cancel(); }
};
const rendererMessages = [];
window.engineStatus = (text, error) => {
  rendererMessages.push({text, error:!!error});
  if(rendererMessages.length>20) rendererMessages.shift();
  if(error&&!runtimeFailed) setStatus('The garden could not be drawn. Please retry.', true);
};
window.engineStats = (meshes, lights, layers) => { window.pawsSceneStats = { meshes, lights, layers }; };
const profiling = new URLSearchParams(location.search).get('profile') === '1';
const webgpuProbes = {};
if(profiling&&navigator.gpu) {
  (async()=>{
    for(const preference of ['default','low-power','high-performance']) {
      try {
        const adapter=await navigator.gpu.requestAdapter(preference==='default'?{}:{powerPreference:preference});
        const info=adapter?.info;
        webgpuProbes[preference]=adapter?{available:true,info:info?{
          vendor:info.vendor,architecture:info.architecture,device:info.device,
          description:info.description,isFallbackAdapter:info.isFallbackAdapter
        }:null}:{available:false};
      } catch(error) {webgpuProbes[preference]={available:false,error:String(error)};}
    }
  })();
}
let profileOutput, profileStatus;
if (profiling) {
  const panel = document.createElement('aside');
  panel.setAttribute('aria-label', 'WebEngine profiling');
  panel.style.cssText = 'position:fixed;bottom:12px;right:12px;z-index:100;background:#fff8e9;color:#253e37;padding:12px;border:1px solid #29655e;border-radius:8px;font:14px monospace;max-width:450px;pointer-events:none';
  profileStatus = document.createElement('div');
  profileStatus.textContent = 'WebEngine profiler · collecting…';
  profileOutput = document.createElement('pre');
  profileOutput.id = 'paws-profile-data';
  profileOutput.hidden = true;
  panel.append(profileStatus, profileOutput);document.body.append(panel);
}
window.enginePerformance = snapshot => {
  window.pawsPerformance = snapshot;
  if (!profiling) return;
  const sample = { capturedAt: new Date().toISOString(), ...snapshot };
  let graphics;
  if(window.pawsRenderState?.backend==='webgl2') {
    const gl=canvas.getContext('webgl2'), debug=gl?.getExtension('WEBGL_debug_renderer_info');
    graphics={vendor:debug?gl.getParameter(debug.UNMASKED_VENDOR_WEBGL):null,
      renderer:debug?gl.getParameter(debug.UNMASKED_RENDERER_WEBGL):null};
  }
  profileOutput.textContent = JSON.stringify({
    build: window.pawsBuild, quality: window.pawsQuality, rendering: window.pawsRenderState,
    browser: navigator.userAgent, devicePixelRatio, wasmCapacityBytes: window.Module?.HEAPU8?.byteLength,
    graphics: {secureContext:isSecureContext,webgpuExposed:!!navigator.gpu,...graphics,rendererMessages,webgpuProbes},
    viewport: { width: innerWidth, height: innerHeight }, latest: sample
  }, (_, value) => typeof value === 'bigint' ? Number(value) : value);
  const t = snapshot.timings, c = snapshot.current, gpu = snapshot.gpu;
  const ms = value => Number.isFinite(value) ? value.toFixed(2) : '—';
  profileStatus.textContent = `WebEngine · ${ms(t.intervalMs.mean)} ms/frame · CPU ${ms(t.cpuMs.mean)} ms · GPU ${ms(gpu.totalMs?.mean)} ms (${gpu.state}) · ${c.drawCalls} draws`;
};
window.pawsMaybeReady = () => {
  if(window.pawsGuiReady && window.pawsRenderState?.state === 'ready' && !runtimeFailed) {
    startup.hidden = true; setStatus('Ready');
  }
};
window.EngineQuality = { report: report => {
  window.pawsRenderState = report;
  if(report.state === 'ready') window.pawsMaybeReady();
  else if(window.pawsGuiReady) { cancel(); startup.hidden=false;setStatus('Restoring the garden display…'); }
} };
var Module = {
  canvas,
  onRuntimeInitialized() { loaded = true; window.pawsRuntimeReady = true; },
  postRun: [async () => {
    invoke('_paws_suspend', document.hidden ? 1 : 0);
    try {
      setStatus('Preparing the garden lettering…');
      const names=['NunitoSans-Regular.ttf','NunitoSans-Semibold.ttf','Fraunces-Semibold.ttf'];
      const fonts=await Promise.all(names.map(async name=>{
        const response=await fetch(`assets/fonts/${name}`);
        if(!response.ok) throw new Error('Font download failed');
        return new Uint8Array(await response.arrayBuffer());
      }));
      fonts.forEach((bytes,index)=>{
        if(bytes.length>4*1024*1024) throw new Error('Font size limit');
        const p=Module._malloc(bytes.length);
        if(!p) throw new Error('Font allocation failed');
        try {Module.HEAPU8.set(bytes,p);Module._paws_font(index,p,bytes.length);} finally {Module._free(p);}
      });
      invoke('_paws_motion', matchMedia('(prefers-reduced-motion: reduce)').matches ? 1 : 0);
      await window.pawsLoadSave();
      window.pawsRefreshCloudAccess?.();
    } catch { runtimeFailed=true;setStatus('The lettering could not load. Check your connection and retry.',true); }
  }],
  onAbort() { runtimeFailed = true; setStatus('The garden could not start. Please retry.', true); },
  printErr: text => console.error(text)
};
function invoke(action, ...args) { if(loaded && !runtimeFailed) return Module[action]?.(...args); }
function blocked() { return !startup.hidden || !!invoke('_paws_blocks_scene'); }
function cancel() {
  invoke('_paws_object_pointer',3,0,0);
  pointer=null;
  if(loaded) { invoke('_engine_gui_blur');invoke('_paws_pointer',-1,-1,0); }
}
function local(event) {const r=canvas.getBoundingClientRect();return [event.clientX-r.left,event.clientY-r.top];}
canvas.addEventListener('pointerdown', event => {
  if(pointer || event.button !== 0) return;
  const [x,y]=local(event);invoke('_paws_pointer',x,y,1);
  const gui=window.EngineGui?.pointer(1,event);
  const owner=gui||blocked()?'gui':invoke('_paws_object_pointer',0,x,y)?'object':'camera';
  pointer={id:event.pointerId,x:event.clientX,y:event.clientY,owner};
  canvas.setPointerCapture(event.pointerId);
  if(!gui) canvas.focus();
});
canvas.addEventListener('pointermove', event => {
  const [x,y]=local(event);invoke('_paws_pointer',x,y,Number(!!pointer));
  if(!pointer) { window.EngineGui?.pointer(0,event);return; }
  if(pointer.id!==event.pointerId) return;
  if(pointer.owner==='gui') window.EngineGui?.pointer(0,event);
  else if(pointer.owner==='object'&&!blocked()) invoke('_paws_object_pointer',1,x,y);
  else if(!blocked()) invoke('_paws_orbit',event.clientX-pointer.x,event.clientY-pointer.y);
  else {cancel();return;}
  pointer.x=event.clientX;pointer.y=event.clientY;
});
for(const type of ['pointerup','pointercancel','lostpointercapture']) canvas.addEventListener(type,event=>{
  if(pointer?.id!==event.pointerId) return;
  const owner=pointer.owner;pointer=null;
  invoke('_paws_pointer',...local(event),0);
  if(owner==='gui') window.EngineGui?.pointer(type==='pointerup'?2:3,event);
  if(owner==='object') invoke('_paws_object_pointer',type==='pointerup'?2:3,...local(event));
});
canvas.addEventListener('pointerleave',()=>{if(!pointer)invoke('_paws_pointer',-1,-1,0);});
canvas.addEventListener('wheel',event=>{
  event.preventDefault();if(window.EngineGui?.wheel(event)||blocked())return;
  if(pointer?.owner==='object')cancel();
  invoke('_paws_zoom',Math.sign(event.deltaY)*80);
},{passive:false});
document.addEventListener('keydown',event=>{
  if(document.getElementById('cloud-panel')?.open)return;
  if(event.key==='Escape'&&loaded&&!event.repeat) {event.preventDefault();event.stopImmediatePropagation();cancel();invoke('_paws_escape');}
  if(document.activeElement===canvas&&!blocked()) {
    const key={ArrowUp:0,ArrowRight:1,ArrowDown:2,ArrowLeft:3,r:4,R:4,' ':5}[event.key];
    if(key!==undefined&&invoke('_paws_object_key',key)){event.preventDefault();event.stopImmediatePropagation();}
  }
},true);
canvas.addEventListener('keydown',event=>{
  if(blocked()||window.EngineGui?.ownsKeyboard())return;
  const d={ArrowLeft:[-12,0],ArrowRight:[12,0],ArrowUp:[0,-12],ArrowDown:[0,12]};
  if(d[event.key]) {event.preventDefault();invoke('_paws_orbit',...d[event.key]);}
  if(['+','=','-'].includes(event.key)) {event.preventDefault();invoke('_paws_zoom',event.key==='-'?80:-80);}
});
window.addEventListener('blur',cancel);
window.addEventListener('resize',cancel);
let pageAway=false, pageFrozen=false, accountModal=false;
window.pawsAccountModal=value=>{accountModal=!!value;lifecycle();};
function lifecycle() {
  cancel();const suspended=document.hidden||pageAway||pageFrozen||accountModal;
  invoke('_paws_suspend',suspended?1:0);
  if(suspended)window.pawsFlushSave?.();
}
document.addEventListener('visibilitychange',lifecycle);
window.addEventListener('pagehide',()=>{pageAway=true;lifecycle();});
window.addEventListener('pageshow',()=>{pageAway=false;lifecycle();});
document.addEventListener('freeze',()=>{pageFrozen=true;lifecycle();});
document.addEventListener('resume',()=>{pageFrozen=false;lifecycle();});
retry.addEventListener('click',()=>location.reload());
window.addEventListener('error',event=>{
  if(event.target?.tagName==='SCRIPT') {runtimeFailed=true;setStatus('Some game files are missing. Please retry.',true);}
},true);
if(!isSecureContext||!navigator.gpu||typeof WebAssembly==='undefined') {
  runtimeFailed=true;setStatus('Paws needs WebGPU in a supported browser over HTTPS (or localhost). Please update your browser and retry.',true);
} else {
  // Wait for the deferred storage/audio adapters before starting the runtime.
  // Unsupported hosts retain the readable shell without entering Emscripten.
  document.addEventListener('DOMContentLoaded',()=>{
    const runtime=document.createElement('script');
    runtime.src='paws.js';document.head.appendChild(runtime);
  },{once:true});
}
window.pawsHostState=()=>({owner:pointer?.owner||null,blocked:blocked(),build:window.pawsBuild?.buildId||'unidentified'});

