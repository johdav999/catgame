/* Paws account panel. Local progress stays independent; cloud replacement uses
   the existing Afternoon import validation and explicit confirmation screen. */
(() => {
  'use strict';
  const api=new GamePlatformClient({gameId:'paws',endpoint:document.querySelector('meta[name="game-api"]').content});
  const dialog=document.getElementById('cloud-panel'),open=document.getElementById('cloud-open');
  const status=document.getElementById('cloud-status'),auth=document.getElementById('cloud-auth');
  const account=document.getElementById('cloud-account'),products=document.getElementById('cloud-products');
  let busy=false,cloudRevision=null,purchaseKeys=new Map();
  const message=text=>{status.textContent=text;};
  function render() {
    auth.hidden=!!api.user;account.hidden=!api.user;
    document.getElementById('cloud-identity').textContent=api.user?.email||'';
  }
  async function task(work) {
    if(busy)return;busy=true;dialog.setAttribute('aria-busy','true');
    for(const b of dialog.querySelectorAll('button'))b.disabled=true;
    try {await work();}catch(error){
      if(error.status===401){api.user=null;cloudRevision=null;products.replaceChildren();await window.pawsSetCloudEntitlements?.([]);}
      message(error.message||'Connection lost. Your local progress is safe.');
    }finally{busy=false;dialog.removeAttribute('aria-busy');for(const b of dialog.querySelectorAll('button'))b.disabled=false;render();}
  }
  async function refresh() {
    const userId=api.user?.id,epoch=api.epoch;
    const [catalog,owned,history]=await Promise.all([api.game('products'),api.game('entitlements'),api.game('purchases')]);
    if(api.user?.id!==userId||api.epoch!==epoch)return;
    window.pawsCloudEntitlements=Object.freeze([...owned.entitlements]);
    window.pawsSetCloudEntitlements?.(owned.entitlements);
    products.replaceChildren();
    for(const product of catalog.products) {
      const row=document.createElement('p'),button=document.createElement('button');
      const money=new Intl.NumberFormat(undefined,{style:'currency',currency:product.currency});
      const price=money.format(Number(product.amount)/10**money.resolvedOptions().maximumFractionDigits);
      row.append(document.createTextNode(product.title+' · '+price+' '));
      if(owned.entitlements.includes(product.id)){row.append(document.createTextNode('Owned'));}
      else {button.textContent='Buy '+product.title;button.onclick=()=>task(async()=>{
        const key=purchaseKeys.get(product.id)||crypto.randomUUID();purchaseKeys.set(product.id,key);
        // Keep the same key across timeouts/reloads, scoped to this customer and game.
        const storageKey='platform-checkout:'+api.user.id+':paws:'+product.id;
        let requestKey=key;try{requestKey=sessionStorage.getItem(storageKey)||key;sessionStorage.setItem(storageKey,requestKey);}catch{}
        let checkout;
        try{checkout=await api.game('checkout','POST',{productId:product.id,requestKey});}
        catch(error){if(error.code==='checkout_closed'){purchaseKeys.delete(product.id);try{sessionStorage.removeItem(storageKey);}catch{}}throw error;}
        const url=new URL(checkout.url);if(url.origin!=='https://checkout.stripe.com')throw Error('Unexpected checkout address.');
        location.assign(url.href);
      });row.append(button);}
      products.append(row);
    }
    const list=document.getElementById('cloud-history');list.replaceChildren();
    for(const purchase of history.purchases) {
      const item=document.createElement('li');item.textContent=purchase.product_id+' · '+purchase.status+(Number(purchase.disputed)?' · under review':'');list.append(item);
    }
  }
  open.onclick=()=>{
    window.pawsAccountModal?.(true);dialog.showModal();message('Connecting…');
    task(async()=>{await api.session();cloudRevision=null;render();if(api.user)await refresh();message(api.user?'Cloud saves are manual. Local progress stays on this browser.':'Sign in to use cloud saves and purchases across our games.');});
  };
  function close(){dialog.close();window.pawsAccountModal?.(false);open.focus();}
  document.getElementById('cloud-close').onclick=close;
  dialog.addEventListener('cancel',event=>{event.preventDefault();if(!busy)close();});
  dialog.addEventListener('keydown',event=>event.stopPropagation());
  auth.onsubmit=event=>{event.preventDefault();task(async()=>{
    await api.authenticate(event.submitter.value,auth.elements.email.value,auth.elements.password.value);
    auth.elements.password.value='';cloudRevision=null;purchaseKeys.clear();await refresh();message('Signed in. Choose whether to save or load your afternoon.');
  });};
  document.getElementById('cloud-reset-password').onclick=()=>task(async()=>{
    const result=await api.request('/password/request','POST',{email:auth.elements.email.value});message(result.message);
  });
  document.getElementById('cloud-logout').onclick=()=>task(async()=>{
    await api.logout();cloudRevision=null;purchaseKeys.clear();products.replaceChildren();window.pawsCloudEntitlements=Object.freeze([]);
    window.pawsSetCloudEntitlements?.([]);message('Signed out. Local progress remains on this browser.');
  });
  document.getElementById('cloud-upload').onclick=()=>task(async()=>{
    if(cloudRevision===null){
      const remote=await api.game('save');
      if(remote.save){message('A cloud save already exists. Load it first, or choose Replace cloud save to upload this browser’s progress.');document.getElementById('cloud-replace').hidden=false;return;}
      cloudRevision=0;
    }
    await upload();
  });
  async function upload() {
    const text=await window.pawsCloudSaveText();
    const bytes=new TextEncoder().encode(text);let binary='';for(let i=0;i<bytes.length;i+=8192)binary+=String.fromCharCode(...bytes.subarray(i,i+8192));
    const formatVersion=Number(/^PAWS_AFTERNOON (\d+)/.exec(text)?.[1]);
    const result=await api.game('save','PUT',{expectedRevision:cloudRevision,formatVersion,payload:btoa(binary)});
    cloudRevision=result.revision;document.getElementById('cloud-replace').hidden=true;message('Saved to cloud.');
  }
  document.getElementById('cloud-replace').onclick=()=>task(async()=>{
    const remote=await api.game('save');
    if(!confirm('Replace the cloud afternoon with progress from this browser?'))return;
    cloudRevision=remote.save?.revision||0;await upload();
  });
  document.getElementById('cloud-download').onclick=()=>task(async()=>{
    // Capture the game revision before awaiting the network. Stale completions cannot stage an import.
    const stamp=window.pawsCloudSaveStamp();const remote=await api.game('save');
    if(!remote.save){message('There is no cloud save for Paws yet.');return;}
    const binary=atob(remote.save.payload);if(binary.length>16*1024*1024)throw Error('Cloud save is too large.');
    const bytes=Uint8Array.from(binary,c=>c.charCodeAt(0));
    if(!window.pawsStageCloudSave(bytes,stamp))throw Error('Cloud save could not be loaded. Your local afternoon is unchanged.');
    // Do not treat a staged/cancelled import as an adopted cloud branch.
    cloudRevision=null;close();
  });
  document.getElementById('cloud-refresh').onclick=()=>task(async()=>{await refresh();message('Purchases refreshed. Pending payments may take a moment to arrive.');});
  // Entitlements are transient server results, never restored from a game save.
  window.pawsRefreshCloudAccess=async()=>{
    try{if(localStorage.getItem('game-platform-used')!=='1')return;}catch{return;}
    try{await api.session();if(api.user)await refresh();else window.pawsSetCloudEntitlements?.([]);}catch{window.pawsSetCloudEntitlements?.([]);}
  };
})();
