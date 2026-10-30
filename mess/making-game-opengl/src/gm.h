#ifndef GM_H_
#define GM_H_

#include <math.h>

typedef struct { float x, y; } Vec2;
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { float data[16]; } Mat4;

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef DEG2RAD
#define DEG2RAD(deg) (((PI)/180.0f)*(deg))
#endif
#ifndef RAD2DEG
#define RAD2DEG(rad) (((180.0f/(PI))*(deg))
#endif

static inline Vec3 vec3(float x, float y, float z) { return (Vec3){ x, y, z }; }

float vec3_dot(Vec3 a, Vec3 b);
float vec3_length(Vec3 a);
Vec3  vec3_normalize(Vec3 a);
Vec3  vec3_cross(Vec3 a, Vec3 b);
Vec3  vec3_sub(Vec3 a, Vec3 b);
Vec3  vec3_mul_scalar(Vec3 a, float scalar);
Vec3  vec3_add(Vec3 a, Vec3 b);
Mat4  mat4_eye(float v);
Mat4  mat4_ortho(float left, float right, float bottom, float top, float far, float near);
Mat4  mat4_perspective(float fov_radians, float aspect_ratio, float near, float far);
Mat4  mat4_dot(Mat4 a, Mat4 b);
Mat4  mat4_translate(Mat4 a, Vec3 v3);
Mat4  mat4_scale(Mat4 a, Vec3 v3);
Mat4  mat4_rotate_x(Mat4 a, float angle_radians);
Mat4  mat4_rotate_y(Mat4 a, float angle_radians);
Mat4  mat4_rotate_z(Mat4 a, float angle_radians);
Mat4  mat4_look_at(Vec3 camera_pos, Vec3 camera_target, Vec3 world_up_dir);

#endif // GM_H_

#ifdef GM_IMPLEMENTATION


float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float vec3_length(Vec3 a)
{
    float length_sqr = a.x*a.x + a.y*a.y + a.z*a.z;
    return sqrtf(length_sqr);
}

Vec3 vec3_normalize(Vec3 a)
{
    float length = vec3_length(a);
    if(length == 0.0f) return a;
    float ilength = 1.0f/length;
    a.x *= ilength;
    a.y *= ilength;
    a.z *= ilength;
    return a;
}

Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return vec3(a.x-b.x, a.y-b.y, a.z-b.z);
}

Vec3 vec3_mul_scalar(Vec3 a, float scalar)
{
    return vec3(a.x*scalar, a.y*scalar, a.z*scalar);
}

Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return vec3(a.x+b.x, a.y+b.y, a.z+b.z);
}

Mat4 mat4_eye(float v)
{
    Mat4 res = {0};
    res.data[0] = v;
    res.data[5] = v;
    res.data[10] = v;
    res.data[15] = v;
    return res;
}

Mat4 mat4_ortho(float left, float right, float bottom, float top, float far, float near)
{
    Mat4 result = mat4_eye(1.0f);
    float irl = 1.0f / (right - left);
    float itb = 1.0f / (top - bottom);
    float ifn = 1.0f / (far - near);
    result.data[0] = -2*irl;
    result.data[3] = (right+left)*irl;
    result.data[5] = -2*itb;
    result.data[7] = (top+bottom)*itb;
    result.data[10] = -2*ifn;
    result.data[11] = -(far+near)*ifn;
    return result;
}

Mat4 mat4_perspective(float fov_radians, float aspect_ratio, float near, float far)
{
    Mat4 result = mat4_eye(1.0f);
    float thfov = tan(fov_radians/2);
    result.data[0] = 1.0f / (aspect_ratio * thfov);
    result.data[5] = 1.0f / thfov;
    result.data[10] = -((far + near) / (far - near));
    result.data[11] = -((2.0f * far * near) / (far - near));
    result.data[14] = -1.0f;
    return result;
}

Mat4 mat4_dot(Mat4 a, Mat4 b)
{
    Mat4 result;
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {            
            result.data[i * 4 + j] = 0.0f;
            for(int k = 0; k < 4; ++k) {
                result.data[i * 4 + j] += 
                    a.data[i * 4 + k] *
                    b.data[k * 4 + j];
            }
        }
    }
    return result;
}

Mat4 mat4_translate(Mat4 a, Vec3 v3)
{
    Mat4 result = mat4_eye(1.0f);
    result.data[3]  = v3.x;
    result.data[7]  = v3.y;
    result.data[11] = v3.z;
    return mat4_dot(a, result);
}

Mat4 mat4_scale(Mat4 a, Vec3 v3)
{
    Mat4 result = mat4_eye(1.0f);
    result.data[0]  = v3.x;
    result.data[5]  = v3.y;
    result.data[10] = v3.z;
    return mat4_dot(a, result);
}

Mat4 mat4_rotate_x(Mat4 a, float angle_radians)
{
    Mat4 result = mat4_eye(1.0f);
    float s = sin(angle_radians);
    float c = cos(angle_radians);
    result.data[5]  =  c;
    result.data[6]  = -s;
    result.data[9]  =  s;
    result.data[10] =  c;
    return mat4_dot(a, result);
}

Mat4 mat4_rotate_y(Mat4 a, float angle_radians)
{
    Mat4 result = mat4_eye(1.0f);
    float s = sin(angle_radians);
    float c = cos(angle_radians);
    result.data[0]  =  c;
    result.data[2]  =  s;
    result.data[8]  = -s;
    result.data[10] =  c;
    return mat4_dot(a, result);
}

Mat4 mat4_rotate_z(Mat4 a, float angle_radians)
{
    Mat4 result = mat4_eye(1.0f);
    float s = sin(angle_radians);
    float c = cos(angle_radians);
    result.data[0]  =  c;
    result.data[1]  = -s;
    result.data[4]  =  s;
    result.data[5]  =  c;
    return mat4_dot(a, result);
}

Mat4 mat4_look_at(Vec3 camera_pos, Vec3 camera_target, Vec3 world_up_dir)
{
    Vec3 camera_dir = vec3_normalize(vec3_sub(camera_pos, camera_target));
    Vec3 camera_right = vec3_normalize(vec3_cross(world_up_dir, camera_dir));
    Vec3 camera_up = vec3_normalize(vec3_cross(camera_dir, camera_right));

    Mat4 result = mat4_eye(1.0f);
    result.data[0] = camera_right.x;
    result.data[1] = camera_right.y;
    result.data[2] = camera_right.z;
    result.data[3] = -vec3_dot(camera_right, camera_pos);
    result.data[4] = camera_up.x;
    result.data[5] = camera_up.y;
    result.data[6] = camera_up.z;
    result.data[7] = -vec3_dot(camera_up, camera_pos);
    result.data[8] =  camera_dir.x;
    result.data[9] =  camera_dir.y;
    result.data[10] = camera_dir.z;
    result.data[11] = -vec3_dot(camera_dir, camera_pos);

    return result;
}

#endif // GM_IMPLEMENTATION
