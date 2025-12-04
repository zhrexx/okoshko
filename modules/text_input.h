// TI = Text Input
#pragma once

// TODO: implement special chars and , .

#include <ctype.h>
#include <string.h>

#include "../okoshko.h"

#ifndef OKO_TI_MAX_LENGTH
#define OKO_TI_MAX_LENGTH 256
#endif

typedef struct {
    u8 buffer[OKO_TI_MAX_LENGTH];
    i32 length;
    u8 last_key_state[256];
} oko_TI;

oko_TI ti_init();
void text_input_update(oko_TI* input, oko_Window* win);

#ifdef MODULE_TI_IMPLEMENTATION

oko_TI ti_init() {
    oko_TI in;
    memset(in.buffer, 0, OKO_TI_MAX_LENGTH);
    in.length = 0;
    memset(in.last_key_state, 0, 256);
    return in;
}

void ti_update(oko_TI* input, oko_Window* win) {
    for (char c = 'a'; c <= 'z'; c++)
    {
        if (oko_key_down(win, c) && !input->last_key_state[c])
        {
            if (input->length < OKO_TI_MAX_LENGTH - 1)
            {
                input->buffer[input->length++] =
                    win->keyboard.shift ? (u8)toupper(c) : (u8)c;
            }
        }
        input->last_key_state[c] = oko_key_down(win, c);
    }

    for (char c = '0'; c <= '9'; c++)
    {
        if (oko_key_down(win, c) && !input->last_key_state[c])
        {
            if (input->length < OKO_TI_MAX_LENGTH - 1)
            {
                input->buffer[input->length++] = c;
            }
        }
        input->last_key_state[c] = oko_key_down(win, c);
    }

    if (oko_key_down(win, OKO_KEY_SPACE) &&
        !input->last_key_state[OKO_KEY_SPACE])
    {
        if (input->length < OKO_TI_MAX_LENGTH - 1)
        {
            input->buffer[input->length++] = ' ';
        }
    }
    input->last_key_state[OKO_KEY_SPACE] = oko_key_down(win, OKO_KEY_SPACE);

    if (oko_key_down(win, OKO_KEY_BACKSPACE) &&
        !input->last_key_state[OKO_KEY_BACKSPACE])
    {
        if (input->length > 0)
        {
            input->length--;
            input->buffer[input->length] = '\0';
        }
    }
    input->last_key_state[OKO_KEY_BACKSPACE] =
        oko_key_down(win, OKO_KEY_BACKSPACE);
}
#endif
