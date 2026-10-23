// M5: actual retained GUI, mouse puzzle input, storage migration and reload.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
const base=process.env.PAWS_URL||'http://127.0.0.1:8151/';
const out=process.env.EVIDENCE_DIR||'build/evidence/m5/browser';
(async()=>{
 await fs.mkdir(out,{recursive:true});const report={runs:[]};
 const browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe',args:['--disk-cache-size=1048576']});
 try{for(const backend of ['webgpu','webgl2'])for(const id of ['pip_missing_ball','beach_fresh_water']){
  const context=await browser.newContext({viewport:{width:1600,height:1000},acceptDownloads:true});
  if(backend==='webgl2')await context.addInitScript(()=>{navigator.gpu.requestAdapter=async()=>null;});
  const page=await context.newPage(),errors=[],run={backend,id};page.on('pageerror',e=>errors.push(e.message));
  const settle=()=>page.waitForTimeout(220),state=()=>page.evaluate(()=>pawsState);
  const ready=()=>page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});
  const saved=async()=>{await page.evaluate(()=>pawsFlushSave());await page.waitForFunction(()=>pawsState.save===2,null,{timeout:20000});};
  const key=async name=>{await page.getByRole('button',{name,exact:true}).focus();await page.keyboard.press('Enter');await settle();};
  const points=()=>page.evaluate(()=>{Module._paws_interaction_snapshot();return pawsInteraction;});
  const click=async name=>{const p=(await points()).objects[name];assert(p,'Missing interaction '+name);await page.mouse.click(p.x,p.y);await settle();};
  const step=async s=>{
   const before=await state();
   if(s.verb===1){const p=await points(),o=before.objects.find(o=>o.id===s.object),a=p.objects[s.object],from=p.slots[`${o.x},${o.y}`],to=p.slots[`${o.x+[0,1,0,-1][s.direction]},${o.y+[-1,0,1,0][s.direction]}`];assert(a&&from&&to,'Missing slide destination');await page.mouse.move(a.x,a.y);await page.mouse.down();await page.mouse.move(a.x+to.x-from.x,a.y+to.y-from.y,{steps:12});await page.mouse.up();await settle();}
   else if(s.verb===2)await click(s.object);
   else if(s.verb===3){await click(id==='pip_missing_ball'?'source_ball':before.objects.find(o=>o.role==='water_basin'&&o.active)?.id||before.objects.find(o=>o.id==='catch_tray'&&o.active)?.id||'rain_barrel');if(id==='pip_missing_ball')await page.waitForFunction(()=>pawsState.completed||!pawsBall.rolling,null,{timeout:90000});}
   else throw Error('Unexpected pilot verb '+s.verb);
   const after=await state();assert(after.revision>before.revision||after.completed,'Move rejected: '+JSON.stringify(s)+' '+after.message);assert.equal(after.questKernel.matched,1);
  };
  await page.goto(base+'?devBeach=1');await ready();assert.equal(await page.evaluate(()=>pawsSaveDiagnostics().requested),0,'Seed before any gameplay autosave');
  const original=await fs.readFile(`tests/fixtures/quest-migration-m0/${id}-partial.save`,'utf8');
  await page.evaluate(async text=>{const db=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon',maxBytes:16*1024*1024});await db.put('current',new TextEncoder().encode(text));db.dispose();},original);
  await page.reload();await ready();assert.equal(await page.evaluate(()=>pawsRenderState.backend),backend);run.build=await page.evaluate(()=>pawsBuild);
  await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.garden,null,{timeout:90000});
  assert.equal((await state()).quest,id);assert.equal((await state()).questKernel.authoritative,29);
  const coins=(await state()).coins;
  const restart=async()=>{await key('Menu');await key('Restart puzzle');await key('Restart puzzle');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.paused);};
  await restart();const solution=(await state()).solution;
  await step(solution[0]);await key('Menu');await key('Undo');assert((await state()).paused);await key('Continue');await restart();
  let checkpoint=false;
  for(const s of solution){await step(s);
   if(!checkpoint&&s.verb===3&&!(await state()).completed){checkpoint=true;assert.equal((await state()).questKernel.stages.beach_fresh_water,'puzzle');await saved();const objects=(await state()).objects;
    await page.screenshot({path:path.join(out,`${backend}-${id}-basin.png`)});await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2);assert.deepEqual((await state()).objects,objects);assert.equal((await state()).questKernel.stages.beach_fresh_water,'puzzle');
    await key('Menu');await key('Undo');assert.equal((await state()).questKernel.stages.beach_fresh_water,'basin');await key('Continue');await step(s);
   }
  }
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});await saved();
  assert.equal((await state()).coins,coins+(id==='pip_missing_ball'?20:0));assert(id!=='beach_fresh_water'||checkpoint);
  await page.screenshot({path:path.join(out,`${backend}-${id}-complete.png`)});
  // Completion offers Back to garden. Import a validated post-completion undo
  // snapshot from the portable/native test to exercise continued practice via
  // the public recovery flow without inventing a completion Menu control.
  await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});
  await key('Menu');await key('Save and recovery');const undoChooser=page.waitForEvent('filechooser');await key('Import save');
  await(await undoChooser).setFiles(`${process.env.PAWS_PILOT_UNDO_PREFIX||'build/evidence/m5/native.trace'}.${id}.undo.save`);
  await key('Use imported afternoon');await key('Continue');await page.waitForFunction(()=>pawsUi.screen===2&&!pawsState.completed,null,{timeout:90000});await restart();
  for(const s of solution)await step(s);
  await page.waitForFunction(()=>pawsState.completed&&pawsUi.screen===9,null,{timeout:90000});assert.equal((await state()).coins,coins+(id==='pip_missing_ball'?20:0));
  await key('Back to garden');await page.waitForFunction(()=>pawsState.garden&&pawsUi.screen===1,null,{timeout:90000});await saved();
  const backup=await page.evaluate(async()=>{const db=new EngineStorage.IndexedDBStorage({namespace:'paws-afternoon',maxBytes:16*1024*1024});try{return new TextDecoder().decode(await db.get('pre-quest-m5'));}finally{db.dispose();}});assert.equal(backup,original,'Exact original retained outside rotating history');
  await page.reload();await ready();await key('Continue');await page.waitForFunction(()=>pawsState.garden&&pawsState.production,null,{timeout:90000});
  const expected=await state();await key('Menu');await key('Save and recovery');const download=page.waitForEvent('download');await key('Export save');const file=path.join(out,`${backend}-${id}.save`);await(await download).saveAs(file);
  const bytes=await fs.readFile(file,'utf8');assert.match(bytes,/^PAWS_AFTERNOON 14/);assert.equal(bytes.split(`${id}/first-completion`).length-1,1);
  const bad=file+'.bad';await fs.writeFile(bad,bytes.replace(`${id}/first-completion`,`${id}/invalid-receipt`));const beforeRejectedImport=await state();const chooser=page.waitForEvent('filechooser');await key('Import save');await(await chooser).setFiles(bad);await settle();assert.equal((await state()).coins,expected.coins);assert.equal((await state()).revision,beforeRejectedImport.revision);assert.notEqual(await page.evaluate(()=>pawsUi.screen),18);
  assert.deepEqual(errors,[]);run.state=await state();run.backupPreserved=true;run.checkpointReload=checkpoint;run.errors=errors;report.runs.push(run);console.log('M5 browser passed',backend,id);await context.close();
 }}finally{await browser.close();await fs.writeFile(path.join(out,'report.json'),JSON.stringify(report,null,2));}
})().catch(e=>{console.error(e);process.exitCode=1;});
