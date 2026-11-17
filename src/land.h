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

    void fill_perlin(std::vector<float> &vec, int w, int h, int octaves = 6);
};


struct Terrain {
    int size;
    Mesh mesh;

    float i_texture_per_x;
    float i_texture_per_y;
    Texture *hor;
    Texture *vert;

    int height_map_size;
    float i_height_map_scale;
    std::vector<float> height_map;

    float len_per_step;

    std::vector<Vertex> vert_buf;

    Terrain(int size, float scale,  Texture *hor, Texture *vert, float height_map_scale=20,  int height_map_size=256): 
    size(size), hor(hor), vert(vert),  height_map_size(height_map_size) , i_height_map_scale(1 / height_map_scale)
    {
        //hor and vert shoulbe the same size
        vert_buf.resize(size * size);

        PerlinNoise perl(1234);
        mesh = Mesh();

        
        perl.fill_perlin(height_map, height_map_size, height_map_size);

        std::vector<float> heightmap_image;

        heightmap_image.resize(height_map_size * height_map_size * 3);

        for (int row = 0; row < height_map_size; row++) {
                for (int col = 0; col < height_map_size; col++) {
                    int i =  row * height_map_size + col;

                    const float val = 0.5 * height_map[i] + 0.5;

                    heightmap_image[i * 3] = val;
                    heightmap_image[i * 3 + 1] = val;
                    heightmap_image[i * 3 + 2] = val;

                }
            }
        save_image_f32_png_rgb(heightmap_image.data(), "peerlin_noize.png", height_map_size, height_map_size, 3, 1.0f);

        


        this->generate_mesh(size, scale);

        len_per_step = 2.0f * scale / (size - 1);

        const float tiling = 128.0f;
        i_texture_per_x = tiling / hor->w;
        i_texture_per_y = tiling / hor->h;

    }

    void
    generate_mesh(int size, float scale);

    inline float
    get_height(float x, float y){
    // x, y in [-1, 1]
        return  1.5 * bilinear_inter(height_map.data(), height_map_size, height_map_size, x * i_height_map_scale + 0.5 * size, y* i_height_map_scale + 0.5 * size) - 1;
    }

    inline vec2
    get_uv(float x, float y) {
        return vec2(x * i_texture_per_x, y * i_texture_per_y);
    }

    inline vec3
    get_texture(float x, float z, float f, int mip) {
        if (f <= 0.3) return vert->get_texture(x, z, mip);
        if (f >= 0.7) hor->get_texture(x, z, mip);
        return f * hor->get_texture(x, z, mip) + (1 - f) * vert->get_texture(x, z, mip);
    }


};