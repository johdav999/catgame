/* Task-local host: all legality and commit decisions live in the Wasm module. */
let current=null,selected='',ready=false;
const byId=id=>document.getElementById(id);
window.engineStatus=(message,error)=>{byId('graphics').textContent=message;if(error)console.error(message);};
window.engineStats=(meshes,lights,layers)=>window.rulesGraphics={meshes,lights,layers};
window.enginePerformance=value=>window.rulesPerformance=value;
window.EngineQuality={report:value=>{window.rulesRender=value;byId('graphics').textContent=`Pinned renderer: ${value.state||'active'}`;}};
function same(a,b){return JSON.stringify(a)===JSON.stringify(b);}
function action(index){const v=current;Module._rules_action(index,v.revision,v.attempt,v.generation);}
window.rulesPublish=v=>{
  current=v;window.rulesView=v;
  if(!byId('quests').options.length)v.quests.forEach((title,i)=>byId('quests').add(new Option(title,i)));
  byId('title').textContent=v.title+(v.completed?' — complete':'');byId('objective').textContent=v.objective;
  byId('feedback').textContent=v.feedback;byId('pause').textContent=v.paused?'Resume':'Pause';
  byId('quests').disabled=v.paused;
  const options=v.objects.filter(o=>v.actions.some(a=>a.intent.object===o.id));
  if(!options.some(o=>o.id===selected))selected=options[0]?.id||'';
  byId('objects').replaceChildren();options.forEach(o=>byId('objects').add(new Option(o.name,o.id)));byId('objects').value=selected;
  drawActions();
  byId('grid').style.gridTemplateColumns=`repeat(${v.width},minmax(0,1fr))`;byId('grid').replaceChildren();
  for(let y=0;y<v.height;y++)for(let x=0;x<v.width;x++){
    const cell=document.createElement('div');cell.className='cell';const here=v.objects.filter(o=>!o.held&&!o.collected&&!o.role.includes('clue')&&(o.socket?o.footprint:[[...o.cell]]).some(c=>c[0]===x&&c[1]===y));
    let text=`(${x},${y}) `;if(v.basket.some(c=>c[0]===x&&c[1]===y))text+='BASKET ';if(v.shade.some(c=>c[0]===x&&c[1]===y))text+='SHADE ';if(v.quiet.some(c=>c[0]===x&&c[1]===y))text+='QUIET ';
    cell.textContent=text+here.map(o=>o.name+(o.active?' [held/active]':'')).join(' / ');byId('grid').append(cell);
  }
  byId('sequence').textContent=v.symbols.length?`Picture strip: ${v.symbols.join(' → ')}. Entered: ${v.entered.join(' → ')||'empty'}. Wrong picture clears the row. Restart clears it too.`:'';
  byId('hints').replaceChildren();v.hints.forEach(h=>{const li=document.createElement('li');li.textContent=h;byId('hints').append(li);});
  byId('states').replaceChildren();v.objects.forEach(o=>{const li=document.createElement('li');li.textContent=`${o.name}: (${o.cell}), clockwise turns ${o.orientation}; ${o.held?'held':o.collected?'retrieved':o.socket?`at ${o.socket}`:'on board'}${o.active?'; active now':''}`;byId('states').append(li);});
  byId('trace').replaceChildren();v.trace.forEach(s=>{const li=document.createElement('li');li.textContent=v.actions.find(a=>same(a.intent,s))?.label||JSON.stringify(s);byId('trace').append(li);});
};
function drawActions(){
  const focused=document.activeElement?.dataset.actionLabel;byId('actions').replaceChildren();
  current.actions.forEach((a,i)=>{
    if(a.intent.object&&a.intent.object!==selected)return;
    if(a.intent.verb===0)return;
    const row=document.createElement('div'),button=document.createElement('button');button.textContent=a.label;button.disabled=!a.enabled;button.dataset.actionIndex=i;button.dataset.actionLabel=a.label;
    button.addEventListener('click',()=>action(i));row.append(button);
    if(a.reason){const reason=document.createElement('p');reason.className='reason';reason.textContent=a.reason;row.append(reason);}byId('actions').append(row);
  });
  if(focused)Array.from(byId('actions').querySelectorAll('button')).find(b=>b.dataset.actionLabel===focused&&!b.disabled)?.focus({preventScroll:true});
}
byId('objects').addEventListener('change',e=>{selected=e.target.value;drawActions();});
byId('quests').addEventListener('change',e=>{selected='';if(ready)Module._rules_quest(Number(e.target.value));});
byId('pause').addEventListener('click',()=>{if(ready)Module._rules_pause(current.paused?0:1);});
byId('large').addEventListener('change',e=>document.documentElement.classList.toggle('large',e.target.checked));
var Module={canvas:byId('viewport'),onRuntimeInitialized(){ready=true;},postRun:[()=>Module._rules_suspend(document.hidden?1:0)],onAbort(reason){byId('feedback').textContent=`Could not start rules harness: ${reason}`;},printErr:message=>console.error(message)};
document.addEventListener('visibilitychange',()=>{if(ready)Module._rules_suspend(document.hidden?1:0);});
window.addEventListener('pagehide',()=>{if(ready)Module._rules_suspend(1);});
window.addEventListener('pageshow',()=>{if(ready)Module._rules_suspend(document.hidden?1:0);});
