#ifndef OKOSHKO_LIBRARY_H
#define OKOSHKO_LIBRARY_H

#include "helpers/st.h"
#include "helpers/log.h"
#include "helpers/allocator.h"

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#define _DEFAULT_SOURCE 1
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <time.h>
#else
#error "Platform not supported!"
#endif

typedef struct {
    i32 x, y, w, h;
} oko_Rect;

typedef struct {
    i32 x, y;
} oko_Point;

typedef struct {
    u8 keys[256];
    u8 ctrl, shift, alt, meta;
} oko_Keyboard;

typedef struct {
    i32 x, y;
    u8 left, right, middle;
} oko_Mouse;

typedef struct oko_Timer oko_Timer;

typedef struct {
#ifdef __APPLE__
    id wnd;
#elif defined(_WIN32)
    HWND hwnd;
#elif defined(__linux__)
    Display *dpy;
    Window w;
    GC gc;
    XImage *img;
    Atom wm_delete_window;
#endif
} oko_OsWindow;

typedef struct {
    char character;
    int width;
    int height;
    int advance;
    int offsetX;
    int offsetY;
    unsigned char* bitmap;
} oko_Glyph;

typedef struct {
    int size;
    int ascent;
    int descent;
    int lineGap;
    int glyphCount;
    oko_Glyph* glyphs;
} oko_Font;

typedef struct {
    char *title;
    i32 width, height;
    u32 *pixels;
    u32 *back_buffer;
    u8 vsync;
    u8 running;
    u8 showed;

    oko_Mouse mouse;
    oko_Keyboard keyboard;
    
    oko_OsWindow osw;
    oko_Timer *timer;
    u64 target_frame_time;
    u64 last_frame_time;
} oko_Window;

#ifndef OKO_API
#define OKO_API extern
#endif

OKO_API void oko_init();

OKO_API oko_Window* oko_create(const char *title, i32 width, i32 height);
OKO_API void oko_destroy(oko_Window *win);
OKO_API void oko_set_fps(oko_Window *win, i32 fps);
OKO_API u8 oko_is_running(oko_Window *win);

OKO_API void oko_begin_drawing(oko_Window *win);
OKO_API void oko_end_drawing(oko_Window *win);
OKO_API void oko_poll_events(oko_Window *win);

OKO_API void oko_clear(oko_Window *win, u32 color);
OKO_API void oko_set_pixel(oko_Window *win, i32 x, i32 y, u32 color);
OKO_API u32 oko_get_pixel(oko_Window *win, i32 x, i32 y);

OKO_API void oko_fill_rect(oko_Window *win, oko_Rect rect, u32 color);
OKO_API void oko_fill_circle(oko_Window *win, i32 cx, i32 cy, i32 radius, u32 color);

OKO_API void oko_draw_rect(oko_Window *win, oko_Rect rect, u32 color);
OKO_API void oko_draw_line(oko_Window *win, i32 x0, i32 y0, i32 x1, i32 y1, u32 color);
OKO_API void oko_draw_circle(oko_Window *win, i32 cx, i32 cy, i32 radius, u32 color);
OKO_API void oko_draw_text(oko_Window *win, const char *text, oko_Font *font, i32 x, i32 y, float scale, u32 color);

OKO_API u8 oko_key_down(oko_Window *win, u8 key);
OKO_API u8 oko_key_pressed(oko_Window *win, u8 key);
OKO_API u8 oko_mouse_down(oko_Window *win, u8 button);

OKO_API u64 oko_time_ms(oko_Window *win);
OKO_API void oko_sleep(u64 ms);

OKO_API oko_Glyph oko_create_glyph(u8** bitmap, i32 width, i32 height,
                              i32 startX, u8 character);
OKO_API oko_Font* oko_bitmap_to_font(u8** bitmap, i32 totalWidth, i32 totalHeight,
                       i32 glyphWidth, i32 glyphCount, u8 startChar);
OKO_API void oko_free_font(oko_Font* font);

OKO_API char *oko_format(const char *format, ...);

#include "helpers/macros.h"

#endif