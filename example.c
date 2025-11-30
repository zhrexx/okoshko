#include <stdio.h>

#include "okoshko.h"

#define MODULE_TI_IMPLEMENTATION
#include "modules/text_input.h"

int main() {
    oko_Window *win = oko_create("Example", 800, 600);
    oko_set_fps(win, 60);
    oko_TI in = ti_init();

    while (oko_is_running(win))
    {
        oko_begin_drawing(win);
        oko_clear(win, 0x00);
        ti_update(&in, win);

        printf("Input: %s\n", in.buffer);

        oko_end_drawing(win);
    }

    oko_destroy(win);
    return 0;
}