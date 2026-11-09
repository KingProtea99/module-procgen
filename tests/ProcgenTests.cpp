// Procgen module tests: same seed → same points, minimum spacing holds, keep doesn't move other cells, weighted pick, shuffle
#include "procgen/sim/Placement.h"

#include <cstdio>

static int g_failed = 0;
#define CHECK(x)                                                                                                       \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            std::printf("%s(%d): FAIL %s\n", __FILE__, __LINE__, #x);                                                  \
            ++g_failed;                                                                                                \
        }                                                                                                              \
    } while (0)

using mealkit::u32;
using mealkit::Vec3;

int main() {
    procgen::Area area;
    area.center = {5, 1, -3};
    procgen::Scatter s; // 10x10 cells of 2 m, jitter 0.5
    static Vec3 a[256], b[256];
    const u32 na = procgen::scatter(area, s, 42, a, 256);
    const u32 nb = procgen::scatter(area, s, 42, b, 256);
    CHECK(na == 100 && nb == 100);
    bool same = true;
    for (u32 i = 0; i < na; ++i) same = same && a[i].x == b[i].x && a[i].z == b[i].z;
    CHECK(same);
    CHECK(procgen::scatter(area, s, 43, b, 256) == 100 && (a[0].x != b[0].x || a[0].z != b[0].z));

    const float md = procgen::minDistance(s);
    bool spaced = true, inside = true;
    for (u32 i = 0; i < na; ++i) {
        inside = inside && a[i].y == 1.0f && a[i].x >= -5.0f && a[i].x <= 15.0f && a[i].z >= -13.0f && a[i].z <= 7.0f;
        for (u32 k = i + 1; k < na; ++k) spaced = spaced && mealkit::lengthSq(a[i] - a[k]) >= md * md * 0.9999f;
    }
    CHECK(spaced && inside);

    // keep < 1 drops cells but leaves kept points where they were
    procgen::Scatter half = s;
    half.keep = 0.5f;
    const u32 nh = procgen::scatter(area, half, 42, b, 256);
    CHECK(nh > 20 && nh < 80);
    bool subset = true;
    for (u32 i = 0; i < nh; ++i) {
        bool found = false;
        for (u32 k = 0; k < na; ++k) found = found || (a[k].x == b[i].x && a[k].z == b[i].z);
        subset = subset && found;
    }
    CHECK(subset);
    CHECK(procgen::scatter(area, s, 42, b, 7) == 7);

    // Weighted pick: zero weights never come out, roughly in proportion
    mealkit::Pcg32 rng(7);
    const u32 w[] = {1, 0, 3};
    u32 hits[4]{};
    for (int i = 0; i < 4000; ++i) ++hits[procgen::pickWeighted(rng, w, 3)];
    CHECK(hits[1] == 0 && hits[3] == 0 && hits[2] > hits[0] * 2);
    const u32 zero[] = {0, 0};
    CHECK(procgen::pickWeighted(rng, zero, 2) == 2);

    // Shuffle is a permutation and repeats with the same seed
    u32 p[10], q[10];
    for (u32 i = 0; i < 10; ++i) p[i] = q[i] = i;
    mealkit::Pcg32 r1(9), r2(9);
    procgen::shuffle(r1, p, 10);
    procgen::shuffle(r2, q, 10);
    u32 sum = 0;
    bool eq = true;
    for (u32 i = 0; i < 10; ++i) sum += p[i], eq = eq && p[i] == q[i];
    CHECK(sum == 45 && eq);

    CHECK(procgen::subSeed(1, 0) == procgen::subSeed(1, 0) && procgen::subSeed(1, 0) != procgen::subSeed(1, 1));

    std::printf(g_failed ? "procgen tests: %d failed\n" : "procgen tests: all passed\n", g_failed);
    return g_failed ? 1 : 0;
}
