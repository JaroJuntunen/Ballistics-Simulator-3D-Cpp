# Ballistic Trajectory Simulator — 3D

A real-time 3D ballistic trajectory simulator written in C++17, rendered with OpenGL.
Simulates projectile flight using a full 3D physics model — gravity, aerodynamic drag, altitude-varying air density, 3D wind, Coriolis effect, and gyroscopic spin drift.

A follow-up to [Ballistics-CPP](https://github.com/jaroj/Ballistics-CPP), extending the 2D physics model into three dimensions where effects like Coriolis deflection and spin drift become visible and meaningful.

Built to deepen practical C++ skills and explore the physics of real long-range ballistic systems.

---

## Why 3D matters for ballistics

In a 2D side-view simulator, the most significant real-world effects are invisible:

- **Coriolis deflection** acts laterally — perpendicular to the firing plane. At 1000 m, a 7.62×51mm round deflects roughly 100 mm sideways depending on latitude and firing direction. Invisible in 2D, fully demonstrable in 3D.
- **Spin drift** causes a slow lateral walk in the direction of rifling twist over long distances. Requires a 3D trajectory to observe.
- **Crosswind** has a distinct effect from headwind/tailwind — separating these requires a 3D velocity model.

---

## Physics model

**Gravity**
```
a = (0, 0, -g)        g = 9.81 m/s²
```

**Aerodynamic drag**
```
Fd = 0.5 * Cd * rho * A * v²
```
Drag acts opposite to the velocity vector relative to wind:
```
vRel = v_projectile - v_wind
Fd_vec = -0.5 * Cd * rho * A * |vRel| * vRel
```

**Altitude-varying air density (barometric formula)**
```
rho(h) = 1.225 * exp(-h / 8500)
```

**Coriolis effect**
```
a_coriolis = -2 * (omega × v)
```
Where `omega` is Earth's rotation vector projected at the shooter's latitude.
Deflects the projectile to the right in the northern hemisphere (for a northward shot).

**Spin drift**
Gyroscopic precession causes slow lateral drift in the direction of rifling twist.
Modelled as a small lateral acceleration proportional to yaw rate and spin.

**Wind**
Full 3D wind vector with Perlin noise gusts:
```
windVelocity(t) = baseWind + perlinGust(t)
```

---

## Planned features

- Real-time 3D trajectory rendering with OpenGL
- Free camera (orbit, pan, zoom) for viewing trajectories from any angle
- Multiple simultaneous trajectories for direct comparison
- Runtime parameter adjustment via Dear ImGui
  - Muzzle velocity, mass, drag coefficient, calibre
  - Latitude, azimuth (firing direction)
  - Wind speed and direction
  - Coriolis and spin drift toggles
- Procedurally generated terrain
- Trajectory path tracing — trail persists after landing
- Ballistic table output (range, drop, drift at configurable distance steps)

---

## Planned controls

| Input | Action |
|---|---|
| `Space` | Fire projectile |
| `Shift+Space` | Fire without drag (for comparison) |
| `Left mouse drag` | Orbit camera |
| `Right mouse drag` | Pan camera |
| `Scroll wheel` | Zoom |
| `C` | Clear oldest projectile |
| `R` | Reset all projectiles |

---

## Planned project structure

```
Ballistics3D/
├── CMakeLists.txt
└── src/
    ├── main.cpp                        # Game loop, input dispatch, frame timing
    ├── simulation/
    │   ├── Environment.hpp/.cpp        # Wind, air density, Coriolis parameters
    │   └── Projectile.hpp/.cpp         # Projectile state and 3D physics step
    ├── renderer/
    │   └── Renderer.hpp/.cpp           # OpenGL rendering, camera, ImGui
    ├── input/
    │   └── InputHandler.hpp/.cpp       # SDL event handling, input state
    └── util/
        ├── math/
        │   └── Vec3.hpp                # 3D vector math
        └── noise/
            └── Perlin.hpp              # Perlin noise for wind/terrain
```

---

## Roadmap

**Stage 1 — 3D rendering foundation**
- [ ] OpenGL window with SDL3
- [ ] Orbit camera with pan and zoom
- [ ] Basic terrain mesh
- [ ] Fire projectile under gravity, render trajectory

**Stage 2 — Full 3D physics**
- [ ] Aerodynamic drag with 3D relative airspeed
- [ ] Altitude-varying air density
- [ ] 3D wind with Perlin noise gusts
- [ ] Coriolis effect (latitude input, correct deflection direction)
- [ ] Spin drift

**Stage 3 — User control**
- [ ] Dear ImGui parameter panel
- [ ] Coriolis and spin drift toggles for comparison
- [ ] Adjustable latitude, azimuth, wind vector
- [ ] Multiple simultaneous trajectories

**Stage 4 — Output and analysis**
- [ ] Ballistic table (drop and drift at set range intervals)
- [ ] Export trajectory data to CSV

---

## Dependencies

- C++17 or later
- OpenGL 3.3+
- SDL3 — window and input (via CMake FetchContent)
- GLM — 3D math (via CMake FetchContent)
- Dear ImGui — runtime parameter UI (via CMake FetchContent)
- stb_perlin — Perlin noise for wind and terrain
- CMake 3.15+

---

## Building

```bash
cmake -S . -B build
cmake --build build
```

---

## Notes

This project is a direct continuation of [Ballistics-CPP](https://github.com/jaroj/Ballistics-CPP).
The 2D version established the physics foundation and code structure — this version extends it into 3D where the full ballistic model becomes demonstrable.
