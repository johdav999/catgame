(() => {
  const token=location.hash.slice(1);history.replaceState(null,'',location.pathname);
  const form=document.querySelector('form'),status=document.getElementById('status');
  form.onsubmit=async event=>{
    event.preventDefault();form.querySelector('button').disabled=true;
    try {
      const response=await fetch('/api/index.php?route=/v1/session',{credentials:'same-origin',cache:'no-store'});
      if(!response.ok)throw Error('Account service is unavailable.');
      const session=await response.json();
      const reset=await fetch('/api/index.php?route=/v1/password/reset',{method:'POST',credentials:'same-origin',headers:{'Content-Type':'application/json','X-CSRF-Token':session.csrf},body:JSON.stringify({token,password:form.elements.password.value})});
      const result=await reset.json();if(!reset.ok)throw Error(result.error||'Password could not be changed.');
      form.hidden=true;form.reset();status.textContent=result.message;
    } catch(error){status.textContent=error.message;form.querySelector('button').disabled=false;}
  };
})();
