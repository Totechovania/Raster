#ifndef VECTORS_H
#define VECTORS_H

// Минимальный набор векторных типов, нужный утилитам шаблона.
// Остальную математику (операции над векторами, матрицы) вы пишете сами.


struct vec2
{
    union
    {
        struct
        {
            float x;
            float y;
        };
        float M[2];
    };

    vec2(float x_v=0, float y_v=0): x(x_v), y(y_v) {};
    
};


struct vec3
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        float M[3];
    };

    vec3(float x_v=0, float y_v=0, float z_v=0): x(x_v), y(y_v), z(z_v) {};
    
};

static inline vec2 make2(float x, float y)
{
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline vec3 make3(float x, float y, float z)
{
    vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
};


float edge_func(const vec2 &v1, const vec2 &v2, const vec2 &p);


#endif
