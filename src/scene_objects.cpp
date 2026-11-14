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

vec3 Model::get_texture(float x, float y)
{
   
    if (has_texture && t_shape.w != 0 && t_shape.h != 0) {

        x -= std::floor(x);
        y -= std::floor(y);

        x = x * t_shape.w - 0.5f;
        y = y * t_shape.h - 0.5f;

        int x0 = (int)std::floor(x);
        int y0 = (int)std::floor(y);
        float fx = x - x0;
        float fy = y - y0;

        int x1 = x0 + 1, y1 = y0 + 1;
        x0 = std::clamp(x0, 0, t_shape.w - 1); 
        x1 = std::clamp(x1, 0, t_shape.w - 1);
        y0 = std::clamp(y0, 0, t_shape.h - 1); 
        y1 = std::clamp(y1, 0, t_shape.h - 1);


        float w00 = (1 - fx) * (1 - fy);
        float w10 = fx       * (1 - fy);
        float w01 = (1 - fx) * fy;
        float w11 = fx       * fy;

        vec3 c = w00 * texture[y0 * t_shape.w + x0]
            + w10 * texture[y0 * t_shape.w + x1]
            + w01 * texture[y1 * t_shape.w + x0]
            + w11 * texture[y1 * t_shape.w + x1];
        
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
