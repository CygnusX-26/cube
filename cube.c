#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>

#include "includes/cube.h"

v3 camera_pos = {0, 0, -2.5};
v3 camera_target = {0, 0, 0};
v3 camera_up = {0, 1, 0};

v3 cube_verts[] = {
    {1, -1, -1},
    {1, -1, 1},
    {-1, -1, -1},
    {-1, -1, 1},
    {1, 1, -1},
    {1, 1, 1},
    {-1, 1, -1},
    {-1, 1, 1}
};

// specifies index in cube_verts in which triangles are connected
v3 cube_triangles[] = {
    {0, 1, 5},
    {0, 4, 5},
    {2, 0, 4},
    {2, 7, 4},
    {5, 4, 6},
    {5, 7, 6},
    {2, 0, 1},
    {2, 3, 1},
    {2, 3, 6},
    {2, 7, 6},
    {3, 1, 5},
    {3, 6, 5}
};

// char edge_grad[3][3] = {
//     {'`', '\'', '"', '^', '?'},
//     {''}

// }

void init() {
    initscr();
    noecho();
    curs_set(FALSE);
}


int main() {
    v3* neg_cam_target = v3negate(&camera_target);

    v3* camZ = v3add(&camera_pos, neg_cam_target);
    v3normalize(camZ);

    v3* camX = v3cross(&camera_up, camZ);
    v3normalize(camX);

    v3* camY = v3cross(camZ, camX);

    m4* viewMat = malloc(sizeof(m4));
    viewMat->x = (v4) {camX->x, camY->x, camZ->x, 0};
    viewMat->y = (v4) {camX->y, camY->y, camZ->y, 0};
    viewMat->z = (v4) {camX->z, camY->z, camZ->z, 0};
    viewMat->w = (v4) {-v3dot(camX, &camera_pos), -v3dot(camY, &camera_pos), -v3dot(camZ, &camera_pos), 1};

    free(camX);
    free(camY);
    free(camZ);
    free(neg_cam_target);
    camX = NULL;
    camY = NULL;
    camZ = NULL;
    neg_cam_target = NULL;

    m4* projMat = perspective(FOV, ASPECT, 0, 50);

    m4* modelMat = NULL;

    m4* temp = NULL;
    m4* temp2 = NULL;
    v4* temp3 = NULL;

    init();
    float rad = 0;
    while (1) {
        clear();
        for (int i = 0; i < VERTICIES; i++) {
            modelMat = m4m4matrix_multiply(&(m4) {
                {cos(rad), 0, sin(rad), 0},
                {0, 1, 0, 0},
                {-sin(rad), 0, cos(rad), 0},
                {0, 0, 0, 1}
            }, &(m4) {
                {cos(rad), -sin(rad), 0, 0},
                {sin(rad), cos(rad), 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}
            });
            temp = m4m4matrix_multiply(projMat, viewMat);
            temp2 = m4m4matrix_multiply(temp, modelMat);
            temp3 = m4v4matrix_multiply(temp2, &(v4){cube_verts[i].x, cube_verts[i].y, cube_verts[i].z, 1});
            mvprintw(SCALE * (temp3->y / temp3->z + 2), SCALE * (temp3->x / temp3->z + 10), ".");
            free(temp);
            free(temp2);
            free(temp3);
            free(modelMat);
        }
        refresh();
        usleep(ROTSPEED);
        if (rad > 6.28) {
            rad = 0;
        }
        else {
            rad += 0.01;
        }
    }

    free(viewMat);
    viewMat = NULL;
    free(projMat);
    projMat = NULL;
    
}