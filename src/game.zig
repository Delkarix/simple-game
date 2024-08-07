const std = @import("std");
const graphics = @import("graphics.zig");
const collision = @import("collision.zig");
//const sdl = @import("sdl.zig");

/// Represents an object within the game.
pub const GameObject = struct {
    pos: @Vector(2, f32) = .{0, 0},
    direction: @Vector(2, f32) = .{0, 0},
    velocity: f32 = 0,
    length: f32 = 0,
    color: graphics.Color = .{.r = 0, .g = 0, .b = 0},
    value: i64 = 0,
    update_func: ?*const fn (game: *GameData, self: *@This()) void = null,
    draw_func: ?*const fn (image: *graphics.Image, self: *const @This()) graphics.DrawError!void = null,
    collision_data: ?*const collision.CollisionData = null,
    data: ?*anyopaque = null,
    parent: *GameData,
    invalid: bool = false,
    dyn_alloc: bool = false, // Set this to true if the object was dynamically allocated and needs to be freed
    type_id: u8,
};

/// A collection of data representing the game's data
pub const GameData = struct {
    objects: std.ArrayList(*GameObject),
    running: bool = true,
    paused: bool = false,
    randomizer: std.Random.Xoshiro256,
    //window: sdl.SDL.Window,
    keyboard: [128]bool = undefined,
    frames: u32 = 0,
    curr_fps: u32 = 0,
    target_fps: u32 = 0, // 0 = unlimited
    score: u16 = 0,
    game_over: bool = false,

    /// Initializes the game data
    pub fn init(allocator: std.mem.Allocator) GameData {
        return GameData {
            .objects = std.ArrayList(*GameObject).init(allocator),
            .randomizer = std.Random.DefaultPrng.init(@intCast(std.time.timestamp())), // Probably should update in the future so it uses the current time or something,
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
