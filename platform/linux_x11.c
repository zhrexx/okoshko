#include "../okoshko.h"
#define _DEFAULT_SOURCE 1
#include <string.h>
#include <locale.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

struct oko_PlatformWindow {
    Display* dpy;
    Window w;
    GC gc;
    XImage* img;
    Atom wm_delete_window;
    XIM xim;
    XIC xic;
};

static int oko_x11_key_to_index(KeySym ks) {
    if (ks >= XK_a && ks <= XK_z)
        return (i32)ks - XK_a + 'a';
    if (ks >= XK_A && ks <= XK_Z)
        return (i32)ks - XK_A + 'A';
    if (ks >= XK_0 && ks <= XK_9)
        return (i32)ks - XK_0 + '0';
    if (ks == XK_space)
        return OKO_KEY_SPACE;
    if (ks == XK_Return)
        return OKO_KEY_ENTER;
    if (ks == XK_Escape)
        return OKO_KEY_ESC;

    if (ks == XK_Shift_L || ks == XK_Shift_R)
        return OKO_KEY_SHIFT;
    if (ks == XK_Control_L || ks == XK_Control_R)
        return OKO_KEY_CTRL;
    if (ks == XK_Alt_L || ks == XK_Alt_R)
        return OKO_KEY_ALT;

    if (ks == XK_Left)
        return OKO_KEY_ARROW_LEFT;
    if (ks == XK_Right)
        return OKO_KEY_ARROW_RIGHT;
    if (ks == XK_Up)
        return OKO_KEY_ARROW_UP;
    if (ks == XK_Down)
        return OKO_KEY_ARROW_DOWN;

    if (ks == XK_Tab)
        return OKO_KEY_TAB;
    if (ks == XK_BackSpace)
        return OKO_KEY_BACKSPACE;
    if (ks == XK_Delete)
        return OKO_KEY_DELETE;
    if (ks == XK_Home)
        return OKO_KEY_HOME;
    if (ks == XK_End)
        return OKO_KEY_END;
    if (ks == XK_Page_Up)
        return OKO_KEY_PAGE_UP;
    if (ks == XK_Page_Down)
        return OKO_KEY_PAGE_DOWN;

    if (ks >= 0x20 && ks <= 0x7E)
        return (i32)ks;
    // TODO: add more keys
    return 0;
}

OKO_API oko_Window* oko_create(const char* title, i32 width, i32 height) {
    setlocale(LC_ALL, "");
    
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

    win->pw->dpy = XOpenDisplay(NULL);
    if (!XSupportsLocale()) {
        log_warn("X does not support locale");
    }
    if (!XSetLocaleModifiers("")) {
        log_warn("Cannot set locale modifiers");
    }
    
    int screen = DefaultScreen(win->pw->dpy);

    win->pw->w = XCreateSimpleWindow(
        win->pw->dpy, RootWindow(win->pw->dpy, screen), 0, 0, width, height, 1,
        BlackPixel(win->pw->dpy, screen), WhitePixel(win->pw->dpy, screen));

    XStoreName(win->pw->dpy, win->pw->w, title);
    XSelectInput(win->pw->dpy, win->pw->w,
                 ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                 ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

    win->pw->gc = XCreateGC(win->pw->dpy, win->pw->w, 0, NULL);

    Visual* vis = DefaultVisual(win->pw->dpy, screen);
    win->pw->img = XCreateImage(win->pw->dpy, vis, 24, ZPixmap, 0,
                                (char*)win->pixels, width, height, 32, 0);

    win->pw->xim = XOpenIM(win->pw->dpy, NULL, NULL, NULL);
    if (win->pw->xim) {
        win->pw->xic = XCreateIC(win->pw->xim,
                                 XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                                 XNClientWindow, win->pw->w,
                                 XNFocusWindow, win->pw->w,
                                 NULL);
        if (win->pw->xic) {
            XSetICFocus(win->pw->xic);
        } else {
            log_warn("Failed to create input context");
        }
    } else {
        log_warn("Failed to open input method");
        win->pw->xic = NULL;
    }

    XMapWindow(win->pw->dpy, win->pw->w);
    win->pw->wm_delete_window =
        XInternAtom(win->pw->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(win->pw->dpy, win->pw->w, &win->pw->wm_delete_window, 1);
    log_info("Created X11 Window '%s' (%d, %d)", title, width, height);
    return win;
}

OKO_API void oko_destroy(oko_Window* win) {
    if (!win)
        return;
    if (win->pw->xic)
        XDestroyIC(win->pw->xic);
    if (win->pw->xim)
        XCloseIM(win->pw->xim);
    XFreeGC(win->pw->dpy, win->pw->gc);
    XDestroyWindow(win->pw->dpy, win->pw->w);
    XCloseDisplay(win->pw->dpy);
    free(win->pw);
    free(win->pixels);
    free(win->back_buffer);
    free(win->title);
    free(win);
    log_info("Closing Window");
}

OKO_API void oko_poll_events(oko_Window* win) {
    XEvent ev;
    while (XPending(win->pw->dpy))
    {
        XNextEvent(win->pw->dpy, &ev);
        
        if (win->pw->xic && XFilterEvent(&ev, None))
            continue;
        
        switch (ev.type)
        {
        case KeyPress:
            {
                char buf[32];
                KeySym ks;
                Status status;
                int len = 0;
                
                if (win->pw->xic) {
                    len = Xutf8LookupString(win->pw->xic, &ev.xkey, buf, sizeof(buf) - 1, &ks, &status);
                    if (status == XBufferOverflow) {
                        log_warn("UTF-8 buffer overflow");
                    }
                } else {
                    len = XLookupString(&ev.xkey, buf, sizeof(buf) - 1, &ks, NULL);
                }
                
                if (len > 0) {
                    buf[len] = '\0';
                }
                
                int idx = oko_x11_key_to_index(ks);
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
                break;
            }
        case KeyRelease:
            {
                KeySym ks = XLookupKeysym(&ev.xkey, 0);
                int idx = oko_x11_key_to_index(ks);
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
                break;
            }
        case ButtonPress:
            if (ev.xbutton.button == Button1)
                win->mouse.left = 1;
            else if (ev.xbutton.button == Button3)
                win->mouse.right = 1;
            else if (ev.xbutton.button == Button2)
                win->mouse.middle = 1;
            else if (ev.xbutton.button == Button4)
                win->mouse.scroll_y += 1;
            else if (ev.xbutton.button == Button5)
                win->mouse.scroll_y -= 1;
            else if (ev.xbutton.button == 6)
                win->mouse.scroll_x -= 1;
            else if (ev.xbutton.button == 7)
                win->mouse.scroll_x += 1;
            break;
        case ButtonRelease:
            if (ev.xbutton.button == Button1)
                win->mouse.left = 0;
            else if (ev.xbutton.button == Button3)
                win->mouse.right = 0;
            else if (ev.xbutton.button == Button2)
                win->mouse.middle = 0;
            break;
        case MotionNotify:
            win->mouse.x = ev.xmotion.x;
            win->mouse.y = ev.xmotion.y;
            break;
        case ClientMessage:
            if ((Atom)ev.xclient.data.l[0] == win->pw->wm_delete_window)
            {
                win->running = 0;
            }
            break;
        case MapNotify:
            win->showed = 1;
            break;
        case UnmapNotify:
            win->showed = 0;
            break;
        case Expose:
        case ReparentNotify:
            break;
        case ConfigureNotify:
            {
                if (ev.xconfigure.width != win->width ||
                    ev.xconfigure.height != win->height)
                {
                    i32 new_width = ev.xconfigure.width;
                    i32 new_height = ev.xconfigure.height;

                    u32* new_pixels = realloc(win->pixels,
                                              new_width * new_height * sizeof(u32));
                    if (!new_pixels) {
                        log_error("Failed to reallocate pixels buffer");
                        break;
                    }
                    win->pixels = new_pixels;

                    u32* new_back_buffer = realloc(win->back_buffer,
                                                    new_width * new_height * sizeof(u32));
                    if (!new_back_buffer) {
                        log_error("Failed to reallocate back buffer");
                        break;
                    }
                    win->back_buffer = new_back_buffer;

                    if (win->pw->img) {
                        win->pw->img->data = NULL;
                        XDestroyImage(win->pw->img);
                    }

                    win->width = new_width;
                    win->height = new_height;

                    int screen = DefaultScreen(win->pw->dpy);
                    Visual* vis = DefaultVisual(win->pw->dpy, screen);
                    win->pw->img = XCreateImage(win->pw->dpy, vis, 24, ZPixmap, 0,
                                                (char*)win->pixels, win->width, win->height,
                                                32, 0);
#ifdef OKO_LOG_RESIZE
                    if (win->width % 10 == 0) log_info("Window resized to %d x %d", win->width, win->height);
#endif
                }
                break;
            }
        default:
            log_warn("Unhandled case: %u", ev.type);
            break;
        }
    }
}

struct oko_Timer {
};

OKO_API oko_Timer* okoshko_timer_create() {
    return NULL;
}

OKO_API u64 okoshko_timer_now(oko_Timer* timer) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (u64)now.tv_sec * 1000
         + (u64)now.tv_nsec / 1000000;
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

OKO_API void oko_os_swap_buffers(oko_Window* win) {
    XPutImage(win->pw->dpy, win->pw->w, win->pw->gc, win->pw->img, 0, 0, 0, 0,
              win->width, win->height);
    XFlush(win->pw->dpy);
}