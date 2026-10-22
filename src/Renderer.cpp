#include "Renderer.h"


#include <stdio.h>

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height), cam()
{
    cam.look_at(vec3(0, 0, 2),
        vec3(0, 0, 0), 
        vec3(0, 1, 0));

    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->vert_buf.resize(vert_buf_size); //todo    
    this->update_P_pers();
}


void Renderer::reset() {
    const size_t n_pixels = (size_t)fb_width * fb_height;

    for (size_t i = 0; i < n_pixels; i++)
        {
            pixels[4 * i + 0] = 0;
            pixels[4 * i + 1] = 0;
            pixels[4 * i + 2] = 0;
            pixels[4 * i + 3] = 255;
        }

        for (size_t i = 0; i < n_pixels; i++) {
            this->depth_buf[i] = INFINITY; //?
        }

}

void Renderer::render_scene(std::vector<Model> &models) {
    this->reset();

    Matrix4 VP = this->P * this->cam.V;

    auto p2scr = [this](vec3 p) {
        return vec3((p.x + 1) * 0.5 * (fb_width-1), (p.y + 1) * 0.5 * (fb_height-1), p.z);
    };


    for (const Model &model: models) {
        Matrix4 MVP = VP * model.M;
        
        for (int i = 0; i < model.mesh.num_vertices; i++) {
            vec4 v4(model.mesh.verts[i].x,
                    model.mesh.verts[i].y,
                    model.mesh.verts[i].z,
                    1.0f
            );
            vec4 tmp = MVP * v4;
            vert_buf[i].co = vec3(tmp.x, tmp.y, tmp.z);
            vert_buf[i].w = tmp.w;
        }

        for (int t = 0; t < model.mesh.num_triangles; t++) {
            Vertex c[3];
            for (int k = 0; k < 3; k++)
                c[k] = vert_buf[model.mesh.indices[3*t + k]];

            

            if (std::abs(c[0].w) < this->near || std::abs(c[1].w) < this->near || std::abs(c[2].w) < this->near)
                continue;

            if (c[0].co.z >= c[0].w || c[1].co.z >= c[1].w || c[2].co.z >= c[2].w)
                continue;

                
            for (int k = 0; k < 3; k++) {
                c[k].co = p2scr(c[k].co * (1 / c[k].w));
               
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

        p_x_min = std::floor(std::min(std::min(v1.co.x, v2.co.x), v3.co.x));
        p_y_min = std::floor(std::min(std::min(v1.co.y, v2.co.y), v3.co.y));

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
                    float depth = c[0].co.z * e1 + c[1].co.z * e2 +  c[2].co.z * e3;
                    if (depth < this->depth_buf[i]) {
                        this->depth_buf[i] = depth;

                        pixels[4 * i + 0] = 255 * e1;
                        pixels[4 * i + 1] = 255 * e2;
                        pixels[4 * i + 2] = 255 * e3;
                        pixels[4 * i + 3] = 255;
                    }

                }

            }
        }
}

void Renderer::update_P_pers(float fov, float n, float f) {
    float a = this->fb_width / (float) this->fb_height;
    this->near = n;

    float c = 1 / std::tan(fov /2);
    this->P.fill_zeros();

    this->P(0, 0) = c / a;
    this->P(1, 1) = c;
    this->P(2, 2) = (f + n) / (n - f);
    this->P(3, 3) = 0;

    this->P(2, 3) = (2 * f * n) / (n - f);
    this->P(3, 2) = -1;

};
