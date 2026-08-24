# High-Reynolds-number surface tension in SPHinXsys

Working on supercooled-large-droplet impingement for in-flight icing, I found that the multiphase
surface-tension model then shipping in [SPHinXsys](https://github.com/Xiangyu-Hu/SPHinXsys) breaks
down at high Reynolds number. In the square-droplet equilibrium test at Re ≈ 10³ the fluid
particles do not merely disorder — they disappear from the domain.

This repo holds the experiments that established that, the parameter studies behind them, an HLLC
Riemann solver I wrote while chasing it, and the data. It is 2023–24 research code, kept as a
record. See [Limitations](#limitations) before relying on any of it.

The central result reproduces from stock upstream code by changing two numbers.
[`cases/square_droplet_high_re.cpp`](cases/square_droplet_high_re.cpp) is that case, ready to drop
in; [REPRODUCING.md](REPRODUCING.md) has the build steps.

## The experiments

Each one rules out an explanation for the last.

| | Setup | Result |
|---|---|---|
| 1 | 3D droplet impact, Re 7154 / We 259, default surface tension | Droplet breaks up instead of spreading |
| 2a | Square droplet relaxing to a circle, Re ≈ 1 | Correct: a clean circle at rest |
| 2b | Same, Re ≈ 1025 | All fluid particles gone by t ≈ 0.4 |
| 2c | Same, maximally dissipative Riemann solver | Equilibrium of a sort, but the droplet drifts off-centre and particles cross the wall |
| 3 | Capillary oscillation vs. Adami *et al.* (2010) | Mass-centre trajectory diverges from the benchmark even at low Re, once an initial velocity field is imposed |
| 4 | 2D impact, dissipative solver | Droplet disintegrates; particles leave a domain 20× the droplet radius |

Experiment 3 decides it. The failure survives at low Re as soon as the interface is moving, so the
cause is not insufficient dissipation at high Re — it is how curvature is computed on fast-moving
interfaces. The three particle resolutions in
[that figure](results/figures/com_oscillation_resolution_study.jpg) lie on top of each other, so it
is not a discretisation artefact either.

Why it had gone unnoticed: I went through every example shipped with the library and tabulated the
Reynolds and Weber number each one actually exercises
([`docs/sphinxsys-example-audit.md`](docs/sphinxsys-example-audit.md)). The high-Re examples are
single-phase, where surface tension never enters. The multiphase ones run at low Re, or drop
viscosity and surface tension altogether. Nothing in the suite put high Re and surface tension in
the same simulation.

## The parameter studies

The table above is the diagnostic sequence. Underneath it sits the parameter work that chose the
operating point and then moved one factor at a time away from it — a screening design over
reference velocity and viscosity, a one-factor-at-a-time sweep of density, viscosity, surface
tension and length scale, a ladder of dissipation-limiter settings, and a 2 × 2 factorial on
Poiseuille flow, which is the only case here with a closed-form answer to check against.

**[`experiments/`](experiments/) has the factor tables, the recorded responses, and video of every
run.** Two results from it are worth pulling forward:

- The reference velocity is not the culprit. The case is unstable at the velocity the physics
  dictates, and there is no stable operating point that is still the problem you wanted to solve.
- Driving the limiter to η = 1e20 does **not** recover the dissipative solver — 7.8 against 5.37
  on the same case. The clamp in the acoustic form zeroes the dissipation on expansion however
  large η gets, so the family η sweeps does not contain the dissipative solver as an endpoint.
  That is a caution for anyone planning to tune η against experiment.

## The code

| File | Change |
|---|---|
| [`src/riemann_solver.cpp`](src/riemann_solver.cpp) | Adds `AcousticRiemannSolver::UHllc` — an HLLC solver: wave-speed estimates `S_L`/`S_R`, contact-wave speed `S*`, four-branch intermediate-state selection returning (ρ\*, u\*, v\*). The library shipped a linearised Roe-type solver, whose implicit dissipation was the first suspect. |
| [`src/fluid_dynamics_inner.hpp`](src/fluid_dynamics_inner.hpp) | HLLC wired into `BaseIntegration1stHalf::interaction` for fluid–fluid pairs. The intermediate density goes back through the equation of state for the momentum term; the contact velocity projected on `e_ij` drives density dissipation. |
| [`src/fluid_dynamics_complex.hpp`](src/fluid_dynamics_complex.hpp) | Same for fluid–wall pairs, with the equation of state written out, `p* = ρ_ref c²/γ · ((ρ*/ρ_ref)^γ − 1)`, projected on the wall normal. |

For the impact cases I also swapped the built-in multiphase surface tension for a single-phase
inter-particle-force model, pairwise attractive/repulsive with the force scaled by the physical
surface-tension coefficient.

## Validation

Spread factor against published experiment, D = 2.71 mm, V = 2.64 m/s, Re 7154, We 259:

| | Max spread factor |
|---|---|
| 2D | 5.37 |
| 3D | 3.74 |
| Experiment | 4.5 |

The gap is not the surface-tension model. It tracks the Riemann solver's implicit dissipation —
with the dissipative solver the unphysical splash goes away and spreading matches the reference
closely, at the cost of over-damping.

That leaves an open problem. In Riemann-based SPH the dissipation limiter η₂ produces its
dissipation implicitly, and there is no quantitative relation between the value set and the
dissipation obtained. Matching experiment by tuning η₂ is then no better justified than tuning an
artificial-viscosity coefficient. Meng *et al.* derive an equivalence between η₂ and the
artificial-viscosity parameter α, which is the obvious way in. I have not seen it closed.

Data and figures: [`results/`](results).

## What happened afterwards

I raised the problem upstream:

- [#378](https://github.com/Xiangyu-Hu/SPHinXsys/issues/378) (Aug 2023) — cannot reproduce the
  Adami *et al.* (2010) oscillation benchmark
- [#497](https://github.com/Xiangyu-Hu/SPHinXsys/issues/497) (Dec 2023) — the high-Re failure, with
  the parameter study

The maintainers later traced it to *zero-surface-energy modes*: the surface-tension contributions
of symmetrically placed neighbours cancel, so the force is underestimated. They fixed it with a
momentum-conserving penalty force and reached Re = 10,000, We = 25,000:

> S. Zhang, S.D.N. Lourenço, X. Hu. *Multiphase SPH for surface tension: resolving
> zero-surface-energy modes and achieving high Reynolds number simulations.*
> Computer Methods in Applied Mechanics and Engineering 444 (2025) 118147.
> [arXiv:2503.16082](https://arxiv.org/abs/2503.16082)

The acknowledgements read: *"Xiangyu Hu appreciates the discussions on high-speed drop impact with
Rui Qiao and Nilotpal Chakraborty from Virginia Tech."* The fix shipped in SPHinXsys v1.2.

I reported the failure and mapped where it showed up. The mechanism, the remedy and the high-Re
results are the authors' work, and I am not an author on that paper.

## Limitations

- Never submitted upstream. The files it patches — `fluid_dynamics_inner.hpp`,
  `fluid_dynamics_complex.hpp` — no longer exist in current SPHinXsys, which was restructured
  around `fluid_integration.hpp` and a `shared_ck/` tree. This will not apply to a modern checkout.
- The sources are as they stood in November 2023, against **v1.0-beta.08** (the current release
  from May 2023 to April 2024, and the last with these paths). Full files rather than patches, so
  diffing against that tag is the quickest way to see what changed.
- The HLLC path is 2D. `UHllc` returns (ρ\*, u\*, v\*) and passes the transverse component through.
- Some alternative formulations are commented in place. This is an investigation, not a release.
- The impact case files are gone, so the spread-factor numbers above are reported rather than
  reproducible from here. The high-Re failure reproduces from the stock example regardless.
  What survives of those runs is the settings and video in [`experiments/`](experiments/) —
  contemporaneous evidence that they ran and what they produced, not a route to re-running them.
- The problem itself is solved — by the CMAME work above, not by this code. What is still worth
  something here is the diagnostic sequence, the parameter studies, the example audit, and the
  HLLC integration.

## Licence

Apache 2.0, from SPHinXsys. The three files in [`src/`](src) are modified SPHinXsys sources, not
original in their entirety — see [`NOTICE`](NOTICE). Upstream:

> C. Zhang, M. Rezavand, Y. Zhu, Y. Yu, D. Wu, W. Zhang, J. Wang, X. Hu. *SPHinXsys: An
> open-source multi-physics and multi-resolution library based on smoothed particle hydrodynamics.*
> Computer Physics Communications 267 (2021) 108066.

Nilotpal Chakraborty. Work done 2023–24 at Virginia Tech under Prof. Rui Qiao, with discussions
with Prof. Xiangyu Hu's group at TUM.
