#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

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
    Vector2* point1,
    Vector2* point2
) {
    int A = point1->y - point2->y;
    int B = point2->x - point1->x;
    int C = A * point1->x + B * point1->y;
    Line line = {
	A,
	B,
	C
    };

    return line; 
}
Line line_from_angle(
    Vector2* point,
    float angle_deg //Positive is clockwise
) {
    float math_angle = -angle_deg;
    float A = sin(angle_deg / 180 * M_PI);
    float B = cos(angle_deg / 180 * M_PI);
    float C = A * point->x + B * point->y;
    Line line = {
	A,
	B,
	C
    };
    return line;
};




void intersect_lines(
    Line* line1,
    Line* line2,
    Vector2* intersect
) {
    float denom = (line1->A - (line1->B * line2->A / line2->B));
    if (denom == 0) { 
	intersect = NULL;
	return;
    }
    float x = (line1->C - (line1->B * line2->C / line2->B)) / denom;
    float y = (line2->C - (line2->A - x)) / line2->B;
    *intersect = (Vector2){ x, y };
}
bool intersect_segment(
    Segment* segment,
    Line* line,
    Vector2* intersect
) {
    Line seg_line = line_from_points(&segment->start, &segment->end);
    intersect_lines(&seg_line, line, intersect);
    if (
	intersect == NULL ||
	intersect->x < segment->start.x ||
	intersect->x > segment->end.x
    ) {
	intersect = NULL;
	return false;
    };
    return true;
}
float hor_distance(
    Vector2* point1,
    Vector2* point2
) { 
    return sqrt(pow((point2->x - point1->x), 2) + pow((point2->y - point1->y), 2));
};
void displayed_height_deg (
    Vector2* point1,
    Vector2* point2,
    float obj_height,
    float* bottom,
    float* top
) {
    float dist = hor_distance(point1, point2);
    if (dist == 0) {
	*top = VERTICAL_FOV / 2;	
	*bottom = VERTICAL_FOV / 2;	
	return;
    }
    printf("Distance: %f\n", dist);
    printf("Height: %f\n", (obj_height - PLAYER_HEIGHT));
    float val1 = atan((obj_height - PLAYER_HEIGHT) / dist) / M_PI * 180;
    float val2 = atan(PLAYER_HEIGHT / dist) / M_PI * 180;
    printf("%f %f\n", val1, val2);
    *top = atan((obj_height - PLAYER_HEIGHT) / dist) / M_PI * 180;
    *bottom = atan(PLAYER_HEIGHT / dist) / M_PI * 180;
};

int main() {
    Segment wall = { 
	(Vector2){ 0, 1 }, 
	(Vector2){ 2, 2 } 
    };
    Player player = {
	(Vector2){ 0, 0 },
	125
    };
    InitWindow(WIDTH, HEIGHT, "Gloom");
    while(!WindowShouldClose()) {
	printf("");
	Line orient = line_from_angle(&player.pos, player.orient);
	Vector2 intersect;
	bool res = intersect_segment(&wall, &orient, &intersect);
	if (res) {
	    float bottom, top;
	    displayed_height_deg(&player.pos, &intersect, 3, &bottom, &top);
	};
	ClearBackground(RAYWHITE);
	BeginDrawing();
	EndDrawing();
	//free(intersect);
	usleep(100000);
	//printf("pog");
    }
    CloseWindow();
}
