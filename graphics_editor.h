#ifndef GRAPHICS_EDITOR_H
#define GRAPHICS_EDITOR_H

#include <stdio.h>
#include <stdlib.h>

// Canvas size
#define WIDTH 60
#define HEIGHT 20

// Characters for drawing
#define EMPTY '_'
#define DRAWN '*'

// Structure to store one shape's information
typedef struct {
    int x1, y1;    // First point
    int x2, y2;    // Second point
    char shape_type;  // 'L' = Line, 'R' = Rectangle, 'C' = Circle, 'T' = Triangle
} Shape;

// Structure for the canvas (drawing area)
typedef struct {
    char grid[HEIGHT][WIDTH];  // 2D array to draw on
    Shape shapes[50];          // Store up to 50 shapes
    int total_shapes;          // Count of shapes
} Canvas;

// Function declarations (what functions we will use)
Canvas* create_canvas();
void display_canvas(Canvas* canvas);
void clear_canvas(Canvas* canvas);
void free_canvas(Canvas* canvas);

void draw_line(Canvas* canvas, int x1, int y1, int x2, int y2);
void draw_rectangle(Canvas* canvas, int x1, int y1, int x2, int y2);
void draw_circle(Canvas* canvas, int cx, int cy, int radius);
void draw_triangle(Canvas* canvas, int x1, int y1, int x2, int y2, int x3, int y3);

void add_shape(Canvas* canvas, int x1, int y1, int x2, int y2, char type);
void list_shapes(Canvas* canvas);
void delete_shape(Canvas* canvas, int index);

void show_menu();
void run_program();

#endif