"""One-time font production. Runtime builds only consume the checked-in static TTFs.
Run with fonttools==4.60.1 on PYTHONPATH. Raw sources stay outside assets/.
"""
import hashlib
import json
from pathlib import Path
from urllib.request import urlopen
from urllib.parse import quote
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

ROOT = Path(__file__).resolve().parents[1]
RAW = ROOT / 'artifacts/gui/fonts'
OUT = ROOT / 'assets/runtime/fonts'
RAW.mkdir(parents=True, exist_ok=True)
OUT.mkdir(parents=True, exist_ok=True)
commit = json.load(urlopen('https://api.github.com/repos/google/fonts/commits/main'))['sha']
records = []
for family, filename, weights in [
    ('fraunces', 'Fraunces[SOFT,WONK,opsz,wght].ttf', [600]),
    ('nunitosans', 'NunitoSans[YTLC,opsz,wdth,wght].ttf', [400, 600]),
]:
    base = f'https://raw.githubusercontent.com/google/fonts/{commit}/ofl/{family}/'
    source = urlopen(base + quote(filename)).read()
    (RAW / filename).write_bytes(source)
    license_bytes = urlopen(base + 'OFL.txt').read()
    (OUT / f'{family}-OFL.txt').write_bytes(license_bytes)
    for weight in weights:
        font = TTFont(RAW / filename)
        axes = {a.axisTag: a.defaultValue for a in font['fvar'].axes}
        axes.update(wght=weight, opsz=24 if family == 'fraunces' else 12)
        font = instantiateVariableFont(font, axes, inplace=True)
        name = ('Fraunces' if family == 'fraunces' else 'NunitoSans') + ('-Semibold' if weight == 600 else '-Regular') + '.ttf'
        font.save(OUT / name)
        assert all(ord(c) in font.getBestCmap() for c in 'ÅÄÖåäö')
        records.append(dict(id=name[:-4], file=name, source=base+quote(filename),
            source_sha256=hashlib.sha256(source).hexdigest(), axes=axes,
            sha256=hashlib.sha256((OUT/name).read_bytes()).hexdigest(),
            bytes=(OUT/name).stat().st_size, license=f'{family}-OFL.txt',
            modification='Static instance using fontTools 4.60.1; no subsetting', date='2026-10-10'))
(OUT / 'manifest.json').write_text(json.dumps(records, indent=2)+'\n')

