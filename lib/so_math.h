// so_math.h - ver 10
// + Vector, matrix math.
// + Linear algebra.
// + GLSL like functions
// + Euclidean transformations.
// + Conversions between Euler angles, quaternions and matrices
// + Representations of SE(3) and SO(3)
// + Conversions between SO(3) and so(3)
// + Conversions between SE(3) and se(3)
//
// :::::::::::::::::::::::::Changelog::::::::::::::::::::::::::
//   3/6/16: Circle-circle intersection test: m_is_circle_circle
//
//   4/1/16: m_smoothstep
//
// 11/12/15: Fixed sign convention on the rotation matrices in
//           mat_rotate_*. They now follow the right-hand rule:
//           Positive angle means counter-clockwise rotation
//           about the respective axis.
//
//  6/12/15: m_sign
//
// 26/11/15: Made it a template library after reading
//           http://www.reedbeta.com/blog/2013/12/28/on-vector-math-libraries/
//           I agree on a lot of the points. I do however not
//           like templates in general, due to poor compiler
//           error messages. Kind of unsure if this was a good
//           idea or not. I don't really use vectors or matrices
//           or dimension greater than 4x4; but I do sometimes
//           use matrices of odd dimensions.
//
// Some earlier entries have been omitted.

#ifndef SO_MATH_HEADER_INCLUDE
#define SO_MATH_HEADER_INCLUDE
#include "math.h"

#define PI     3.14159265359
#define TWO_PI 6.28318530718

template <typename T, int n>
struct Vector
{
    T data[n];
};

template <typename T, int rows, int columns>
struct Matrix
{
    T data[rows*columns];
};

typedef Vector<float, 2>        vec2;
typedef Vector<int, 2>          vec2i;
typedef Vector<unsigned int, 2> vec2u;

typedef Vector<float, 3>        vec3;
typedef Vector<int, 3>          vec3i;
typedef Vector<unsigned int, 3> vec3u;

typedef Vector<float, 4>        vec4;
typedef Vector<int, 4>          vec4i;
typedef Vector<unsigned int, 4> vec4u;

typedef Matrix<float, 2, 2> mat2;
typedef Matrix<float, 3, 3> mat3;
typedef Matrix<float, 4, 4> mat4;

///////////////// Vector Specializations /////////////////
// Specializations for n = 2, 3, 4 so that we can access each
// component of a vector through .a notation, as well as access
// truncated vectors.

template <typename T> struct Vector<T, 2> {
    union {
        T data[2];
        struct { T x, y; };
    };
};

template <typename T> struct Vector<T, 3> {
    union {
        T data[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
        Vector<T, 2> xy;
    };
};

template <typename T> struct Vector<T, 4> {
    union {
        T data[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        Vector<T, 3> xyz;
        Vector<T, 3> rgb;
        Vector<T, 2> xy;
    };
};

///////////////// Matrix Specializations /////////////////
// Specializations for m = n = 2, 3, 4 so that we can access
// each column of a matrix through .a notation.

// The data layout for each matrix is interpreted as column
// order. For example
// | a11 a12 |
// | a21 a22 |
// is stored in memory as [a11, a21, a12, a22].
// The columns of the matrix can be accessed via .a1 or .a2.

template <typename T> struct Matrix<T, 2, 2> {
    union {
        T data[4];
        struct { T a11, a21, a12, a22; };
        struct { Vector<T, 2> a1, a2; };
    };
};

template <typename T> struct Matrix<T, 3, 3> {
    union {
        T data[9];
        struct { T a11, a21, a31,
                   a12, a22, a32,
                   a13, a23, a33; };
        struct { Vector<T, 3> a1, a2, a3; };
    };
};

template <typename T> struct Matrix<T, 4, 4> {
    union {
        T data[16];
        struct { T a11, a21, a31, a41,
                   a12, a22, a32, a42,
                   a13, a23, a33, a43,
                   a14, a24, a34, a44; };
        struct { Vector<T, 4> a1, a2, a3, a4; };
    };
};

//////////// Matrix and vector constructors ////////////
// Convenience functions for constructing matrices and
// vectors, from components or other vectors.

vec2 m_vec2(float s)                            { vec2 result = { s, s       }; return result; }
vec3 m_vec3(float s)                            { vec3 result = { s, s, s    }; return result; }
vec4 m_vec4(float s)                            { vec4 result = { s, s, s, s }; return result; }
vec2 m_vec2(float x, float y)                   { vec2 result = { x, y       }; return result; }
vec3 m_vec3(float x, float y, float z)          { vec3 result = { x, y, z    }; return result; }
vec4 m_vec4(float x, float y, float z, float w) { vec4 result = { x, y, z, w }; return result; }
vec4 m_vec4(vec3 xyz, float w)                  { vec4 result = { xyz.x, xyz.y, xyz.z, w }; return result; }

template <typename T, int n>
Matrix<T, n, n> m_identity_()
{
    Matrix<T, n, n> result = { };
    for (unsigned int i = 0; i < n; i++)
        result.data[i*n+i] = 1;
    return result;
}

template <typename T>
Matrix<T, 2, 2> m_mat2_(Vector<T, 2> a1,
                        Vector<T, 2> a2)
{
    mat2 result = { };
    result.a1 = a1;
    result.a2 = a2;
    return result;
}

template <typename T>
Matrix<T, 3, 3> m_mat3_(Vector<T, 3> a1,
                        Vector<T, 3> a2,
                        Vector<T, 3> a3)
{
    mat3 result = { };
    result.a1 = a1;
    result.a2 = a2;
    result.a3 = a3;
    return result;
}

template <typename T>
Matrix<T, 3, 3> m_mat3_(Matrix<T, 4, 4> m)
{
    mat3 result = { };
    result.a1 = m.a1.xyz;
    result.a2 = m.a2.xyz;
    result.a3 = m.a3.xyz;
    return result;
}

template <typename T>
Matrix<T, 4, 4> m_mat4_(Vector<T, 4> a1,
                        Vector<T, 4> a2,
                        Vector<T, 4> a3,
                        Vector<T, 4> a4)
{
    mat4 result = { };
    result.a1 = a1;
    result.a2 = a2;
    result.a3 = a3;
    result.a4 = a4;
    return result;
}

template <typename T>
Matrix<T, 4, 4> m_mat4_(Matrix<T, 3, 3> m)
{
    mat4 result = m_identity_<T, 4>();
    result.a1.xyz = m.a1;
    result.a2.xyz = m.a2;
    result.a3.xyz = m.a3;
    result.a4.xyz = m.a4;
    return result;
}

#define m_mat2  m_mat2_<float>
#define m_mat2i m_mat2_<int>
#define m_mat2u m_mat2_<unsigned int>

#define m_mat3  m_mat3_<float>
#define m_mat3i m_mat3_<int>
#define m_mat3u m_mat3_<unsigned int>

#define m_mat4  m_mat4_<float>
#define m_mat4i m_mat4_<int>
#define m_mat4u m_mat4_<unsigned int>

#define m_id2   m_identity_<float, 2>
#define m_id2i  m_identity_<int, 2>
#define m_id2u  m_identity_<unsigned int, 2>

#define m_id3   m_identity_<float, 3>
#define m_id3i  m_identity_<int, 3>
#define m_id3u  m_identity_<unsigned int, 3>

#define m_id4   m_identity_<float, 4>
#define m_id4i  m_identity_<int, 4>
#define m_id4u  m_identity_<unsigned int, 4>

///////////////// Matrix functions /////////////////
template <typename T, int r, int c>
T *m_element(Matrix<T, r, c> *m, int row, int column)
{
    return m->data + row + column * r;
}

template <typename T, int r, int c>
Vector<T, r> m_column(Matrix<T, r, c> m, int column)
{
    Vector<T, r> result = {};
    for (int i = 0; i < r; i++)
        result.data[i] = m.data[i + column * r];
    return result;
}

template <typename T, int ra, int ca, int cb>
Matrix<T, ra, cb> operator *(Matrix<T, ra, ca> a,
                             Matrix<T, ca, cb> b)
{
    Matrix<T, ra, cb> result = {};
    T *entry = (T*)result.data;
    for (int col = 0; col < cb; col++)
    for (int row = 0; row < ra; row++)
    {
        for (int i = 0; i < ca; i++)
        {
            T x = a.data[i * ra + row];
            T y = b.data[i + col * ca];
            *entry += x * y;
        }
        entry++;
    }
    return result;
}

template <typename T, int r, int c>
Matrix<T, r, c> operator +(Matrix<T, r, c> a, Matrix<T, r, c> b)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] + b.data[i];
    return result;
}

template <typename T, int r, int c>
Matrix<T, r, c> operator -(Matrix<T, r, c> a, Matrix<T, r, c> b)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] - b.data[i];
    return result;
}

template <int r, int c>
Matrix<double, r, c> operator *(Matrix<double, r, c> a, double s)
{
    Matrix<double, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] * s;
    return result;
}

template <int r, int c>
Matrix<float, r, c> operator *(Matrix<float, r, c> a, float s)
{
    Matrix<float, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] * s;
    return result;
}

template <int r, int c>
Matrix<double, r, c> operator *(double s, Matrix<double, r, c> a) { return a*s; }

template <int r, int c>
Matrix<float, r, c> operator *(float s, Matrix<float, r, c> a) { return a*s; }

template <typename T, int r, int c>
Matrix<T, r, c> operator -(Matrix<T, r, c> a)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = -a.data[i];
    return result;
}

template <typename T, int r, int c>
Matrix<T, r, c> operator *(T s, Matrix<T, r, c> a)
{
    Matrix<T, r, c> result = {};
    for (int i = 0; i < r*c; i++) result.data[i] = a.data[i] * s;
    return result;
}

template <typename T, int r, int c>
Matrix<T, c, r> m_transpose(Matrix<T, r, c> m)
{
    Matrix<T, c, r> result = {};
    for (int row = 0; row < r; row++)
    for (int col = 0; col < c; col++)
    {
        T *a = m_element(&result, col, row);
        T *b = m_element(&m, row, col);
        *a = *b;
    }
    return result;
}

///////////////// Vector functions /////////////////
// Let a := vector of dimension N and type T
//     b := another vector of dimension N and type T
//     s := scalar of same type T
//
// Then the following operators are defined
//
//     -A        :: Component-wise negation of A
//     A+B = B+A :: B added component-wise to A
//     A-B       :: B subtracted component-wise from A
//     A*s = s*A :: s multiplied each component of A
//     A*B = B*A :: A multiplied component-wise by B
//     A/B       :: A divided component-wise by B
//     dot(A, B) :: The inner product of A and B
#define vec_template template <typename T, int n>
#define tvec Vector<T, n>

vec_template
tvec operator +(tvec a, tvec b)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = a.data[i] + b.data[i];
    return result;
}

vec_template
tvec &operator +=(tvec &a, tvec b)
{
    for (int i = 0; i < n; i++)
        a.data[i] += b.data[i];
    return a;
}

vec_template
tvec operator -(tvec a, tvec b)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = a.data[i] - b.data[i];
    return result;
}

vec_template
tvec &operator -=(tvec &a, tvec b)
{
    for (int i = 0; i < n; i++)
        a.data[i] -= b.data[i];
    return a;
}

vec_template
tvec operator -(tvec a)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = -a.data[i];
    return result;
}

vec_template
tvec operator *(tvec v, T s)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = v.data[i] * s;
    return result;
}

vec_template
tvec &operator *=(tvec &v, T s)
{
    for (int i = 0; i < n; i++)
        v.data[i] *= s;
    return v;
}

vec_template
tvec operator *(T s, tvec v)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = v.data[i] * s;
    return result;
}

vec_template
tvec operator *(tvec a, tvec b)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = a.data[i] * b.data[i];
    return result;
}

vec_template
tvec &operator *=(tvec &a, tvec b)
{
    for (int i = 0; i < n; i++)
        a.data[i] *= b.data[i];
    return a;
}

vec_template
tvec operator /(tvec a, tvec b)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = a.data[i] / b.data[i];
    return result;
}

vec_template
tvec &operator /=(tvec &a, tvec b)
{
    for (int i = 0; i < n; i++)
        a.data[i] /= b.data[i];
    return a;
}

vec_template
tvec operator /(tvec v, T s)
{
    tvec result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = v.data[i] / s;
    return result;
}

vec_template
tvec &operator /=(tvec &v, T s)
{
    for (int i = 0; i < n; i++)
        v.data[i] /= s;
    return v;
}

vec_template
T m_dot(tvec a, tvec b)
{
    T result = 0;
    for (int i = 0; i < n; i++)
        result += a.data[i] * b.data[i];
    return result;
}

template<typename T>
void m_vec_max_(T *data, int n, T *max_component, int *max_index)
{
    int maxi = 0;
    T maxc = data[maxi];
    for (int i = 1; i < n; i++)
    {
        if (data[i] > maxc)
        {
            maxc = data[i];
            maxi = i;
        }
    }
    if (max_component)
        *max_component = maxc;
    if (max_index)
        *max_index = maxi;
}

#define m_vec_max(vec, cmp, ind) m_vec_max_((vec).data, sizeof((vec).data)/sizeof((vec).data[0]), cmp, ind)

#undef vec_template
#undef tvec

///////////// Vector matrix functions /////////////
template <typename T, int r, int c>
Vector<T, r> operator *(Matrix<T, r, c> m, Vector<T, c> x)
{
    Vector<T, r> result = {};
    for (int col = 0; col < c; col++)
    for (int row = 0; row < r; row++)
        result.data[row] += m.data[row + col * r] * x.data[col];
    return result;
}

mat3 m_outer_product(vec3 a, vec3 b)
{
    mat3 result;
    result.a11 = a.x*b.x; result.a12 = a.x*b.y; result.a13 = a.x*b.z;
    result.a21 = a.y*b.x; result.a22 = a.y*b.y; result.a23 = a.y*b.z;
    result.a31 = a.z*b.x; result.a32 = a.z*b.y; result.a33 = a.z*b.z;
    return result;
}

// Because I like readable error messages when there is a
// dimension mismatch in the most common operations.
vec2 operator *(mat2 m, vec2 b) { return m.a1*b.x + m.a2*b.y; }
vec3 operator *(mat3 m, vec3 b) { return m.a1*b.x + m.a2*b.y + m.a3*b.z; }
vec4 operator *(mat4 m, vec4 b) { return m.a1*b.x + m.a2*b.y + m.a3*b.z + m.a4*b.w; }

template <int n>
float m_length(Vector<float, n> v)
{
    float result = sqrt(m_dot(v, v));
    return result;
}

float m_fast_inv_sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;          // Integer representation of float
    i = 0x5f3759df - (i >> 1);  // Initial guess
    x = *(float*)&i;            // Converted to floating point
    x = x*(1.5f-(xhalf*x*x));   // One round of Newton-Raphson's method
    return x;
}

template <int n>
Vector<float, n> m_normalize(Vector<float, n> v)
{
    Vector<float, n> result = v * m_fast_inv_sqrt(m_dot(v, v));
    return result;
}

///////////////// GLSL-like stuff ////////////////
int m_sign(int x)       { return x < 0 ? -1 : 1; }
int m_abs(int x)        { return x < 0 ? -x : x; }
int m_min(int x, int y) { return x < y ? x : y; }
int m_max(int x, int y) { return x > y ? x : y; }
int m_clamp(int x, int low, int high)
{
    return x < low ? low : (x > high ? high : x);
}

float m_sign(float x)         { return x < 0 ? -1.0f : +1.0f; }
float m_abs(float x)          { return x < 0 ? -x : x; }
float m_min(float x, float y) { return x < y ? x : y; }
float m_max(float x, float y) { return x > y ? x : y; }
float m_clamp(float x, float low, float high)
{
    return x < low ? low : (x > high ? high : x);
}

float m_square(float x) { return x*x; }

// return: Linear mapping from [t0, t1] to [y0, y1]
float m_map(float t0, float t1, float t, float y0, float y1)
{
    return m_clamp(y0 + (y1 - y0) * (t - t0) / (t1 - t0), y0, y1);
}

// return: Linear mapping from [0, 1] to [low, high]
float m_mix(float low, float high, float t)
{
    return low + (high - low) * t;
}

float m_smoothstep(float lo, float hi, float x)
{
    float t = m_clamp((x - lo) / (hi - lo), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

template <typename T, int n>
Vector<T, n> m_clamp(Vector<T, n> v, Vector<T, n> low, Vector<T, n> high)
{
    Vector<T, n> result = {};
    for (int i = 0; i < n; i++)
        result.data[i] = m_clamp(v.data[i], low.data[i], high.data[i]);
    return result;
}

template <int n>
Vector<float, n> m_mix(Vector<float, n> low,
                       Vector<float, n> high,
                       float t)
{
    return low + (high - low) * t;
}

///////////////// Linear algebra /////////////////
vec3 m_cross(vec3 a, vec3 b)
{
    return m_vec3(a.y*b.z-a.z*b.y,
                  a.z*b.x-a.x*b.z,
                  a.x*b.y-a.y*b.x);
}

// return: The skew-symmetric matrix form of the
//         cross product operator applied by v.
mat3 m_skew(vec3 v)
{
    mat3 result = {0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0};
    return result;
}

// return: A vector that is orthogonal to v.
//         - Always works if the input is non-zero.
//         - Doesn’t require the input to be normalised.
//         - Doesn’t normalize the output.
// thanks: http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
vec3 m_orthogonal_vector(vec3 v)
{
    return m_abs(v.x) > m_abs(v.z) ? m_vec3(-v.y, v.x, 0.0)
                                   : m_vec3(0.0, -v.z, v.y);
}

// return: A transformation matrix in SE3 (rotation and translation)
mat4 m_se3(mat3 R, vec3 r)
{
    mat4 result = m_id4();
    result.a1.xyz = R.a1;
    result.a2.xyz = R.a2;
    result.a3.xyz = R.a3;
    result.a4.xyz = r;
    return result;
}

void m_se3_decompose(mat4 se3, mat3 *R, vec3 *p)
{
    *R = m_mat3(se3);
    *p = se3.a4.xyz;
}

// return: The inverse of a SO3 matrix
mat4 m_se3_inverse(mat4 m)
{
    mat3 R = m_transpose(m_mat3(m));
    vec3 r = -R * m.a4.xyz;
    return m_se3(R, r);
}

///////////////// Unit quaternions /////////////////
// Quaternions are represented as a vec4, with the
// xyz components representing the imaginary part
// and w component representing the real part.
typedef vec4 quat;

// return: The quaternion describing the rotation of
// _angle_ radians about a normalized axis _axis_.
quat m_quat_from_angle_axis(vec3 axis, float angle)
{
    quat result = {};
    float s = sin(angle/2.0f);
    float c = cos(angle/2.0f);
    result.x = axis.x*s;
    result.y = axis.y*s;
    result.z = axis.z*s;
    result.w = c;
    return result;
}

// return: A quaternion describing the rotation
// represented by the given Euler angles that
// parametrize the rotation matrix given by
//   R = Rx(ex)Ry(ey)Rz(ez)
quat m_quat_from_euler(float ex, float ey, float ez)
{
    // Implements Shepperd's method
    float cz = cos(ez); float sz = sin(ez);
    float cy = cos(ey); float sy = sin(ey);
    float cx = cos(ex); float sx = sin(ex);

    float R11 = cy*cz;
    float R12 = cz*sx*sy - cx*sz;
    float R13 = sx*sz + cx*cz*sy;
    float R21 = cy*sz;
    float R22 = cx*cz + sx*sy*sz;
    float R23 = cx*sy*sz - cz*sx;
    float R31 = -sy;
    float R32 = cy*sx;
    float R33 = cx*cy;

    float T = R11 + R22 + R33;
    vec4 v = { T, R11, R22, R33 };
    float rii; int i;
    m_vec_max(v, &rii, &i);

    float z0,z1,z2,z3;

    if (i == 0)
    {
        z0 = sqrt(1.0f + 2.0f*rii - T);
        z1 = (R32 - R23) / z0;
        z2 = (R13 - R31) / z0;
        z3 = (R21 - R12) / z0;
    }
    else if (i == 1)
    {
        z1 = sqrt(1.0f + 2.0f*rii - T);
        z0 = (R32 - R23) / z1;
        z2 = (R21 + R12) / z1;
        z3 = (R13 + R31) / z1;
    }
    else if (i == 2)
    {
        z2 = sqrt(1.0f + 2.0f*rii - T);
        z0 = (R13 - R31) / z2;
        z1 = (R21 + R12) / z2;
        z3 = (R32 + R23) / z2;
    }
    else
    {
        z3 = sqrt(1.0f + 2.0f*rii - T);
        z0 = (R21 - R12) / z3;
        z1 = (R13 + R31) / z3;
        z2 = (R32 + R23) / z3;
    }

    quat result;
    result.xyz = 0.5f * m_vec3(z1, z2, z3);
    result.w = 0.5f* z0;
    return result;
}

// return: The SO3 rotation matrix of q
mat3 m_quat_to_so3(quat q)
{
    mat3 e_skew = m_skew(q.xyz);
    mat3 result = m_id3() + 2.0f*q.w*e_skew + 2.0f*e_skew*e_skew;
    return result;
}

// return: The quaternion product q MUL r
quat m_quat_mul(quat q, quat r)
{
    quat result = {};
    result.w = q.w*r.w - m_dot(q.xyz, r.xyz);
    result.xyz = q.w*r.xyz + r.w*q.xyz + m_skew(q.xyz)*r.xyz;
    return result;
}

// return: The matrix which when left-multiplied by a vector v=(x,y,z)
// produces the same result as m_quat_mul(q, m_vec4(v, 0)).
Matrix<float,4,3> m_quat_mul_matrix(quat q)
{
    Matrix<float,4,3> result = {};
    *m_element(&result, 0, 0) = +q.w;
    *m_element(&result, 1, 0) = +q.z;
    *m_element(&result, 2, 0) = -q.y;
    *m_element(&result, 3, 0) = -q.x;

    *m_element(&result, 0, 1) = -q.z;
    *m_element(&result, 1, 1) = +q.w;
    *m_element(&result, 2, 1) = +q.x;
    *m_element(&result, 3, 1) = -q.y;

    *m_element(&result, 0, 2) = +q.y;
    *m_element(&result, 1, 2) = -q.x;
    *m_element(&result, 2, 2) = +q.w;
    *m_element(&result, 3, 2) = -q.z;
    return result;
}

///////////////// SO(3)/SE(3) maps /////////////////
// Define w to be a rotation axis, and t to be an
// angle about this axis. Then wt is angular twist.
// so3_exp(wt): Returns the rotation matrix describing
//              a rotation about w for t radians.
// so3_log(R):  Returns the angular twist vector
//              describing the rotation matrix R.
//
// Define v to be a linear velocity. Then (w, v)
// is an element of the se(3) Lie algebra, also
// called a twist vector, x.
//
// se3_exp(x): The matrix H such that
//               DH/Dt = x H
//             evaluated at t=1.
// se3_log(H): The vector x such that the solution
//             of DJ/Dt = x J at t=1 is H.

mat3 m_so3_exp(vec3 wt)
{
    r32 t = m_length(wt);
    if (t < 0.01f)
    {
        return m_id3() + m_skew(wt);
    }
    else
    {
        mat3 W = m_skew(wt/t);
        return m_id3() + sin(t)*W + (1.0f-cos(t))*W*W;
    }
}

// https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Log_map_from_SO.283.29_to_so.283.29
vec3 m_so3_log(mat3 R)
{
    r32 tr = R.a11+R.a22+R.a33;
    r32 theta = acos((tr-1.0f)/2.0f);
    if (theta < 0.01f)
    {
        // For small angles
        // R = I + theta K = I + W
        vec3 w;
        w.x = R.a32;
        w.y = R.a13;
        w.z = R.a21;
        return w;
    }
    else
    {
        vec3 k;
        r32 s = sin(theta);
        k.x = (R.a32-R.a23)/(2.0f*s);
        k.y = (R.a13-R.a31)/(2.0f*s);
        k.z = (R.a21-R.a12)/(2.0f*s);
        vec3 w = k*theta;
        return w;
    }
}

// Motilal Agrawal.
// A Lie Algebraic Approach for Consistent Pose Registration for General Euclidean Motion.
// Proceedings of the 2006 IEEE/RSJ International Conference on Intelligent Robots and Systems
void m_se3_log(mat4 SE3, vec3 *out_w, vec3 *out_v)
{
    mat3 R;
    vec3 T;
    m_se3_decompose(SE3, &R, &T);
    vec3 w = m_so3_log(R);

    r32 t = m_length(w);
    if (t < 0.01f)
    {
        *out_v = T;
    }
    else
    {
        mat3 W = m_skew(w);
        r32 s = sin(t);
        r32 c = cos(t);
        mat3 M = m_id3() - 0.5f*W + W*W*((2.0f*s - t*(1.0f+c))/(2.0f*t*t*s));
        *out_v = M*T;
    }

    *out_w = w;
}

mat4 se3_exp(vec3 w, vec3 v)
{
    mat3 R = m_so3_exp(w);
    r32 t = m_length(w);
    vec3 T = v;
    if (t >= 0.01f)
    {
        vec3 a = w / t;
        mat3 A = m_skew(a);
        T = v + (A*(1.0f-cos(t)) + A*A*(t-sin(t)))*(v/t);

        // This is equivalent
        // T = m_outer_product(a, a)*v + ((m_id3()-R)*m_skew(a))*(v/t);
    }
    mat4 result = m_se3(R, T);
    return result;
}

mat4
mat_scale(float s)
{
    mat4 result = {};
    result.a1.x = s;
    result.a2.y = s;
    result.a3.z = s;
    result.a4.w = 1;
    return result;
}

mat4
mat_scale(float x, float y, float z)
{
    mat4 result = {};
    result.a1.x = x;
    result.a2.y = y;
    result.a3.z = z;
    result.a4.w = 1;
    return result;
}

mat4
mat_scale(vec3 s)
{
    return mat_scale(s.x, s.y, s.z);
}

// Positive angle indicates counter-clockwise rotation about x axis
// according to the right hand rule.
mat4
mat_rotate_x(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = m_id4();
    result.a2.y = c;
    result.a2.z = s;
    result.a3.y = -s;
    result.a3.z = c;
    return result;
}

// Positive angle indicates counter-clockwise rotation about y axis
mat4
mat_rotate_y(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = m_id4();
    result.a1.x = c;
    result.a1.z = -s;
    result.a3.x = s;
    result.a3.z = c;
    return result;
}

// Positive angle indicates counter-clockwise rotation about z axis
mat4
mat_rotate_z(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = m_id4();
    result.a1.x = c;
    result.a1.y = s;
    result.a2.x = -s;
    result.a2.y = c;
    return result;
}

mat4
mat_translate(vec3 x)
{
    mat4 result = m_id4();
    result.a4.x = x.x;
    result.a4.y = x.y;
    result.a4.z = x.z;
    return result;
}

mat4
mat_translate(float x, float y, float z)
{
    mat4 result = m_id4();
    result.a4.x = x;
    result.a4.y = y;
    result.a4.z = z;
    return result;
}

mat4
mat_ortho(float left, float right, float bottom, float top)
{
    mat4 result = m_id4();
    result.a1.x = 2.0f / (right - left);
    result.a2.y = 2.0f / (top - bottom);
    return result;
}

/*
Map x from [left, right] to [-1, +1]
    y from [bottom, top] to [-1, +1]
    z from [zn, zf]      to [-1, +1]
*/
mat4
mat_ortho_depth(float left, float right, float bottom, float top, float zn, float zf)
{
    mat4 result = {};
    result.a1.x = 2.0f / (right - left);
    result.a2.y = 2.0f / (top - bottom);
    result.a3.z = 2.0f / (zn - zf);
    result.a4.x = (right + left) / (left - right);
    result.a4.y = (top + bottom) / (bottom - top);
    result.a4.z = (zf + zn) / (zn - zf);
    result.a4.w = 1.0f;
    return result;
}

mat4
mat_perspective(float fov, float width, float height, float zn, float zf)
{
    mat4 result = {};
    float a = width / height;
    result.a1.x = 1.0f / (a * tan(fov / 2.0f));
    result.a2.y = 1.0f / tan(fov / 2.0f);
    result.a3.z = (zn + zf) / (zn - zf);
    result.a3.w = -1.0f;
    result.a4.z = 2.0f * zn * zf / (zn - zf);
    return result;
}

/////////////// Intersection tests ///////////////

// CIRCLE CIRCLE INTERSECTION
// Both x1 and x2 are set even if there was only one intersection
// RETURN
//  FALSE No intersection or infinitely many intersections
//   TRUE One or two intersections
bool
m_is_circle_circle(vec2 a, vec2 b, r32 ra, r32 rb, vec2 *x1, vec2 *x2)
{
    vec2 d = b-a;
    vec2 p = m_vec2(-d.y, d.x);
    r32 ld = m_dot(d,d);
    if (ld < 0.001f)
    {
        return false;
    }
    else
    {
        r32 t = (ra*ra-rb*rb+ld)/(2.0f*ld);
        r32 D = ra*ra/ld - t*t;
        if (D < 0.0f)
        {
            return false;
        }
        else
        {
            r32 s = sqrt(D);
            *x1 = a + t*d + s*p;
            *x2 = a + t*d - s*p;
            return true;
        }
    }
}

#endif
