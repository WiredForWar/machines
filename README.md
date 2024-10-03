# Machines: Wired For War (community fork)

**3D Real Time Strategy video game GL port source code**
Released as a [free software](https://en.wikipedia.org/wiki/Free_software) under a permission of [Night Dive Studios](https://www.nightdivestudios.com/) who owns the rights to the game.

## Community fork

This version of the game is a community fork. It is purposely different from the original source code, which is available [here](https://github.com/markol/machines).

## Changes

The most notable changes are:
- Added support for 2X UI scaling
- Added keyboard shortcuts
- Fixed various LAN game issues
- Fixed tens of bugs
- Fixed a dozen of crashes

Windows builds are now done automatically by GitHub.

## Installation

### Windows

1. Download https://download.wiredforwar.org/Game/Community-Build-1.5/machines-assets.7z. The archive has `machines` directory inside.
2. Extract the archive (`machines-assets.7z`) to the wanted installation directory.
3. Open https://github.com/WiredForWar/machines/releases and download `machines-win64-wfw-%releasedate%.7z` from the assets of the latest release. E.g. release [v1.7.1](https://github.com/WiredForWar/machines/releases/tag/v1.7.1) has [machines-win64-wfw-20240910.7z](https://github.com/WiredForWar/machines/releases/download/v1.7.1/machines-win64-wfw-20240910.7z) in its Assets. The downloaded win64 archive also has `machines` directory inside.
4. Extract the archive (`machines-win64-wfw-%releasedate%.7z`) *to the same* installation directory and *overwrite* the existing files (the win64 archive has new version of some assets).

Note: if you don't have `machines.exe` inside the extracted `machines` then maybe you've extracted the second archive into the first, which resulted in `machines/machines/machines.exe`. In order to get a correct installation, try to redo the steps from `#2`, this time make sure that you've extracted both archives into the same directory.

### GNU/Linux

#### Flatpak

Use Flatpak to install the client (only amd64 for now):
```
flatpak install --user https://wiredforwar.github.io/flatpak/wiredforwar-machines.flatpakref
```
or just open the linked file https://wiredforwar.github.io/flatpak/wiredforwar-machines.flatpakref with default Flatpak frontend.
Flatpak (the manager) installation howto: https://flatpak.org/setup

After the installation but *before* the first run, download the assets from https://lordovervolt.com/machines (e.g. http://markol.usermd.net/machines/machines-assets.7z) and extract them to `~/.var/app/io.github.wiredforwar.machines/data/`, so there are `sounds` and other assets inside the `~/.var/app/io.github.wiredforwar.machines/data/machines/data/machines` directory.
Personally I also use the "Enlarged textures" (http://markol.usermd.net/machines/machines-models.7z). Those should be extracted to the `machines` directory so the `models/planet` from the archive overwrite the files at `~/.var/app/io.github.wiredforwar.machines/data/machines/models/`.
Here is a short script doing that for you:
```
mkdir -p ~/.var/app/io.github.wiredforwar.machines/downloads
cd ~/.var/app/io.github.wiredforwar.machines/downloads
wget https://download.wiredforwar.org/Game/Community-Build-1.5/machines-assets.7z
7z x machines-assets.7z -o../data
```
Then you can run the game normally (e.g. via desktop menu). Flatpak command to run the game is:
```
flatpak run io.github.wiredforwar.machines
```
On an update released you can use your preferred Flatpak frontend or install the update via console:
```
flatpak update io.github.wiredforwar.machines
```
The logs are in `~/.var/app/io.github.wiredforwar.machines/logs`, and the save files are in `~/.var/app/io.github.wiredforwar.machines/savegame`.

### Development

#### Followed conventions
**The main code is spilt into a set of libraries:**
- afx - SDL app window creation, string resources handling,
- ani - smacker animations,
- base - compiler type things, new and delete overloaded global operators, leak and stack tracker (not used), persistence and logging mechanisms,
- ctl - charybdis template library - this was created to improve STL functionality, now it is a wrapper around gnu stl - note it uses public inheritance from std containers, found no problems yet? Only ctl containers support persistence,
- device - mice, keyboards, timers, (back when they did that stuff and didn't just rely on direct x), cd audio (music track),
- envirnmt - maps, sky loading code,
- gui - base classes for gui concepts (buttons, list, scrolling dialog windows and such like),
- gxin - parser for old .agt text format meshes,
- machgui - the real machines based UI,
- machlog - the logic stuff, AI, choosing levels and such like,
- machphys - the renderable bits of machines (agressors, administrators, mines, etc),
- mathex - maths libraries,
- network - low level communication layer,
- phys - base classes of some physics types things - this is NOT real time physics - by how to fiddle it,
- profiler - not used now,
- recorder - allows to record and replay all the  in game activity, events, random seeds etc. Game works in single thread, useful when reproducing a bug (not checked if it works),
- render - base library for rendering primitives, triangles, meshes, textures, materials, surfaces, etc,
- sim - base classes for things that can be simulated,
- sound - loading and playing wave samples 2D/3D,
- stdlib - now a little has left from it (there was cout, cerr, etc ),
- system - mostly file device handling, msgbox, metafile, registry (xml config),
- utility - helpers of various types, e.g. file parsers. again a lot of this stuff you wouldn't bother with now but back then there was no source forge or even google, 
- world4d - built on top of the render libraries, turned basic rendering into multi-mesh entities (W4dEntity is a base class for lot of things),
- xin - .X mesh file loading into game engine (added now),

#### Build dependencies
Use provided Docker image for building or manually setup your environment.

`libsdl2-dev libsdl2-image-dev libopenal-dev libalure-dev libenet-dev libfreetype6-dev libglew-dev libglm-dev libaudio-dev libswscale-dev`

####  Building
`git clone && git lfs pull`

##### For Windows on linux/docker
```
sudo apt-get install mingw-w64
mkdir buildMingw64 && cd buildMingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw64.cmake ..
```
or 32-bit cpu:
```
mkdir buildMingw32 && cd buildMingw32
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake -DBUILD_32=1 ..
```

Extra parameters for release optimizations: 
`-DCMAKE_BUILD_TYPE=RELEASE -DDEV_BUILD=OFF`

##### For GNU/Linux
+ `mkdir build && cd build`
+ `cmake ..`
+ `cmake --build . -j 9`

For ARM SoC development boards (like Raspberry or Nitrogen8M) if you do not have cross compilation toolchain prepared it will be easier to boot-up one of dedicated system distribution and build it within this enviroment (takes approx one hour for 4 cores).

##### Making a package
Run `make package` to collect all of the data files, graphics files, and compiled binaries into a single package file. You may then copy the contents of the package into your game's directory.

### Debugging

#### Class invariant checks
`export CB_INVARIANT_CHECKING=on`

#### Debug streams
To turn on a stream define environment variable with correct name, eg. `set CB_HAL_TO=cout` or in linux shell `export CB_HAL_TO=cout`.
Target cout is a current shell of course, replace it with filename to write log into or leave empty to turn off.

Avaiable streams: (*base/diag.hpp*)
- BOB
- HAL
- IAIN
- JERRY
- RICHARD
- JON
- YUEAI
- LIONEL
- CERI
- NEIL
- MISC
- WHERE
- TRACE
- MEMORY
- RENDER
- PER_READ
- PER_WRITE
- NETWORK
- WAYNE
- DANIEL
- MOTSEQ
- CS2PATH         // Config space pathfinding stuff (domain and normal)
- CS2VGRA         // Visibility graph diagnostics
- OBSERVER        // trapping observer pattern problems - stream added to due volume of output
- PATH_PROFILE    // Debugging the path profile across the planet surface
- SOUND
- REC_RECORD
- REC_PLAYBACK
- NET_ANALYSIS
- A_STAR
- LINEAR_TRAVEL      // Stream for use by PhysLinearTravelPlan
- PLANET_SURFACE     // Used for reporting errors in a planet surface
- PHYS_MOTION        // Physical motion stream
- NETWORK_ERRORS     // Specifically to help us spot network errors (such as loss of synchronicity)
- MEX_GRID2D
- CONFIG_SPACE
