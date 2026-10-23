#include "scene_objects.h"

Matrix4 Camera::update_V() {
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

float Model::texture_at(int x, int y, int ch) {
   


    if (x < 0) x = 0;
    if (x >= t_shape.w) x = t_shape.w - 1;

    if (y < 0) y = 0;
    if (y >= t_shape.h) y = t_shape.h - 1;

    if (has_texture) {
        return texture[(y*t_shape.w + x)*3 + ch];
    }

    return base_color[ch];
}

float Model::get_texture(float x, float y, int ch)
{
   
    if (has_texture && t_shape.w != 0 && t_shape.h != 0) {
        x = x - std::floor(x);
        y = y - std::floor(y);
        x = 1 - x;

        x = t_shape.w * x - 0.5f;
        y = t_shape.h * y - 0.5f;

        float a = x - std::floor(x);
        float b = y - std::floor(y);

        float c = (1 - a) * (1 - b) * texture_at(x - a, y - b, ch) + 
                (a) * (1 - b) * texture_at(x - a + 1, y - b, ch) + 
                (1 - a) * (b) * texture_at(x - a, y - b + 1, ch) + 
                (a) * (b) * texture_at(x - a + 1, y - b + 1, ch);

        if (c < 0) c = 0;
        if (c > 1) c = 1;
        
        return c;
    } else {
        return base_color[ch];
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
