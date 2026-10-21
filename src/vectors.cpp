#include "vectors.h"
#include <stdexcept>




float edge_func(const vec3 &v1, const vec3 &v2, const vec3 &p)
{
    return (v2.x - v1.x) * (p.y - v1.y) - (v2.y - v1.y) * (p.x - v1.x);
};


vec3 cross(const vec3 & a, const vec3 &b) {
    return vec3(a.y * b.z - a.z * b.y, - a.x * b.z + a.z * b.x, a.x * b.y - a.y * b.x);
};



float &Matrix4::operator()(int i, int j){
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix4 index out of range");
    }

    return this->data[i * size + j];
}

const float &Matrix4::operator()(int i, int j) const {
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix4 index out of range");
    }

    return this->data[i * size + j];
}

Matrix4 &Matrix4::fill_zeros() {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) = 0;
        }
    }

    return *this;
}

Matrix4 &Matrix4::fill_identity() {
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
swap_raws(Matrix4 &m, int i, int j) {
    for (int k = 0; k < m.size; k++ ) {
        swap(m(i, k), m(j, k));
    }
}

void 
mlt_raw(Matrix4 &m, int i, float v) {
    for (int k = 0; k < m.size; k++ ) {
        m(i, k) *= v;
    }
}

void
diff_raws(Matrix4 &m, int j, int i, float i_coef) {
    for (int k = 0; k < m.size; k++ ) {
        m(j, k) -= i_coef * m(i, k);
    }

}


Matrix4 Matrix4::inverse() {

    Matrix4 res = Matrix4();
    res.fill_identity();

    for(int col = 0; col < (*this).size; col++) {
        int row = col;
        float max_v = std::abs((*this)(col, col));
        for (int i = col + 1; i < this->size; ++i) {
            float v = std::abs((*this)(i, col));
            if (v > max_v) {
                max_v = v;
                row = i;
            }
        }


        if (max_v < 1e-10) {
            throw std::logic_error("Matrix4 dont have an inverse");
        }

        if (row != col) {
            swap_raws(*this, row, col);
            swap_raws(res, row, col);
        }
        
        mlt_raw(res, col,  1.f / (*this)(col, col));
        mlt_raw((*this), col,  1.f / (*this)(col, col));

        for (int i = 0; i < (*this).size; i++) {
            if (i == col) continue;
            diff_raws(res, i, col, (*this)(i, col));
            diff_raws((*this), i, col, (*this)(i, col));
        }

    }

    (*this) = res;

    return (*this);
};


Matrix4 inverse(const Matrix4& a) {

    Matrix4 tmp = a;

    Matrix4 res = Matrix4();
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
            throw std::logic_error("Matrix4 dont have an inverse");
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




Matrix4 operator+(const Matrix4 &a, const Matrix4 &b) {

    Matrix4 res = Matrix4();

    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < a.size; j++) {
            res(i, j) = a(i, j) + b(i, j);
        }
    }

    return res;
}

Matrix4 operator*(const Matrix4 &a, const Matrix4 &b) {

    Matrix4 res = Matrix4();

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

Matrix4 operator+(const Matrix4 &a, float b) {
        Matrix4 res = Matrix4();

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) + b;
            }
        }

        return res;
}
Matrix4 operator+(float a, const Matrix4 &b) {
    return b + a; 
}


Matrix4 operator*(const Matrix4 &a, float b) {
            Matrix4 res = Matrix4();

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) * b;
            }
        }

        return res;
}
Matrix4 operator*(float a, const Matrix4 &b) {
    return b * a;
}


Matrix4 &Matrix4::operator+=(const Matrix4 &b) {

    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b(i, j);
        }
    }

    return *this;
}

Matrix4 &Matrix4::operator+=(float b) {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b;
        }
    }

    return *this;
}


Matrix4 &Matrix4::operator*=(float b)
{
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) *= b;
        }
    }

    return *this;
}

Matrix4 &Matrix4::transpose() {
    for (int i = 0; i < this->size; i++) {       
        for (int j = 0; j < this->size; j++) {
                swap((*this)(i, j), (*this)(j, i));
        }
    }

}

Matrix4 &transpose(const Matrix4 &a){
    Matrix4 res = a;
    for (int i = 0; i < a.size; i++) {       
        for (int j = 0; j < a.size; j++) {
                swap(res(i, j), res(j, i));
        }
    }

}


std::ostream& operator<<(std::ostream &os, const Matrix4 &a) {                                                     
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







float &Matrix3::operator()(int i, int j){
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix3 index out of range");
    }

    return this->data[i * size + j];
}

const float &Matrix3::operator()(int i, int j) const {
    if (i < 0 || i >= this->size || j < 0 || j >= this->size) {
        throw std::out_of_range("Matrix3 index out of range");
    }

    return this->data[i * size + j];
}

Matrix3 &Matrix3::fill_zeros() {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) = 0;
        }
    }

    return *this;
}

Matrix3 &Matrix3::fill_identity() {
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



void
swap_raws(Matrix3 &m, int i, int j) {
    for (int k = 0; k < m.size; k++ ) {
        swap(m(i, k), m(j, k));
    }
}

void 
mlt_raw(Matrix3 &m, int i, float v) {
    for (int k = 0; k < m.size; k++ ) {
        m(i, k) *= v;
    }
}

void
diff_raws(Matrix3 &m, int j, int i, float i_coef) {
    for (int k = 0; k < m.size; k++ ) {
        m(j, k) -= i_coef * m(i, k);
    }

}

Matrix3 operator+(const Matrix3 &a, const Matrix3 &b) {

    Matrix3 res = Matrix3();

    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < a.size; j++) {
            res(i, j) = a(i, j) + b(i, j);
        }
    }

    return res;
}

Matrix3 operator*(const Matrix3 &a, const Matrix3 &b) {

    Matrix3 res = Matrix3();

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

Matrix3 operator+(const Matrix3 &a, float b) {
        Matrix3 res = Matrix3();

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) + b;
            }
        }

        return res;
}
Matrix3 operator+(float a, const Matrix3 &b) {
    return b + a; 
}


Matrix3 operator*(const Matrix3 &a, float b) {
            Matrix3 res = Matrix3();

        for (int i = 0; i < a.size; i++) {
            for (int j = 0; j < a.size; j++) {
                res(i, j) = a(i, j) * b;
            }
        }

        return res;
}
Matrix3 operator*(float a, const Matrix3 &b) {
    return b * a;
}


Matrix3 &Matrix3::operator+=(const Matrix3 &b) {

    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b(i, j);
        }
    }

    return *this;
}

Matrix3 &Matrix3::operator+=(float b) {
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) += b;
        }
    }

    return *this;
}


Matrix3 &Matrix3::operator*=(float b)
{
    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->size; j++) {
            (*this)(i, j) *= b;
        }
    }

    return *this;
}

Matrix3 &Matrix3::transpose() {
    for (int i = 0; i < this->size; i++) {       
        for (int j = 0; j < this->size; j++) {
                swap((*this)(i, j), (*this)(j, i));
        }
    }

}

Matrix3 &transpose(const Matrix3 &a){
    Matrix3 res = a;
    for (int i = 0; i < a.size; i++) {       
        for (int j = 0; j < a.size; j++) {
                swap(res(i, j), res(j, i));
        }
    }
}



Matrix3 &Matrix3::inverse() {

    Matrix3 res = Matrix3();
    res.fill_identity();

    for(int col = 0; col < (*this).size; col++) {
        int row = col;
        float max_v = std::abs((*this)(col, col));
        for (int i = col + 1; i < this->size; ++i) {
            float v = std::abs((*this)(i, col));
            if (v > max_v) {
                max_v = v;
                row = i;
            }
        }


        if (max_v < 1e-10) {
            throw std::logic_error("Matrix4 dont have an inverse");
        }

        if (row != col) {
            swap_raws(*this, row, col);
            swap_raws(res, row, col);
        }
        
        mlt_raw(res, col,  1.f / (*this)(col, col));
        mlt_raw((*this), col,  1.f / (*this)(col, col));

        for (int i = 0; i < (*this).size; i++) {
            if (i == col) continue;
            diff_raws(res, i, col, (*this)(i, col));
            diff_raws((*this), i, col, (*this)(i, col));
        }

    }

    (*this) = res;

    return (*this);
};


Matrix3 inverse(const Matrix3& a) {

    Matrix3 tmp = a;

    Matrix3 res = Matrix3();
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
            throw std::logic_error("Matrix4 dont have an inverse");
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





std::ostream& operator<<(std::ostream &os, const Matrix3 &a) {                                                     
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




    

    
