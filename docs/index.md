# Procedural placement

Seed-based placement for lockstep games: peers exchange one seed instead of a map, and every peer builds the same layout. Points in an area with a guaranteed minimum spacing, weighted picks and shuffles.

- Version 0.1.0, engine 0.9, no required modules
- No state of its own: functions take a seed or your `Pcg32`
- Simulation: `sim/Placement.h`

## Adding

```
tools\module.ps1 add -Project <your project> procgen
```

## Usage

Generate at match start, from the match seed, into a fixed array of your world state:

```cpp
#include "procgen/sim/Placement.h"

procgen::Area area;                  // from a scene area object: center and size / 2
area.center = obj.position;
area.halfSize = obj.size * 0.5f;
procgen::Scatter s;                  // keep it in your Tuning so designers can change it
s.spacing = 3.0f; s.jitter = 0.6f; s.keep = 0.7f;
const u32 n = procgen::scatter(area, s, procgen::subSeed(matchSeed, 1), points, kMaxPoints);

mealkit::Pcg32 rng(procgen::subSeed(matchSeed, 2));
for (u32 i = 0; i < n; ++i) kinds[i] = procgen::pickWeighted(rng, weights, kindCount);
```

- `scatter` walks a grid of `spacing` cells row by row and moves each point inside its cell. Any two points are at least `minDistance(s)` = `spacing * (1 - jitter)` apart.
- `keep` drops a share of cells without moving the others, so lowering it thins a layout instead of reshuffling it.
- `subSeed(seed, part)` gives each layer its own seed, so adding a layer later leaves the others where they were.
- `pickWeighted` uses integer weights (0 never picked); `shuffle` is Fisher-Yates.

Points lie at `area.center.y`; drop them onto terrain with your own height lookup or a physics ray in the game.

## How it works

Only `Pcg32` and float `+ - * /` in a fixed order, no `sqrt` or CRT trig, so results are the same on every platform the engine's determinism checks cover. Each cell always draws three numbers whether or not it is kept, which is why `keep` and `maxCount` never move other points.

Baking a layout into a `.scene` in the editor for hand-tweaking is planned once modules can add editor tools (DECISIONS 2026-11-09).

## Version history

- 0.1.0: first version
