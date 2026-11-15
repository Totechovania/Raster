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

struct Texture {
    
    int h;
    int w;
    int max_mip;
    std::vector<int> mip_widths;
    std::vector<int> mip_heights;
    std::vector<std::vector<vec3>> textures;

    Texture(const char *filename, int w_v, int h_v, float gamma=2.2);

    void generate_mips();
};


struct Model {
    Mesh &mesh;

    vec3 scale_v;
    vec3 rotation_v;
    vec3 translation_v;

    Matrix4 M;
    Matrix3 iMt;

    int id;

    Texture *texture;
    
    vec3 base_color;

    Model(Mesh &mesh, Texture *texture=nullptr, int w=0, int h=0): 
        texture(texture), mesh(mesh)   
    {
        scale_v = vec3(1, 1, 1);
        rotation_v = vec3();
        translation_v = vec3();

        base_color = vec3(1);

        update_M();
    };

    vec3 get_texture(float x, float y, int mip);

    Matrix4 update_M();

    Model &scale(vec3 scale_v);
    Model &rotate(vec3 rotation_v);
    Model &translate(vec3 translation_v);
};



