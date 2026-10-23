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

Matrix4 Model::update_M() {
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
