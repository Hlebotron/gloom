#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

const int width = 800;
const int height = 450;

struct Unit {};
struct Line {
    int32_t A;
    int32_t B;
    int32_t C;
};
struct Line line_from_pts(
    int32_t point1_x, 
    int32_t point1_y, 
    int32_t point2_x, 
    int32_t point2_y
) {
    int32_t A = point1_y - point2_y;
    int32_t B = point2_x - point1_x;
    int32_t C = A * point1_x + B * point1_y;
    struct Line line = {
	A,
	B,
	C
    };

    return line; 
}


Vector2* intersection(
    float orient_deg, 
    int point_x, 
    int point_y, 
    int line_1x, 
    int line_1y, 
    int line_2x, 
    int line_2y
) {
    float inter_x;
    float inter_y;
    float orient_x = cos(orient_deg / 180 * M_PI);
    float orient_y = sin(orient_deg / 180 * M_PI);

    if (orient_x == 0) {
	inter_y = point_y; 
    }
    if (orient_y == 0) {
	inter_x = point_x; 
    }
    Vector2 vector = {
	inter_x,
	inter_y
    };
    Vector2* ptr = malloc(sizeof(Vector2));
    if (ptr == NULL) {
	printf("Allocation of Vector2 failed (Line 61)");
	exit(0);
    }
    return ptr;
}
int main() {
    InitWindow(width, height, "Gloom");
    while(!WindowShouldClose()) {
	ClearBackground(RAYWHITE);
	BeginDrawing();
	DrawLine(10, 20, 50, 100, BLACK);
	EndDrawing();
    }
    CloseWindow();
}
