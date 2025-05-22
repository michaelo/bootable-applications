#ifndef MATRIX_H
#define MATRIX_H

#include "math.h"
#include "memory.h"

typedef float Matrix4[4][4];

void mul(Matrix4 src1, Matrix4 src2, Matrix4 dest)
{
    dest[0][0] = src1[0][0] * src2[0][0] + src1[0][1] * src2[1][0] + src1[0][2] * src2[2][0] + src1[0][3] * src2[3][0]; 
    dest[0][1] = src1[0][0] * src2[0][1] + src1[0][1] * src2[1][1] + src1[0][2] * src2[2][1] + src1[0][3] * src2[3][1]; 
    dest[0][2] = src1[0][0] * src2[0][2] + src1[0][1] * src2[1][2] + src1[0][2] * src2[2][2] + src1[0][3] * src2[3][2]; 
    dest[0][3] = src1[0][0] * src2[0][3] + src1[0][1] * src2[1][3] + src1[0][2] * src2[2][3] + src1[0][3] * src2[3][3]; 
    dest[1][0] = src1[1][0] * src2[0][0] + src1[1][1] * src2[1][0] + src1[1][2] * src2[2][0] + src1[1][3] * src2[3][0]; 
    dest[1][1] = src1[1][0] * src2[0][1] + src1[1][1] * src2[1][1] + src1[1][2] * src2[2][1] + src1[1][3] * src2[3][1]; 
    dest[1][2] = src1[1][0] * src2[0][2] + src1[1][1] * src2[1][2] + src1[1][2] * src2[2][2] + src1[1][3] * src2[3][2]; 
    dest[1][3] = src1[1][0] * src2[0][3] + src1[1][1] * src2[1][3] + src1[1][2] * src2[2][3] + src1[1][3] * src2[3][3]; 
    dest[2][0] = src1[2][0] * src2[0][0] + src1[2][1] * src2[1][0] + src1[2][2] * src2[2][0] + src1[2][3] * src2[3][0]; 
    dest[2][1] = src1[2][0] * src2[0][1] + src1[2][1] * src2[1][1] + src1[2][2] * src2[2][1] + src1[2][3] * src2[3][1]; 
    dest[2][2] = src1[2][0] * src2[0][2] + src1[2][1] * src2[1][2] + src1[2][2] * src2[2][2] + src1[2][3] * src2[3][2]; 
    dest[2][3] = src1[2][0] * src2[0][3] + src1[2][1] * src2[1][3] + src1[2][2] * src2[2][3] + src1[2][3] * src2[3][3]; 
    dest[3][0] = src1[3][0] * src2[0][0] + src1[3][1] * src2[1][0] + src1[3][2] * src2[2][0] + src1[3][3] * src2[3][0]; 
    dest[3][1] = src1[3][0] * src2[0][1] + src1[3][1] * src2[1][1] + src1[3][2] * src2[2][1] + src1[3][3] * src2[3][1]; 
    dest[3][2] = src1[3][0] * src2[0][2] + src1[3][1] * src2[1][2] + src1[3][2] * src2[2][2] + src1[3][3] * src2[3][2]; 
    dest[3][3] = src1[3][0] * src2[0][3] + src1[3][1] * src2[1][3] + src1[3][2] * src2[2][3] + src1[3][3] * src2[3][3]; 
}

void make_identity(Matrix4 matrix)
{
    matrix[0][0] = 1;
    matrix[0][1] = 0;
    matrix[0][2] = 0;
    matrix[0][3] = 0;

    matrix[1][0] = 0;
    matrix[1][1] = 1;
    matrix[1][2] = 0;
    matrix[1][3] = 0;

    matrix[2][0] = 0;
    matrix[2][1] = 0;
    matrix[2][2] = 1;
    matrix[2][3] = 0;

    matrix[3][0] = 0;
    matrix[3][1] = 0;
    matrix[3][2] = 0;
    matrix[3][3] = 1;
}

void make_rotateZ(Matrix4 matrix, float θ) //yaw
{
    make_identity(matrix);
    matrix[0][0] = cos(θ);
    matrix[0][1] = -sin(θ);

    matrix[1][0] = sin(θ);
    matrix[1][1] = cos(θ);
}

void rotateZ(Matrix4 matrix, float θ)
{
    Matrix4 tmp;
    Matrix4 result;
    make_rotateZ(tmp, θ);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void make_rotateY(Matrix4 matrix, float θ) //pitch
{
    make_identity(matrix);
    matrix[0][0] = cos(θ);
    matrix[0][2] = sin(θ);

    matrix[2][0] = -sin(θ);
    matrix[2][2] = cos(θ);
}


void rotateY(Matrix4 matrix, float θ)
{
    Matrix4 tmp;
    Matrix4 result;
    make_rotateY(tmp, θ);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void make_rotateX(Matrix4 matrix, float θ) //roll
{
    make_identity(matrix);
    matrix[1][1] = cos(θ);
    matrix[1][2] = -sin(θ);

    matrix[2][1] = sin(θ);
    matrix[2][2] = cos(θ);
}

void rotateX(Matrix4 matrix, float θ)
{
    Matrix4 tmp;
    Matrix4 result;
    make_rotateX(tmp, θ);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void make_translate(Matrix4 matrix, float tx, float ty, float tz)
{
    make_identity(matrix);
    matrix[3][0] = tx;
    matrix[3][1] = ty;
    matrix[3][2] = tz;
}

void translate(Matrix4 matrix, float tx, float ty, float tz)
{
    Matrix4 tmp;
    Matrix4 result;
    make_translate(tmp, tx, ty, tz);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void make_scale(Matrix4 matrix, float sx, float sy, float sz)
{
    make_identity(matrix);
    matrix[0][0] = sx;
    matrix[1][1] = sy;
    matrix[2][2] = sz;
}

void scale(Matrix4 matrix, float sx, float sy, float sz)
{
    Matrix4 tmp;
    Matrix4 result;
    make_scale(tmp, sx, sy, sz);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void make_perspective_divide(Matrix4 matrix)
{
    make_identity(matrix);
    matrix[2][2] = -1;
    matrix[2][3] = -1;
}

void perspective_divide(Matrix4 matrix)
{
    Matrix4 tmp;
    Matrix4 result;
    make_perspective_divide(tmp);
    mul(matrix, tmp, result);
    memcpy(matrix, result, 16 * sizeof(float));
}

void transformVec4(Matrix4 m, float in[4], float out[4])
{
    float x = in[0], y = in[1], z = in[2], w = in[3];
    out[0] = m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0] * w;
    out[1] = m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1] * w;
    out[2] = m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2] * w;
    out[3] = m[0][3] * x + m[1][3] * y + m[2][3] * z + m[3][3] * w;
}

void toCartesian(float vec[4], float out[3])
{
    // Convert to Cartesian coordinates
    if (vec[3] != 0.0f) {
        out[0] = vec[0] / vec[3];
        out[1] = vec[1] / vec[3];
        out[2] = vec[2] / vec[3];
    } else {
        // Handle the case where w is 0 (leave unchanged)
        out[0] = vec[0];
        out[1] = vec[1];
        out[2] = vec[2];
    }
}

void transformVec3(Matrix4 m, float in[3], float out[3])
{
    float extended[] = {in[0], in[1], in[2], 1.0f};
    float transformed[4];
    transformVec4(m, extended, transformed);
    toCartesian(transformed, out);
}

#endif