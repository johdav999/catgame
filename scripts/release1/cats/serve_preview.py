"""Serve only the generated cat workshop on loopback."""
import argparse,http.server
from functools import partial
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
SITE=ROOT/'build/web-release1-cats/site'
for file in ['index.html','release1_preview.js','release1_preview.wasm']:
    assert (SITE/file).is_file(),f'Build the cat workshop first: missing {file}'
class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map={**http.server.SimpleHTTPRequestHandler.extensions_map,'.wasm':'application/wasm','.glb':'model/gltf-binary'}
    def end_headers(self):
        self.send_header('Cache-Control','no-store');super().end_headers()
p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=19612);args=p.parse_args()
print(f'Cat workshop: http://127.0.0.1:{args.port}/?cat=button\nSite: {SITE}',flush=True)
http.server.ThreadingHTTPServer(('127.0.0.1',args.port),partial(Handler,directory=str(SITE))).serve_forever()
