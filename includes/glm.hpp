#ifndef GLM_HPP
#define GLM_HPP

#include <cmath>
#include <cstring>

#define PI 3.14159265359f

namespace glm {

struct vec3 {
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    float& operator[](int i) { return i==0? x : (i==1? y : z); }
    const float& operator[](int i) const { return i==0? x : (i==1? y : z); }
    const vec3 operator-(const vec3 other) const { return vec3(x - other.x, y - other.y, z - other.z); }
    const vec3 operator+(const vec3 other) const { return vec3(x + other.x, y + other.y, z + other.z); }
};

struct vec2 {
    float x, y;
    vec2() : x(0), y(0) {}
    vec2(float X, float Y) : x(X), y(Y) {}
    float& operator[](int i) { return i==0? x : y; }
    const float& operator[](int i) const { return i==0? x : y; }
};

struct vec4 {
    float x, y, z, w;
    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
    float& operator[](int i) { return i==0? x : (i==1? y : (i==2? z : w)); }
    const float& operator[](int i) const { return i==0? x : (i==1? y : (i==2? z : w)); }
};

struct mat4 {
    float data[16];

    mat4() {
    }

    explicit mat4(float diagonal) {
        for (int i = 0; i < 16; ++i) data[i] = 0.0f;
        data[0] = diagonal;
        data[5] = diagonal;
        data[10] = diagonal;
        data[15] = diagonal;
    }
    float* operator[](int col) { return &data[col * 4]; }
    const float* operator[](int col) const { return &data[col * 4]; }
};

// mat3 and mat2 to satisfy shader helper
struct mat3 {
    float data[9];
    mat3() {}
    explicit mat3(float diagonal) {
        for (int i = 0; i < 9; ++i) data[i] = 0.0f;
        data[0] = diagonal; data[4] = diagonal; data[8] = diagonal;
    }
    float* operator[](int col) { return &data[col*3]; }
    const float* operator[](int col) const { return &data[col*3]; }
};

struct mat2 {
    float data[4];
    mat2() {}
    explicit mat2(float diagonal) {
        for (int i = 0; i < 4; ++i) data[i] = 0.0f;
        data[0] = diagonal; data[3] = diagonal;
    }
    float* operator[](int col) { return &data[col*2]; }
    const float* operator[](int col) const { return &data[col*2]; }
};

// multiply two mat4 (column-major)
inline mat4 operator*(const mat4& A, const mat4& B) {
    mat4 R;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int i = 0; i < 4; ++i) {
                sum += A.data[i*4 + row] * B.data[col*4 + i];
            }
            R.data[col*4 + row] = sum;
        }
    }
    return R;
}

// translate: returns T * M (so transform = translate(transform, v) behaves like glm)
inline mat4 translate(const mat4& M, const vec3& v) {
    mat4 T(1.0f);
    T.data[12] = v.x;
    T.data[13] = v.y;
    T.data[14] = v.z;
    return M * T;
}

// rotate: angle in radians, axis normalized-ish
inline mat4 rotate(const mat4& M, float angle, const vec3& axis) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    float x = axis.x, y = axis.y, z = axis.z;
    // normalize
    float len = std::sqrt(x*x + y*y + z*z);
    if (len > 0.0f) { x /= len; y /= len; z /= len; }

    mat4 R(1.0f);
    R.data[0] = x*x*(1-c) + c;
    R.data[1] = x*y*(1-c) + z*s;
    R.data[2] = x*z*(1-c) - y*s;

    R.data[4] = y*x*(1-c) - z*s;
    R.data[5] = y*y*(1-c) + c;
    R.data[6] = y*z*(1-c) + x*s;

    R.data[8] = z*x*(1-c) + y*s;
    R.data[9] = z*y*(1-c) - x*s;
    R.data[10] = z*z*(1-c) + c;

    return M * R;
}

// value_ptr returns pointer to first element in column-major layout
inline const float* value_ptr(const mat4& m) {
    return m.data;
}

// Convert degrees to radians
inline float radians(float degrees) {
    // 180 degrees = π radians
    float radians = degrees * (PI / 180.0f);
    return radians;
}

// Create a perspective projection matrix (stub - implement yourself)
inline mat4 perspective(float fovy, float aspect, float znear, float zfar) {
    // fovy is the vertical field of view angle, in radians
    float tanHalfFovy = std::tan(fovy / 2.0f);

    mat4 Result(0.0f);
    // Column-major order
    Result.data[0] = 1.0f / (aspect * tanHalfFovy); // m00
    Result.data[5] = 1.0f / (tanHalfFovy);           // m11
    Result.data[10] = -(zfar + znear) / (zfar - znear); // m22
    Result.data[11] = -1.0f;                         // m23
    Result.data[14] = -(2.0f * zfar * znear) / (zfar - znear); // m32
    Result.data[15] = 0.0f;                          // m33
    return Result;
}

inline vec3 normalize(const vec3 &v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0f) {
        return vec3(v.x / len, v.y / len, v.z / len);
    }
    return v;
}

inline float dot(const vec3 &a, const vec3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross(const vec3 &a, const vec3 &b) {
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline mat4 scale(const mat4& M, const vec3& v) {
    mat4 S(1.0f);
    S.data[0] = v.x;
    S.data[5] = v.y;
    S.data[10] = v.z;
    return M * S;
}

inline mat4 lookAt(const vec3 &cam, const vec3 &center, const vec3 &up) {
    vec3 f = normalize(center - cam);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    mat4 Result(1.0f);
    Result.data[0] = s.x;
    Result.data[1] = u.x;
    Result.data[2] = -f.x;
    Result.data[4] = s.y;
    Result.data[5] = u.y;
    Result.data[6] = -f.y;
    Result.data[8] = s.z;
    Result.data[9] = u.z;
    Result.data[10] = -f.z;
    Result.data[12] = -dot(s, cam);
    Result.data[13] = -dot(u, cam);
    Result.data[14] = dot(f, cam);

    return Result;
}

} // namespace glm

#endif