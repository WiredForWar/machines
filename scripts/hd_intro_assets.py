"""Export the four restored static startup cards; never read or write video frames."""
import hashlib
import json
from pathlib import Path
import os
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'art/hd-intros'
PACK = ROOT / 'mods/faithful-hd-intros'
SOURCE = Path(os.environ.get('MACHINES_ART_SOURCE', ROOT / 'build/art-source')) / 'gui/menu'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    textures, masters = [], {}
    (PACK / 'gui/menu').mkdir(parents=True, exist_ok=True)
    (ART / 'previews-4k').mkdir(exist_ok=True)
    for name in ('acclaim', 'charlogo', 'legal', 'wait'):
        path = ART / 'masters' / (name + '.png')
        master = Image.open(path).convert('RGB')
        assert abs(master.width / master.height - 4 / 3) < .01, (name, master.size)
        masters[name] = {'path': path.relative_to(ROOT).as_posix(), 'size': list(master.size),
                         'sha256': sha(path), 'original_sha256': sha(SOURCE / (name + '.bmp'))}
        for scale in range(1, 7):
            suffix = '' if scale == 1 else f'_{scale}x'
            relative = f'gui/menu/{name}{suffix}.png'
            image = master.resize((640 * scale, 480 * scale), Image.Resampling.LANCZOS)
            output = PACK / relative
            image.save(output, compress_level=6)
            textures.append({'path': relative, 'size': list(image.size), 'sha256': sha(output)})
        # The bootstrap loader requests BMP directly before Gui's PNG resolver.
        # A dedicated HD BMP supplies that path; subsequent GUI requests select
        # the normal PNG variants, preserving their 640x480 logical dimensions.
        if name == 'wait':
            relative = 'gui/menu/wait.bmp'
            image = master.resize((2880, 2160), Image.Resampling.LANCZOS)
            output = PACK / relative
            image.save(output)
            textures.append({'path': relative, 'size': list(image.size), 'sha256': sha(output)})
        preview = Image.new('RGB', (3840, 2160))
        preview.paste(master.resize((2880, 2160), Image.Resampling.LANCZOS), (480, 0))
        preview.save(ART / 'previews-4k' / (name + '.png'), compress_level=6)
    (PACK / 'gui/menu/hd-static-intros.json').write_text(json.dumps({'version': 1, 'loading_bar': 'engine-drawn'}) + '\n')
    report = {'masters': masters, 'textures': textures, 'texture_count': len(textures),
              'scope': 'four static cards only; original video and animation files untouched'}
    (PACK / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    (PACK / 'native-check.tsv').write_text(''.join(f"{t['path']}\t{t['size'][0]}\t{t['size'][1]}\n" for t in textures))
    print('Exported 24 PNG variants, one bootstrap BMP, and four 3840x2160 presentation previews.')


if __name__ == '__main__':
    main()
