# Modified SPHinXsys sources

Complete modified files as they stood in **November 2023**, against a late-2023 `v1.0-beta`
checkout of SPHinXsys. Diff them against that revision to see the changes.

⚠ These will **not** apply to current SPHinXsys. `fluid_dynamics_inner.hpp` and
`fluid_dynamics_complex.hpp` no longer exist upstream; the library has been restructured around
`fluid_integration.hpp` and a `shared_ck/` compute-kernel tree.

## `riemann_solver.cpp`

Adds `AcousticRiemannSolver::UHllc(...)` — an HLLC approximate Riemann solver:

- left/right wave-speed estimates `S_L = u_L - c_L q_L`, `S_R = u_R + c_R q_R`
- contact-wave speed `S*` from the pressure/momentum jump relation
- four-branch selection of the intermediate state, returning `{rho, u, v}`

`NoRiemannSolver::UHllc` is a no-op stub so the interface is satisfied when no solver is active.

## `fluid_dynamics_inner.hpp` — fluid–fluid pairs

In `BaseIntegration1stHalf<RiemannSolverType>::interaction`:

- calls `riemann_solver_.UHllc(...)` for the interacting pair
- momentum term uses the intermediate density pushed back through the equation of state,
  `fluid_.getPressure(UHllc[0])`
- density dissipation uses the contact velocity projected on `e_ij`

## `fluid_dynamics_complex.hpp` — fluid–wall pairs

The same, with the wall state substituted for the neighbour, the equation of state written out
explicitly as `p* = rho_ref * c^2 / gamma * ((rho*/rho_ref)^gamma - 1)`, and the contact velocity
projected on the wall normal `n_k` rather than `e_ij`.

Some alternative formulations remain commented in place — this is an investigation record.
