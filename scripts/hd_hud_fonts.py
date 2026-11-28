"""Rasterize outline HUD text and package code-audited, generated 3D glyphs.

Artwork comes from the retained image-generation master. Pillow only performs
font rasterization, atlas extraction, alpha/format normalization and packaging.
"""
from collections import Counter
import hashlib
import json
from pathlib import Path
import os

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "art/hd-hud"
PACK = ROOT / "mods/faithful-hd-hud"
SOURCE = Path(os.environ.get("MACHINES_ART_SOURCE", ROOT / "build/art-source"))
FONT = SOURCE / "gui/fonts/U001/u001con-bol.ttf"
FALLBACK_FONT = SOURCE / "gui/Arial.ttf"
SYMBOLS = {0x7f: 0, 0x80: 1, 0x81: 2, 0x99: 3, 0x9d: 4,
           0x9e: 5, 0xa2: 6, 0xa3: 7, 0xa4: 8}
_symbol_cache = {}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def symbol_texture(index, size):
    """Fit a generated semantic icon into its original glyph advance box."""
    if index not in _symbol_cache:
        atlas = Image.open(ART / "masters/hud-symbols-3d.png").convert("RGBA")
        cell = SYMBOLS[index]
        x, y = cell % 3, cell // 3
        box = (round(x * atlas.width / 3), round(y * atlas.height / 3),
               round((x + 1) * atlas.width / 3), round((y + 1) * atlas.height / 3))
        icon = atlas.crop(box)
        # Ignore nearly transparent generation dust when finding the crop.
        bounds = icon.getchannel("A").point(lambda a: 255 if a > 48 else 0).getbbox()
        if bounds is None:
            raise ValueError(f"Empty generated symbol {index:02x}")
        _symbol_cache[index] = icon.crop(bounds)
    icon = _symbol_cache[index].copy()
    icon.thumbnail(size, Image.Resampling.LANCZOS)
    result = Image.new("RGBA", size)
    result.alpha_composite(icon, ((size[0] - icon.width) // 2, (size[1] - icon.height) // 2))
    return result


def read_atlas(path):
    original = Image.open(path).convert("RGBA")
    ends = [x for x in range(original.width)
            if original.getpixel((x, original.height - 1))[:3] == (255, 255, 255)]
    if len(ends) != 256:
        raise ValueError(f"Expected 256 glyph markers: {path}")
    start = 0
    glyphs = []
    for end in ends:
        glyph = original.crop((start, 0, end, original.height - 1))
        glyph.putdata([(0, 0, 0, 0) if (r, g, b) == (255, 0, 255) else (r, g, b, a)
                       for r, g, b, a in glyph.get_flattened_data()])
        glyphs.append(glyph)
        start = end + 1
    return original, glyphs


def text_mask(char, size):
    """Rasterize the outline at final density, retaining source advances."""
    oversample = 4
    face = FALLBACK_FONT if any(ord(c) > 127 for c in char) else FONT
    font = ImageFont.truetype(str(face), max(8, size[1] * oversample * 2))
    box = font.getbbox(char)
    if box[2] <= box[0] or box[3] <= box[1]:
        return Image.new("L", size)
    mask = Image.new("L", (box[2] - box[0], box[3] - box[1]))
    ImageDraw.Draw(mask).text((-box[0], -box[1]), char, font=font, fill=255)
    ratio = min(size[0] * oversample / mask.width, size[1] * oversample / mask.height)
    mask = mask.resize((max(1, round(mask.width * ratio / oversample)),
                        max(1, round(mask.height * ratio / oversample))), Image.Resampling.LANCZOS)
    result = Image.new("L", size)
    result.paste(mask, ((size[0] - mask.width) // 2, size[1] - mask.height))
    return result


def outline_glyph(char, size, color, bevel=False):
    mask = text_mask(char, size)
    image = Image.new("RGBA", size, color + (255,))
    if bevel:
        # A restrained vertical material gradient, computed from the native
        # glyph outline rather than enlarged legacy pixels.
        draw = ImageDraw.Draw(image)
        for y in range(size[1]):
            factor = 1.12 - .28 * y / max(1, size[1] - 1)
            draw.line((0, y, size[0], y), fill=tuple(min(255, round(c * factor)) for c in color) + (255,))
    image.putalpha(mask)
    return image


def glyph_character(index, glyph, fallback):
    if glyph.size == fallback.size and glyph.tobytes() == fallback.tobytes():
        return "\u25a1"  # Original missing-glyph box, never a gameplay symbol.
    arrows = {0x8d: "\u2191", 0x8e: "\u2193", 0x8f: "\u2190", 0x90: "\u2192"}
    if index in arrows:
        return arrows[index]
    if 33 <= index <= 126 or index >= 160:
        return bytes([index]).decode("cp1252", errors="replace")
    # Existing Windows-1252 punctuation that is not a special gameplay glyph.
    if index in (0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
                 0x8a, 0x8b, 0x8c, 0x91, 0x92, 0x93, 0x94,
                 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9e, 0x9f):
        return bytes([index]).decode("cp1252")
    return None


def export_fonts(source, iterations=1):
    from hd_hud_assets import compressed_png

    textures = []
    layouts = []
    _, normal_prompt = read_atlas(SOURCE / "gui/menu/promtfnt.bmp")
    for name in source["fonts"]:
        path = SOURCE / "gui/menu" / (name + ".bmp")
        original, glyphs = read_atlas(path)
        prompt = name in ("promtfnt", "promdfnt")
        shadow = name == "promdfnt"
        for scale in (1, 2, 3, 4):
            height = (original.height - 1) * scale
            widths = [g.width * scale for g in glyphs]
            atlas = Image.new("RGBA", (sum(w + 1 for w in widths), original.height * scale))
            starts = []
            cursor = 0
            for index, (glyph, w) in enumerate(zip(glyphs, widths)):
                starts.append(cursor)
                size = (w, height)
                if w and glyph.getchannel("A").getbbox():
                    if prompt and index in SYMBOLS:
                        generated = symbol_texture(index, size)
                    elif prompt and index in (0xa9, 0xaa, 0xab, 0xac):
                        # Faction colour swatches retain their precise role.
                        generated = glyph.resize(size, Image.Resampling.LANCZOS)
                    else:
                        # The shown large-font slots are unsupported-character
                        # boxes, not gameplay symbols or a Unicode Z-caron.
                        if not prompt and index in (0x7f, 0x80, 0x81, 0x9d, 0x9e):
                            char = "\u25af"
                        else:
                            char = glyph_character(index, glyph, glyphs[0x7f])
                        if char is not None and prompt:
                            char = char.upper()
                        if char is None:
                            generated = glyph.resize(size, Image.Resampling.LANCZOS)
                        else:
                            if prompt:
                                base = normal_prompt[index]
                                colors = Counter((r, g, b) for r, g, b, a in base.get_flattened_data() if a)
                                dominant = colors.most_common(1)[0][0] if colors else (255, 255, 255)
                                color = (255, 232, 24) if dominant[0] > 160 and dominant[1] > 160 and dominant[2] < 80 else (249, 247, 231)
                            else:
                                color = {"largefnt": (239, 231, 178), "largyfnt": (249, 231, 24),
                                         "largdfnt": (35, 42, 110)}[name]
                            generated = outline_glyph(char, size, color, bevel=not prompt)
                    if shadow:
                        alpha = generated.getchannel("A")
                        generated = Image.new("RGBA", size)
                        generated.putalpha(alpha)
                    atlas.alpha_composite(generated, (cursor, 0))
                # Exactly one separator pixel per glyph, in the final marker
                # row. The preceding scale-1 marker rows remain fully empty.
                atlas.putpixel((cursor + w, atlas.height - 1), (255, 255, 255, 255))
                cursor += w + 1
            suffix = "" if scale == 1 else f"_{scale}x"
            destination = PACK / "gui/menu" / f"{name}{suffix}.png"
            destination.parent.mkdir(parents=True, exist_ok=True)
            encoded, before = compressed_png(atlas, min(iterations, 1))
            destination.write_bytes(encoded)
            textures.append({"path": destination.relative_to(PACK).as_posix(),
                "size": list(atlas.size), "scale": scale, "mode": "RGBA",
                "file_bytes": len(encoded), "before_zopfli_bytes": before,
                "decoded_rgba_bytes": atlas.width * atlas.height * 4,
                "sha256": sha(destination), "master": "hud-symbols-3d" if prompt else "outline-large-font",
                "kind": "outline-hud-font"})
            layouts.append({"path": destination.relative_to(PACK).as_posix(), "scale": scale,
                            "glyph_widths": widths, "glyph_starts": starts, "glyph_height": height,
                            "original_source": path.relative_to(SOURCE).as_posix(), "source_sha256": sha(path)})
            print(f"Rasterized {name} at {scale}x", flush=True)
    report = {"font_source": FONT.relative_to(SOURCE).as_posix(), "font_sha256": sha(FONT),
              "fallback_font_source": FALLBACK_FONT.relative_to(SOURCE).as_posix(),
              "fallback_font_sha256": sha(FALLBACK_FONT),
              "symbol_master_sha256": sha(ART / "masters/hud-symbols-3d.png"),
              "textures": textures, "layouts": layouts}
    (ART / "fonts.json").write_text(json.dumps(report, indent=2) + "\n")
    return report
