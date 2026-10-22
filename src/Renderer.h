#include <stdint.h>
#include "scene_objects.h"
#include "vectors.h"
#include <math.h>
#include <math.h>
#include <algorithm>

struct Vertex {
    vec3 co;
    float w;
    vec3 n;
    vec2 ts;
    vec3 color;
};



struct Renderer{
    
    std::vector<uint8_t> &pixels;
    std::vector<float> depth_buf;

    std::vector<Vertex> vert_buf;

    int &fb_width;
    int &fb_height;

    float near;
    Matrix4 P;

    Camera cam;

    Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size);

    void reset();

    void render_scene(std::vector<Model> &models);

    void rasterize_triangle(Vertex *c);

    void update_P_pers(float fov = 3.14 / 2, float n = 0.01, float f = 10);
};