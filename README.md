# Ballistic Trajectory Simulator — 3D

A real-time 3D ballistic trajectory simulator written in C++17, rendered with OpenGL 3.3 Core.

Simulates projectile flight using a full physics model — gravity, aerodynamic drag, altitude-varying air density, 3D wind with Perlin noise gusts, Coriolis effect, and gyroscopic spin drift. Designed to be extended into an advanced fire control and analysis tool with a reverse fire solution solver, time-on-target synchronization, area of effect calculations, and 6DOF mesh-based aerodynamics.

A follow-up to [Ballistics-CPP](https://github.com/jaroj/Ballistics-CPP), extending the 2D physics model into three dimensions where lateral effects become visible and meaningful — and pushing the simulation further into territory that a 2D view cannot demonstrate.

Built to deepen practical C++ skills and explore the physics of real long-range ballistic systems.

---

## Why 3D matters for ballistics

In a 2D side-view simulator, the most significant real-world effects are invisible:

- **Coriolis deflection** acts laterally — perpendicular to the firing plane. At 1000 m, a 7.62×51mm round deflects roughly 100 mm sideways depending on latitude and firing direction. Invisible in 2D, fully demonstrable in 3D.
- **Spin drift** causes a slow lateral walk in the direction of rifling twist over long distances. Requires a 3D trajectory to observe.
- **Crosswind** has a distinct effect from headwind and tailwind — separating these requires a 3D velocity model.
- **Fire solutions** for multiple launchers targeting the same point, and time-on-target synchronization, only make sense in a 3D world.

---

## Physics model

**Gravity**
```
a = (0, 0, -g)        g = 9.81 m/s²
```

**Aerodynamic drag (3DOF)**
```
Fd = 0.5 * Cd * rho * A * v²
```
Drag acts opposite to the velocity vector relative to wind:
```
vRel    = v_projectile - v_wind
Fd_vec  = -0.5 * Cd * rho * A * |vRel| * vRel
```
Cd is looked up from a velocity-indexed table and linearly interpolated between the two nearest entries. The table covers the full velocity range from subsonic through supersonic, including the transonic drag spike around 350–450 m/s where wave drag appears. Loaded from per-projectile JSON files in `data/projectiles/`.

**Aerodynamic drag (6DOF, Phase 5)**

When 6DOF mode is enabled, a 3D projectile mesh is loaded via Assimp. Drag is computed from the angle of attack (alpha) and sideslip angle (beta) using axial and normal force coefficients, giving realistic asymmetric drag under crosswind and yaw.

**Altitude-varying air density (ISA barometric formula)**
```
rho(h) = 1.225 * exp(-h / 8500)
```

**Coriolis effect**
```
a_coriolis = -2 * (omega × v)
```
Where `omega` is Earth's rotation vector projected at the shooter's latitude. Deflects the projectile to the right in the northern hemisphere (for a northward shot).

**Spin drift**

Gyroscopic precession causes slow lateral drift in the direction of rifling twist. Modelled as a small lateral acceleration proportional to yaw rate and spin magnitude, parameterised per projectile type.

**Wind**

Full 3D wind vector with Perlin noise gusts:
```
windVelocity(t) = baseWind + perlinGust(t)
```

**Numerical integration**

All forces are computed in a pure function `BallisticsModel::derivative()`. The integrator applies 4th-order Runge-Kutta (RK4) at each timestep. Euler integration is not used — RK4 is required for the accuracy the fire solution solver depends on.

All simulation state is kept in double precision (`dvec3`, `dquat`). Conversion to single-precision `vec3` happens once per frame when populating the GPU buffer.

---

## Features

### Simulation
- Full 3DOF point-mass ballistics: gravity, drag, Coriolis, spin drift, ISA air density
- Multiple simultaneous trajectories — instant (full RK4 in one frame) or real-time stepping modes
- Per-trajectory color cycling; persistent path trails until cleared
- Projectile deactivation on terrain contact

### Catalog and scenario system
- JSON-based projectile and launcher catalogs (`data/projectiles/`, `data/launchers/`)
- Launchers: M109 Paladin (155mm), M252 81mm Mortar
- Projectiles: 155mm HE, 155mm APHE, 81mm HE M821, 81mm Smoke M375, 81mm Illumination M853
- Compatible projectile list per launcher with per-projectile muzzle velocity
- Launcher and projectile selection from ImGui dropdowns; parameters shown and editable at runtime
- Launcher placement in 3D scene: press `M` to move launcher to cursor position via ray-terrain intersection
- Scenario save/load to JSON (`data/scenarios/`) — persists terrain tile, launcher position/angles, projectile, and wind

### Terrain
- **Real-world terrain** loaded from SRTM3 HGT files (NASA Shuttle Radar Topography Mission, ~90m resolution)
- Tile parsed from binary big-endian `int16_t` samples; bilinear interpolation between the four surrounding grid points
- Non-square tiles correctly handled: east-west extent shrinks with latitude via `cos(lat)` factor
- Height queries for terrain contact detection use full-resolution in-RAM data
- Renderer samples the terrain at uniform world-space intervals; spacing derived from tile extent at load time
- Runtime terrain switching from ImGui — swap tiles without restarting; launcher height auto-adjusted to new surface
- **Procedural fallback** (Perlin noise) used when no HGT file is loaded; selectable from the same dropdown
- HGT files are discovered automatically from `data/terrain/` (recursive scan, subdirectories supported)
- Tile info shown in UI: origin lat/lon, width and height in km
- Coriolis latitude auto-derived from launcher's world-space position within the loaded tile

### Rendering and UI
- Real-time 3D OpenGL rendering with orbit, pan, and zoom camera
- Dear ImGui panels for all parameters — all values adjustable at runtime
- Ballistic table output: range, height, drift, speed, and time of flight at 500m intervals; scrollable ImGui table
- CSV export with configurable separator (comma, semicolon, tab); auto-numbered files saved to `Exports/`

---

## Controls

| Input | Action |
|---|---|
| `Space` | Fire projectile from selected launcher |
| `Left mouse drag` | Orbit camera |
| `Right mouse drag` | Pan camera |
| `Scroll wheel` | Zoom |
| `M` | Move launcher to cursor position on terrain |

---

## Project structure

```
Ballistics3D/
├── CMakeLists.txt
├── data/
│   ├── projectiles/         # JSON projectile definitions
│   ├── launchers/           # JSON launcher definitions
│   ├── scenarios/           # Saved scenario files
│   ├── terrain/             # SRTM HGT elevation tiles (user-provided)
│   └── shaders/             # GLSL vertex and fragment shaders
└── src/
    ├── main.cpp
    ├── app/
    │   ├── Application.hpp               # Class declaration, all member data
    │   ├── Application.cpp               # Main loop, input, simulation stepping, terrain switching, ray casting
    │   ├── Application_Parsers.cpp       # JSON loaders, scenario save/load, CSV export
    │   └── Application_GUI.cpp           # ImGui panel rendering
    ├── simulation/
    │   ├── core/
    │   │   ├── RigidBodyState.hpp        # dvec3 pos/vel/angVel + dquat orientation
    │   │   └── Integrator.hpp/.cpp       # step() RK4 + simulateSteps(); Trajectory typedef
    │   ├── physics/
    │   │   ├── PhysicsConstants.hpp      # g, ISA constants, Earth omega
    │   │   └── BallisticsModel.hpp/.cpp  # derivative() forces, hasImpacted() stop condition
    │   ├── launchers/
    │   │   └── Launcher.hpp/.cpp         # Position, angles, muzzle speed — fire() -> initial state
    │   ├── projectiles/
    │   │   └── Projectile.hpp/.cpp       # Mass, diameter, velocity-indexed Cd table; DragTable typedef
    │   └── environment/
    │       ├── Terrain.hpp               # Abstract interface: heightAt(), width(), height()
    │       ├── ProceduralTerrain.hpp/.cpp # Perlin noise terrain backend
    │       ├── SRTMTerrain.hpp/.cpp      # SRTM HGT loader: binary parse, bilinear interp, lat/lon extents
    │       └── wind.hpp/.cpp             # 3D wind with Perlin noise gusts
    ├── renderer/
    │   ├── core/
    │   │   ├── GLContext.hpp/.cpp        # SDL3 + GLAD init, OpenGL context, swap
    │   │   ├── ShaderProgram.hpp/.cpp    # GLSL load, compile, uniform setters
    │   │   └── Camera.hpp/.cpp           # Orbit/pan/zoom, view + projection matrices
    │   ├── passes/
    │   │   ├── TerrainPass.hpp/.cpp      # Terrain mesh upload and wireframe render
    │   │   └── TrajectoryPass.hpp/.cpp   # Multiple trajectory buffers, per-trajectory color cycling
    │   └── Renderer.hpp/.cpp             # Owns all passes, drives frame
    └── input/
        ├── InputHandler.hpp/.cpp
        └── InputState.hpp
```

---

## Roadmap

**Phase 1 — 3D rendering foundation**
- [x] CMakeLists.txt with all dependencies
- [x] SDL3 window + OpenGL 3.3 Core context via GLAD
- [x] Orbit camera with pan and zoom
- [x] Procedurally generated Perlin terrain mesh
- [x] Fire projectile under gravity, render trajectory as line strip

**Phase 2 — Full 3DOF physics**
- [x] RK4 integrator with all force components
- [x] Aerodynamic drag with Cd table interpolation
- [x] ISA altitude-varying air density
- [x] 3D wind field with Perlin noise gusts
- [x] Coriolis effect (latitude input, correct hemisphere deflection)
- [x] Spin drift
- [x] Dear ImGui panels for all parameters
- [x] Multiple simultaneous trajectories (instant and real-time stepping modes)
- [x] Ballistic table output (range, height, drift, speed, TOF sampled at 500m intervals)

**Phase 3 — Catalog, scenario system, and real terrain**
- [x] JSON projectile and launcher catalogs
- [x] Launcher and projectile selection from JSON catalog via ImGui dropdowns
- [x] SRTM HGT terrain loader: binary parse, big-endian swap, bilinear interpolation, correct lat/lon extents
- [x] Terrain backend selection: switch between real SRTM tiles and procedural Perlin fallback at runtime
- [x] Launcher placement in 3D scene: ray-terrain intersection from mouse cursor, `M` to place
- [x] Scenario container: save/load to JSON (terrain, launcher position/angles, projectile, wind)
- [x] CSV export of trajectory data (configurable separator, auto-numbered files in Exports/)

**Phase 4 — Fire solution solver and time-on-target**
- [ ] Target placement on terrain for fire solution mode (click to place target marker)
- [ ] FireSolutionSolver: coarse sweep + Newton-Raphson refinement
- [ ] Fire solution results panel per launcher
- [ ] TOTSolver: compute staggered launch times
- [ ] TOT execution sequence with countdown

**Phase 5 — 6DOF physics and mesh-based aerodynamics**
- [ ] Assimp mesh loading for projectile geometry
- [ ] Quaternion orientation integration (RK4 on rotation)
- [ ] AeroDrag6DOF: drag from angle of attack and sideslip
- [ ] Gyroscopic stability computation (Miller formula)
- [ ] 3DOF vs 6DOF comparison mode per round

**Phase 6 — Area of effect and polish**
- [ ] Blast and fragmentation radius model per projectile
- [ ] AoECalculator: grid-sampled coverage with greedy optimizer
- [ ] Coverage heatmap overlay on terrain
- [ ] Terrain window repositioning: pan scenario center to a new lat/lon, mesh rebuilds
- [ ] Terrain slope-based texturing
- [ ] Trajectory trail alpha fade
- [ ] ImPlot velocity/drop curves (optional)

---

## Dependencies

| Library | Purpose | How |
|---|---|---|
| C++17 | Language standard | — |
| OpenGL 3.3 Core | Rendering | System |
| SDL3 | Window, input, GL context | CMake FetchContent |
| GLM | 3D math (`dvec3`, `dquat`, `mat4`) | CMake FetchContent |
| GLAD2 | OpenGL function loader | CMake FetchContent (generated at configure time) |
| Dear ImGui | Runtime parameter UI | CMake FetchContent |
| stb_perlin | Perlin noise for wind and terrain | CMake FetchContent |
| nlohmann/json | Projectile/launcher catalog, scenario I/O | CMake FetchContent |
| SRTM HGT data | Real-world terrain elevation tiles | Free download (USGS EarthExplorer / OpenTopography), placed in `data/terrain/` |
| Assimp | 3D mesh loading for projectile geometry | CMake FetchContent (Phase 5, `-DENABLE_6DOF=ON`) |
| ImPlot | Data curves inside ImGui | CMake FetchContent (Phase 6, optional) |

---

## Building

**Prerequisites (one-time):**

```bash
sudo apt-get install -y libgl-dev
```

CMake will check for this and print a clear error with the install command if it is missing.

**Build:**

```bash
cmake -S . -B build
cmake --build build -- -j$(nproc)
./build/BallisticsSimulator3D
```

With 6DOF physics enabled (Phase 5+):

```bash
cmake -S . -B build -DENABLE_6DOF=ON
cmake --build build -- -j$(nproc)
```

If you change CMake options or dependencies, delete the build directory first:

```bash
rm -rf build
```

**Real terrain data:**

Download 1°×1° SRTM3 tiles in HGT format from [viewfinderpanoramas.org](http://viewfinderpanoramas.org/dem3.html) (easiest, no account required) or [USGS EarthExplorer](https://earthexplorer.usgs.gov). Place the `.hgt` files anywhere under `data/terrain/` — subdirectories are supported. The filename encodes the tile's SW corner (e.g. `N60E025.hgt` = 60–61°N, 25–26°E).

Select the tile at runtime from the **Terrain** panel in the ImGui window. If no tile is loaded, the simulator uses procedural Perlin terrain automatically.

---

## Architecture notes

**Simulation never touches the renderer.**
Nothing under `src/simulation/` includes anything from `src/renderer/`. The application layer owns both and performs the translation. This is what makes the fire solution solver work: it calls the integrator thousands of times per solution search without any rendering side effects.

**The integrator is a pure function.**
`Integrator::step()` and `Integrator::simulateSteps()` — no member state, no mutation. The solver depends on this.

**Double precision for physics, float for rendering.**
All simulation state uses `dvec3` and `dquat`. Conversion to `vec3` happens once per frame when uploading trajectory points to the GPU. Coriolis and spin drift effects are small enough that float accumulation errors would obscure them.

**6DOF state from day one.**
`RigidBodyState` contains orientation and angular velocity from the start, even when they are unused in early phases. This avoids retrofitting the solver, catalog, and serialization systems when 6DOF is added.

---

## AI Assistance

This project was built collaboratively with [Claude](https://claude.ai) (Anthropic).

The goal is to learn C++ and systems programming deeply, with a focus on applied physics and simulation. The 3D rendering layer is intentionally delegated to Claude so I can focus on the parts I want to learn. Using AI as a tool to accelerate that — rather than to replace the thinking — is intentional.

**Claude's contributions:**
- Wrote the initial code based on my specifications and decisions
- Explained every component so I understood it before it was accepted
- Suggested technical approaches when I asked, which I then approved or rejected
- Keeps the README up to date

**Claude-generated features:**

The following features were written primarily by Claude. Physics implementation, architectural decisions, and all JSON data files are my own work.

- Build system and dependency setup
- SDL3 window and OpenGL 3.3 Core context initialisation
- Orbit camera (pan, zoom, rotate)
- GLSL shader loading and compilation
- Procedural Perlin noise terrain mesh
- SDL3 input handling
- TrajectoryPass — GPU upload and line strip rendering of trajectory points
- Scenario save/load — JSON serialization of full simulator state (I didn't want to spend time building just another json parser/writer functions)

---

## Notes

This project is a direct continuation of [Ballistics-CPP](https://github.com/jaroj/Ballistics-CPP).
The 2D version established the physics foundation — this version extends it into 3D and adds fire control tools that require the third dimension to be meaningful.
