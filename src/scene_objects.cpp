#include "scene_objects.h"

Matrix4 Camera::ubdate_V() {
    vec3 z_c = (this->eye - this->target).norm();
    vec3 x_c = cross(this->up, z_c).norm();
    vec3 y_c = cross(z_c, x_c);

    this->V.fill_identity();

    for (int i = 0; i < 3; i++) {
        this->V(0, i) = x_c[i];
    }

    for (int i = 0; i < 3; i++) {
        this->V(0, i) = x_c[i];
    }

    for (int i = 0; i < 3; i++) {
        this->V(0, i) = x_c[i];
    }

    for (int i = 0; i < 3; i++) {
        this->V(i, 0) = dot(-x_c,  this->eye);
    }

    for (int i = 0; i < 3; i++) {
        this->V(i, 1) = dot(-y_c,  this->eye);
    }

    for (int i = 0; i < 3; i++) {
        this->V(i, 2) = dot(-z_c,  this->eye);
    }

    return this->V;
}

Matrix4 Model::update_M() {
    this->M = translate_matrix(this->translation) * rotation_matrix(this->rotation) * scale_matrix(this->scale);

    return this->M;
}
