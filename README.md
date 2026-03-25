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
Cd is looked up from a velocity-indexed table and linearly interpolated between the two nearest entries. The table covers the full velocity range from subsonic through supersonic, including the transonic drag spike around 350–450 m/s where wave drag appears. Currently hardcoded per projectile type; will be loaded from JSON catalog in Phase 3.

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

All forces are computed in a pure function `computeDerivatives(state, env, params, t)`. The integrator applies 4th-order Runge-Kutta (RK4) at each timestep. Euler integration is not used — RK4 is required for the accuracy the fire solution solver depends on.

All simulation state is kept in double precision (`dvec3`, `dquat`). Conversion to single-precision `vec3` happens once per frame when populating the GPU buffer.

---

## Features

### Simulation
- Full 3DOF point-mass ballistics with all real-world effects
- 6DOF rigid-body simulation with mesh-based aerodynamics (Phase 5)
- Multiple simultaneous trajectories with persistent path trails
- Per-round toggle between 3DOF and 6DOF modes for comparison
- Projectile deactivation on terrain contact

### Scenario system
- JSON-based projectile and launcher catalogs (`data/projectiles/`, `data/launchers/`)
- Real cartridge data: 7.62×51mm NATO, 12.7×99mm API, 40mm HE, 81mm mortar
- Multiple launcher types: M24 SWS, M2HB, M203, M252 mortar
- Place and orient multiple launchers in the 3D world
- Save and load scenarios to JSON

### Fire solution solver
- Click a target point on the terrain to enter fire solution mode
- Solver computes elevation, azimuth, and muzzle velocity for each placed launcher
- Two-stage approach: coarse grid sweep followed by Newton-Raphson refinement
- Displays time of flight, impact velocity, and solution error per launcher
- Fires the solved trajectory for visual verification

### Time-on-target (TOT)
- Given fire solutions for multiple launchers, compute staggered launch times
- All rounds arrive at the target simultaneously
- Execute TOT sequence: each launcher fires at its computed offset time
- Visual convergence of all trajectories on the target point

### Area of effect (AoE)
- Define a target area by center point and radius, or by drawn polygon
- Models blast and fragmentation radius per projectile type
- Greedy coverage optimizer: selects the most efficient launcher and projectile combination to cover the area
- Coverage heatmap overlay on terrain

### Terrain
- **Real-world terrain** loaded from SRTM HGT files (NASA Shuttle Radar Topography Mission)
- 90m resolution elevation data, free download, covers the entire Earth
- Subregion extraction: only the area relevant to the scenario is rendered — no LOD complexity needed
- Window center is computed as the midpoint of all launchers and targets; radius scales with the scenario's maximum range
- Auto-coarsening for very long range scenarios (MLRS-scale): samples every Nth point to keep triangle count reasonable
- Height queries for terrain contact detection always use full-resolution in-RAM data, independent of the rendered mesh
- **Procedural fallback** (Perlin noise) for testing without a real terrain file
- Slope-based terrain texturing

| Weapon class | Max range | Rendered window | Approx triangles |
|---|---|---|---|
| Rifle | ~1.5 km | 5×5 km | ~6k |
| Mortar | ~5.5 km | 12×12 km | ~35k |
| Howitzer (155mm) | ~25 km | 40×40 km | ~394k |
| MLRS rockets | ~70 km | 90×90 km (2× step) | ~500k |

### Rendering and UI
- Real-time 3D OpenGL rendering with orbit, pan, and zoom camera
- Dear ImGui panels for all parameters — all values adjustable at runtime
- Trajectory trail fade: older segments fade in alpha
- Launcher meshes rendered in the scene at their placed positions
- Ballistic table output: range, drop, drift, and velocity at configurable distance intervals
- CSV export of trajectory data per round

---

## Controls

| Input | Action |
|---|---|
| `Space` | Fire projectile from selected launcher |
| `Shift+Space` | Fire without drag (vacuum trajectory, for comparison) |
| `Left mouse drag` | Orbit camera |
| `Right mouse drag` | Pan camera |
| `Scroll wheel` | Zoom |
| `Left click` (fire solution mode) | Place target marker on terrain |
| `C` | Clear oldest trajectory |
| `R` | Reset all trajectories |

---

## Project structure

```
Ballistics3D/
├── CMakeLists.txt
├── data/
│   ├── projectiles/         # JSON projectile definitions
│   ├── launchers/           # JSON launcher definitions
│   ├── scenarios/           # Saved scenario files
│   ├── meshes/              # 3D projectile models (Phase 5)
│   └── shaders/             # GLSL vertex and fragment shaders
└── src/
    ├── main.cpp
    ├── app/
    │   ├── Application.hpp/.cpp          # Main loop, subsystem ownership
    │   ├── ScenarioController.hpp/.cpp   # Mediates input/UI events -> Scenario
    │   └── AppState.hpp                  # Mode enum: SIMULATE, FIRE_SOLUTION, AOE, TOT
    ├── simulation/
    │   ├── core/
    │   │   ├── RigidBodyState.hpp        # dvec3 pos/vel/angVel + dquat orientation
    │   │   └── Integrator.hpp/.cpp       # step() RK4 + simulateSteps() trajectory loop
    │   ├── physics/
    │   │   ├── PhysicsConstants.hpp      # g, ISA constants, Earth omega
    │   │   └── BallisticsModel.hpp/.cpp  # derivative() forces, hasImpacted() stop condition
    │   ├── launchers/
    │   │   └── Launcher.hpp/.cpp         # Position, angles, muzzle speed — fire() -> initial state
    │   ├── projectiles/
    │   │   └── Projectile.hpp/.cpp       # Mass, diameter, drag coefficient
    │   ├── environment/
    │   │   ├── Terrain.hpp               # Abstract interface: heightAt(), extent()
    │   │   ├── ProceduralTerrain.hpp/.cpp # Perlin noise terrain backend
    │   │   └── wind.hpp/.cpp             # 3D wind with Perlin noise gusts
    │   └── solvers/                      # Phase 4+
    │       ├── FireSolutionSolver.hpp/.cpp
    │       ├── TOTSolver.hpp/.cpp
    │       └── AoECalculator.hpp/.cpp
    ├── renderer/
    │   ├── core/
    │   │   ├── GLContext.hpp/.cpp        # SDL3 + GLAD init, OpenGL context, swap
    │   │   ├── ShaderProgram.hpp/.cpp
    │   │   ├── VertexBuffer.hpp/.cpp     # VAO/VBO RAII wrappers
    │   │   └── Camera.hpp/.cpp           # Orbit/pan/zoom, view + projection matrices
    │   ├── passes/
    │   │   ├── TerrainPass.hpp/.cpp
    │   │   ├── TrajectoryPass.hpp/.cpp   # Path lines and active projectile dots
    │   │   ├── LauncherPass.hpp/.cpp
    │   │   └── MeshPass.hpp/.cpp         # Assimp-loaded projectile meshes (Phase 5)
    │   ├── Renderer.hpp/.cpp             # Owns all passes, drives frame
    │   └── RenderData.hpp                # POD structs: double->float conversion point
    ├── ui/
    │   ├── UIManager.hpp/.cpp
    │   └── panels/
    │       ├── EnvironmentPanel.hpp/.cpp
    │       ├── ProjectilePanel.hpp/.cpp
    │       ├── LauncherPanel.hpp/.cpp
    │       ├── ScenarioPanel.hpp/.cpp
    │       ├── FireSolutionPanel.hpp/.cpp
    │       ├── TOTPanel.hpp/.cpp
    │       └── AoEPanel.hpp/.cpp
    ├── input/
    │   ├── InputHandler.hpp/.cpp
    │   └── InputState.hpp
    └── util/
        ├── math/
        │   ├── Angles.hpp               # deg/rad, azimuth/elevation helpers
        │   ├── CoordTransform.hpp       # World<->screen, ray-terrain intersection
        │   └── Interpolation.hpp        # Linear interpolation for Cd tables
        ├── io/
        │   ├── JsonLoader.hpp/.cpp
        │   └── CsvExporter.hpp/.cpp
        └── debug/
            └── Logger.hpp
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
- [ ] Dear ImGui panels for all parameters
- [ ] Multiple simultaneous trajectories, ballistic table output

**Phase 3 — Catalog, scenario system, and real terrain**
- [ ] JSON projectile and launcher catalogs
- [ ] LauncherInstance placement in 3D scene
- [ ] Scenario container with save/load
- [ ] SRTM HGT terrain loader: parse tile, extract subregion around scenario, auto-coarsen for large ranges
- [ ] Terrain backend selection: real SRTM or procedural Perlin fallback
- [ ] CSV export of trajectory data

**Phase 4 — Fire solution solver and time-on-target**
- [ ] Ray-terrain intersection for target placement
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

**Real terrain data (Phase 3+):**

Download a 1°×1° SRTM tile in HGT format for your area of interest from [USGS EarthExplorer](https://earthexplorer.usgs.gov) or [OpenTopography](https://opentopography.org) and place it in `data/terrain/`. The filename encodes the tile coordinates (e.g. `N47E013.hgt` for 47°N 13°E). If no HGT file is found, the simulator falls back to procedural Perlin terrain automatically.

---

## Architecture notes

**Simulation never touches the renderer.**
Nothing under `src/simulation/` includes anything from `src/renderer/` or `src/ui/`. The application layer owns both and performs the translation. This is what makes the fire solution solver work: it calls the integrator thousands of times per solution search without any rendering side effects.

**The integrator is a pure function.**
`integrateRK4(state, env, params, t, dt) -> RigidBodyState` — no member state, no mutation. The solver depends on this.

**Double precision for physics, float for rendering.**
All simulation state uses `dvec3` and `dquat`. Conversion to `vec3` happens exactly once per frame in `Application::buildRenderData()`. Coriolis and spin drift effects are small enough that float accumulation errors would obscure them.

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

The following features were written by Claude as the initial foundation. All future modifications and extensions are my own work.

- Build system and dependency setup
- SDL3 window and OpenGL 3.3 Core context initialisation
- Orbit camera (pan, zoom, rotate)
- GLSL shader loading and compilation
- Procedural Perlin noise terrain mesh
- SDL3 input handling
- TrajectoryPass — GPU upload and line strip rendering of trajectory points

---

## Notes

This project is a direct continuation of [Ballistics-CPP](https://github.com/jaroj/Ballistics-CPP).
The 2D version established the physics foundation — this version extends it into 3D and adds fire control tools that require the third dimension to be meaningful.
