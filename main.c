#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 450;
const float HORIZONTAL_FOV = 90.0;
const float VERTICAL_FOV = 45.0;
const float PLAYER_HEIGHT = 1.91;

typedef struct {
    Vector2 pos;
    float orient;
} Player;
typedef struct {
    float A;
    float B;
    float C;
} Line;
typedef struct {
    Vector2 start;
    Vector2 end;
    float height;
} Wall;
typedef struct {
    Vector2 start;
    Vector2 end;
} Segment;
Segment segment_from_points(Vector2 start, Vector2 end) {
    Segment segment = { start, end };
    return segment;
}
Line line_from_points(
    Vector2 point1,
    Vector2 point2
) {
    int A = point1.y - point2.y;
    int B = point2.x - point1.x;
    int C = A * point1.x + B * point1.y;
    Line line = {
	A,
	B,
	C
    };

    return line; 
}
Line line_from_angle(
    float angle_deg, //Positive is clockwise
    float p_x, 
    float p_y
) {
    float math_angle = -angle_deg;
    float A = sin(angle_deg / 180 * M_PI);
    float B = cos(angle_deg / 180 * M_PI);
    float C = A * p_x + B * p_y;
    Line line = {
	A,
	B,
	C
    };
    return line;
};




Vector2* intersect_lines(
    Line line1,
    Line line2
) {
    float denom = (line1.A - (line1.B * line2.A / line2.B));
    if (denom == 0) return NULL;
    float x = (line1.C - (line1.B * line2.C / line2.B)) / denom;
    float y = (line2.C - (line2.A - x)) / line2.B;
    Vector2* ptr = malloc(sizeof(Vector2));
    if (ptr == NULL) {
	printf("Allocation of Vector2 failed");
	exit(0);
    }
    Vector2 vector = {
	x,
	y
    };
    *ptr = vector;
    return ptr;
}
Vector2* intersect_segment(
    Segment segment,
    Line line
) {
    Line seg_line = line_from_points(segment.start, segment.end);
    Vector2* intersect = intersect_lines(seg_line, line);
    if (
	intersect == NULL ||
	intersect->x < segment.start.x ||
	intersect->x > segment.end.x
    ) return NULL; 
    else return intersect; 
}
float distance(
    Vector2 point1,
    Vector2 point2
) { 
    return sqrt(pow((point2.x - point1.x), 2) + pow((point2.y - point1.y), 2));
};
float displayed_height_deg(
    Vector2 point1,
    Vector2 point2
) {
    float dist = distance(point1, point2);
};

int main() {
    Segment wall = { 
	(Vector2){ 0, 1 }, 
	(Vector2){ 2, 2 } 
    };
    Line orient = line_from_angle(125, 0, 0);
    InitWindow(WIDTH, HEIGHT, "Gloom");
    while(!WindowShouldClose()) {
	Vector2* intersect = intersect_segment(wall, orient);
	ClearBackground(RAYWHITE);
	BeginDrawing();
	EndDrawing();
	free(intersect);
    }
    CloseWindow();
}
