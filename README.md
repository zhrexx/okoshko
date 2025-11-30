# Okoshko

**Okoshko** (окошко) — Russian for "little window" — is a minimalist window library for portable software rendering.

> The name draws inspiration from [zserge/fenster](https://github.com/zserge/fenster), where "fenster" means "window" in German.

## What is Okoshko?

A lightweight yet powerful, highly portable window library that takes advance of 
Software Rendering for a minimal amount of platform-dependent code (3 things: create window, poll events, close window) which increases portability.

Currently, we support Linux (X11), Windows (winapi) and macOS (Obj-C)
planned are also FreeBSD, terminal, Android, iOS

## Philosophy

**Okoshko** is built around a few core principles:

- **Straightforward** — Simple/Obvious API (you should take one look into okoshko.h and understand it fully), obvious behavior, no surprises
- **Portable** — Write once, run anywhere (Currently Windows, macOS, Linux)
- **Fast** — Be fast
- **Joyful** — Simple things should be simple, complex things should be possible

## Perfect For / Where should be used

- GUI Applications
- Retro-style games and emulators
- Applications requiring precise pixel control

Or as a backbone for window creation in other libraries

## Bad For
- Games where high performance/fps are required

## Notes
- It uses a system called Modules you can import a module from the modules directory then you need to define MODULE_SOMETHING_IMPLEMENTATION
- It uses ARGB

## Quick Example
```c
#include "okoshko.h"

int main() {
    // Creates a window with default framerate limit of 60fps
    oko_Window *win = oko_create("Example Window", 800, 600);
    oko_init(); // recommended to init but not required, but be aware usage of oko_format then could segfault
    while (oko_is_running(win)) {
        oko_begin_drawing(win);
        // Clear with black
        oko_clear(okoshko, 0x00);
        // Create a red-filled rectangle at (0, 0) with size (100, 100)
        oko_fill_rect(okoshko, OKO_RECT(0, 0, 100, 100), 0xFFFF0000);
        oko_end_drawing(win);
    }
    oko_destroy(win);
    return 0;
}

```

## Getting Started

[Installation and usage documentation coming soon]

## License

MIT License

Copyright (c) 2025 Jan Moretz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

Contributions welcome! Please feel free to submit issues and pull requests.

## TODO
- Implement loading ttf files
- Add basic elements
- More modules