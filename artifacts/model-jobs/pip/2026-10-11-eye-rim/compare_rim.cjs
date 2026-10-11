const fs=require('fs'), {PNG}=require('pngjs');
const load=p=>PNG.sync.read(fs.readFileSync(p));
const rows=[];
for(let v=0;v<3;v++){
 const bare=load(`${__dirname}/no-margin-browser/view-${v}-open.png`);
 const old=load(`${__dirname}/../2026-10-11-blink-production/browser/view-${v}-open.png`);
 const now=load(`${__dirname}/browser/view-${v}-open.png`);
 let previous=0,current=0;
 for(let i=0;i<bare.data.length;i+=4){
  const light=p=>(p.data[i]+p.data[i+1]+p.data[i+2])/3;
  if(light(bare)-light(old)>12)previous++;
  if(light(bare)-light(now)>12)current++;
 }
 rows.push({view:v,previous_dark_pixels:previous,current_dark_pixels:current,reduction:1-current/previous});
}
fs.writeFileSync(__dirname+'/rim-comparison.json',JSON.stringify(rows,null,2));console.log(rows);
if(rows.some(x=>x.reduction<.90))throw Error('Residual open-eye rim exceeds visual regression threshold');
