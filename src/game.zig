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
    update_func: *fn (game: GameData, self: *@This()) void,
    draw_func: *fn (game: GameData, self: *@This()) graphics.DrawError!void
};

/// A collection of data representing the game's data
pub const GameData = struct {
    objects: std.ArrayList(GameObject),
    running: bool = true,
    //player_pos: @Vector(2, f32),
    mouse_pos: @Vector(2, f32),
    randomizer: std.Random.Xoshiro256,
    //window: sdl.SDL.Window,
    enemy_speed: f32 = 1.0,

    /// Initializes the game data
    pub fn init(allocator: std.mem.Allocator) GameData {
        return GameData {
            .objects = std.ArrayList(GameObject).init(allocator),
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

    //pub fn CreateLaser(self: *GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
    //    const laser = GameObject {
    //         .pos = .{x, y},
    //         .vel = 10*(self.mouse_pos - self.player_pos)/@sqrt((self.mouse_pos[0] - self.player_pos[0])*(self.mouse_pos[0] - self.player_pos[0]) + (self.mouse_pos[1] - self.player_pos[1])*(self.mouse_pos[1] - self.player_pos[1])),
    //         .length = length,
    //         .color = color,
    //         .update_func =  UpdateLaser,
    //         .draw_func = graphics.DrawLaser
    //     };

    //     self.object_list.append(laser);
    // }

    // pub fn CreateEnemy(self: *GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
    //     //var try_pos: @Vector(2, f32) = .{self.randomizer.float(f32)*self.window.image.width, self.randomizer.float(f32)*self.window.image.height};

    //     // Ensure that any spawned enemies are at least 100 units away from the player
    //     //while (@sqrt((self.player_pos[0] - try_pos[0])*(self.player_pos[0] - try_pos[0]) + (self.player_pos[1] - try_pos[1])*(self.player_pos[1] - try_pos[1])) < 100) {
    //     //    try_pos = .{self.randomizer.float(f32)*self.window.image.width, self.randomizer.float(f32)*self.window.image.height};
    //     //}

    //     const enemy = GameObject {
    //         .pos = .{x, y},
    //         .length = length,
    //         .color = color,
    //         .update_func =  UpdateEnemy,
    //         .draw_func = graphics.DrawEnemy
    //     };

    //     self.object_list.append(enemy);
    // }

    pub fn createObject(self: *GameData, object: GameObject) !void {
        try self.object_list.append(object);
    }

    pub fn destroyObject(self: *GameData, index: usize) !void {
        try self.object_list.swapRemove(index);
    }
};

fn updateLaser(game: GameData, self: *GameObject) void {
    _ = game;
    self.pos += self.vel;
}

fn updateEnemy(game: GameData, self: *GameObject) void {
    self.pos += game.enemy_speed*(game.player_pos - self.pos) / @sqrt((game.player_pos[0] - self.pos[0])*(game.player_pos[0] - self.pos[0]) + (game.player_pos[1] - self.pos[1])*(game.player_pos[1] - self.pos[1]));
}