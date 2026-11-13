#include "vectors.h"
#include "mesh_utils.h"
#include "image_utils.h"
#include <stdint.h>
#include <algorithm>
#include <cstring>


struct Camera {
    vec3 eye;
    vec3 target;
    vec3 up;
    Matrix4 V;

    float pitch;
    float yaw;

    Camera(vec3 eye = vec3(0, 0, 1),
         vec3 target = vec3(0, 0, 0), 
         vec3 up = vec3(0, 1, 0)): eye(eye), target(target), up(up) {

        
        this->sync_angles();
        this->update_V();
    };

    void
    sync_angles();

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

    int id;

    
    bool has_texture;
    std::vector<vec3> texture;
    struct 
    {
        int h;
        int w;
    } t_shape;
    
    vec3 base_color;

    Model(Mesh &mesh, const char *texture_file="", int w=0, int h=0, float gamma=2.2): 
        mesh(mesh)   
    {
        scale_v = vec3(1, 1, 1);
        rotation_v = vec3();
        translation_v = vec3();

        std::vector<float> texture_img = load_image_f32_rgb(texture_file, &w, &h, gamma); 
        if (texture_img.size() == 0) {
            has_texture = false;
            t_shape.h = 0;
            t_shape.w = 0;
        } else {
            has_texture = true;
            
            t_shape.h = h;
            t_shape.w = w;

            texture.resize(w * h);
            std::memcpy(texture.data(), texture_img.data(), w * h * 3 * sizeof(float));
        }

        base_color = vec3(1);

        update_M();
    };

    vec3 texture_at(int x, int y);

    vec3 get_texture(float x, float y);

    Matrix4 update_M();

    Model &scale(vec3 scale_v);
    Model &rotate(vec3 rotation_v);
    Model &translate(vec3 translation_v);
};
