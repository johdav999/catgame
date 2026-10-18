/* Development host only. Browser events stay outside portable scene composition. */
const canvas=document.querySelector('#viewport'),state=document.querySelector('#status');
let ready=false,pointer=null;
function invoke(name,...args){if(ready)Module[name](...args);}
const aspect=()=>canvas.clientWidth/canvas.clientHeight;
window.engineStatus=(text,error)=>{if(error){state.textContent=text;state.dataset.error='true';}};
window.engineStats=(meshes,lights,layers)=>window.pawsSceneStats={meshes,lights,layers};
window.enginePerformance=s=>window.pawsPerformance=s;
window.EngineQuality={report:r=>window.pawsRenderState=r};
window.gardenStatus=(value,text)=>{window.gardenState=value;state.textContent=text;state.dataset.error=String(value==='failed');document.querySelector('#retry').hidden=value!=='failed';document.querySelector('#actions').disabled=value!=='ready';if(value==='ready'){invoke('_garden_resize',aspect());invoke('_garden_shadows',document.querySelector('#shadows').checked?1:0);}};
var Module={canvas,onRuntimeInitialized(){ready=true;},postRun:[()=>invoke('_garden_suspend',document.hidden?1:0)],onAbort(reason){window.gardenStatus('failed',`Garden could not start: ${reason}`);},printErr:text=>console.error(text)};
document.querySelectorAll('[data-pose]').forEach(b=>b.addEventListener('click',()=>{invoke('_garden_pose',Number(b.dataset.pose));document.querySelectorAll('[data-pose]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));}));
document.querySelector('#view').addEventListener('change',e=>invoke('_garden_view',Number(e.target.value),aspect()));
fetch('views.json').then(r=>r.json()).then(names=>names.forEach((name,i)=>{const o=document.createElement('option');o.value=i;o.textContent=name.replaceAll('_',' ');document.querySelector('#view').appendChild(o);}));
document.querySelector('#reset').addEventListener('click',()=>{document.querySelector('#view').value='0';invoke('_garden_view',0,aspect());});
document.querySelector('#retry').addEventListener('click',()=>invoke('_garden_retry'));
document.querySelector('#shadows').addEventListener('change',e=>invoke('_garden_shadows',e.target.checked?1:0));
new ResizeObserver(()=>invoke('_garden_resize',aspect())).observe(canvas);
canvas.addEventListener('pointerdown',e=>{if(pointer||e.button!==0)return;pointer={id:e.pointerId,x:e.clientX,y:e.clientY};canvas.setPointerCapture(e.pointerId);canvas.focus();});
canvas.addEventListener('pointermove',e=>{if(pointer?.id!==e.pointerId)return;invoke('_garden_orbit',e.clientX-pointer.x,e.clientY-pointer.y);pointer.x=e.clientX;pointer.y=e.clientY;});
for(const type of ['pointerup','pointercancel','lostpointercapture'])canvas.addEventListener(type,e=>{if(pointer?.id===e.pointerId)pointer=null;});
canvas.addEventListener('wheel',e=>{e.preventDefault();invoke('_garden_zoom',Math.sign(e.deltaY)*80);},{passive:false});
canvas.addEventListener('keydown',e=>{const d={ArrowLeft:[-12,0],ArrowRight:[12,0],ArrowUp:[0,-12],ArrowDown:[0,12]}[e.key];if(d){e.preventDefault();invoke('_garden_orbit',...d);}if(['+','=','-'].includes(e.key)){e.preventDefault();invoke('_garden_zoom',e.key==='-'?80:-80);}});
document.addEventListener('visibilitychange',()=>invoke('_garden_suspend',document.hidden?1:0));
window.addEventListener('pagehide',()=>invoke('_garden_suspend',1));window.addEventListener('pageshow',()=>invoke('_garden_suspend',document.hidden?1:0));window.addEventListener('blur',()=>{pointer=null;});
if(!navigator.gpu)window.gardenStatus('failed','This preview needs a browser with WebGPU.');
