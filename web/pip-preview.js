/* Development inspection host; production game GUI remains in Paws' engine GUI. */
const canvas=document.querySelector('#viewport');
const state=document.querySelector('#status');
let ready=false,selected=0,pointer=null;
const labels=['Relaxed idle / blink','Attentive request','Happy pounce','Garden walk','Settle on cushion','Curled rest','Approach bowl','Drink','Return to idle'];
const reduced=document.querySelector('#reduced');reduced.checked=matchMedia('(prefers-reduced-motion: reduce)').matches;
function invoke(name,...args){if(ready)Module[name](...args);}
window.engineStatus=(text,error)=>{if(error){state.textContent=text;state.dataset.error='true';}};
window.engineStats=(meshes,lights,layers)=>window.pawsSceneStats={meshes,lights,layers};
window.enginePerformance=snapshot=>window.pawsPerformance=snapshot;
window.EngineQuality={report:report=>window.pawsRenderState=report};
window.pipStatus=(value,text)=>{
  window.pipState=value;state.textContent=text;state.dataset.error=String(value==='failed');
  document.querySelector('#retry').hidden=value!=='failed';document.querySelector('#actions').disabled=value!=='ready';
  if(value==='ready')choose(selected);
};
var Module={canvas,onRuntimeInitialized(){ready=true;},postRun:[()=>invoke('_pip_suspend',document.hidden?1:0)],onAbort(reason){window.pipStatus('failed',`Pip could not start: ${reason}`);},printErr:text=>console.error(text)};
function choose(index){selected=index;invoke('_pip_action',index,reduced.checked?1:0);document.querySelectorAll('#clips button').forEach((b,i)=>b.setAttribute('aria-pressed',String(i===index)));window.pipAction=index;window.pipReduced=reduced.checked;}
labels.forEach((label,index)=>{const b=document.createElement('button');b.textContent=label;b.setAttribute('aria-pressed',String(index===0));b.addEventListener('click',()=>choose(index));document.querySelector('#clips').append(b);});
reduced.addEventListener('change',()=>choose(selected));
function reviewWalk(moving,sample=-1){choose(3);invoke('_pip_walk_review',moving?1:0,reduced.checked?0:sample);invoke('_pip_review_view',Number(document.querySelector('#walk-view').value));}
document.querySelector('#walk-place').addEventListener('click',()=>reviewWalk(false));
document.querySelector('#walk-lane').addEventListener('click',()=>reviewWalk(true));
document.querySelector('#walk-phase').addEventListener('input',e=>reviewWalk(false,Number(e.target.value)));
document.querySelector('#walk-view').addEventListener('change',e=>invoke('_pip_review_view',Number(e.target.value)));
document.querySelector('#shadows').addEventListener('change',e=>invoke('_pip_shadows',e.target.checked?1:0));
document.querySelector('#close').addEventListener('click',()=>invoke('_pip_view',0));document.querySelector('#wide').addEventListener('click',()=>invoke('_pip_view',1));
document.querySelector('#retry').addEventListener('click',()=>invoke('_pip_retry'));
canvas.addEventListener('pointerdown',e=>{if(pointer||e.button!==0)return;pointer={id:e.pointerId,x:e.clientX,y:e.clientY};canvas.setPointerCapture(e.pointerId);canvas.focus();});
canvas.addEventListener('pointermove',e=>{if(pointer?.id!==e.pointerId)return;invoke('_pip_orbit',e.clientX-pointer.x,e.clientY-pointer.y);pointer.x=e.clientX;pointer.y=e.clientY;});
for(const type of ['pointerup','pointercancel','lostpointercapture'])canvas.addEventListener(type,e=>{if(pointer?.id===e.pointerId)pointer=null;});
canvas.addEventListener('wheel',e=>{e.preventDefault();invoke('_pip_zoom',Math.sign(e.deltaY)*80);},{passive:false});
canvas.addEventListener('keydown',e=>{const d={ArrowLeft:[-12,0],ArrowRight:[12,0],ArrowUp:[0,-12],ArrowDown:[0,12]}[e.key];if(d){e.preventDefault();invoke('_pip_orbit',...d);}if(['+','=','-'].includes(e.key)){e.preventDefault();invoke('_pip_zoom',e.key==='-'?80:-80);}});
document.addEventListener('visibilitychange',()=>invoke('_pip_suspend',document.hidden?1:0));window.addEventListener('pagehide',()=>invoke('_pip_suspend',1));window.addEventListener('pageshow',()=>invoke('_pip_suspend',document.hidden?1:0));window.addEventListener('blur',()=>{pointer=null;});
if(!navigator.gpu)window.pipStatus('failed','Pip needs a browser with WebGPU. Open this page in a supported browser.');

function reviewBlink(sample){choose(0);invoke("_pip_blink_review",reduced.checked?2.25:sample,Number(document.querySelector("#blink-view").value));}
document.querySelector("#blink-play").addEventListener("click",()=>reviewBlink(-1));
document.querySelector("#blink-phase").addEventListener("input",e=>reviewBlink(Number(e.target.value)));
document.querySelector("#blink-view").addEventListener("change",()=>reviewBlink(Number(document.querySelector("#blink-phase").value)));
