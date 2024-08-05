const std = @import("std");
const graphics = @import("graphics.zig");
//const sdl = @import("sdl.zig");

/// Represents an object within the game.
pub const GameObject = struct {
    pos: @Vector(2, f32),
    vel: @Vector(2, f32) = .{0, 0},
    length: f32,
    color: graphics.Color,
    health: u8 = 100,
    update_func: *const fn (game: GameData, self: *@This()) void,
    draw_func: *const fn (image: *graphics.Image, self: *const @This()) graphics.DrawError!void
};

/// A collection of data representing the game's data
pub const GameData = struct {
    objects: std.ArrayList(*GameObject),
    running: bool = true,
    paused: bool = false,
    //player_pos: @Vector(2, f32),
    mouse_pos: @Vector(2, f32),
    randomizer: std.Random.Xoshiro256,
    //window: sdl.SDL.Window,
    enemy_speed: f32 = 10,
    keys: Keyboard = .{},

    /// Initializes the game data
    pub fn init(allocator: std.mem.Allocator) GameData {
        return GameData {
            .objects = std.ArrayList(*GameObject).init(allocator),
            .randomizer = std.Random.DefaultPrng.init(@intCast(std.time.timestamp())), // Probably should update in the future so it uses the current time or something,
            //.player_pos = .{0, 0},
            .mouse_pos = .{0, 0}
        };
    }

    /// Deinitializes the game data
    pub fn deinit(self: *@This()) void {
        self.objects.deinit();
        self.* = undefined;
    }


    pub fn createObject(self: *GameData, object: GameObject) !void {
        try self.object_list.append(object);
    }

    pub fn destroyObject(self: *GameData, index: usize) !void {
        try self.object_list.swapRemove(index);
    }
};

pub const Keyboard = struct {
    w_down: bool = false,
    a_down: bool = false,
    s_down: bool = false,
    d_down: bool = false
};
