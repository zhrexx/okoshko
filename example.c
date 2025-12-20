#include <stdio.h>

#include "assets/font.h"
#include "okoshko.h"
#define OKO_BUTTON_IMPLEMENTATION
#define OKO_INPUT_IMPLEMENTATION
#include <math.h>

#include "modules/button.h"
#include "modules/input.h"

#include <stdlib.h>
#include <string.h>

f32 VELOCITY = 32;
i32 RADIUS = 12;
u8 LIVES = 32;

i32* width;
i32* height;

struct Ball {
    f32 vx, vy;
    i32 x, y;
    u64 color;
    u8 hits;
    u8 cooldown;
};

#define GRID_CELL_SIZE 50
#define MAX_BALLS_PER_CELL 64

typedef struct {
    i32 count;
    i32 indices[MAX_BALLS_PER_CELL];
} GridCell;

typedef struct {
    i32 cols, rows;
    GridCell* cells;
} SpatialGrid;

SpatialGrid grid;

void grid_init(SpatialGrid* g, i32 w, i32 h) {
    g->cols = (w / GRID_CELL_SIZE) + 1;
    g->rows = (h / GRID_CELL_SIZE) + 1;
    g->cells = (GridCell*)calloc(g->cols * g->rows, sizeof(GridCell));
}

void grid_clear(SpatialGrid* g) {
    memset(g->cells, 0, g->cols * g->rows * sizeof(GridCell));
}

void grid_insert(SpatialGrid* g, i32 ball_idx, i32 x, i32 y) {
    i32 gx = x / GRID_CELL_SIZE;
    i32 gy = y / GRID_CELL_SIZE;

    if (gx < 0) gx = 0;
    if (gy < 0) gy = 0;
    if (gx >= g->cols) gx = g->cols - 1;
    if (gy >= g->rows) gy = g->rows - 1;

    GridCell* cell = &g->cells[gy * g->cols + gx];
    if (cell->count < MAX_BALLS_PER_CELL) {
        cell->indices[cell->count++] = ball_idx;
    }
}

f32 randf() {
    return (f32)rand() / (f32)RAND_MAX;
}

void init_balls(struct Ball *balls, const int N) {
    for (int i = 0; i < N; i++)
    {
        f32 angle = randf() * 3.0f * 3.14159f;

        f32 vx = cos(angle) * VELOCITY;
        f32 vy = sin(angle) * VELOCITY;

        i32 x = RADIUS + (rand() % (*width - 2 * RADIUS));
        i32 y = RADIUS + (rand() % (*height - 2 * RADIUS));

        u64 color = (i < N/2) ? 0xFF0000FF : 0xFFFF0000;

        balls[i] = (struct Ball){vx, vy, x, y, color, LIVES, 0};
    }
}

static inline f32 fast_inv_sqrt(f32 x) {
    f32 xhalf = 0.5f * x;
    i32 i = *(i32*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(f32*)&i;
    x = x * (1.5f - xhalf * x * x);
    return x;
}

int main() {
    srand(time(NULL));

    oko_Font* f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_temp_allocator* ta = oko_get_temp_allocator();
    oko_Window* win = oko_create("Example", 1600, 1200);
    width = &win->width;
    height = &win->height;
    oko_set_fps(win, 0);
    #define N (4196)


    struct Ball balls[N] = {0};
    init_balls(balls, N);

    grid_init(&grid, *width, *height);

    u64 collision_checks = 0;

    while (oko_is_running(win))
    {
        OKO_PROFILE_START(aaaa);
        width = &win->width;
        height = &win->height;
        const u32 fps = oko_get_fps(win);
        oko_begin_frame(win);
        i32 red = 0;
        i32 blue = 0;
        collision_checks = 0;

        oko_clear(win, 0x00);

        grid_clear(&grid);

        for (int i = 0; i < N; i++)
        {
            struct Ball *b = &balls[i];
            if (b->hits == 0) continue;

            b->x += (i32)b->vx;
            b->y += (i32)b->vy;

            if (b->x - RADIUS <= 0 || b->x + RADIUS >= win->width) {
                b->vx = -(f32)(b->vx*1.005);
            }
            if (b->y - RADIUS <= 0 || b->y + RADIUS >= win->height) {
                b->vy = -(f32)(b->vy*1.005);
            }

            grid_insert(&grid, i, b->x, b->y);
        }

        for (int i = 0; i < N; i++)
        {
            struct Ball *b1 = &balls[i];
            if (b1->hits == 0) continue;
            if (b1->cooldown > 0) b1->cooldown--;

            i32 gx = b1->x / GRID_CELL_SIZE;
            i32 gy = b1->y / GRID_CELL_SIZE;

            for (i32 dy = -1; dy <= 1; dy++) {
                for (i32 dx = -1; dx <= 1; dx++) {
                    i32 cx = gx + dx;
                    i32 cy = gy + dy;

                    if (cx < 0 || cy < 0 || cx >= grid.cols || cy >= grid.rows)
                        continue;

                    GridCell* cell = &grid.cells[cy * grid.cols + cx];

                    for (i32 k = 0; k < cell->count; k++) {
                        i32 j = cell->indices[k];

                        if (j <= i) continue;

                        struct Ball *b2 = &balls[j];
                        if (b2->hits == 0) continue;

                        collision_checks++;

                        f32 dx_f = (f32)b2->x - (f32)b1->x;
                        f32 dy_f = (f32)b2->y - (f32)b1->y;
                        f32 dist_sq = dx_f * dx_f + dy_f * dy_f;
                        f32 min_dist = 2 * RADIUS;
                        f32 min_dist_sq = min_dist * min_dist;

                        if (dist_sq < min_dist_sq && dist_sq > 0.1f)
                        {
                            f32 dist = sqrtf(dist_sq);
                            f32 nx = dx_f / dist;
                            f32 ny = dy_f / dist;

                            f32 overlap = min_dist - dist;
                            b1->x -= (i32)(overlap * nx / 2);
                            b1->y -= (i32)(overlap * ny / 2);
                            b2->x += (i32)(overlap * nx / 2);
                            b2->y += (i32)(overlap * ny / 2);

                            if (b1->x < RADIUS) b1->x = RADIUS;
                            if (b1->x > win->width - RADIUS) b1->x = win->width - RADIUS;
                            if (b1->y < RADIUS) b1->y = RADIUS;
                            if (b1->y > win->height - RADIUS) b1->y = win->height - RADIUS;

                            if (b2->x < RADIUS) b2->x = RADIUS;
                            if (b2->x > win->width - RADIUS) b2->x = win->width - RADIUS;
                            if (b2->y < RADIUS) b2->y = RADIUS;
                            if (b2->y > win->height - RADIUS) b2->y = win->height - RADIUS;

                            f32 v1n = b1->vx * nx + b1->vy * ny;
                            f32 v2n = b2->vx * nx + b2->vy * ny;

                            b1->vx += (v2n - v1n) * nx;
                            b1->vy += (v2n - v1n) * ny;
                            b2->vx += (v1n - v2n) * nx;
                            b2->vy += (v1n - v2n) * ny;

                            const i32 chance = rand() % 100;
                            bool is_in_team = chance%7==0;

                            if (b1->cooldown == 0 && b2->cooldown == 0 && is_in_team) {
                                if (chance%2==0) b1->hits--;
                                if (chance%3==0) b2->hits--;
                                b1->cooldown = 5;
                                b2->cooldown = 5;
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < N; i++)
        {
            struct Ball *b = &balls[i];
            if (b->hits == 0) continue;
            if (b->color == 0xFF0000FF) blue++;
            else red++;
            oko_fill_circle(win, b->x, b->y, RADIUS, b->color);
        }

        if (oko_key_pressed(win, 'r'))
        {
            RADIUS = 12;
            init_balls(balls, N);
        }

        if (oko_key_pressed(win, 'p'))
        {
            RADIUS++;
        }

        if (oko_key_pressed(win, 'm'))
        {
            RADIUS--;
        }

        oko_draw_text(win,
            oko_format("FPS: %u | B: %d | R: %d | Checks: %llu",
                fps, blue, red, collision_checks),
            f, 20, 20, 1.5f, 0xFFFFFFFF);

        oko_end_frame(win);
        oko_temp_reset(ta);
        OKO_PROFILE_END(aaaa);
    }

    free(grid.cells);
    oko_destroy(win);
    return 0;
}