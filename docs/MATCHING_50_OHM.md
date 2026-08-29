# Automatic 50-ohm matching

## Native impedance versus system impedance

An antenna radiator does not have to present 50 ohms by itself. Antenna Pro separates:

- native estimated or measured antenna impedance, `Za = R + jX`;
- impedance seen by the coax after the matching network, with a target of `50 + j0 ohms`.

For the best result, measure the installed antenna with a calibrated VNA. Enter `Measured R = 0` only when an analytical starting estimate is required. Positive X is inductive; negative X is capacitive.

## Automatic methods

### Direct connection and 1:1 current control

A load already close to 50 ohms is connected directly. A balanced antenna receives a 1:1 current-balun recommendation; an unbalanced load receives a 1:1 common-mode choke. A 1:1 device controls common-mode current but does not transform impedance.

### Low-pass L network

The application first cancels measured load reactance and then calculates:

`Q = sqrt(Rhigh/Rlow - 1)`

`Xs = Q x Rlow`

`Xp = Rhigh/Q`

`L = X/(2 pi f)`

`C = 1/(2 pi f |X|)`

The series and shunt locations depend on which side has the higher resistance. The result is exact only at the center frequency with ideal components.

### BALUN or UNUN transformer

The ideal turns ratio is:

`Nantenna/N50 = sqrt(Rantenna/50)`

The program finds a practical integer-turn approximation and calculates residual SWR. Examples:

- 100 to 50 ohms: ideal turns ratio `sqrt(2)`, approximately 10:14 turns.
- 2450 to 50 ohms: 49:1 impedance ratio, 7:1 turns ratio, for example 2:14 turns.

Core material, absolute turn count, winding transmission-line impedance, power handling, leakage inductance, and parasitic capacitance must be selected and measured for the intended frequency range.

### Gamma match

For multi-element Yagis with a low driven-element resistance, the program calculates a gamma-match starting design using:

- driven-element diameter;
- gamma-rod diameter;
- center-to-center spacing;
- two-conductor stub characteristic impedance;
- geometric impedance step-up factor;
- numerical gamma-rod length search;
- series capacitance or inductance needed to cancel residual reactance.

The rod and shorting strap must be adjustable. Use a variable capacitor spanning roughly 0.5–1.5 times the calculated center value. Add a 1:1 common-mode choke to the feed line.

**Connect the gamma match only to the driven element. Directors and reflectors are parasitic elements and must never be connected to the coax or gamma rod.**

## Power calculations

`Vrms = sqrt(P x 50)`

`Irms = sqrt(P/50)`

`Vpeak = sqrt(2) x Vrms`

Use at least a 2x component margin, and allow more for modulation peaks, SWR, rain, contamination, corona, and end voltage.

## Construction and tuning sequence

1. Leave 1–3% trimming margin on adjustable radiator sections.
2. Install the complete antenna, boom, mast, feed cable, insulators, and grounding arrangement.
3. Calibrate the VNA at the plane where the matching network will be installed.
4. Measure the unmatched antenna R+jX.
5. Enter those values and recalculate.
6. Open **50 Ω MATCHING DIAGRAM**.
7. Build the network with short, low-loss interconnections.
8. For a gamma match, adjust rod/strap position for resistance and the series capacitor for zero reactance; repeat iteratively.
9. Confirm impedance, SWR, bandwidth, and common-mode current.
10. Increase RF power gradually while checking heating and arcing.
11. Validate radiation performance. A low SWR alone does not prove that the antenna radiates efficiently.

## Technical references

- ARRL, gamma-match material: https://www.arrl.org/files/file/Product%20Notes/A%20Revision%20to%20Low%20Band%20DXing.pdf
- N6MW, *What's with the Gamma Match Equations?*: https://www.nonstopsystems.com/radio/pdf-ant/article-antenna-magloop-gamma-6MW.pdf
- L. B. Cebik, W4RNL, gamma-match modeling: https://antenna2.github.io/cebik/content/amod/amod118.html
- DX Engineering, balun selection: https://www.dxengineering.com/techarticles/balunsandfeedlinechokes/baluns-choosing-the-correct-balun
- Applied Concepts, matching-network equations: https://jake-peters.com/applied-concepts/rf-circuit-design/impedance-matching-networks

Calculated values are engineering starting points. Always validate the physical network and installed antenna with a VNA and, for professional work, a full-wave electromagnetic model.
