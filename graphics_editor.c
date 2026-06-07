#include "graphics_editor.h"

Canvas* create_canvas() {
    Canvas* c = (Canvas*)malloc(sizeof(Canvas));
    c->total_shapes = 0;
    clear_canvas(c);
    return c;
}

void clear_canvas(Canvas* c) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            c->grid[i][j] = EMPTY;
        }
    }
    c->total_shapes = 0;
}

void set_pixel(Canvas* c, int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        c->grid[y][x] = DRAWN;
    }
}

void display_canvas(Canvas* c) {
    printf("\n");
    for (int i = 0; i < WIDTH + 2; i++) printf("-");
    printf("\n");
    
    for (int i = 0; i < HEIGHT; i++) {
        printf("|");
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", c->grid[i][j]);
        }
        printf("|\n");
    }
    
    for (int i = 0; i < WIDTH + 2; i++) printf("-");
    printf("\n\n");
}

void free_canvas(Canvas* c) {
    free(c);
}

// LINE - DDA Algorithm (proven to work)
void draw_line(Canvas* c, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    
    if (steps == 0) {
        set_pixel(c, x1, y1);
        return;
    }
    
    float xinc = (float)dx / steps;
    float yinc = (float)dy / steps;
    float x = x1, y = y1;
    
    for (int i = 0; i <= steps; i++) {
        set_pixel(c, (int)(x + 0.5), (int)(y + 0.5));
        x += xinc;
        y += yinc;
    }
}

// RECTANGLE - Simple outline
void draw_rectangle(Canvas* c, int x1, int y1, int x2, int y2) {
    int minx = (x1 < x2) ? x1 : x2;
    int maxx = (x1 > x2) ? x1 : x2;
    int miny = (y1 < y2) ? y1 : y2;
    int maxy = (y1 > y2) ? y1 : y2;
    
    for (int x = minx; x <= maxx; x++) {
        set_pixel(c, x, miny);
        set_pixel(c, x, maxy);
    }
    
    for (int y = miny; y <= maxy; y++) {
        set_pixel(c, minx, y);
        set_pixel(c, maxx, y);
    }
}

// CIRCLE - Midpoint Algorithm (proven & tested)
void draw_circle(Canvas* c, int cx, int cy, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    while (x <= y) {
        set_pixel(c, cx + x, cy + y);
        set_pixel(c, cx - x, cy + y);
        set_pixel(c, cx + x, cy - y);
        set_pixel(c, cx - x, cy - y);
        set_pixel(c, cx + y, cy + x);
        set_pixel(c, cx - y, cy + x);
        set_pixel(c, cx + y, cy - x);
        set_pixel(c, cx - y, cy - x);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// TRIANGLE - Three lines
void draw_triangle(Canvas* c, int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(c, x1, y1, x2, y2);
    draw_line(c, x2, y2, x3, y3);
    draw_line(c, x3, y3, x1, y1);
}

void add_shape(Canvas* c, int x1, int y1, int x2, int y2, char type) {
    if (c->total_shapes >= 50) {
        printf("Max shapes reached!\n");
        return;
    }
    
    c->shapes[c->total_shapes].x1 = x1;
    c->shapes[c->total_shapes].y1 = y1;
    c->shapes[c->total_shapes].x2 = x2;
    c->shapes[c->total_shapes].y2 = y2;
    c->shapes[c->total_shapes].type = type;
    c->total_shapes++;
    
    if (type == 'L') {
        draw_line(c, x1, y1, x2, y2);
        printf("Line added!\n");
    } else if (type == 'R') {
        draw_rectangle(c, x1, y1, x2, y2);
        printf("Rectangle added!\n");
    } else if (type == 'C') {
        int radius = abs(x2 - x1);
        draw_circle(c, x1, y1, radius);
        printf("Circle added!\n");
    }
}

void list_shapes(Canvas* c) {
    printf("\n--- Shapes ---\n");
    if (c->total_shapes == 0) {
        printf("None\n");
        return;
    }
    for (int i = 0; i < c->total_shapes; i++) {
        printf("%d. %c: (%d,%d) to (%d,%d)\n", i, c->shapes[i].type,
               c->shapes[i].x1, c->shapes[i].y1, c->shapes[i].x2, c->shapes[i].y2);
    }
    printf("-----\n\n");
}

void delete_shape(Canvas* c, int idx) {
    if (idx < 0 || idx >= c->total_shapes) {
        printf("Invalid!\n");
        return;
    }
    
    for (int i = idx; i < c->total_shapes - 1; i++) {
        c->shapes[i] = c->shapes[i + 1];
    }
    c->total_shapes--;
    
    clear_canvas(c);
    for (int i = 0; i < c->total_shapes; i++) {
        add_shape(c, c->shapes[i].x1, c->shapes[i].y1, 
                 c->shapes[i].x2, c->shapes[i].y2, c->shapes[i].type);
    }
    printf("Deleted!\n");
}

void show_menu() {
    printf("\n=== 2D Graphics ===\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. List\n");
    printf("6. Delete\n");
    printf("7. Clear\n");
    printf("8. Display\n");
    printf("9. Exit\n");
    printf("===================\n");
    printf("Choice: ");
}

void run_program() {
    Canvas* c = create_canvas();
    int choice, x1, y1, x2, y2, x3, y3, idx;
    
    while (1) {
        show_menu();
        scanf("%d", &choice);
        getchar();
        
        switch(choice) {
            case 1:
                printf("Line: x1 y1 x2 y2: ");
                scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
                getchar();
                add_shape(c, x1, y1, x2, y2, 'L');
                break;
            case 2:
                printf("Rectangle: x1 y1 x2 y2: ");
                scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
                getchar();
                add_shape(c, x1, y1, x2, y2, 'R');
                break;
            case 3:
                printf("Circle: cx cy radius: ");
                scanf("%d %d %d", &x1, &y1, &x2);
                getchar();
                add_shape(c, x1, y1, x1 + x2, y1, 'C');
                break;
            case 4:
                printf("Triangle: x1 y1 x2 y2 x3 y3: ");
                scanf("%d %d %d %d %d %d", &x1, &y1, &x2, &y2, &x3, &y3);
                getchar();
                draw_triangle(c, x1, y1, x2, y2, x3, y3);
                printf("Triangle added!\n");
                break;
            case 5:
                list_shapes(c);
                break;
            case 6:
                list_shapes(c);
                printf("Delete: ");
                scanf("%d", &idx);
                getchar();
                delete_shape(c, idx);
                break;
            case 7:
                clear_canvas(c);
                printf("Cleared!\n");
                break;
            case 8:
                display_canvas(c);
                break;
            case 9:
                free_canvas(c);
                printf("Goodbye!\n");
                return;
            default:
                printf("Invalid!\n");
        }
    }
}

int main() {
    run_program();
    return 0;
}