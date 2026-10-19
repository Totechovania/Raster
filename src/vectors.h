#ifndef VECTORS_H
#define VECTORS_H
#include <vector>
#include <iostream>



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


float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p);



struct Matrix {
    std::vector<float> data;
    int size;
    int b = 0;

    explicit Matrix(int size);
    float &operator()(int i, int j);
    const float &operator()(int i, int j) const;
    Matrix &fill_zeros();
    Matrix &fill_identity();

    Matrix(const Matrix&)=default;
    Matrix& operator=(const Matrix&)=default;
    Matrix(Matrix&&)=default;
    Matrix& operator=(Matrix&&)=default;
    ~Matrix()=default;



    Matrix inverse() const;

    Matrix &operator+=(const Matrix &b);
    Matrix &operator+=(float b);
    Matrix &operator*=(float b);
};

Matrix operator+(const Matrix &a, const Matrix &b);
Matrix operator*(const Matrix &a, const Matrix &b);

Matrix operator+(const Matrix &a, float b); 
Matrix operator+(float a, const Matrix &b); 


Matrix operator*(const Matrix &a, float b); 
Matrix operator*(float a, const Matrix &b);

std::ostream& operator<<(std::ostream &os, const Matrix &a);


#endif
