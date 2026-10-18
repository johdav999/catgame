"""Historical-version fixtures derived from the checked-in actual v3 MVP save."""
import shlex
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
source=(ROOT/'tests/fixtures/phase7-completed.save').read_text().splitlines()
out=ROOT/'tests/release1/progression/fixtures';out.mkdir(parents=True,exist_ok=True)
def save(n,lines): (out/f'mvp-v{n}.save').write_text('\n'.join(lines)+'\n',encoding='utf-8')
save(3,source)
header=source[0].split();header[1]='2';header[10]='60'
save(2,[' '.join(header)]+source[2:])
header[1]='1';rows=[' '.join(header[:9])];at=2
while at<len(source):
    content,count,attempt,revision=shlex.split(source[at]);count=int(count)
    rows.append(f'"{content}" {count}');rows+=source[at+1:at+1+count];at+=count+1
save(1,rows)
header=source[0].split();header[1]='4'
save(4,[' '.join(header),'42 "migration-fixture" 1']+source[1:])
header[1]='5';save(5,[' '.join(header),'42 "migration-fixture" 1','2']+source[1:])
