#include "land.h"

#include <cmath>

PerlinNoise::PerlinNoise(uint32_t seed) : seed(seed) {
    gradients.resize(256);
    for (int i = 0; i < 256; ++i) {
        uint32_t h = hash_u32(i, 0x9E3779B9u, seed);
        float a = 2.0f * 3.14159265f * ((h & 0xFFFFFF) / 16777216.0f);
        gradients[i] = vec2(std::cos(a), std::sin(a));
    }
}

uint32_t PerlinNoise::hash_u32(int x, int y, uint32_t seed) {
    uint32_t h = seed;
    h ^= (uint32_t)x * 0x27D4EB2Du;
    h ^= (uint32_t)y * 0x165667B1u;
    h ^= h >> 15;  h *= 0x85EBCA77u;
    h ^= h >> 13;  h *= 0xC2B2AE3Du;
    h ^= h >> 16;
    return h;
}

vec2 PerlinNoise::grad(int ix, int iy) const {
    return gradients[hash_u32(ix, iy, seed) & 255];
}

float PerlinNoise::fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float PerlinNoise::perlin(vec2 p) const {
    float fx = std::floor(p.x), fy = std::floor(p.y);
    int ix = (int)fx, iy = (int)fy;
    vec2  f(p.x - fx, p.y - fy);

    float n00 = dot(grad(ix,     iy    ), f);
    float n10 = dot(grad(ix + 1, iy    ), f - vec2(1, 0));
    float n01 = dot(grad(ix,     iy + 1), f - vec2(0, 1));
    float n11 = dot(grad(ix + 1, iy + 1), f - vec2(1, 1));

    float ux = fade(f.x), uy = fade(f.y);
    float a = n00 + (n10 - n00) * ux;
    float b = n01 + (n11 - n01) * ux;
    return a + (b - a) * uy;
}

float PerlinNoise::fbm(vec2 p, int octaves) const {
    const float c = 0.8f, s = 0.6f;
    float sum = 0.0f, amp = 1.0f, norm = 0.0f;
    vec2  q = p;
    for (int k = 0; k < octaves; ++k) {
        sum  += amp * perlin(q);
        norm += amp;
        q = vec2(c*q.x - s*q.y, s*q.x + c*q.y) * 2.0f + vec2(17.3f, 9.1f);
        amp *= 0.5f;
    }
    return sum / norm;
}

void PerlinNoise::fill_perlin(std::vector<float> &vec, int w, int h, int octaves) {
    vec.resize(w * h);

    const float scale = 1.0f / 32.0f;

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int i = row * w + col;
            vec[i] = fbm(vec2(col, row) * scale, octaves);
        }
    }
}

void Terrain::generate_mesh(int size, float scale) {
    mesh.verts.resize(size * size);
    mesh.num_vertices  = size * size;
    mesh.num_triangles = (size - 1) * (size - 1) * 2;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float y = scale * (((2.0f *  i) / (size - 1)) - 1);
            float x = scale * (((2.0f *  j) / (size - 1)) - 1);
            mesh.verts[i * size + j] = vec3(x, 0, y);
        }
    }
    

    
    mesh.indices.resize((size - 1) * (size - 1) * 3 * 2);

    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {

            int ind = i * (size - 1) + j;
            int v1 = i * size + j;
            int v2 = v1 + 1;
            int v3 = v1 + size;
            int v4 = v2 + size;

            mesh.indices[6 * ind] = v1;
            mesh.indices[6 * ind + 1] = v4;
            mesh.indices[6 * ind + 2] = v2;
            
            mesh.indices[6 * ind + 3] = v1;
            mesh.indices[6 * ind + 4] = v3;
            mesh.indices[6 * ind + 5] = v4;
        }
    }



}
