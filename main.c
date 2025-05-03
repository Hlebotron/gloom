#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

const uint32_t SCREEN_WIDTH = 1080;
const uint32_t SCREEN_HEIGHT = 720;
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

const Segment WALLS[] = {
    (Segment){
	(Vector2){ 2, 3 }, 
	(Vector2){ 2, -3 } 
    }
};
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
    float angle_deg //Positive direction is clockwise, 0 degrees is North
) {
    float math_angle = -angle_deg - 90;
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
bool point_is_on_line(Vector2* point, Line* line) {
    return (line->A * point->x + line->B * point->y == line->C);
}
Line line_normal(Line* line, Vector2* point) {
    return (Line){
	line->B,
	-line->A,
	(line->B * point->x - line->A * point->y)
    };
}




bool intersection_lines(
    Line* line1,
    Line* line2,
    Vector2* intersection
) {
    float denom = (line1->A - (line1->B * line2->A / line2->B));
    if (denom == 0) { 
	intersection = NULL;
	return false;
    }
    float x = (line1->C - (line1->B * line2->C / line2->B)) / denom;
    float y = (line2->C - (line2->A - x)) / line2->B;
    *intersection = (Vector2){ x, y };
    return true;
}
bool intersection_segment(
    Segment* segment,
    Line* line,
    Vector2* intersection
) {
    Line seg_line = line_from_points(&segment->start, &segment->end);
    intersection_lines(&seg_line, line, intersection);
    if (
	intersection == NULL ||
	intersection->x < segment->start.x ||
	intersection->x > segment->end.x
    ) {
	intersection = NULL;
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
void displayed_angles (
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
    //printf("Distance: %f\n", dist);
    //printf("Height: %f\n", (obj_height - PLAYER_HEIGHT));
    float val1 = atan((obj_height - PLAYER_HEIGHT) / dist) / M_PI * 180;
    float val2 = atan(PLAYER_HEIGHT / dist) / M_PI * 180;
    //printf("atan: %f %f\n", val1, val2);
    *top = atan((obj_height - PLAYER_HEIGHT) / dist) / M_PI * 180;
    *bottom = atan(PLAYER_HEIGHT / dist) / M_PI * 180;
};
//TODO: See if this function needs to be corrected mathematically
void draw_segment(const Segment* segment, Player* player, float height) {
    Vector2 vertices[5];
    Vector2 ends[2] = { segment->start, segment->end };
    int hor_positions[2];
    int vert_positions[4];
    Line player_dir = line_from_angle(&player->pos, player->orient);
    printf("Player angle: %f\n", player->orient);
    for (int i = 0; i < 2; i++) {
	Vector2 end = ends[i];
	float dist = hor_distance(&player->pos, &end);
	Line dir_norm = line_normal(&player_dir, &end);
	Vector2 intersection;
	intersection_lines(&player_dir, &dir_norm, &intersection);
	float dir_deviance = hor_distance(&intersection, &end);
	if (dir_deviance / dist > 1) {
	    //printf("Player angle: %f, Ratio %d: %f\n", player->orient, i, dir_deviance / dist);
	    printf("Segment not visible; not rendering segment\n\n");
	    return;
	}
	float angle = asin(dir_deviance / dist) / M_PI * 180;
	int pos = round(angle * 2 * SCREEN_WIDTH / HORIZONTAL_FOV);
	hor_positions[i] = pos;
	//How is the direction deviance larger than the distance?
	//  Especially at 180 degrees
	//Going through this rigamarole may not be necessary because this function is meant to be called with the finished segment (after determining the total length with other sections covering it)
	float top, bottom;
	displayed_angles(&player->pos, &intersection, height, &bottom, &top);
	//Get the actual screen positions from the angles
	float top_ratio = top * 2 / VERTICAL_FOV;
	float bottom_ratio = bottom * 2 / VERTICAL_FOV;
	int top_pos = round((SCREEN_HEIGHT / 2) * (1 - top_ratio));
	int bottom_pos = round((SCREEN_HEIGHT / 2) * (1 + top_ratio));
	vert_positions[i] = bottom_pos;
	vert_positions[i + 1] = bottom_pos;
    }
    Vector2 points[5] = {
	(Vector2){ hor_positions[0], vert_positions[0] },
	(Vector2){ hor_positions[0], vert_positions[1] },
	(Vector2){ hor_positions[1], vert_positions[3] },
	(Vector2){ hor_positions[1], vert_positions[2] },
	(Vector2){ hor_positions[0], vert_positions[0] }
    };
    DrawSplineLinear(&points, 5, 1, RED);
}

int main() {
    Player player = {
	(Vector2){ -1, 0 },
	0	
    };
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gloom");
    while(!WindowShouldClose()) {
	ClearBackground(RAYWHITE);
	BeginDrawing();
	    draw_segment(&WALLS[0], &player, 3);
	    player.orient += 0.1;
	EndDrawing();
	usleep(10000);
    }
    CloseWindow();
    return 0;
}
