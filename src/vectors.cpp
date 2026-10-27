#include "vectors.h"







vec3 cross(const vec3 & a, const vec3 &b) {
    return vec3(a.y * b.z - a.z * b.y, - a.x * b.z + a.z * b.x, a.x * b.y - a.y * b.x);
}

vec4 operator*(const Matrix4 &m, const vec4 &v) {
    vec4 res;

    for (int i = 0; i < 4; i++) {
        res[i] = 0;
        for (int j = 0; j < m.size; j++) {
            res[i] += m(i, j) * v[j];
        }
    }

    return res;
}
vec3 operator*(const Matrix3 &m, const vec3 &v)
{
    vec3 res;

    for (int i = 0; i < 3; i++) {
        res[i] = 0;
        for (int j = 0; j < 3; j++) {
            res[i] += m(i, j) * v[j];
        }
    }

    return res;;
};

Matrix4::Matrix4(const Matrix3 m3) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            (*this)(i, j) = m3(i, j);
        }
    }

    for (int j = 0; j < 3; j++) {
            (*this)(3, j) = 0;
    }

    for (int i = 0; i < 3; i++) {
            (*this)(i, 3) = 0;
    }

    (*this)(3, 3) = 1;
}

float &Matrix4::operator()(int i, int j)
{
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
        for (int j = i + 1; j < this->size; j++) {
                swap((*this)(i, j), (*this)(j, i));
        }
    }

    return *this;
}

Matrix4 transpose(const Matrix4 &a){
    Matrix4 res = a;
    for (int i = 0; i < a.size; i++) {       
        for (int j = i + 1; j < a.size; j++) {
                swap(res(i, j), res(j, i));
        }
    }

    return res;
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

Matrix3::Matrix3(const Matrix4 &m) {

    for (int i = 0; i < 3; i++) {    
        for (int j = 0; j < 3; j++) {
            (*this)(i, j) = m(i, j);       
        }
    }
}

float &Matrix3::operator()(int i, int j)
{
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
        for (int j = i + 1; j < this->size; j++) {
                swap((*this)(i, j), (*this)(j, i));
        }
    }

    return *this;
}

Matrix3 transpose(const Matrix3 &a){
    Matrix3 res = a;
    for (int i = 0; i < a.size; i++) {       
        for (int j = i + 1; j < a.size; j++) {
                swap(res(i, j), res(j, i));
        }
    }


    return res;
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

Matrix3 scale_matrix(const vec3 &s_v) {
    Matrix3 res;
    res.fill_zeros();
    
    for (int i = 0; i < 3; i++) {
        res(i, i) = s_v[i];
    }

    return res;
}



Matrix3 rotation_x_matrix(float x_angle) {
    Matrix3 res;
    res.fill_identity();
    
    res(1, 1) = std::cos(x_angle);
    res(2, 2) = std::cos(x_angle);
    res(1, 2) = -std::sin(x_angle);
    res(2, 1) = std::sin(x_angle);


    return res;
}

Matrix3 rotation_y_matrix(float y_angle){
    Matrix3 res;
    res.fill_identity();
    
    res(0, 0) = std::cos(y_angle);
    res(2, 2) = std::cos(y_angle);
    res(2, 0) = -std::sin(y_angle);
    res(0, 2) = std::sin(y_angle);


    return res;
}

Matrix3 rotation_z_matrix(float z_angle){
    Matrix3 res;
    res.fill_identity();
    
    res(0, 0) = std::cos(z_angle);
    res(1, 1) = std::cos(z_angle);
    res(0, 1) = -std::sin(z_angle);
    res(1, 0) = std::sin(z_angle);


    return res;
}

Matrix3 rotation_matrix(const vec3 &s_v) {
    return rotation_z_matrix(s_v.z) * rotation_y_matrix(s_v.y) * rotation_x_matrix(s_v.x);
}

Matrix4 translate_matrix(const vec3 &s_v) {
    Matrix4 res;
    res.fill_identity();
    
    for (int i = 0; i < 3; i++) {
        res(i, 3) = s_v[i];
    }

    return res;
}

