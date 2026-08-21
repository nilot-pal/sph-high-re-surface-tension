# Results

## figures/

**`com_oscillation_resolution_study.jpg`** — capillary oscillation (experiment 3). Mass-centre
position in x and y against time at 900, 3600 and 14 400 particles. The three resolutions collapse
onto each other, so the divergence from the benchmark is not a discretisation artefact. Density
ratio 1.

**`com_trajectory.svg`** — the 2D oscillation run plotted straight from `COM_position.dat` below,
all 18 201 steps.

## validation/

**`COM_position.dat`** — raw solver output from the 2D oscillation run. Whitespace-separated,
header `"run_time" "COM_pos[0]" "COM_pos[1]"`, 18 201 rows, t up to 0.5017. Note this run is
centred near the origin, on a different scale from the resolution study above; they are different
configurations of the oscillation case.

## video/

**`square_droplet_deformation.mp4`** — a square patch of water in air deforming under surface
tension.

**`square_droplet_to_circular_oscillation.mp4`** — the same relaxing towards circular equilibrium
and oscillating about it.

Both are the low-Reynolds regime, where the shipped model behaves. They are the baseline the
high-Re failure is measured against; [REPRODUCING.md](../REPRODUCING.md) has the failing case.
