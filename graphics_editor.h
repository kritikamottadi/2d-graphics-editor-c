#ifndef GRAPHICS_EDITOR_H
#define GRAPHICS_EDITOR_H

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 90
#define HEIGHT 30
#define EMPTY '_'
#define DRAWN '*'

typedef struct {
    int x1, y1, x2, y2;
    char type;
} Shape;

typedef struct {
    char grid[HEIGHT][WIDTH];
    Shape shapes[50];
    int total_shapes;
} Canvas;

Canvas* create_canvas();
void display_canvas(Canvas* c);
void clear_canvas(Canvas* c);
void free_canvas(Canvas* c);

void draw_line(Canvas* c, int x1, int y1, int x2, int y2);
void draw_rectangle(Canvas* c, int x1, int y1, int x2, int y2);
void draw_circle(Canvas* c, int cx, int cy, int radius);
void draw_triangle(Canvas* c, int x1, int y1, int x2, int y2, int x3, int y3);

void add_shape(Canvas* c, int x1, int y1, int x2, int y2, char type);
void list_shapes(Canvas* c);
void delete_shape(Canvas* c, int index);

void show_menu();
void run_program();

#endif