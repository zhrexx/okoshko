#include <stdio.h>

#include "okoshko.h"
#include "assets/font.h"
#define OKO_BUTTON_IMPLEMENTATION
#include "modules/button.h"

#include <stdlib.h>

const i32 WIDTH = 800;
const i32 HEIGHT = 600;

int main() {
    srand(time(NULL));

    oko_Font *f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_Window *win = oko_create("Example", WIDTH, HEIGHT);
    oko_set_fps(win, 100000);

    oko_Button btn1 = oko_button_create(300, 250, 200, 50, "Click Me!", f);

    u64 i = 0;
    while (oko_is_running(win))
    {

        if (oko_button_update(win, &btn1))
        {
            log_info("Button was pressed");
        }

        oko_begin_drawing(win);

        oko_clear(win, 0x00);

        oko_button_draw(win, &btn1);

        oko_draw_text(win, oko_format("FPS: %llu", oko_get_fps(win)), f, 20, 20, 1.5f, 0xFFFFFFFF);
        oko_end_drawing(win);
    }

    oko_destroy(win);
    return 0;
}