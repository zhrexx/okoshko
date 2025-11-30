#define OKO_TEMP_ALLOCATOR_IMPLEMENTATION
#include "okoshko.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static oko_temp_allocator ta;

OKO_API void oko_init() {
    ta = oko_temp_init(64*1024*1024);
}

#ifdef __APPLE__
#include <mach/mach_time.h>

struct oko_Timer {
    mach_timebase_info_data_t timebase;
    uint64_t start;
};

OKO_API oko_Timer *okoshko_timer_create() {
    oko_Timer *timer = malloc(sizeof(oko_Timer));
    mach_timebase_info(&timer->timebase);
    timer->start = mach_absolute_time();
    return timer;
}

OKO_API u64 okoshko_timer_now(oko_Timer *timer) {
    uint64_t now = mach_absolute_time();
    uint64_t elapsed = now - timer->start;
    return (elapsed * timer->timebase.numer) / (timer->timebase.denom * 1000000);
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

#elif defined(_WIN32)

struct oko_Timer {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
};

OKO_API oko_Timer *okoshko_timer_create() {
    oko_Timer *timer = malloc(sizeof(oko_Timer));
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start);
    return timer;
}

OKO_API u64 okoshko_timer_now(oko_Timer *timer) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (u64)(((now.QuadPart - timer->start.QuadPart) * 1000) / timer->frequency.QuadPart);
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    Sleep((DWORD)ms);
}

#elif defined(__linux__)

struct oko_Timer {
    struct timespec start;
};

OKO_API oko_Timer *okoshko_timer_create() {
    oko_Timer *timer = malloc(sizeof(oko_Timer));
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
    return timer;
}

OKO_API u64 okoshko_timer_now(oko_Timer *timer) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    u64 elapsed_sec = now.tv_sec - timer->start.tv_sec;
    i64 elapsed_nsec = now.tv_nsec - timer->start.tv_nsec;
    if (elapsed_nsec < 0) {
        elapsed_sec--;
        elapsed_nsec += 1000000000;
    }
    return elapsed_sec * 1000 + elapsed_nsec / 1000000;
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

#endif

#ifdef __APPLE__
#include "platform/apple.h"
#elif defined(_WIN32)
#include "platform/windows.h"
#elif defined(__linux__)
#include "platform/linux_x11.h"
#endif

OKO_API void oko_set_fps(oko_Window *win, u32 fps) {
    if (fps <= 0)
    {
        win->vsync = false;
        win->target_frame_time = 0;
    } else
    {
        win->vsync = true;
        win->target_frame_time = 1000 / fps;
    }
}

OKO_API u32 oko_get_fps(oko_Window *win) {
    if (win->actual_frame_time <= 0) return 0;
    return (u32)(1000 / win->actual_frame_time);
}

OKO_API u8 oko_is_running(oko_Window *win) {
    return win->running;
}

OKO_API void oko_begin_drawing(oko_Window *win) {
    win->frame_start_time = okoshko_timer_now(win->timer);
    oko_poll_events(win);
}

OKO_API void oko_end_drawing(oko_Window *win) {
    memcpy(win->pixels, win->back_buffer, win->width * win->height * sizeof(u32));

#ifdef _WIN32
    HDC hdc = GetDC(win->osw.hwnd);
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = win->width;
    bmi.bmiHeader.biHeight = -win->height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(hdc, 0, 0, win->width, win->height, 0, 0,
                  win->width, win->height, win->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(win->osw.hwnd, hdc);
#elif defined(__linux__)
    XPutImage(win->osw.dpy, win->osw.w, win->osw.gc, win->osw.img,
              0, 0, 0, 0, win->width, win->height);
    XFlush(win->osw.dpy);
#endif

    u64 frame_end_time = okoshko_timer_now(win->timer);
    u64 elapsed = frame_end_time - win->frame_start_time;

    if (win->target_frame_time > 0 && elapsed < win->target_frame_time) {
        u64 sleep_time = win->target_frame_time - elapsed;
        okoshko_timer_sleep(sleep_time);
        frame_end_time = okoshko_timer_now(win->timer);
    }

    win->actual_frame_time = frame_end_time - win->frame_start_time;
    if (win->actual_frame_time < 1) {
        win->actual_frame_time = 1;
    }
}

OKO_API void oko_clear(oko_Window *win, u32 color) {
    u32 *buf = win->back_buffer;
    i32 count = win->width * win->height;
    for (i32 i = 0; i < count; i++) {
        buf[i] = color;
    }
}

OKO_API void oko_set_pixel(oko_Window *win, i32 x, i32 y, u32 color) {
    if (x >= 0 && x < win->width && y >= 0 && y < win->height) {
        win->back_buffer[y * win->width + x] = color;
    }
}

OKO_API u32 oko_get_pixel(oko_Window *win, i32 x, i32 y) {
    if (x >= 0 && x < win->width && y >= 0 && y < win->height) {
        return win->back_buffer[y * win->width + x];
    }
    return 0;
}

OKO_API void oko_fill_rect(oko_Window *win, oko_Rect rect, u32 color) {
    i32 x0 = rect.x < 0 ? 0 : rect.x;
    i32 y0 = rect.y < 0 ? 0 : rect.y;
    i32 x1 = rect.x + rect.w > win->width ? win->width : rect.x + rect.w;
    i32 y1 = rect.y + rect.h > win->height ? win->height : rect.y + rect.h;

    for (i32 y = y0; y < y1; y++) {
        for (i32 x = x0; x < x1; x++) {
            win->back_buffer[y * win->width + x] = color;
        }
    }
}

OKO_API void oko_fill_circle(oko_Window *win, i32 cx, i32 cy, i32 radius, u32 color) {
    for (i32 y = -radius; y <= radius; y++) {
        for (i32 x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                oko_set_pixel(win, cx + x, cy + y, color);
            }
        }
    }
}

OKO_API void oko_draw_rect(oko_Window *win, oko_Rect rect, u32 color) {
    oko_draw_line(win, rect.x, rect.y, rect.x + rect.w, rect.y, color);
    oko_draw_line(win, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h, color);
    oko_draw_line(win, rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h, color);
    oko_draw_line(win, rect.x, rect.y + rect.h, rect.x, rect.y, color);
}

OKO_API void oko_draw_line(oko_Window *win, i32 x0, i32 y0, i32 x1, i32 y1, u32 color) {
    i32 dx = abs(x1 - x0);
    i32 dy = abs(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;

    while (1) {
        oko_set_pixel(win, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        i32 e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

OKO_API void oko_draw_circle(oko_Window *win, i32 cx, i32 cy, i32 radius, u32 color) {
    i32 x = 0;
    i32 y = radius;
    i32 d = 3 - 2 * radius;

    while (y >= x) {
        oko_set_pixel(win, cx + x, cy + y, color);
        oko_set_pixel(win, cx - x, cy + y, color);
        oko_set_pixel(win, cx + x, cy - y, color);
        oko_set_pixel(win, cx - x, cy - y, color);
        oko_set_pixel(win, cx + y, cy + x, color);
        oko_set_pixel(win, cx - y, cy + x, color);
        oko_set_pixel(win, cx + y, cy - x, color);
        oko_set_pixel(win, cx - y, cy - x, color);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
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
    if (glyph.bitmap) {
        for (int y = 0; y < height; y++) {
            memcpy(&glyph.bitmap[y * width], &bitmap[y][startX], width);
        }
    }

    return glyph;
}

OKO_API oko_Font* oko_bitmap_to_font(u8** bitmap, i32 totalWidth, i32 totalHeight,
                       i32 glyphWidth, i32 glyphCount, u8 startChar) {
    if (!bitmap || totalWidth <= 0 || totalHeight <= 0 ||
        glyphWidth <= 0 || glyphCount <= 0) {
        return NULL;
    }

    oko_Font* font = (oko_Font*)malloc(sizeof(oko_Font));
    if (!font) return NULL;

    font->size = totalHeight;
    font->ascent = (int)(totalHeight * 0.8);
    font->descent = totalHeight - font->ascent;
    font->lineGap = (int)(totalHeight * 0.2);
    font->glyphCount = glyphCount;

    font->glyphs = (oko_Glyph*)malloc(sizeof(oko_Glyph) * glyphCount);
    if (!font->glyphs) {
        free(font);
        return NULL;
    }

    for (int i = 0; i < glyphCount; i++) {
        int startX = i * glyphWidth;
        font->glyphs[i] = oko_create_glyph(bitmap, glyphWidth, totalHeight,
                                      startX, startChar + i);
    }

    return font;
}

OKO_API void oko_free_font(oko_Font* font) {
    if (!font) return;

    if (font->glyphs) {
        for (int i = 0; i < font->glyphCount; i++) {
            free(font->glyphs[i].bitmap);
        }
        free(font->glyphs);
    }
    free(font);
}

OKO_API void oko_draw_text(oko_Window *win, const char *text, oko_Font *font, i32 x, i32 y, float scale, u32 color) {
    if (!win || !text || !font || !font->glyphs) return;

    i32 cursorX = x;
    i32 cursorY = y;

    u8 r = (color >> 16) & 0xFF;
    u8 g = (color >> 8) & 0xFF;
    u8 b = color & 0xFF;

    for (const char *c = text; *c != '\0'; c++) {
        if (*c == '\n') {
            cursorX = x;
            cursorY += (i32)((font->size + font->lineGap) * scale);
            continue;
        }

        oko_Glyph *glyph = NULL;
        for (i32 i = 0; i < font->glyphCount; i++) {
            if (font->glyphs[i].character == *c) {
                glyph = &font->glyphs[i];
                break;
            }
        }

        if (!glyph || !glyph->bitmap) {
            cursorX += (i32)((f32)font->size * 0.5f * scale);
            continue;
        }

        i32 scaledWidth = (i32)(glyph->width * scale);
        i32 scaledHeight = (i32)(glyph->height * scale);

        for (i32 gy = 0; gy < scaledHeight; gy++) {
            for (i32 gx = 0; gx < scaledWidth; gx++) {
                i32 srcX = (i32)(gx / scale);
                i32 srcY = (i32)(gy / scale);

                if (srcX >= glyph->width || srcY >= glyph->height) continue;

                u8 alpha = glyph->bitmap[srcY * glyph->width + srcX];

                if (alpha == 0) continue;

                i32 px = cursorX + gx + (i32)(glyph->offsetX * scale);
                i32 py = cursorY + gy + (i32)(glyph->offsetY * scale);

                if (px < 0 || px >= win->width || py < 0 || py >= win->height) continue;

                if (alpha == 255) {
                    win->back_buffer[py * win->width + px] = color;
                } else {
                    u32 bg = win->back_buffer[py * win->width + px];
                    u8 bgR = (bg >> 16) & 0xFF;
                    u8 bgG = (bg >> 8) & 0xFF;
                    u8 bgB = bg & 0xFF;

                    float a = alpha / 255.0f;
                    u8 outR = (u8)(r * a + bgR * (1.0f - a));
                    u8 outG = (u8)(g * a + bgG * (1.0f - a));
                    u8 outB = (u8)(b * a + bgB * (1.0f - a));

                    win->back_buffer[py * win->width + px] = (outR << 16) | (outG << 8) | outB;
                }
            }
        }

        cursorX += (i32)(glyph->advance * scale);
    }
}

OKO_API u8 oko_key_down(oko_Window *win, u8 key) {
    return win->keyboard.keys[key];
}

OKO_API u8 oko_key_pressed(oko_Window *win, u8 key) {
    if (key >= 'A' && key <= 'Z')
    {
        if (win->keyboard.shift)
        {
            key = key + 32;
        }
    }
    return win->keyboard.keys[key];
}

OKO_API u8 oko_mouse_down(oko_Window *win, u8 button) {
    if (button == OKO_MOUSE_LEFT) return win->mouse.left;
    if (button == OKO_MOUSE_RIGHT) return win->mouse.right;
    if (button == OKO_MOUSE_MIDDLE) return win->mouse.middle;
    return 0;
}

OKO_API u64 oko_time_ms(oko_Window *win) {
    return okoshko_timer_now(win->timer);
}

OKO_API void oko_sleep(u64 ms) {
    okoshko_timer_sleep(ms);
}

OKO_API char *oko_format(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *str = oko_temp_alloc(&ta, 512, 8);

    if (!vsprintf(str, format, args))
    {
        return NULL;
    }
    va_end(args);
    return str;
}