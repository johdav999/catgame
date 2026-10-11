// One entry point runs gameplay, recovery and visual/soak review on the same package.
const {spawnSync}=require('node:child_process');
const path=require('node:path');
const site=path.resolve(process.env.SITE_DIR||'build/web-phase10-pinned-release/site');
const evidence=path.resolve(process.env.EVIDENCE_DIR||'build/evidence/phase10');
for(const [script,folder,port]of[['phase7-smoke.cjs','journey',19520],['phase8-smoke.cjs','recovery',19522],['phase10-review.cjs','review',19521]]){
  const result=spawnSync(process.execPath,[path.join(__dirname,script)],{stdio:'inherit',env:{...process.env,SITE_DIR:site,EVIDENCE_DIR:path.join(evidence,folder),PAWS_TEST_PORT:String(port),PAWS_UAT:'1',PAWS_PLAYED_SAVE:path.join(evidence,'journey/played-afternoon.save')}});
  if(result.status!==0){process.exitCode=result.status||1;break;}
}
