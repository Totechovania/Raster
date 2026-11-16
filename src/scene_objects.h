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

    Texture(const char *filename, int w_v, int h_v, bool mipmaping=true, float gamma=2.2);

    void generate_mips();

    [[gnu::always_inline]] 
    inline vec3 get_texture(float x, float y, int mip)
    {
        if (mip < 0) mip = 0;
        if (mip > max_mip) mip = max_mip; 

        
        const std::vector<vec3> &src = textures[mip];
        int W = mip_widths[mip];
        int H = mip_heights[mip];


        if (W == 0 || H ==0) return vec3();

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
    }
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


    [[gnu::always_inline]] 
    inline vec3 get_texture(float x, float y, int mip)
    {
        if ((texture != nullptr) && texture->w > 0 && texture->h > 0) {
            return texture->get_texture(x, y, mip);
        } else {
            return base_color;
        }
    }

    Matrix4 update_M();

    Model &scale(vec3 scale_v);
    Model &rotate(vec3 rotation_v);
    Model &translate(vec3 translation_v);
};



struct Skybox {

    Texture *bk; 
    Texture *dn; 
    Texture *ft; 
    Texture *lf; 
    Texture *rt; 
    Texture *up;
    
    Skybox( Texture *bk, Texture *dn, Texture *ft, Texture *lf, Texture *rt, Texture *up): bk(bk), dn(dn), ft(ft), lf(lf), rt(rt), up(up) {

    }

};