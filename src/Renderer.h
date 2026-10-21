#include <stdint.h>
#include "scene_objects.h"
#include "vectors.h"
#include <math.h>





struct Renderer{
    
    std::vector<uint8_t> &pixels;
    std::vector<float> depth_buf;

    int &fb_width;
    int &fb_height;

    Matrix4 P;

    Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height);

    void
    render_scene(Camera &cam);

    void
    update_P_pers(float fov = 3.14 / 2, float a = 1, float n = 0.01, float f = 2);
};