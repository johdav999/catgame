// Fixture selects the picnic; all puzzle actions use normal retained UI and drag input.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu';
const version=process.env.PICNIC_VERSION||'v2';
const out=`build/evidence/picnic-identity/${backend}${version==='v1'?'-v1':''}`;
const report={backend,version,errors:[],checks:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const context=await browser.newContext({viewport:{width:1600,height:1000}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:120000});
 const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await page.waitForTimeout(200);};
 const state=()=>page.evaluate(()=>pawsState);
 async function drag(id,target){
  const p=await page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
  const a=p.objects[id],b=p.slots[target];assert(a&&b,JSON.stringify({id,target,p}));
  await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});await page.mouse.up();await page.waitForTimeout(220);
 }
 await page.goto(process.env.PAWS_URL||'http://127.0.0.1:8080/?devBeach=1');await ready();
 let fixture=await fs.readFile(process.env.PICNIC_FIXTURE||'build/evidence/picnic-identity/fixtures/beach-picnic.save','utf8');
 if(version==='v1')fixture=fixture.replace('beach-picnic-v2','beach-picnic-v1');
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});try{await s.put('current',new TextEncoder().encode(text));}finally{s.dispose();}},fixture);
 await page.reload();await ready();await key('Continue');
 await page.waitForFunction(()=>pawsState.quest==='beach_picnic'&&pawsUi.screen===2,null,{timeout:120000});
 report.build=await page.evaluate(()=>pawsBuild);report.render=await page.evaluate(()=>pawsRenderState);assert.equal(report.render.backend,backend);
 await page.waitForTimeout(1000);
 for(const [name,width,height] of [['desktop',1600,1000],['portrait',390,844],['landscape',844,390]]){
  await page.setViewportSize({width,height});await page.waitForTimeout(600);await page.screenshot({path:path.join(out,`${name}.png`)});
 }
 await page.setViewportSize({width:1600,height:1000});await page.waitForTimeout(400);
 const initial=await state();let testedMat=false;
 for(const step of initial.solution){
  if([4,6].includes(step.verb))continue;
  const before=await state();
  if(step.verb===1){
   const o=before.objects.find(o=>o.id===step.object),dx=[0,1,0,-1][step.direction],dy=[-1,0,1,0][step.direction];
   await drag(step.object,`${o.x+dx},${o.y+dy}`);
  }else{
   assert.equal(step.verb,5);await drag(step.object,step.target);
   if(!testedMat){
    assert((await state()).revision>before.revision,'Named mat remains draggable');
    await page.screenshot({path:path.join(out,'mat-moved.png')});
    await key('Undo');assert.deepEqual((await state()).objects,before.objects,'Undo restores named mats');
    await drag(step.object,step.target);testedMat=true;
   }
  }
 }
 await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:25000});
 await page.screenshot({path:path.join(out,'completed.png')});
 report.checks.push('Named mats remain draggable','Undo restores starting arrangement','Picnic solves through real pointer drags','Desktop, portrait, landscape and completion screenshots');
 assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;process.exitCode=1;
 if(page){report.state=await page.evaluate(()=>({state:window.pawsState,ui:window.pawsUi})).catch(()=>null);await page.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});}
}finally{
 await fs.mkdir(out,{recursive:true});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));
 await browser?.close();console.log(JSON.stringify(report,null,2));
}})();
