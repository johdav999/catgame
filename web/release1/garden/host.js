/* Isolated development controls. Every purchase/place/trace reaches Afternoon.
   DOM, pointer ownership and lifecycle remain in this host adapter. */
const canvas=document.querySelector('#viewport'),by=id=>document.getElementById(id);
let ready=false,pointer=null,lastRequestKey='',projection;
const items=['Garden cushion','Drinking bowl','Scratching post','Sunny perch','Hammock','Playhouse','Low picnic table','Picnic mat'];
const spots=['Beside the bench','Sunny corner','Quiet nook','Scratch corner','Perch corner','Hammock nook','Playhouse corner','Picnic table spot','Picnic mat spot'];
const quests=['pip_missing_ball','pip_tidy_toys','pip_stuck_gate','button_mouse_bench','mabel_sunny_spot','button_cushion_bed','mabel_drinking_route','pepper_garden_bells','pip_pepper_playhouse','garden_picnic'];
function invoke(name,...args){if(ready)Module[name](...args);}
function add(select,names){names.forEach((name,i)=>{let o=document.createElement('option');o.value=i;o.textContent=name;select.appendChild(o);});}
add(by('item'),items);add(by('spot'),spots);spots.forEach((name,i)=>{let o=document.createElement('option');o.value=i+5;o.textContent=name;by('view').appendChild(o);});
const aspect=()=>canvas.clientWidth/canvas.clientHeight;
window.engineStatus=(text,error)=>{if(error){by('status').textContent=text;by('status').dataset.error='true';}};
window.engineStats=(meshes,lights,layers)=>window.pawsSceneStats={meshes,lights,layers};window.enginePerformance=s=>window.pawsPerformance=s;window.EngineQuality={report:r=>window.pawsRenderState=r};
window.gardenStatus=(state,text)=>{window.gardenState=state;by('status').textContent=text;by('status').dataset.error=String(state==='failed');by('retry').hidden=state!=='failed';if(state==='ready'){invoke('_garden_resize',aspect());invoke('_garden_shadows',by('shadows').checked?1:0);}};
window.gardenProjection=v=>{
 window.gardenSnapshot=projection=v;by('progress').textContent=`Chapter ${v.chapter} · ${v.coins} coins`;by('feedback').textContent=v.message;by('save').textContent=v.save;by('pause').textContent=v.paused?'Resume':'Pause';by('reduced').checked=v.reduced;
 by('gates').replaceChildren(...v.cats.map(c=>{const li=document.createElement('li');li.textContent=`${c.name}: ${c.introduced?'introduced':'not introduced'}; ${c.available?'model loaded':'accepted model pending'}`;return li;}));
 const c=v.cats[Number(by('cat').value)],n=Number(by('item').value);by('gate').textContent=c.uses[n];
 by('dialogue').textContent=c.introduced?`${c.name} has made a place in this garden.`:["Pip is ready to play.","Mabel: A sunny place would be lovely.","Button: Could you help me find a quiet place?","Pepper: Let's see what the garden can do!"][Number(by('cat').value)];
 const key=JSON.stringify([by('cat').value,c.requests]);if(key!==lastRequestKey){lastRequestKey=key;by('requests').replaceChildren();for(const id of c.requests){const b=document.createElement('button');b.textContent='Open request: '+id.replaceAll('_',' ');b.addEventListener('click',()=>invoke('_garden_quest',quests.indexOf(id)));by('requests').appendChild(b);}}
 by('use').disabled=!!c.uses[n]||v.paused||v.suspended;
};
var Module={canvas,onRuntimeInitialized(){ready=true;},postRun:[()=>invoke('_garden_suspend',document.hidden?1:0)],onAbort:r=>gardenStatus('failed',String(r)),printErr:t=>console.error(t)};
by('retry').onclick=()=>invoke('_garden_rebuild');by('rebuild').onclick=()=>invoke('_garden_rebuild');by('cancel-load').onclick=()=>invoke('_garden_cancel_loading');
by('trace').onclick=()=>invoke('_garden_next_trace');by('buy').onclick=()=>invoke('_garden_action',0,Number(by('item').value),-1);by('place').onclick=()=>invoke('_garden_action',1,Number(by('item').value),Number(by('spot').value));
by('preview').onclick=()=>invoke('_garden_preview',Number(by('item').value),Number(by('spot').value));by('cancel').onclick=()=>invoke('_garden_preview',-1,-1);
by('use').onclick=()=>invoke('_garden_use',Number(by('item').value),Number(by('cat').value));by('cancel-use').onclick=()=>invoke('_garden_cancel_use');by('meet').onclick=()=>invoke('_garden_meet',Number(by('cat').value));by('attention').onclick=()=>invoke('_garden_attention',Number(by('cat').value));
by('gather').onclick=()=>invoke('_garden_gather',Number(by('item').value));by('celebrate').onclick=()=>invoke('_garden_celebration');
for(const id of ['cat','item'])by(id).onchange=()=>{if(projection)gardenProjection(projection);};
by('pause').onclick=()=>invoke('_garden_pause',projection?.paused?0:1);by('reduced').onchange=e=>invoke('_garden_reduced',e.target.checked?1:0);by('shadows').onchange=e=>invoke('_garden_shadows',e.target.checked?1:0);by('view').onchange=e=>invoke('_garden_view',Number(e.target.value));by('retry-save').onclick=()=>window.pawsRetrySave();
new ResizeObserver(()=>invoke('_garden_resize',aspect())).observe(canvas);
canvas.onpointerdown=e=>{if(pointer||e.button!==0)return;pointer={id:e.pointerId,x:e.clientX,y:e.clientY};canvas.setPointerCapture(e.pointerId);canvas.focus();};canvas.onpointermove=e=>{if(pointer?.id!==e.pointerId)return;invoke('_garden_orbit',e.clientX-pointer.x,e.clientY-pointer.y);pointer.x=e.clientX;pointer.y=e.clientY;};
for(const event of ['pointerup','pointercancel','lostpointercapture'])canvas.addEventListener(event,e=>{if(pointer?.id===e.pointerId)pointer=null;});
canvas.onkeydown=e=>{const d={ArrowLeft:[-12,0],ArrowRight:[12,0],ArrowUp:[0,-12],ArrowDown:[0,12]}[e.key];if(d){e.preventDefault();invoke('_garden_orbit',...d);}};
document.addEventListener('visibilitychange',()=>{pointer=null;invoke('_garden_suspend',document.hidden?1:0);});window.addEventListener('pagehide',()=>{invoke('_garden_suspend',1);window.pawsFlushSave();});window.addEventListener('pageshow',()=>invoke('_garden_suspend',document.hidden?1:0));window.addEventListener('blur',()=>{pointer=null;});
