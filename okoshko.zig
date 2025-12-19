// TODO: proper wrapper not just binding

const std = @import("std");

pub const c = @cImport({
    @cInclude("./okoshko.h");
});

pub const Rect = extern struct {
    x: i32,
    y: i32,
    w: i32,
    h: i32,
};

pub const Point = extern struct {
    x: i32,
    y: i32,
};

pub const Keyboard = extern struct {
    keys: [256]u8,
    prev_keys: [256]u8,
    ctrl: u8,
    shift: u8,
    alt: u8,
    meta: u8,
};

pub const Mouse = extern struct {
    x: i32,
    y: i32,
    left: u8,
    right: u8,
    middle: u8,
    scroll_x: i32,
    scroll_y: i32,
};

pub const Glyph = extern struct {
    character: u8,
    width: i32,
    height: i32,
    advance: i32,
    offsetX: i32,
    offsetY: i32,
    bitmap: ?[*]u8,
};

pub const Font = extern struct {
    size: i32,
    ascent: i32,
    descent: i32,
    lineGap: i32,
    glyphCount: i32,
    glyphs: ?[*]Glyph,
};

pub const EventType = enum(c_int) {
    no_event = c.OKO_NO_EVENT,
};

pub const EventCallback = *const fn (?*anyopaque) callconv(.C) void;

pub const Timer = opaque {};
pub const PlatformWindow = opaque {};

pub const Window = c.oko_Window;

pub const OkoError = error{
InitFailed,
CreateFailed,
InvalidWindow,
InvalidFont,
};

pub fn getLastError() ?[]const u8 {
    if (c.oko_error) |err| {
        return std.mem.span(err);
    }
    return null;
}

pub fn getLastError2() ?[]const u8 {
    if (c.oko_error2) |err| {
        return std.mem.span(err);
    }
    return null;
}

pub fn init() void {
    c.oko_init();
}

pub fn createWindow(title: [:0]const u8, width: i32, height: i32) OkoError!*Window {
    const win = c.oko_create(title.ptr, width, height);
    return win orelse OkoError.CreateFailed;
}

pub fn destroyWindow(win: *Window) void {
    c.oko_destroy(win);
}

pub fn setFps(win: *Window, fps: u32) void {
    c.oko_set_fps(win, fps);
}

pub fn getFps(win: *Window) u32 {
    return c.oko_get_fps(win);
}

pub fn isRunning(win: *const Window) bool {
    return c.oko_is_running(win) != 0;
}

pub fn createTimer() OkoError!*Timer {
    const timer = c.okoshko_timer_create();
    return timer orelse OkoError.InitFailed;
}

pub fn timerNow(timer: *Timer) u64 {
    return c.okoshko_timer_now(timer);
}

pub fn timerSleep(ms: u64) void {
    c.okoshko_timer_sleep(ms);
}

pub fn swapBuffers(win: *Window) void {
    c.oko_os_swap_buffers(win);
}

pub fn beginFrame(win: *Window) void {
    c.oko_begin_frame(win);
}

pub fn endFrame(win: *Window) void {
    c.oko_end_frame(win);
}

pub fn pollEvents(win: *Window) void {
    c.oko_poll_events(win);
}

pub fn clear(win: *Window, color: u32) void {
    c.oko_clear(win, color);
}

pub fn setPixel(win: *Window, x: i32, y: i32, color: u32) void {
    c.oko_set_pixel(win, x, y, color);
}

pub fn getPixel(win: *Window, x: i32, y: i32) u32 {
    return c.oko_get_pixel(win, x, y);
}

pub fn fillRect(win: *Window, rect: Rect, color: u32) void {
    c.oko_fill_rect(win, @bitCast(rect), color);
}

pub fn fillCircle(win: *Window, cx: i32, cy: i32, radius: i32, color: u32) void {
    c.oko_fill_circle(win, cx, cy, radius, color);
}

pub fn drawRect(win: *Window, rect: Rect, color: u32) void {
    c.oko_draw_rect(win, @bitCast(rect), color);
}

pub fn drawLine(win: *Window, x0: i32, y0: i32, x1: i32, y1: i32, color: u32) void {
    c.oko_draw_line(win, x0, y0, x1, y1, color);
}

pub fn drawCircle(win: *Window, cx: i32, cy: i32, radius: i32, color: u32) void {
    c.oko_draw_circle(win, cx, cy, radius, color);
}

pub fn drawText(win: *Window, text: [:0]const u8, font: *Font, x: i32, y: i32, scale: f32, color: u32) void {
    c.oko_draw_text(win, text.ptr, font, x, y, scale, color);
}

pub fn keyDown(win: *Window, key: u8) bool {
    return c.oko_key_down(win, key) != 0;
}

pub fn keyPressed(win: *Window, key: u8) bool {
    return c.oko_key_pressed(win, key) != 0;
}

pub fn keyReset(win: *Window, key: u8) void {
    c.oko_key_reset(win, key);
}

pub fn mouseDown(win: *Window, button: u8) bool {
    return c.oko_mouse_down(win, button) != 0;
}

pub fn timeMs(win: *Window) u64 {
    return c.oko_time_ms(win);
}

pub fn sleep(ms: u64) void {
    c.oko_sleep(ms);
}

pub fn createGlyph(bitmap: *[*]u8, width: i32, height: i32, startX: i32, character: u8) Glyph {
    return @bitCast(c.oko_create_glyph(bitmap, width, height, startX, character));
}

pub fn bitmapToFont(
    bitmap: *[*]u8,
    totalWidth: i32,
    totalHeight: i32,
    glyphWidth: i32,
    glyphCount: i32,
    startChar: u8,
) OkoError!*Font {
    const font = c.oko_bitmap_to_font(bitmap, totalWidth, totalHeight, glyphWidth, glyphCount, startChar);
    return font orelse OkoError.InvalidFont;
}

pub fn freeFont(font: *Font) void {
    c.oko_free_font(font);
}

pub fn format(allocator: std.mem.Allocator, comptime fmt: []const u8, args: anytype) ![]u8 {
    return std.fmt.allocPrint(allocator, fmt, args);
}

pub fn getTempAllocator() *c.oko_temp_allocator {
    return c.oko_get_temp_allocator();
}

pub const Color = struct {
    pub fn rgba(r: u8, g: u8, b: u8, a: u8) u32 {
        return (@as(u32, a) << 24) | (@as(u32, b) << 16) | (@as(u32, g) << 8) | @as(u32, r);
    }

    pub fn rgb(r: u8, g: u8, b: u8) u32 {
        return rgba(r, g, b, 255);
    }

    pub const BLACK = rgb(0, 0, 0);
    pub const WHITE = rgb(255, 255, 255);
    pub const RED = rgb(255, 0, 0);
    pub const GREEN = rgb(0, 255, 0);
    pub const BLUE = rgb(0, 0, 255);
    pub const YELLOW = rgb(255, 255, 0);
    pub const CYAN = rgb(0, 255, 255);
    pub const MAGENTA = rgb(255, 0, 255);
};