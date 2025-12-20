#ifndef OKOSHKO_LIBRARY_H
#define OKOSHKO_LIBRARY_H

#include "helpers/allocator.h"
#include "helpers/log.h"
#include "helpers/st.h"

#ifndef OKO_DONT_USE_NATIVE
#ifdef __APPLE__
#define OKO_APPLE
#elif defined(_WIN32)
#define OKO_WINDOWS
#elif defined(__linux__)
#define OKO_LINUX
#endif
#endif

typedef struct {
    i32 x, y, w, h;
} oko_Rect;

typedef struct {
    i32 x, y;
} oko_Point;

typedef struct {
    u8 keys[256];
    u8 prev_keys[256];
    u8 ctrl, shift, alt, meta;
} oko_Keyboard;

typedef struct {
    i32 x, y;
    u8 left, right, middle;
    i32 scroll_x, scroll_y;
} oko_Mouse;

typedef struct oko_Timer oko_Timer;
typedef struct oko_PlatformWindow oko_PlatformWindow;

typedef struct {
    u8 character;
    i32 width;
    i32 height;
    i32 advance;
    i32 offsetX;
    i32 offsetY;
    u8* bitmap;
} oko_Glyph;

typedef struct {
    i32 size;
    i32 ascent;
    i32 descent;
    i32 lineGap;
    i32 glyphCount;
    oko_Glyph* glyphs;
} oko_Font;

typedef void (*oko_Event_Callback_t)(void*);

typedef enum {
    // TODO: add events
    OKO_NO_EVENT
} oko_EventT;

typedef struct listener {
    int event;
    oko_Event_Callback_t cb;
    struct listener* next;
} listener_t;

typedef struct {
    listener_t* head;
} event_dispatcher_t;

static inline void ed_init(event_dispatcher_t* d) {
    d->head = NULL;
}
static inline void ed_on(event_dispatcher_t* d, int event, oko_Event_Callback_t cb) {
    listener_t* l = malloc(sizeof(listener_t));
    l->event = event;
    l->cb = cb;
    l->next = d->head;
    d->head = l;
}
static inline void ed_emit(event_dispatcher_t* d, int event, void* data) {
    for (listener_t* l = d->head; l; l = l->next)
    {
        if (l->event == event) l->cb(data);
    }
}
static inline void ed_free(event_dispatcher_t* d) {
    while (d->head)
    {
        listener_t* tmp = d->head;
        d->head = d->head->next;
        free(tmp);
    }
}

// TODO: add events
typedef struct {
    char* title;
    i32 width, height;
    u32* pixels;
    u32* back_buffer;
    u8 vsync;
    u8 running;
    u8 showed;

    oko_Mouse mouse;
    oko_Keyboard keyboard;

    event_dispatcher_t ed;

    oko_PlatformWindow *pw;
    oko_Timer* timer;
    u64 target_frame_time;
    u64 frame_start_time;
    u64 actual_frame_time;
} oko_Window;

#ifndef OKO_API
#define OKO_API extern
#endif

OKO_API const char* oko_error;
OKO_API const char* oko_error2;

OKO_API void oko_init();

// WINDOW STUFF
OKO_API oko_Window* oko_create(const char* title, i32 width, i32 height);
OKO_API void oko_destroy(oko_Window* win);
OKO_API void oko_set_fps(oko_Window* win, u32 fps);
OKO_API u32 oko_get_fps(oko_Window* win);
OKO_API u8 oko_is_running(const oko_Window* win);

// TIMER STUFF
OKO_API oko_Timer* okoshko_timer_create();
OKO_API u64 okoshko_timer_now(oko_Timer* timer);
OKO_API void okoshko_timer_sleep(u64 ms);

// EVENT STUFF & BATCH RENDERING
OKO_API void oko_os_swap_buffers(oko_Window* win);
OKO_API void oko_begin_frame(oko_Window* win);
OKO_API void oko_end_frame(oko_Window* win);
OKO_API void oko_poll_events(oko_Window* win);

// PIXEL MANIPULATION STUFF
OKO_API void oko_clear(oko_Window* win, u32 color);
OKO_API inline void oko_set_pixel(oko_Window* win, i32 x, i32 y, u32 color);
OKO_API u32 oko_get_pixel(oko_Window* win, i32 x, i32 y);

// DRAW STUFF
OKO_API void oko_fill_rect(oko_Window* win, oko_Rect rect, u32 color);
OKO_API void oko_fill_circle(oko_Window* win, i32 cx, i32 cy, i32 radius,
                             u32 color);
OKO_API void oko_draw_rect(oko_Window* win, oko_Rect rect, u32 color);
OKO_API void oko_draw_line(oko_Window* win, i32 x0, i32 y0, i32 x1, i32 y1,
                           u32 color);
OKO_API void oko_draw_circle(oko_Window* win, i32 cx, i32 cy, i32 radius,
                             u32 color);
OKO_API void oko_draw_text(oko_Window* win, const char* text, oko_Font* font,
                           i32 x, i32 y, float scale, u32 color);

// KEYBOARD STUFF
OKO_API u8 oko_key_down(oko_Window* win, u8 key);
OKO_API u8 oko_key_pressed(oko_Window* win, u8 key);
OKO_API void oko_key_reset(oko_Window* win, u8 key);
OKO_API u8 oko_mouse_down(oko_Window* win, u8 button);

// TIME STUFF
OKO_API u64 oko_time_ms(oko_Window* win);
OKO_API void oko_sleep(u64 ms);

// FONT STUFF
OKO_API oko_Glyph oko_create_glyph(u8** bitmap, i32 width, i32 height,
                                   i32 startX, u8 character);
OKO_API oko_Font* oko_bitmap_to_font(u8** bitmap, i32 totalWidth,
                                     i32 totalHeight, i32 glyphWidth,
                                     i32 glyphCount, u8 startChar);
OKO_API void oko_free_font(oko_Font* font);

// HELPER STUFF
OKO_API char* oko_format(const char* format, ...);
OKO_API oko_temp_allocator* oko_get_temp_allocator();

#include "helpers/macros.h"

#endif
