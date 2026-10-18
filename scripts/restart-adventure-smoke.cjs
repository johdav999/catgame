// Isolated browser profile: never modifies the user's campaign.
const {chromium}=require('playwright');
const fs=require('node:fs/promises'),path=require('node:path'),assert=require('node:assert/strict');
(async()=>{
 const out=process.env.EVIDENCE_DIR||'build/evidence/restart-adventure';await fs.mkdir(out,{recursive:true});
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const page=await browser.newPage({viewport:{width:1280,height:900}});const errors=[];page.on('pageerror',e=>errors.push(e.message));
 async function action(name){for(let n=0;n<200;n++){if(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label'))===name){await page.keyboard.press('Enter');await page.waitForTimeout(250);return;}await page.keyboard.press('Tab');}throw Error('Unreachable '+name);}
 async function saved(){await page.waitForFunction(()=>pawsState.save===2);}
 try{
  await page.goto(process.env.PAWS_URL||'http://localhost:8080/');await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady);
  await action('Save and recovery');const chooser=page.waitForEvent('filechooser');await action('Import save');await (await chooser).setFiles(path.resolve('tests/fixtures/phase7-completed.save'));
  await action('Use imported afternoon');await action('Play');await page.waitForFunction(()=>pawsState.garden);await saved();
  const previous=await page.evaluate(()=>pawsState);assert(previous.coins>0);
  await action('Menu');await action('Settings');if(await page.getByRole('button',{name:'Motion: Full',exact:true}).count())await action('Motion: Full');await action('Back');
  await page.screenshot({path:path.join(out,'menu.png')});
  await action('New Game');await page.screenshot({path:path.join(out,'confirmation.png')});
  assert.equal(await page.evaluate(()=>document.activeElement?.getAttribute('aria-label')),'Keep playing');
  await action('Keep playing');assert.equal((await page.evaluate(()=>pawsState)).coins,previous.coins);
  await action('New Game');await action('New Game');await page.waitForFunction(()=>pawsState.garden&&pawsState.coins===0);await saved();
  await page.screenshot({path:path.join(out,'fresh-garden.png')});
  await page.reload();await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady);await action('Play');await page.waitForFunction(()=>pawsState.garden);await saved();
  assert.equal((await page.evaluate(()=>pawsState)).coins,0);
  await action('Menu');await action('Settings');assert(await page.getByRole('button',{name:'Motion: Reduced',exact:true}).count());await action('Back');
  await page.setViewportSize({width:390,height:844});await action('New Game');await page.screenshot({path:path.join(out,'mobile-confirmation.png')});await action('Keep playing');
  assert.deepEqual(errors,[]);await fs.writeFile(path.join(out,'report.json'),JSON.stringify({passed:true,checks:['cancel preserves progress','confirm resets completed campaign','fresh save survives reload','preferences retained','narrow-screen confirmation reachable'],build:await page.evaluate(()=>pawsBuild)},null,2));
  console.log('Restart adventure browser acceptance passed');
 }catch(e){await page.screenshot({path:path.join(out,'failure.png')});throw e;}finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});

