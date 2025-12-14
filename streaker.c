#include <stdio.h>

#include "assets/font.h"
#include "okoshko.h"
#define OKO_BUTTON_IMPLEMENTATION
#define OKO_INPUT_IMPLEMENTATION
#include <math.h>

#include "modules/button.h"
#include "modules/input.h"

#include <stdlib.h>

oko_Input task_input;

typedef struct {
    const char *goal; 
    u64 timestamp; // start
} Streak; 

static Streak streaks[256]; // TODO: maybe extend
static u64 streak_count = 0;

void save_streaks() {
    FILE *f = fopen("streaks.dat", "wb");
    if (!f) {
        printf("Failed to open streaks.dat for writing\n");
        return;
    }
    for (u64 i = 0; i < streak_count; i++) {
        u64 len = strlen(streaks[i].goal) + 1;
        fwrite(&len, sizeof(u64), 1, f);
        fwrite(streaks[i].goal, sizeof(char), len, f);
        
        fwrite(&streaks[i].timestamp, sizeof(u64), 1, f);
    }

    fclose(f);
}

void load_streaks() {
    FILE *f = fopen("streaks.dat", "rb");
    if (!f) {
        printf("No streaks.dat found, starting fresh\n");
        return;
    }
    streak_count = 0;
    while (!feof(f) && streak_count < 256) {
        u64 len = 0;
        if (fread(&len, sizeof(u64), 1, f) != 1) break;
        
        char *goal = (char *)malloc(len);
        if (fread(goal, sizeof(char), len, f) != len) {
            free(goal);
            break;
        }

        u64 timestamp = 0;
        if (fread(&timestamp, sizeof(u64), 1, f) != 1) {
            free(goal);
            break;
        }

        streaks[streak_count].goal = goal;
        streaks[streak_count].timestamp = timestamp;
        streak_count++;
    }

    fclose(f);
}

void draw(oko_Window* win, oko_Font *f) {
        oko_clear(win, 0xFF202020);
        oko_draw_text(win, "Streaker :)", f, 20, 20,
                        2.0f, 0xFFFFFFFF);
        
        oko_draw_text(win, "ENTER. Add new Goal", f, 20, 50,
                        2.0f, 0xFFFFFFFF);
        oko_draw_text(win, "CTRL+ENTER. Remove Goal", f, 20, 80,
                        2.0f, 0xFFFFFFFF);
        oko_draw_text(win, "q. Exit", f, 20, 110,
                        2.0f, 0xFFFFFFFF);
        oko_draw_text(win, "================", f, 20, 140,
                        2.0f, 0xFFFFFFFF);

        oko_input_draw(win, &task_input);

        oko_draw_text(win, "================", f, 20, 205,
                        2.0f, 0xFFFFFFFF);


        for (u64 i = 0; i < streak_count; i++) {
            u64 elapsed = (oko_time_ms(win) - streaks[i].timestamp) / 1000;

            u64 days    = elapsed / 86400;
            elapsed    %= 86400;
            u64 hours   = elapsed / 3600;
            elapsed    %= 3600;
            u64 minutes = elapsed / 60;
            u64 seconds = elapsed % 60;

            char *buffer = oko_format("%llu. %s - %llu days, %llu:%llu:%llu",
                                        i + 1, streaks[i].goal, days, hours, minutes, seconds);
            oko_draw_text(win, buffer, f, 20, 230 + (i * 22),
                            1.5f, 0xFFFFFFFF);
        }
        
}

void update(oko_Window* win) {
    oko_input_update(win, &task_input);
    
    if (oko_key_pressed(win, OKO_KEY_ENTER) && win->keyboard.ctrl) {
        if (task_input.text_length > 0 && streak_count > 0) {
            int index = atoi(task_input.text);
            if (index > 0 && (u64)index <= streak_count) {
                index--;
                free((char *)streaks[index].goal);
                for (u64 i = index; i < streak_count - 1; i++) {
                    streaks[i] = streaks[i + 1];
                }
                streak_count--;
            }

            task_input.text[0] = '\0';
            task_input.text_length = 0;
            task_input.cursor_pos = 0;

        } else if (task_input.text_length == 0 && streak_count > 0) {
            streak_count--;
        }
    } else if (oko_key_pressed(win, 'q')) {
        win->running = 0;
    }

    if (oko_key_pressed(win, OKO_KEY_ENTER)) {
        if (task_input.text_length > 0 && streak_count < 256) {
            char *new_goal = (char *)malloc(task_input.text_length + 1);
            memcpy(new_goal, task_input.text, task_input.text_length);
            new_goal[task_input.text_length] = '\0';
            streaks[streak_count].goal = new_goal;
            streaks[streak_count].timestamp = oko_time_ms(win);
            streak_count++;

            task_input.text[0] = '\0';
            task_input.text_length = 0;
            task_input.cursor_pos = 0;
        }
    }
}

int main() {
    srand(time(NULL));
    oko_Font* f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_temp_allocator* ta = oko_get_temp_allocator();
    oko_Window* win = oko_create("Streaker =)", 800, 600);
    oko_set_fps(win, 60);
    task_input = oko_input_create(20, 160, 400, 40, f);
    task_input.placeholder = "Enter your goal...";

    load_streaks();
    while (oko_is_running(win))
    {
        const u32 fps = oko_get_fps(win);

        oko_begin_frame(win);
        update(win);

        draw(win, f);

        oko_end_frame(win);   
        oko_temp_reset(ta);
    }

    save_streaks();
    for (u64 i = 0; i < streak_count; i++) {
        free((char *)streaks[i].goal);
    }
    oko_destroy(win);
    return 0;
}
