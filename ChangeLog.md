### v1.7.3

- Added an option to use WASD for camera and 1st person control (enable it in the Options menu).
  Check the in-game Hotkeys menu for more information about the assigned keys.
- Added initial mods support (allows game data override)
- Fixed special characters in menu fonts (fixes German localization)
- Fixed a crash on focus on LandMine (thanks @Fuechschen-San for discovering that)
- Fixed negative damage on construction capture (fixes armor overflow, thanks Iris for detailed steps-to-reproduce)
- Fixed actor Icon not updated on race changed (e.g. for captured constructions)
- Camera inertia reduced by 50% (this needs its own option of course)
- 'Windowed mode' option now always added to the config file (this also needs a checkbox)

Under the hood:
- Totally reworked keyboard input
- Much improved localization support (still no Unicode)

### v1.7.2

Skipped (had only a broken pre-release).

- Prompt now shows the BMUs possibly gained on a Deconstruction
- Fixed weird units reaction to given commands
- Fixed a few possible crashes

### v1.7.1

- Implemented 2X UI scaling (a lot of HD textures wanted)
- Implemented recent events manager (to cycle through the positions of the last important events with SpaceBar key)
- Implemented a following camera (click on the Actor in corral or double click the squadron key (<0-9>)
- Implemented mouse grab (important for multi-display configurations)
- Implemented LAN game discovery
- Added Shift+<0-9> to add units to the squadron
- Added the same type selection by Ctrl-click or double click
- Shift+click now toggles selection
- Added grave (tilde) hotkey to toggle the 1st person camera
- Removed selection clear on fast RMB click on mini-map (actually felt like a bug: "sometimes selection is lost on mini-map clicked")
- Production and research items can be canceled by a click
- Screenshots are now saved as PNG rather than BMP format
- Navigation button with counter now uses TTF instead of BMP font
- Actor name in the panel description now respectes the HW level (e.g. now it names 'Scout' as 'Scout' rather than 'Grunt')
- Increased the number of drawn tiles (fixes missing terrain textures)
- Fixed interaction with the world on 4k screens
- Fixed Windows-specific issue with DPI scaling enabled
- Fixed Alt+A and Alt+S hotkeys
- Fixed prompt message on a lab hovered
- Fixed incorrect game resolution until the player opens the options menu for the first time
- Fixed illumination on Desert map
- Fixed the updating of the game load progress bar
- Fixed Performance Options not loaded correctly
- Fixed network desync which caused empty/invalid players list on join (client side)
- Fixed a crash on processing a net message referencing object which is not loaded yet
- Fixed Reinforcements had no voice mails after loading a saved game
- Fixed a few crashes
- A few more small fixes
