/*
 * sst_matrix.c
 * By Steven Smith
 *
 * This file contains a number of helper functions for generating matrices one
 * would potentially want when doing 3D graphics. It's mostly based on the old
 * OpenGL fixed-functionality pipeline functions, which were dropped with no
 * replacement provided.
 */

#include <stdlib.h>
#include <math.h>
#include "sst.h"

/*
 * Important note: We use column-order matrices to stay consistent with OpenGL.
 * That means a 4x4 matrix is ordered in memory thusly:
 *  1  5  9 13     0  4  8 12
 *  2  6 10 14  o  1  5  9 13
 *  3  7 11 15  r  2  6 10 14
 *  4  8 12 16     3  7 11 15
 *
 *  1  4  7        0  3  6
 *  2  5  8        1  4  7
 *  3  6  9        2  5  8
 */

static GLfloat * sstGenMatrix4x4() {
    GLfloat *result;
    result = (GLfloat*)malloc(sizeof(GLfloat) * 4 * 4);
    return result;
}

static GLfloat * sstGenMatrix3x3() {
    GLfloat *result;
    result = (GLfloat*)malloc(sizeof(GLfloat) * 3 * 3);
    return result;
}

static GLfloat * sstGenVec3() {
    GLfloat *result;
    result = (GLfloat*)malloc(sizeof(GLfloat) * 3);
    return result;
}

/* Unused for now
static GLfloat * sstGenVec4() {
    GLfloat *result;
    result = (GLfloat*)malloc(sizeof(GLfloat) * 4);
    return result;
}
*/

GLfloat * sstIdentityMatrix4x4() {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    sstIdentityMatrix4x4_(mat);
    return mat;
}

void sstIdentityMatrix4x4_( GLfloat *mat ) {
    mat[ 0] = 1.0f;
    mat[ 1] = 0.0f;
    mat[ 2] = 0.0f;
    mat[ 3] = 0.0f;
    /**/
    mat[ 4] = 0.0f;
    mat[ 5] = 1.0f;
    mat[ 6] = 0.0f;
    mat[ 7] = 0.0f;
    /**/
    mat[ 8] = 0.0f;
    mat[ 9] = 0.0f;
    mat[10] = 1.0f;
    mat[11] = 0.0f;
    /**/
    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = 0.0f;
    mat[15] = 1.0f;
}

GLfloat * sstIdentityMatrix3x3() {
    GLfloat *mat;
    mat = sstGenMatrix3x3();
    sstIdentityMatrix3x3_(mat);
    return mat;
}

void sstIdentityMatrix3x3_( GLfloat *mat ) {
    mat[0] = 1.0f;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    /**/
    mat[3] = 0.0f;
    mat[4] = 1.0f;
    mat[5] = 0.0f;
    /**/
    mat[6] = 0.0f;
    mat[7] = 0.0f;
    mat[8] = 1.0f;
}

GLfloat * sstPerspectiveMatrix( GLfloat fovy, GLfloat aspect, GLfloat znear,
GLfloat zfar ) {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    sstPerspectiveMatrix_(fovy, aspect, znear, zfar, mat);
    return mat;
}

void sstPerspectiveMatrix_( GLfloat fovy, GLfloat aspect, GLfloat znear,
GLfloat zfar, GLfloat *mat ) {
    GLfloat f;
    f = 1 / tan((fovy/2) * (M_PI/180.0));
    mat[ 0] = f / aspect;
    mat[ 1] = 0.0f;
    mat[ 2] = 0.0f;
    mat[ 3] = 0.0f;
    /**/
    mat[ 4] = 0.0f;
    mat[ 5] = f;
    mat[ 6] = 0.0f;
    mat[ 7] = 0.0f;
    /**/
    mat[ 8] = 0.0f;
    mat[ 9] = 0.0f;
    mat[10] = (znear + zfar) / (znear - zfar);
    mat[11] = -1.0f;
    /**/
    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = (2 * znear * zfar) / (znear - zfar);
    mat[15] = 0.0f;
}

GLfloat * sstTranslateMatrix( GLfloat x, GLfloat y, GLfloat z ) {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    sstTranslateMatrix_(x, y, z, mat);
    return mat;
}

void sstTranslateMatrix_( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat ) {
    mat[ 0] = 1.0f;
    mat[ 1] = 0.0f;
    mat[ 2] = 0.0f;
    mat[ 3] = 0.0f;
    /**/
    mat[ 4] = 0.0f;
    mat[ 5] = 1.0f;
    mat[ 6] = 0.0f;
    mat[ 7] = 0.0f;
    /**/
    mat[ 8] = 0.0f;
    mat[ 9] = 0.0f;
    mat[10] = 1.0f;
    mat[11] = 0.0f;
    /**/
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
    mat[15] = 1.0f;
}

GLfloat * sstRotateMatrix( GLfloat theta, GLfloat x, GLfloat y, GLfloat z ) {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    sstRotateMatrix_(theta, x, y, z, mat);
    return mat;
}

void sstRotateMatrix_( GLfloat theta, GLfloat x, GLfloat y, GLfloat z,
GLfloat *mat ) {
    GLfloat c, s, c1;
    c = cos(theta);
    s = sin(theta);
    c1 = 1.0f - c;
    mat[ 0] = x * x * c1 + c;
    mat[ 1] = y * x * c1 + z * s;
    mat[ 2] = z * x * c1 - y * s;
    mat[ 3] = 0.0f;
    /**/
    mat[ 4] = x * y * c1 - z * s;
    mat[ 5] = y * y * c1 + c;
    mat[ 6] = z * y * c1 + x * s;
    mat[ 7] = 0.0f;
    /**/
    mat[ 8] = x * z * c1 + y * s;
    mat[ 9] = y * z * c1 - x * s;
    mat[10] = z * z * c1 + c;
    mat[11] = 0.0f;
    /**/
    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = 0.0f;
    mat[15] = 1.0f;
}

GLfloat * sstScaleMatrix( GLfloat x, GLfloat y, GLfloat z ) {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    sstScaleMatrix_(x, y, z, mat);
    return mat;
}

void sstScaleMatrix_( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat ) {
    mat[ 0] = x;
    mat[ 1] = 0.0f;
    mat[ 2] = 0.0f;
    mat[ 3] = 0.0f;
    /**/
    mat[ 4] = 0.0f;
    mat[ 5] = y;
    mat[ 6] = 0.0f;
    mat[ 7] = 0.0f;
    /**/
    mat[ 8] = 0.0f;
    mat[ 9] = 0.0f;
    mat[10] = z;
    mat[11] = 0.0f;
    /**/
    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = 0.0f;
    mat[15] = 1.0f;
}

GLfloat * sstCrossProduct3( GLfloat *v1, GLfloat *v2 ) {
    GLfloat *vec;
    vec = sstGenVec3();
    vec[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    vec[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    vec[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    return vec;
}

void sstCrossProduct3_( GLfloat *v1, GLfloat *v2, GLfloat *target ) {
    GLfloat temp[2];
    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    /* We can skip an extra assignment here because there's no more data 
     * dependencies. This also lessens required stack space. */
    target[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    target[0] = temp[0];
    target[1] = temp[1];
}

GLfloat sstDotProduct3( GLfloat *v1, GLfloat *v2 ) {
    return (v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]);
}

GLfloat * sstNormalize3( GLfloat *v ) {
    GLfloat *vec, length;
    vec = sstGenVec3();
    length = sqrt( (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]) );
    vec[0] = v[0] / length;
    vec[1] = v[1] / length;
    vec[2] = v[2] / length;
    return vec;
}

void sstNormalize3_( GLfloat *v ) {
    GLfloat length;
    length = sqrt( (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]) );
    v[0] = v[0] / length;
    v[1] = v[1] / length;
    v[2] = v[2] / length;
}

GLfloat * sstMatMult3( GLfloat *m1, GLfloat *m2 ) {
    GLfloat *mat;
    mat = sstGenMatrix3x3();
    mat[0] = (m1[0] * m2[0]) + (m1[3] * m2[1]) + (m1[6] * m2[2]);
    mat[1] = (m1[1] * m2[0]) + (m1[4] * m2[1]) + (m1[7] * m2[2]);
    mat[2] = (m1[2] * m2[0]) + (m1[5] * m2[1]) + (m1[8] * m2[2]);
    /**/
    mat[3] = (m1[0] * m2[3]) + (m1[3] * m2[4]) + (m1[6] * m2[5]);
    mat[4] = (m1[1] * m2[3]) + (m1[4] * m2[4]) + (m1[7] * m2[5]);
    mat[5] = (m1[2] * m2[3]) + (m1[5] * m2[4]) + (m1[8] * m2[5]);
    /**/
    mat[6] = (m1[0] * m2[6]) + (m1[3] * m2[7]) + (m1[6] * m2[8]);
    mat[7] = (m1[1] * m2[6]) + (m1[4] * m2[7]) + (m1[7] * m2[8]);
    mat[8] = (m1[2] * m2[6]) + (m1[5] * m2[7]) + (m1[8] * m2[8]);
    return mat;
}

void sstMatMult3_( GLfloat *m1, GLfloat *m2, GLfloat *target ) {
    GLfloat mat[9];
    int i;
    mat[0] = (m1[0] * m2[0]) + (m1[3] * m2[1]) + (m1[6] * m2[2]);
    mat[1] = (m1[1] * m2[0]) + (m1[4] * m2[1]) + (m1[7] * m2[2]);
    mat[2] = (m1[2] * m2[0]) + (m1[5] * m2[1]) + (m1[8] * m2[2]);
    /**/
    mat[3] = (m1[0] * m2[3]) + (m1[3] * m2[4]) + (m1[6] * m2[5]);
    mat[4] = (m1[1] * m2[3]) + (m1[4] * m2[4]) + (m1[7] * m2[5]);
    mat[5] = (m1[2] * m2[3]) + (m1[5] * m2[4]) + (m1[8] * m2[5]);
    /**/
    mat[6] = (m1[0] * m2[6]) + (m1[3] * m2[7]) + (m1[6] * m2[8]);
    mat[7] = (m1[1] * m2[6]) + (m1[4] * m2[7]) + (m1[7] * m2[8]);
    mat[8] = (m1[2] * m2[6]) + (m1[5] * m2[7]) + (m1[8] * m2[8]);
    for( i = 0; i < 9; i++ ) {
        target[i] = mat[i];
    }
}

GLfloat * sstMatMult4( GLfloat *m1, GLfloat *m2 ) {
    GLfloat *mat;
    mat = sstGenMatrix4x4();
    mat[ 0] = (m1[ 0] * m2[ 0]) + (m1[ 4] * m2[ 1]) + (m1[ 8] * m2[ 2]) + (m1[12] * m2[ 3]);
    mat[ 1] = (m1[ 1] * m2[ 0]) + (m1[ 5] * m2[ 1]) + (m1[ 9] * m2[ 2]) + (m1[13] * m2[ 3]);
    mat[ 2] = (m1[ 2] * m2[ 0]) + (m1[ 6] * m2[ 1]) + (m1[10] * m2[ 2]) + (m1[14] * m2[ 3]);
    mat[ 3] = (m1[ 3] * m2[ 0]) + (m1[ 7] * m2[ 1]) + (m1[11] * m2[ 2]) + (m1[15] * m2[ 3]);
    /**/
    mat[ 4] = (m1[ 0] * m2[ 4]) + (m1[ 4] * m2[ 5]) + (m1[ 8] * m2[ 6]) + (m1[12] * m2[ 7]);
    mat[ 5] = (m1[ 1] * m2[ 4]) + (m1[ 5] * m2[ 5]) + (m1[ 9] * m2[ 6]) + (m1[13] * m2[ 7]);
    mat[ 6] = (m1[ 2] * m2[ 4]) + (m1[ 6] * m2[ 5]) + (m1[10] * m2[ 6]) + (m1[14] * m2[ 7]);
    mat[ 7] = (m1[ 3] * m2[ 4]) + (m1[ 7] * m2[ 5]) + (m1[11] * m2[ 6]) + (m1[15] * m2[ 7]);
    /**/
    mat[ 8] = (m1[ 0] * m2[ 8]) + (m1[ 4] * m2[ 9]) + (m1[ 8] * m2[10]) + (m1[12] * m2[11]);
    mat[ 9] = (m1[ 1] * m2[ 8]) + (m1[ 5] * m2[ 9]) + (m1[ 9] * m2[10]) + (m1[13] * m2[11]);
    mat[10] = (m1[ 2] * m2[ 8]) + (m1[ 6] * m2[ 9]) + (m1[10] * m2[10]) + (m1[14] * m2[11]);
    mat[11] = (m1[ 3] * m2[ 8]) + (m1[ 7] * m2[ 9]) + (m1[11] * m2[10]) + (m1[15] * m2[11]);
    /**/
    mat[12] = (m1[ 0] * m2[12]) + (m1[ 4] * m2[13]) + (m1[ 8] * m2[14]) + (m1[12] * m2[15]);
    mat[13] = (m1[ 1] * m2[12]) + (m1[ 5] * m2[13]) + (m1[ 9] * m2[14]) + (m1[13] * m2[15]);
    mat[14] = (m1[ 2] * m2[12]) + (m1[ 6] * m2[13]) + (m1[10] * m2[14]) + (m1[14] * m2[15]);
    mat[15] = (m1[ 3] * m2[12]) + (m1[ 7] * m2[13]) + (m1[11] * m2[14]) + (m1[15] * m2[15]);
    return mat;
}

void sstMatMult4_( GLfloat *m1, GLfloat *m2, GLfloat *target ) {
    GLfloat mat[16];
    int i;
    mat[ 0] = (m1[ 0] * m2[ 0]) + (m1[ 4] * m2[ 1]) + (m1[ 8] * m2[ 2]) + (m1[12] * m2[ 3]);
    mat[ 1] = (m1[ 1] * m2[ 0]) + (m1[ 5] * m2[ 1]) + (m1[ 9] * m2[ 2]) + (m1[13] * m2[ 3]);
    mat[ 2] = (m1[ 2] * m2[ 0]) + (m1[ 6] * m2[ 1]) + (m1[10] * m2[ 2]) + (m1[14] * m2[ 3]);
    mat[ 3] = (m1[ 3] * m2[ 0]) + (m1[ 7] * m2[ 1]) + (m1[11] * m2[ 2]) + (m1[15] * m2[ 3]);
    /**/
    mat[ 4] = (m1[ 0] * m2[ 4]) + (m1[ 4] * m2[ 5]) + (m1[ 8] * m2[ 6]) + (m1[12] * m2[ 7]);
    mat[ 5] = (m1[ 1] * m2[ 4]) + (m1[ 5] * m2[ 5]) + (m1[ 9] * m2[ 6]) + (m1[13] * m2[ 7]);
    mat[ 6] = (m1[ 2] * m2[ 4]) + (m1[ 6] * m2[ 5]) + (m1[10] * m2[ 6]) + (m1[14] * m2[ 7]);
    mat[ 7] = (m1[ 3] * m2[ 4]) + (m1[ 7] * m2[ 5]) + (m1[11] * m2[ 6]) + (m1[15] * m2[ 7]);
    /**/
    mat[ 8] = (m1[ 0] * m2[ 8]) + (m1[ 4] * m2[ 9]) + (m1[ 8] * m2[10]) + (m1[12] * m2[11]);
    mat[ 9] = (m1[ 1] * m2[ 8]) + (m1[ 5] * m2[ 9]) + (m1[ 9] * m2[10]) + (m1[13] * m2[11]);
    mat[10] = (m1[ 2] * m2[ 8]) + (m1[ 6] * m2[ 9]) + (m1[10] * m2[10]) + (m1[14] * m2[11]);
    mat[11] = (m1[ 3] * m2[ 8]) + (m1[ 7] * m2[ 9]) + (m1[11] * m2[10]) + (m1[15] * m2[11]);
    /**/
    mat[12] = (m1[ 0] * m2[12]) + (m1[ 4] * m2[13]) + (m1[ 8] * m2[14]) + (m1[12] * m2[15]);
    mat[13] = (m1[ 1] * m2[12]) + (m1[ 5] * m2[13]) + (m1[ 9] * m2[14]) + (m1[13] * m2[15]);
    mat[14] = (m1[ 2] * m2[12]) + (m1[ 6] * m2[13]) + (m1[10] * m2[14]) + (m1[14] * m2[15]);
    mat[15] = (m1[ 3] * m2[12]) + (m1[ 7] * m2[13]) + (m1[11] * m2[14]) + (m1[15] * m2[15]);
    for( i = 0; i < 16; i++ ) {
        target[i] = mat[i];
    }
}
