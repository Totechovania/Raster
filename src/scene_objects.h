#include "vectors.h"



struct Camera {
    vec3 eye;
    vec3 target;
    vec3 up;
    Matrix4 V;

    Camera(vec3 eye = vec3(0, 0, 1),
         vec3 target = vec3(0, 0, 0), 
         vec3 up = vec3(0, 1, 0)): eye(eye), target(target), up(up) {
        this->ubdate_V();
    };

    Matrix4 ubdate_V();

    Matrix4 look_at(vec3 eye, vec3 target, vec3 up);
        
};


struct Model {

};
