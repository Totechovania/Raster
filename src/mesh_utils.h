#ifndef MESH_UTILS_H
#define MESH_UTILS_H
#include "vectors.h"
#include <vector>

// Треугольный меш. Все атрибуты индексируются одним индексом вершины:
// verts[i], normals[i], tcs[i]. Треугольник t - вершины indices[3*t+0..2].
//
// Нормали и текстурные координаты не генерируются: если в файле их нет (или они
// заданы не для всех вершин), normals / tcs равны NULL.
struct Mesh
{
    int num_vertices;
    int num_triangles;
    std::vector<vec3> verts;
    std::vector<vec3> normals;
    std::vector<vec2> tcs;
    std::vector<int>  indices;

    bool has_uv_co;
    
    // При ошибке возвращает пустой меш (num_vertices = num_triangles = 0)
    Mesh(const char *filename);
    Mesh &norm();
    Mesh &generate_normals();
};




#endif
