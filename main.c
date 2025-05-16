#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

const uint32_t SCREEN_WIDTH = 1080;
const uint32_t SCREEN_HEIGHT = 720;
const float HORIZONTAL_FOV = 90.0;
const float VERTICAL_FOV = 45.0;
const float PLAYER_HEIGHT = 1.91;

typedef enum {
    ZERO,
    ONE,
    INFINITE
} IntersectionCount;
//typedef enum {
//    CLOSED,
//    HALF_CLOSED,
//    HALF_OPEN,
//    OPEN
//} IntervalType;
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

Segment WALLS[] = {
    (Segment){
	(Vector2){ 2, 3 }, 
	(Vector2){ 2, -3 } 
    },
    (Segment){
	(Vector2){ -3, 3 }, 
	(Vector2){ 3, 3 } 
    }
};
Line line_from_points(
    Vector2* point1,
    Vector2* point2
) {
    float A = point1->y - point2->y;
    float B = point2->x - point1->x;
    float C = A * point1->x + B * point1->y;
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
    float A = sin(math_angle / 180 * M_PI);
    float B = -cos(math_angle / 180 * M_PI);
    float C = A * point->x + B * point->y;
    Line line = {
	A,
	B,
	C
    };
    return line;
};
/*bool point_is_on_line(Vector2* point, Line* line) {
    return (line->A * point->x + line->B * point->y == line->C);
}*/
Line line_normal(Line* line, Vector2* point) {
    return (Line){
	line->B,
	-line->A,
	(line->B * point->x - line->A * point->y)
    };
}

IntersectionCount intersection_lines(
    Line* line1,
    Line* line2,
    Vector2* intersection
) {
    if (line1->A == line2->A && line1->B == line2->B) {
	if (line1->C == line2->C) return INFINITE;
	else return ZERO;
    }
    float denom = (line1->A - (line1->B * line2->A / line2->B));
    float x = (line1->C - (line1->B * line2->C / line2->B)) / denom;
    float y = (line2->C - (line2->A - x)) / line2->B;
    *intersection = (Vector2){ x, y };
    return ONE;
}
IntersectionCount intersection_segment(
    Segment* segment,
    Line* line,
    Vector2* intersection
) {
    Line seg_line = line_from_points(&segment->start, &segment->end);
    IntersectionCount count = intersection_lines(&seg_line, line, intersection);
    if (count == ONE && (
	intersection->x < segment->start.x ||
	intersection->x > segment->end.x
    )) count = ZERO;
    return count;
}
float hor_distance(Vector2* point1, Vector2* point2) { 
    return sqrt(pow((point2->x - point1->x), 2) + pow((point2->y - point1->y), 2));
};
bool line_apply_x(Line* line, float val, float* res) {
    if (line->B == 0) return false;
    *res = (line->C - (line->A * val)) / line->B;
    return true;
}
bool line_apply_y(Line* line, float val, float* res) {
    if (line->A == 0) return false;
    *res = (line->C - (line->B * val)) / line->A;
    return true;
};
void displayed_angles_y (
    Player* player,
    Vector2* point,
    float obj_height,
    float* bottom,
    float* top
) {
    float dist = hor_distance(&player->pos, point);
    if (dist == 0 && obj_height >= PLAYER_HEIGHT) {
	*top = VERTICAL_FOV / 2;	
	*bottom = VERTICAL_FOV / 2;	
	return;
    }

    //Lines are in the "3D plane" (Y axis)
    /*float fov_deg[2] = {
	//Is player->orient actually needed
	(VERTICAL_FOV / 2),
	-(VERTICAL_FOV / 2)
    };*/
    Line wall_line = line_from_points(&(Vector2){-dist, 0}, &(Vector2){-dist, 1});
    //First fov_edge is bottom, second is top
    for (int i = 0; i < 2; i++) {
	float fov_deg;
	if (i == 1) 
	    fov_deg = VERTICAL_FOV / 2;
	else 
	    fov_deg = -(VERTICAL_FOV / 2);
	Line fov_edge = line_from_angle(&player->pos, fov_deg);
	Vector2 intersection;
	IntersectionCount count = intersection_lines(&fov_edge, &wall_line, &intersection);	
	//TODO: What happens if the lines are parallel? (eg. 180 deg)
	//  Panic for now
	if (count != ONE) exit(0);
	//Deduce if the point is on the segment
	//What to do with that info though?
	//  It decides if the line reaches to the top or bottom of the screen
	if (intersection.y > obj_height) {
	    
	};
    }

};
bool segment_from_triangle(Line* seg_line, Line* line1, Line* line2, Segment* result) {
    Line* lines[2] = { line1, line2 };
    Vector2 ends[2];
    for (int i = 0; i < 2; i++) {
	Vector2 intersection;
	IntersectionCount count = intersection_lines(seg_line, lines[i], &intersection);
	if (count != ONE) return false;
	ends[i] = intersection;
    }
    *result = (Segment){ ends[0], ends[1] };
    return true;
}
//Closed interval
inline bool is_between_c(float val, float begin, float end) {
    return (val >= begin && val <= end);
}
//Half-closed interval
inline bool is_between_hc(float val, float begin, float end) {
    return (val >= begin && val < end);
}
bool is_segment_visible(Player* player, Segment* segment, Segment* result) {
    bool result = false;
    Line vision_line = line_from_angle(&player->pos, player->orient);
    Vector2 ends[2] = { segment->start, segment->end };
    Line fov_left = line_from_angle(&player->pos, player->orient - (HORIZONTAL_FOV / 2));
    Line fov_right = line_from_angle(&player->pos, player->orient + (HORIZONTAL_FOV / 2));

    for (int i = 0; i < 2; i++) {
	Vector2 end = ends[i];
	float start_x = segment->start.x;
	float start_y = segment->start.y;
	float end_x = segment->end.x;
	float end_y = segment->end.y;
	Segment segment;

	Line normal = line_normal(&vision_line, &end);
	bool res = segment_from_triangle(&normal, &fov_left, &fov_right, &segment);
	if (!res) {
	    printf("At least one line is the same as the normal line (FOV >= 180 deg)");
	    return false;
	}
	if (
	    (( start_x <= end_x && is_between_c(end.x, start_x, end_x) )
	    ||
	    ( start_x >= end_x && is_between_c(end.x, end_x, start_x) ))
	    &&
	    (( start_y <= end_y && is_between_c(end.y, start_y, end_y) )
	    ||
	    ( start_y >= end_y && is_between_c(end.y, end_y, start_y) ))
	) result = true;
    }
    //TODO: Orientation of player also decides in visibility
    //The player's position splits the line in two, one half will go in quadrant q, then other one will go into q + 2
    //The orientation of the player will decide which quadrant is the right one
    float player_mod = fmod(player->orient, 360);
    bool is_right = !is_between_hc(player_mod, 90, 270);
    bool is_top = is_between_hc(player_mod, 0, 180);
    //Perhaps the x and y isn't what I should use, perhaps it's the FOV lines' orientations in degrees (player->orient +- HORIZONTAL_FOV / 2) 
    //What happens with the coords of the segment ends?
    if (is_right ^ x > player->pos.x) result = false;
    if (is_top ^ y > player->pos.y) result = false;
}
void draw_segment(
    const Segment* segment, 
    Player* player, 
    float height
) {
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
	displayed_angles_y(player, &intersection, height, &bottom, &top);

	float top_ratio = top * 2 / VERTICAL_FOV;
	float bottom_ratio = bottom * 2 / VERTICAL_FOV;
	int top_pos = round((SCREEN_HEIGHT / 2) * (1 - top_ratio));
	int bottom_pos = round((SCREEN_HEIGHT / 2) * (1 + top_ratio));
	vert_positions[i] = bottom_pos;
	vert_positions[i + 1] = bottom_pos;
    }
    Vector2 vertices[5] = {
	(Vector2){ hor_positions[0], vert_positions[0] },
	(Vector2){ hor_positions[0], vert_positions[1] },
	(Vector2){ hor_positions[1], vert_positions[3] },
	(Vector2){ hor_positions[1], vert_positions[2] },
	(Vector2){ hor_positions[0], vert_positions[0] }
    };
    DrawSplineLinear(&vertices[0], 5, 1, RED);
}

int main() {
    Player player = {
	(Vector2){ 0, 0 },
	0	
    };
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gloom");
    while(!WindowShouldClose()) {
	BeginDrawing();
	EndDrawing();
	ClearBackground(BLACK);
	usleep(10000);
    }
    /*while(player.orient < 360 && !WindowShouldClose()) {
	Line player_line = line_from_angle(&player.pos, player.orient);
	printf("-%f: %f, %f, %f\n", player.orient, player_line.A, player_line.B, player_line.C);
	float y;
	bool apply_res = line_apply(&player_line, 5, false, &y);
	if (apply_res) {
	    Vector2 point = { 5, y };
	    Line normal = line_normal(&player_line, &point);
	    Line fov_left = line_from_angle(&player.pos, player.orient - (HORIZONTAL_FOV / 2));
	    Line fov_right = line_from_angle(&player.pos, player.orient + (HORIZONTAL_FOV / 2));
	    Segment result;
	    bool res = segment_from_triangle(&normal, &fov_left, &fov_right, &result);
	    Line result_line = line_from_points(&result.start, &result.end);
	    printf("%f: %f, %f, %f\n", player.orient, result_line.A, result_line.B, result_line.C);
	} else {
	    printf("fail");
	}

	Segment result;
	bool res = segment_visibility(&player, &WALLS[1], &result);
	printf("%f %d\n", player.orient, res);
    	ClearBackground(RAYWHITE);
	BeginDrawing();

	EndDrawing();
	player.orient += 0.1;
    }
    while(!WindowShouldClose()) {
	Line line = line_from_angle(&player.pos, player.orient);
	printf("%f: %f, %f, %f\n", player.orient, line.A, line.B, line.C);
	float y1;
	bool res1 = line_apply_x(&line, -500, &y1);
	float y2;
	bool res2 = line_apply_x(&line, 500, &y2);
	printf("Val: %f, %f\n", y1, y2);
	if (!res1 || !res2) continue;
	Vector2 point1 = {
	    200,
	    y1 + 500
	};
	Vector2 point2 = {
	    700,
	    y2 + 500
	};
	BeginDrawing();
	DrawLineV(point1, point2, RED);
	EndDrawing();
	ClearBackground(BLACK);
	player.orient += 1;
	usleep(10000);
    }*/
    CloseWindow();
    return 0;
}
