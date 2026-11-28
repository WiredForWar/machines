"""Rasterize clean outline text into compatible menu atlases, retaining advances."""
from pathlib import Path
import os
from collections import Counter
from PIL import Image, ImageDraw, ImageFont
import hashlib, json

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path(os.environ.get("MACHINES_ART_SOURCE", ROOT / "build/art-source")) / "gui"
PACK = ROOT / "mods/faithful-hd-menus/gui/menu"
FONT = SOURCE / "Arial.ttf"


def export_fonts():
    report = {"font_source": "gui/Arial.ttf", "font_sha256": hashlib.sha256(FONT.read_bytes()).hexdigest(), "textures": []}
    for name in ("smallfnt", "smaldfnt", "smalwfnt"):
        original = Image.open(SOURCE / "menu" / f"{name}.bmp").convert("RGB")
        markers = [x for x in range(original.width) if original.getpixel((x, original.height - 1)) == (255, 255, 255)]
        starts, widths, x = [], [], 0
        for end in markers:
            starts.append(x); widths.append(end - x); x = end + 1
        assert len(markers) == 256
        color = {"smallfnt": (96, 224, 64), "smaldfnt": (10, 16, 10),
                 "smalwfnt": (245, 242, 218)}[name]
        for scale in range(1, 7):
            height = 6 * scale
            atlas = Image.new("RGBA", (sum(w * scale + 1 for w in widths), 7 * scale))
            cursor = 0
            for index, (start, width) in enumerate(zip(starts, widths)):
                w = width * scale
                if w and index == 0x7f:
                    # wonSymbolText uses this glyph for completed scenarios;
                    # it is not the green mined-resource symbol at byte 0x99.
                    from hd_hud_fonts import symbol_texture
                    atlas.alpha_composite(symbol_texture(index, (w, height)), (cursor, 0))
                elif w and 33 <= index <= 126:
                    char = chr(index).upper()
                    oversample = 4
                    # Select the outline size by actual cap height, not its em.
                    pixels = 5 * scale * oversample
                    size = max(1, round(pixels / .72))
                    font = ImageFont.truetype(str(FONT), size)
                    box = font.getbbox(char)
                    mask = Image.new("L", (max(1, box[2]-box[0]), max(1, box[3]-box[1])))
                    ImageDraw.Draw(mask).text((-box[0], -box[1]), char, font=font, fill=255)
                    factor = min(w * oversample / mask.width, pixels / mask.height)
                    mask = mask.resize((max(1, round(mask.width*factor/oversample)), max(1, round(mask.height*factor/oversample))), Image.Resampling.LANCZOS)
                    glyph = Image.new("RGBA", mask.size, color + (255,)); glyph.putalpha(mask)
                    atlas.alpha_composite(glyph, (cursor + (w-mask.width)//2, height-mask.height))
                elif w:
                    glyph = original.crop((start, 0, start + width, 6)).convert("RGBA")
                    glyph.putdata([(0,0,0,0) if (r,g,b)==(255,0,255) else (r,g,b,a) for r,g,b,a in glyph.get_flattened_data()])
                    atlas.alpha_composite(glyph.resize((w,height), Image.Resampling.LANCZOS),(cursor,0))
                atlas.putpixel((cursor+w, 7*scale-1),(255,255,255,255))
                cursor += w+1
            suffix = "" if scale == 1 else f"_{scale}x"
            path = PACK / f"hd-{name}{suffix}.png"
            atlas.save(path, compress_level=6)
            report["textures"].append({"path": path.relative_to(ROOT/'mods/faithful-hd-menus').as_posix(), "size":list(atlas.size),"scale":scale,"sha256":hashlib.sha256(path.read_bytes()).hexdigest(),"kind":"outline-menu-font"})
    (ROOT/'art/hd-menus/fonts.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Generated 18 antialiased menu font atlases from bundled outline font')

if __name__ == '__main__':
    export_fonts()
