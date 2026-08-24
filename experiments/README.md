# Controlled experiments

The [root README](../README.md) gives the diagnostic sequence — the six results that between
them locate the failure. This file is the parameter work underneath it: how the operating point
was chosen in the first place, and what happened when each factor was moved on its own.

Everything here ran between **October 2023 and February 2024** on SPHinXsys v1.0-beta.08. The
videos are ParaView screen captures recorded at the time. Numbers are the ones recorded with each
run.

⚠ Read this as a lab notebook, not a paper. The raw case files for most of these are gone; what
survives is the settings, the recorded response, and the video. Where I am reasoning rather than
reporting, it says so.

---

## Choosing the reference velocity

Weakly-compressible SPH needs a reference velocity `U_ref` up front: it sets the artificial speed
of sound, and through it the acoustic time step and the density variation. Pick it badly and the
simulation is either uselessly slow or silently wrong. It is not an output, so it cannot be
checked against anything — which makes it the natural first suspect when a case misbehaves.

There is no single balance to take it from, so I took three, each from equating two terms:

| | Balance | Gives |
|---|---|---|
| `v1` | rho_w v^2 l_s = sigma | inertia vs surface tension |
| `v2` | sigma = mu_w v | surface tension vs viscous |
| `v3` | rho_w v^2 l_s = mu_w v | inertia vs viscous |

**Approach 1** takes `U_ref = min(v1, v2, v3)` — the most restrictive of the three, so no
mechanism is under-resolved.

**Approach 2** uses `v4 ≈ 0.95 v1` for unit thickness: the velocity a droplet would reach if its
surface energy converted entirely to kinetic energy. That makes `v4` a **ceiling with physical
meaning**, and the comparison of `U_ref` against it a prediction rather than a diagnosis:

- `U_ref < v4` — only part of the surface energy becomes kinetic energy.
- `U_ref > v4` — the droplet carries kinetic energy the physics cannot account for. Whether that
  shows up as a visible failure then depends on whether viscosity can dissipate it.

That gives two factors — `U_ref` relative to `v4`, and viscosity — and a predicted outcome for
each combination before running anything. Study A tests exactly that.

---

## Study A — screening over (U_ref, viscosity)

Five runs spanning the corners of those two factors. `(S)` marks a value in the library's internal
scaled units rather than SI.

| | `l_s` | rho_w | rho_a | mu_w | mu_a | sigma | `U_ref` | Re | Prediction | Video |
|---|---|---|---|---|---|---|---|---|---|---|
| **1** Very low `U_ref`, low viscosity | 2.7e-3 | 1e3 | 1 | 1e-3 | 1.789e-5 | 0.073 | 3.7e-4 | 1.0 | `U_ref` < `v4`; viscosity too low to dissipate the excess | [`a1`](video/a1-low-uref-low-visc.mp4) |
| **2** Low `U_ref`, high viscosity *(library default)* | 1.0 (S) | 1.0 (S) | 1e-3 (S) | 0.2 (S) | 0.002 (S) | 1.0 (S) | 0.2 | 1.0 | `U_ref` < `v4`; viscosity may dissipate it | [`a2`](video/a2-low-uref-high-visc.mp4) |
| **3** High `U_ref`, low viscosity | 1.0 | 1.0 | 1e-3 | 1e-3 | 0.002 | 1.0 | 1.0 | 1.0e3 | `U_ref` > `v4`; viscosity too low | [`a3`](video/a3-high-uref-low-visc.mp4) |
| **4** Matched `U_ref`, high viscosity | 2.7e-3 | 1e3 | 1 | 0.2 | 1.789e-5 | 0.073 | 0.156 | 2.1 | `U_ref` = `v4`; viscosity high, will dissipate any artificial excess | [`a4`](video/a4-matched-uref-high-visc.mp4) |
| **5** High `U_ref`, low viscosity *(the real case)* | 2.7e-3 | 1e3 | 1 | 1e-3 | 1.789e-5 | 0.073 | 2.65 | **7155** | `U_ref` > `v4`; excess artificial energy, viscosity too low | [`a5`](video/a5-high-uref-low-visc-default.mp4) |

Run 4 is the control: it is the only one where `U_ref` sits at `v4` **and** viscosity is high
enough to absorb what is left. Run 5 is the case of interest — the physical droplet, at
Re 7155 — and it is the opposite corner on both factors.

**What this settles.** The failure is not an unlucky choice of `U_ref`. Run 5 is unstable at the
`U_ref` the physics dictates (2.65 m/s, the impact velocity), and the operating point cannot be
moved to a stable corner without abandoning the problem. Approach 1 would have set `U_ref` to
3.7e-4 here — four orders of magnitude below the physical velocity, which is why the
minimum-of-three rule is not usable for this case.

---

## Study B — one factor at a time

Study A moves two factors together. This one moves them singly from a common baseline, with
`U_ref` used as the compensating variable so that Re can be held fixed where wanted.

Characteristic length is `l = 1.3 * D_L / 40` throughout — the smoothing length, not the domain
size.

| Factor moved | `D_L` | rho_w | mu_w | sigma | `U_ref` | Re | Video |
|---|---|---|---|---|---|---|---|
| — *(baseline)* | 2.0 | 1.0 | 0.2 | 1.0 | 5.0 | 1.625 | [`b0`](video/b0-baseline-Re1.63.mp4) |
| length down | **5.4e-3** | 1.0 | 0.2 | 1.0 | 1139.6 | **1.0** | [`b1`](video/b1-length-Re1.0.mp4) |
| density up x1000 | 2.0 | **1.0e3** | 0.2 | 1.0 | 5.0 | **1625** | [`b2`](video/b2-density-Re1625.mp4) |
| viscosity down x200 | 2.0 | 1.0 | **1.0e-3** | 1.0 | 1.0e3 | **65e3** | [`b3`](video/b3-viscosity-Re65000.mp4) |
| surface tension down | 2.0 | 1.0 | 0.2 | **7.3e-2** | 3.07 | **1.0** | [`b4`](video/b4-surface-tension-Re1.0.mp4) |

Every row is consistent with `Re = rho_w * U_ref * l / mu_w` to better than 0.25%, which is worth
stating because it is checkable from the table alone.

Rows `b1` and `b4` are the informative ones: they change a factor while **holding Re at 1.0**. If
the failure tracked Reynolds number alone, those two would behave like the low-Re baseline. Rows
`b2` and `b3` do the reverse — they push Re to 1625 and 65 000 by moving density and viscosity
respectively, separating "high Re" from "the specific thing that changed".

---

## Study C — the dissipation limiter

The library's acoustic Riemann solver limits its own dissipation. The two return statements differ
only in that limiter:

```cpp
// acoustic Riemann solver
return rho0c0_geo_ave_ * u_jump * SMIN(Real(eta) * SMAX(u_jump * inv_c_ave_, Real(0)), Real(1));

// dissipative Riemann solver
return rho0c0_geo_ave_ * u_jump;
```

`eta` scales the velocity jump before it saturates at 1, so raising it should walk the acoustic
solver towards the dissipative one. The ladder:

| Setting | Case | Max spread ratio | Video |
|---|---|---|---|
| beta = 1 | square droplet | — | [`c1`](video/c1-acoustic-beta1.mp4) |
| beta = 2 | square droplet | — | [`c2`](video/c2-acoustic-beta2.mp4) |
| low dissipation | square droplet | — | [`c3`](video/c3-low-dissipation.mp4) |
| eta = 3 | 2D impact | — | [`c4`](video/c4-impact-2d-eta3.mp4) |
| eta = 3e4 | 2D impact | — | [`c5`](video/c5-impact-2d-eta3e4.mp4) |
| dissipative | 2D impact | — | [`c6`](video/c6-impact-2d-dissipative.mp4) |
| eta = 1e20 | 2D impact | **7.8** | [`c7`](video/c7-spread-2d-eta1e20.mp4) |
| dissipative | 2D impact | **5.37** | [`c8`](video/c8-spread-2d-dissipative.mp4) |
| dissipative | 3D impact | **3.74** | [`c9`](video/c9-spread-3d-dissipative.mp4) |
| eta = f(alpha) | 2D impact | — | [`c10`](video/c10-eta-as-function-of-alpha.mp4) |

Experimental maximum spread ratio for this droplet is **4.5**.

### The eta to infinity limit is not the dissipative solver

At eta = 1e20 the limiter is saturated, so the acoustic solver should be indistinguishable from
the dissipative one. It is not: **7.8 against 5.37 on the same 2D case.**

*[my reading, not measured]* — the reason is visible in the code above. `SMAX(u_jump * inv_c_ave_, 0)`
clamps at zero, so when `u_jump` is negative the whole product is zero **no matter how large
`eta` is**. The dissipative form has no such clamp and returns `rho0c0 * u_jump` throughout. The
two therefore differ precisely on expansion, and no value of eta closes the gap. The limit of the
acoustic solver is one-sided dissipation, not the dissipative solver.

This matters for the open problem in the root README. If eta is going to be tuned against
experiment, it is worth knowing that the family it sweeps does not contain the dissipative solver
as an endpoint.

A related point, established in the discussions with the TUM group: the various Riemann solvers
are largely different routes to the same numerical dissipation in differing amounts, and the
acoustic solver can be driven to high dissipation through a high `U_ref` alone. That is consistent
with Study A, where `U_ref` and the observed stability move together.

---

## Study D — Poiseuille flow, 2 x 2

The droplet cases have no analytical solution, so a limiter tuned on them cannot be checked.
Plane Poiseuille flow does. Two limiter settings crossed with two Reynolds numbers — a full
factorial, and the only study here whose answer is known in advance.

| | eta = 3 | eta = 1e20 |
|---|---|---|
| **Re = 0.0125** | [`d1`](video/d1-poiseuille-eta3-Re0.0125.mp4) | [`d3`](video/d3-poiseuille-eta1e20-Re0.0125.mp4) |
| **Re = 5** | [`d2`](video/d2-poiseuille-eta3-Re5.mp4) | [`d4`](video/d4-poiseuille-eta1e20-Re5.mp4) |

This is the study I would run first if starting again. Verifying the limiter against a closed-form
solution constrains it in a way that matching a spread ratio does not.

---

## Studies E and F — two single-factor checks

**E — artificial viscosity coefficient.** alpha = 0.03, the value from Yang *et al.*, against
alpha = 0.04. Baseline is [`c4`](video/c4-impact-2d-eta3.mp4); the raised value is
[`e1`](video/e1-alpha-0.04.mp4). Paired with `c10`, where eta is instead made a function of alpha.

**F — density reinitialisation.** [`f1`](video/f1-no-density-reinitialization.mp4) is the same
case with it switched off, to confirm it was not masking or causing the behaviour.

---

## Baseline runs

The square droplet at the operating point of interest, and the 3D impact case.

| | Video |
|---|---|
| Square droplet to circle, 2D, Re 7155, We 260 | [`g1`](video/g1-square-droplet-2d-Re7155.mp4) |
| Same, 3D | [`g2`](video/g2-square-droplet-3d-Re7155.mp4) |
| 3D impact, no surrounding air | [`g3`](video/g3-impact-3d-no-air.mp4) |

Note the root README quotes **Re 7154 / We 259** for this case, computed from the droplet
diameter; the meeting slides quote **7155 / 260** as the nominal target. Same case, same
conclusions — the difference is rounding in the input, not a discrepancy in the result.

---

## Provenance

The videos were recorded to a screen-capture host and linked from the group-meeting decks rather
than embedded in them. They were recovered from that host and committed here in **August 2026**;
had the account lapsed they would have been lost.

Resolution is whatever the ParaView window happened to be — 388x384 up to 1264x560, 4 to 22
seconds. They are contemporaneous evidence, not figures prepared for publication.
