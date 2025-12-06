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

#ifdef OKO_APPLE
#include <AudioToolbox/AudioQueue.h>
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#elif defined(OKO_WINDOWS)
#include <mmsystem.h>
#include <windows.h>
#elif defined(OKO_LINUX)
#define _DEFAULT_SOURCE 1
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#elif defined(OKO_WEB)
#include <time.h>
#else
#error "Platform not supported!"
#endif

#ifndef OKO_AUDIO_SAMPLE_RATE
#define OKO_AUDIO_SAMPLE_RATE 44100
#endif

#ifndef OKO_AUDIO_BUFFER_SIZE
#define OKO_AUDIO_BUFFER_SIZE 8192
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
typedef struct oko_PlatformWindow oko_PlatformWindow;
typedef struct oko_PlatformAudioSystem oko_PlatformAudioSystem;

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

typedef struct {
    f32 volume; // TODO implement
    i32 is_open;
    i32 is_paused;
    oko_PlatformAudioSystem* os_audio;
} oko_AudioSystem;

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

    oko_PlatformWindow *pw; // TODO: refactor
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

// AUDIO STUFF
OKO_API oko_PlatformAudioSystem* oko_os_audio_create(u64 sample_rate,
                                               u64 buffer_size);
OKO_API void oko_os_audio_destroy(oko_PlatformAudioSystem* os_audio);
OKO_API u64 oko_os_audio_get_available_frames(oko_PlatformAudioSystem* os_audio);
OKO_API i32 oko_os_audio_submit_buffer(oko_PlatformAudioSystem* os_audio,
                                       const f32* buffer, u64 n);

OKO_API oko_AudioSystem* oko_audio_create();
OKO_API void oko_audio_destroy(oko_AudioSystem* audio);
OKO_API u64 oko_audio_get_available_frames(oko_AudioSystem* audio);
OKO_API i32 oko_audio_write(oko_AudioSystem* audio, const f32* samples,
                            u64 frame_count);

// EVENT STUFF & BATCH RENDERING
OKO_API void oko_begin_drawing(oko_Window* win);
OKO_API void oko_end_drawing(oko_Window* win);
OKO_API void oko_poll_events(oko_Window* win);

// PIXEL MANIPULATION STUFF
OKO_API void oko_clear(oko_Window* win, u32 color);
OKO_API void oko_set_pixel(oko_Window* win, i32 x, i32 y, u32 color);
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
