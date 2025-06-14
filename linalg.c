#include <stdlib.h>

#include "includes/linalg.h"

float v4dot(v4* x, v4* y) {
    return x->x * y->x + x->y * y->y + x->z * y->z + x->w * y->w;
}

float v3dot(v3* x, v3* y) {
    return x->x * y->x + x->y * y->y + x->z * y->z;
}

// resultant vector needs to be freed
v3* v3add(v3*a, v3* b) {
    v3* ptr = malloc(sizeof(v3));
    ptr->x = a->x + b->x;
    ptr->y = a->y + b->y;
    ptr->z = a->z + b->z;
    return ptr;
}

// resultant vector needs to be freed
v3* v3negate(v3* a) {
    v3* ptr = malloc(sizeof(v3));
    ptr->x = -a->x;
    ptr->y = -a->y;
    ptr->z = -a->z;
    return ptr;
}

// resultant vector needs to be freed
v3* v3cross(v3* x, v3* y) {
    v3* ptr = malloc(sizeof(v3));
    ptr->x = x->y * y->z - x->z * y->y;
    ptr->y = x->z * y->x - x->x * y->z;
    ptr->z = x->x * y->y - x->y * y->x;
    return ptr;
}

// resultant vector needs to be freed
m4* perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f);
    m4* result = malloc(sizeof(m4));
    
    result->x = (v4){f / aspect, 0, 0, 0};
    result->y = (v4){0, f, 0, 0};
    result->z = (v4){0, 0, (far + near) / (near - far), -1};
    result->w = (v4){0, 0, (2 * far * near) / (near - far), 0};
    
    return result;
}


void v3normalize(v3* a) {
    float hyp = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
    a->x /= hyp;
    a->y /= hyp;
    a->z /= hyp;
}

float m4_get(m4* mat, int row, int col) {
    v4* cols[] = { &mat->x, &mat->y, &mat->z, &mat->w };
    float* col_ptr = (float*)cols[col];
    return col_ptr[row];
}

void m4_set(m4* mat, int row, int col, float value) {
    v4* cols[] = { &mat->x, &mat->y, &mat->z, &mat->w };
    float* col_ptr = (float*)cols[col];
    col_ptr[row] = value;
}

m4* m4m4matrix_multiply(m4* a, m4* b) {
    m4* res = malloc(sizeof(m4));
    if (!res) return NULL;

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += m4_get(a, row, k) * m4_get(b, k, col);
            }
            m4_set(res, row, col, sum);
        }
    }

    return res;
}

v4* m4v4matrix_multiply(m4* a, v4* b) {
    v4* res = malloc(sizeof(v4));
    if (!res) return NULL;

    float* col0 = (float*)&a->x;
    float* col1 = (float*)&a->y;
    float* col2 = (float*)&a->z;
    float* col3 = (float*)&a->w;

    res->x = col0[0] * b->x + col1[0] * b->y + col2[0] * b->z + col3[0] * b->w;
    res->y = col0[1] * b->x + col1[1] * b->y + col2[1] * b->z + col3[1] * b->w;
    res->z = col0[2] * b->x + col1[2] * b->y + col2[2] * b->z + col3[2] * b->w;
    res->w = col0[3] * b->x + col1[3] * b->y + col2[3] * b->z + col3[3] * b->w;

    return res;
}