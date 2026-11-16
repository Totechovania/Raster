#include <vector>

#include "scene_objects.h"
#include "vectors.h"

class PerlinNoise {
public:
    uint32_t seed;
    std::vector<vec2> gradients;

    explicit PerlinNoise(uint32_t seed = 0) : seed(seed) {
        gradients.resize(256);

        for (int i = 0; i < 256; ++i) {
            uint32_t h = hash_u32(i, 0x9E3779B9u, seed);
            float a = 2.0f * 3.14159265f * ((h & 0xFFFFFF) / 16777216.0f);
            gradients[i] = vec2(std::cos(a), std::sin(a));
        }
    }

    static inline uint32_t hash_u32(int x, int y, uint32_t seed) {
        uint32_t h = seed;
        h ^= (uint32_t)x * 0x27D4EB2Du;
        h ^= (uint32_t)y * 0x165667B1u;
        h ^= h >> 15;  h *= 0x85EBCA77u;
        h ^= h >> 13;  h *= 0xC2B2AE3Du;
        h ^= h >> 16;
        return h;
    }

    inline vec2 grad(int ix, int iy) const {
        return gradients[hash_u32(ix, iy, seed) & 255];
    }

    static inline float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    float perlin(vec2 p) const {
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

    float fbm(vec2 p, int octaves = 6) const {

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

    void
    fill_perlin(std::vector<float> &vec) {}
};