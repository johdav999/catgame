"""Contact sheets and timestamp-preserving review videos from captured renders."""
import json,subprocess
from pathlib import Path
from PIL import Image,ImageDraw,ImageOps
JOB=Path(__file__).resolve().parents[1]
for directory,prefix,views,size in [('renders','clean',['side','front','rear','three-quarter','puzzle'],(240,240)),('browser-evidence','cycle',['side','front','rear','three-quarter','puzzle'],(320,170))]:
 p=JOB/directory;w,h=size;sheet=Image.new('RGB',(w*6,(h+24)*len(views)),'#fff8e9');draw=ImageDraw.Draw(sheet)
 for row,view in enumerate(views):
  for col,frame in enumerate(range(0,36,6)):
   im=Image.open(p/f'{prefix}-{view}-{frame:02}.png');im=ImageOps.contain(im,size)
   sheet.paste(im,(col*w+(w-im.width)//2,row*(h+24)+(h-im.height)//2));draw.text((col*w+8,row*(h+24)+h+3),f'{view} / {frame/30:.1f}s',fill='#253e37')
 sheet.save(p/'review-sheet.jpg',quality=90)
for folder,report,key,prefix,out in [('browser-evidence','walk-report.json','laneSamples','lane','browser-walk.mp4'),('game-evidence-final','garden-walk-samples.json',None,'garden-walk','garden-walk.mp4')]:
 p=JOB/folder
 if not (p/report).exists():continue
 samples=json.loads((p/report).read_text());samples=samples[key] if key else samples
 lines=[]
 for i,sample in enumerate(samples):
  duration=(samples[i+1]['wall']-sample['wall'])/1000 if i+1<len(samples) else .1
  lines.extend([f"file '{prefix}-{sample['frame']:03}.png'",f'duration {duration:.6f}'])
 concat=p/(prefix+'-concat.txt');concat.write_text('\n'.join(lines)+'\n')
 subprocess.run(['ffmpeg','-y','-loglevel','error','-f','concat','-safe','0','-i',str(concat),'-vf','fps=30','-c:v','libx264','-crf','18','-pix_fmt','yuv420p',str(JOB/out)],check=True)
