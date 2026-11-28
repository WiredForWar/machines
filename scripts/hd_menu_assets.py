"""Package generated menu masters and retain the original animated light signals.

The source frames supply signed lighting/motion deltas, not opaque board patches.
Each delta is resampled onto the same HD background crop. Original animation
definitions, randomized delays, loop points and sound triggers are untouched.
"""
import hashlib
import json
from pathlib import Path
import os

from PIL import Image, ImageMath

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "art/hd-menus"
SOURCE = Path(os.environ.get("MACHINES_ART_SOURCE", ROOT / "build/art-source"))
PACK = ROOT / "mods/faithful-hd-menus"
SCALES = (1, 2, 3, 4)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def package():
    PACK.mkdir(parents=True, exist_ok=True)
    entries = {}
    masters = {}
    definitions = {}
    backgrounds = {}

    def save(image, relative, scale, kind):
        path = Path(relative)
        suffix = "" if scale == 1 else f"_{scale}x"
        relative = (path.parent / f"{path.stem}{suffix}.png").as_posix()
        output = PACK / relative
        output.parent.mkdir(parents=True, exist_ok=True)
        same = False
        if output.exists():
            try:
                with Image.open(output) as previous:
                    same = previous.mode == image.mode and previous.size == image.size and previous.tobytes() == image.tobytes()
            except OSError:
                pass
        if not same:
            image.save(output, compress_level=6)
        entries[relative] = {"path": relative, "size": list(image.size),
                             "scale": scale, "kind": kind, "sha256": sha(output)}

    for name in [f"s{chr(n)}" for n in range(ord('a'), ord('q') + 1)] + ["msgbox", "msgbox2"]:
        wide_path = ART / "masters-wide" / f"{name}.png"
        master_path = wide_path if wide_path.exists() else ART / "masters" / f"{name}.png"
        master = Image.open(master_path).convert("RGBA" if name.startswith("msgbox") else "RGB")
        if wide_path.exists():
            if abs(master.width / master.height / (16 / 9) - 1) > .025:
                raise ValueError(f"Wrong widescreen aspect ratio: {name}: {master.size}")
            full = master.resize((3840, 2160), Image.Resampling.LANCZOS)
            save(full, f"gui/menu/wide/{name}.bmp", 1, "continuous-wide-background")
            master = full.crop((480, 0, 3360, 2160))
        if name.startswith("msgbox"):
            # The original color-key checker dimmed the menu behind a modal.
            # An alpha matte preserves that behavior without visible dithering.
            master = Image.alpha_composite(Image.new("RGBA", master.size, (0, 0, 0, 128)), master)
        if abs(master.width / master.height / (4 / 3) - 1) > .025:
            raise ValueError(f"Wrong master aspect ratio: {name}: {master.size}")
        masters[name] = {"path": master_path.relative_to(ROOT).as_posix(), "size": list(Image.open(master_path).size),
                         "sha256": sha(master_path)}
        for scale in SCALES:
            image = master.resize((640 * scale, 480 * scale), Image.Resampling.LANCZOS)
            backgrounds[name, scale] = image
            save(image, f"gui/menu/{name}.bmp", scale, "generated-background")
        print(f"Exported {name}", flush=True)

    frame_sources = {}
    for definition in sorted((SOURCE / "gui/menu/anim").glob("*.anm")):
        lines = [line.split() for line in definition.read_text().splitlines() if line.strip()]
        x, y, count, loop = map(int, lines[0])
        name = definition.stem.split("_")[0]
        background = "msgbox2" if name == "mb" and definition.stem == "mb_anim2" else "msgbox" if name == "mb" else name
        baseline = Image.open(SOURCE / lines[1][0]).convert("RGB")
        definitions[definition.relative_to(SOURCE).as_posix()] = {
            "sha256": sha(definition), "frames": count, "loop": loop, "offset": [x, y]}
        assert len(lines) - 1 >= count
        definitions[definition.relative_to(SOURCE).as_posix()]["ignored_trailing_rows"] = len(lines) - 1 - count
        for line in lines[1:]:
            relative = line[0]
            if relative in frame_sources:
                continue
            frame = Image.open(SOURCE / relative).convert("RGB")
            assert frame.size == baseline.size
            frame_sources[relative] = sha(SOURCE / relative)
            # Convert the original frame into a signed signal. Float channels
            # avoid clipping bright sparks before they are composited on HD art.
            deltas = [ImageMath.lambda_eval(lambda a: a["frame"] - a["base"],
                       frame=f.convert("F"), base=b.convert("F"))
                      for f, b in zip(frame.split(), baseline.split())]
            for scale in SCALES:
                rect = (x * scale, y * scale, (x + frame.width) * scale, (y + frame.height) * scale)
                patch = backgrounds[background, scale].convert("RGB").crop(rect)
                channels = [ImageMath.lambda_eval(lambda a: a["hd"] + a["signal"],
                            hd=hd.convert("F"), signal=signal.resize(patch.size, Image.Resampling.BICUBIC)).convert("L")
                            for hd, signal in zip(patch.split(), deltas)]
                save(Image.merge("RGB", channels), relative, scale, "retained-animation-signal")
        print(f"Exported {definition.name}: {count} frames, original timing/sound", flush=True)

    surround = ART / "masters/widescreen.png"
    image = Image.open(surround).convert("RGB").resize((3840, 2160), Image.Resampling.LANCZOS)
    save(image, "gui/menu/widescreen.bmp", 1, "generated-surround")
    masters["widescreen"] = {"size": list(Image.open(surround).size), "sha256": sha(surround)}
    from hd_menu_fonts import export_fonts
    export_fonts()
    font_report = json.loads((ART / "fonts.json").read_text())
    for texture in font_report["textures"]:
        entries[texture["path"]] = texture
    textures = list(entries.values())
    report = {"backgrounds": 19, "animation_frames": len(frame_sources),
              "animation_definitions": definitions, "source_frames": frame_sources,
              "masters": masters, "textures": textures, "texture_count": len(textures),
              "font_source": font_report["font_source"], "font_sha256": font_report["font_sha256"],
              "compressed_bytes": sum((PACK / t["path"]).stat().st_size for t in textures)}
    (PACK / "manifest.json").write_text(json.dumps(report, indent=2) + "\n")
    (PACK / "native-check.tsv").write_text("".join(f"{t['path']}\t{t['size'][0]}\t{t['size'][1]}\n" for t in textures))
    print(f"Packaged {len(textures)} textures; {len(frame_sources)} animated source frames", flush=True)


if __name__ == "__main__":
    package()
