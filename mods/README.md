## Mods priority

The optional [HD interface packs](../art/README.md) are supplied with this
contribution. Copy complete `faithful-hd-*` directories into the game's `mods`
directory to enable them. Move them outside `mods` to disable them, then restart
the game. Merely renaming a directory inside `mods` does not disable a mod.

There is no way to set mod priority just yet. For now the mods are sorted
alphabetically. That is, if you have mods `a-mod`, `b-mod` then `a-mod` files
'll get the highest priority.

## Files and paths which support override

The game will use exactly one file from the most highest priority mod only.

- "data/factory.bld"
- "data/global.cst"
- "data/global.rst"
- "data/zenith.dat"

Also:
- Scenario files (e.g. "data/m_desert.scn")
- Planet map files (e.g. "models/planet/desert/desert.psf")
  - Other files such as `.csp` are always looked up next to the `.psf` file.
- Scenario `RESEARCH_ITEMS` files.

## Files and paths which support partial override

For those files the game will accumulate data from the vanilla file and then
from each mod file.

- "data/parmdata.dat"
- "machstrg.xml"
