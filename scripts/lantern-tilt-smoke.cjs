// Real drags and rendered beam endpoints, with an isolated saved-game fixture.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=`build/evidence/lantern-tilt/${backend}`;
const report={backend,checks:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
 const context=await browser.newContext({viewport:{width:844,height:964}});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const settle=()=>page.waitForTimeout(250),state=()=>page.evaluate(()=>pawsState);
 const saved=()=>page.waitForFunction(()=>pawsState.save===2);
 const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();};
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const drag=async(id,target)=>{const p=await points(),a=p.objects[id],b=p.slots[target];assert(a&&b);await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:12});await page.mouse.up();await settle();};
 const verify=async(sign,name)=>{
  const p=await points(),left=p.objects['balance_beam.left'],right=p.objects['balance_beam.right'];assert(left&&right);
  const delta=right.worldY-left.worldY;assert(sign===0?Math.abs(delta)<.0001:delta*sign<-.01,JSON.stringify({sign,left,right}));
  const text=await page.locator('body').innerText();assert(text.includes(sign===0?'Pull: even':sign>0?'Pull: right':'Pull: left'));
  if(sign!==0)assert((right.y-left.y)*sign>0,'Heavier end is also lower on screen in the initial view');
  report.checks.push({name,delta,left,right});await page.screenshot({path:`${out}/${name}.png`});
 };
 await page.goto('http://127.0.0.1:8080/?devBeach=1');await ready();await key('Play');await saved();
 const fixture=await fs.readFile('build/evidence/beach-outings-start.save.lantern','utf8');assert(fixture.length>100);
 await page.evaluate(async text=>{const s=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon'});await s.put('current',new TextEncoder().encode(text));s.dispose();},fixture);
 await page.reload();await ready();report.build=await page.evaluate(()=>pawsBuild);assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);
 await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1);
 await key('Adventures');await key('Start / Continue beach');await key('Help Pip');await page.waitForFunction(()=>pawsState.quest==='beach_lantern_balance'&&pawsUi.screen===2);
 await verify(1,'right-heavy');
 await drag('weight_4','beam_1');await verify(-1,'left-heavy');await saved();
 await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);await settle();await verify(-1,'left-heavy-restored');
 await key('Menu');await key('Restart puzzle');await key('Restart puzzle');
 const solution=(await state()).solution;
 for(const step of solution){if(step.verb===21)break;await drag(step.object,step.target);}
 await verify(0,'level');await key('Test balance');assert((await state()).revision>0);
 for(let completed=1;completed<4;++completed){
  await page.getByRole('img',{name:`Setup ${completed} complete!`,exact:true}).waitFor({state:'attached'});
  const text=await page.locator('body').innerText();
  assert(text.includes(`${completed} of 4 setups balanced.`)&&text.includes('returned to their storage mats'));
  assert(text.includes(completed===1?'Support moves from position 3 to 4.':completed===2?'Support moves from position 4 to 5.':'Support moves from position 5 to 4.'));
  if(completed===1)assert(text.includes('2 weight, 3 steps right')&&text.includes('3 weight, 3 steps left'));
  if(completed===3)assert(text.includes('off the beam'));
  await saved();await page.waitForTimeout(400);assert.equal(await page.getByRole('img',{name:`Setup ${completed} complete!`,exact:true}).count(),1,'Status updates retain the success card');
  if(completed===1){
   for(const size of [{width:844,height:964},{width:390,height:844},{width:844,height:390}]){
    await page.setViewportSize(size);await settle();const start=page.getByRole('button',{name:'Start setup 2',exact:true});assert(await start.isVisible());
    const box=await start.boundingBox();assert(box.x>=0&&box.y>=0&&box.x+box.width<=size.width+1&&box.y+box.height<=size.height+1);
    await page.screenshot({path:`${out}/setup-complete-${size.width}x${size.height}.png`});
   }
   await page.setViewportSize({width:844,height:964});await settle();
  }
  if(completed===2)await page.screenshot({path:`${out}/setup-2-complete.png`});
  const before=(await state()).revision;await key(`Start setup ${completed+1}`);
  await page.waitForFunction(()=>pawsUi.screen===2);assert.equal((await state()).revision,before,'Acknowledgement changes no rules or save state');
  if(completed===1){
   await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);
   assert((await page.locator('body').innerText()).includes('Setup 2 / 4.'),'Next setup survives reload without another success prompt');
   await key('Test balance');assert.equal(await page.getByRole('img',{name:'Setup 2 complete!',exact:true}).count(),0,'Failed test has no success prompt');
  }
  for(const step of solution.slice(completed*5,completed*5+4))await drag(step.object,step.target);
  await verify(0,`level-setup-${completed+1}`);await key('Test balance');
 }
 await page.getByText('Beach quest complete',{exact:true}).waitFor({timeout:30000});
 assert.equal((await state()).beachProgress,2047);assert.deepEqual(report.errors,[]);
 report.checks.push({name:'All three success transitions explain changed support/Pip load, remain until acknowledged, and lead to final completion.'});report.ok=true;
}catch(error){report.ok=false;report.failure=error.stack;await page?.screenshot({path:out+'/failure.png'}).catch(()=>{});process.exitCode=1;}
finally{await fs.writeFile(out+'/report.json',JSON.stringify(report,null,2));console.log(JSON.stringify({ok:report.ok,failure:report.failure,checks:report.checks.length}));await browser?.close();}})();
