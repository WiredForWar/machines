"""Prepare references and package generated HUD art; never repaint game art here.

Image generation produces the masters. This tool only assembles references,
extracts atlas cells, converts formats/scales, and compresses PNGs losslessly.
"""

import argparse
import hashlib
import io
import json
from pathlib import Path
import os

from PIL import Image
import zopfli.png

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "art/hd-hud"
PACK = ROOT / "mods/faithful-hd-hud"
RACES = ("red", "blue", "green", "yellow")


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_reference(path):
    companion = path.with_name(path.stem + "_2x.png")
    image = Image.open(companion if companion.exists() else path).convert("RGBA")
    # The engine's original BMP colour key is magenta. PNG companions already
    # supply alpha. Convert only that legacy key when preparing a reference.
    image.putdata([(0, 0, 0, 0) if a == 0 or (r, g, b) == (255, 0, 255) else (r, g, b, a)
                   for r, g, b, a in image.get_flattened_data()])
    return image


def prepare(source):
    groups = {
        "controls": ([f"navigate/{race}/{name}.bmp" for race in RACES
                       for name in ("machines", "construc", "squads")]
                      + [f"misc/{name}.bmp" for name in ("return1", "return2", "zenith", "ground")],
                     4, (320, 320), 8),
        "commands": ([f"commands/{name}.bmp" for name in
                      ("move", "stop", "const", "repair", "capture", "deconst", "recycle", "patrol", "formsqad", "stand")]
                     + [f"commands/selfdes{i}.bmp" for i in range(1, 9)]
                     + [f"commands/defco{i}.bmp" for i in (1, 2, 3, 12, 23, 123, 13)],
                     2, (640, 128), 8),
        "arrows": ([f"misc/{name}.bmp" for name in
                    ("scrlf", "scrlfh", "scrll", "scrllh", "scrolll", "scrolllh", "scrollr", "scrollrh", "scrrf", "scrrfh", "scrrl", "scrrlh")]
                   + [f"corral/{name}.bmp" for name in
                      ("scrlup", "scrluph", "scrlup2", "scrlup2h", "scrldn", "scrldnh", "scrldn2", "scrldn2h")],
                   5, (256, 512), 8),
        "digits": ([f"navigate/navi{i}.bmp" for i in range(10)]
                   + [f"navigate/numbmu{i}.bmp" for i in range(10)]
                   + ["navigate/numbmus.bmp"]
                   + [f"navigate/numtop{i}.bmp" for i in range(10)]
                   + [f"misc/numsqd{i}.bmp" for i in range(10)],
                   10, (128, 192), 12),
    }
    for race in RACES:
        groups[f"decals-{race}"] = ([f"misc/{race}/decal{i}.bmp" for i in range(1, 5)],
                                   2, (768, 512), 8)
    manifest = {"source": str(source.resolve()), "assets": [], "atlases": {}}
    references = ART / "references"
    references.mkdir(parents=True, exist_ok=True)
    for name, (files, columns, (cw, ch), scale) in groups.items():
        rows = (len(files) + columns - 1) // columns
        canvas = Image.new("RGBA", (columns * cw, rows * ch))
        for index, relative in enumerate(files):
            source_file = source / "gui" / relative
            original_size = Image.open(source_file).size
            w, h = [n * scale for n in original_size]
            if w > cw - 24 or h > ch - 24:
                raise ValueError(f"Reference cell too small: {relative} {w}x{h}")
            x = index % columns * cw + (cw - w) // 2
            y = index // columns * ch + (ch - h) // 2
            reference = load_reference(source_file).resize((w, h), Image.Resampling.NEAREST)
            canvas.paste(reference, (x, y))
            manifest["assets"].append({"path": "gui/" + relative,
                                       "size": original_size, "atlas": name,
                                       "rect": [x, y, x + w, y + h],
                                       "source_sha256": sha(source_file.read_bytes())})
        canvas.save(references / f"{name}.png", optimize=True)
        manifest["atlases"][name] = {"size": canvas.size, "columns": columns,
                                     "rows": rows, "cell": [cw, ch], "count": len(files)}
    # The wide top trim and narrow ornaments share one tightly packed master.
    supplements = {"decal5": (92, 64), "decal6": (20, 512), "decal7": (500, 700),
                   "slider": (900, 700), "slidhole": (1200, 570)}
    canvas = Image.new("RGBA", (1536, 1152))
    for name, (x, y) in supplements.items():
        source_file = source / "gui/misc" / f"{name}.bmp"
        original_size = Image.open(source_file).size
        w, h = [n * 8 for n in original_size]
        reference = load_reference(source_file).resize((w, h), Image.Resampling.NEAREST)
        canvas.paste(reference, (x, y))
        manifest["assets"].append({"path": f"gui/misc/{name}.bmp", "size": original_size,
                                   "atlas": "supplements", "rect": [x, y, x + w, y + h],
                                   "source_sha256": sha(source_file.read_bytes())})
    canvas.save(references / "supplements.png", optimize=True)
    manifest["atlases"]["supplements"] = {"size": canvas.size, "count": len(supplements)}
    singles = ["map/" + f + "logo.bmp" for f in "rbgy"] + [
        "misc/backtile.bmp", "misc/camtab.bmp"]
    for relative in singles:
        source_file = source / "gui" / relative
        original_size = Image.open(source_file).size
        name = Path(relative).stem
        reference = load_reference(source_file)
        reference.save(references / f"{name}.png", optimize=True)
        manifest["assets"].append({"path": "gui/" + relative, "size": original_size,
                                   "master": name, "source_sha256": sha(source_file.read_bytes())})
    (ART / "source-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Prepared {len(manifest['assets'])} assets and {len(manifest['atlases'])} reference atlases.")


def compressed_png(image, iterations):
    buffer = io.BytesIO()
    image.save(buffer, format="PNG", optimize=True, compress_level=9)
    encoded = buffer.getvalue()
    optimized = zopfli.png.optimize(encoded, num_iterations=iterations,
                                    num_iterations_large=iterations,
                                    lossy_transparent=False, lossy_8bit=False)
    result = optimized if len(optimized) < len(encoded) else encoded
    # Compression must never change the image, including transparent RGB data.
    decoded = Image.open(io.BytesIO(result)).convert(image.mode)
    if decoded.size != image.size or decoded.tobytes() != image.tobytes():
        raise ValueError("Compression changed decoded pixels")
    return result, len(encoded)


def load_master(path, background=None):
    image = Image.open(path).convert("RGBA")
    if background == "black":
        # Decode an additive black matte without introducing colored fringes.
        pixels = []
        for r, g, b, _ in image.get_flattened_data():
            alpha = max(r, g, b)
            pixels.append((0, 0, 0, 0) if alpha <= 20 else
                          (round(r * 255 / alpha), round(g * 255 / alpha),
                           round(b * 255 / alpha), alpha))
        image.putdata(pixels)
    if background == "magenta":
        # Decode the generated chroma-key format, as with the source BMPs.
        # Unmatte antialiased boundaries so magenta cannot fringe the HUD.
        pixels = []
        for r, g, b, _ in image.get_flattened_data():
            alpha = 255 - max(0, min(r, b) - g)
            if alpha <= 16:
                pixels.append((0, 0, 0, 0))
            else:
                pixels.append((max(0, min(255, round((r - 255 + alpha) * 255 / alpha))),
                               min(255, round(g * 255 / alpha)),
                               max(0, min(255, round((b - 255 + alpha) * 255 / alpha))), alpha))
        image.putdata(pixels)
    return image


def pack(iterations, only=None):
    source = json.loads((ART / "source-manifest.json").read_text())
    previous = json.loads((PACK / "manifest.json").read_text()) if only else {}
    by_path = {entry["path"]: entry for entry in previous.get("textures", [])}
    selected = set(only or [])
    known = {entry.get("atlas", entry.get("master")) for entry in source["assets"]}
    if source.get("fonts"):
        known.add("hud-symbols-3d")
    if selected - known:
        raise ValueError(f"Unknown masters: {sorted(selected - known)}")
    masters = {}
    for entry in source["assets"]:
        name = entry.get("atlas", entry.get("master"))
        if selected and name not in selected:
            continue
        master_path = ART / "masters" / f"{name}.png"
        if not master_path.is_file():
            raise FileNotFoundError(f"Missing generated master: {master_path}")
        if name not in masters:
            masters[name] = load_master(master_path, entry.get("master_background"))
        master = masters[name]
        if "master_cell" in entry:
            layout = source["atlases"][name]
            column, row = entry["master_cell"]
            master = master.crop((round(column * master.width / layout["columns"]),
                                  round(row * master.height / layout["rows"]),
                                  round((column + 1) * master.width / layout["columns"]),
                                  round((row + 1) * master.height / layout["rows"])))
            bounds = master.getchannel("A").point(lambda alpha: 255 if alpha >= 240 else 0).getbbox()
            if bounds is None:
                raise ValueError(f"Empty generated navigation cell: {entry['path']}")
            master = master.crop((max(0, bounds[0] - 1), max(0, bounds[1] - 1),
                                  min(master.width, bounds[2] + 1), min(master.height, bounds[3] + 1)))
        elif "atlas" in entry:
            ref_size = source["atlases"][name]["size"]
            if abs(master.width / master.height / (ref_size[0] / ref_size[1]) - 1) > .025:
                raise ValueError(f"Atlas aspect ratio changed: {name}")
            box = tuple(round(v * master.size[i % 2] / ref_size[i % 2])
                        for i, v in enumerate(entry["rect"]))
            master = master.crop(box)
        for scale in (1, 2, 3, 4):
            size = tuple(n * scale for n in entry["size"])
            if max(size) > 2048:
                raise ValueError(f"Texture edge budget exceeded: {entry['path']} {size}")
            texture = master.resize(size, Image.Resampling.LANCZOS)
            if "runtime_overlay" in entry:
                overlay = entry["runtime_overlay"]
                housing = load_master(ART / "masters" / (overlay["master"] + ".png"))
                housing = housing.resize(tuple(n * scale for n in overlay["base_size"]),
                                         Image.Resampling.LANCZOS)
                texture = housing.crop(tuple(n * scale for n in overlay["rect"]))
            # Hidden RGB in alpha-zero pixels is discarded during format
            # normalization. It has no visible effect and needlessly inflates PNG.
            texture.putdata([(0, 0, 0, 0) if a == 0 else (r, g, b, a)
                             for r, g, b, a in texture.get_flattened_data()])
            # Opaque files need no alpha plane. Transparent files retain alpha.
            if texture.getextrema()[3] == (255, 255):
                texture = texture.convert("RGB")
            path = Path(entry["path"])
            suffix = "" if scale == 1 else f"_{scale}x"
            destination = PACK / path.parent / f"{path.stem}{suffix}.png"
            destination.parent.mkdir(parents=True, exist_ok=True)
            encoded, standard_size = compressed_png(texture, iterations)
            destination.write_bytes(encoded)
            relative = destination.relative_to(PACK).as_posix()
            by_path[relative] = {"path": relative,
                           "size": size, "scale": scale, "mode": texture.mode,
                           "file_bytes": len(encoded), "before_zopfli_bytes": standard_size,
                           "decoded_rgba_bytes": size[0] * size[1] * 4,
                           "sha256": sha(encoded), "master": name}
        print(f"Packed {entry['path']}", flush=True)
    output = []
    for entry in source["assets"]:
        path = Path(entry["path"])
        for scale in (1, 2, 3, 4):
            suffix = "" if scale == 1 else f"_{scale}x"
            relative = (path.parent / f"{path.stem}{suffix}.png").as_posix()
            output.append(by_path[relative])
    master_report = dict(previous.get("masters", {}))
    master_report.update({name: {"size": list(im.size), "sha256": sha((ART / 'masters' / f'{name}.png').read_bytes())}
                          for name, im in masters.items()})
    master_report = {name: data for name, data in master_report.items() if name in known}
    if source.get("fonts"):
        from hd_hud_fonts import export_fonts
        font_report = export_fonts(source, iterations)
        output.extend(font_report["textures"])
        symbol_path = ART / "masters/hud-symbols-3d.png"
        master_report["hud-symbols-3d"] = {"size": list(Image.open(symbol_path).size),
                                           "sha256": sha(symbol_path.read_bytes())}
    report = {"asset_count": len(source["assets"]) + len(source.get("fonts", [])), "texture_count": len(output),
              "compressed_bytes": sum(e["file_bytes"] for e in output),
              "rgba_bytes_all_scales": sum(e["decoded_rgba_bytes"] for e in output),
              "rgba_bytes_by_scale": {str(s): sum(e["decoded_rgba_bytes"] for e in output if e["scale"] == s)
                                      for s in (1, 2, 3, 4)},
              "masters": master_report, "textures": output}
    if report["rgba_bytes_all_scales"] > 64 * 1024 * 1024:
        raise ValueError("Aggregate texture memory budget exceeded")
    (PACK / "manifest.json").write_text(json.dumps(report, indent=2) + "\n")
    (PACK / "native-check.tsv").write_text("".join(
        f"{e['path']}\t{e['size'][0]}\t{e['size'][1]}\n" for e in output))
    print(json.dumps({k: v for k, v in report.items() if k not in ("textures", "masters")}, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("prepare", "pack"))
    parser.add_argument("--source", type=Path)
    parser.add_argument("--iterations", type=int, default=15)
    parser.add_argument("--only", nargs="+", help="Export selected masters, retaining other packaged textures")
    args = parser.parse_args()
    if args.command == "prepare":
        if not args.source:
            parser.error("prepare requires --source")
        prepare(args.source)
    else:
        pack(args.iterations, args.only)
