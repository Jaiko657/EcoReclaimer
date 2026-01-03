# final-year-project

Final year project: a small 2D tile-based game built on a custom C99 engine (ECS + custom 2D collision/physics-lite) with Raylib used as the platform/renderer backend.

## Quick start (Linux)

### Prereqs

- C compiler: `gcc`
- Build tools: `git`, `cmake`, `make`
- X11/OpenGL dev packages (needed by Raylib/GLFW)

Ubuntu/Debian example:

```bash
sudo apt install build-essential cmake git \
  libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev \
  libxcursor-dev libxinerama-dev libxxf86vm-dev libasound2-dev
```

### Build third-party deps (once)

```bash
cd third_party
cc -O2 -o nob_deps dependencies_build.c
./nob_deps
cd ..
```

This initializes `git` submodules, applies local patches, and builds:
- `third_party/raylib` (static library, windows version downloads libs from release as building raylib on windows is a pain, not primary build target so i dont mind)
- `third_party/xml.c`
- `third_party/unity`

### Build + run

```bash
# Build tool (only needed if ./nob isn't already present)
cc -O2 -o nob src/build.c

# Build the game (default is release)
./nob

# runs game
./build/src/game
```

### Headless build (no window)

Useful for quick simulation/CI-style runs.

```bash
./nob --headless
HEADLESS_MAX_FRAMES=600 ./build/src/game_headless
```

Build flags:
- `--debug` enables extra debug toggles/overlays
- `--release` forces release flags
- `--headless` builds `build/src/game_headless`

### Unit tests

```bash
# Build the test helper
cc -O2 -o nob_tests tests/build_tests.c

# Build the unit suite into `build/tests`
./nob_tests

# Build + run the unit suite
./nob_tests --run

# (Optional) Build + run with coverage output under `build/tests/coverage`
./nob_tests --coverage
```

## Controls

- Move: WASD / arrow keys
- Interact: `E`
- Lift/throw: `C`
#### Debug controls
- Screenshot: `Print Screen` (saves to `./screenshots/` as `screenshot_#####.png`)
- Debug build extras:
  - `Space` reloads assets and prints asset debug
  - `1/2/3` toggle collider overlays
  - `4` toggle trigger overlay
  - `5` toggle inspect mode (click entity to log components)
  - `R` reloads current TMX map
  - `` ` `` toggles FPS overlay

## Game overview

- Player spawns in abandoned factory, picks up a gravity gun getting ability to lift heavy things.
- Player collects the raw materials on the ground (metal and plastic) he can then store them in TARDAS device.
- Doors open/close based on proximity using Tiled tile animations.
- When Gravity gun runs out of energy it can be charged.
- When player drags TARDAS with resources in it, they are put out of unloader, and player sorts them into recyclers.

Content is authored in Tiled (`assets/maps/*.tmx`) and entity prefabs (`assets/prefabs/*.ent`).

## Engine design (high level)

- Fixed timestep simulation (60Hz) with variable render framerate. (game_ticks vs present ticks)
- ECS with SoA component storage and phase-based system scheduling (`PHASE_INPUT`, `PHASE_PHYSICS`, `PHASE_SIM_*`, `PHASE_PRESENT`, `PHASE_RENDER`).
- Collision/physics-lite: kinematic intent velocities + tile collision and simple entity pushing. (custom code)
- Tile/world pipeline:
  - TMX parsing + tilesets via a small XML loader (`xml.c`) and a custom Tiled module.
  - Collision built from per-tile 4x4 subtile bitmasks; static colliders merged, with “dynamic” tiles (e.g. doors) kept separate.
- Rendering + input via Raylib (kept behind engine modules so it can be swapped; there is also a headless backend).

## Portability notes

The core engine/game code is C99 with a small set of “platform edges” (window/input, timing, filesystem). C is a good fit here because it compiles to native code across most targets, has stable toolchains, and keeps dependencies explicit.

In practice, “runs anywhere with a C compiler” means:
- the ECS/gameplay/physics logic is largely portable as-is
- the platform layer may need tweaks per target (windowing backend, filesystem paths, build flags)

## Third-party

- Raylib (rendering/window/input) has multiple backends making it portable to other systems such as N64,PSP etc. But also goal is to be loosely coupled to enable custom render code for platforms without raylib.
- xml.c (TMX parsing helper)
- Unity a small C testing framework, only used in tests.
- nob.h small header file used to have my build system also be in C. allowing cross platform win/linux scripts while getting to write more C!!!
