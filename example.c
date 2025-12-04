#include <stdio.h>

#include "assets/font.h"
#include "okoshko.h"
#define OKO_BUTTON_IMPLEMENTATION
#define OKO_INPUT_IMPLEMENTATION
#include "modules/button.h"
#include "modules/input.h"

#include <stdlib.h>

const i32 WIDTH = 800;
const i32 HEIGHT = 600;

int main() {
    srand(time(NULL));

    oko_Font* f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_temp_allocator* ta = oko_get_temp_allocator();
    oko_Window* win = oko_create("Example", WIDTH, HEIGHT);
    oko_set_fps(win, 120);

    oko_Button btn1 =
        oko_button_create(300, 250, 200, 50, "Click to start Timer", f);
    oko_Input input = oko_input_create(250, 200, 300, 40, f);
    input.placeholder = "Seconds..";

    f32 show_text = 0;

    u64 i = 0;
    while (oko_is_running(win))
    {
        oko_begin_drawing(win);

        oko_clear(win, 0x00);

        if (show_text >= 0)
        {
            oko_draw_text(win,
                          oko_format("Time left: %.1f", show_text / oko_get_fps(win)),
                          f, 315, 175, 1.5f, 0xFFFFFFFF);
            show_text -= 1;
        }

        // Update
        oko_input_update(win, &input);

        if (oko_button_update(win, &btn1))
        {
            show_text = (f32)oko_get_fps(win) * (f32)atof(input.text);
        }

        // Draw
        oko_input_draw(win, &input);
        oko_button_draw(win, &btn1);

        oko_draw_text(win, oko_format("FPS: %llu", oko_get_fps(win)), f, 20, 20,
                      1.5f, 0xFFFFFFFF);
        oko_end_drawing(win);
        oko_temp_reset(ta);
    }

    oko_destroy(win);
    return 0;
}
