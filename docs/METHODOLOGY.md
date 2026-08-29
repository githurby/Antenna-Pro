# Engineering methodology

## Core conventions

- Free-space propagation speed: `c = 299,792,458 m/s`.
- Wavelength: `lambda = c/f`.
- Feed-line target: `Z0 = 50 ohms`.
- Initial dipole shortening factor: `0.95` applied to a half wavelength.
- Dimensions are initial electrical values, not certified fabrication dimensions.

## Implemented starting dimensions

- Half-wave dipole: total length `0.475 lambda`.
- Quarter-wave Ground Plane: radiator `0.238 lambda`; radials `0.25 lambda`, sloped 45 degrees.
- Delta Loop: perimeter `0.99 lambda`; arrays use 1–5 loops and initial spacing `0.15 lambda`.
- Yagi-Uda: driven element `0.475 lambda`, reflector `0.51 lambda`, progressively shorter directors, and initial spacing `0.18 lambda`.
- T2LT: two active sections of `0.25 lambda x velocity factor`.
- Moxon: initial envelope approximately `0.36 lambda x 0.14 lambda`.
- End-Fed Half-Wave: wire `0.475 lambda`, nominal 2450-ohm feed resistance, and a 49:1 UNUN.
- J-Pole: `0.75 lambda` and `0.25 lambda` sections with an adjustable feed tap.
- Cubical Quad: 1–5 elements, driven side near `0.25 lambda`, initial spacing `0.18 lambda`.

## SWR calculation

The local sweep uses an equivalent resonator. Reflection and SWR are then calculated from complex impedance without further approximation:

`Gamma = (Z - Z0) / (Z + Z0)`

`SWR = (1 + |Gamma|) / (1 - |Gamma|)`

`Return loss = -20 log10(|Gamma|)`

The sweep is useful for preliminary comparison, but it does not replace a full-wave solution.

## Radiation patterns

The displayed azimuth and elevation curves are normalized analytical family models. They communicate expected broad behavior—omnidirectional, bidirectional, or directional—but do not include real ground, mounting hardware, cable current, buildings, or optimized parasitic-element currents.

## Professional validation protocol

1. Define frequency, bandwidth, polarization, gain, F/B, power, and mechanical envelope.
2. Generate a preliminary design in Antenna Pro.
3. Model the complete structure in NEC-2/NEC-4, FEKO, CST, or another validated full-wave solver.
4. Include conductor loss, realistic ground, mast, boom, insulators, and feed geometry.
5. Run sensitivity analysis on dimensions, height, material, and load.
6. Verify peak current, peak voltage, matching loss, core flux, and heating.
7. Perform a separate structural calculation.
8. Build an adjustable prototype.
9. Calibrate the VNA at the real feed reference plane and measure S11 and R+jX.
10. Enter measured R+jX into Antenna Pro and recalculate the matching system.
11. Validate radiation pattern and gain in a suitable test range.
12. Freeze dimensions, tolerances, materials, inspection criteria, and test records.
