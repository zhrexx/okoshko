#include <stdio.h>

#include "okoshko.h"
#include "assets/font.h"

#include <stdlib.h>

int main() {
    srand(time(NULL));

    oko_Font *f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_Window *win = oko_create("Example", 800, 600);
    oko_set_fps(win, 10000);

    while (oko_is_running(win))
    {
        oko_begin_drawing(win);
        oko_clear(win, 0x00);
        oko_end_drawing(win);
        log_info(oko_format("FPS: %0u", oko_get_fps(win)));
    }

    oko_destroy(win);
    return 0;
}