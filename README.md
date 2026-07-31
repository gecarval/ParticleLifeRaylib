# ParticleEngineRaylib

A Godot-inspired 2D game engine built on top of [Raylib](https://www.raylib.com/) 5.5/6.0 (with a custom-merged C++ wrapper for retro-compatibility). Node-based scene graph, three singleton servers for rendering / physics / gravity, and an **O(n log n) Fast Multipole Method** gravity solver that simulates thousands of bodies in real time.

> **📖 Full Interactive Documentation** — code reference, math, and 6 live canvas demos:
>
> **[Open the Documentation →](https://pro-2684.github.io/GitHub-Preview/?url=https%3A%2F%2Fgithub.com%2Fgecarval%2FParticleEngineRaylib%2Fblob%2Fmaster%2FDOCUMENTATION.html&preview=1#gecarval/ParticleEngineRaylib/master/DOCUMENTATION.html)**

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Architecture at a Glance](#architecture-at-a-glance)
- [The Three Singleton Servers](#the-three-singleton-servers)
- [Class Hierarchy](#class-hierarchy)
- [Getting Started](#getting-started)
- [Quick Start](#quick-start)
- [The Main Loop](#the-main-loop)
- [GravityServer — Fast Multipole Method](#gravityserver--fast-multipole-method)
- [Project Layout](#project-layout)
- [Building](#building)
- [Controls](#controls)
- [Interactive Demos](#interactive-demos)
- [API Cookbook](#api-cookbook)
- [License](#license)

---

## Overview

This engine was built as the substrate for a particle-life simulator: thousands of gravitating bodies, colliding with each other, rendered at 60 FPS on commodity hardware. The headline feature is a faithful 2D implementation of the **Fast Multipole Method (FMM)** — the same algorithm Greengard & Rokhlin introduced in their 1987 paper — which reduces N-body gravity from the naïve `O(n²)` to `O(n log n)`. At `n = 2000` that's roughly a 300× speedup, turning an unusable 4-million-pairwise-tests-per-frame workload into a smooth real-time simulation.

Around the FMM core, the engine provides a Godot-style node tree (parent ownership, delta-propagating transforms), auto-registration with three singleton servers (no manual "register" calls), an impulse-based rigid-body collision solver with rotational terms, and a uniform spatial hash grid for `O(1)` "who is near me?" queries.

---

## Key Features

- **Node-based scene graph** with parent ownership and delta-propagating 2D transforms.
- **Three singleton servers** — `RenderServer`, `PhysicsServer`, `GravityServer` — auto-discover nodes by construction, no manual registration.
- **Fast Multipole Method gravity solver** — `O(n log n)` N-body gravity via adaptive quadtree + truncated multipole/local expansions on the complex plane. 2000+ bodies at 60 FPS.
- **Spatial hash grid** for collision broadphase — `O(1)` amortized 3×3 neighborhood queries.
- **Impulse-based collision solver** with full rotational terms (lever arms, moments of inertia) for circle-circle, circle-rect, and rect-rect pairs.
- **Fluent setter API** — every setter returns `*this` for easy configuration chaining.
- **Single dependency**: Raylib (the merged 5.5/6.0 C++ wrapper is shipped in `include/`).

---

## Architecture at a Glance

The engine is organized around three orthogonal concerns:

| Concern | Owned by | Examples |
|---|---|---|
| **Identity & ownership** | `Object` → `Node` tree | `Object`, `Node`, `CanvasItem`, `Node2D` |
| **Spatial representation** | `Shape2D` hierarchy | `Shape2D`, `CircleShape2D`, `RectangleShape2D` |
| **Global services** | Singleton servers | `RenderServer`, `PhysicsServer`, `GravityServer` |

The defining pattern is **constructor side-effects**: every `CanvasItem` auto-registers with `RenderServer`, every `CollisionObject2D` auto-registers with `PhysicsServer`, every `PhysicsBody2D` auto-registers with `GravityServer`. The practical consequence is that **creating a node is sufficient to make it part of the engine's update loops** — there is no separate "register" or "spawn" call to forget.

```
new Particle("P0", pos, color);
// → Particle is now: visible (RenderServer), collidable (PhysicsServer), gravitationally active (GravityServer)
```

---

## The Three Singleton Servers

### RenderServer
Singleton. Maintains a static map of every `CanvasItem`. Each frame, `render(window, camera)` iterates the map, culls off-screen items via `isInView(...)`, and calls `draw()` / `drawDebug()` on each visible one.

### PhysicsServer
Singleton. Uniform spatial hash grid with `CELL_SIZE = 10`. `rebuild()` re-buckets every `CollisionObject2D` by `floor(pos / 10)`. `getCollisions(query, out)` returns objects in the 3×3 neighborhood of the query — `O(1)` amortized.

### GravityServer
Singleton. The crown jewel. Builds an adaptive quadtree from every `PhysicsBody2D`, computes per-cell multipole expansions (P2M + M2M), then runs a dual-tree traversal that exchanges multipole-to-local translations for well-separated cell pairs (M2L), propagates local expansions down (L2L), does direct P2P summation for nearby pairs, and evaluates the local expansion at each body (L2P). Result: `O(n log n)` N-body gravity.

---

## Class Hierarchy

```
Object                                    ← universal base, hands out _instanceID
├── Node                                  ← scene-graph primitive, owns children
│   └── CanvasItem                        ← auto-regs with RenderServer
│       ├── Node2D                        ← 2D transform, delta-propagating
│       │   ├── Sprite2D                  ← texture or procedural shape
│       │   ├── CollisionShape2D          ← wraps a Shape2D
│       │   └── CollisionObject2D         ← auto-regs with PhysicsServer
│       │       ├── Area2D                ← sensor (no mass)
│       │       └── PhysicsBody2D         ← auto-regs with GravityServer
│       │           └── Particle          ← ready-to-spawn circle body
│       └── Control
│           └── Button                    ← immediate-mode clickable widget
├── Shape2D                               ← abstract, lives outside the scene graph
│   ├── CircleShape2D
│   └── RectangleShape2D
├── RenderServer    (singleton)
├── PhysicsServer   (singleton)
└── GravityServer   (singleton, FMM)
```

---

## Getting Started

### Prerequisites

- A C++17-compatible compiler (g++ 8+, clang++ 7+, MSVC 19.14+)
- GNU Make
- Raylib 5.5 or 6.0 (the merged C++ wrapper is shipped in `include/`, so you only need the base Raylib library installed system-wide)

### Clone

```bash
git clone https://github.com/gecarval/ParticleEngineRaylib.git
cd ParticleEngineRaylib
```

---

## Quick Start

The smallest possible application boots the three servers, spawns one particle, and runs the main loop. Note how `new Particle(...)` alone is sufficient to make the body visible, collidable, and gravitationally active — all three servers learn about it automatically through constructor side-effects.

```cpp
#include "../classes/gravity_server/GravityServer.hpp"
#include "../classes/node/canvas_item/node2d/collision_object2d/physics_body2d/particle/Particle.hpp"
#include "../classes/physics_server/PhysicsServer.hpp"
#include "../classes/render_server/RenderServer.hpp"
#include "../include/raylib-cpp.hpp"

int main(void) {
    // 1. Boot all three singletons
    GravityServer::getInstance();
    PhysicsServer::getInstance();
    RenderServer::getInstance();

    // 2. Spawn a particle — auto-regs with all 3 servers
    Particle* p = new Particle("P0",
        raylib::Vector2(400, 300),
        raylib::Color::White());
    p->setMass(2.0f);

    // 3. Window + main loop
    raylib::Window window(800, 600);
    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground();

        PhysicsServer::getInstance().rebuild();
        GravityServer::getInstance().rebuild();
        GravityServer::getInstance().applyGravity();
        p->updatePhysics();

        RenderServer::getInstance().render(window);
        window.EndDrawing();
    }
    delete p;
    return 0;
}
```

---

## The Main Loop

The canonical frame sequence (from `src/main.cpp`):

```cpp
while (!window.ShouldClose()) {
    cameraControl(cam);
    window.BeginDrawing();
    window.ClearBackground();
    cam.BeginMode();

    // 1. Rebuild spatial hash for collisions (every frame!)
    PhysicsServer::getInstance().rebuild();

    // 2. Rebuild FMM quadtree + run upward pass (P2M, M2M)
    GravityServer::getInstance().rebuild();

    // 3. Run downward pass (M2L, L2L, P2P, L2P) + applyForce to each body
    GravityServer::getInstance().applyGravity();

    // 4. Per-body: query nearby bodies, resolve collisions, integrate
    for (Particle* p : particles) {
        colliders.clear();
        PhysicsServer::getInstance().getCollisions(*p, colliders);
        for (CollisionObject2D* pn2 : colliders) {
            if (*p != *pn2) {
                Particle* p2 = dynamic_cast<Particle*>(pn2);
                p->collideWith(*p2, 0.8f);
            }
        }
        p->updatePhysics();
    }

    // 5. Render visible items (camera-aware culling)
    RenderServer::getInstance().render(window, cam);
    cam.EndMode();
    window.DrawFPS();
    window.EndDrawing();
}
```

### Why this order?

| Step | Why it must come here |
|---|---|
| 1. `PhysicsServer.rebuild` | Re-buckets bodies by current position. Must run before any `getCollisions` call. |
| 2. `GravityServer.rebuild` | Builds the quadtree from current body positions. |
| 3. `GravityServer.applyGravity` | Computes accelerations and pushes them into each body's `_linearAcc` accumulator. Must come before `updatePhysics`. |
| 4. `collideWith` + `updatePhysics` | Collision resolution adjusts velocities; integration consumes the acceleration accumulator and updates position. |
| 5. `RenderServer.render` | Last — draws the post-integration positions. Camera-aware culling skips off-screen bodies. |

---

## GravityServer — Fast Multipole Method

The naïve N-body gravity loop is `O(n²)` — for 2000 particles that's 4 million pairwise force computations per frame, unplayable. `GravityServer` implements the 2D Fast Multipole Method to reduce this to `O(n log n)`.

### How it works

The 2D gravitational potential of a point mass `m_i` at position `z_i` evaluated at `z` is:

$$ \Phi(z) = \sum_i m_i \log(z - z_i) $$

The FMM exploits the fact that when `z` is far from a cluster of sources `{z_i}`, the cluster's contribution can be approximated by a truncated multipole expansion centered on the cluster's center of mass. The engine keeps `P = 6` terms beyond the monopole.

### Per-frame pipeline (6 passes)

| Pass | Name | Direction | Purpose |
|---|---|---|---|
| 1 | **P2M** | upward | Each leaf builds its multipole expansion from its bodies. |
| 2 | **M2M** | upward | Child expansions translated & summed into parents (recursive). |
| 3 | **M2L** | downward | Well-separated cell pairs exchange multipole → local translations during dual-tree traversal. |
| 4 | **L2L** | downward | Local expansions propagated from parents to children. |
| 5 | **P2P** | downward | Nearby body pairs (in same/adjacent leaves) summed directly. |
| 6 | **L2P** | downward | Each leaf evaluates its final local expansion at each body, producing the body's acceleration. |

Passes 1–2 happen in `rebuild()`; passes 3–6 happen in `applyGravity()`.

### The θ acceptance criterion

Two cells `A` and `B` are considered "well-separated" (and thus eligible for M2L instead of P2P) if:

$$ \frac{(h_A + h_B)\sqrt{2}}{d(A, B)} < \theta $$

where `h` is the cell's half-size. Default `θ = 0.6`. Smaller `θ` → more accurate, more direct work. Larger `θ` → faster, less accurate.

### Complexity comparison (n = 2000)

| Approach | Per-frame cost | Notes |
|---|---|---|
| Naïve `O(n²)` | ~4,000,000 pair tests | Unusable for real-time |
| Barnes-Hut `O(n log n)` | ~22,000 operations | Single-tree, asymmetric |
| **FMM `O(n)`** | ~12,000 operations (P=6) | Dual-tree, symmetric, **what this engine uses** |

> 📖 **For the full mathematical derivation with all 6 expansion formulas rendered in KaTeX, see the [interactive documentation](https://pro-2684.github.io/GitHub-Preview/?url=https%3A%2F%2Fgithub.com%2Fgecarval%2FParticleEngineRaylib%2Fblob%2Fmaster%2FDOCUMENTATION.html&preview=1#gecarval/ParticleEngineRaylib/master/DOCUMENTATION.html).**

---

## Project Layout

```
.
├── classes/
│   ├── Object.hpp / .cpp                          ← universal base class
│   ├── Vector2i.hpp                               ← integer vector + hash functor
│   ├── gravity_server/
│   │   └── GravityServer.{hpp,cpp}                ← FMM O(n log n) gravity
│   ├── physics_server/
│   │   └── PhysicsServer.{hpp,cpp}                ← spatial hash grid
│   ├── render_server/
│   │   └── RenderServer.{hpp,cpp}                 ← draw loop + culling
│   ├── shape2d/
│   │   ├── Shape2D.{hpp,cpp}                      ← abstract base
│   │   ├── circle_shape2d/CircleShape2D.{hpp,cpp}
│   │   └── rectangle_shape2d/RectangleShape2D.{hpp,cpp}
│   └── node/
│       ├── Node.{hpp,cpp}                         ← scene-graph primitive
│       └── canvas_item/
│           ├── CanvasItem.{hpp,cpp}               ← auto-regs RenderServer
│           ├── control/
│           │   ├── Control.{hpp,cpp}
│           │   └── button/Button.{hpp,cpp}
│           └── node2d/
│               ├── Node2D.{hpp,cpp}               ← 2D transform
│               ├── sprite2d/Sprite2D.{hpp,cpp}
│               ├── collision_shape2d/CollisionShape2D.{hpp,cpp}
│               └── collision_object2d/
│                   ├── CollisionObject2D.{hpp,cpp}    ← auto-regs PhysicsServer
│                   ├── area2d/Area2D.{hpp,cpp}
│                   └── physics_body2d/
│                       ├── PhysicsBody2D.{hpp,cpp}    ← auto-regs GravityServer
│                       └── particle/Particle.{hpp,cpp}
├── include/                                       ← Raylib 5.5/6.0 merged C++ wrapper
├── src/
│   └── main.cpp                                   ← 2000-particle demo
├── DOCUMENTATION.html                             ← interactive docs (open in browser)
├── Makefile
└── README.md
```

---

## Building

```bash
make
./particle_life_raylib    # or whatever the Makefile outputs
```

The default `Makefile` targets the platform-appropriate Raylib. The merged C++ wrapper in `include/` provides retro-compatibility with Raylib 5.5 while exposing some 6.0 features.

---

## Controls (in the shipped demo)

| Input | Action |
|---|---|
| `W` `A` `S` `D` | Pan the camera |
| Middle mouse drag | Pan the camera (mouse) |
| Mouse wheel | Zoom in/out (clamped to `[0.1, 3.0]`) |
| `ESC` | Close window |

The demo spawns 2000 particles in a disc with tangential velocity and inverse-distance mass distribution, creating a roughly Keplerian orbital pattern.

---

## Interactive Demos

The [full documentation](https://pro-2684.github.io/GitHub-Preview/?url=https%3A%2F%2Fgithub.com%2Fgecarval%2FParticleEngineRaylib%2Fblob%2Fmaster%2FDOCUMENTATION.html&preview=1#gecarval/ParticleEngineRaylib/master/DOCUMENTATION.html) includes 6 live in-browser demos (pure JavaScript reimplementations of the C++ algorithms):

1. **FMM Adaptive Quadtree** — drag bodies around, watch the tree split/merge, see M2L vs P2P pairs color-coded
2. **N-Body Simulation** — toggle naïve `O(n²)` vs FMM `O(n log n)` with live FPS counter
3. **Scene Graph Explorer** — click any of 18 classes to see its full inheritance chain and member list
4. **Spatial Hash Grid** — hover to highlight the 3×3 neighborhood, adjustable cell size
5. **Collision Solver** — two-body impulse resolution with visible contact point, normal, and impulse magnitude
6. **API Playground** — editable config that re-runs a small simulation, with Galaxy and Collapse presets

---

## API Cookbook

A few copy-paste recipes (more in the [full documentation](https://pro-2684.github.io/GitHub-Preview/?url=https%3A%2F%2Fgithub.com%2Fgecarval%2FParticleEngineRaylib%2Fblob%2Fmaster%2FDOCUMENTATION.html&preview=1#gecarval/ParticleEngineRaylib/master/DOCUMENTATION.html)):

### Spawn a particle

```cpp
Particle* p = new Particle("hero",
    Vector2(400, 300),
    Color::Red());
p->setMass(2.0f)
 .setLinearVel(Vector2(50, 0));
```

### Add a static wall

```cpp
PhysicsBody2D* wall = new PhysicsBody2D("wall");
wall->setStatic(true);

Sprite2D* wallSprite = new Sprite2D("wallSprite");
wallSprite->setShape(200.0f, 20.0f)
          .setColor(Color::Gray());

CollisionShape2D* wallShape = new CollisionShape2D("wallShape");
wallShape->setShape(200.0f, 20.0f);

wall->pushBackChild(*wallSprite);
wall->pushBackChild(*wallShape);
wall->setPos(Vector2(300, 500));  // propagates to children
```

### Register a button with onClick

```cpp
Button* btn = new Button(20, 20, 160, 40, "Spawn 10");
btn->setOnClick([&]() {
    for (int i = 0; i < 10; ++i)
        spawnParticles(particles, 1, 100.0f);
});

// In main loop:
btn->update();  // polls mouse + may fire onClick
```

### Set up a 2-body orbit

```cpp
Particle* sun    = new Particle("sun",    Vector2(0, 0),   Color::Yellow());
Particle* planet = new Particle("planet", Vector2(200, 0), Color::Blue());

sun->setMass(1000.0f);
planet->setMass(1.0f);

// For circular orbit, v = sqrt(G * M / r). With G=1, M=1000, r=200:
const float orbitalVel = sqrtf(1000.0f / 200.0f);  // ≈ 2.236
planet->setLinearVel(Vector2(0, orbitalVel));
```

### Clean shutdown

```cpp
for (Particle* p : particles) delete p;
particles.clear();

RenderServer::deleteInstance();
PhysicsServer::deleteInstance();
GravityServer::deleteInstance();
```

---

## License

See the project repository for licensing information.

---

## Further Reading

- **📖 [Full Interactive Documentation](https://pro-2684.github.io/GitHub-Preview/?url=https%3A%2F%2Fgithub.com%2Fgecarval%2FParticleEngineRaylib%2Fblob%2Fmaster%2FDOCUMENTATION.html&preview=1#gecarval/ParticleEngineRaylib/master/DOCUMENTATION.html)** — 17 classes documented with Reference+Why depth, KaTeX-rendered FMM formulas, and 6 live canvas demos.
- Greengard, L. & Rokhlin, V. (1987). *A fast algorithm for particle simulations.* Journal of Computational Physics, 73(1), 325–348.
- [Raylib](https://www.raylib.com/) — the underlying hardware-accelerated library.
- [Godot Engine](https://godotengine.org/) — inspiration for the node-tree and singleton-server architecture.

