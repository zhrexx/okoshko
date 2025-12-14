#include <stdint.h>
#include <time.h>

#include "../okoshko.h"
#include <AudioToolbox/AudioQueue.h>
#include <CoreGraphics/CoreGraphics.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>

struct oko_PlatformWindow {
    id wnd;
};

extern id objc_msgSend(id, SEL, ...);
extern id objc_getClass(const char*);
extern SEL sel_registerName(const char*);

#define msg(obj, sel) objc_msgSend(obj, sel_registerName(sel))
#define msg1(obj, sel, arg) objc_msgSend(obj, sel_registerName(sel), arg)
#define msg2(obj, sel, a1, a2) objc_msgSend(obj, sel_registerName(sel), a1, a2)

static u8 g_should_close = 0;

static int oko_macos_key_to_index(unsigned short keyCode) {
    if (keyCode == 0)
        return 'A';
    if (keyCode == 11)
        return 'B';
    if (keyCode == 8)
        return 'C';
    if (keyCode == 2)
        return 'D';
    if (keyCode == 14)
        return 'E';
    if (keyCode == 3)
        return 'F';
    if (keyCode == 5)
        return 'G';
    if (keyCode == 4)
        return 'H';
    if (keyCode == 34)
        return 'I';
    if (keyCode == 38)
        return 'J';
    if (keyCode == 40)
        return 'K';
    if (keyCode == 37)
        return 'L';
    if (keyCode == 46)
        return 'M';
    if (keyCode == 45)
        return 'N';
    if (keyCode == 31)
        return 'O';
    if (keyCode == 35)
        return 'P';
    if (keyCode == 12)
        return 'Q';
    if (keyCode == 15)
        return 'R';
    if (keyCode == 1)
        return 'S';
    if (keyCode == 17)
        return 'T';
    if (keyCode == 32)
        return 'U';
    if (keyCode == 9)
        return 'V';
    if (keyCode == 13)
        return 'W';
    if (keyCode == 7)
        return 'X';
    if (keyCode == 16)
        return 'Y';
    if (keyCode == 6)
        return 'Z';

    if (keyCode == 29)
        return '0';
    if (keyCode == 18)
        return '1';
    if (keyCode == 19)
        return '2';
    if (keyCode == 20)
        return '3';
    if (keyCode == 21)
        return '4';
    if (keyCode == 23)
        return '5';
    if (keyCode == 22)
        return '6';
    if (keyCode == 26)
        return '7';
    if (keyCode == 28)
        return '8';
    if (keyCode == 25)
        return '9';

    if (keyCode == 49)
        return OKO_KEY_SPACE;
    if (keyCode == 36)
        return OKO_KEY_ENTER;
    if (keyCode == 53)
        return OKO_KEY_ESC;
    if (keyCode == 48)
        return OKO_KEY_TAB;
    if (keyCode == 51)
        return OKO_KEY_BACKSPACE;

    if (keyCode == 56 || keyCode == 60)
        return OKO_KEY_SHIFT;
    if (keyCode == 59 || keyCode == 62)
        return OKO_KEY_CTRL;
    if (keyCode == 58 || keyCode == 61)
        return OKO_KEY_ALT;

    if (keyCode == 123)
        return OKO_KEY_ARROW_LEFT;
    if (keyCode == 124)
        return OKO_KEY_ARROW_RIGHT;
    if (keyCode == 126)
        return OKO_KEY_ARROW_UP;
    if (keyCode == 125)
        return OKO_KEY_ARROW_DOWN;

    if (keyCode == 117)
        return OKO_KEY_DELETE;
    if (keyCode == 115)
        return OKO_KEY_HOME;
    if (keyCode == 119)
        return OKO_KEY_END;
    if (keyCode == 116)
        return OKO_KEY_PAGE_UP;
    if (keyCode == 121)
        return OKO_KEY_PAGE_DOWN;

    if (keyCode == 24)
        return '=';
    if (keyCode == 27)
        return '-';
    if (keyCode == 30)
        return ']';
    if (keyCode == 33)
        return '[';
    if (keyCode == 39)
        return '\'';
    if (keyCode == 41)
        return ';';
    if (keyCode == 42)
        return '\\';
    if (keyCode == 43)
        return ',';
    if (keyCode == 44)
        return '/';
    if (keyCode == 47)
        return '.';
    if (keyCode == 50)
        return '`';

    return 0;
}

OKO_API oko_Window* oko_create(const char* title, i32 width, i32 height) {
    oko_Window* win = calloc(1, sizeof(oko_Window));
    win->pw = calloc(1, sizeof(oko_PlatformWindow));
    win->title = strdup(title);
    win->width = width;
    win->height = height;
    win->pixels = calloc(width * height, sizeof(u32));
    win->back_buffer = calloc(width * height, sizeof(u32));
    win->running = 1;
    win->vsync = 1;
    win->target_frame_time = 16;
    win->timer = okoshko_timer_create();
    win->frame_start_time = okoshko_timer_now(win->timer);
    ed_init(&win->ed);

    id app = msg(objc_getClass("NSApplication"), "sharedApplication");
    msg1(app, "setActivationPolicy:", 0);

    CGRect frame = CGRectMake(0, 0, width, height);
    id window = msg(objc_getClass("NSWindow"), "alloc");
    window = msg2(window, "initWithContentRect:styleMask:backing:defer:", frame, 15, 2, NO);

    id titleStr = msg1(objc_getClass("NSString"), "stringWithUTF8String:", title);
    msg1(window, "setTitle:", titleStr);
    msg1(window, "makeKeyAndOrderFront:", window);

    win->pw->wnd = window;
    g_should_close = 0;

    log_info("Created Apple Window '%s' (%d, %d)", title, width, height);
    return win;
}

OKO_API void oko_destroy(oko_Window* win) {
    if (!win)
        return;
    msg(win->pw->wnd, "close");
    free(win->pw);
    free(win->pixels);
    free(win->back_buffer);
    free(win->title);
    free(win);
    log_info("Closing Window");
}

OKO_API void oko_poll_events(oko_Window* win) {
    id app = msg(objc_getClass("NSApplication"), "sharedApplication");
    id pool = msg(objc_getClass("NSAutoreleasePool"), "alloc");
    pool = msg(pool, "init");

    id event;
    do
    {
        event = msg2(app, "nextEventMatchingMask:untilDate:inMode:dequeue:", ULONG_MAX, NULL, 0, YES);
        if (event)
        {
            unsigned long type = (unsigned long)msg(event, "type");

            if (type == 10)
            {
                unsigned short keyCode = (unsigned short)(unsigned long)msg(event, "keyCode");
                int idx = oko_macos_key_to_index(keyCode);
                if (idx > 0 && idx < 256)
                {
                    win->keyboard.keys[idx] = 1;
                    if (idx == OKO_KEY_SHIFT)
                        win->keyboard.shift = 1;
                    if (idx == OKO_KEY_CTRL)
                        win->keyboard.ctrl = 1;
                    if (idx == OKO_KEY_ALT)
                        win->keyboard.alt = 1;
                }

                if (win->text_input_callback)
                {
                    id characters = msg(event, "characters");
                    if (characters)
                    {
                        const char* utf8 = (const char*)msg(characters, "UTF8String");
                        if (utf8 && utf8[0] >= 32)
                        {
                            win->text_input_callback(win, utf8);
                        }
                    }
                }
            }
            else if (type == 11)
            {
                unsigned short keyCode = (unsigned short)(unsigned long)msg(event, "keyCode");
                int idx = oko_macos_key_to_index(keyCode);
                if (idx > 0 && idx < 256)
                {
                    win->keyboard.keys[idx] = 0;
                    if (idx == OKO_KEY_SHIFT)
                        win->keyboard.shift = 0;
                    if (idx == OKO_KEY_CTRL)
                        win->keyboard.ctrl = 0;
                    if (idx == OKO_KEY_ALT)
                        win->keyboard.alt = 0;
                }
            }
            else if (type == 12)
            {
                unsigned long modifierFlags = (unsigned long)msg(event, "modifierFlags");
                int shift_pressed = (modifierFlags & (1 << 17)) ? 1 : 0;
                int ctrl_pressed = (modifierFlags & (1 << 18)) ? 1 : 0;
                int alt_pressed = (modifierFlags & (1 << 19)) ? 1 : 0;

                win->keyboard.keys[OKO_KEY_SHIFT] = shift_pressed;
                win->keyboard.keys[OKO_KEY_CTRL] = ctrl_pressed;
                win->keyboard.keys[OKO_KEY_ALT] = alt_pressed;

                win->keyboard.shift = shift_pressed;
                win->keyboard.ctrl = ctrl_pressed;
                win->keyboard.alt = alt_pressed;
            }
            else if (type == 1)
            {
                win->mouse.left = 1;
            }
            else if (type == 2)
            {
                win->mouse.left = 0;
            }
            else if (type == 3)
            {
                win->mouse.right = 1;
            }
            else if (type == 4)
            {
                win->mouse.right = 0;
            }
            else if (type == 25)
            {
                win->mouse.middle = 1;
            }
            else if (type == 26)
            {
                win->mouse.middle = 0;
            }
            else if (type == 5 || type == 6 || type == 7)
            {
                id contentView = msg(win->pw->wnd, "contentView");
                CGPoint loc = *(CGPoint*)&objc_msgSend(event, sel_registerName("locationInWindow"));
                CGPoint converted = *(CGPoint*)&msg2(contentView, "convertPoint:fromView:", loc, NULL);
                win->mouse.x = (int)converted.x;
                win->mouse.y = win->height - (int)converted.y;
            }
            else if (type == 14)
            {
                win->running = 0;
            }
            else if (type == 22)
            {
                CGFloat deltaX = *(CGFloat*)&msg(event, "scrollingDeltaX");
                CGFloat deltaY = *(CGFloat*)&msg(event, "scrollingDeltaY");
                
                BOOL hasPreciseScrollingDeltas = (BOOL)(long)msg(event, "hasPreciseScrollingDeltas");
                
                if (hasPreciseScrollingDeltas)
                {
                    win->mouse.scroll_x += (i32)(deltaX / 10.0);
                    win->mouse.scroll_y += (i32)(deltaY / 10.0);
                }
                else
                {
                    win->mouse.scroll_x += (i32)deltaX;
                    win->mouse.scroll_y += (i32)deltaY;
                }
            }

            msg1(app, "sendEvent:", event);
        }
    }
    while (event);

    id contentView = msg(win->pw->wnd, "contentView");
    CGRect frame = *(CGRect*)&msg(contentView, "frame");
    i32 new_width = (i32)frame.size.width;
    i32 new_height = (i32)frame.size.height;

    if (new_width != win->width || new_height != win->height)
    {
        u32* new_pixels = realloc(win->pixels, new_width * new_height * sizeof(u32));
        if (!new_pixels)
        {
            log_error("Failed to reallocate pixels buffer");
        }
        else
        {
            win->pixels = new_pixels;

            u32* new_back_buffer = realloc(win->back_buffer, new_width * new_height * sizeof(u32));
            if (!new_back_buffer)
            {
                log_error("Failed to reallocate back buffer");
            }
            else
            {
                win->back_buffer = new_back_buffer;
                win->width = new_width;
                win->height = new_height;
#ifdef OKO_LOG_RESIZE
                if (win->width % 10 == 0) log_info("Window resized to %d x %d", win->width, win->height);
#endif
            }
        }
    }

    BOOL isMiniaturized = (BOOL)(long)msg(win->pw->wnd, "isMiniaturized");
    win->showed = !isMiniaturized;

    msg(pool, "drain");
}

#include <mach/mach_time.h>

struct oko_Timer {
    mach_timebase_info_data_t timebase;
};

OKO_API oko_Timer* okoshko_timer_create(void) {
    oko_Timer* timer = malloc(sizeof(oko_Timer));
    mach_timebase_info(&timer->timebase);
    return timer;
}

OKO_API u64 okoshko_timer_now(oko_Timer* timer) {
    uint64_t now = mach_absolute_time();
    return (now * timer->timebase.numer) / (timer->timebase.denom * 1000000ULL);
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

OKO_API void oko_os_swap_buffers(oko_Window* win) {
    id contentView = msg(win->pw->wnd, "contentView");
    msg1(contentView, "setNeedsDisplay:", YES);
}