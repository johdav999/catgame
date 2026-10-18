/* Reusable same-origin client. No provider secrets or authoritative purchases in the browser. */
(() => {
  'use strict';
  class GamePlatformClient {
    constructor({gameId,endpoint='/api/index.php'}) {
      this.gameId=gameId;this.endpoint=new URL(endpoint,location.href);
      if(this.endpoint.origin!==location.origin)throw Error('The account API must use the game origin.');
      this.csrf='';this.user=null;this.epoch=0;
    }
    async request(route,method='GET',data) {
      const url=new URL(this.endpoint);url.searchParams.set('route','/v1'+route);
      const response=await fetch(url,{method,credentials:'same-origin',cache:'no-store',
        headers:{'Content-Type':'application/json',...(method==='GET'?{}:{'X-CSRF-Token':this.csrf})},
        body:data===undefined?undefined:JSON.stringify(data),signal:AbortSignal.timeout(30000)});
      let result;
      try {result=await response.json();}catch {throw Error('Accounts are unavailable. Your local progress is safe.');}
      if(!response.ok){const error=Error(result.error||'Request failed.');error.status=response.status;error.code=result.code;throw error;}
      return result;
    }
    remember() {try{if(this.user)localStorage.setItem('game-platform-used','1');else localStorage.removeItem('game-platform-used');}catch{}}
    async session() {const epoch=++this.epoch,s=await this.request('/session');if(epoch===this.epoch){this.csrf=s.csrf;this.user=s.user;this.remember();}return {user:this.user,csrf:this.csrf};}
    async authenticate(action,email,password) {
      const epoch=++this.epoch,s=await this.request('/'+action,'POST',{email,password});if(epoch===this.epoch){this.csrf=s.csrf;this.user=s.user;this.remember();}return s;
    }
    async logout() {++this.epoch;const s=await this.request('/logout','POST',{});this.csrf=s.csrf;this.user=null;this.remember();}
    game(action,method='GET',data) {return this.request('/games/'+encodeURIComponent(this.gameId)+'/'+action,method,data);}
  }
  window.GamePlatformClient=GamePlatformClient;
})();
