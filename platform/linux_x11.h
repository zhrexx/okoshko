#pragma once

#include "../okoshko.h"

#ifdef OKO_LINUX

static int oko_x11_key_to_index(KeySym ks) {
    if (ks >= XK_a && ks <= XK_z) return (i32)ks - XK_a + 'a';
    if (ks >= XK_A && ks <= XK_Z) return (i32)ks - XK_A + 'A';
    if (ks >= XK_0 && ks <= XK_9) return (i32)ks - XK_0 + '0';
    if (ks == XK_space) return OKO_KEY_SPACE;
    if (ks == XK_Return) return OKO_KEY_ENTER;
    if (ks == XK_Escape) return OKO_KEY_ESC;

    if (ks == XK_Shift_L || ks == XK_Shift_R) return OKO_KEY_SHIFT;
    if (ks == XK_Control_L || ks == XK_Control_R) return OKO_KEY_CTRL;
    if (ks == XK_Alt_L || ks == XK_Alt_R) return OKO_KEY_ALT;

    if (ks == XK_Left) return OKO_KEY_ARROW_LEFT;
    if (ks == XK_Right) return OKO_KEY_ARROW_RIGHT;
    if (ks == XK_Up) return OKO_KEY_ARROW_UP;
    if (ks == XK_Down) return OKO_KEY_ARROW_DOWN;

    if (ks == XK_Tab) return OKO_KEY_TAB;
    if (ks == XK_BackSpace) return OKO_KEY_BACKSPACE;
    if (ks == XK_Delete) return OKO_KEY_DELETE;
    if (ks == XK_Home) return OKO_KEY_HOME;
    if (ks == XK_End) return OKO_KEY_END;
    if (ks == XK_Page_Up) return OKO_KEY_PAGE_UP;
    if (ks == XK_Page_Down) return OKO_KEY_PAGE_DOWN;

    return 0;
}


OKO_API oko_Window* oko_create(const char *title, i32 width, i32 height) {
    oko_Window *win = calloc(1, sizeof(oko_Window));
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

    win->osw.dpy = XOpenDisplay(nullptr);
    int screen = DefaultScreen(win->osw.dpy);

    win->osw.w = XCreateSimpleWindow(win->osw.dpy, RootWindow(win->osw.dpy, screen),
                                     0, 0, width, height, 1,
                                     BlackPixel(win->osw.dpy, screen),
                                     WhitePixel(win->osw.dpy, screen));

    XStoreName(win->osw.dpy, win->osw.w, title);
    XSelectInput(win->osw.dpy, win->osw.w,
                 ExposureMask | KeyPressMask | KeyReleaseMask |
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

    win->osw.gc = XCreateGC(win->osw.dpy, win->osw.w, 0, nullptr);

    Visual *vis = DefaultVisual(win->osw.dpy, screen);
    win->osw.img = XCreateImage(win->osw.dpy, vis, 24, ZPixmap, 0,
                                (char *)win->pixels, width, height, 32, 0);

    XMapWindow(win->osw.dpy, win->osw.w);
    win->osw.wm_delete_window = XInternAtom(win->osw.dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(win->osw.dpy, win->osw.w, &win->osw.wm_delete_window, 1);
    log_info("Created X11 Window '%s' (%d, %d)", title, width, height);
    return win;
}

OKO_API void oko_destroy(oko_Window *win) {
    if (!win) return;
    XFreeGC(win->osw.dpy, win->osw.gc);
    XDestroyWindow(win->osw.dpy, win->osw.w);
    XCloseDisplay(win->osw.dpy);
    free(win->pixels);
    free(win->back_buffer);
    free(win->title);
    free(win);
    log_info("Closing Window");
}

OKO_API void oko_poll_events(oko_Window *win) {
    XEvent ev;
    while (XPending(win->osw.dpy)) {
        XNextEvent(win->osw.dpy, &ev);
        switch (ev.type) {
        case KeyPress: {
            int idx = oko_x11_key_to_index(XLookupKeysym(&ev.xkey, 0));
            if (idx > 0 && idx < 256) {
                win->keyboard.keys[idx] = 1;
                if (idx == OKO_KEY_SHIFT) win->keyboard.shift = 1;
                if (idx == OKO_KEY_CTRL) win->keyboard.ctrl = 1;
                if (idx == OKO_KEY_ALT) win->keyboard.alt = 1;
            }
            break;
        }
        case KeyRelease: {
            int idx = oko_x11_key_to_index(XLookupKeysym(&ev.xkey, 0));
            if (idx > 0 && idx < 256) {
                win->keyboard.keys[idx] = 0;
                if (idx == OKO_KEY_SHIFT) win->keyboard.shift = 0;
                if (idx == OKO_KEY_CTRL) win->keyboard.ctrl = 0;
                if (idx == OKO_KEY_ALT) win->keyboard.alt = 0;
            }
            break;
        }
        case ButtonPress:
            if (ev.xbutton.button == Button1) win->mouse.left = 1;
            else if (ev.xbutton.button == Button3) win->mouse.right = 1;
            else if (ev.xbutton.button == Button2) win->mouse.middle = 1;
            break;
        case ButtonRelease:
            if (ev.xbutton.button == Button1) win->mouse.left = 0;
            else if (ev.xbutton.button == Button3) win->mouse.right = 0;
            else if (ev.xbutton.button == Button2) win->mouse.middle = 0;
            break;
        case MotionNotify:
            win->mouse.x = ev.xmotion.x;
            win->mouse.y = ev.xmotion.y;
            break;
        case ClientMessage:
            if ((Atom)ev.xclient.data.l[0] == win->osw.wm_delete_window) {
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
        case ConfigureNotify:
            break;
        default: log_warn("Unhandled case: %u", ev.type); break;
        }
    }
}

// TIMER STUFF
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

// AUDIO STUFF
int snd_pcm_open(void **, const char *, i32, i32);
int snd_pcm_set_params(void *pcm, i32 format, i32 access, i32 channels, i32 rate, i32 soft_resample, i32 latency);
int snd_pcm_avail(void *);
int snd_pcm_writei(void *, const void *, u64);
int snd_pcm_recover(void *, i32, i32);
int snd_pcm_close(void *);

struct oko_OsAudioSystem {
    void *pcm;
    f32 buf[OKO_AUDIO_BUFFER_SIZE];
    u64 pos;
};

OKO_API oko_OsAudioSystem* oko_os_audio_create(u64 sample_rate, u64 buffer_size) {
    oko_OsAudioSystem *os_audio = malloc(sizeof(oko_OsAudioSystem));
    if (snd_pcm_open(&os_audio->pcm, "default", 0, 0)) return NULL;
    int fmt = (*(unsigned char *)(&(uint16_t){1})) ? 14 : 15; // Checking Endiness
    if (!snd_pcm_set_params(os_audio->pcm, fmt, 3, 1, sample_rate, 1, 100000))
    {
        snd_pcm_close(os_audio->pcm);
        free(os_audio);
        return NULL;
    }
    return os_audio;
}

OKO_API void oko_os_audio_destroy(oko_OsAudioSystem *os_audio) {
    snd_pcm_close(os_audio->pcm);
}

OKO_API u64 oko_os_audio_get_available_frames(oko_OsAudioSystem *os_audio) {
    i32 n = snd_pcm_avail(os_audio->pcm);
    if (n < 0) snd_pcm_recover(os_audio->pcm, n, 0);
    return n;
}

OKO_API i32 oko_os_audio_submit_buffer(oko_OsAudioSystem *os_audio, const f32 *buffer, u64 n) {
    int r = snd_pcm_writei(os_audio->pcm, buffer, n);
    if (r < 0)
        snd_pcm_recover(os_audio->pcm, r, 0);
    return r;
}



#endif