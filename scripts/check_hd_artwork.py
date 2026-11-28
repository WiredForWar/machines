"""Verify the optional HD interface pack using only Python's standard library.

This checks bytes and image headers, not rendered appearance or GPU behavior.
For native SDL decoding, use tools/hd-hud-check against each pack.
"""
import argparse
import hashlib
import html
import json
from pathlib import Path
import struct
import re

ROOT = Path(__file__).resolve().parents[1]
PACKS = {'faithful-hd-hud': 'hd-hud', 'faithful-hd-menus': 'hd-menus',
         'faithful-hd-intros': 'hd-intros', 'faithful-hd-interface-art': 'hd-interface'}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dimensions(path):
    with path.open('rb') as stream:
        header = stream.read(32)
    if header[:8] == b'\x89PNG\r\n\x1a\n' and header[12:16] == b'IHDR':
        return list(struct.unpack('>II', header[16:24]))
    if header[:2] == b'BM':
        w, h = struct.unpack('<ii', header[18:26])
        return [w, abs(h)]
    raise ValueError(f'Unsupported image: {path}')


def validate():
    rows, paths = [], set()
    for name, category in PACKS.items():
        pack = ROOT / 'mods' / name
        manifest = json.loads((pack / 'manifest.json').read_text(encoding='utf-8'))
        if manifest.get('remaining_atlases'):
            raise ValueError(f'Unfinished masters in {name}')
        textures = manifest['textures']
        native = []
        for entry in textures:
            relative = entry['path']
            if relative in paths:
                raise ValueError(f'Conflicting mod path: {relative}')
            paths.add(relative)
            path = pack / relative
            if not path.resolve().is_relative_to(pack.resolve()):
                raise ValueError(f'Escaping texture path: {relative}')
            if sha(path) != entry['sha256']:
                raise ValueError(f'Image hash mismatch: {path}')
            size = dimensions(path)
            if size != entry['size'] or min(size) <= 0 or max(size) > 8192:
                raise ValueError(f'Image dimensions mismatch: {path}')
            native.append(f'{relative}\t{size[0]}\t{size[1]}\n')
        expected = ''.join(native)
        actual = (pack / 'native-check.tsv').read_text()
        if expected != actual:
            raise ValueError(f'Native decoder list differs from manifest: {name}')
        for name_, master in manifest['masters'].items():
            relative = master.get('path', f'art/{category}/masters/{name_}.png')
            path = ROOT / relative
            if sha(path) != master['sha256'] or dimensions(path) != master['size']:
                raise ValueError(f'Master differs from manifest: {path}')
        rows.append({'pack': name, 'images': len(textures), 'masters': len(manifest['masters']),
                     'compressed_bytes': sum((pack / t['path']).stat().st_size for t in textures),
                     'rgba_bytes_all_variants': sum(t['size'][0] * t['size'][1] * 4 for t in textures)})
    return {'packs': rows, 'total_images': len(paths), 'total_masters': sum(r['masters'] for r in rows),
            'total_compressed_bytes': sum(r['compressed_bytes'] for r in rows),
            'scope': 'Hash/header/source-master checks only; interactive review remains required.'}


def gallery():
    cards = []
    for name in PACKS:
        report = json.loads((ROOT / 'mods' / name / 'manifest.json').read_text())
        families = {}
        for entry in report['textures']:
            if not entry['path'].endswith('.png'):
                continue
            family = re.sub(r'_[2-6]x(?=\.png$)', '', entry['path'])
            suffix = re.search(r'_([2-6])x\.png$', entry['path'])
            scale = entry.get('scale', int(suffix.group(1)) if suffix else 1)
            rank = -abs(scale - 4)
            if family not in families or rank > families[family][0]:
                families[family] = (rank, entry)
        for _, entry in families.values():
            path = entry['path']
            url = '../mods/' + name + '/' + path
            label = html.escape(name + '/' + path)
            w, h = entry['size']
            cards.append(f'<article data-name="{label.lower()}"><a href="{url}"><img loading="lazy" src="{url}" alt="{label}"></a><p>{label}</p><small>{w} × {h} exported pixels</small></article>')
    page = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>HD interface artwork gallery</title>
<style>body{margin:24px;background:#252a30;color:#eee;font:16px Arial}header{position:sticky;top:0;background:#252a30;padding:12px 0}input{padding:12px;width:min(560px,80vw)}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:18px}article{padding:16px;background:#343b43;overflow-wrap:anywhere}img{max-width:100%;max-height:210px;object-fit:contain;background:repeating-conic-gradient(#3d434b 0% 25%,#30363e 0% 50%) 50%/16px 16px}a{color:#b5d9ff}small{color:#b5c0ca}p{font-size:13px}</style>
<header><h1>HD interface artwork</h1><p>Packaged image previews. Click an image for full size. Interactive gameplay review remains pending.</p><input id="q" aria-label="Filter images" placeholder="Filter by pack or filename"> <span id="count"></span></header><main>'''
    page += '\n'.join(cards) + '''</main><script>const q=document.querySelector('#q');q.oninput=()=>{let n=0;document.querySelectorAll('article').forEach(a=>{a.hidden=!a.dataset.name.includes(q.value.toLowerCase());n+=!a.hidden});document.querySelector('#count').textContent=n+' images'};q.oninput()</script></html>'''
    (ROOT / 'art/preview.html').write_text(page, encoding='utf-8')
    print(f'Wrote gallery with {len(cards)} image families')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path)
    parser.add_argument('--gallery', action='store_true')
    args = parser.parse_args()
    result = validate()
    text = json.dumps(result, indent=2) + '\n'
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(text, encoding='utf-8')
    print(text)
    if args.gallery:
        gallery()
