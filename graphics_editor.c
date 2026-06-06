#include "graphics_editor.h"

// ===== CANVAS FUNCTIONS =====

// Create a new empty canvas
Canvas* create_canvas() {
    Canvas* canvas = (Canvas*)malloc(sizeof(Canvas));
    canvas->total_shapes = 0;
    clear_canvas(canvas);
    return canvas;
}

// Fill canvas with empty character
void clear_canvas(Canvas* canvas) {
    int i, j;
    
    // Loop through every row
    for (i = 0; i < HEIGHT; i++) {
        // Loop through every column
        for (j = 0; j < WIDTH; j++) {
            canvas->grid[i][j] = EMPTY;  // Fill with '_'
        }
    }
    
    // Clear shapes list
    canvas->total_shapes = 0;
}

// Display the canvas on screen
void display_canvas(Canvas* canvas) {
    int i, j;
    
    printf("\n");
    
    // Print top border
    printf("+");
    for (j = 0; j < WIDTH; j++) {
        printf("-");
    }
    printf("+\n");
    
    // Print each row of canvas
    for (i = 0; i < HEIGHT; i++) {
        printf("|");
        for (j = 0; j < WIDTH; j++) {
            printf("%c", canvas->grid[i][j]);  // Print each character
        }
        printf("|\n");
    }
    
    // Print bottom border
    printf("+");
    for (j = 0; j < WIDTH; j++) {
        printf("-");
    }
    printf("+\n\n");
}

// Free memory
void free_canvas(Canvas* canvas) {
    free(canvas);
}

// ===== DRAWING HELPER FUNCTION =====

// Put a '*' at position (x, y) on canvas
void set_pixel(Canvas* canvas, int x, int y) {
    // Check if position is valid (inside canvas)
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas->grid[y][x] = DRAWN;  // Set to '*'
    }
}

// ===== DRAWING FUNCTIONS =====

// Draw a line from (x1,y1) to (x2,y2) using simple algorithm
void draw_line(Canvas* canvas, int x1, int y1, int x2, int y2) {
    int x, y;
    int steps;
    float x_increment, y_increment;
    
    // How many steps needed?
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // More horizontal or vertical?
    if (abs(dx) > abs(dy)) {
        steps = abs(dx);
    } else {
        steps = abs(dy);
    }
    
    // Calculate increment for each step
    if (steps > 0) {
        x_increment = (float)dx / steps;
        y_increment = (float)dy / steps;
    } else {
        return;
    }
    
    x = x1;
    y = y1;
    
    // Draw each point on the line
    int i;
    for (i = 0; i <= steps; i++) {
        set_pixel(canvas, x, y);
        x = x + x_increment;
        y = y + y_increment;
    }
}

// Draw a rectangle (hollow box)
void draw_rectangle(Canvas* canvas, int x1, int y1, int x2, int y2) {
    int x, y;
    int left, right, top, bottom;
    
    // Find which coordinates are corners
    left = (x1 < x2) ? x1 : x2;
    right = (x1 > x2) ? x1 : x2;
    top = (y1 < y2) ? y1 : y2;
    bottom = (y1 > y2) ? y1 : y2;
    
    // Draw top and bottom edges
    for (x = left; x <= right; x++) {
        set_pixel(canvas, x, top);
        set_pixel(canvas, x, bottom);
    }
    
    // Draw left and right edges
    for (y = top; y <= bottom; y++) {
        set_pixel(canvas, left, y);
        set_pixel(canvas, right, y);
    }
}

// Draw a circle at center (cx, cy) with given radius
void draw_circle(Canvas* canvas, int cx, int cy, int radius) {
    int x, y;
    int radius_sq = radius * radius;
    
    // Loop through all points in a square around the circle
    for (y = cy - radius; y <= cy + radius; y++) {
        for (x = cx - radius; x <= cx + radius; x++) {
            // Distance formula: check if point is on circle
            int dx = x - cx;
            int dy = y - cy;
            int distance_sq = dx*dx + dy*dy;
            
            // If distance is close to radius, it's on the circle
            if (distance_sq >= radius_sq - radius && distance_sq <= radius_sq + radius) {
                set_pixel(canvas, x, y);
            }
        }
    }
}

// Draw a triangle by connecting 3 points
void draw_triangle(Canvas* canvas, int x1, int y1, int x2, int y2, int x3, int y3) {
    // Draw three lines connecting the three points
    draw_line(canvas, x1, y1, x2, y2);  // Line 1
    draw_line(canvas, x2, y2, x3, y3);  // Line 2
    draw_line(canvas, x3, y3, x1, y1);  // Line 3
}

// ===== SHAPE MANAGEMENT =====

// Add a shape to the list and draw it
void add_shape(Canvas* canvas, int x1, int y1, int x2, int y2, char type) {
    // Check if we have room for more shapes
    if (canvas->total_shapes >= 50) {
        printf("ERROR: Cannot add more shapes!\n");
        return;
    }
    
    // Store shape information
    canvas->shapes[canvas->total_shapes].x1 = x1;
    canvas->shapes[canvas->total_shapes].y1 = y1;
    canvas->shapes[canvas->total_shapes].x2 = x2;
    canvas->shapes[canvas->total_shapes].y2 = y2;
    canvas->shapes[canvas->total_shapes].shape_type = type;
    canvas->total_shapes++;
    
    // Draw the shape based on type
    if (type == 'L') {
        draw_line(canvas, x1, y1, x2, y2);
        printf("Line drawn!\n");
    } 
    else if (type == 'R') {
        draw_rectangle(canvas, x1, y1, x2, y2);
        printf("Rectangle drawn!\n");
    } 
    else if (type == 'C') {
        int radius = abs(x2 - x1);  // Radius is distance
        draw_circle(canvas, x1, y1, radius);
        printf("Circle drawn!\n");
    }
}

// Show all shapes that were drawn
void list_shapes(Canvas* canvas) {
    int i;
    
    printf("\n=== Shapes on Canvas ===\n");
    
    if (canvas->total_shapes == 0) {
        printf("No shapes yet.\n");
        return;
    }
    
    // Print each shape
    for (i = 0; i < canvas->total_shapes; i++) {
        printf("Shape %d: ", i);
        
        if (canvas->shapes[i].shape_type == 'L') {
            printf("Line from (%d,%d) to (%d,%d)\n",
                   canvas->shapes[i].x1, canvas->shapes[i].y1,
                   canvas->shapes[i].x2, canvas->shapes[i].y2);
        } 
        else if (canvas->shapes[i].shape_type == 'R') {
            printf("Rectangle from (%d,%d) to (%d,%d)\n",
                   canvas->shapes[i].x1, canvas->shapes[i].y1,
                   canvas->shapes[i].x2, canvas->shapes[i].y2);
        } 
        else if (canvas->shapes[i].shape_type == 'C') {
            printf("Circle at (%d,%d)\n",
                   canvas->shapes[i].x1, canvas->shapes[i].y1);
        }
    }
    printf("=======================\n\n");
}

// Delete a shape by index and redraw canvas
void delete_shape(Canvas* canvas, int index) {
    int i;
    
    // Check if index is valid
    if (index < 0 || index >= canvas->total_shapes) {
        printf("ERROR: Invalid shape number!\n");
        return;
    }
    
    // Remove the shape from list
    for (i = index; i < canvas->total_shapes - 1; i++) {
        canvas->shapes[i] = canvas->shapes[i + 1];
    }
    canvas->total_shapes--;
    
    // Clear canvas and redraw all shapes
    clear_canvas(canvas);
    for (i = 0; i < canvas->total_shapes; i++) {
        add_shape(canvas, 
                  canvas->shapes[i].x1, canvas->shapes[i].y1,
                  canvas->shapes[i].x2, canvas->shapes[i].y2,
                  canvas->shapes[i].shape_type);
    }
    
    printf("Shape deleted!\n");
}

// ===== MENU AND MAIN PROGRAM =====

// Display menu options
void show_menu() {
    printf("\n========== MENU ==========\n");
    printf("1. Draw Line\n");
    printf("2. Draw Rectangle\n");
    printf("3. Draw Circle\n");
    printf("4. Draw Triangle\n");
    printf("5. Show All Shapes\n");
    printf("6. Delete Shape\n");
    printf("7. Clear Canvas\n");
    printf("8. Display Canvas\n");
    printf("9. Exit\n");
    printf("==========================\n");
    printf("Enter choice: ");
}

// Main program loop
void run_program() {
    Canvas* canvas = create_canvas();
    int choice;
    int x1, y1, x2, y2, x3, y3;
    int index;
    
    while (1) {
        show_menu();
        scanf("%d", &choice);
        
        if (choice == 1) {
            // Draw Line
            printf("Enter coordinates (x1 y1 x2 y2): ");
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            add_shape(canvas, x1, y1, x2, y2, 'L');
        }
        else if (choice == 2) {
            // Draw Rectangle
            printf("Enter coordinates (x1 y1 x2 y2): ");
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            add_shape(canvas, x1, y1, x2, y2, 'R');
        }
        else if (choice == 3) {
            // Draw Circle
            printf("Enter center and radius (cx cy radius): ");
            scanf("%d %d %d", &x1, &y1, &x2);
            add_shape(canvas, x1, y1, x1 + x2, y1, 'C');
        }
        else if (choice == 4) {
            // Draw Triangle
            printf("Enter 3 points (x1 y1 x2 y2 x3 y3): ");
            scanf("%d %d %d %d %d %d", &x1, &y1, &x2, &y2, &x3, &y3);
            draw_triangle(canvas, x1, y1, x2, y2, x3, y3);
            printf("Triangle drawn!\n");
        }
        else if (choice == 5) {
            // Show shapes
            list_shapes(canvas);
        }
        else if (choice == 6) {
            // Delete shape
            list_shapes(canvas);
            printf("Enter shape number to delete: ");
            scanf("%d", &index);
            delete_shape(canvas, index);
        }
        else if (choice == 7) {
            // Clear canvas
            clear_canvas(canvas);
            printf("Canvas cleared!\n");
        }
        else if (choice == 8) {
            // Display canvas
            display_canvas(canvas);
        }
        else if (choice == 9) {
            // Exit
            free_canvas(canvas);
            printf("Goodbye!\n");
            break;
        }
        else {
            printf("Invalid choice!\n");
        }
    }
}

// Main function - program starts here
int main() {
    run_program();
    return 0;
}