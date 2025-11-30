#include <stdio.h>

#include "okoshko.h"

int main() {
    oko_Window *okoshko = oko_create("Hello, world!", 800, 600);
    oko_set_fps(okoshko, 60);

    u64 x = 0, y = 0;
    u64 w = 100, h = 100;

    while (oko_is_running(okoshko))
    {
        if (oko_key_pressed(okoshko, 'Q'))
        {
            break;
        }
        oko_begin_drawing(okoshko);
        oko_clear(okoshko, 0x00);
        oko_fill_rect(okoshko, OKO_RECT(x, y, w, h), 0xFFFF0000);
        x += 1;
        y += 1;
        oko_end_drawing(okoshko);
    }

    oko_destroy(okoshko);
    return 0;
}