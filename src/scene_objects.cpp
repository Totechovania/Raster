#include "scene_objects.h"

void Camera::sync_angles()
{
    vec3 f = (target - eye).norm();
    pitch = asinf(std::clamp(f.y, -1.0f, 1.0f));
    yaw   = atan2f(f.z, f.x); 
}

Matrix4 Camera::update_V()
{
    vec3 z_c = (this->eye - this->target).norm();
    vec3 x_c = cross(this->up, z_c).norm();
    vec3 y_c = cross(z_c, x_c);

    this->V.fill_identity();

    for (int i = 0; i < 3; i++) {
        this->V(0, i) = x_c[i];
    }

    for (int i = 0; i < 3; i++) {
        this->V(1, i) = y_c[i];
    }

    for (int i = 0; i < 3; i++) {
        this->V(2, i) = z_c[i];
    }

    
    this->V(0, 3) = dot(-x_c,  this->eye);
    this->V(1, 3) = dot(-y_c,  this->eye);
    this->V(2, 3) = dot(-z_c,  this->eye);
    

    return this->V;
}

Matrix4 Camera::look_at(vec3 eye, vec3 target, vec3 up)
{
    this->eye = eye;
    this->target = target;
    this->up = up;


    return this->update_V();
}


Texture::Texture(const char *filename, int w_v, int h_v, float gamma)
{
    this->w = w_v;
    this->h = h_v;

    std::vector<float> texture_img = load_image_f32_rgb(filename, &w, &h, gamma); 
        if (texture_img.size() != 0) {
            

            textures.resize(1);
            textures[0].resize(w * h);
            std::copy_n(reinterpret_cast<const vec3*>(texture_img.data()), w*h, textures[0].data());

            this->generate_mips();
        } else {
            this->w = 0;
            this->h = 0;
            max_mip = 0;
        }
}

void Texture::generate_mips()
{
    if (textures.empty() || w <= 0 || h <= 0) {
        max_mip = 0;
        return;
    }

    mip_widths.clear();
    mip_heights.clear();
    textures.resize(1); 

    int cur_w = w;
    int cur_h = h;

    mip_widths.push_back(cur_w);
    mip_heights.push_back(cur_h);

    while (cur_w > 1 || cur_h > 1) {
        int next_w = std::max(1, (cur_w + 1) / 2);
        int next_h = std::max(1, (cur_h + 1) / 2);

        const std::vector<vec3>& src = textures.back();
        std::vector<vec3> dst(next_w * next_h);

        for (int y = 0; y < next_h; ++y) {
            for (int x = 0; x < next_w; ++x) {
                vec3 sum = vec3(0.0f);
                float cnt = 0.0f;

                for (int dy = 0; dy < 2; ++dy) {
                    int sy = std::min(y * 2 + dy, cur_h - 1);
                    for (int dx = 0; dx < 2; ++dx) {
                        int sx = std::min(x * 2 + dx, cur_w - 1);
                        sum += src[sy * cur_w + sx];
                        cnt += 1.0f;
                    }
                }

                dst[y * next_w + x] = sum * (1 / cnt);
            }
        }

        textures.push_back(std::move(dst));
        cur_w = next_w;
        cur_h = next_h;

        mip_widths.push_back(cur_w);
        mip_heights.push_back(cur_h);
    }

    max_mip = static_cast<int>(textures.size()) - 1;
}

vec3 Model::get_texture(float x, float y, int mip)
{
   
    if ((texture != nullptr) && texture->w > 0 && texture->h > 0) {
        if (mip < 0) mip = 0;
        if (mip > texture->max_mip) mip = texture->max_mip; 

        
        const std::vector<vec3> &src = texture->textures[mip];
        int W = texture->mip_widths[mip];
        int H = texture->mip_heights[mip];

        x -= std::floor(x);
        y -= std::floor(y);

        x = x * W - 0.5f;
        y = y * H - 0.5f;

        int x0 = (int)std::floor(x);
        int y0 = (int)std::floor(y);
        float fx = x - x0;
        float fy = y - y0;

        int x1 = x0 + 1, y1 = y0 + 1;
        x0 = std::clamp(x0, 0, W - 1); 
        x1 = std::clamp(x1, 0, W - 1);
        y0 = std::clamp(y0, 0, H - 1); 
        y1 = std::clamp(y1, 0, H - 1);


        float w00 = (1 - fx) * (1 - fy);
        float w10 = fx       * (1 - fy);
        float w01 = (1 - fx) * fy;
        float w11 = fx       * fy;

        vec3 c = w00 * src[y0 * W + x0]
            + w10 * src[y0 * W + x1]
            + w01 * src[y1 * W + x0]
            + w11 * src[y1 * W + x1];
        
        return c;
    } else {
        return base_color;
    }
}

Matrix4 Model::update_M()
{
    this->M = translate_matrix(this->translation_v) * rotation_matrix(this->rotation_v) * scale_matrix(this->scale_v);

    iMt =  Matrix3(M).inverse().transpose();

    
    return this->M;
}

Model &Model::scale(vec3 scale_v)
{
    this->scale_v = scale_v;
    update_M();

    return *this;
}

Model &Model::rotate(vec3 rotation_v)
{
    this->rotation_v = rotation_v;
    update_M();

    return *this;
}

Model &Model::translate(vec3 translation_v)
{
    this->translation_v = translation_v;
    update_M();

    return *this;
}
