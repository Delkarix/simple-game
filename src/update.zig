const std = @import("std");
const sdl = @import("engine/sdl.zig"); // This might fail if sdl is not initialized yet
const game = @import("engine/game.zig");
const main = @import("main.zig");
const collision = @import("collision.zig");
const draw = @import("draw.zig");



// Updates the player.
pub fn updatePlayer(game_obj: *game.GameData, self: *game.GameObject) void {
    // Update player velocity
    if (game_obj.keyboard['w'] and self.pos[1] > self.length) {
        self.pos[1] -= self.velocity;
    }
    else if (game_obj.keyboard['s'] and self.pos[1] < main.HEIGHT - self.length) {
        self.pos[1] += self.velocity;
    }

    if (game_obj.keyboard['a'] and self.pos[0] > self.length) {
        self.pos[0] -= self.velocity;
    }
    else if (game_obj.keyboard['d'] and self.pos[0] < main.WIDTH - self.length) {
        self.pos[0] += self.velocity;
    }
}

// Updates the laser
pub fn updateLaser(game_data: *game.GameData, self: *game.GameObject) void {
    _ = game_data;
    self.pos += self.direction*@as(@Vector(2, f32), @splat(self.velocity));

    const end = self.pos + @as(@Vector(2, f32), @splat(self.length))*self.direction;
    if (end[0] < 0 or end[0] >= main.WIDTH or end[1] < 0 or end[1] >= main.HEIGHT) {
        self.invalid = true;
    }
}

// Updates the enemy
pub fn updateEnemy(game_data: *game.GameData, self: *game.GameObject) void {
    const player = game_data.objects.items[0]; // This *should* be the player.
    self.direction = (player.pos - self.pos) / @as(@Vector(2, f32), @splat(std.math.hypot(player.pos[0] - self.pos[0], player.pos[1] - self.pos[1])));
    self.pos += self.direction*@as(@Vector(2, f32), @splat(self.velocity));
}

pub fn updateSpawnEnemy(game_data: *game.GameData, self: *game.GameObject) void {
    const timestamp = std.time.milliTimestamp();

    // Wait a specified amount of time before attempting to spawn another enemy
    if (self.value + @as(i64, @intFromFloat(self.length)) <= timestamp) {
        self.pos = game_data.objects.items[0].pos; // This *should* be the player

        // Attempt to find a viable position that is reasonably distant from the player
        var try_pos: @Vector(2, f32) = .{game_data.randomizer.random().float(f32)*main.WIDTH, game_data.randomizer.random().float(f32)*main.HEIGHT};
        while (std.math.hypot(self.pos[0] - try_pos[0], self.pos[1] - try_pos[1]) < 100 or try_pos[0] <= 20/2 or try_pos[0] >= main.WIDTH - 20/2 or try_pos[1] <= 20/2 or try_pos[1] >= main.HEIGHT - 20/2) {
            try_pos = .{game_data.randomizer.random().float(f32)*main.WIDTH, game_data.randomizer.random().float(f32)*main.HEIGHT};
        }
        // std.debug.print("{}\n", .{try_pos});
        
        const mem_pool = @as(*std.heap.MemoryPool(game.GameObject), @alignCast(@ptrCast(self.data.?)));
        const enemy_ptr = mem_pool.create() catch {return;}; // If this fails, we're fucked because we can't return an error
        enemy_ptr.* = game.GameObject {
            .pos = try_pos,
            .velocity = self.velocity,
            .length = 20,
            .color = self.color,
            .update_func = &updateEnemy,
            .draw_func = &draw.drawSquare,
            .collision_data = &collision.enemy_collision,
            .parent = @constCast(game_data),
            .dyn_alloc = true,
            .type_id = @intFromEnum(main.TYPES.ENEMY),
        };
        game_data.objects.append(enemy_ptr) catch {return;}; // If this fails, we're fucked because we can't return an error
        self.value = timestamp;
    }
}

pub fn updateStatus(game_data: *game.GameData, self: *game.GameObject) void {
    const timestamp = std.time.milliTimestamp();

    if (self.value + 1000 <= timestamp) {
        game_data.curr_fps = game_data.frames;
        game_data.frames = 0;
        self.value = timestamp;
    }
}

pub fn updateTarget(game_data: *game.GameData, self: *game.GameObject) void {
    // Getting the mouse info from here instead of saving it somewhere for every mouse movement event is technically more efficient and better on resources
    _ = game_data;

    var x_temp: f32 = undefined;
    var y_temp: f32 = undefined;
    _ = sdl.SDL_GetMouseState(&x_temp, &y_temp);

    const mouse_loc: @Vector(2, f32) = .{x_temp, y_temp};

    // if (self.length/2 < x_temp and x_temp < WIDTH - self.length/2) {
    //     self.pos[0] = x_temp;
    // }

    // if (self.length/2 <= y_temp and y_temp < HEIGHT - self.length/2) {
    //     self.pos[1] = y_temp;
    // }

    const player = @as(*game.GameObject, @alignCast(@ptrCast(self.data.?)));
    self.pos = player.pos;

    self.direction = (mouse_loc - self.pos)/@as(@Vector(2, f32), @splat(std.math.hypot(mouse_loc[0] - self.pos[0], mouse_loc[1] - self.pos[1])));
}

pub fn updateCollision(game_data: *game.GameData, self: *game.GameObject) void {
    _ = self;
    
    // Iterate through all items
    for (0..game_data.objects.items.len) |i| {
        // Skip if this object doesn't have any collision data
        if (game_data.objects.items[i].collision_data != null) {
            // Iterate through all items
            for (0..game_data.objects.items.len) |j| {
                // If this is not the same item and the items collide, execute the collision function.
                if (i != j and game_data.objects.items[i].collision_data.?.test_func(game_data.objects.items[i], game_data.objects.items[j])) {
                    // std.debug.print("Object {} collided with Object {}\n", .{@as(TYPES, @enumFromInt(game_data.objects.items[i].type_id)), @as(TYPES, @enumFromInt(game_data.objects.items[j].type_id))});
                    game_data.objects.items[i].collision_data.?.collision_func(game_data, game_data.objects.items[i], game_data.objects.items[j]);
                }
            }
        }
    }
}
