#ifndef VECTORS_H
#define VECTORS_H
#include <vector>
#include <iostream>



// Минимальный набор векторных типов, нужный утилитам шаблона.
// Остальную математику (операции над векторами, матрицы) вы пишете сами.

template <typename V>
concept VectorLike = requires(V v, const V cv, int i) {
    { V::dim } -> std::convertible_to<int>;
    { cv[i] }  -> std::convertible_to<float>;
    { v[i]  }  -> std::convertible_to<float&>;
};

template <typename Derived, int N>
struct vec_base {
    static constexpr int dim = N;

    Derived& self() { 
        return static_cast<Derived&>(*this); 
    }

    const Derived& self() const { 
        return static_cast<const Derived&>(*this); 
    }


    float& operator[](int i) { 
        return self().M[i]; 
    }
    const float& operator[](int i) const { 
        return self().M[i]; 
    }

    float len() const {
        static_assert(VectorLike<Derived>);

        return dot(self(), self());
    }

    Derived& operator+=(const Derived& other) {
        static_assert(VectorLike<Derived>);

        for (int i=0; i < N; i++) {
            self()[i] += other[i];
        }

        return self();
    }
};

struct vec2: vec_base<vec2, 2>
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

struct vec3: vec_base<vec3, 3>
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

struct vec4: vec_base<vec3, 3>
{

    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        float M[4];
    };

    vec4(float x_v=0, float y_v=0, float z_v=0, float w_v=0): x(x_v), y(y_v), z(z_v), w(w_v) {};
};




template< VectorLike V> 
V operator+(const V &a, const V &b);

template< VectorLike V> 
V operator+(const V &a, float b);
template< VectorLike V> 
V operator+(float a, const V &b);

template< VectorLike V> 
V operator*(const V &a, float b);
template< VectorLike V> 
V operator*(float a, const V &b);



template< VectorLike V> 
float dot(const V & a, const V &b);
vec3 cross(const vec3 & a, const vec3 &b);








float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p);



struct Matrix {
    std::vector<float> data;
    int size;

    explicit Matrix(int size=4);
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
