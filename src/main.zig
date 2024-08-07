const std = @import("std");
const graphics = @import("graphics.zig");
const sdl = @import("sdl.zig");
const game = @import("game.zig");
const font = @import("font.zig");
const collision = @import("collision.zig");

const WIDTH = 640;
const HEIGHT = 480;

pub const TYPES = enum(u8) {
    PLAYER,
    TEXT,
    TARGET,
    ENEMY_SPAWNER,
    COLLISION_DETECTOR,
    ENEMY,
    LASER
};

// IDEA: Right-click = shield

// Updates the player.
fn updatePlayer(game_obj: *game.GameData, self: *game.GameObject) void {
    // Update player velocity
    if (game_obj.keyboard['w'] and self.pos[1] > self.length) {
        self.pos[1] -= self.velocity;
    }
    else if (game_obj.keyboard['s'] and self.pos[1] < HEIGHT - self.length) {
        self.pos[1] += self.velocity;
    }

    if (game_obj.keyboard['a'] and self.pos[0] > self.length) {
        self.pos[0] -= self.velocity;
    }
    else if (game_obj.keyboard['d'] and self.pos[0] < WIDTH - self.length) {
        self.pos[0] += self.velocity;
    }
}

// Updates the laser
fn updateLaser(game_data: *game.GameData, self: *game.GameObject) void {
    _ = game_data;
    self.pos += self.direction*@as(@Vector(2, f32), @splat(self.velocity));

    const end = self.pos + @as(@Vector(2, f32), @splat(self.length))*self.direction;
    if (end[0] < 0 or end[0] >= WIDTH or end[1] < 0 or end[1] >= HEIGHT) {
        self.invalid = true;
    }
}

// Updates the enemy
fn updateEnemy(game_data: *game.GameData, self: *game.GameObject) void {
    const player = game_data.objects.items[0]; // This *should* be the player.
    self.direction = (player.pos - self.pos) / @as(@Vector(2, f32), @splat(std.math.hypot(player.pos[0] - self.pos[0], player.pos[1] - self.pos[1])));
    self.pos += self.direction*@as(@Vector(2, f32), @splat(self.velocity));
}

fn updateSpawnEnemy(game_data: *game.GameData, self: *game.GameObject) void {
    const timestamp = std.time.milliTimestamp();

    // Wait a specified amount of time before attempting to spawn another enemy
    if (self.value + @as(i64, @intFromFloat(self.length)) <= timestamp) {
        self.pos = game_data.objects.items[0].pos; // This *should* be the player

        // Attempt to find a viable position that is reasonably distant from the player
        var try_pos: @Vector(2, f32) = .{game_data.randomizer.random().float(f32)*WIDTH, game_data.randomizer.random().float(f32)*HEIGHT};
        while (std.math.hypot(self.pos[0] - try_pos[0], self.pos[1] - try_pos[1]) < 100 or try_pos[0] <= 20/2 or try_pos[0] >= WIDTH - 20/2 or try_pos[1] <= 20/2 or try_pos[1] >= HEIGHT - 20/2) {
            try_pos = .{game_data.randomizer.random().float(f32)*WIDTH, game_data.randomizer.random().float(f32)*HEIGHT};
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
            .draw_func = &drawSquare,
            .collision_data = &collision.enemy_collision,
            .parent = @constCast(game_data),
            .dyn_alloc = true,
            .type_id = @intFromEnum(TYPES.ENEMY),
        };
        game_data.objects.append(enemy_ptr) catch {return;}; // If this fails, we're fucked because we can't return an error
        self.value = timestamp;
    }
}

fn updateStatus(game_data: *game.GameData, self: *game.GameObject) void {
    const timestamp = std.time.milliTimestamp();

    if (self.value + 1000 <= timestamp) {
        game_data.curr_fps = game_data.frames;
        game_data.frames = 0;
        self.value = timestamp;
    }
}

fn updateTarget(game_data: *game.GameData, self: *game.GameObject) void {
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

fn updateCollision(game_data: *game.GameData, self: *game.GameObject) void {
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

// Draws the laser sprite.
fn drawLaser(image: *graphics.Image, laser: *const game.GameObject) graphics.DrawError!void {
    const t_diff: f32 = 1/laser.length;
    var lerp_res: @Vector(2, f32) = laser.pos;
    var t: f32 = 0;

    while (t < 1.0) : (t += t_diff) {
        // Write the pixels to the screen. If one of the lasers cannot finish drawing, that means a pixel went out-of-bounds.
        // Therefore, we should mark it as invalid so it can be automatically removed later.
        // std.debug.print("{}\n", .{lerp_res});
        try image.setPixel(@intFromFloat(lerp_res[0]), @intFromFloat(lerp_res[1]), laser.color);
        lerp_res += laser.direction;
        // lerp_res = std.math.lerp(laser.pos, laser.pos + @as(@Vector(2, f32), @splat(laser.length))*laser.vel, @as(@Vector(2, f32), @splat(t)));
    }    
}

// Draws the enemy sprite.
fn drawSquare(image: *graphics.Image, obj: *const game.GameObject) graphics.DrawError!void {
    // std.debug.print("OBJ: P={}, L={}, T={}\n", .{obj.pos, obj.length/2, @as(TYPES, @enumFromInt(obj.type_id))});
    for (@intFromFloat(obj.pos[1]-obj.length/2)..@intFromFloat(obj.pos[1]+obj.length/2)) |y| {
        try image.fillRow(@intFromFloat(obj.pos[0] - obj.length/2), @intCast(y), @intFromFloat(obj.length), obj.color);
    }
}

fn drawStatus(image: *graphics.Image, text: *const game.GameObject) graphics.DrawError!void {
    var buf: [64]u8 = undefined;
    var out: []u8 = undefined;

    // This is quite the convoluted system.
    // Essentially we store the GameData pointer inside the text GameObject and use another stored value to get the index of the font object.
    const game_data = text.parent;
    const font_obj = @as(*font.Font, @alignCast(@ptrCast(text.data.?)));
    
    if (game_data.paused) {
        out = std.fmt.bufPrint(&buf, "FPS: {d}\nSCORE: {d}\nPAUSED", .{game_data.curr_fps, game_data.score}) catch brk: {break :brk @constCast("ERROR");};
    }
    else {
        out = std.fmt.bufPrint(&buf, "FPS: {d}\nSCORE: {d}", .{game_data.curr_fps, game_data.score}) catch brk: {break :brk @constCast("ERROR");};
    }

    try font_obj.renderString(image, out, @intFromFloat(text.pos[0]), @intFromFloat(text.pos[1]), graphics.Color {.r = 255, .g = 255, .b = 255});
    // TODO: PUT TEXT FOR IF DEAD
}

pub fn main() !void {
// <INITIALIZATION>
    const SDL = try sdl.init();
    defer sdl.deinit();

    var window = try sdl.SDL.Window.init(&SDL, WIDTH, HEIGHT);
    defer window.deinit();

    // Create the allocator and arraylist
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        const leaks = gpa.deinit();

        // If there are leaks, notify
        if (leaks == .leak) {
            std.debug.print("Leaks detected.\n", .{});
        } 
    }

    var data: game.GameData = game.GameData.init(gpa.allocator());
    data.target_fps = 60;
    defer data.deinit();

    // Init the font system
    const font_obj = try font.Font.init("FONT");

    // Init the memory pool
    var mem_pool = std.heap.MemoryPool(game.GameObject).init(gpa.allocator());
    defer mem_pool.deinit();
    // std.debug.print("{}\n", .{data.randomizer.next()});
    
// </INITIALIZATION>

// <OBJECTS>
    // Create the player object
    // #0
    var player = game.GameObject {
        .pos = .{ WIDTH / 2, HEIGHT / 2 },
        .color = graphics.Color{ .r = 0, .g = 255, .b = 0 },
        .value = 100,
        .update_func = &updatePlayer,
        .draw_func = &drawSquare,
        .velocity = 2,
        .length = 10,
        .parent = &data,
        .type_id = @intFromEnum(TYPES.PLAYER),
    };
    try data.objects.append(&player); // Player should always (theoretically) be item #0 in the list
    
    // Create status text game object (FPS, Score, Paused, Game Over)
    // #1
    var status = game.GameObject {
        .pos = .{0, 0},
        .color = .{.r = 255, .g = 255, .b = 255},
        .update_func = &updateStatus,
        .draw_func = &drawStatus,
        .data = @constCast(@ptrCast(&font_obj)),
        .parent = &data,
        .value = std.time.milliTimestamp(), // last timestamp
        .type_id = @intFromEnum(TYPES.TEXT),
    };
    try data.objects.append(&status);

    // Create target thingy
    // #2
    var target = game.GameObject {
        .pos = .{ WIDTH/2, HEIGHT/2 },
        .length = 10,
        .color = .{.r = 0, .g = 0, .b = 255},
        .update_func = &updateTarget,
        .draw_func = &drawLaser, //&drawSquare,
        .parent = &data,
        .type_id = @intFromEnum(TYPES.TARGET),
        .data = &player,
    };
    try data.objects.append(&target);

    // Create enemy spawner
    // #3
    var enemy_spawner = game.GameObject {
        .pos = .{-1, -1},
        .velocity = 1, // Enemy speed
        .length = 1000, // How long in milliseconds before spawning another enemy
        .color = .{ .r = 255, .g = 0, .b = 0 },
        .update_func = &updateSpawnEnemy,
        .data = &mem_pool, // Allocator
        .parent = &data,
        .value = std.time.milliTimestamp(), // last timestamp
        .type_id = @intFromEnum(TYPES.ENEMY_SPAWNER),
    };
    // enemy_spawner.length += 1;
    try data.objects.append(&enemy_spawner);

    // Create the collision detector. NOTE: OPTIMIZE IN THE FUTURE
    // #4
    var collision_detector = game.GameObject {
        .pos = .{-1, -1},
        .length = 2, // How many subdivisions to make
        .parent = &data,
        .update_func = &updateCollision,
        .type_id = @intFromEnum(TYPES.COLLISION_DETECTOR),
    };
    try data.objects.append(&collision_detector);
    
// </OBJECTS>

    // Main loop
    while (data.running) {
        // Force FPS
        if (data.target_fps > 0) {
            const ticks = std.time.milliTimestamp();
            while (std.time.milliTimestamp() < ticks + 1000/data.target_fps) {}
        }
        
// <EVENT>
        // Event loop
        var event: sdl.SDL_Event = undefined;
        while (sdl.SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                sdl.SDL_EVENT_QUIT => {
                    data.running = false;
                },
                // WARNING: the (& 0xFF)%128 part is to nab the byte off the end to prevent crashes,
                //however higher-end keys (i.e. CAPS-LOCK will trigger the 9 key) will end up tricking the system into triggering the wrong keys.
                sdl.SDL_EVENT_KEY_DOWN => data.keyboard[(event.key.key & 0xFF) % 128] = true,
                sdl.SDL_EVENT_KEY_UP => data.keyboard[(event.key.key & 0xFF) % 128] = false,
                
                sdl.SDL_EVENT_MOUSE_BUTTON_DOWN => {
                    // TODO: ADD LASERS
                    // TODO: ADD ENEMIES
                    // TODO: Collision system?

                    // Create laser
                    const laser_ptr = try mem_pool.create();
                    laser_ptr.* = game.GameObject {
                        .pos = player.pos,
                        .direction = target.direction,//(target.pos - player.pos)/@as(@Vector(2, f32), @splat(std.math.hypot(target.pos[0] - player.pos[0], target.pos[1] - player.pos[1]))),
                        .velocity = 5,
                        .length = 10,
                        .color = .{.r = 0, .g = 0, .b = 255},
                        .update_func = &updateLaser,
                        .draw_func = &drawLaser,
                        .collision_data = &collision.laser_collision,
                        .data = &mem_pool, // Allocator
                        .parent = &data,
                        .dyn_alloc = true,
                        .type_id = @intFromEnum(TYPES.LASER),
                    };
                    try data.objects.append(laser_ptr);
                },
                else => {},
            }

            if (data.keyboard['q']) {
                data.running = false; // You could just do a direct assignment here probably
            }

            if (data.keyboard[sdl.SDLK_ESCAPE]) {
                if (!data.game_over) {
                    data.paused = !data.paused;

                    // Notify the player that the game is paused
                    updateStatus(&data, &status);
                    try drawStatus(&window.image, &status);
                    window.update();
                }
            }
        }

// </EVENT>

// <ITERATE>

        // If the game is over, display a message stating so
        if (data.game_over) {
            for (0..12) |y| {
                try window.image.fillRow(WIDTH/2 - 8*4 - 8/2, HEIGHT/2 - 12/2 + @as(u16, @intCast(y)), 8*9, graphics.Color {.r = 0, .g = 0, .b = 0});
            }
            try font_obj.renderString(&window.image, "GAME OVER", WIDTH/2 - 8*4 - 8/2, HEIGHT/2 - 8/2, graphics.Color {.r = 255, .g = 0, .b = 0});
            data.paused = true;
            window.update();
        }

        if (!data.paused) {
            // Clear the screen
            @memset(window.image.pixels, graphics.Color {.r = 0, .g = 0, .b = 0});
        
            // Draw and update all game objects
            for (0..data.objects.items.len) |i| {
                if (!data.paused and data.objects.items[i].update_func != null) {
                    data.objects.items[i].update_func.?(&data, data.objects.items[i]);
                }

                if (data.objects.items[i].draw_func != null and !data.objects.items[i].invalid) {
                    try data.objects.items[i].draw_func.?(&window.image, data.objects.items[i]);
                }
                //i += 1;
            }
            // for (data.objects.items) |object| {
                // if (!data.paused and object.update_func != null) {
                    // object.update_func.?(&data, object);
                // }

                // if (object.draw_func != null and !object.invalid) {
                    // try object.draw_func.?(&window.image, object);
                // }
            // }

            // Remove invalid objects
            // i = 0;
            var i: usize = 0;
            while (i < data.objects.items.len) : (i += 1) {
            // for (0..data.objects.items.len) |i| {
                if (data.objects.items[i].invalid) {
                    const obj = data.objects.swapRemove(i);
                    
                    if (obj.dyn_alloc) {
                        mem_pool.destroy(obj);
                    }
                    continue;
                }
            }
       
            window.update(); // For some reason, a single call won't always suffice. Annoying.
            data.frames += 1;
        }
// </ITERATE>
    }
}
