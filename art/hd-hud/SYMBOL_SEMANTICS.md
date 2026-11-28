# HUD symbol meanings and redraw contract

The gameplay meanings were audited against code before rendering the symbols.
Only the active, code-audited masters are included in this contribution.

## Verified gameplay meanings

| Asset or glyph | Verified meaning | Evidence in source | Redraw direction |
| --- | --- | --- | --- |
| `promtfnt` byte `0x9d` | Current hit points / health | `render/BmpFont.cpp::healthPointsIndex`; `machgui/InGameScreen.cpp::addPromptTextMachineInfo` passes `pActor->hp()` | Green, solid extruded medical plus; smooth bevels, no pixel blocks |
| `0x80` | Current armour | `armorPointsIndex`; same prompt passes `pActor->armour()` | Blue/purple 3D armour shield with downward point; not an arrow or attack statistic |
| `0x81` | Building material units (BMUs), context-dependent quantity | `bmuPointsIndex`; build/research cost prompts, resource-carrier `amountCarried()/capacity()`, mine remaining ore, debris quantity | Compact yellow/black striped industrial material block; not a gold coin or an unrelated warning sign |
| `0x99` | Ore accumulated at a mine, distinct from remaining mineral-site ore | `bmuMinedPointsIndex`; `addPromptTextConstructionInfo` passes `pActor->asMine().ore()` | Related green/black material block, visibly distinct from yellow BMUs |
| `0x9e` | Research points or research rate, according to context | `researchPointsIndex`; `HWResearchIcon.cpp` uses `researchCost()`, machine prompt uses technician `researchRate()`, lab prompt uses `totalResearchRate()` | Compact 3D idea/research light bulb with yellow-white glass and a metal base. The bulb interpretation is visual; its research meaning is established by code |
| `gui/navigate/bmus.bmp` | Toggle the player's stored BMU count on the minimap | `InGameScreen.cpp` constructs `MachGuiBmuButton`; `BmuText.cpp` shows/hides BMU totals; `machstrg.xml` string 1830 | Bevelled resource-container/button matching the yellow material symbol and original silver frame |

`machphys/machphys.hpp` explicitly names the numeric type
`BuildingMaterialUnits`. Do not expand BMU as an unverified alternative phrase.
`BmuText::refresh` reads `nBuildingMaterialUnits` and
`nMaxBuildingMaterialUnits`; its display is current stock / maximum storage.
For a resource carrier the same yellow glyph precedes its carried quantity /
capacity. Incomplete construction percentages are ordinary dynamic text, not
individual numbered sprites.

## Other symbols: preserve their distinct roles

- `0x7f` is inserted by `MachGui::wonSymbolText()` when a scenario/planet has
  been completed. Its green appearance does not make it a resource glyph.
- `0xa2` represents an administrator transported inside an APC.
- `0xa3` represents an aggressor transported inside an APC.
- `0xa4` represents any other transported machine. The caller's default case
  includes more than civilian units, so do not imply a narrower gameplay rule.
- `0xa5` / `0xa6` delimit the APC occupant list, and `0x85` is its overflow
  ellipsis after the first ten occupants.
- `0x8d`–`0x90` are keyboard direction-arrow glyphs, as used by KeyToString.
- `0xa9`–`0xac` are faction-colour markers. Preserve their indices and colours.

The redraw covers all nine shown gameplay symbols. The
victory marker and three APC class insignia are now covered. Keep the original
insignia designs (yellow diamond/pattern, yellow V, blue diagonal) with modern
3D metal/enamel materials; generic robot portraits were rejected. The list
brackets, overflow ellipsis, keyboard arrows and faction swatches keep their
distinct roles and byte indices.

The large menu fonts use a different character mapping: `0x99` is the trademark
sign, `0xa2` is cent, `0xa3` is pound and `0xa4` is the generic currency sign.
The pictured `0x7f`, `0x80`, `0x81`, `0x9d` and `0x9e` large-font cells are
unsupported-character boxes. They must remain clean fallback boxes, not become
gameplay resources, armour, robots, the digit zero or an invented Z-caron.
The normal and yellow large fonts receive outline characters at native density;
their lower-case alphabet is preserved separately from the all-capital prompt
font. Arial supplies Unicode currency/fallback characters absent from the
bundled condensed font.

## Production requirements

Render semantic shapes with real smooth geometry, bevels and material lighting,
not a 3D treatment of an enlarged pixel grid. Preserve original functional
colours and legibility at final on-screen sizes. Masters have more detail than
the 100–400% exported glyphs; individual runtime icons should match their actual
HUD dimensions rather than each being a wasteful 3840-pixel texture.

The prompt font must retain all 256 byte indices, original character advances,
normal/shadow pairing, dynamic numbers, percentage signs and existing colour
conventions. White text and yellow numbers are rasterized from a scalable outline
font at their final density. A glyph in a menu font may have a different meaning
from the same byte in the gameplay font: do not globally replace font bytes.

Existing command artwork also encodes states: camouflage on/off, ion/nuclear
recharge, self-destruct animation, and initiative LEDs. Preserve every filename,
state, frame sequence and timing. Complete all missing command types rather than
testing only the first selected unit's buttons.
