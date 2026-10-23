// Full catalogue: real scene gestures and retained GUI on the generated site.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const backend=process.env.QUEST_BACKEND||'webgpu',out=process.env.EVIDENCE_DIR||`build/evidence/m6/${backend}`;
const report={backend,quests:[],errors:[]};let browser,page;
(async()=>{try{
 await fs.mkdir(out,{recursive:true});
 const from=Number(process.env.QUEST_FROM||0);
 if(from){const previous=JSON.parse(await fs.readFile(path.join(out,'report.json'),'utf8'));assert.equal(previous.quests.length,from);report.quests=previous.quests;report.build=previous.build;}
 const catalogue=(await fs.readFile('tests/fixtures/quest-migration-m5/catalogue.tsv','utf8')).trim().split('\n').slice(1).map(row=>{const [id,content,pack]=row.split('\t');return {id,content,pack};});
 browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576','--media-cache-size=1048576']});
 const context=await browser.newContext({viewport:{width:1600,height:1000},acceptDownloads:true});
 if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
 page=await context.newPage();page.setDefaultTimeout(60000);page.on('pageerror',e=>report.errors.push(e.message));
 const settle=()=>page.waitForTimeout(200),state=()=>page.evaluate(()=>pawsState);
 const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
 const saved=async()=>{await page.evaluate(()=>pawsFlushSave());await page.waitForFunction(()=>pawsState.save===2,null,{timeout:20000});};
 const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();};
 const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
 const click=async id=>{const p=(await points()).objects[id];assert(p,'Missing object '+id);await page.mouse.click(p.x,p.y);await settle();};
 const rotate=async id=>{const orientation=(await state()).objects.find(o=>o.id===id).orientation;await click(id);if((await state()).objects.find(o=>o.id===id).orientation===orientation&&(await points()).selectedPlank===id)await click(id);};
 const drag=async(id,target)=>{
  const p=await points(),a=p.objects[id],to=p.slots[target];assert(a&&to,'Missing drag '+JSON.stringify({id,target,p}));
  const o=(await state()).objects.find(o=>o.id===id),from=o&&p.slots[`${o.x},${o.y}`];
  const b=/^-?\d+,-?\d+$/.test(target)&&from?{x:a.x+to.x-from.x,y:a.y+to.y-from.y}:to;
  await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(b.x,b.y,{steps:3});await page.mouse.up();await settle();
 };
 const setup=async()=>{if(await page.evaluate(()=>pawsUi.screen===22)){const name=await page.getByRole('button',{name:/^Start setup /}).getAttribute('aria-label');await key(name);}};
 const doStep=async s=>{
  await ready();
  await setup();if(s.verb===4)return; // Pickup and fit share one drag transaction.
  await page.waitForFunction(()=>{Module._paws_interaction_snapshot();return !pawsInteraction.inputBusy;},null,{timeout:90000,polling:200});
  if(s.verb===8)await page.waitForFunction(()=>!window.pawsSequence?.watching,null,{timeout:60000});
  const before=await state();if(before.completed)return;
  if(s.verb===1){const o=before.objects.find(o=>o.id===s.object),x=o.x+[0,1,0,-1][s.direction],y=o.y+[-1,0,1,0][s.direction];
   // The raised gate crate's centre is a poor drag anchor at this camera angle.
   // Use its shipped select + arrow-key equivalent for an exact discrete move.
   if(s.object==='gate_crate'){await click(s.object);await page.locator('canvas').focus();await page.keyboard.press(['ArrowUp','ArrowRight','ArrowDown','ArrowLeft'][s.direction]);await settle();}
   else await drag(s.object,`${x},${y}`);
   const moved=(await state()).objects.find(o=>o.id===s.object);assert.equal(moved.x,x);assert.equal(moved.y,y);
  }
  else if(s.verb===2)await rotate(s.object);
  else if(s.verb===3){const water=before.objects.some(o=>o.id==='rain_barrel'),basin=before.objects.find(o=>o.id==='catch_tray'&&o.active);await click(water?(basin?'catch_tray':'rain_barrel'):'source_ball');if(!water)await page.waitForFunction(()=>pawsState.completed||!window.pawsBall?.rolling,null,{timeout:90000});}
  else if(s.verb===5||s.verb===9){const p=await points(),shape=`${s.object}@${s.x},${s.y}`;await drag(s.object,p.slots[shape]?shape:s.target);}
  else if([7,8,19,20].includes(s.verb)){await click(s.object);if(s.verb===20)await page.waitForFunction(()=>!window.pawsPepper?.active||pawsPepper.reaction<3,null,{timeout:90000});}
  else if(s.verb===21)await key('Test balance');
  else if(s.verb===6){assert((await state()).completed,'Last fit did not complete arrangement');return;}
  else throw Error('Unmapped authored step '+JSON.stringify(s));
  if(await page.evaluate(()=>pawsRenderState.state!=='ready')){await ready();const recovered=await state();if(recovered.revision===before.revision&&!recovered.completed)return doStep(s);}
  const after=await state();assert(after.revision>before.revision||after.completed,'Rejected gesture '+JSON.stringify(s)+' '+after.message);assert.equal(after.questKernel.authoritative,catalogue.length);assert.equal(after.questKernel.matched,1);
 };
 await page.goto((process.env.PAWS_URL||'http://127.0.0.1:8151/')+'?devBeach=1');await ready();assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);report.build=await page.evaluate(()=>pawsBuild);
 if(from){const previous=JSON.parse(await fs.readFile(path.join(out,'report.json'),'utf8'));assert.equal(previous.build.buildId,report.build.buildId,'Resume requires identical tested build');}
 for(const quest of catalogue.slice(from)){
  const fixture=await fs.readFile(`tests/fixtures/quest-migration-m5/${quest.id}-partial.save`,'utf8');
  assert(await page.evaluate(text=>{Module._paws_cloud_stamp();return pawsStageCloudSave(new TextEncoder().encode(text),pawsCloudStamp);},fixture),'Version-13 fixture rejected: '+quest.id);
  await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});await settle();
  assert.equal((await state()).quest,quest.id);const initial=await state(),coins=initial.coins;
  await key('Menu');await page.waitForFunction(()=>pawsState.paused);await settle();const paused=(await state()).revision;await page.waitForTimeout(150);assert.equal((await state()).revision,paused,'Paused quest advanced');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);
  const solution=(await state()).solution;let reloaded=false;
  for(let n=0;n<solution.length;++n){await doStep(solution[n]);
   if(!reloaded&&n>=Math.floor(solution.length/2)&&!(await state()).completed){
    await setup();await saved();const before=await state();await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});assert.deepEqual((await state()).objects,before.objects);assert.equal((await state()).coins,before.coins);assert.deepEqual((await state()).questKernel.stages,before.questKernel.stages);reloaded=true;
   }
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});await saved();const completed=await state();
  assert.equal(completed.coins,coins+(quest.pack?0:20));assert.equal(completed.questKernel.authoritative,catalogue.length);assert.equal(completed.sceneCount,1);
  await page.screenshot({path:path.join(out,quest.id+'-complete.png')});
  await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});await saved();assert.equal((await state()).coins,completed.coins);assert.equal((await state()).sceneCount,1);
  report.quests.push({id:quest.id,completed:true,coins:completed.coins,reloaded,sceneCount:completed.sceneCount});await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log('Catalogue browser passed',backend,quest.id);
 }
 assert.equal(report.quests.length,catalogue.length);assert.deepEqual(report.errors,[]);report.passed=true;
}catch(e){report.failure=e.stack;report.diagnostics=await page?.evaluate(()=>({state:window.pawsState,ui:window.pawsUi,render:window.pawsRenderState,text:document.body.innerText})).catch(()=>null);await page?.screenshot({path:path.join(out,'failure.png')}).catch(()=>{});process.exitCode=1;}
finally{await browser?.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({backend,passed:report.passed,quests:report.quests.length,failure:report.failure}));}})();
