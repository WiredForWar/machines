"""Package missing interface artwork from retained image-generation atlases.

Pillow assembles references and extracts/resizes generated art; it does not paint
replacement artwork. Existing HD packs, fonts, status text and movies are held.
"""
import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'art/hd-interface'
PACK = ROOT / 'mods/faithful-hd-interface-art'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def export():
    manifest = json.loads((ART / 'source-manifest.json').read_text())
    entries = {e['path']: e for e in manifest['assets']}
    previous = json.loads((PACK / 'manifest.json').read_text()) if (PACK / 'manifest.json').exists() else {}
    textures = {e['path']: e for e in previous.get('textures', [])}
    masters = previous.get('masters', {})
    completed = []
    for atlas in manifest['atlases']:
        path = ART / 'masters' / (atlas['name'] + '.png')
        if not path.exists():
            continue
        digest = sha(path)
        if masters.get(atlas['name'], {}).get('sha256') == digest:
            completed.append(atlas['name'])
            continue
        master = Image.open(path).convert('RGBA')
        if abs(master.width / master.height / (atlas['columns'] / atlas['rows']) - 1) > .04:
            raise ValueError('Generated atlas aspect changed: ' + atlas['name'])
        for relative in atlas['entries']:
            entry = entries[relative]
            column, row = entry['cell']
            cell = master.crop((round(column * master.width / atlas['columns']),
                round(row * master.height / atlas['rows']),
                round((column + 1) * master.width / atlas['columns']),
                round((row + 1) * master.height / atlas['rows'])))
            # Preserve registration for every sprite, not only cursors. The
            # weapon dropdown's frames share an 87x86 canvas with different
            # amounts of empty space; trimming each would break its animation.
            x0, y0, x1, y1 = entry['reference_rect']
            ref = atlas['cell_size']
            box = (round((x0 - column * ref) / ref * cell.width),
                   round((y0 - row * ref) / ref * cell.height),
                   round((x1 - column * ref) / ref * cell.width),
                   round((y1 - row * ref) / ref * cell.height))
            image = cell.crop(box)
            if image.getchannel('A').getbbox() is None:
                raise ValueError('Empty generated cell: ' + relative)
            for scale in (1, 2, 3, 4):
                size = tuple(v * scale for v in entry['size'])
                if max(size) > 2048:
                    raise ValueError('Runtime texture too large: ' + relative)
                texture = image.resize(size, Image.Resampling.LANCZOS)
                texture.putdata([(0, 0, 0, 0) if px[3] == 0 else px for px in texture.get_flattened_data()])
                file = Path(relative)
                suffix = '' if scale == 1 else f'_{scale}x'
                rel = (file.parent / (file.stem + suffix + '.png')).as_posix()
                output = PACK / rel
                output.parent.mkdir(parents=True, exist_ok=True)
                texture.save(output, optimize=True, compress_level=9)
                textures[rel] = {'path': rel, 'size': size, 'scale': scale,
                    'sha256': sha(output), 'file_bytes': output.stat().st_size,
                    'decoded_rgba_bytes': size[0] * size[1] * 4, 'master': atlas['name']}
        masters[atlas['name']] = {'path': path.relative_to(ROOT).as_posix(),
                                 'sha256': digest, 'size': master.size}
        completed.append(atlas['name'])
        print('Exported ' + atlas['name'], flush=True)
    for entry in manifest['assets']:
        if 'alias_of' not in entry:
            continue
        for scale in (1, 2, 3, 4):
            suffix = '' if scale == 1 else f'_{scale}x'
            src = Path(entry['alias_of'])
            key = (src.parent / (src.stem + suffix + '.png')).as_posix()
            if key not in textures:
                continue
            file = Path(entry['path'])
            rel = (file.parent / (file.stem + suffix + '.png')).as_posix()
            destination = PACK / rel
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes((PACK / key).read_bytes())
            textures[rel] = dict(textures[key], path=rel, alias_of=entry['alias_of'])
    output = sorted(textures.values(), key=lambda e: e['path'])
    report = {'asset_count': len(output) // 4, 'target_asset_count': len(manifest['assets']),
              'texture_count': len(output), 'masters': masters,
              'compressed_bytes': sum(e['file_bytes'] for e in output),
              'rgba_bytes_all_scales': sum(e['decoded_rgba_bytes'] for e in output),
              'rgba_bytes_at_4x': sum(e['decoded_rgba_bytes'] for e in output if e['scale'] == 4),
              'textures': output, 'remaining_atlases': [a['name'] for a in manifest['atlases'] if a['name'] not in completed]}
    PACK.mkdir(parents=True, exist_ok=True)
    (PACK / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    (PACK / 'native-check.tsv').write_text(''.join(f"{e['path']}\t{e['size'][0]}\t{e['size'][1]}\n" for e in output))
    print(json.dumps({k: v for k, v in report.items() if k not in ['textures', 'masters']}, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['export'])
    args = parser.parse_args()
    export()
