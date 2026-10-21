#include <stdint.h>
#include "scene_objects.h"






struct Renderer{
    
    std::vector<uint8_t> &pixels;
    std::vector<float> depth_buf;

    int &fb_width;
    int &fb_height;

    Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height);

    void
    render_scene(Camera &cam);



};