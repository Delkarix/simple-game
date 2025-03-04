const std = @import("std");
const graphics = @import("graphics.zig");
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
    collision_data: ?*const CollisionData = null,
    has_collision: bool = false, // Determines whether an object will accept collisions. If they have valid collision_data, they can still invoke collision functions on other objects.
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
    randomizer: std.Random,
    //window: sdl.SDL.Window,
    keyboard: [128]bool = undefined,
    frames: u32 = 0,
    curr_fps: u32 = 0,
    target_fps: u32 = 0, // 0 = unlimited
    score: u16 = 0,
    game_over: bool = false,
    thread_count: usize,

    /// Initializes the game data
    pub fn init(allocator: std.mem.Allocator, randomizer: std.Random) GameData {
        return GameData {
            .objects = std.ArrayList(*GameObject).init(allocator),
            .randomizer = randomizer, // Probably should update in the future so it uses the current time or something
            .keyboard = std.mem.zeroes([128]bool),
            .thread_count = std.Thread.getCpuCount() catch 1,
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

pub const CollisionData = struct {
    test_func: *const fn (self: *GameObject, collider: *GameObject) bool,
    collision_func: *const fn (game_data: *GameData, self: *GameObject, collider: *GameObject) void
};
