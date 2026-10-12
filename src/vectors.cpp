#include "vectors.h"




float edge_func(const vec2 &v1, const vec2 &v2, const vec2 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
}
