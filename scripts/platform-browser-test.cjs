// Actual pinned generated site + actual PHP/MariaDB; no live Stripe calls.
const {chromium}=require('playwright');const assert=require('node:assert/strict');
const fs=require('node:fs/promises');const {randomUUID,createHmac,createHash}=require('node:crypto');
const {execFileSync}=require('node:child_process');
const fixture=(...args)=>execFileSync(process.env.PHP_EXE||'build/cloud-tools/php/php.exe',['-d','extension_dir=build/cloud-tools/php/ext','-d','extension=pdo_mysql','tests/platform-fixtures.php',...args],{stdio:'pipe'});
async function paid(purchase,session){const stamp=Math.floor(Date.now()/1000),body=JSON.stringify({id:'evt_'+randomUUID(),type:'checkout.session.completed',livemode:false,data:{object:{id:session,mode:'payment',client_reference_id:purchase,metadata:{purchase_id:purchase},amount_total:500,currency:'sek',payment_status:'paid',payment_intent:'pi_'+purchase}}});const signature=createHmac('sha256','fixture-webhook-secret').update(stamp+'.'+body).digest('hex');const r=await fetch(origin+'/api/index.php?route=/v1/stripe/webhook',{method:'POST',body,headers:{'Stripe-Signature':`t=${stamp},v1=${signature}`}});assert.equal(r.status,200);}
const origin='http://127.0.0.1:19337',url=origin+'/games/paws/?devBeach=0';
let browser;const errors=[];let checks=0;
async function ready(page){await page.goto(url);try{await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady&&window.pawsRenderState?.state==='ready',null,{timeout:90000});}catch(e){console.error(await page.evaluate(()=>({status:document.getElementById('status').textContent,render:window.pawsRenderState,storage:window.pawsStorageReady,gui:window.pawsGuiReady})),errors);throw e;}}
async function action(page,name){console.log('Action:',name);const button=page.getByRole('button',{name,exact:true});if(await button.getAttribute('data-widget-id')){await button.focus();await page.keyboard.press('Enter');}else await button.click();}
(async()=>{
  fixture('seed');
  browser=await chromium.launch({headless:true,executablePath:process.env.BROWSER_EXECUTABLE||'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  const context=await browser.newContext({viewport:{width:1280,height:800}});const page=await context.newPage();
  page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});await ready(page);
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  const email='browser-'+randomUUID()+'@example.com';await page.getByLabel('Email',{exact:true}).fill(email);
  await page.getByLabel('Password',{exact:true}).fill('Browser-fixture-password');await action(page,'Create account');
  await page.getByText('Signed in. Choose whether to save or load your afternoon.',{exact:true}).waitFor();checks++;
  await action(page,'Back to game');await action(page,'Play');await page.waitForFunction(()=>!!pawsState.garden,null,{timeout:90000});
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  await action(page,'Save to cloud');await page.getByText('Saved to cloud.',{exact:true}).waitFor();checks++;
  const first=await page.evaluate(async()=>{const api=new GamePlatformClient({gameId:'paws'});await api.session();return api.game('save');});
  assert.match(Buffer.from(first.save.payload,'base64').toString(),/^PAWS_AFTERNOON 11/);checks++;
  await action(page,'Back to game');
  await page.waitForFunction(()=>!!pawsState.garden,null,{timeout:90000});
  // A competing authenticated device changes the cloud revision.
  await page.evaluate(async()=>{const api=new GamePlatformClient({gameId:'paws'});await api.session();const s=(await api.game('save')).save;await api.game('save','PUT',{expectedRevision:s.revision,formatVersion:s.formatVersion,payload:s.payload});});
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  await action(page,'Save to cloud');await page.getByRole('button',{name:'Replace cloud save',exact:true}).waitFor();checks++;
  // Cloud bytes enter the portable import path; current local bytes are untouched until confirmation.
  await action(page,'Load from cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').open);
  await page.getByRole('button',{name:'Keep current afternoon',exact:true}).waitFor();checks++;
  await action(page,'Keep current afternoon');
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  await action(page,'Load from cloud');await action(page,'Use imported afternoon');checks++;
  await page.waitForFunction(()=>pawsState.save===2);
  await page.reload();await page.waitForFunction(()=>window.pawsStorageReady&&window.pawsGuiReady);checks++;
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  assert.equal(await page.locator('#cloud-identity').textContent(),email);checks++;
  const userId=await page.evaluate(async()=>{const api=new GamePlatformClient({gameId:'paws'});return (await api.session()).user.id;});
  const purchase=randomUUID().replaceAll('-',''),session='cs_'+randomUUID();fixture('purchase',purchase,userId,'paws',session);await paid(purchase,session);
  await action(page,'Refresh purchases');await page.locator('#cloud-products').getByText('Owned',{exact:false}).waitFor();checks++;
  assert(await page.evaluate(()=>pawsCloudEntitlements.includes('beach_afternoon')));checks++;
  // A separate browser/device has an independent local save and the same account ownership.
  const device=await browser.newContext();const other=await device.newPage();await ready(other);
  await action(other,'Account & cloud');await other.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  await other.getByLabel('Email',{exact:true}).fill(email);await other.getByLabel('Password',{exact:true}).fill('Browser-fixture-password');await action(other,'Sign in');
  await other.locator('#cloud-products').getByText('Owned',{exact:false}).waitFor();
  const remote=await other.evaluate(async()=>{const api=new GamePlatformClient({gameId:'paws'});await api.session();return api.game('save');});
  assert.equal(remote.save.payload,first.save.payload);checks++;await device.close();
  await action(page,'Back to game');await action(page,'Continue');await page.waitForFunction(()=>!!pawsState.garden&&pawsUi.screen===1);
  await action(page,'Adventures');await action(page,'Start / Continue beach');await page.waitForFunction(()=>pawsState.quest==='beach_pip_ball');checks++;
  await action(page,'Account & cloud');await page.waitForFunction(()=>!document.getElementById('cloud-panel').hasAttribute('aria-busy'));
  await action(page,'Sign out');await page.getByText('Signed out. Local progress remains on this browser.',{exact:true}).waitFor();checks++;
  assert(await page.evaluate(()=>pawsState.garden&&!pawsCloudEntitlements.length));checks++;
  assert(await page.evaluate(()=>pawsCloudSaveText().then(t=>t.startsWith('PAWS_AFTERNOON'))));checks++;
  await page.setViewportSize({width:390,height:844});
  assert(await page.locator('#cloud-panel').evaluate(d=>d.getBoundingClientRect().width<=innerWidth));checks++;
  await fs.mkdir('build/evidence/platform',{recursive:true});await page.screenshot({path:'build/evidence/platform/account-mobile.png'});
  assert.deepEqual(errors,[]);checks++;
  const report={checks,status:'passed',site:'build/web-pinned-release/site',browser:browser.version(),build:await page.evaluate(()=>pawsBuild),render:await page.evaluate(()=>pawsRenderState),errors,hashes:{}};
  for(const file of ['index.html','host.js','saves.js','game-platform.js','cloud.js','paws.wasm'])report.hashes[file]=createHash('sha256').update(await fs.readFile('build/web-pinned-release/site/'+file)).digest('hex');
  await fs.writeFile('build/evidence/platform/browser.json',JSON.stringify(report,null,2));console.log(JSON.stringify(report));
})().catch(e=>{console.error(e);process.exitCode=1;}).finally(async()=>browser?.close());
