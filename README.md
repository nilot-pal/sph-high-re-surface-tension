# High-Reynolds-number surface tension in SPHinXsys

**A diagnosis, an HLLC Riemann solver, and a replacement surface-tension model.**

While simulating supercooled-large-droplet impingement for in-flight icing, I found that the
multiphase surface-tension model then shipping in [SPHinXsys](https://github.com/Xiangyu-Hu/SPHinXsys)
— an open-source weakly-compressible SPH library — **fails at high Reynolds number.** Not
"degrades": in the square-droplet equilibrium test at Re ≈ 10³, *every fluid particle
disappears.*

This repository holds the work that established that, the code I wrote to get around it, and the
validation data. It is research code from 2023–24, published as a record rather than as a
maintained library. **See [Status and honest limitations](#status-and-honest-limitations).**

---

## What was found

The failure is not a bad parameter choice. It was isolated by a designed sequence of experiments,
each one removing a candidate explanation:

| # | Experiment | Result |
|---|---|---|
| 1 | 3D droplet impact, Re 1154 / We 253, default multiphase surface tension | Droplet **breaks up completely** instead of spreading |
| 2a | Square droplet relaxing to a circle, **Re ≈ 1** | ✅ Perfect circle. The model works at low Re. |
| 2b | Same test, **Re ≈ 1025** | ⛔ **All fluid particles vanish by t = 0.4.** Not instability — annihilation. |
| 2c | Same, with the maximally dissipative Riemann solver | Reaches equilibrium, but the droplet **migrates off-centre and particles pass through the wall** |
| 3 | Capillary oscillation vs. the Adami *et al.* (2010) benchmark | Diverges from the published mass-centre trajectory even at **low** Re once an initial velocity field is imposed |
| 4 | 2D impact, dissipative solver | Droplet disintegrates; particles escape a domain 20× the droplet radius |

**Conclusion:** the cause is the treatment of interface curvature on *fast-moving* interfaces, not
insufficient numerical dissipation. Experiment 3 is the one that settles it — the failure appears
at low Re as soon as the interface is given a velocity, so it cannot be a high-Re dissipation
problem.

⭐ **Why nobody had hit it.** I audited every example shipped with the library and tabulated the
Reynolds and Weber number each one actually exercises — see
[`docs/sphinxsys-example-audit.md`](docs/sphinxsys-example-audit.md). The high-Re examples are all
**single phase**, where surface tension never enters; the multiphase examples run at low Re or
omit surface tension and viscosity entirely. **There was no example in the suite that put high Re
and surface tension in the same simulation**, so the gap sat in untested territory.

## What was built

| File | What it is |
|---|---|
| [`src/riemann_solver.cpp`](src/riemann_solver.cpp) | An **HLLC approximate Riemann solver** (`AcousticRiemannSolver::UHllc`) — wave-speed estimates `S_L`/`S_R`, contact-wave speed `S*`, four-branch intermediate-state selection returning (ρ\*, u\*, v\*). The library shipped a linearised Roe-type solver whose *implicit* dissipation was the leading suspect. |
| [`src/fluid_dynamics_inner.hpp`](src/fluid_dynamics_inner.hpp) | HLLC wired into `BaseIntegration1stHalf::interaction` for **fluid–fluid** pairs: the intermediate density is pushed back through the equation of state for the momentum term, and the contact velocity projected on `e_ij` drives the density dissipation term. |
| [`src/fluid_dynamics_complex.hpp`](src/fluid_dynamics_complex.hpp) | The same for **fluid–wall** pairs, with the equation of state written explicitly, `p* = ρ_ref c²/γ · ((ρ*/ρ_ref)^γ − 1)`, projected on the wall normal. |

A replacement **inter-particle-force surface-tension model** (single-phase form, pairwise
attractive/repulsive with the force scaled by the physical surface-tension coefficient) was used
in place of the built-in multiphase model for the impact cases.

## Validation

Spread factor against published experiment at **Re 1154, We 253** (D = 2.11 mm, V = 2.04 m/s):

| | Max spread factor |
|---|---|
| This work, 2D | 5.31 |
| This work, 3D | 3.14 |
| Experiment | 4.5 |

The residual discrepancy is **not** attributed to the surface-tension model. It tracks the
Riemann solver's implicit dissipation: with the dissipative solver the unphysical splash
disappears and the spreading matches the reference closely, at the cost of over-damping.

⚠ **The open problem this exposes.** In Riemann-based SPH the dissipation limiter η₂ produces
numerical dissipation *implicitly*, and there is no quantitative relation between the value you
set and the dissipation you get — so matching experiment by tuning η₂ is indistinguishable from
tuning an artificial-viscosity coefficient, with no physical justification for the value chosen.
Meng *et al.* derive an equivalence between η₂ and the artificial-viscosity parameter α, which is
the obvious route to quantifying it. **This remains open as far as I know.**

Data: [`results/validation/`](results/validation) · figures and short videos of each experiment in
[`results/`](results).

## What happened next

I reported the failure in the project's issue tracker:

- **[#378](https://github.com/Xiangyu-Hu/SPHinXsys/issues/378)** (Aug 2023) — unable to reproduce
  the Adami *et al.* (2010) droplet-oscillation benchmark
- **[#497](https://github.com/Xiangyu-Hu/SPHinXsys/issues/497)** (Dec 2023) — the high-Re failure,
  with the parameter study, asking whether the acoustic Riemann solver lacks sufficient
  dissipation for high-Re flows

The maintainers subsequently identified the root cause as **zero-surface-energy modes** — a
numerical instability in which the surface-tension contributions of symmetrically positioned
neighbours cancel, underestimating the force — and resolved it with a momentum-conserving penalty
force, reaching Re = 10,000 and We = 25,000. That work is:

> S. Zhang, S.D.N. Lourenço, X. Hu. *Multiphase SPH for surface tension: resolving
> zero-surface-energy modes and achieving high Reynolds number simulations.*
> **Computer Methods in Applied Mechanics and Engineering 444 (2025) 118147.**
> [arXiv:2503.16082](https://arxiv.org/abs/2503.16082)

whose acknowledgements read: *"Xiangyu Hu appreciates the discussions on high-speed drop impact
with Rui Qiao and Nilotpal Chakraborty from Virginia Tech."* The fix shipped in SPHinXsys v1.2.

> **To be clear about what this repository does and does not claim.** I reported the failure and
> characterised where it appeared; I did not find the mechanism and I am not an author on that
> paper. The diagnosis of zero-surface-energy modes, the penalty-force remedy and the high-Re
> results are the authors' work. The timeline above is stated so a reader can check it, not to
> imply more than it says.

## Status and honest limitations

- **This is research code, not a contribution to SPHinXsys.** It was never submitted upstream, and
  the file layout it patches (`fluid_dynamics_inner.hpp`, `fluid_dynamics_complex.hpp`) no longer
  exists in current SPHinXsys, which has since been restructured around `fluid_integration.hpp`
  and a `shared_ck/` compute-kernel tree. **It will not apply to a modern checkout.**
- Files here are the modified versions as they stood in **November 2023**, against a late-2023
  `v1.0-beta` checkout. ⟦Exact upstream tag to confirm.⟧ They are full files, not patches, so
  diffing them against that revision is the fastest way to see the changes.
- **The HLLC solver is 2D.** It returns (ρ\*, u\*, v\*) and carries the transverse component
  through unchanged.
- Some experimental branches remain commented in and out — this is a record of an investigation,
  not a tidied release.
- The problem this was built for (supercooled large droplet impingement, FAA Appendix O icing
  certification) was set aside in early 2024 when I moved to a sponsored compressor project.
- **The surface-tension problem itself is solved** — by the CMAME work above, not by this code.
  What remains of interest here is the diagnostic sequence, the example audit, and the HLLC
  integration.

## Licence and attribution

Apache 2.0, inherited from SPHinXsys. The three files in [`src/`](src) are **modified versions of
SPHinXsys source files** and are not original in their entirety; see [`NOTICE`](NOTICE). The
upstream project is:

> C. Zhang, M. Rezavand, Y. Zhu, Y. Yu, D. Wu, W. Zhang, J. Wang, X. Hu. *SPHinXsys: An
> open-source multi-physics and multi-resolution library based on smoothed particle hydrodynamics.*
> Computer Physics Communications 267 (2021) 108066.

— Nilotpal Chakraborty · work carried out 2023–24 at Virginia Tech, advised by Prof. Rui Qiao,
in discussion with Prof. Xiangyu Hu's group at TUM.
