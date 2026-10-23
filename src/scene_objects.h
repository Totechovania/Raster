#include "vectors.h"
#include "mesh_utils.h"



struct Camera {
    vec3 eye;
    vec3 target;
    vec3 up;
    Matrix4 V;

    Camera(vec3 eye = vec3(0, 0, 1),
         vec3 target = vec3(0, 0, 0), 
         vec3 up = vec3(0, 1, 0)): eye(eye), target(target), up(up) {
        this->update_V();
    };

    Matrix4 update_V();

    Matrix4 look_at(vec3 eye, vec3 target, vec3 up);
        
};


struct Model {
    Mesh &mesh;

    vec3 scale_v;
    vec3 rotation_v;
    vec3 translation_v;

    Matrix4 M;
    Matrix3 iMt;

    Model(Mesh &mesh, vec3 translation = vec3(), vec3 rotation = vec3(), vec3 scale = vec3(1, 1, 1)): 
        mesh(mesh),
        scale_v(scale),
        rotation_v(rotation),
        translation_v(translation) 
    {
        update_M();
    };

    Matrix4 update_M();

    Model &scale(vec3 scale_v);
    Model &rotate(vec3 rotation_v);
    Model &translate(vec3 translation_v);
};
