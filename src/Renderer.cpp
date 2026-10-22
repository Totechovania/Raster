#include "Renderer.h"


#include <stdio.h>

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height)
{
    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->vert_buf.resize(3); //todo
    this->update_P_pers();
}


void Renderer::render_scene() {
        const size_t n_pixels = (size_t)fb_width * fb_height;

        for (size_t i = 0; i < n_pixels; i++)
        {
            pixels[4 * i + 0] = 0;
            pixels[4 * i + 1] = 0;
            pixels[4 * i + 2] = 0;
            pixels[4 * i + 3] = 255;
        }

        for (size_t i = 0; i < n_pixels; i++) {
            this->depth_buf[i] = -INFINITY; //?
        }



        Matrix4 VP = this->P * this->cam.V;

        
        
        int tri[3]{0, 1, 2};

        auto p2scr = [this](vec3 p) {
            return vec3((p.x + 1) * 0.5 * fb_width, (p.y + 1) * 0.5 * fb_height);
        };

        this->vert_buf[tri[0]].co = p2scr(vec3(-0.5, -0.5));
        this->vert_buf[tri[1]].co = p2scr(vec3(1, 0));
        this->vert_buf[tri[2]].co = p2scr(vec3(0, 1));

        for (auto vert: this->vert_buf) {
            
        }

        this->vert_buf[tri[0]].color = vec3(1);
        this->vert_buf[tri[1]].color = vec3(0, 1);
        this->vert_buf[tri[2]].color = vec3(0, 0, 1);


        rasterize_triangle(tri);

}

void Renderer::rasterize_triangle(int tri[3]) {
        Vertex v1 = this->vert_buf[tri[0]];
        Vertex v2 = this->vert_buf[tri[1]];
        Vertex v3 = this->vert_buf[tri[2]];
        

        auto scr2p = [this](float x, float y) {
            return vec3(2.f * x / fb_width - 1, 2.f * y / fb_height - 1);
        };

        

        int p_x_max, p_x_min, p_y_max, p_y_min;

        p_x_max = std::ceil(std::max(std::max(v1.co.x, v2.co.x), v3.co.x));
        p_y_max = std::ceil(std::max(std::max(v1.co.y, v2.co.y), v3.co.y));

        p_x_min = std::ceil(std::min(std::min(v1.co.y, v2.co.y), v3.co.y));
        p_y_min = std::ceil(std::min(std::min(v1.co.y, v2.co.y), v3.co.y));

        if (p_x_max >= fb_width) p_x_max = fb_width - 1;
        if (p_x_min < 0) p_x_min = 0;
        if (p_y_max >= fb_height) p_y_max = fb_height - 1;
        if (p_y_min < 0) p_y_min = 0;

        for (int row = p_y_min; row < p_y_max + 1; row++) {
            for (int col = p_x_min; col < p_x_max + 1; col++) {
                vec3 p_co = vec3(col, row);
                
                float e1 = edge_func(v1.co, v2.co, p_co); 
                float e2 = edge_func(v2.co, v3.co, p_co); 
                float e3 = edge_func(v3.co, v1.co, p_co); 

                if (e1 > 0 && e2 > 0 && e3 >0) {
                    float s = (e1 + e2 + e3);
                    e1 /= s;
                    e2 /= s;
                    e3 /= s;
                        

                    int i = row * fb_width + col;
                    pixels[4 * i + 0] = 255 * (e1 * v1.color[0] + e2 * v2.color[0] + e3 * v3.color[0]);
                    pixels[4 * i + 1] = 255 * (e1 * v1.color[1] + e2 * v2.color[1] + e3 * v3.color[1]);
                    pixels[4 * i + 2] = 255 * (e1 * v1.color[2] + e2 * v2.color[2] + e3 * v3.color[2]);
                    pixels[4 * i + 3] = 255;

                }

            }
        }
}

void Renderer::update_P_pers(float fov, float a, float n, float f) {
    this->near = n;

    float c = 1 / std::tan(fov /2);
    this->P.fill_identity();

    this->P(0, 0) = c / a;
    this->P(1, 1) = c;
    this->P(2, 2) = (f + n) / (n - f);
    this->P(3, 3) = 0;

    this->P(2, 3) = (2 * f * n) / (n - f);
    this->P(3, 2) = -1;

};
