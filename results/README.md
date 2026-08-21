# Results

Everything here is output of, or extracted from, the experiments described in the
[README](../README.md). Nothing in this folder is output of a stock SPHinXsys example.

## `figures/`

**`com_oscillation_resolution_study.jpg`** — the capillary-oscillation case (experiment 3).
Mass-centre position of the droplet in the **X** and **Y** directions against time, at three
particle resolutions: **900, 3600 and 14 400 particles**. The three resolutions collapse onto each
other, so the divergence from the published benchmark is **not** a discretisation artefact — which
is what makes this experiment decisive. Density ratio 1.

## `validation/`

| File | What it is |
|---|---|
| `COM_position.dat` | **Raw solver output** from the 2D oscillation run — 18 202 rows, whitespace-separated, header `"run_time" "COM_pos[0]" "COM_pos[1]"`, t up to 0.5017. |

Two extracted curve files were removed before publication: their provenance could not be
established — 28–29 irregularly spaced points is the signature of a curve digitised from a
published plot rather than solver output, and I could not confirm whether they were reference data
or a coarse run of my own. Unattributed data of unknown origin does not belong in a public repo.

## `video/`

| File | What it shows |
|---|---|
| `square_droplet_deformation.mp4` | A square patch of water in air deforming under surface tension. |
| `square_droplet_to_circular_oscillation.mp4` | The same relaxing towards the circular equilibrium and oscillating about it. |

These are the low-Reynolds-number regime, where the shipped model behaves correctly — the
baseline against which the high-Re failure is the contrast. See
[REPRODUCING.md](../REPRODUCING.md) Part 1 for how to produce the failing case.
