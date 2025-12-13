#include <stdio.h>

#include "assets/font.h"
#include "okoshko.h"
#define OKO_BUTTON_IMPLEMENTATION
#define OKO_INPUT_IMPLEMENTATION
#include <math.h>

#include "modules/button.h"
#include "modules/input.h"

#include <stdlib.h>

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

int main() {
    srand(time(NULL));

    oko_Font* f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_temp_allocator* ta = oko_get_temp_allocator();
    oko_Window* win = oko_create("Example", 800, 600);
    width = &win->width;
    height = &win->height;
    oko_set_fps(win, 60);
    #define N (128)

    struct Ball balls[N] = {0};
    init_balls(balls, N);

    while (oko_is_running(win))
    {
        width = &win->width;
        height = &win->height;
        const u32 fps = oko_get_fps(win);
        oko_begin_drawing(win);
        i32 red = 0;
        i32 blue = 0;

        oko_clear(win, 0x00);


        // Draw
        for (int i = 0; i < N; i++)
        {
            struct Ball *b = &balls[i];
            if (b->hits == 0) continue;
            if (b->color == 0xFF0000FF) blue++;
            else red++;
            oko_fill_circle(win, b->x, b->y, RADIUS, b->color);
            b->x += (i32)b->vx;
            b->y += (i32)b->vy;
            if (b->x - RADIUS <= 0 || b->x + RADIUS >= win->width) {
                b->vx = -(f32)(b->vx*1.005);
            }
            if (b->y - RADIUS <= 0 || b->y + RADIUS >= win->height) {
                b->vy = -(f32)(b->vy*1.005);
            }
        }

        // Check for collision with other balls
        for (int i = 0; i < N; i++)
        {
            struct Ball *b1 = &balls[i];
            if (b1->hits == 0) continue;
            if (b1->cooldown > 0) b1->cooldown--;

            for (int j = i + 1; j < N; j++)
            {
                struct Ball *b2 = &balls[j];
                if (b2->hits == 0) continue;
                if (b2->cooldown > 0) b2->cooldown--;

                f32 dx = (f32)b2->x - (f32)b1->x;
                f32 dy = (f32)b2->y - (f32)b1->y;
                f32 dist = sqrtf(dx * dx + dy * dy);

                if (dist < 2 * RADIUS && dist > 0)
                {
                    f32 nx = dx / dist;
                    f32 ny = dy / dist;

                    f32 overlap = 2 * RADIUS - dist;
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

        oko_draw_text(win, oko_format("FPS: %llu ; B = %d ; R = %d", fps, blue, red), f, 20, 20,
                      1.5f, 0xFFFFFFFF);
        oko_end_drawing(win);
        oko_temp_reset(ta);
    }

    oko_destroy(win);
    return 0;
}
