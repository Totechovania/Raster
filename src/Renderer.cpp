#include "Renderer.h"
#include <stdio.h>


static inline float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
}; 

Renderer::Renderer(std::vector<uint8_t> &pixels, int &fb_width, int &fb_height, int vert_buf_size, Skybox *skybox)
    : pixels(pixels), fb_width(fb_width), fb_height(fb_height), PSI(), skybox(skybox), cam()
{
    cam.look_at(vec3(0, 0, 2),
        vec3(0, 0, 0), 
        vec3(0, 1, 0));

    this->depth_buf.resize((size_t)fb_width * fb_height);
    this->vert_buf.resize(vert_buf_size);  
    this->update_P_pers();

}


inline void Renderer::reset() {
    constexpr float DEPTH_FAR = 1e30f;
    std::fill(depth_buf.begin(), depth_buf.end(),
              DEPTH_FAR);

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

    

    //skybox
    constexpr float DEPTH_FAR = 1e30f;
    if (skybox != nullptr) {

        vec3 f = (cam.target - cam.eye).norm();       
        vec3 s = (cross(f, cam.up)).norm();         
        vec3 u = cross(s, f);

        const float inv_Px  = 1 / P(0, 0);
        const float inv_Py  = 1 / P(1, 1);

        const float inv_w = 2.0f / fb_width;
        const float inv_h = 2.0f / fb_height;


        for (int row = 0; row < fb_height; row++) {
            float ny = ((row + 0.5f) * inv_h - 1.0f) * inv_Py;
            vec3 u_ny =  u * ny;

            for (int col = 0; col < fb_width; col++) {
                int i = row * fb_width + col;

                if (depth_buf[i] != DEPTH_FAR) continue;
                //std::cout << "yes" << std::endl;



                /*
                float ndc_x = 2.0f * (col + 0.5f) / (float)fb_width  - 1.0f;
                float ndc_y = 2.0f * (row + 0.5f) / (float)fb_height - 1.0f;

                vec4 RA = invVP * vec4(ndc_x, ndc_y, -1.0f, 1.0f);
                vec4 RB = invVP * vec4(ndc_x, ndc_y, +1.0f, 1.0f);
                RA = RA * (1.0f / RA.w);
                RB = RB * (1.0f / RB.w);
                vec3 d(RB.x - RA.x, RB.y - RA.y, RB.z - RA.z); 
                */

                float nx = ((col+ 0.5f) * inv_w - 1.0f) * inv_Px;     
                    
                vec3 d = f + s * nx + u_ny;

                float adx = std::abs(d.x), ady = std::abs(d.y), adz = std::abs(d.z);

                Texture* src = nullptr;
                float sc = 0.f, tc = 0.f, ma = 0.f;

                if (adx >= ady && adx >= adz) {
                    ma = adx;
                    if (d.x > 0) { src = skybox->rt; sc = -d.z; tc = -d.y; }  // +X
                    else         { src = skybox->lf; sc = +d.z; tc = -d.y; }  // -X
                } else if (ady >= adz) {
                    ma = ady;
                    if (d.y > 0) { src = skybox->up; sc = -d.x; tc = -d.z; }  // +Y
                    else         { src = skybox->dn; sc = +d.x; tc = -d.z; }  // -Y
                } else {
                    ma = adz;
                    if (d.z > 0) { src = skybox->bk; sc = +d.x; tc = -d.y; }  // +Z
                    else         { src = skybox->ft; sc = -d.x; tc = -d.y; }  // -Z
                }

                if (!src || ma < 1e-6f) continue; 

                float u = 0.5f * (sc / ma + 1.0f);
                float v = 0.5f * (tc / ma + 1.0f);
                v = 1 - v;

                u = std::clamp(u, 0.0f, 1.0f);
                v = std::clamp(v, 0.0f, 1.0f);

                vec3 color = src->get_texture(u, v, 0); 

                pixels[4 * i + 0] = color[0] * 255;
                pixels[4 * i + 1] = color[1] * 255;
                pixels[4 * i + 2] = color[2] * 255;
            }
        }   
    }

}


inline float Renderer::mip_from_depth(float d, int max_mip) const {

    float t = (log_f + 1 - std::log2((d) * (near + 2 - far) + (near + 2 + far))) * i_log_fn;

    float mip = t * max_mip;

    if (mip < 0)        mip = 0;
    if (mip > max_mip)  mip = max_mip;

    return mip;
}

static inline vec3 mip_debug_color(int mip, int max_mip) {
    // Отображаем уровень в hue: mip=0 → красный (h=0),
    // mip=max_mip → фиолетовый (h≈0.9). Деление на (max_mip+1)
    // гарантирует, что последний уровень не совпадёт по цвету с первым.
    const float h = (float)mip / (float)(max_mip + 1);

    // HSV → RGB при s = 1, v = 1
    const float h6 = h * 6.0f;
    const int   i  = (int)h6;
    const float f  = h6 - (float)i;
    const float q  = 1.0f - f;

    switch (i % 6) {
        case 0:  return vec3(1.0f, f,    0.0f);   // R→Y
        case 1:  return vec3(q,    1.0f, 0.0f);   // Y→G
        case 2:  return vec3(0.0f, 1.0f, f);      // G→C
        case 3:  return vec3(0.0f, q,    1.0f);   // C→B
        case 4:  return vec3(f,    0.0f, 1.0f);   // B→M
        default: return vec3(1.0f, 0.0f, q);      // M→R
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


        int max_mip = 0;

        float dC_dx_H;
        float dC_dy_W;
        float dC_dx_W;
        float dC_dy_H;

        float dP_u_dx;
        float dP_u_dy;
        float dP_v_dx;
        float dP_v_dy;
        if (mode == RENDER_MODE::MIP_LEVELS_DER || mode == RENDER_MODE::LAMBERT) {
            if (PSI.model->texture != nullptr) {
                max_mip = PSI.model->texture->max_mip;

                const float dq0_dx = n_e * de1_col * v1.iw;
                const float dq1_dx = n_e * de2_col * v2.iw;
                const float dq2_dx = n_e * de3_col * v3.iw;

                const float dq0_dy = n_e * de1_row * v1.iw;
                const float dq1_dy = n_e * de2_row * v2.iw;
                const float dq2_dy = n_e * de3_row * v3.iw;

                int W = PSI.model->texture->w;
                int H = PSI.model->texture->h;

                const float dC_dx = (dq0_dx + dq1_dx + dq2_dx);
                const float dC_dy = (dq0_dy + dq1_dy + dq2_dy); 

                dC_dx_H = dC_dx * H;
                dC_dx_W = dC_dx * W;
                dC_dy_H = dC_dy * H;
                dC_dy_W = dC_dy * W;

                dP_u_dx = (dq0_dx * v1.ts.x + dq1_dx * v2.ts.x + dq2_dx * v3.ts.x) * W;
                dP_u_dy = (dq0_dy * v1.ts.x + dq1_dy * v2.ts.x + dq2_dy * v3.ts.x) *  W; 

                dP_v_dx = (dq0_dx * v1.ts.y + dq1_dx * v2.ts.y + dq2_dx * v3.ts.y) * H;
                dP_v_dy = (dq0_dy * v1.ts.y + dq1_dy * v2.ts.y + dq2_dy * v3.ts.y) * H;
            }
        }


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

                            case RENDER_MODE::MIP_LEVELS_DEPTH: {
                                const int max_mip = 10;

                                int mip = (int) mip_from_depth(depth, max_mip);

                                vec3 color = mip_debug_color(mip, max_mip);


                                pixels[4 * i + 0] = color[0] * 255;
                                pixels[4 * i + 1] = color[1] * 255;
                                pixels[4 * i + 2] = color[2] * 255;


                            } break;

                            case RENDER_MODE::NORMALS:
                            case RENDER_MODE::LAMBERT:
                            case RENDER_MODE::MIP_LEVELS_DER:
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
            
                                int m0; 
                                int m1;
                                float f;
                                if (mode == RENDER_MODE::LAMBERT || mode == RENDER_MODE::MIP_LEVELS_DER) {
                                    if (PSI.model->texture != nullptr) {
                                        const float nu_x = dP_u_dx - uv[0] * dC_dx_W;
                                        const float nv_x = dP_v_dx - uv[1] * dC_dx_H;
                                        const float nu_y = dP_u_dy - uv[0] * dC_dy_W;
                                        const float nv_y = dP_v_dy - uv[1] * dC_dy_H;

                                        const float M = std::max(nu_x*nu_x + nv_x*nv_x,
                                                                nu_y*nu_y + nv_y*nv_y);
                                        float lod = 0.5f * std::log2f(M * s * s);

                                        if (lod < 0.0f)                    lod = 0.0f;
                                        if (lod > (float) max_mip)    lod = (float) max_mip;


                                        m0 = (int)lod;
                                        m1 = (m0 + 1 < max_mip) ? m0 + 1 : max_mip;
                                        f  = lod - (float)m0;
                                    } else {
                                        m0 = 0; 
                                        m1 = 0;
                                        f = 0;
                                    }

                                }
                                

                                if (mode == RENDER_MODE::NORMALS) {
                                    for (int ch = 0; ch < 3; ch++) {
                                        pixels[4 * i + ch] = 255 * 0.5 * (norm_inter[ch]  + 1);
                                    }
                                }
                                else if (mode == RENDER_MODE::LAMBERT) {
                                    float l_light = dot(norm_inter, PSI.light_direction);
                                    if (l_light > 1) l_light = 1;
                                    if (l_light < 0) l_light = 0;

                                    vec3 color = PSI.model->get_texture(uv.x, uv.y, m0) * (1.0f - f) + PSI.model->get_texture(uv.x, uv.y, m1) * f;

                                    color = color * (255 * (l_light * 0.85 + 0.15));
                                    
                                    for (int ch = 0; ch < 3; ch++) {
                                        pixels[4 * i + ch] = color[ch];
                                    }
                                }
                                else if (mode == RENDER_MODE::UV){ 
                                    pixels[4 * i + 0] = 255 * uv.x;
                                    pixels[4 * i + 1] = 255 * uv.y;
                                } 
                                else if (mode == RENDER_MODE::MIP_LEVELS_DER) {
                                    //const vec3  c  = model.get_texture(u, v, m0) * (1.0f - f) + model.get_texture(u, v, m1) * f;

                                    const vec3 color = mip_debug_color(m0, max_mip) * (1.0f - f) + mip_debug_color(m1, max_mip) * f;

                                    pixels[4 * i + 0] = color[0] * 255;
                                    pixels[4 * i + 1] = color[1] * 255;
                                    pixels[4 * i + 2] = color[2] * 255;

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
    this->far = f;

    this->log_n = std::log2(this->near);
    this->log_f = std::log2(this->far);
    this->i_log_fn = 1 / (log_f - log_n);



    float c = 1 / std::tan(fov /2);
    this->P.fill_zeros();

    this->P(0, 0) = c / a;
    this->P(1, 1) = c;
    this->P(2, 2) = (f + n) / (n - f);
    this->P(3, 3) = 0;

    this->P(2, 3) = (2 * f * n) / (n - f);
    this->P(3, 2) = -1;

};