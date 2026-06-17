# Game Engine Journey

A self-directed, multi-year path to become a **game engine programmer**: a *builder* of engines, not a user of them. This repository is the working record of that journey. Every topic on the roadmap lives in its own folder with code that **compiles**, plus a short README explaining what it demonstrates.

The proof of understanding is not notes. It's working code under version control.

## Principles (fixed)

A multi-year plan is a hypothesis, not a contract. The details flex as I learn; these principles do not:

- Foundations and math before graphics.
- Build real things, not follow tutorials.
- Working code in Git is the proof. Notes don't count.
- Naive engine first, ECS later: feel the pain before reaching for the abstraction.

## Repository layout

Folders are created as I reach each topic, not all upfront. Each topic is:

```
phase-N-name/
  X.Y-topic-name/
    main.cpp
    README.md
```

- `phase-0-systems/`: C++ and systems fundamentals.
- `phase-1-math/`: Math for graphics (runs in parallel with phase 0).
- Later phases are added as the journey progresses.

## Build

Visual Studio 2022 is temporary scaffolding. The real build system arrives with CMake in **Phase 0.9**. Until then, exercises are single-file and compiled from the *Developer Command Prompt for VS 2022*:

```
cl /EHsc /std:c++20 main.cpp
```

That is why VS project files (`.sln`, `.vcxproj`) are **not** versioned. The `.cpp` is the artifact, not the IDE metadata.

## Roadmap

> **Living document.** The order of sub-topics, the choice of graphics API, and the scope of Phase 6 can change. Finishing every item is **not** the goal; professionals specialize. This is a map, not a 69-box checklist.

**Key convergence milestone:** Phase 2.2, a software rasterizer from scratch, where C++ and math first meet on screen.

### Phase 0: Systems programming fundamentals
- 0.1 Modern C++ I: types, pointers, references, stack vs heap
- 0.2 Modern C++ II: RAII, smart pointers, move semantics, rvalue references
- 0.3 Modern C++ III: templates, concepts, basic metaprogramming
- 0.4 Modern C++ IV: UB, memory model, alignment, object layout
- 0.5 C and its differences from C++
- 0.6 Data structures and their real cost in cache (beyond big-O)
- 0.7 Computer architecture: cache, branch prediction, pipelining
- 0.8 OS for engines: virtual memory, file I/O, threads vs processes
- 0.9 Build systems: CMake from scratch
- 0.10 Debugging and profiling: gdb/lldb, sanitizers, Tracy, perf
- 0.11 Git for serious projects
- 0.12 Testing: Catch2/doctest, basic CI
- **Milestone:** CLI tool in C++ with a custom allocator, tests, CMake and CI.

### Phase 1: Math for graphics (parallel to Phase 0)
- 1.1 Linear algebra I: vectors, dot/cross products, bases, change of basis
- 1.2 Linear algebra II: matrices, affine transforms, model/view/world space
- 1.3 Linear algebra III: perspective/orthographic projection, frustum, NDC
- 1.4 Rotations: matrices, Euler, gimbal lock, quaternions
- 1.5 Applied trigonometry and curves (Bézier, splines, Catmull-Rom)
- 1.6 IEEE 754 floating point: precision, NaN, accumulated error
- 1.7 Numerical methods: integrators (Euler, semi-implicit, RK4, Verlet)
- 1.8 Basic computational geometry: intersections, projections
- **Milestone:** Own math library (Vec, Mat, Quat) with tests.

### Phase 2: Low-level graphics
- 2.1 How a GPU really works: the graphics pipeline
- 2.2 Software rasterizer from scratch (foundational project, no APIs) [*]
- 2.3 OpenGL I: context, buffers, first triangle
- 2.4 OpenGL II: GLSL shaders, uniforms, varyings
- 2.5 OpenGL III: textures, sampling, mipmaps, filtering
- 2.6 OpenGL IV: framebuffers, depth/stencil, render targets
- 2.7 Classic lighting: Phong, Blinn-Phong, normal mapping
- 2.8 Model loading: OBJ, glTF, mesh representation
- **Milestone:** Renderer with a free camera, glTF with textures and Phong lighting.
- *Tooling:* add RenderDoc here. **Portfolio starts here:** capture screenshots/GIFs from 2.2 on.

### Phase 3: Basic engine architecture
- 3.1 Game loop: fixed vs variable timestep, accumulator pattern
- 3.2 Input systems with event queues
- 3.3 Resource/asset management: handles, hot-reload
- 3.4 Cameras: first-person, orbital, view systems
- 3.5 Scene representation: scene graph and flat alternatives
- 3.6 In-engine logging and telemetry
- 3.7 Configuration and CVars (Quake-style)
- **Milestone:** Small engine that loads assets, moves a camera, renders a scene.
- *Around here you are a credible junior candidate; start applying in parallel.*

### Phase 4: Patterns, performance and advanced architecture
- 4.1 Game Programming Patterns: the ones that matter
- 4.2 ECS: Entity Component System from scratch
- 4.3 Data-oriented design
- 4.4 Custom memory allocators: linear, pool, stack, frame
- 4.5 Performance I: systematic profiling, finding real bottlenecks
- 4.6 Performance II: cache-friendly layout, AoS vs SoA
- 4.7 Performance III: basic SIMD (SSE/AVX intrinsics)
- 4.8 Concurrency I: C++ memory model, atomics, memory ordering
- 4.9 Concurrency II: job systems / task schedulers
- 4.10 Asset pipeline and custom binary serialized formats
- **Milestone:** Engine rewritten with ECS, custom allocators and a job system.

### Phase 5: Modern rendering
- 5.1 Architectures: forward, deferred, forward+, visibility buffer
- 5.2 PBR I: theory (BRDF, microfacets, energy conservation)
- 5.3 PBR II: implementation (Cook-Torrance, GGX, Fresnel-Schlick)
- 5.4 Shadow mapping and cascaded shadow maps
- 5.5 Post-processing: bloom, tone mapping, SSAO
- 5.6 HDR, gamma, color spaces (sRGB, linear, ACES)
- 5.7 Vulkan: validation layers, descriptors, render passes, synchronization
- 5.8 Render graphs / frame graphs
- **Milestone:** PBR renderer with cascaded shadows in Vulkan with a render graph.
- *Tooling:* add Nsight Graphics for Vulkan.

### Phase 6: Remaining subsystems (flexible order)
- 6.1 Physics I: collision detection (AABB, SAT, GJK)
- 6.2 Physics II: rigid body dynamics, constraints, stable integrators
- 6.3 Audio: mixing, 3D spatialization, basic DSP
- 6.4 Animation I: skeletal animation, skinning, GPU skinning
- 6.5 Animation II: blend trees, IK, state machines
- 6.6 Networking I: TCP vs UDP, fundamentals
- 6.7 Networking II: client-server, snapshots, interpolation
- 6.8 Networking III: lag compensation, rollback netcode
- 6.9 Scripting integration: Lua, AngelScript, custom VM
- 6.10 Tooling: editor with ImGui, reflection, undo/redo, gizmos
- **Milestone:** Integrate at least two subsystems + a minimal editor.

### Phase 7: Deep specialization
*Portfolio and job applications already run continuously from Phase 2.2. This phase is about going deep.*
- 7.1 Choose a deep specialization (rendering, physics, tools, etc.)
- 7.2 Study real code: id Tech (Quake/Doom 3), Godot, bgfx
- 7.3 Contribute to an open-source engine with accepted PRs
- 7.4 Own public mini-engine + a small game built with it
- 7.5 Technical interview prep (continuous, from when you start applying)
- 7.6 CV, portfolio site, online presence (continuous, from Phase 2.2)
- **Milestone:** Complete public portfolio and active applications to studios.

## Reference texts

- *Game Engine Architecture*, Jason Gregory
- *Real-Time Rendering*, Akenine-Möller et al.
- *Game Programming Patterns*, Robert Nystrom (free online)
- *Foundations of Game Engine Development*, Eric Lengyel
- *Physically Based Rendering*, Pharr, Jakob, Humphreys
- *3D Math Primer*, Dunn, Parberry
- Handmade Hero, Casey Muratori
- Quake / Doom 3 source, id Software