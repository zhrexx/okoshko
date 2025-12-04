#pragma once

#include "../okoshko.h"

#define OKO_RECT(x, y, w, h) ((oko_Rect){x, y, w, h})
#define OKO_POINT(x, y) ((oko_Point){x, y})
#define OKO_RGB(r, g, b) (0xFF000000 | ((r) << 16) | ((g) << 8) | (b))
#define OKO_RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define OKO_KEY_ESC 27
#define OKO_KEY_SPACE ' '
#define OKO_KEY_ENTER '\r'
#define OKO_KEY_TAB '\t'
#define OKO_KEY_BACKSPACE '\b'
#define OKO_KEY_SHIFT 128
#define OKO_KEY_CTRL 129
#define OKO_KEY_ALT 130
#define OKO_KEY_ARROW_LEFT 131
#define OKO_KEY_ARROW_RIGHT 132
#define OKO_KEY_ARROW_UP 133
#define OKO_KEY_ARROW_DOWN 134
#define OKO_KEY_DELETE 135
#define OKO_KEY_HOME 136
#define OKO_KEY_END 137
#define OKO_KEY_PAGE_UP 138
#define OKO_KEY_PAGE_DOWN 139
// #define OKO_KEY_

// Mouse
#define OKO_MOUSE_LEFT 1
#define OKO_MOUSE_RIGHT 2
#define OKO_MOUSE_MIDDLE 3
