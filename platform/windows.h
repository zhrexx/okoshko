#pragma once
#include "../okoshko.h"
#ifdef _WIN32

static int oko_win32_key_to_index(WPARAM vk) {
    if (vk >= 'A' && vk <= 'Z') return vk;

    if (vk >= '0' && vk <= '9') return vk;

    if (vk == VK_SPACE) return OKO_KEY_SPACE;
    if (vk == VK_RETURN) return OKO_KEY_ENTER;
    if (vk == VK_ESCAPE) return OKO_KEY_ESC;
    if (vk == VK_TAB) return OKO_KEY_TAB;
    if (vk == VK_BACK) return OKO_KEY_BACKSPACE;

    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT) return OKO_KEY_SHIFT;
    if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL) return OKO_KEY_CTRL;
    if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU) return OKO_KEY_ALT;

    if (vk == VK_LEFT) return OKO_KEY_ARROW_LEFT;
    if (vk == VK_RIGHT) return OKO_KEY_ARROW_RIGHT;
    if (vk == VK_UP) return OKO_KEY_ARROW_UP;
    if (vk == VK_DOWN) return OKO_KEY_ARROW_DOWN;

    if (vk == VK_DELETE) return OKO_KEY_DELETE;
    if (vk == VK_HOME) return OKO_KEY_HOME;
    if (vk == VK_END) return OKO_KEY_END;
    if (vk == VK_PRIOR) return OKO_KEY_PAGE_UP;
    if (vk == VK_NEXT) return OKO_KEY_PAGE_DOWN;

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    oko_Window *win = (oko_Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CLOSE:
        if (win) win->running = 0;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        if (win) {
            int idx = oko_win32_key_to_index(wp);
            if (idx > 0 && idx < 256) {
                win->keyboard.keys[idx] = 1;
                if (idx == OKO_KEY_SHIFT) win->keyboard.shift = 1;
                if (idx == OKO_KEY_CTRL) win->keyboard.ctrl = 1;
                if (idx == OKO_KEY_ALT) win->keyboard.alt = 1;
            }
        }
        return 0;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        if (win) {
            int idx = oko_win32_key_to_index(wp);
            if (idx > 0 && idx < 256) {
                win->keyboard.keys[idx] = 0;

                if (idx == OKO_KEY_SHIFT) win->keyboard.shift = 0;
                if (idx == OKO_KEY_CTRL) win->keyboard.ctrl = 0;
                if (idx == OKO_KEY_ALT) win->keyboard.alt = 0;
            }
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
        if (win) win->mouse.left = 1;
        return 0;
    case WM_RBUTTONDOWN:
        if (win) win->mouse.right = 1;
        return 0;
    case WM_MBUTTONDOWN:
        if (win) win->mouse.middle = 1;
        return 0;
    case WM_LBUTTONUP:
        if (win) win->mouse.left = 0;
        return 0;
    case WM_RBUTTONUP:
        if (win) win->mouse.right = 0;
        return 0;
    case WM_MBUTTONUP:
        if (win) win->mouse.middle = 0;
        return 0;
    case WM_MOUSEMOVE:
        if (win) {
            win->mouse.x = LOWORD(lp);
            win->mouse.y = HIWORD(lp);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
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

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OkoshkoClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    win->osw.hwnd = CreateWindowA("OkoshkoClass", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  rect.right - rect.left, rect.bottom - rect.top,
                                  NULL, NULL, wc.hInstance, NULL);

    SetWindowLongPtr(win->osw.hwnd, GWLP_USERDATA, (LONG_PTR)win);
    log_info("Created Windows Window '%s' (%d, %d)", title, width, height);
    return win;
}

OKO_API void oko_destroy(oko_Window *win) {
    if (!win) return;
    DestroyWindow(win->osw.hwnd);
    free(win->pixels);
    free(win->back_buffer);
    free(win->title);
    free(win);
    log_info("Closing Window");
}

OKO_API void oko_poll_events(oko_Window *win) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            win->running = 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    win->showed = !IsIconic(win->osw.hwnd);
}
#endif