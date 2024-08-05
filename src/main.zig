const std = @import("std");
const graphics = @import("graphics.zig");
const sdl = @import("sdl.zig");
const game = @import("game.zig");

const WIDTH = 640;
const HEIGHT = 480;

pub fn main() !void {
    const SDL = try sdl.init();
    defer sdl.deinit();

    var window = try sdl.SDL.Window.init(&SDL, WIDTH, HEIGHT);
    defer window.deinit();
    
    // Create the allocator and arraylist
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    var data: game.GameData = game.GameData.init(gpa.allocator());
    defer data.deinit();

    //data.objects.append(game.GameObject{});

    // Initialize the player's position
    //data.player_pos = .{WIDTH/2, HEIGHT/2};
    // TODO: MAKE THE PLAYER INTO A GAME OBJECT AS WELL

    for (0..50) |_| {
        //@memset(window.image.pixels, graphics.Color {.r = 255, .g = 0, .b = 255, .a = 255});
        for (0..640) |x| {
            for (0..480) |y| {
                try window.image.setPixel(@intCast(x), @intCast(y), graphics.Color {.r = @intCast(data.randomizer.next() % 256), .g = @intCast(data.randomizer.next() % 256), .b = @intCast(data.randomizer.next() % 256), .a = 255});
            }
        }
        window.update(); // For some reason, a single call won't always suffice. Annoying.
        //std.debug.print("{s}\n", .{sdl.SDL_GetError()});

        std.time.sleep(10000000);
    }
}
