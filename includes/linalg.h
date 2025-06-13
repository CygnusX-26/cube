#ifndef _LINALG
#define _LINALG

#include <stdlib.h>
#include <math.h>

typedef struct v3 {
    float x;
    float y;
    float z;
} v3;

typedef struct m3 {
    struct v3 x;
    struct v3 y;
    struct v3 z;
} m3;

typedef struct v4 {
    float x;
    float y;
    float z;
    float w;
} v4;

typedef struct m4 {
    struct v4 x;
    struct v4 y;
    struct v4 z;
    struct v4 w;
} m4;

float v4dot(v4*, v4*);
float v3dot(v3*, v3*);
v3* v3add(v3*, v3*);
v3* v3negate(v3*);
v3* v3cross(v3*, v3*);
m4* perspective(float, float, float, float);
void v3normalize(v3*);
m4* m4m4matrix_multiply(m4*, m4*);
v4* m4v4matrix_multiply(m4*, v4*);

#endif // _LINALG