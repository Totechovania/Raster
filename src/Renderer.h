#include <stdint.h>
#include "scene_objects.h"
#include "vectors.h"
#include <math.h>
#include <math.h>
#include <algorithm>

struct Vertex {
    vec3 co;
    float w;
    float iw;
    vec3 n;
    vec2 ts;
    vec3 color;
};

enum class RENDER_MODE{
    RGB_TRINAGLE,
    WHITE,
    DEPTH,
    NORMALS,
    UV,
    LAMBERT
};


struct PixelShaderInfo{
    //depth filrter
    float max_depth_cur;
    float min_depth_cur;
    float max_depth_pr;
    float min_depth_pr;
    
    
    //lambert
    vec3 light_direction;
    Model *model;
    

    PixelShaderInfo() {
        //depth filrter
        max_depth_cur = -1;
        min_depth_cur = 1;

        //lambert
        light_direction = vec3(0, 1, 1).norm();
        model = NULL;
    };

    //depth filrter
    void
    reset_depth() {
        min_depth_pr = min_depth_cur;
        max_depth_pr = max_depth_cur;

        max_depth_cur = -1;
        min_depth_cur = 1;
    }

    //depth filrter
    void 
    update_depth(float d) {
        if (d > max_depth_cur) {
            max_depth_cur = d;
        } else if (d < min_depth_cur) {
            min_depth_cur = d;
        }

    }
};

struct Renderer{
    
    std::vector<uint8_t> &pixels;
    std::vector<float> depth_buf;

    std::vector<Vertex> vert_buf;

    
    int &fb_width;
    int &fb_height;

    float near;
    Matrix4 P;

    PixelShaderInfo PSI;


    Camera cam;

    

    Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size);

    void reset();

    void render_scene(std::vector<Model> &models, RENDER_MODE mode = RENDER_MODE::WHITE);
    void rasterize_triangle(Vertex *c, RENDER_MODE mode);
 
    

    void update_P_pers(float fov = 3.14 / 2, float n = 0.01, float f = 10);
};