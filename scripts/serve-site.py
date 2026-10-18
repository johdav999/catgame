"""Serve one consistent Paws build without browser cache or duplicate listeners."""
import argparse
import errno
import hashlib
import io
import socket
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit


def build_prefix(directory):
    digest = hashlib.sha256()
    for path in sorted(directory.rglob('*')):
        if path.is_file():
            info = path.stat()
            digest.update(f'{path.relative_to(directory).as_posix()}:{info.st_size}:{info.st_mtime_ns}\n'.encode())
    return '/__paws_build_' + digest.hexdigest()[:16] + '/'


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--directory', required=True, type=Path)
    parser.add_argument('--port', type=int, default=8080)
    parser.add_argument('--port-attempts', type=int, default=1)
    args = parser.parse_args()
    if not 1 <= args.port <= 65535 or not 1 <= args.port_attempts <= 100:
        parser.error('port must be 1..65535 and port-attempts must be 1..100')
    directory = args.directory.resolve(strict=True)
    prefix = build_prefix(directory)

    class Handler(SimpleHTTPRequestHandler):
        def __init__(self, *values, **options):
            super().__init__(*values, directory=str(directory), **options)

        def translate_path(self, path):
            if urlsplit(path).path.startswith(prefix):
                path = '/' + path[len(prefix):]
            return super().translate_path(path)

        def end_headers(self):
            self.send_header('Cache-Control', 'no-store')
            self.send_header('X-Paws-Build', prefix.strip('/'))
            super().end_headers()

        def send_head(self):
            path = Path(self.translate_path(self.path))
            if path.is_dir():
                path = path / 'index.html'
            if path.is_file() and path.suffix.lower() == '.html':
                # Relative scripts, Wasm and fetch URLs all resolve to this build.
                # The visible URL and same-origin save namespace stay stable.
                page = path.read_text(encoding='utf-8')
                page = page.replace('<head>', f'<head><base href="{prefix}">', 1)
                data = page.encode('utf-8')
                self.send_response(200)
                self.send_header('Content-Type', 'text/html; charset=utf-8')
                self.send_header('Content-Length', str(len(data)))
                self.end_headers()
                return io.BytesIO(data)
            return super().send_head()

    class Server(ThreadingHTTPServer):
        allow_reuse_address = False

        def server_bind(self):
            if hasattr(socket, 'SO_EXCLUSIVEADDRUSE'):
                self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
            super().server_bind()

    last_port = min(65535, args.port + args.port_attempts - 1)
    for port in range(args.port, last_port + 1):
        try:
            server = Server(('127.0.0.1', port), Handler)
            break
        except OSError as error:
            if error.errno not in (errno.EADDRINUSE, errno.EACCES) and getattr(error, 'winerror', None) not in (10048, 10013):
                raise
            if port == last_port:
                raise SystemExit(f'No available port in {args.port}..{last_port}. Choose another -Port. ({error})')
            print(f'Port {port} is unavailable; trying {port + 1}.', flush=True)
    print(f'Serving {directory} at http://127.0.0.1:{port}/ ({prefix})', flush=True)
    with server:
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == '__main__':
    main()
