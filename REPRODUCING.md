# Reproducing this

This is not a buildable project. It is three modified SPHinXsys sources plus the evidence around
them, and it will not compile on its own.

| | |
|---|---|
| The shipped surface-tension model fails at high Re | Reproducible from stock upstream code. Nothing from this repo needed. Part 1. |
| Building the HLLC solver | Overwrite three files in a v1.0-beta.08 checkout. Part 2. |
| The Re 7154 / We 259 spread-factor numbers | Not reproducible. The case files are gone. Part 3. |

## Part 1 — the failure, from stock upstream code

The failure shows up in SPHinXsys's own `test_2d_square_droplet` example by changing two numbers.

```bash
git clone https://github.com/Xiangyu-Hu/SPHinXsys.git
cd SPHinXsys && git checkout v1.0-beta.08
```

Build with the `Dockerfile` at the repo root — it pins Boost, TBB, Eigen and SimBody, which is
most of the work — or follow the upstream build instructions for that tag.

**1a, low Re.** Run `tests/2d_examples/test_2d_square_droplet` unmodified. The square patch of
water relaxes to a circle and comes to rest. Stock parameters in `src/droplet.cpp`:

```cpp
Real rho0_f = 1.0;    /**< Reference density of water. */
Real rho0_a = 0.001;  /**< Reference density of air. */
Real U_max  = 1.0;
Real mu_f   = 0.2;    /**< Water viscosity. */
Real mu_a   = 0.0002; /**< Air viscosity. */
```

**1b, high Re.** Raise both densities by 10³, keeping the density ratio at 1000 and the viscosity
ratio at 100:

Ready-made: [`cases/square_droplet_high_re.cpp`](cases/square_droplet_high_re.cpp) is the stock
file with exactly that change. Drop it in place of `src/droplet.cpp` and rebuild.

```cpp
Real rho0_f = 1000.0;  // was 1.0
Real rho0_a = 1.0;     // was 0.001
// mu_f, mu_a, U_max unchanged
```

The droplet never stabilises, and by t ≈ 0.4 every fluid particle has left the domain.

**1c.** Repeat 1b with the maximally dissipative Riemann solver. The droplet settles, but drifts
off-centre and water particles pass through the solid wall. So the problem is not the amount of
dissipation.

**1d.** Run the capillary oscillation case with an imposed initial velocity field and compare the
mass-centre trajectory of the upper-right quadrant against Adami *et al.* (2010). It diverges at
low Re too. Since the failure appears as soon as the interface is moving, it is curvature
treatment on fast-moving interfaces rather than a high-Re dissipation problem.

Trajectory data: [`results/validation/COM_position.dat`](results/validation/COM_position.dat).

Build at v1.2 or later and none of this fails — that is the point.

## Part 2 — building the HLLC solver

The three files in [`src/`](src) are complete modified sources against v1.0-beta.08:

| This repo | Upstream |
|---|---|
| `src/riemann_solver.cpp` | `src/shared/materials/riemann_solver.cpp` |
| `src/fluid_dynamics_inner.hpp` | `src/shared/particle_dynamics/fluid_dynamics/fluid_dynamics_inner.hpp` |
| `src/fluid_dynamics_complex.hpp` | `src/shared/particle_dynamics/fluid_dynamics/fluid_dynamics_complex.hpp` |

```bash
# from a v1.0-beta.08 checkout, this repo cloned alongside
cp ../sph-high-re-surface-tension/src/riemann_solver.cpp          src/shared/materials/
cp ../sph-high-re-surface-tension/src/fluid_dynamics_inner.hpp    src/shared/particle_dynamics/fluid_dynamics/
cp ../sph-high-re-surface-tension/src/fluid_dynamics_complex.hpp  src/shared/particle_dynamics/fluid_dynamics/
git diff --stat
```

Only against v1.0-beta.08. Current SPHinXsys has neither `fluid_dynamics_inner.hpp` nor
`fluid_dynamics_complex.hpp`; porting would be a rewrite, not a rebase. The HLLC path is 2D.

## Part 3 — what cannot be reproduced

The droplet-impact case files are gone. The 2D and 3D runs behind the spread-factor comparison
(D = 2.71 mm, V = 2.64 m/s, Re 7154, We 259) sat on machines I no longer have. What survives is
the figures, videos and oscillation data in [`results/`](results), none of which is from the
impact runs. The numbers in the README are traceable to the write-up in [`docs/`](docs); the
experimental reference is Yang *et al.* Rebuilding the case from that description is the only
route back to them.
