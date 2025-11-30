#include <stdio.h>

#define TTF_IMPLEMENTATION
#include "okoshko.h"
#include "assets/font.h"

#define MODULE_TI_IMPLEMENTATION
#include <stdlib.h>

#include "modules/text_input.h"

OKO_API oko_Font* oko_font_from_8x8(char font8x8[128][8]) {
    oko_Font* font = (oko_Font*)malloc(sizeof(oko_Font));
    if (!font) return NULL;

    font->size = 8;
    font->ascent = 6;
    font->descent = 2;
    font->lineGap = 2;
    font->glyphCount = 128;

    font->glyphs = (oko_Glyph*)malloc(sizeof(oko_Glyph) * 128);
    if (!font->glyphs) {
        free(font);
        return NULL;
    }

    for (int i = 0; i < 128; i++) {
        oko_Glyph* glyph = &font->glyphs[i];
        glyph->character = i;
        glyph->width = 8;
        glyph->height = 8;
        glyph->advance = 8;
        glyph->offsetX = 0;
        glyph->offsetY = 0;

        glyph->bitmap = (unsigned char*)malloc(64);
        if (!glyph->bitmap) {
            for (int j = 0; j < i; j++) {
                free(font->glyphs[j].bitmap);
            }
            free(font->glyphs);
            free(font);
            return NULL;
        }

        for (int y = 0; y < 8; y++) {
            unsigned char row = font8x8[i][y];
            for (int x = 0; x < 8; x++) {
                glyph->bitmap[y * 8 + x] = (row & (1 << x)) ? 255 : 0;
            }
        }
    }

    return font;
}

int main() {
    oko_Font *f = oko_font_from_8x8(font8x8_basic);
    oko_init();
    oko_Window *win = oko_create("Example", 800, 600);
    oko_TI in = ti_init();

    u64 i = 0;
    while (oko_is_running(win))
    {
        oko_begin_drawing(win);
        oko_clear(win, 0x00);
        ti_update(&in, win);

        //printf("Input: %s\n", in.buffer);
        oko_draw_text(win, oko_format("%llu times i fucked monkeys", i), f, 100, 100, 3.0f, OKO_RGB(0xFF, 0xFF, 0xFF));
        oko_end_drawing(win);
        i += 1;
    }

    oko_destroy(win);
    return 0;
}