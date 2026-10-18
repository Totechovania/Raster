#include "vectors.h"




float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
};


 

Matrix::Matrix(int size) {
    this->size = size;
    this->data.resize(size * size);

};


Matrix &Matrix::inverse() {

    
};


Matrix operator+(const Matrix &a, const Matrix &b);
Matrix operator*(const Matrix &a, const Matrix &b);

Matrix operator+(const Matrix &a, float b); 
Matrix operator+(float a, const Matrix &b); 


Matrix operator*(const Matrix &a, float b); 
Matrix operator*(float a, const Matrix &b); 

std::ostream& operator<<(std::ostream &os, const Matrix &a);
