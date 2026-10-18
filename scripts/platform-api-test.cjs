// Requires an isolated MariaDB cloud_test on 127.0.0.1:19336 and PHP API :19337.
// Uses fixture signatures only. Does not call Stripe or send email.
const assert=require('node:assert/strict');
const {createHmac,randomUUID}=require('node:crypto');
const {execFileSync}=require('node:child_process');
const fs=require('node:fs/promises');
const php=process.env.PHP_EXE||'build/cloud-tools/php/php.exe';
const fixture=(...args)=>execFileSync(php,['-d','extension_dir=build/cloud-tools/php/ext','-d','extension=pdo_mysql','tests/platform-fixtures.php',...args],{stdio:'pipe'});
const origin='http://127.0.0.1:19337';let checks=0;
class Client {
  cookie='';csrf='';
  endpoint='/api/index.php';
  async call(route,method='GET',body,headers={}) {
    const response=await fetch(origin+this.endpoint+'?route='+encodeURIComponent('/v1'+route),{
      method,headers:{Cookie:this.cookie,Origin:origin,'Content-Type':'application/json','X-CSRF-Token':this.csrf,...headers},
      body:body===undefined?undefined:JSON.stringify(body)});
    const cookie=response.headers.getSetCookie()[0];if(cookie)this.cookie=cookie.split(';')[0];
    const data=await response.json();if(data.csrf)this.csrf=data.csrf;
    return {status:response.status,...data};
  }
}
async function signed(type,object,{id='evt_'+randomUUID(),mode=false,stamp=Math.floor(Date.now()/1000),secret='fixture-webhook-secret'}={}) {
  const body=JSON.stringify({id,type,livemode:mode,data:{object}});
  const sig=createHmac('sha256',secret).update(stamp+'.'+body).digest('hex');
  const r=await fetch(origin+'/api/index.php?route=/v1/stripe/webhook',{method:'POST',headers:{'Stripe-Signature':`t=${stamp},v1=${sig}`},body});
  return {status:r.status,...await r.json()};
}
function check(actual,expected,label){assert.deepEqual(actual,expected,label);checks++;}
(async()=>{
  fixture('seed');const a=new Client(),b=new Client(),fresh=new Client();
  check((await a.call('/games/paws/save')).status,401,'anonymous reads rejected');
  await a.call('/session');await b.call('/session');
  const email='fixture-'+randomUUID()+'@example.com',password='Fixture-password-123';
  const user=await a.call('/register','POST',{email,password});check(user.status,201,'registration');
  check((await b.call('/login','POST',{email,password:'wrong-password'})).status,401,'wrong password');
  check((await b.call('/register','POST',{email:'second-'+randomUUID()+'@example.com',password})).status,201,'second customer');
  check((await a.call('/logout','POST',{}, {'X-CSRF-Token':'wrong'})).status,403,'CSRF');
  check((await a.call('/session','GET',undefined,{Origin:'https://evil.example'})).status,403,'cross origin');
  const save={expectedRevision:0,formatVersion:11,payload:Buffer.from('isolated opaque game bytes').toString('base64')};
  check((await a.call('/games/paws/save','PUT',save)).revision,1,'first save');
  check((await a.call('/games/paws/save','PUT',save)).revision,1,'lost response retry');
  check((await b.call('/games/paws/save')).save,null,'customer isolation');
  check((await a.call('/games/second-game/save')).save,null,'game isolation');
  check((await a.call('/games/paws/save','PUT',{...save,payload:Buffer.from('competing bytes').toString('base64')})).status,409,'stale write rejected');
  check((await a.call('/games/paws/save')).save.payload,save.payload,'conflict preserves bytes');
  check((await a.call('/games/second-game/save','PUT',save)).revision,1,'second game first revision');
  check((await a.call('/games/paws/save','PUT',{...save,expectedRevision:1,payload:'not base64!'})).status,400,'malformed save');
  check((await a.call('/games/not-registered/save')).status,404,'unregistered game');
  check((await a.call('/games/paws/checkout','POST',{productId:'beach_afternoon',requestKey:randomUUID()})).status,503,'checkout disabled without key');
  const gateway=new Client();gateway.endpoint='/fixture-api.php';await gateway.call('/session');
  await gateway.call('/register','POST',{email:'gateway-'+randomUUID()+'@example.com',password});
  const key=randomUUID(),checkout=await gateway.call('/games/paws/checkout','POST',{productId:'beach_afternoon',requestKey:key,amount:1,returnUrl:'https://evil.example'});
  check(checkout.status,200,'checkout creates with server-owned price and return URL');
  check((await gateway.call('/games/paws/checkout','POST',{productId:'beach_afternoon',requestKey:key})).purchaseId,checkout.purchaseId,'checkout retry reuses order');
  check((await gateway.call('/games/paws/checkout','POST',{productId:'beach_afternoon',requestKey:randomUUID()})).purchaseId,checkout.purchaseId,'new key reuses unresolved purchase');
  check((await gateway.call('/games/paws/purchases')).purchases.length,1,'checkout retry creates no duplicate purchase');
  check((await gateway.call('/games/paws/entitlements')).entitlements,[],'checkout URL grants nothing');
  check((await gateway.call('/games/second-game/checkout','POST',{productId:'beach_afternoon',requestKey:key})).status,409,'idempotency key cannot switch game');
  const id=randomUUID().replaceAll('-',''),session='cs_fixture_'+randomUUID();fixture('purchase',id,user.user.id,'paws',session);
  const obj={id:session,mode:'payment',client_reference_id:id,metadata:{purchase_id:id},amount_total:500,currency:'sek',payment_status:'paid',payment_intent:'pi_'+id};
  check((await a.call('/games/paws/entitlements')).entitlements,[],'pending grants nothing');
  check((await signed('checkout.session.completed',obj,{secret:'wrong'})).status,400,'forged signature');
  check((await signed('checkout.session.completed',obj,{stamp:1})).status,400,'expired signature');
  check((await signed('checkout.session.completed',obj,{mode:true})).status,400,'live/test isolation');
  check((await signed('checkout.session.completed',{...obj,amount_total:1})).status,400,'price mismatch');
  check((await signed('checkout.session.completed',{...obj,payment_status:'unpaid'})).status,200,'delayed payment acknowledged');
  check((await a.call('/games/paws/entitlements')).entitlements,[],'unpaid grants nothing');
  const eventId='evt_'+randomUUID();check((await signed('checkout.session.async_payment_succeeded',obj,{id:eventId})).status,200,'paid callback');
  check((await signed('checkout.session.async_payment_succeeded',obj,{id:eventId})).status,200,'duplicate callback');
  check((await a.call('/games/paws/entitlements')).entitlements,['beach_afternoon'],'paid unlock');
  check((await b.call('/games/paws/entitlements')).entitlements,[],'purchase customer isolation');
  check((await a.call('/games/second-game/entitlements')).entitlements,[],'purchase game isolation');
  check((await a.call('/games/paws/purchases')).purchases.length,1,'single purchase');
  check((await a.call('/purchases')).purchases[0].game_id,'paws','cross-game account history');
  await signed('charge.refunded',{payment_intent:obj.payment_intent,amount:500,amount_refunded:100,currency:'sek'});
  check((await a.call('/games/paws/entitlements')).entitlements,['beach_afternoon'],'partial refund retains unlock');
  check((await signed('charge.refunded',{payment_intent:obj.payment_intent,amount:500,amount_refunded:500,currency:'sek'})).status,200,'refund');
  check((await a.call('/games/paws/entitlements')).entitlements,[],'refund revokes unlock');
  await signed('checkout.session.completed',obj);check((await a.call('/games/paws/entitlements')).entitlements,[],'late paid event cannot undo refund');
  await signed('charge.refunded',{payment_intent:obj.payment_intent,amount:500,amount_refunded:100,currency:'sek'});
  check((await a.call('/games/paws/entitlements')).entitlements,[],'late partial refund cannot undo full refund');
  const id2=randomUUID().replaceAll('-',''),session2='cs_fixture_'+randomUUID();fixture('purchase',id2,user.user.id,'paws',session2);
  const obj2={...obj,id:session2,client_reference_id:id2,metadata:{purchase_id:id2},payment_intent:'pi_'+id2};
  await signed('checkout.session.completed',obj2);await signed('charge.dispute.created',{payment_intent:obj2.payment_intent,status:'needs_response'});
  check((await a.call('/games/paws/entitlements')).entitlements,[],'dispute suspends unlock');
  await signed('charge.dispute.closed',{payment_intent:obj2.payment_intent,status:'won'});
  check((await a.call('/games/paws/entitlements')).entitlements,[],'dispute restoration needs operator review');
  check((await a.call('/logout','POST',{})).status,200,'logout');check((await a.call('/games/paws/save')).status,401,'session revoked');
  await fresh.call('/session');check((await fresh.call('/login','POST',{email,password})).status,200,'sign in new session');
  check((await fresh.call('/games/paws/save')).save.payload,save.payload,'cloud save survives sign out');
  const token=randomUUID().replaceAll('-','')+randomUUID().replaceAll('-','');fixture('reset',user.user.id,token);
  const resetter=new Client();await resetter.call('/session');
  check((await resetter.call('/password/reset','POST',{token,password:'New-fixture-password'})).status,200,'password reset');
  check((await resetter.call('/password/reset','POST',{token,password:'New-fixture-password'})).status,400,'reset single use');
  check((await fresh.call('/games/paws/save')).status,401,'reset revokes old sessions');
  check((await resetter.call('/login','POST',{email,password:'New-fixture-password'})).status,200,'new password works');
  const report={checks,status:'passed',provider:'signed local fixtures and injected checkout transport; no live Stripe calls'};
  await fs.mkdir('build/evidence/platform',{recursive:true});await fs.writeFile('build/evidence/platform/api.json',JSON.stringify(report,null,2));
  console.log(JSON.stringify(report));
})().catch(e=>{console.error(e);process.exitCode=1;});
