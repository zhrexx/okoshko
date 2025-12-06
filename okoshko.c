#define OKO_TEMP_ALLOCATOR_IMPLEMENTATION
#include "okoshko.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// TODO: use oko_error everywhere

static oko_temp_allocator ta;
const char* oko_error = NULL;
const char* oko_error2 = NULL;

OKO_API void oko_init() { ta = oko_temp_init(64 * 1024 * 1024); }

#ifdef OKO_APPLE
#include "platform/apple.h"
#elif defined(OKO_WINDOWS)
#include "platform/windows.h"
#elif defined(OKO_LINUX)
#include "platform/linux_x11.h"
#endif

OKO_API void oko_set_fps(oko_Window* win, u32 fps) {
    if (fps <= 0)
    {
        win->vsync = false;
        win->target_frame_time = 0;
    }
    else
    {
        win->vsync = true;
        win->target_frame_time = (u64)((1000000.0 / fps) + 0.5);
    }
}

OKO_API u32 oko_get_fps(oko_Window* win) {
    if (win->actual_frame_time <= 0)
        return 0;
    return (u32)((1000000.0 / (f64)win->actual_frame_time) + 0.5);
}

OKO_API u8 oko_is_running(const oko_Window* win) { return win->running; }

OKO_API void oko_begin_drawing(oko_Window* win) {
    win->frame_start_time = okoshko_timer_now(win->timer);
    oko_poll_events(win);
}

OKO_API void oko_end_drawing(oko_Window* win) {
    memcpy(win->pixels, win->back_buffer, win->width * win->height * sizeof(u32));

#ifdef OKO_WINDOWS
    HDC hdc = GetDC(win->pw->hwnd);
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = win->width;
    bmi.bmiHeader.biHeight = -win->height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(hdc, 0, 0, win->width, win->height, 0, 0, win->width,
                  win->height, win->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(win->pw->hwnd, hdc);
#elif defined(OKO_LINUX)
    XPutImage(win->pw->dpy, win->pw->w, win->pw->gc, win->pw->img, 0, 0, 0, 0,
              win->width, win->height);
    XFlush(win->pw->dpy);
#endif

    u64 frame_end_time = okoshko_timer_now(win->timer);
    u64 elapsed_us = (frame_end_time - win->frame_start_time) * 1000;

    if (win->target_frame_time > 0)
    {
        u64 spin_threshold = 1000;

        if (elapsed_us + spin_threshold < win->target_frame_time)
        {
            u64 sleep_time_ms =
                (win->target_frame_time - elapsed_us - spin_threshold) / 1000;
            if (sleep_time_ms > 0)
            {
                okoshko_timer_sleep(sleep_time_ms);
            }
        }

        while (1)
        {
            frame_end_time = okoshko_timer_now(win->timer);
            elapsed_us = (frame_end_time - win->frame_start_time) * 1000;
            if (elapsed_us >= win->target_frame_time)
            {
                break;
            }
        }
    }

    win->actual_frame_time = elapsed_us;
    if (win->actual_frame_time < 1)
    {
        win->actual_frame_time = 1;
    }
}

OKO_API void oko_clear(oko_Window* win, u32 color) {
    u32* buf = win->back_buffer;
    i32 count = win->width * win->height;
    for (i32 i = 0; i < count; i++)
    {
        buf[i] = color;
    }
}

OKO_API void oko_set_pixel(oko_Window* win, i32 x, i32 y, u32 color) {
    if (x >= 0 && x < win->width && y >= 0 && y < win->height)
    {
        win->back_buffer[y * win->width + x] = color;
    }
}

OKO_API u32 oko_get_pixel(oko_Window* win, i32 x, i32 y) {
    if (x >= 0 && x < win->width && y >= 0 && y < win->height)
    {
        return win->back_buffer[y * win->width + x];
    }
    return 0;
}

OKO_API void oko_fill_rect(oko_Window* win, oko_Rect rect, u32 color) {
    i32 x0 = rect.x < 0 ? 0 : rect.x;
    i32 y0 = rect.y < 0 ? 0 : rect.y;
    i32 x1 = rect.x + rect.w > win->width ? win->width : rect.x + rect.w;
    i32 y1 = rect.y + rect.h > win->height ? win->height : rect.y + rect.h;

    for (i32 y = y0; y < y1; y++)
    {
        for (i32 x = x0; x < x1; x++)
        {
            win->back_buffer[y * win->width + x] = color;
        }
    }
}

OKO_API void oko_fill_circle(oko_Window* win, i32 cx, i32 cy, i32 radius,
                             u32 color) {
    for (i32 y = -radius; y <= radius; y++)
    {
        for (i32 x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                oko_set_pixel(win, cx + x, cy + y, color);
            }
        }
    }
}

OKO_API void oko_draw_rect(oko_Window* win, oko_Rect rect, u32 color) {
    oko_draw_line(win, rect.x, rect.y, rect.x + rect.w, rect.y, color);
    oko_draw_line(win, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h,
                  color);
    oko_draw_line(win, rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h,
                  color);
    oko_draw_line(win, rect.x, rect.y + rect.h, rect.x, rect.y, color);
}

OKO_API void oko_draw_line(oko_Window* win, i32 x0, i32 y0, i32 x1, i32 y1,
                           u32 color) {
    i32 dx = abs(x1 - x0);
    i32 dy = abs(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;

    while (1)
    {
        oko_set_pixel(win, x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        i32 e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

OKO_API void oko_draw_circle(oko_Window* win, i32 cx, i32 cy, i32 radius,
                             u32 color) {
    i32 x = 0;
    i32 y = radius;
    i32 d = 3 - 2 * radius;

    while (y >= x)
    {
        oko_set_pixel(win, cx + x, cy + y, color);
        oko_set_pixel(win, cx - x, cy + y, color);
        oko_set_pixel(win, cx + x, cy - y, color);
        oko_set_pixel(win, cx - x, cy - y, color);
        oko_set_pixel(win, cx + y, cy + x, color);
        oko_set_pixel(win, cx - y, cy + x, color);
        oko_set_pixel(win, cx + y, cy - x, color);
        oko_set_pixel(win, cx - y, cy - x, color);
        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

OKO_API oko_Glyph oko_create_glyph(u8** bitmap, i32 width, i32 height,
                                   i32 startX, u8 character) {
    oko_Glyph glyph;
    glyph.character = character;
    glyph.width = width;
    glyph.height = height;
    glyph.advance = width;
    glyph.offsetX = 0;
    glyph.offsetY = 0;

    glyph.bitmap = (unsigned char*)malloc(width * height);
    if (glyph.bitmap)
    {
        for (int y = 0; y < height; y++)
        {
            memcpy(&glyph.bitmap[y * width], &bitmap[y][startX], width);
        }
    }

    return glyph;
}

OKO_API oko_Font* oko_bitmap_to_font(u8** bitmap, i32 totalWidth,
                                     i32 totalHeight, i32 glyphWidth,
                                     i32 glyphCount, u8 startChar) {
    if (!bitmap || totalWidth <= 0 || totalHeight <= 0 || glyphWidth <= 0 ||
        glyphCount <= 0)
    {
        return NULL;
    }

    oko_Font* font = (oko_Font*)malloc(sizeof(oko_Font));
    if (!font)
        return NULL;

    font->size = totalHeight;
    font->ascent = (int)(totalHeight * 0.8);
    font->descent = totalHeight - font->ascent;
    font->lineGap = (int)(totalHeight * 0.2);
    font->glyphCount = glyphCount;

    font->glyphs = (oko_Glyph*)malloc(sizeof(oko_Glyph) * glyphCount);
    if (!font->glyphs)
    {
        free(font);
        return NULL;
    }

    for (int i = 0; i < glyphCount; i++)
    {
        int startX = i * glyphWidth;
        font->glyphs[i] = oko_create_glyph(bitmap, glyphWidth, totalHeight, startX,
                                           startChar + i);
    }

    return font;
}

OKO_API void oko_free_font(oko_Font* font) {
    if (!font)
        return;

    if (font->glyphs)
    {
        for (int i = 0; i < font->glyphCount; i++)
        {
            free(font->glyphs[i].bitmap);
        }
        free(font->glyphs);
    }
    free(font);
}

OKO_API void oko_draw_text(oko_Window* win, const char* text, oko_Font* font,
                           i32 x, i32 y, float scale, u32 color) {
    if (!win || !text || !font || !font->glyphs)
        return;

    i32 cursorX = x;
    i32 cursorY = y;

    u8 r = (color >> 16) & 0xFF;
    u8 g = (color >> 8) & 0xFF;
    u8 b = color & 0xFF;

    for (const char* c = text; *c != '\0'; c++)
    {
        if (*c == '\n')
        {
            cursorX = x;
            cursorY += (i32)((font->size + font->lineGap) * scale);
            continue;
        }

        oko_Glyph* glyph = NULL;
        for (i32 i = 0; i < font->glyphCount; i++)
        {
            if (font->glyphs[i].character == *c)
            {
                glyph = &font->glyphs[i];
                break;
            }
        }

        if (!glyph || !glyph->bitmap)
        {
            cursorX += (i32)((f32)font->size * 0.5f * scale);
            continue;
        }

        i32 scaledWidth = (i32)(glyph->width * scale);
        i32 scaledHeight = (i32)(glyph->height * scale);

        for (i32 gy = 0; gy < scaledHeight; gy++)
        {
            for (i32 gx = 0; gx < scaledWidth; gx++)
            {
                i32 srcX = (i32)(gx / scale);
                i32 srcY = (i32)(gy / scale);

                if (srcX >= glyph->width || srcY >= glyph->height)
                    continue;

                u8 alpha = glyph->bitmap[srcY * glyph->width + srcX];

                if (alpha == 0)
                    continue;

                i32 px = cursorX + gx + (i32)(glyph->offsetX * scale);
                i32 py = cursorY + gy + (i32)(glyph->offsetY * scale);

                if (px < 0 || px >= win->width || py < 0 || py >= win->height)
                    continue;

                if (alpha == 255)
                {
                    win->back_buffer[py * win->width + px] = color;
                }
                else
                {
                    u32 bg = win->back_buffer[py * win->width + px];
                    u8 bgR = (bg >> 16) & 0xFF;
                    u8 bgG = (bg >> 8) & 0xFF;
                    u8 bgB = bg & 0xFF;

                    float a = alpha / 255.0f;
                    u8 outR = (u8)(r * a + bgR * (1.0f - a));
                    u8 outG = (u8)(g * a + bgG * (1.0f - a));
                    u8 outB = (u8)(b * a + bgB * (1.0f - a));

                    win->back_buffer[py * win->width + px] =
                        (outR << 16) | (outG << 8) | outB;
                }
            }
        }

        cursorX += (i32)(glyph->advance * scale);
    }
}

OKO_API u8 oko_key_down(oko_Window* win, u8 key) {
    return win->keyboard.keys[key];
}

OKO_API u8 oko_key_pressed(oko_Window* win, u8 key) {
    if (key >= 'A' && key <= 'Z')
    {
        if (win->keyboard.shift)
        {
            key = key + 32;
        }
    }
    return win->keyboard.keys[key];
}

OKO_API u8 oko_mouse_down(oko_Window* win, u8 button) {
    if (button == OKO_MOUSE_LEFT)
        return win->mouse.left;
    if (button == OKO_MOUSE_RIGHT)
        return win->mouse.right;
    if (button == OKO_MOUSE_MIDDLE)
        return win->mouse.middle;
    return 0;
}

OKO_API u64 oko_time_ms(oko_Window* win) {
    return okoshko_timer_now(win->timer);
}

OKO_API void oko_sleep(u64 ms) { okoshko_timer_sleep(ms); }

OKO_API char* oko_format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* str = oko_temp_alloc(&ta, 512, 8);
    if (!str)
        return NULL;

    if (!vsprintf(str, format, args))
    {
        return NULL;
    }
    va_end(args);
    return str;
}

OKO_API oko_temp_allocator* oko_get_temp_allocator() { return &ta; }

OKO_API oko_AudioSystem* oko_audio_create() {
    oko_AudioSystem* audio = malloc(sizeof(oko_AudioSystem));
    if (!audio)
    {
        oko_error = "Could not allocate memory for audio system";
        return NULL;
    }
    audio->os_audio =
        oko_os_audio_create(OKO_AUDIO_SAMPLE_RATE, OKO_AUDIO_BUFFER_SIZE);
    if (!audio->os_audio)
    {
        oko_error = "Could not create os audio system";
        return NULL;
    }
    audio->is_open = 0;
    audio->is_paused = 1;
    audio->volume = 0.f;
    return audio;
}

OKO_API void oko_audio_destroy(oko_AudioSystem* audio) {
    oko_os_audio_destroy(audio->os_audio);
    free(audio);
}

OKO_API u64 oko_audio_get_available_frames(oko_AudioSystem* audio) {
    return oko_os_audio_get_available_frames(audio->os_audio);
}

OKO_API i32 oko_audio_write(oko_AudioSystem* audio, const f32* samples,
                            u64 frame_count) {
    return oko_os_audio_submit_buffer(audio->os_audio, samples, frame_count);
}
