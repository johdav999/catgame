// Each runner owns its server and isolated browser contexts; no user saves.
const {spawn}=require('node:child_process');
const http=require('node:http');
const fs=require('node:fs/promises');
const path=require('node:path');
const site=path.resolve(process.env.SITE_DIR||'build/web-release1-p01-pinned-release/site');
const evidence=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/release1/prompt-01');
const basePort=Number(process.env.PAWS_TEST_PORT||19601);
function run(script,env){return new Promise((resolve,reject)=>{
  const child=spawn(process.execPath,[path.resolve('scripts',script)],{stdio:'inherit',env:{...process.env,...env}});
  child.once('error',reject);child.once('exit',code=>code===0?resolve():reject(Error(script+' exited '+code)));
});}
async function direct(){
  const port=basePort+2;
  const server=http.createServer(async(req,res)=>{
    try{
      const url=new URL(req.url,'http://localhost');
      const file=path.resolve(site,'.'+decodeURIComponent(url.pathname==='/'?'/index.html':url.pathname));
      if(!file.startsWith(site+path.sep))throw Error('Outside site');
      const bytes=await fs.readFile(file);
      res.setHeader('Content-Type',({'.wasm':'application/wasm','.js':'text/javascript','.html':'text/html','.css':'text/css','.svg':'image/svg+xml'})[path.extname(file)]||'application/octet-stream');
      res.end(bytes);
    }catch{res.statusCode=404;res.end('Missing file');}
  });
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',resolve);});
  try{await run('direct-controls-smoke.cjs',{PAWS_URL:`http://127.0.0.1:${port}`,EVIDENCE_DIR:path.join(evidence,'direct')});}
  finally{await new Promise(resolve=>server.close(resolve));}
}
(async()=>{
  if(process.env.PAWS_DIRECT_ONLY==='1'){await direct();return;}
  await run('phase7-smoke.cjs',{SITE_DIR:site,EVIDENCE_DIR:path.join(evidence,'journey'),PAWS_TEST_PORT:String(basePort),PAWS_UAT:'1'});
  await run('phase8-smoke.cjs',{SITE_DIR:site,EVIDENCE_DIR:path.join(evidence,'recovery'),PAWS_TEST_PORT:String(basePort+1)});
  await direct();
})().catch(error=>{console.error(error);process.exitCode=1;});
