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
| `com_position_x.csv` | Time vs. mass-centre **x**-position, 28 points over 0 ≤ t ≤ 0.5. Comma-separated, no header. Same axes as the figure above. |
| `com_position_y.csv` | The same for **y**, 29 points. |
| `COM_position.dat` | **Raw solver output** from the 2D oscillation run — 18 202 rows, whitespace-separated, header `"run_time" "COM_pos[0]" "COM_pos[1]"`, t up to 0.5017. Note the coordinate scale differs from the CSVs above: this run is centred near the origin (≈ −0.003, +0.002) rather than on the ≈ 0.08 scale of the figure, so the two are from **different configurations of the oscillation case**, not two views of one run. |

⚠ The two CSVs are 28–29 irregularly spaced points across the same interval the figure spans,
which is the signature of a curve digitised from a published plot rather than solver output.
⟦Confirm whether these are the Adami *et al.* (2010) reference curves or a coarse run of my own —
if they are the reference, they should be labelled as such and attributed.⟧

## `video/`

| File | What it shows |
|---|---|
| `square_droplet_deformation.mp4` | A square patch of water in air deforming under surface tension. |
| `square_droplet_to_circular_oscillation.mp4` | The same relaxing towards the circular equilibrium and oscillating about it. |

These are the low-Reynolds-number regime, where the shipped model behaves correctly — the
baseline against which the high-Re failure is the contrast. See
[REPRODUCING.md](../REPRODUCING.md) Part 1 for how to produce the failing case.
