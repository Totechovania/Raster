#include "vectors.h"
#include <stdexcept>




float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
};


 


Matrix::Matrix(int size) {
    this->size = size;
    this->data.resize(size * size);

};


float &Matrix::operator()(int i, int j){
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix index out of range");
    }

    return this->data[i * size + j];
}

const float &Matrix::operator()(int i, int j) const {
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix index out of range");
    }

    return this->data[i * size + j];
}

Matrix &Matrix::fill_zeros() {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) = 0;
        }
    }

    return *this;
}

Matrix &Matrix::fill_identity() {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            if (i != j) {
                (*this)(i, j) = 0;
            } else {
                (*this)(i, j) = 1;
            }
            
        }
    }

    return *this;
}



void swap(float &a, float &b) {
    float t = a;
    a = b;
    b = t;
}

void
swap_raws(Matrix &m, int i, int j) {
    for (int k = 0; k < m.size; k++ ) {
        swap(m(i, k), m(j, k));
    }
}

void 
mlt_raw(Matrix &m, int i, float v) {
    for (int k = 0; k < m.size; k++ ) {
        m(i, k) *= v;
    }
}

void
diff_raws(Matrix &m, int j, int i, float i_coef) {
    for (int k = 0; k < m.size; k++ ) {
        m(j, k) -= i_coef * m(i, k);
    }

}


Matrix Matrix::inverse() const{

    Matrix tmp = *this;

    Matrix res = Matrix(tmp.size);
    res.fill_identity();

    for(int col = 0; col < tmp.size; col++) {
        int row = col;
        float max_v = std::abs(tmp(col, col));
        for (int i = col + 1; i < tmp.size; ++i) {
            float v = std::abs(tmp(i, col));
            if (v > max_v) {
                max_v = v;
                row = i;
            }
        }


        if (max_v < 1e-10) {
            throw std::logic_error("Matrix dont have an inverse");
        }

        if (row != col) {
            swap_raws(tmp, row, col);
            swap_raws(res, row, col);
        }
        
        mlt_raw(res, col,  1.f / tmp(col, col));
        mlt_raw(tmp, col,  1.f / tmp(col, col));

        for (int i = 0; i < tmp.size; i++) {
            if (i == col) continue;
            diff_raws(res, i, col, tmp(i, col));
            diff_raws(tmp, i, col, tmp(i, col));
        }

    }

    return res;
};


Matrix operator+(const Matrix &a, const Matrix &b) {
    if (a.size != b.size) {
        throw std::logic_error("Matrices should be same size");
    }

    Matrix res = Matrix(a.size);

    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < a.size; j++) {
            res(i, j) = a(i, j) + b(i, j);
        }
    }

    return res;
}

Matrix operator*(const Matrix &a, const Matrix &b) {
     if (a.size != b.size) {
        throw std::logic_error("Matrices should be same size");
    }

    Matrix res = Matrix(a.size);

    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < a.size; j++) {
            float s = 0;

            for (int k = 0; k < a.size; k++) {
                s += a(i, k) * b(k, j);
            }

            res(i, j) = s;
        }
    }

    return res;

}

Matrix operator+(const Matrix &a, float b) {
        Matrix res = Matrix(a.size);

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) + b;
            }
        }

        return res;
}
Matrix operator+(float a, const Matrix &b) {
    return b + a; 
}


Matrix operator*(const Matrix &a, float b) {
            Matrix res = Matrix(a.size);

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) * b;
            }
        }

        return res;
}
Matrix operator*(float a, const Matrix &b) {
    return b * a;
}


Matrix &Matrix::operator+=(const Matrix &b) {
    if (this->size != b.size) {
        throw std::logic_error("Matrices should be same size");
    }

    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b(i, j);
        }
    }

    return *this;
}

Matrix &Matrix::operator+=(float b) {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b;
        }
    }

    return *this;
}


Matrix &Matrix::operator*=(float b)
{
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) *= b;
        }
    }

    return *this;
}

std::ostream& operator<<(std::ostream &os, const Matrix &a) {                                                     
    os << "[\n";
    for (int i = 0; i < a.size; i++) {    
        for (int j = 0; j < a.size; j++) {
            os << "\t" << a(i, j);        
        }
    
        os << "\n";
    }
        
    os << "]\n";

    return os;
}



