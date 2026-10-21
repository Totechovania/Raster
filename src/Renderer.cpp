#include "Renderer.h"
#include <math.h>

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height)
{
    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->update_P_pers();
}

void Renderer::render_scene(Camera &cam) {
        const size_t n_pixels = (size_t)fb_width * fb_height;

        for (size_t i = 0; i < n_pixels; i++)
        {
            pixels[4 * i + 0] = 0;
            pixels[4 * i + 1] = 0;
            pixels[4 * i + 2] = 0;
            pixels[4 * i + 3] = 255;
        }

        vec3 v1(-0.5, -0.5);
        vec3 v2{1, 0};
        vec3 v3{0, 1};

        

        auto scr2p = [this](float x, float y) {
            return vec3(2.f * x / fb_width - 1, 2.f * y / fb_height - 1);
        };

        auto p2scr = [this](vec3 &p) {
            return vec3((p.x + 1) * 0.5 * fb_width, (p.y + 1) * 0.5 * fb_height);
        };

        int p_x_max, p_x_min, p_y_max, p_y_min;

        p_x_max = std::ceil(std::max(std::max(p2scr(v1).x, p2scr(v2).x), p2scr(v3).x));
        p_y_max = std::ceil(std::max(std::max(p2scr(v1).y, p2scr(v2).y), p2scr(v3).y));

        p_x_min = std::ceil(std::min(std::min(p2scr(v1).x, p2scr(v2).x), p2scr(v3).x));
        p_y_min = std::ceil(std::min(std::min(p2scr(v1).y, p2scr(v2).y), p2scr(v3).y));

        if (p_x_max >= fb_width) p_x_max = fb_width - 1;
        if (p_x_min < 0) p_x_min = 0;
        if (p_y_max >= fb_height) p_y_max = fb_height - 1;
        if (p_y_min < 0) p_y_min = 0;

        for (int row = p_y_min; row < p_y_max + 1; row++) {
            for (int col = p_x_min; col < p_x_max + 1; col++) {
                vec3 p_co = scr2p(col, row);
                
                float e1 = edge_func(v1, v2, p_co); 
                float e2 = edge_func(v2, v3, p_co); 
                float e3 = edge_func(v3, v1, p_co); 

                if (e1 > 0 && e2 > 0 && e3 >0) {
                    int i = row * fb_width + col;
                    pixels[4 * i + 0] = 255 * e1 / 2;
                    pixels[4 * i + 1] = 255 * e2 / 2;
                    pixels[4 * i + 2] = 255 * e3 / 2;
                    pixels[4 * i + 3] = 255;

                }

            }
        }



}

void Renderer::update_P_pers(float fov, float a, float n, float f) {
    float c = 1 / std::tan(fov /2);
    this->P.fill_identity();

    this->P(0, 0) = c / a;
    this->P(1, 1) = c;
    this->P(2, 2) = (f + n) / (n - f);
    this->P(3, 3) = 0;

    this->P(2, 3) = (2 * f * n) / (n - f);
    this->P(3, 2) = -1;

};
