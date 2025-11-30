# Okoshko

**Okoshko** (окошко) — Russian for "little window" — is a minimalist window library for portable software rendering.

> The name draws inspiration from [zserge/fenster](https://github.com/zserge/fenster), which means "window" in German. Continuing the multilingual tradition, we chose the Russian diminutive for its charm and approachability.

## What is Okoshko?

A lightweight yet powerful, highly portable window library that takes advance of 
Software Rendering for minimal amound of platform-dependent code which increases portability.

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

[Your license here]

## Contributing

Contributions welcome! Please feel free to submit issues and pull requests.