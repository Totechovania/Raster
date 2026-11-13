#include "Renderer.h"
#include <stdio.h>


static inline float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
};

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height), PSI(), cam()
{
    cam.look_at(vec3(0, 0, 2),
        vec3(0, 0, 0), 
        vec3(0, 1, 0));

    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->vert_buf.resize(vert_buf_size);  
    this->update_P_pers();

}


inline void Renderer::reset() {
    const size_t n_pixels = (size_t)fb_width * fb_height;

    std::fill(depth_buf.begin(), depth_buf.end(),
              std::numeric_limits<float>::infinity());

    uint32_t* p32 = reinterpret_cast<uint32_t*>(pixels.data());
    std::fill(p32, p32 + (size_t)fb_width * fb_height, 0xFF000000u);

}

inline Vertex lerp_vertex(const Vertex& a, const Vertex& b, float t) {
    Vertex res;

    res.co = a.co * (1.0f - t) + b.co * t;

    res.w  = a.w  * (1.0f - t) + b.w  * t;

    res.n  = (a.n  * (1.0f - t) + b.n  * t).norm();

    res.ts = a.ts * (1.0f - t) + b.ts * t;

    return res;
}

inline int clip_near(const Vertex in[3], Vertex out[4]) {
    int n = 0;
    for (int i = 0; i < 3; ++i) {
        const Vertex& a = in[i];
        const Vertex& b = in[(i + 1) % 3];

        float da = a.co.z + a.w;
        float db = b.co.z + b.w;

        if (da >= 0.0f) out[n++] = a;

        if ((da >= 0.0f) != (db >= 0.0f)) {
            float t = da / (da - db);  
            out[n++] = lerp_vertex(a, b, t);
        }
    }

    return n; 
}

void Renderer::render_scene(std::vector<Model> &models, RENDER_MODE mode) {
    this->reset();

    Matrix4 VP = this->P * this->cam.V;


    PSI.reset_depth();
    for (Model &model: models) {
        PSI.model = &model;


        Matrix4 MVP = VP * model.M;
        
        for (int i = 0; i < model.mesh.num_vertices; i++) {
            vec4 tmp = MVP * vec4(model.mesh.verts[i]);
            vert_buf[i].co = vec3(tmp.x, tmp.y, tmp.z);
            vert_buf[i].w = tmp.w;

            vert_buf[i].n = (model.iMt * model.mesh.normals[i]).norm();

            if (model.mesh.has_uv_co) {
                vert_buf[i].ts =  model.mesh.tcs[i];
            } else {
                vert_buf[i].ts = vec2();
            }
        }

       

        for (int t = 0; t < model.mesh.num_triangles; t++) {
            Vertex c[3];
            for (int k = 0; k < 3; k++)
                c[k] = vert_buf[model.mesh.indices[3*t + k]];

            

            if (c[0].w < this->near && c[1].w < this->near && c[2].w < this->near)
                continue;


            if (c[0].co.z >= c[0].w || c[1].co.z >= c[1].w || c[2].co.z >= c[2].w)
                continue;

            
            auto p2scr = [this](vec3 p) {
                return vec3((p.x + 1) * 0.5 * (fb_width-1), (p.y + 1) * 0.5 * (fb_height-1), p.z);
            };
                
            

            Vertex cliped[4];
            int n = clip_near(c, cliped);

            for (int k = 0; k < n; k++) {
                cliped[k].iw = 1 / cliped[k].w;
                cliped[k].co = p2scr(cliped[k].co * cliped[k].iw);
            }

            if (n == 4) rasterize_triangle(cliped[0], cliped[2], cliped[3], mode);
            rasterize_triangle(cliped[0], cliped[1], cliped[2], mode);
        }
    }

}

inline void Renderer::rasterize_triangle(Vertex& v1, Vertex& v2, Vertex& v3, RENDER_MODE mode) {

    
        

        int p_x_max, p_x_min, p_y_max, p_y_min;

        p_x_max = std::ceil(std::max(std::max(v1.co.x, v2.co.x), v3.co.x));
        p_y_max = std::ceil(std::max(std::max(v1.co.y, v2.co.y), v3.co.y));

        p_x_min = std::floor(std::min(std::min(v1.co.x, v2.co.x), v3.co.x));
        p_y_min = std::floor(std::min(std::min(v1.co.y, v2.co.y), v3.co.y));

        if (p_x_max >= fb_width) p_x_max = fb_width - 1;
        if (p_x_min < 0) p_x_min = 0;
        if (p_y_max >= fb_height) p_y_max = fb_height - 1;
        if (p_y_min < 0) p_y_min = 0;


        vec3 start_p = vec3(p_x_min, p_y_min);

        float e1_row = edge_func(v2.co, v3.co, start_p);
        const float de1_row = v3.co.x - v2.co.x;
        const float de1_col = -(v3.co.y - v2.co.y);

        float e2_row = edge_func(v3.co, v1.co, start_p);
        const float de2_row = v1.co.x - v3.co.x;
        const float de2_col = -(v1.co.y - v3.co.y);

        float e3_row = edge_func(v1.co, v2.co, start_p);
        const float de3_row = v2.co.x - v1.co.x;
        const float de3_col = -(v2.co.y - v1.co.y);


        if (std::abs(e1_row + e2_row + e3_row) < 1e-7) return;

        float n_e = 1.f /(e1_row + e2_row + e3_row);

        if (n_e < 0) return;


        

        float depth;
        const float z0 = v1.co.z, z1 = v2.co.z, z2 = v3.co.z;
        const float d_depth_col = n_e * (z0*de1_col + z1*de2_col + z2*de3_col);


        for (int row = p_y_min; row < p_y_max + 1; row++) {

            float e1 = e1_row;
            float e2 = e2_row;
            float e3 = e3_row;

            bool inside = false;
            bool entered = false;

            float n_e1, n_e2, n_e3;

            int i = row * fb_width + p_x_min;

            for (int col = p_x_min; col < p_x_max + 1; col++) {
                

                if (e1 > 0 && e2 > 0 && e3 >0) {

                    entered = true;
                    if (!inside) {
                        n_e1 = e1 * n_e;
                        n_e2 = e2 * n_e;
                        n_e3 = e3 * n_e;

                        depth = z0*n_e1 + z1*n_e2 + z2*n_e3;
                    }



                    if (depth < this->depth_buf[i]) {
                        this->depth_buf[i] = depth;
                        
                        
                        if (inside) { 
                            n_e1 = e1*n_e; 
                            n_e2 = e2*n_e; 
                            n_e3 = e3*n_e; 
                        }
                        inside = true;
                        

                        switch (mode) {
                            case RENDER_MODE::WHITE: {
                                pixels[4 * i + 0] = 255;
                                pixels[4 * i + 1] = 255;
                                pixels[4 * i + 2] = 255;
                            } break;

                            case RENDER_MODE::RGB_TRINAGLE: {
                                pixels[4 * i + 0] = 255 * n_e1;
                                pixels[4 * i + 1] = 255 * n_e2;
                                pixels[4 * i + 2] = 255 * n_e3;
                            } break;

                            case RENDER_MODE::DEPTH: {
                                PSI.update_depth(depth);
                                float color;
                                if (std::abs(PSI.max_depth_pr - PSI.min_depth_pr) > 1e-10) {
                                    color = 255 * ((1 - (depth - PSI.min_depth_pr) / (PSI.max_depth_pr - PSI.min_depth_pr)) * 0.7  + 0.2);
                                } else {
                                    color = 255 * 0.7;
                                }
                                
                                pixels[4 * i + 0] = color;
                                pixels[4 * i + 1] = color;
                                pixels[4 * i + 2] = color;
                            } break;

                            case RENDER_MODE::NORMALS:
                            case RENDER_MODE::LAMBERT:
                            case RENDER_MODE::UV: {
                                float q0 = n_e1 * v1.iw;
                                float q1 = n_e2 * v2.iw;
                                float q2 = n_e3 * v3.iw;

                                float s = 1.0f / (q0 + q1 + q2);
                                
                                q0 *= s; 
                                q1 *= s; 
                                q2 *= s;
                                

                                vec3 norm_inter = (q0 * v1.n + q1 * v2.n + q2 * v3.n).norm();
                                vec2 uv{q0 * v1.ts.x + q1 * v2.ts.x + q2 * v3.ts.x, q0 * v1.ts.y + q1 * v2.ts.y + q2 * v3.ts.y};

                                if (mode == RENDER_MODE::NORMALS) {
                                    for (int ch = 0; ch < 3; ch++) {
                                        pixels[4 * i + ch] = 255 * 0.5 * (norm_inter[ch]  + 1);
                                    }
                                }
                                else if (mode == RENDER_MODE::LAMBERT) {
                                    float l_light = dot(norm_inter, PSI.light_direction);
                                    if (l_light > 1) l_light = 1;
                                    if (l_light < 0) l_light = 0;


                                    vec3 color = PSI.model->get_texture(uv.x, uv.y) * (255 * (l_light * 0.85 + 0.15));
                                    
                                    for (int ch = 0; ch < 3; ch++) {
                                        pixels[4 * i + ch] = color[ch];
                                    }
                                }
                                else if (mode == RENDER_MODE::UV){ 
                                    pixels[4 * i + 0] = 255 * uv.x;
                                    pixels[4 * i + 1] = 255 * uv.y;
                                }
                            } break;
                        }
                        
                    }

                depth += d_depth_col;

                } else if (entered) {
                    break;
                }

                
                
                e1 += de1_col; 
                e2 += de2_col; 
                e3 += de3_col; 

                i++;
            }

            e1_row += de1_row;
            e2_row += de2_row;
            e3_row += de3_row;

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