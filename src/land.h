#pragma once

#include <vector>
#include <cstdint>

#include "scene_objects.h"  
#include "vectors.h"
#include "mesh_utils.h"

struct PerlinNoise {
    uint32_t seed;
    std::vector<vec2> gradients;

    explicit PerlinNoise(uint32_t seed = 0);

    static uint32_t hash_u32(int x, int y, uint32_t seed);
    vec2 grad(int ix, int iy) const;
    static float fade(float t);

    float perlin(vec2 p) const;
    float fbm(vec2 p, int octaves = 8) const;

    void fill_perlin(std::vector<float> &vec, int w, int h, int octaves = 8);
};


struct Terrain {
    Mesh mesh;
    Texture* hor;
    Texture* vert;

    std::vector<float> height_map;

    Terrain();

    void
    generate_mesh(int size);


};