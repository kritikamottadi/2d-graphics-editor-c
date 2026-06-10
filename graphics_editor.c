#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
void enable_ansi_support() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void enable_ansi_support() {}
#endif

#define MAX_WIDTH 80
#define MAX_HEIGHT 40
#define MAX_OBJECTS 100

typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

typedef struct {
    char val;
    int color_code; // 1=Green, 2=Yellow, 3=Red, 4=Blue, 5=Cyan, 6=Magenta
} CanvasCell;

typedef struct {
    int x1, y1;
    int x2, y2;
} LineData;

typedef struct {
    int x, y;
    int width, height;
} RectData;

typedef struct {
    int cx, cy;
    int radius;
} CircleData;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleData;

typedef struct {
    int id;
    ShapeType type;
    int is_filled;
    int color_code;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriangleData triangle;
    } data;
    int is_active;
} DrawingObject;

// Global State
CanvasCell canvas[MAX_HEIGHT][MAX_WIDTH];
int canvas_width = 60;
int canvas_height = 20;
DrawingObject objects[MAX_OBJECTS];
int object_count = 0;
int next_id = 1;

// Function declarations
void clear_input_buffer();
int get_int_input(const char* prompt, int min_val, int max_val);
void initialize_canvas();
void redraw_all();
void display_canvas();
void add_object_menu();
void delete_object_menu();
void modify_object_menu();
int get_color_selection();
const char* get_color_name(int code);

// Drawing Utilities
void draw_pixel_type_color(int x, int y, char type, int color_code) {
    if (x >= 0 && x < canvas_width && y >= 0 && y < canvas_height) {
        canvas[y][x].val = type;
        canvas[y][x].color_code = color_code;
    }
}

// Drawing Algorithms
void draw_line(int x0, int y0, int x1, int y1, int color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        draw_pixel_type_color(x0, y0, '.', color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_rectangle(int rx, int ry, int rwidth, int rheight, int fill, int color) {
    if (fill) {
        for (int y = ry; y < ry + rheight; y++) {
            for (int x = rx; x < rx + rwidth; x++) {
                draw_pixel_type_color(x, y, ':', color);
            }
        }
    }
    // Draw boundary border on top
    for (int i = 0; i < rwidth; i++) {
        draw_pixel_type_color(rx + i, ry, '.', color);
        draw_pixel_type_color(rx + i, ry + rheight - 1, '.', color);
    }
    for (int i = 0; i < rheight; i++) {
        draw_pixel_type_color(rx, ry + i, '.', color);
        draw_pixel_type_color(rx + rwidth - 1, ry + i, '.', color);
    }
}

void draw_circle(int cx, int cy, int r, int fill, int color) {
    const double ASPECT_RATIO = 2.0; 
    
    int min_y = cy - r;
    int max_y = cy + r;
    int min_x = cx - (int)(r * ASPECT_RATIO) - 1;
    int max_x = cx + (int)(r * ASPECT_RATIO) + 1;
    
    // Draw interior if filled
    if (fill) {
        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                double dx = (x - cx) / ASPECT_RATIO;
                double dy = y - cy;
                double dist = sqrt(dx * dx + dy * dy);
                if (dist <= r + 0.3) {
                    draw_pixel_type_color(x, y, ':', color);
                }
            }
        }
    }
    
    // Draw outline boundary
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            double dx = (x - cx) / ASPECT_RATIO;
            double dy = y - cy;
            double dist = sqrt(dx * dx + dy * dy);
            if (fabs(dist - r) <= 0.6) {
                draw_pixel_type_color(x, y, '.', color);
            }
        }
    }
}

int is_point_in_triangle(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3) {
    long long d1 = (long long)(px - x2) * (y1 - y2) - (long long)(x1 - x2) * (py - y2);
    long long d2 = (long long)(px - x3) * (y2 - y3) - (long long)(x2 - x3) * (py - y3);
    long long d3 = (long long)(px - x1) * (y3 - y1) - (long long)(x3 - x1) * (py - y1);
    
    int has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    int has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    
    return !(has_neg && has_pos);
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int fill, int color) {
    if (fill) {
        int min_x = x1; if (x2 < min_x) min_x = x2; if (x3 < min_x) min_x = x3;
        int max_x = x1; if (x2 > max_x) max_x = x2; if (x3 > max_x) max_x = x3;
        int min_y = y1; if (y2 < min_y) min_y = y2; if (y3 < min_y) min_y = y3;
        int max_y = y1; if (y2 > max_y) max_y = y2; if (y3 > max_y) max_y = y3;
        
        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                if (is_point_in_triangle(x, y, x1, y1, x2, y2, x3, y3)) {
                    draw_pixel_type_color(x, y, ':', color);
                }
            }
        }
    }
    // Draw boundary outline lines
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x3, y3, color);
    draw_line(x3, y3, x1, y1, color);
}

// Canvas & Object Management
void initialize_canvas() {
    for (int y = 0; y < canvas_height; y++) {
        for (int x = 0; x < canvas_width; x++) {
            canvas[y][x].val = '_';
            canvas[y][x].color_code = 0;
        }
    }
}

void redraw_all() {
    initialize_canvas();
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].is_active) continue;
        switch (objects[i].type) {
            case SHAPE_LINE:
                draw_line(objects[i].data.line.x1, objects[i].data.line.y1,
                          objects[i].data.line.x2, objects[i].data.line.y2,
                          objects[i].color_code);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(objects[i].data.rect.x, objects[i].data.rect.y,
                               objects[i].data.rect.width, objects[i].data.rect.height,
                               objects[i].is_filled, objects[i].color_code);
                break;
            case SHAPE_CIRCLE:
                draw_circle(objects[i].data.circle.cx, objects[i].data.circle.cy,
                            objects[i].data.circle.radius, objects[i].is_filled,
                            objects[i].color_code);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(objects[i].data.triangle.x1, objects[i].data.triangle.y1,
                              objects[i].data.triangle.x2, objects[i].data.triangle.y2,
                              objects[i].data.triangle.x3, objects[i].data.triangle.y3,
                              objects[i].is_filled, objects[i].color_code);
                break;
        }
    }
}

void display_canvas() {
    printf("\033[H\033[J");
    printf("\033[1;36m==================== 2D GRAPHICS EDITOR ====================\033[0m\n\n");
    
    // Print column coordinate headers
    printf("     ");
    for (int x = 0; x < canvas_width; x++) {
        if (x % 10 == 0) printf("%d", x / 10);
        else printf(" ");
    }
    printf("\n     ");
    for (int x = 0; x < canvas_width; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
    
    // Top border
    printf("   +");
    for (int x = 0; x < canvas_width; x++) printf("-");
    printf("+\n");
    
    // Render cells
    for (int y = 0; y < canvas_height; y++) {
        printf("%2d |", y);
        for (int x = 0; x < canvas_width; x++) {
            CanvasCell cell = canvas[y][x];
            if (cell.val == '_') {
                printf("\033[90m%c\033[0m", cell.val); // dark grey background
            } else {
                switch (cell.color_code) {
                    case 1: printf("\033[1;32m%c\033[0m", cell.val); break; // Green
                    case 2: printf("\033[1;33m%c\033[0m", cell.val); break; // Yellow
                    case 3: printf("\033[1;31m%c\033[0m", cell.val); break; // Red
                    case 4: printf("\033[1;34m%c\033[0m", cell.val); break; // Blue
                    case 5: printf("\033[1;36m%c\033[0m", cell.val); break; // Cyan
                    case 6: printf("\033[1;35m%c\033[0m", cell.val); break; // Magenta
                    default: printf("%c", cell.val); break;
                }
            }
        }
        printf("|\n");
    }
    
    // Bottom border
    printf("   +");
    for (int x = 0; x < canvas_width; x++) printf("-");
    printf("+\n\n");
}

const char* get_color_name(int code) {
    switch (code) {
        case 1: return "Green";
        case 2: return "Yellow";
        case 3: return "Red";
        case 4: return "Blue";
        case 5: return "Cyan";
        case 6: return "Magenta";
        default: return "Unknown";
    }
}

void print_object_details(DrawingObject* obj) {
    const char* fill_str = obj->is_filled ? "Filled" : "Outline";
    const char* color_str = get_color_name(obj->color_code);
    switch (obj->type) {
        case SHAPE_LINE:
            printf("Line from (%d, %d) to (%d, %d) [Color: %s]",
                   obj->data.line.x1, obj->data.line.y1,
                   obj->data.line.x2, obj->data.line.y2, color_str);
            break;
        case SHAPE_RECTANGLE:
            printf("Rectangle at (%d, %d) width %d, height %d [%s] [Color: %s]",
                   obj->data.rect.x, obj->data.rect.y,
                   obj->data.rect.width, obj->data.rect.height, fill_str, color_str);
            break;
        case SHAPE_CIRCLE:
            printf("Circle at (%d, %d) radius %d [%s] [Color: %s]",
                   obj->data.circle.cx, obj->data.circle.cy,
                   obj->data.circle.radius, fill_str, color_str);
            break;
        case SHAPE_TRIANGLE:
            printf("Triangle Vertices: (%d, %d), (%d, %d), (%d, %d) [%s] [Color: %s]",
                   obj->data.triangle.x1, obj->data.triangle.y1,
                   obj->data.triangle.x2, obj->data.triangle.y2,
                   obj->data.triangle.x3, obj->data.triangle.y3, fill_str, color_str);
            break;
    }
}

// User Inputs & Safe Parsing
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int get_int_input(const char* prompt, int min_val, int max_val) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1) {
            clear_input_buffer();
            if (val >= min_val && val <= max_val) {
                return val;
            }
            printf("\033[1;31mError: Input must be between %d and %d.\033[0m\n", min_val, max_val);
        } else {
            clear_input_buffer();
            printf("\033[1;31mError: Invalid integer input.\033[0m\n");
        }
    }
}

int get_color_selection() {
    printf("\033[1;36mSelect Shape Color:\033[0m\n");
    printf("1. \033[1;32mGreen\033[0m\n");
    printf("2. \033[1;33mYellow\033[0m\n");
    printf("3. \033[1;31mRed\033[0m\n");
    printf("4. \033[1;34mBlue\033[0m\n");
    printf("5. \033[1;36mCyan\033[0m\n");
    printf("6. \033[1;35mMagenta\033[0m\n");
    return get_int_input("Enter choice (1-6): ", 1, 6);
}

// Shape Add Menu
void add_object_menu() {
    if (object_count >= MAX_OBJECTS) {
        printf("\033[1;31mError: Canvas object capacity reached (%d objects).\033[0m\n", MAX_OBJECTS);
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    printf("\033[1;36mSelect Shape to Add:\033[0m\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. Cancel\n");
    
    int choice = get_int_input("Enter choice: ", 1, 5);
    if (choice == 5) return;

    DrawingObject obj;
    obj.id = next_id++;
    obj.is_active = 1;
    obj.is_filled = 0;
    obj.color_code = 1; // default Green

    switch (choice) {
        case 1: // Line
            obj.type = SHAPE_LINE;
            printf("\033[1;32m--- Adding Line ---\033[0m\n");
            obj.data.line.x1 = get_int_input("Enter start X1: ", 0, canvas_width - 1);
            obj.data.line.y1 = get_int_input("Enter start Y1: ", 0, canvas_height - 1);
            obj.data.line.x2 = get_int_input("Enter end X2: ", 0, canvas_width - 1);
            obj.data.line.y2 = get_int_input("Enter end Y2: ", 0, canvas_height - 1);
            break;
        case 2: // Rectangle
            obj.type = SHAPE_RECTANGLE;
            printf("\033[1;32m--- Adding Rectangle ---\033[0m\n");
            obj.data.rect.x = get_int_input("Enter top-left X: ", 0, canvas_width - 1);
            obj.data.rect.y = get_int_input("Enter top-left Y: ", 0, canvas_height - 1);
            obj.data.rect.width = get_int_input("Enter width: ", 1, canvas_width);
            obj.data.rect.height = get_int_input("Enter height: ", 1, canvas_height);
            obj.is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
        case 3: // Circle
            obj.type = SHAPE_CIRCLE;
            printf("\033[1;32m--- Adding Circle ---\033[0m\n");
            obj.data.circle.cx = get_int_input("Enter center X: ", 0, canvas_width - 1);
            obj.data.circle.cy = get_int_input("Enter center Y: ", 0, canvas_height - 1);
            obj.data.circle.radius = get_int_input("Enter radius (>= 1): ", 1, canvas_width);
            obj.is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
        case 4: // Triangle
            obj.type = SHAPE_TRIANGLE;
            printf("\033[1;32m--- Adding Triangle ---\033[0m\n");
            obj.data.triangle.x1 = get_int_input("Enter vertex X1: ", 0, canvas_width - 1);
            obj.data.triangle.y1 = get_int_input("Enter vertex Y1: ", 0, canvas_height - 1);
            obj.data.triangle.x2 = get_int_input("Enter vertex X2: ", 0, canvas_width - 1);
            obj.data.triangle.y2 = get_int_input("Enter vertex Y2: ", 0, canvas_height - 1);
            obj.data.triangle.x3 = get_int_input("Enter vertex X3: ", 0, canvas_width - 1);
            obj.data.triangle.y3 = get_int_input("Enter vertex Y3: ", 0, canvas_height - 1);
            obj.is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
    }

    obj.color_code = get_color_selection();

    objects[object_count++] = obj;
    redraw_all();
    printf("\033[1;32mObject added successfully!\033[0m\n");
    printf("Press Enter to continue...");
    getchar();
}

// Shape Delete Menu
void delete_object_menu() {
    int active_found = 0;
    printf("\033[1;36mActive Objects:\033[0m\n");
    for (int i = 0; i < object_count; i++) {
        if (objects[i].is_active) {
            printf("[%d] ", objects[i].id);
            print_object_details(&objects[i]);
            printf("\n");
            active_found++;
        }
    }

    if (active_found == 0) {
        printf("No active shapes to delete.\n");
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    int del_id = get_int_input("Enter ID of object to delete (0 to cancel): ", 0, next_id - 1);
    if (del_id == 0) return;

    int deleted = 0;
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == del_id && objects[i].is_active) {
            objects[i].is_active = 0;
            deleted = 1;
            break;
        }
    }

    if (deleted) {
        redraw_all();
        printf("\033[1;32mObject %d deleted and canvas updated.\033[0m\n", del_id);
    } else {
        printf("\033[1;31mError: Object ID not found or already deleted.\033[0m\n");
    }
    printf("Press Enter to continue...");
    getchar();
}

// Shape Modify Menu
void modify_object_menu() {
    int active_found = 0;
    printf("\033[1;36mSelect Object to Modify:\033[0m\n");
    for (int i = 0; i < object_count; i++) {
        if (objects[i].is_active) {
            printf("[%d] ", objects[i].id);
            print_object_details(&objects[i]);
            printf("\n");
            active_found++;
        }
    }

    if (active_found == 0) {
        printf("No active shapes to modify.\n");
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    int mod_id = get_int_input("Enter ID of object to modify (0 to cancel): ", 0, next_id - 1);
    if (mod_id == 0) return;

    DrawingObject* obj = NULL;
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == mod_id && objects[i].is_active) {
            obj = &objects[i];
            break;
        }
    }

    if (!obj) {
        printf("\033[1;31mError: Object ID not found.\033[0m\n");
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    printf("\033[1;32mModifying Object %d:\033[0m ", obj->id);
    print_object_details(obj);
    printf("\n");

    switch (obj->type) {
        case SHAPE_LINE:
            obj->data.line.x1 = get_int_input("Enter new start X1: ", 0, canvas_width - 1);
            obj->data.line.y1 = get_int_input("Enter new start Y1: ", 0, canvas_height - 1);
            obj->data.line.x2 = get_int_input("Enter new end X2: ", 0, canvas_width - 1);
            obj->data.line.y2 = get_int_input("Enter new end Y2: ", 0, canvas_height - 1);
            break;
        case SHAPE_RECTANGLE:
            obj->data.rect.x = get_int_input("Enter new top-left X: ", 0, canvas_width - 1);
            obj->data.rect.y = get_int_input("Enter new top-left Y: ", 0, canvas_height - 1);
            obj->data.rect.width = get_int_input("Enter new width: ", 1, canvas_width);
            obj->data.rect.height = get_int_input("Enter new height: ", 1, canvas_height);
            obj->is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
        case SHAPE_CIRCLE:
            obj->data.circle.cx = get_int_input("Enter new center X: ", 0, canvas_width - 1);
            obj->data.circle.cy = get_int_input("Enter new center Y: ", 0, canvas_height - 1);
            obj->data.circle.radius = get_int_input("Enter new radius (>= 1): ", 1, canvas_width);
            obj->is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
        case SHAPE_TRIANGLE:
            obj->data.triangle.x1 = get_int_input("Enter new vertex X1: ", 0, canvas_width - 1);
            obj->data.triangle.y1 = get_int_input("Enter new vertex Y1: ", 0, canvas_height - 1);
            obj->data.triangle.x2 = get_int_input("Enter new vertex X2: ", 0, canvas_width - 1);
            obj->data.triangle.y2 = get_int_input("Enter new vertex Y2: ", 0, canvas_height - 1);
            obj->data.triangle.x3 = get_int_input("Enter new vertex X3: ", 0, canvas_width - 1);
            obj->data.triangle.y3 = get_int_input("Enter new vertex Y3: ", 0, canvas_height - 1);
            obj->is_filled = get_int_input("Fill shape? (0 = Outline only, 1 = Filled): ", 0, 1);
            break;
    }

    obj->color_code = get_color_selection();

    redraw_all();
    printf("\033[1;32mObject updated successfully!\033[0m\n");
    printf("Press Enter to continue...");
    getchar();
}

void resize_canvas_menu() {
    printf("\033[1;36m--- Resize Canvas ---\033[0m\n");
    printf("Current size: %d columns x %d rows\n", canvas_width, canvas_height);
    int new_w = get_int_input("Enter new width (10 to 80): ", 10, MAX_WIDTH);
    int new_h = get_int_input("Enter new height (5 to 40): ", 5, MAX_HEIGHT);
    canvas_width = new_w;
    canvas_height = new_h;
    redraw_all();
    printf("\033[1;32mCanvas resized and objects redrawn.\033[0m\n");
    printf("Press Enter to continue...");
    getchar();
}

int main() {
    enable_ansi_support();
    initialize_canvas();

    // No default objects initially loaded so user starts with a clean screen
    redraw_all();

    while (1) {
        display_canvas();
        printf("\033[1;36mChoose an operation:\033[0m\n");
        printf("1. Add graphical object\n");
        printf("2. Delete graphical object\n");
        printf("3. Modify graphical object\n");
        printf("4. Display canvas (Redraw)\n");
        printf("5. Resize Canvas\n");
        printf("6. Exit program\n\n");

        int choice = get_int_input("Enter menu choice (1-6): ", 1, 6);
        switch (choice) {
            case 1:
                add_object_menu();
                break;
            case 2:
                delete_object_menu();
                break;
            case 3:
                modify_object_menu();
                break;
            case 4:
                redraw_all();
                printf("\033[1;32mCanvas re-rendered.\033[0m\n");
                printf("Press Enter to continue...");
                getchar();
                break;
            case 5:
                resize_canvas_menu();
                break;
            case 6:
                printf("\033[1;32mExiting Graphics Editor. Goodbye!\033[0m\n");
                return 0;
        }
    }
}
