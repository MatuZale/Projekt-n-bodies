# N-body simulator

A 3D gravitational N-body simulator in C++20, built as a vehicle for learning C++ and numerical methods.

- **Integrator:** symplectic leapfrog (kick–drift–kick), so the energy error stays bounded instead of drifting.
- **Check:** `obliczEnergie()` computes the total energy; it is printed during the run to verify the numerics.
- **Rendering:** OpenGL via an SFML window and the GLAD loader, with a mouse-controlled orbital camera (yaw / pitch / zoom) and particle trails.
- **Plan:** Lennard-Jones molecular dynamics next, then a Schrödinger-equation solver.

## Files

| File | Role |
|---|---|
| `nbody.h`, `nbody.cpp` | `Cialo` (body) struct, `kick` / `drift`, accelerations and energy |
| `sim3d.cpp` | 3D visualisation and simulation loop |
| `test_opengl.cpp` | minimal OpenGL / SFML smoke test |
| `glad/` | OpenGL loader |
| `archiwum/` | earlier 2D versions (Euler, SFML) kept for reference |

## Building

Developed on macOS with `clang++ -std=c++20 -DGL_SILENCE_DEPRECATION`, SFML 3 and GLM from Homebrew;
link `glad/src/glad.c` and the SFML window/system libraries. Identifiers in the code are in Polish (`Cialo` = body).

## License

MIT — see [LICENSE](LICENSE).
