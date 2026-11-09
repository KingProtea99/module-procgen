#pragma once

#include "mealkit/core/Math.h"
#include "mealkit/core/Random.h"
#include "mealkit/core/Reflect.h"

// Procedural placement module (docs/index.md): one seed → the same placement on every peer.
// Only Pcg32 and + - * / on floats (no sqrt, no CRT trig), fixed loop order. Lockstep peers send the seed, not the map.
namespace procgen {

using mealkit::f32;
using mealkit::u32;
using mealkit::Vec3;

// Area on the XZ plane at height center.y (usually read from a scene area object: position + size / 2)
MEALKIT_STRUCT(Area) {
    MEALKIT_FIELD(Vec3, center, {}, "Center");
    MEALKIT_FIELD(Vec3, halfSize, (Vec3{10, 0, 10}), "Half size (x, z)");
};

MEALKIT_STRUCT(Scatter) {
    MEALKIT_FIELD(f32, spacing, 2.0f, "Grid cell (m)");
    MEALKIT_FIELD(f32, jitter, 0.5f, "Jitter (0 = grid, 1 = anywhere in the cell)");
    MEALKIT_FIELD(f32, keep, 1.0f, "Share of cells kept (0..1)");
};

// Guaranteed distance between any two points: cells are spacing apart and each point moves at most spacing*jitter/2 from its cell center
inline f32 minDistance(const Scatter& s) { return s.spacing * (1.0f - s.jitter); }

// Jittered grid over the area, row by row (z, then x). Writes at most maxCount points to out and returns how many.
inline u32 scatter(const Area& a, const Scatter& s, u32 seed, Vec3* out, u32 maxCount) {
    if (s.spacing <= 0.0f || maxCount == 0) return 0;
    mealkit::Pcg32 rng(seed);
    const u32 nx = static_cast<u32>(2.0f * a.halfSize.x / s.spacing);
    const u32 nz = static_cast<u32>(2.0f * a.halfSize.z / s.spacing);
    const f32 x0 = a.center.x - 0.5f * static_cast<f32>(nx) * s.spacing;
    const f32 z0 = a.center.z - 0.5f * static_cast<f32>(nz) * s.spacing;
    const f32 j = s.jitter < 0.0f ? 0.0f : (s.jitter > 1.0f ? 1.0f : s.jitter);
    const f32 half = 0.5f * s.spacing * j;
    u32 n = 0;
    for (u32 iz = 0; iz < nz && n < maxCount; ++iz)
        for (u32 ix = 0; ix < nx && n < maxCount; ++ix) {
            // always draw three numbers per cell so `keep` doesn't shift the other cells' positions
            const f32 dx = rng.range(-half, half), dz = rng.range(-half, half), roll = rng.next01();
            if (roll >= s.keep) continue;
            out[n++] = {x0 + (static_cast<f32>(ix) + 0.5f) * s.spacing + dx, a.center.y,
                        z0 + (static_cast<f32>(iz) + 0.5f) * s.spacing + dz};
        }
    return n;
}

// Index picked by integer weights (0 weights never picked). Returns count if every weight is 0.
inline u32 pickWeighted(mealkit::Pcg32& rng, const u32* weights, u32 count) {
    u32 total = 0;
    for (u32 i = 0; i < count; ++i) total += weights[i];
    if (total == 0) return count;
    u32 r = rng.below(total);
    for (u32 i = 0; i < count; ++i) {
        if (r < weights[i]) return i;
        r -= weights[i];
    }
    return count;
}

// Fisher-Yates, from the back
template <class T>
void shuffle(mealkit::Pcg32& rng, T* items, u32 count) {
    for (u32 i = count; i > 1; --i) {
        const u32 k = rng.below(i);
        T t = items[i - 1];
        items[i - 1] = items[k];
        items[k] = t;
    }
}

// Seed for a sub-part (a room, a layer) from the match seed, so adding a layer doesn't change the others
inline u32 subSeed(u32 seed, u32 part) {
    mealkit::Pcg32 rng(seed, 0x9e3779b97f4a7c15ull + part);
    return rng.next();
}

} // namespace procgen
