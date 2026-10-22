#include "Renderer.h"


#include <stdio.h>

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height)
{
    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->vert_buf.resize(vert_buf_size); //todo    
    this->update_P_pers();
}


void Renderer::render_scene(std::vector<Model> &models) {
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
            return vec3((p.x + 1) * 0.5 * fb_width, (p.y + 1) * 0.5 * fb_height, p.z);
        };


        for (const Model &model: models) {
            Matrix4 MVP = VP * model.M;

            for (int i = 0; i < model.mesh.num_vertices; i++) {
                vec4 tmp = MVP * model.mesh.verts[i];
                vert_buf[i].co = vec3(tmp.x, tmp.y, tmp.z);
                vert_buf[i].w = tmp.w;
            }

            for (int t = 0; t < model.mesh.num_triangles; t++) {
                Vertex c[3];
                for (int k = 0; k < 3; k++)
                    c[k] = vert_buf[model.mesh.indices[3*t + k]];

                if (c[0].w < this->near || c[1].w < this->near || c[2].w < this->near)
                    continue;

                for (int k = 0; k < 3; k++) {
                    c[k].co = p2scr(c[k].co * c[k].w);
                }

                rasterize_triangle(c);
            }
        }

}

void inline Renderer::rasterize_triangle(Vertex *c) {
        Vertex &v1 = c[0];
        Vertex &v2 = c[1];
        Vertex &v3 = c[2];
        

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
                    pixels[4 * i + 0] = 255 * e1;
                    pixels[4 * i + 1] = 255 * e2;
                    pixels[4 * i + 2] = 255 * e3;
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
