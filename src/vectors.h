#ifndef VECTORS_H
#define VECTORS_H
#include <vector>
#include <iostream>
#include <stdexcept>
#include <math.h>


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

        return std::sqrt(dot(self(), self()));
    }

    Derived& norm() {
        float l = self().len();
        if (std::abs(l) < 1e-10) {
            throw std::range_error("trying to normolise zero len vector");
        } 

        self() *= 1/l;

        return self();
    }
    

    Derived& operator+=(const Derived& other) {
        static_assert(VectorLike<Derived>);

        for (int i=0; i < N; i++) {
            self()[i] += other[i];
        }

        return self();
    }

    Derived& operator*=(float a) {
        static_assert(VectorLike<Derived>);

        for (int i=0; i < N; i++) {
            self()[i] *= a;
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

struct vec4: vec_base<vec4, 4>
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

    vec4(const vec3 &v) {
        for (int i = 0; i < 3; i++) {
            M[i] = v[i];
        }

        M[3] = 1;
    }
};




template< VectorLike V> 
inline V operator+(const V &a, const V &b) {
    V res;

    for (int i = 0; i < V::dim; i++) {
        res[i] = a[i] + b[i];
    }

    return res;
};

template< VectorLike V>
std::ostream &operator<<(std::ostream &os, const V &a) {
    os << "[ ";
    
    for (int i = 0; i < V::dim; i++) {
        os << a[i] << " ";
    }

    os << " ]\n";
    
    return os;
}


template< VectorLike V> 
inline V operator-(const V &a, const V &b) {
    V res;

    for (int i = 0; i < V::dim; i++) {
        res[i] = a[i] - b[i];
    }

    return res;
};

template< VectorLike V> 
inline V operator-(const V &b) {
    V res;

    for (int i = 0; i < V::dim; i++) {
        res[i] = -b[i];
    }

    return res;
};

template< VectorLike V> 
inline V operator+(const V &a, float b){
    V res;

    for (int i = 0; i < V::dim; i++) {
        res[i] = a[i] + b;
    }

    return res;
};

template< VectorLike V> 
inline V operator+(float a, const V &b){
    return b + a;
};

template< VectorLike V> 
inline V operator*(const V &a, float b){
    V res;


    for (int i = 0; i < V::dim; i++) {
        res[i] = a[i] * b;
    }

    return res;
};

template< VectorLike V> 
inline V operator*(float a, const V &b){
    return b * a;
};




template< VectorLike V> 
inline float dot(const V & a, const V &b) {
    float res = 0;

    for (int i = 0; i < V::dim; i++) {
        res += a[i] * b[i];
    }

    return res;
}

vec3 cross(const vec3 & a, const vec3 &b);






float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p);





struct Matrix3 {
    static constexpr int size = 3;
    float data[size * size];

    explicit Matrix3() {};
    float &operator()(int i, int j);
    const float &operator()(int i, int j) const;
    Matrix3 &fill_zeros();
    Matrix3 &fill_identity();

    Matrix3(const Matrix3&)=default;
    Matrix3& operator=(const Matrix3&)=default;
    Matrix3(Matrix3&&)=default;
    Matrix3& operator=(Matrix3&&)=default;
    ~Matrix3()=default;

    Matrix3 &operator+=(const Matrix3 &b);
    Matrix3 &operator+=(float b);
    Matrix3 &operator*=(float b);
    Matrix3 &transpose();
    Matrix3 &inverse();
};

Matrix3 operator+(const Matrix3 &a, const Matrix3 &b);
Matrix3 operator*(const Matrix3 &a, const Matrix3 &b);

Matrix3 operator+(const Matrix3 &a, float b); 
Matrix3 operator+(float a, const Matrix3 &b); 


Matrix3 operator*(const Matrix3 &a, float b); 
Matrix3 operator*(float a, const Matrix3 &b);

Matrix3 transpose(const Matrix3 &a);

Matrix3 inverse(const Matrix3 &a);

std::ostream &operator<<(std::ostream &os, const Matrix3 &a);



Matrix3 scale_matrix(const vec3 &s_v);

Matrix3 rotation_matrix(const vec3 &s_v);


struct Matrix4 {
    static constexpr int size = 4;
    float data[size * size];

    Matrix4() {};

    Matrix4(const Matrix3 m3);

    float &operator()(int i, int j);
    const float &operator()(int i, int j) const;
    Matrix4 &fill_zeros();
    Matrix4 &fill_identity();

    Matrix4 inverse();

    Matrix4(const Matrix4&)=default;
    Matrix4& operator=(const Matrix4&)=default;
    Matrix4(Matrix4&&)=default;
    Matrix4& operator=(Matrix4&&)=default;
    ~Matrix4()=default;

    Matrix4 &operator+=(const Matrix4 &b);
    Matrix4 &operator+=(float b);
    Matrix4 &operator*=(float b);
    Matrix4 &transpose();
};

Matrix4 inverse(const Matrix4 &a);

Matrix4 operator+(const Matrix4 &a, const Matrix4 &b);
Matrix4 operator*(const Matrix4 &a, const Matrix4 &b);

Matrix4 operator+(const Matrix4 &a, float b); 
Matrix4 operator+(float a, const Matrix4 &b); 


Matrix4 operator*(const Matrix4 &a, float b); 
Matrix4 operator*(float a, const Matrix4 &b);

Matrix4 transpose(const Matrix4 &a);

std::ostream &operator<<(std::ostream &os, const Matrix4 &a);




Matrix3 rotation_x_matrix(float x_angle);
Matrix3 rotation_y_matrix(float y_angle);
Matrix3 rotation_z_matrix(float z_angle);

Matrix4 translate_matrix(const vec3 &s_v);



vec4 operator*(const Matrix4 &m, const vec4 &v);

#endif
