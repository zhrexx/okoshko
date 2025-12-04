#pragma once

#include <ctype.h>
#include <string.h>

// TODO: implement special chars and , .
// NOTE: each shift+num char

#include "../okoshko.h"

#define OKO_INPUT_MAX_LENGTH 256

typedef enum { OKO_INPUT_IDLE, OKO_INPUT_FOCUSED } oko_InputState;

typedef struct {
    oko_Rect bounds;
    char text[OKO_INPUT_MAX_LENGTH];
    i32 text_length;
    i32 cursor_pos;
    oko_Font* font;
    float text_scale;

    u32 color_idle;
    u32 color_focused;
    u32 border_color;
    u32 text_color;
    u32 cursor_color;

    oko_InputState state;
    u8 was_clicked_outside;

    u64 last_blink_time;
    u8 cursor_visible;

    const char* placeholder;
    u32 placeholder_color;

    u8 last_key_state[256];
} oko_Input;

static oko_Input oko_input_create(i32 x, i32 y, i32 w, i32 h, oko_Font* font);
static void oko_input_update(oko_Window* win, oko_Input* input);
static void oko_input_draw(oko_Window* win, oko_Input* input);
static void oko_input_set_text(oko_Input* input, const char* text);
static void oko_input_clear(oko_Input* input);

#ifdef OKO_INPUT_IMPLEMENTATION

static oko_Input oko_input_create(i32 x, i32 y, i32 w, i32 h, oko_Font* font) {
    oko_Input input = {0};
    input.bounds = (oko_Rect){x, y, w, h};
    input.text[0] = '\0';
    input.text_length = 0;
    input.cursor_pos = 0;
    input.font = font;
    input.text_scale = 2.0f;

    input.color_idle = 0xFF303030;
    input.color_focused = 0xFF404040;
    input.border_color = 0xFF808080;
    input.text_color = 0xFFFFFFFF;
    input.cursor_color = 0xFFFFFFFF;

    input.state = OKO_INPUT_IDLE;
    input.was_clicked_outside = 0;
    input.last_blink_time = 0;
    input.cursor_visible = 1;

    input.placeholder = NULL;
    input.placeholder_color = 0xFF808080;

    memset(input.last_key_state, 0, 256);

    return input;
}

static u8 oko_input_key_pressed(oko_Window* win, oko_Input* input, u8 key) {
    u8 current = oko_key_down(win, key);
    u8 pressed = current && !input->last_key_state[key];
    input->last_key_state[key] = current;
    return pressed;
}

static void oko_input_update(oko_Window* win, oko_Input* input) {
    i32 mx = win->mouse.x;
    i32 my = win->mouse.y;

    u8 inside =
    (mx >= input->bounds.x && mx < input->bounds.x + input->bounds.w &&
        my >= input->bounds.y && my < input->bounds.y + input->bounds.h);

    if (win->mouse.left)
    {
        if (inside)
        {
            input->state = OKO_INPUT_FOCUSED;
            input->was_clicked_outside = 0;

            i32 text_x = input->bounds.x + 10;
            i32 click_offset = mx - text_x;

            if (click_offset <= 0)
            {
                input->cursor_pos = 0;
            }
            else
            {
                i32 accumulated_width = 0;
                i32 best_pos = input->text_length;

                for (i32 i = 0; i < input->text_length; i++)
                {
                    oko_Glyph* glyph = NULL;
                    for (i32 j = 0; j < input->font->glyphCount; j++)
                    {
                        if (input->font->glyphs[j].character == input->text[i])
                        {
                            glyph = &input->font->glyphs[j];
                            break;
                        }
                    }

                    i32 char_width =
                        glyph
                            ? (i32)(glyph->advance * input->text_scale)
                            : (i32)(input->font->size * 0.5f * input->text_scale);

                    if (click_offset < accumulated_width + char_width / 2)
                    {
                        best_pos = i;
                        break;
                    }
                    accumulated_width += char_width;
                }

                input->cursor_pos = best_pos;
            }
        }
        else if (!input->was_clicked_outside)
        {
            input->state = OKO_INPUT_IDLE;
            input->was_clicked_outside = 1;
        }
    }
    else
    {
        input->was_clicked_outside = 0;
    }

    if (input->state == OKO_INPUT_FOCUSED)
    {
        u64 current_time = oko_time_ms(win);
        if (current_time - input->last_blink_time > 500)
        {
            input->cursor_visible = !input->cursor_visible;
            input->last_blink_time = current_time;
        }

        if (oko_input_key_pressed(win, input, OKO_KEY_BACKSPACE))
        {
            if (input->cursor_pos > 0)
            {
                for (i32 i = input->cursor_pos - 1; i < input->text_length; i++)
                {
                    input->text[i] = input->text[i + 1];
                }
                input->cursor_pos--;
                input->text_length--;
                input->cursor_visible = 1;
                input->last_blink_time = current_time;
            }
        }

        if (oko_input_key_pressed(win, input, OKO_KEY_DELETE))
        {
            if (input->cursor_pos < input->text_length)
            {
                for (i32 i = input->cursor_pos; i < input->text_length; i++)
                {
                    input->text[i] = input->text[i + 1];
                }
                input->text_length--;
                input->cursor_visible = 1;
                input->last_blink_time = current_time;
            }
        }

        if (oko_input_key_pressed(win, input, OKO_KEY_ARROW_LEFT))
        {
            if (input->cursor_pos > 0)
            {
                input->cursor_pos--;
                input->cursor_visible = 1;
                input->last_blink_time = current_time;
            }
        }
        if (oko_input_key_pressed(win, input, OKO_KEY_ARROW_RIGHT))
        {
            if (input->cursor_pos < input->text_length)
            {
                input->cursor_pos++;
                input->cursor_visible = 1;
                input->last_blink_time = current_time;
            }
        }

        if (oko_input_key_pressed(win, input, OKO_KEY_HOME))
        {
            input->cursor_pos = 0;
            input->cursor_visible = 1;
            input->last_blink_time = current_time;
        }
        if (oko_input_key_pressed(win, input, OKO_KEY_END))
        {
            input->cursor_pos = input->text_length;
            input->cursor_visible = 1;
            input->last_blink_time = current_time;
        }

        for (i32 key = 32; key < 127; key++)
        {
            if (oko_input_key_pressed(win, input, key))
            {
                if (input->text_length < OKO_INPUT_MAX_LENGTH - 1)
                {
                    for (i32 i = input->text_length; i > input->cursor_pos; i--)
                    {
                        input->text[i] = input->text[i - 1];
                    }

                    char c = (char)key;

                    if (win->keyboard.shift)
                    {
                        if (c >= 'a' && c <= 'z')
                        {
                            c = c - 32;
                        }
                        else
                        {
                            const char* shift_map = "!@#$%^&*()_+{}|:\"<>?";
                            const char* normal_map = "1234567890-=[]\\;',./";
                            for (i32 i = 0; normal_map[i]; i++)
                            {
                                if (c == normal_map[i])
                                {
                                    c = shift_map[i];
                                    break;
                                }
                            }
                        }
                    }

                    input->text[input->cursor_pos] = c;
                    input->cursor_pos++;
                    input->text_length++;
                    input->text[input->text_length] = '\0';
                    input->cursor_visible = 1;
                    input->last_blink_time = current_time;
                }
            }
        }
    }
    else
    {
        for (i32 key = 0; key < 256; key++)
        {
            input->last_key_state[key] = oko_key_down(win, key);
        }
    }
}

static void oko_input_draw(oko_Window* win, oko_Input* input) {
    u32 bg_color = (input->state == OKO_INPUT_FOCUSED)
                       ? input->color_focused
                       : input->color_idle;

    oko_fill_rect(win, input->bounds, bg_color);

    if (input->state == OKO_INPUT_FOCUSED)
    {
        oko_draw_rect(win, input->bounds, 0xFFFFFFFF);
        oko_Rect inner = {
            input->bounds.x + 1, input->bounds.y + 1,
            input->bounds.w - 2, input->bounds.h - 2
        };
        oko_draw_rect(win, inner, 0xFFFFFFFF);
    }
    else
    {
        oko_draw_rect(win, input->bounds, input->border_color);
    }

    i32 text_x = input->bounds.x + 10;
    i32 text_y = input->bounds.y + (input->bounds.h - input->font->size) / 2;

    if (input->text_length == 0 && input->placeholder &&
        input->state == OKO_INPUT_IDLE)
    {
        oko_draw_text(win, input->placeholder, input->font, text_x, text_y,
                      input->text_scale, input->placeholder_color);
    }
    else if (input->text_length > 0)
    {
        oko_draw_text(win, input->text, input->font, text_x, text_y,
                      input->text_scale, input->text_color);
    }

    if (input->state == OKO_INPUT_FOCUSED && input->cursor_visible)
    {
        i32 cursor_x = text_x;

        for (i32 i = 0; i < input->cursor_pos; i++)
        {
            oko_Glyph* glyph = NULL;
            for (i32 j = 0; j < input->font->glyphCount; j++)
            {
                if (input->font->glyphs[j].character == input->text[i])
                {
                    glyph = &input->font->glyphs[j];
                    break;
                }
            }
            cursor_x += glyph
                            ? (i32)(glyph->advance * input->text_scale)
                            : (i32)(input->font->size * 0.5f * input->text_scale);
        }

        i32 cursor_height = (i32)(input->font->size * input->text_scale);
        oko_draw_line(win, cursor_x, text_y, cursor_x, text_y + cursor_height,
                      input->cursor_color);
    }
}

static void oko_input_set_text(oko_Input* input, const char* text) {
    if (!text)
    {
        input->text[0] = '\0';
        input->text_length = 0;
        input->cursor_pos = 0;
        return;
    }

    i32 len = strlen(text);
    if (len >= OKO_INPUT_MAX_LENGTH)
    {
        len = OKO_INPUT_MAX_LENGTH - 1;
    }

    memcpy(input->text, text, len);
    input->text[len] = '\0';
    input->text_length = len;
    input->cursor_pos = len;
}

static void oko_input_clear(oko_Input* input) {
    input->text[0] = '\0';
    input->text_length = 0;
    input->cursor_pos = 0;
}

#endif // OKO_INPUT_IMPLEMENTATION
