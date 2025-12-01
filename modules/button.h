#pragma once

// TODO: implement special chars and , .

#include <ctype.h>
#include <string.h>

#include "../okoshko.h"


typedef enum {
    OKO_BUTTON_IDLE,
    OKO_BUTTON_HOVER,
    OKO_BUTTON_PRESSED
} oko_ButtonState;

typedef struct {
    oko_Rect bounds;
    const char *text;
    oko_Font *font;
    float text_scale;

    u32 color_idle;
    u32 color_hover;
    u32 color_pressed;
    u32 text_color;

    oko_ButtonState state;
    u8 was_pressed;
} oko_Button;

static oko_Button oko_button_create(i32 x, i32 y, i32 w, i32 h, const char *text, oko_Font *font);
static u8 oko_button_update(oko_Window *win, oko_Button *btn);

static void oko_button_draw(oko_Window *win, oko_Button *btn);

#ifdef OKO_BUTTON_IMPLEMENTATION
static oko_Button oko_button_create(i32 x, i32 y, i32 w, i32 h, const char *text, oko_Font *font) {
    oko_Button btn = {0};
    btn.bounds = (oko_Rect){x, y, w, h};
    btn.text = text;
    btn.font = font;
    btn.text_scale = 1.0f;

    btn.color_idle = 0xFF404040;
    btn.color_hover = 0xFF606060;
    btn.color_pressed = 0xFF202020;
    btn.text_color = 0xFFFFFFFF;

    btn.state = OKO_BUTTON_IDLE;
    btn.was_pressed = 0;

    return btn;
}

static u8 oko_button_update(oko_Window *win, oko_Button *btn) {
    i32 mx = win->mouse.x;
    i32 my = win->mouse.y;

    u8 inside = (mx >= btn->bounds.x && mx < btn->bounds.x + btn->bounds.w &&
                 my >= btn->bounds.y && my < btn->bounds.y + btn->bounds.h);

    u8 clicked = 0;

    if (inside) {
        if (win->mouse.left) {
            btn->state = OKO_BUTTON_PRESSED;
            btn->was_pressed = 1;
        } else {
            if (btn->was_pressed) {
                clicked = 1;
            }
            btn->state = OKO_BUTTON_HOVER;
            btn->was_pressed = 0;
        }
    } else {
        btn->state = OKO_BUTTON_IDLE;
        btn->was_pressed = 0;
    }

    return clicked;
}

static void oko_button_draw(oko_Window *win, oko_Button *btn) {
    u32 color;
    switch (btn->state) {
    case OKO_BUTTON_HOVER:   color = btn->color_hover; break;
    case OKO_BUTTON_PRESSED: color = btn->color_pressed; break;
    default:                 color = btn->color_idle; break;
    }

    oko_fill_rect(win, btn->bounds, color);

    oko_draw_rect(win, btn->bounds, 0xFFAAAAAA);

    if (btn->text && btn->font) {
        i32 text_x = btn->bounds.x + 10;
        i32 text_y = btn->bounds.y + (btn->bounds.h - btn->font->size) / 2;
        oko_draw_text(win, btn->text, btn->font, text_x, text_y, btn->text_scale, btn->text_color);
    }
}
#endif