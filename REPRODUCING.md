# Reproducing this

**Read this before cloning.** This repository is not a buildable project. It is three modified
source files from SPHinXsys plus the evidence around them, and it will not compile on its own.

What that means in practice, split honestly:

| | Reproducible? |
|---|---|
| **The central finding** — the shipped multiphase surface-tension model fails at high Re | ✅ **Yes, from stock upstream code.** No file from this repository is needed. See Part 1. |
| **The HLLC solver** — building and running it | ⚠ Buildable by overwriting three files in a v1.0-beta.08 checkout. See Part 2. |
| **The droplet-impact validation** — the Re 7154 / We 259 spread-factor numbers | ❌ **No.** The case files are lost. See Part 3. |

---

## Part 1 — Reproduce the failure with no code from this repo

⭐ This is the part that matters, and it needs nothing from me. The failure appears in
SPHinXsys's own shipped example, `test_2d_square_droplet`, by changing **two numbers**.

```bash
git clone https://github.com/Xiangyu-Hu/SPHinXsys.git
cd SPHinXsys && git checkout v1.0-beta.08
```

Build with the upstream `Dockerfile` at the repository root (simplest — it pins Boost, TBB, Eigen
and SimBody, which is most of the difficulty), or follow the upstream build instructions for that
tag.

**1a — baseline, low Re.** Build and run `tests/2d_examples/test_2d_square_droplet` unmodified. A
square patch of water in air relaxes to a **perfect circle** and comes to rest. The stock
parameters in `src/droplet.cpp` are:

```cpp
Real rho0_f = 1.0;    /**< Reference density of water. */
Real rho0_a = 0.001;  /**< Reference density of air. */
Real U_max  = 1.0;
Real mu_f   = 0.2;    /**< Water viscosity. */
Real mu_a   = 0.0002; /**< Air viscosity. */
```

**1b — the failure, high Re.** Raise both densities by 10³, holding the density ratio at 1000 and
the viscosity ratio at 100:

```cpp
Real rho0_f = 1000.0;  // was 1.0
Real rho0_a = 1.0;     // was 0.001
// mu_f, mu_a, U_max unchanged
```

Re rises by three orders of magnitude. **Expected result: the droplet never stabilises, and by
t ≈ 0.4 every fluid particle has disappeared from the domain.** Not a distorted interface —
annihilation.

**1c — rule out insufficient dissipation.** Repeat 1b with the maximally dissipative Riemann
solver. The droplet now reaches an equilibrium of sorts, but **migrates off-centre and water
particles pass through the solid wall.** Still unphysical, so the cause is not the amount of
dissipation.

**1d — the experiment that settles it.** Run the capillary-oscillation case with an imposed
initial velocity field and compare the mass-centre trajectory of the upper-right quadrant against
Adami *et al.* (2010). It diverges **even at low Re**. Since the failure survives at low Re once
the interface is given a velocity, it is not a high-Re dissipation problem — it is curvature
treatment on fast-moving interfaces.

Reference trajectory data: [`results/validation/COM_position.dat`](results/validation/COM_position.dat).

> The mechanism behind all of this was later identified by the maintainers as **zero-surface-energy
> modes** and fixed — see the README. If you build a version at or after **v1.2**, these
> experiments will *not* fail, which is the point.

## Part 2 — Building the HLLC solver

The three files in [`src/`](src) are complete modified sources against **v1.0-beta.08**. They map
onto upstream as:

| This repo | Upstream path |
|---|---|
| `src/riemann_solver.cpp` | `src/shared/materials/riemann_solver.cpp` |
| `src/fluid_dynamics_inner.hpp` | `src/shared/particle_dynamics/fluid_dynamics/fluid_dynamics_inner.hpp` |
| `src/fluid_dynamics_complex.hpp` | `src/shared/particle_dynamics/fluid_dynamics/fluid_dynamics_complex.hpp` |

```bash
# from a v1.0-beta.08 checkout, with this repo cloned alongside as ../sph-high-re-surface-tension
cp ../sph-high-re-surface-tension/src/riemann_solver.cpp          src/shared/materials/
cp ../sph-high-re-surface-tension/src/fluid_dynamics_inner.hpp    src/shared/particle_dynamics/fluid_dynamics/
cp ../sph-high-re-surface-tension/src/fluid_dynamics_complex.hpp  src/shared/particle_dynamics/fluid_dynamics/
```

Diff before copying — that is the fastest way to see what changed:

```bash
git diff --stat
```

⚠ **Only against v1.0-beta.08.** Current SPHinXsys has no `fluid_dynamics_inner.hpp` or
`fluid_dynamics_complex.hpp`; the library was restructured around `fluid_integration.hpp` and a
`shared_ck/` compute-kernel tree. These files will not apply to a modern checkout, and porting
them would be a rewrite rather than a rebase.

⚠ **The HLLC path is 2D.** `UHllc` returns (ρ\*, u\*, v\*) and carries the transverse component
through unchanged.

## Part 3 — What cannot be reproduced

**The droplet-impact case files are lost.** The 2D and 3D impact simulations behind the
spread-factor comparison (D = 2.71 mm, V = 2.64 m/s, Re 7154, We 259) used a case built on top of
SPHinXsys that is not in my possession — it lived on machines I no longer have. What survives is the
figures, videos and oscillation-case data in [`results/`](results) — none of which is from the
impact runs.

So the impact numbers in the README are **reported, not reproducible from this repository.** They
are traceable to the write-up in [`docs/`](docs), which is the contemporaneous record, and the
experimental reference is Yang *et al.* Anyone wanting to re-derive them would rebuild the case
from the description there.

I would rather say this plainly than ship a repository that implies more than it can deliver.
