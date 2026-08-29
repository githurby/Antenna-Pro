# Antenna Pro Engineering — C++17 / Win32 / OpenGL

Native Windows desktop application for preliminary antenna engineering, 3D visualization, 50-ohm matching design, plots, and construction documentation. It uses only the Win32 API, Common Controls, GDI, and OpenGL 1.1; no Qt, .NET, or third-party runtime is required.

## Main features

- 11 unique antenna topologies across MF, HF, VHF, UHF, and above.
- Yagi-Uda, Delta Loop, and Cubical Quad arrays with 1–5 elements.
- Frequency, conductor diameter, velocity factor, installation height, element count, measured R+jX, and design power inputs.
- Fixed 50-ohm feed-line target.
- Automatic selection of direct feed, 1:1 current balun/choke, low-pass L network, impedance-transforming BALUN/UNUN, or gamma match.
- Gamma-rod length, spacing, rod diameter, stub impedance, step-up factor, and series component calculation.
- Transformer turns ratio and practical integer-turn approximation.
- 3D OpenGL geometry, SWR/R/X sweep, and normalized radiation cuts.
- Separate construction-diagram window showing how the matching system connects to the driven element.
- TXT engineering report and CSV sweep export.
- Fully English Unicode interface.

## Build with Code::Blocks and MinGW-w64

1. Install Code::Blocks with a 64-bit MinGW-w64 toolchain.
2. Open `AntennaPro.cbp`.
3. Select **GNU GCC Compiler** if Code::Blocks asks for a compiler profile.
4. Select the `Release` target.
5. Run **Build > Clean**, then **Build > Rebuild**.
6. The executable is written to `bin/Release/AntennaPro.exe`.

The project includes `-municode` because the application entry point is `wWinMain`. The required link libraries are already configured: `opengl32`, `gdi32`, `comctl32`, `comdlg32`, `user32`, and `kernel32`.

Command-line alternative from a MinGW shell:

```bat
mingw32-make -f Makefile.win clean
mingw32-make -f Makefile.win
```

A CMake build file is also included.

## Recommended workflow

1. Select the topology and real center frequency.
2. Enter conductor and installation data.
3. Use `Measured R = 0` for an analytical starting estimate.
4. Build the radiator with adjustable margins.
5. Install it in its final environment.
6. Calibrate a VNA at the matching-network reference plane.
7. Enter the measured R and X and recalculate.
8. Open **50 Ω MATCHING DIAGRAM** and construct the displayed network.
9. Tune at low power, remeasure, and repeat.
10. Validate common-mode current, bandwidth, temperature, structure, and radiated performance.

## Important engineering limitation

The SWR sweep is a local equivalent-RLC model and the radiation cuts are family-level analytical approximations. This is not a Method-of-Moments, FDTD, or FEM full-wave solver. Ground, mast, boom, feed cable, dielectric hardware, nearby structures, losses, and parasitic coupling can move the real result.

A displayed `50 + j0 ohms` is the ideal result of the calculated matching network at one frequency. It is not a guarantee that the physical build will measure exactly 50 ohms. Professional work requires full-wave simulation and VNA verification at the installed antenna.

## Safety

- Keep every antenna and mast away from power lines.
- Perform a separate wind, ice, mast, guy, and foundation analysis.
- Use lightning protection, bonding, and grounding compliant with local rules.
- Observe RF exposure limits, licensing, authorized bands, and power limits.
- End-fed antennas and matching networks can develop hazardous RF voltage.
- Start tuning at low power and use components with adequate voltage, current, loss, and thermal margins.

## Files

- `src/AntennaModel.cpp` — antenna geometry, analytical model, 50-ohm matching engine, and reports.
- `src/GLRenderer.cpp` — OpenGL 3D and graph rendering.
- `src/MatchDiagram.cpp` — graphical matching-network construction window.
- `src/main.cpp` — Win32 user interface, input handling, and exports.
- `docs/METHODOLOGY.md` — equations and validation protocol.
- `docs/MATCHING_50_OHM.md` — detailed matching guide.
- `tutorial.html` — complete illustrated offline tutorial.

## Application icon

The executable now embeds `resources/app.ico`, a multi-resolution Windows icon containing 16, 24, 32, 48, 64, 128, and 256 pixel variants. The same resource is used by the executable file, taskbar, Alt+Tab switcher, main-window title bar, and matching-diagram window. Resource ID `IDI_APP_ICON` is defined in `resources/resource.h` and compiled by `resources/app.rc`.
