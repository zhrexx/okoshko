// BUILD: zig build-exe example.zig platform/linux_x11.c okoshko.c helpers/log.c -lc -I. -lX11
const oko = @import("okoshko.zig");

pub fn main() !void {
    const app = try oko.createWindow("Hi Zig!", 800, 600);

    while (oko.isRunning(app)) {}
    oko.destroyWindow(app);
}