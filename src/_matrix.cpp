struct vdbMat4
{
    float data[4*4];
    #if defined(VDB_MATRIX_COLUMN_MAJOR)
    float &operator()(int row, int col) { return data[col + row*4]; }
    #else
    float &operator()(int row, int col) { return data[row + col*4]; }
    #endif
};

inline void UniformMat4fv(GLint u, int n, float *v)
{
    #if defined(VDB_MATRIX_ROW_MAJOR)
    glUniformMatrix4fv(u, n, GL_FALSE, v);
    #elif defined(VDB_MATRIX_COLUMN_MAJOR)
    glUniformMatrix4fv(u, n, GL_TRUE, v);
    #else
    #error "You must #define VDB_MATRIX_ROW_MAJOR or VDB_MATRIX_COLUMN_MAJOR"
    #endif
}
inline void UniformMat4(GLint u, int n, vdbMat4 &m) { UniformMat4fv(u, n, m.data); }
inline void UniformVec4(GLuint u, vdbVec4 &v) { glUniform4f(u, v.x, v.y, v.z, v.w); }
inline void UniformVec3(GLuint u, vdbVec3 &v) { glUniform3f(u, v.x, v.y, v.z); }
inline void UniformVec2(GLuint u, vdbVec2 &v) { glUniform2f(u, v.x, v.y); }

static vdbVec4 operator-(vdbVec4 a)              { return vdbVec4(-a.x, -a.y, -a.z, -a.w); }
static vdbVec4 operator+(vdbVec4 a, vdbVec4 b)   { return vdbVec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
static vdbVec4 operator-(vdbVec4 a, vdbVec4 b)   { return vdbVec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
static vdbVec4 operator*(vdbVec4 a, vdbVec4 b)   { return vdbVec4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
static vdbVec4 operator*(vdbVec4 a, float b)     { return vdbVec4(a.x*b, a.y*b, a.z*b, a.w*b); }
static vdbVec4 operator*(float b, vdbVec4 a)     { return vdbVec4(a.x*b, a.y*b, a.z*b, a.w*b); }
static vdbVec4 operator/(vdbVec4 a, float b)     { return vdbVec4(a.x/b, a.y/b, a.z/b, a.w/b); }
static vdbVec4 operator/(float b, vdbVec4 a)     { return vdbVec4(a.x/b, a.y/b, a.z/b, a.w/b); }
static float vdbVecLength(vdbVec4 v)             { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }
static float vdbVecDot(vdbVec4 a, vdbVec4 b)     { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
static vdbVec4 vdbVecNormalize(vdbVec4 v)
{
    float length = vdbVecLength(v);
    if (length == 0.0f)
        return vdbVec4(0.0f,0.0f,0.0f,0.0f);
    else
        return v*(1.0f/length);
}

static vdbVec3 operator-(vdbVec3 a)              { return vdbVec3(-a.x, -a.y, -a.z); }
static vdbVec3 operator+(vdbVec3 a, vdbVec3 b)   { return vdbVec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static vdbVec3 operator-(vdbVec3 a, vdbVec3 b)   { return vdbVec3(a.x - b.x, a.y - b.y, a.z - b.z); }
static vdbVec3 operator*(vdbVec3 a, vdbVec3 b)   { return vdbVec3(a.x*b.x, a.y*b.y, a.z*b.z); }
static vdbVec3 operator*(vdbVec3 a, float b)     { return vdbVec3(a.x*b, a.y*b, a.z*b); }
static vdbVec3 operator*(float b, vdbVec3 a)     { return vdbVec3(a.x*b, a.y*b, a.z*b); }
static vdbVec3 operator/(vdbVec3 a, float b)     { return vdbVec3(a.x/b, a.y/b, a.z/b); }
static vdbVec3 operator/(float b, vdbVec3 a)     { return vdbVec3(a.x/b, a.y/b, a.z/b); }
static float vdbVecLength(vdbVec3 v)             { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }
static float vdbVecDot(vdbVec3 a, vdbVec3 b)     { return a.x*b.x + a.y*b.y + a.z*b.z; }
static vdbVec3 vdbVecNormalize(vdbVec3 v)
{
    float length = vdbVecLength(v);
    if (length == 0.0f)
        return vdbVec3(0.0f,0.0f,0.0f);
    else
        return v*(1.0f/length);
}
static vdbVec3 vdbVecCross(vdbVec3 a, vdbVec3 b)
{
    return vdbVec3(a.y*b.z - a.z*b.y,
                   a.z*b.x - a.x*b.z,
                   a.x*b.y - a.y*b.x);
}

static vdbMat4 vdbInitMat4(float a00, float a01, float a02, float a03,
                           float a10, float a11, float a12, float a13,
                           float a20, float a21, float a22, float a23,
                           float a30, float a31, float a32, float a33)
{
    #ifdef VDB_MATRIX_COLUMN_MAJOR
    vdbMat4 a = {
        a00, a01, a02, a03,
        a10, a11, a12, a13,
        a20, a21, a22, a23,
        a30, a31, a32, a33
    };
    #else
    vdbMat4 a = {
        a00, a10, a20, a30,
        a01, a11, a21, a31,
        a02, a12, a22, a32,
        a03, a13, a23, a33
    };
    #endif
    return a;
}

static vdbMat4 vdbMatTranspose(vdbMat4 m)
{
    return vdbInitMat4(m(0,0), m(1,0), m(2,0), m(3,0),
                       m(0,1), m(1,1), m(2,1), m(3,1),
                       m(0,2), m(1,2), m(2,2), m(3,2),
                       m(0,3), m(1,3), m(2,3), m(3,3));
}

static vdbMat4 operator+(vdbMat4 a, vdbMat4 b)
{
    vdbMat4 c = {0};
    for (int i = 0; i < 4*4; i++)
        c.data[i] = a.data[i] + b.data[i];
    return c;
}

static vdbMat4 operator*(vdbMat4 a, float b)
{
    vdbMat4 c = {0};
    for (int i = 0; i < 4*4; i++)
        c.data[i] = a.data[i]*b;
    return c;
}

static vdbMat4 operator/(vdbMat4 a, float b)
{
    vdbMat4 c = {0};
    for (int i = 0; i < 4*4; i++)
        c.data[i] = a.data[i]/b;
    return c;
}

static vdbMat4 operator*(float b, vdbMat4 a) { return a*b; }
static vdbMat4 operator/(float b, vdbMat4 a) { return a/b; }
static vdbMat4 vdbMul4x4(vdbMat4 a, vdbMat4 b)
{
    vdbMat4 c = {0};
    for (int row = 0; row < 4; row++)
    for (int col = 0; col < 4; col++)
    {
        c(row,col) = 0.0f;
        for (int i = 0; i < 4; i++)
            c(row,col) += a(row,i)*b(i,col);
    }
    return c;
}
static vdbMat4 operator*(vdbMat4 a, vdbMat4 b) { return vdbMul4x4(a,b); }

static vdbVec4 vdbMul4x1(vdbMat4 a, vdbVec4 b)
{
    vdbVec4 c(0.0f,0.0f,0.0f,0.0f);
    c.x = b.x*a(0,0) + b.y*a(0,1) + b.z*a(0,2) + b.w*a(0,3);
    c.y = b.x*a(1,0) + b.y*a(1,1) + b.z*a(1,2) + b.w*a(1,3);
    c.z = b.x*a(2,0) + b.y*a(2,1) + b.z*a(2,2) + b.w*a(2,3);
    c.w = b.x*a(3,0) + b.y*a(3,1) + b.z*a(3,2) + b.w*a(3,3);
    return c;
}
static vdbVec4 operator*(vdbMat4 a, vdbVec4 b) { return vdbMul4x1(a,b); }

static vdbVec4 vdbMulTranspose4x1(vdbMat4 a, vdbVec4 b)
{
    vdbVec4 c(0.0f,0.0f,0.0f,0.0f);
    c.x = b.x*a(0,0) + b.y*a(1,0) + b.z*a(2,0) + b.w*a(3,0);
    c.y = b.x*a(0,1) + b.y*a(1,1) + b.z*a(2,1) + b.w*a(3,1);
    c.z = b.x*a(0,2) + b.y*a(1,2) + b.z*a(2,2) + b.w*a(3,2);
    c.w = b.x*a(0,3) + b.y*a(1,3) + b.z*a(2,3) + b.w*a(3,3);
    return c;
}

static vdbVec4 vdbMulSE3Inverse(vdbMat4 a, vdbVec4 b)
{
    vdbVec4 c(b.x-a(0,3)*b.w, b.y-a(1,3)*b.w, b.z-a(2,3)*b.w, b.w);
    vdbVec4 d(0.0f,0.0f,0.0f,0.0f);
    d.x = c.x*a(0,0) + c.y*a(1,0) + c.z*a(2,0);
    d.y = c.x*a(0,1) + c.y*a(1,1) + c.z*a(2,1);
    d.z = c.x*a(0,2) + c.y*a(1,2) + c.z*a(2,2);
    d.w = c.w;
    return d;
}

static vdbMat4 vdbMatIdentity()
{
    vdbMat4 result = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    return result;
}

static vdbMat4 vdbMatScale(float x, float y, float z)
{
    vdbMat4 result = { x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1};
    return result;
}

static vdbMat4 vdbMatRotateZ(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a(0,0) = cosf(t); a(0,1) = -sinf(t);
    a(1,0) = sinf(t); a(1,1) = cosf(t);
    return a;
}

static vdbMat4 vdbMatRotateY(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a(0,0) =  cosf(t); a(0,2) = sinf(t);
    a(2,0) = -sinf(t); a(2,2) = cosf(t);
    return a;
}

static vdbMat4 vdbMatRotateX(float t)
{
    vdbMat4 a = vdbMatIdentity();
    a(1,1) = cosf(t); a(1,2) = -sinf(t);
    a(2,1) = sinf(t); a(2,2) = cosf(t);
    return a;
}

static vdbMat4 vdbMatTranslate(float x, float y, float z)
{
    vdbMat4 a = vdbMatIdentity();
    a(0,3) = x;
    a(1,3) = y;
    a(2,3) = z;
    return a;
}

// M = Rx*Ry*Rz
static vdbMat4 vdbMatRotateXYZ(float rx,float ry,float rz)
{
    vdbMat4 Rx = vdbMatRotateX(rx);
    vdbMat4 Ry = vdbMatRotateY(ry);
    vdbMat4 Rz = vdbMatRotateZ(rz);
    vdbMat4 M = vdbMul4x4(Ry,Rz);
            M = vdbMul4x4(Rx, M);
    return M;
}

// M = Rz*Ry*Rx
static vdbMat4 vdbMatRotateZYX(float rz,float ry,float rx)
{
    vdbMat4 Rx = vdbMatRotateX(rx);
    vdbMat4 Ry = vdbMatRotateY(ry);
    vdbMat4 Rz = vdbMatRotateZ(rz);
    vdbMat4 M = vdbMul4x4(Ry,Rx);
            M = vdbMul4x4(Rz, M);
    return M;
}

static vdbMat4 vdbMatSkew(vdbVec3 v)
{
    return vdbInitMat4(
      0 , -v.z,  v.y,  0,
     v.z,   0 , -v.x,  0,
    -v.y,  v.x,   0 ,  0,
      0 ,   0 ,   0 ,  0
    );
}

static vdbMat4 vdbMatOrthogonalize(vdbMat4 R)
// Orthogonalizes the upper-left 3x3 matrix of R.
{
    vdbVec3 x = vdbVec3(R(0,0), R(1,0), R(2,0));
    vdbVec3 y = vdbVec3(R(0,1), R(1,1), R(2,1));
    float e = vdbVecDot(x, y);
    vdbVec3 ny = vdbVecNormalize((y - 0.5f*e*x)/(1.0f-0.25f*e*e));
    vdbVec3 nx = vdbVecNormalize((x - 0.5f*e*ny));
    vdbVec3 nz = vdbVecCross(nx, ny);
    vdbMat4 result = vdbInitMat4(
        nx.x,       ny.x,      nz.x,      R(0,3),
        nx.y,       ny.y,      nz.y,      R(1,3),
        nx.z,       ny.z,      nz.z,      R(2,3),
        R(3,0),  R(3,1), R(3,2), R(3,3)
    );
    return result;
}

#if 0
static vdbVec4 vdbMulInverse()
{
    // http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
    vdb_mat4 inv;
    inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] + m[9]  * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];
    inv[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] - m[8]  * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];
    inv[8] = m[4]  * m[9] * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5] * m[15] + m[8]  * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4]  * m[9] * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5] * m[14] - m[8]  * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2] * m[15] - m[9]  * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2] * m[15] + m[8]  * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0]  * m[9] * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1] * m[15] - m[8]  * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0]  * m[9] * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1] * m[14] + m[8]  * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1]  * m[6] * m[15] - m[1]  * m[7] * m[14] - m[5]  * m[2] * m[15] + m[5]  * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0]  * m[6] * m[15] + m[0]  * m[7] * m[14] + m[4]  * m[2] * m[15] - m[4]  * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0]  * m[5] * m[15] - m[0]  * m[7] * m[13] - m[4]  * m[1] * m[15] + m[4]  * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0]  * m[5] * m[14] + m[0]  * m[6] * m[13] + m[4]  * m[1] * m[14] - m[4]  * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    for (int i = 0; i < 16; i++)
        inv[i] /= det;

    *y = vdb_mul4x1(inv, x);
}
#endif
