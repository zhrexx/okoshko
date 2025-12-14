#include "../okoshko.h"
#include <mmsystem.h>
#include <windows.h>

struct oko_PlatformWindow {
    HWND hwnd;
};

static int oko_win32_key_to_index(WPARAM vk) {
    if (vk >= 'A' && vk <= 'Z')
        return vk;

    if (vk >= '0' && vk <= '9')
        return vk;

    if (vk == VK_SPACE)
        return OKO_KEY_SPACE;
    if (vk == VK_RETURN)
        return OKO_KEY_ENTER;
    if (vk == VK_ESCAPE)
        return OKO_KEY_ESC;
    if (vk == VK_TAB)
        return OKO_KEY_TAB;
    if (vk == VK_BACK)
        return OKO_KEY_BACKSPACE;

    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT)
        return OKO_KEY_SHIFT;
    if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL)
        return OKO_KEY_CTRL;
    if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
        return OKO_KEY_ALT;

    if (vk == VK_LEFT)
        return OKO_KEY_ARROW_LEFT;
    if (vk == VK_RIGHT)
        return OKO_KEY_ARROW_RIGHT;
    if (vk == VK_UP)
        return OKO_KEY_ARROW_UP;
    if (vk == VK_DOWN)
        return OKO_KEY_ARROW_DOWN;

    if (vk == VK_DELETE)
        return OKO_KEY_DELETE;
    if (vk == VK_HOME)
        return OKO_KEY_HOME;
    if (vk == VK_END)
        return OKO_KEY_END;
    if (vk == VK_PRIOR)
        return OKO_KEY_PAGE_UP;
    if (vk == VK_NEXT)
        return OKO_KEY_PAGE_DOWN;

    if (vk >= VK_OEM_1 && vk <= VK_OEM_3)
        return vk;
    if (vk >= VK_OEM_4 && vk <= VK_OEM_8)
        return vk;
    if (vk == VK_OEM_PLUS || vk == VK_OEM_COMMA || vk == VK_OEM_MINUS || vk == VK_OEM_PERIOD)
        return vk;

    // TODO: add more keys

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    oko_Window* win = (oko_Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CLOSE:
        if (win)
            win->running = 0;
        return 0;
    case WM_CHAR:
        if (win && win->text_input_callback && wp >= 32 && wp != 127)
        {
            char utf8[5];
            int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)&wp, 1, utf8, sizeof(utf8) - 1, NULL, NULL);
            if (len > 0)
            {
                utf8[len] = '\0';
                win->text_input_callback(win, utf8);
            }
        }
        return 0;
    case WM_UNICHAR:
        if (wp == UNICODE_NOCHAR)
            return TRUE;
        if (win && win->text_input_callback && wp >= 32)
        {
            WCHAR wc = (WCHAR)wp;
            char utf8[5];
            int len = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8, sizeof(utf8) - 1, NULL, NULL);
            if (len > 0)
            {
                utf8[len] = '\0';
                win->text_input_callback(win, utf8);
            }
        }
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (win)
        {
            int idx = oko_win32_key_to_index(wp);
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
        }
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (win)
        {
            int idx = oko_win32_key_to_index(wp);
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
        return 0;
    case WM_LBUTTONDOWN:
        if (win)
            win->mouse.left = 1;
        return 0;
    case WM_RBUTTONDOWN:
        if (win)
            win->mouse.right = 1;
        return 0;
    case WM_MBUTTONDOWN:
        if (win)
            win->mouse.middle = 1;
        return 0;
    case WM_LBUTTONUP:
        if (win)
            win->mouse.left = 0;
        return 0;
    case WM_RBUTTONUP:
        if (win)
            win->mouse.right = 0;
        return 0;
    case WM_MBUTTONUP:
        if (win)
            win->mouse.middle = 0;
        return 0;
    case WM_MOUSEMOVE:
        if (win)
        {
            win->mouse.x = LOWORD(lp);
            win->mouse.y = HIWORD(lp);
        }
        return 0;
    case WM_SIZE:
        if (win && (wp == SIZE_RESTORED || wp == SIZE_MAXIMIZED))
        {
            i32 new_width = LOWORD(lp);
            i32 new_height = HIWORD(lp);

            if (new_width != win->width || new_height != win->height)
            {
                u32* new_pixels = realloc(win->pixels, new_width * new_height * sizeof(u32));
                if (!new_pixels)
                {
                    log_error("Failed to reallocate pixels buffer");
                    return 0;
                }
                win->pixels = new_pixels;

                u32* new_back_buffer = realloc(win->back_buffer, new_width * new_height * sizeof(u32));
                if (!new_back_buffer)
                {
                    log_error("Failed to reallocate back buffer");
                    return 0;
                }
                win->back_buffer = new_back_buffer;

                win->width = new_width;
                win->height = new_height;
#ifdef OKO_LOG_RESIZE
                if (win->width % 10 == 0) log_info("Window resized to %d x %d", win->width, win->height);
#endif
            }
        }
        return 0;
    case WM_MOUSEWHEEL:
        if (win)
        {
            i32 delta = GET_WHEEL_DELTA_WPARAM(wp);
            win->mouse.scroll_y += delta / WHEEL_DELTA;
        }
        return 0;
    case WM_MOUSEHWHEEL:
        if (win)
        {
            i32 delta = GET_WHEEL_DELTA_WPARAM(wp);
            win->mouse.scroll_x += delta / WHEEL_DELTA;
        }
        return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
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

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OkoshkoClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    win->pw->hwnd = CreateWindowA("OkoshkoClass", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
                                  rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);

    SetWindowLongPtr(win->pw->hwnd, GWLP_USERDATA, (LONG_PTR)win);
    log_info("Created Windows Window '%s' (%d, %d)", title, width, height);
    return win;
}

OKO_API void oko_destroy(oko_Window* win) {
    if (!win)
        return;
    DestroyWindow(win->pw->hwnd);
    free(win->pw);
    free(win->pixels);
    free(win->back_buffer);
    free(win->title);
    free(win);
    log_info("Closing Window");
}

OKO_API void oko_poll_events(oko_Window* win) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            win->running = 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    win->showed = !IsIconic(win->pw->hwnd);
}

struct oko_Timer {
    LARGE_INTEGER frequency;
};

OKO_API oko_Timer* okoshko_timer_create(void) {
    oko_Timer* timer = malloc(sizeof(oko_Timer));
    QueryPerformanceFrequency(&timer->frequency);
    return timer;
}

OKO_API u64 okoshko_timer_now(oko_Timer* timer) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (u64)((now.QuadPart * 1000) / timer->frequency.QuadPart);
}

OKO_API void okoshko_timer_sleep(u64 ms) {
    Sleep((DWORD)ms);
}

OKO_API void oko_os_swap_buffers(oko_Window* win) {
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
}