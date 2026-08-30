
### v1.7.5

Mostly a bugfix and maintenance release: the game moved to SDL3, gained a native MSVC build, and
had its renderer rebuilt underneath.

- Ground camera height (z position) now always saved/restored on view save/restore
- Following camera now persists when switching camera type (Ground <-> Zenith)
- A number of pathfinding improvements (performance and responsiveness)
- Screenshot capture is now more robust, and the 'Paused' overlay is no longer included
- Screenshot of a menu holds just the menu instead of the black border around it
- Optimized the game startup
- Added models, cursors, and planet minimaps texture modding support
- Added support for scaled planet maps
- Added a console (press `Ctrl+Alt+Shift+F11` and type `console on` there)
- Screenshots now go into a `screenshots` directory instead of the game's own data
- PNG textures are now preferred over the original formats and can be overridden by mods
- Display options are now Window Mode (Exclusive Fullscreen / Borderless Fullscreen /
  Windowed), Resolution and Refresh Rate as separate settings, instead of one long list
- Reworked first person aiming to behave the same whatever the frame rate and resolution
- Music now crossfades between the menu and gameplay tracks, and each resumes where it
  left off instead of restarting from the beginning on every switch
- The starfield now twinkles, and star colours and sizes follow the star's brightness
- FPS counter (`Ctrl+Shift+F7`) is now available in release builds
- `Enter` now sends a chat message to all players, `F2`/`F3`/`F4` address a target and
  toggle alliances
- Text fields now support word jump and word deletion with `Ctrl`, and `Ctrl+V` paste
- The game version is now carried in LAN discovery and checked when joining. A client
  with a mismatched version is now told so, along with both versions.
- Joining a session no longer blocks the game while it waits for the host
- Removed the 'Screen resolution lock' option
- Removed the Gamma correction option, which SDL3 no longer supports
- Fixed constructions interiors not being race specific (always red)
- Fixed machines moving forth and back in corridors (e.g. in 'Regain Control' mission)
- Fixed many cases where machines could get stuck and lock each other
- Fixed camera move and zoom at the same time (basically reimplemented zoom with mouse wheel)
- Fixed selection marker rendering (2D/3D and runtime switching)
- Fixed rendering/culling issues that could cause missing world tiles / rooms not being rendered
- Fixed nearest Mine and Smelter/Pod lookup for Transporters / resource carriers
- Fixed nearest Lab lookup for Technicians
- Fixed a destroyed machine carrying on with its update while teleporting in or out
- Fixed a crash when a Resource Carrier was sent to a mine it could not reach
- Fixed crash on close by Alt+F4
- Fixed a crash on choosing an entry from a drop down list
- Fixed the audio stuttering during video playback
- Fixed the anti-aliasing option not being applied
- Fixed the drawing of rectangles and lines
- Fixed a number of issues with 2x interface scale
- Fixed the game menus opening while a modifier key was held
- Fixed multiplayer chat messages being truncated
- Fixed the port byte order in LAN discovery packets
- Fixed a double ENet shutdown
- Fixed sprite animations (fires, explosions, and the like) running at the frame rate
  rather than at the speed of the game
- A few more small fixes

Under the hood:
- Ported to SDL3
- Added Conan 2 support
- The game now builds with MSVC
- The game now builds with Clang
- Dropped the Alure dependency (music is decoded with a bundled stb_vorbis)
- Reworked the renderer API (ready for multiple render backends)
- Added `--skip-intro`, which starts the game at the main menu

### v1.7.4

(v1.7.3 had only pre-release builds)

- Added an option to use WASD for the camera and 1st person control
  Check the in-game Hotkeys menu for more information about the assigned keys.
- Added 'Camera acceleration' option
- Added initial mods support (allows game data override)
- Implemented VSync
- TTF font used for the Game Menus
- 'Grab Cursor' option is now applied at runtime
- The game now won't move the camera if the cursor is out of the window
- Implemented Zenith camera rotation support for the Following Camera
- Fixed special characters in menu fonts (fixes German localization)
- Fixed a crash on focus on LandMine (thanks @Fuechschen-San for discovering that)
- Fixed negative damage on construction capture (fixes armor overflow, thanks Iris for detailed steps-to-reproduce)
- Fixed actor Icon not updated on race changed (e.g. for captured constructions)
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
