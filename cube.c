#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>

#include "includes/cube.h"
#include "includes/linalg.h"

v3 camera_pos = {0, 0, -2.5};
v3 camera_target = {0, 0, 0};
v3 camera_up = {0, 1, 0};

v3 light_dir = {-1, 0, 100};

//char* gradient = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
char* gradient = "$Zt\\<,]*.-B%L";

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
float cube_verts_depth[VERTICIES] = {0};

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

void init() {
    initscr();
    noecho();
    curs_set(FALSE);
}

float edge_function(v2 a, v2 b, v2 c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

char char_from_intensity(float intensity) {
    return gradient[(int) intensity];
}


int main() {
    v3* neg_cam_pos = v3negate(&camera_pos);

    v3* camZ = v3add(&camera_target, neg_cam_pos);
    v3normalize(camZ);

    v3* camX = v3cross(&camera_up, camZ);
    v3normalize(camX);

    v3* camY = v3cross(camZ, camX);

    m4* viewMat = malloc(sizeof(m4));
    viewMat->x = (v4) {camX->x, camY->x, camZ->x, 0};
    viewMat->y = (v4) {camX->y, camY->y, camZ->y, 0};
    viewMat->z = (v4) {camX->z, camY->z, camZ->z, 0};
    viewMat->w = (v4) {-v3dot(camX, &camera_pos), -v3dot(camY, &camera_pos), -v3dot(camZ, &camera_pos), 1};

    free(camX); free(camY); free(camZ); free(neg_cam_pos);
    camX = NULL; camY = NULL; camZ = NULL; neg_cam_pos = NULL;

    m4* projMat = perspective(FOV, ASPECT, 0, 50);
    m4* modelMat = NULL;

    m4* vpMat = NULL;
    m4* vpmMat = NULL;
    v4* projVec = NULL;

    init();
    float rad = 0;

    while (1) {
        clear();
        // funny spinnning, could change in future
        modelMat = m4m4matrix_multiply(
            &(m4) {
            {cos(rad), 0, sin(rad), 0},
            {0, 1, 0, 0},
            {-sin(rad), 0, cos(rad), 0},
            {0, 0, 0, 1}
            },
            &(m4) {
            {cos(rad*ZSPEED), -sin(rad*ZSPEED), 0, 0},
            {sin(rad*ZSPEED), cos(rad*ZSPEED), 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
            }
        );

        // project from model to 2d screen coords
        for (int i = 0; i < VERTICIES; i++) {
            vpMat = m4m4matrix_multiply(projMat, viewMat);
            vpmMat = m4m4matrix_multiply(vpMat, modelMat);
            projVec = m4v4matrix_multiply(vpmMat, &(v4){cube_verts[i].x, cube_verts[i].y, cube_verts[i].z, 1});
            cube_verts_screen[i] = (v2) {SCALE * (projVec->x / projVec->z + XPOS), SCALE * (projVec->y / projVec->z + YPOS)};
            cube_verts_depth[i] = projVec->z;
            // Just put a dot on the screen where the points should be
            // mvprintw(SCALE * (projVec->y / projVec->z + YPOS), SCALE * (projVec->x / projVec->z + XPOS), ".");

            free(vpMat); free(vpmMat); free(projVec);
            vpMat = NULL; vpmMat = NULL; projVec = NULL;
        }
        
        for (int i = 0; i < TRIANGLES; i++) {
            v2 v0 = cube_verts_screen[(int)cube_triangles[i].x];
            v2 v1 = cube_verts_screen[(int)cube_triangles[i].y];
            v2 v2_ = cube_verts_screen[(int)cube_triangles[i].z];

            float z0 = cube_verts_depth[(int)cube_triangles[i].x];
            float z1 = cube_verts_depth[(int)cube_triangles[i].y];
            float z2 = cube_verts_depth[(int)cube_triangles[i].z];


            char c = char_from_intensity(i);

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
                            mvaddch(y, x, c);
                    }
                }
            }
        }


        free(modelMat);
        modelMat = NULL;
        refresh();
        napms(1.0 / FPS);
        if (rad > 6.28) {
            rad = 0;
        }
        else {
            rad += 0.0001 * RSPEED;
        }
    }

    free(viewMat);
    viewMat = NULL;
    free(projMat);
    projMat = NULL;
}