#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>

#include "includes/cube.h"
#include "includes/linalg.h"

v3 camera_pos = {0, 0, -2.5};
v3 camera_target = {0, 0, 0};
v3 camera_up = {2, 0, 2};

v3 light_dir = {1, 1, 1};

char* gradient = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";

v3 cube_verts[VERTICIES] = {
    {1, -1, -1},
    {1, -1, 1},
    {-1, -1, -1},
    {-1, -1, 1},
    {1, 1, -1},
    {1, 1, 1},
    {-1, 1, -1},
    {-1, 1, 1}
};

v2 cube_verts_screen[VERTICIES] = {0};

// specifies index in cube_verts in which triangles are connected
v3 cube_triangles[TRIANGLES] = {
    {0, 2, 6},
    {0, 6, 4},
    {1, 5, 7},
    {1, 7, 3},
    {2, 3, 7},
    {2, 7, 6},
    {0, 4, 5},
    {0, 5, 1},
    {4, 6, 7},
    {4, 7, 5},
    {0, 1, 3},
    {0, 3, 2},
};

v3 vertex_normals[VERTICIES] = {0};
v3 transformed_normals[VERTICIES] = {0};

void init() {
    initscr();
    noecho();
    curs_set(FALSE);
}

float edge_function(v2 a, v2 b, v2 c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

char char_from_intensity(float intensity) {
    return gradient[(int) (intensity * (strlen(gradient) - 1))];
}


int main() {

    //setup normals for each vertex
    for (int i = 0; i < TRIANGLES; i++) {
        int a = cube_triangles[i].x;
        int b = cube_triangles[i].y;
        int c = cube_triangles[i].z;

        v3* p0 = &cube_verts[a];
        v3* p1 = &cube_verts[b];
        v3* p2 = &cube_verts[c];

        v3* e1 = v3sub(p1, p0);
        v3* e2 = v3sub(p2, p0);
        v3* normal = v3cross(e1, e2);
        v3normalize(normal);

        v3* norm = v3add(&vertex_normals[a], normal);
        vertex_normals[a] = *norm;
        free(norm);
        norm = v3add(&vertex_normals[b], normal);
        vertex_normals[b] = *norm;
        free(norm);
        norm = v3add(&vertex_normals[c], normal);
        vertex_normals[c] = *norm;

        free(e1); free(e2); free(normal); free(norm);
        e1 = NULL; e2 = NULL; normal = NULL; norm = NULL;
    }

    v3* camZ = v3sub(&camera_target, &camera_pos);
    v3normalize(camZ);

    v3* camX = v3cross(&camera_up, camZ);
    v3normalize(camX);

    v3* camY = v3cross(camZ, camX);

    m4* viewMat = malloc(sizeof(m4));
    viewMat->x = (v4) {camX->x, camY->x, camZ->x, 0};
    viewMat->y = (v4) {camX->y, camY->y, camZ->y, 0};
    viewMat->z = (v4) {camX->z, camY->z, camZ->z, 0};
    viewMat->w = (v4) {-v3dot(camX, &camera_pos), -v3dot(camY, &camera_pos), -v3dot(camZ, &camera_pos), 1};

    free(camX); free(camY); free(camZ);
    camX = NULL; camY = NULL; camZ = NULL;

    m4* projMat = perspective(FOV, ASPECT, 0, 50);
    m4* modelMat = NULL;

    m4* vpMat = NULL;
    m4* vpmMat = NULL;
    v4* projVec = NULL;
    v4* normVec = NULL;

    init();
    float rad = 0;

    while (1) {
        clear();
        // funny spinnning, could change in future
        modelMat = m4m4matrix_multiply(
            &(m4) {
            {cos(rad*YSPEED), 0, sin(rad*YSPEED), 0},
            {0, 1, 0, 0},
            {-sin(rad*YSPEED), 0, cos(rad*YSPEED), 0},
            {0, 0, 0, 1}
            },
            &(m4) {
            {cos(rad*ZSPEED), -sin(rad*ZSPEED), 0, 0},
            {sin(rad*ZSPEED), cos(rad*ZSPEED), 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
            }
        );

        for (int i = 0; i < VERTICIES; i++) {
            vpMat = m4m4matrix_multiply(projMat, viewMat);
            vpmMat = m4m4matrix_multiply(vpMat, modelMat);
            projVec = m4v4matrix_multiply(vpmMat, &(v4){cube_verts[i].x, cube_verts[i].y, cube_verts[i].z, 1});
            cube_verts_screen[i] = (v2) {SCALE * (projVec->x / projVec->z + XPOS), SCALE * (projVec->y / projVec->z + YPOS)};
            v4 n = { vertex_normals[i].x, vertex_normals[i].y, vertex_normals[i].z, 0 };
            normVec = m4v4matrix_multiply(modelMat, &n);
            transformed_normals[i] = (v3) {normVec->x, normVec->y, normVec->z};
            v3normalize(&transformed_normals[i]);

            //mvprintw(SCALE * (projVec->y / projVec->z + YPOS), SCALE * (projVec->x / projVec->z + XPOS), ".");

            free(vpMat); free(vpmMat); free(projVec); free(normVec);
            vpMat = NULL; vpmMat = NULL; projVec = NULL; normVec = NULL;
        }
        
        for (int i = 0; i < TRIANGLES; i++) {
            int a = (int)cube_triangles[i].x;
            int b = (int)cube_triangles[i].y;
            int c = (int)cube_triangles[i].z;

            v2 v0 = cube_verts_screen[a];
            v2 v1 = cube_verts_screen[b];
            v2 v2_ = cube_verts_screen[c];

            v3* c0 = &cube_verts[a];
            v3* c1 = &cube_verts[b];
            v3* c2 = &cube_verts[c];

            v3 n0 = transformed_normals[a];
            v3 n1 = transformed_normals[b];
            v3 n2 = transformed_normals[c];

            int minX = (int)floorf(fminf(fminf(v0.x, v1.x), v2_.x));
            int maxX = (int)ceilf(fmaxf(fmaxf(v0.x, v1.x), v2_.x));
            int minY = (int)floorf(fminf(fminf(v0.y, v1.y), v2_.y));
            int maxY = (int)ceilf(fmaxf(fmaxf(v0.y, v1.y), v2_.y));
            for (int y = minY; y <= maxY; y++) {
                for (int x = minX; x <= maxX; x++) {
                    v2 point = {x + 0.5f, y + 0.5f};
                    float area = edge_function(v0, v1, v2_);

                    float w0 = edge_function(v1,v2_, point);
                    float w1 = edge_function(v2_, v0, point);
                    float w2 = edge_function(v0, v1, point);
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                        float sum = w0 + w1 + w2;
                        if (sum == 0) continue;
                        v3 interp_normal = {
                            (n0.x * w0 + n1.x * w1 + n2.x * w2) / sum,
                            (n0.y * w0 + n1.y * w1 + n2.y * w2) / sum,
                            (n0.z * w0 + n1.z * w1 + n2.z * w2) / sum,
                        };
                        v3normalize(&interp_normal);

                        v3* light = v3negate(&light_dir);
                        v3normalize(light);
                        float intensity = fmaxf(0.0f, v3dot(&interp_normal, light));

                        free(light);
                        light = NULL;

                        char chr = char_from_intensity(intensity);
                        mvaddch(y, x, chr);
                    }
                }
            }
        }


        free(modelMat);
        modelMat = NULL;
        refresh();
        napms(1.0 / FPS);
        // if (rad >= 6.2831852) {
        //     rad = 0;
        // }
        // else {
        rad += 0.0001 * RSPEED;
        // }
    }

    free(viewMat);
    viewMat = NULL;
    free(projMat);
    projMat = NULL;
}