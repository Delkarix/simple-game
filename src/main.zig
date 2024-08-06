const std = @import("std");
const graphics = @import("graphics.zig");
const sdl = @import("sdl.zig");
const game = @import("game.zig");
const font = @import("font.zig");

const WIDTH = 640;
const HEIGHT = 480;
var enemy_speed = 1;

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
    self.velocity = enemy_speed;
    self.pos += self.direction*@as(@Vector(2, 3), @splat(self.vel));
}

fn updateStatus(game_data: *game.GameData, self: *game.GameObject) void {
    _ = self;
    const timestamp = std.time.milliTimestamp();

    if (game_data.last_timestamp + 1000 <= timestamp) {
        game_data.curr_fps = game_data.frames;
        game_data.frames = 0;
        game_data.last_timestamp = timestamp;
    }
}

fn updateTarget(game_data: *game.GameData, self: *game.GameObject) void {
    // Getting the mouse info from here instead of saving it somewhere for every mouse movement event is technically more efficient and better on resources
    _ = game_data;

    var x_temp: f32 = undefined;
    var y_temp: f32 = undefined;
    _ = sdl.SDL_GetMouseState(&x_temp, &y_temp);

    if (self.length/2 < x_temp and x_temp < WIDTH - self.length/2) {
        self.pos[0] = x_temp;
    }

    if (self.length/2 <= y_temp and y_temp < HEIGHT - self.length/2) {
        self.pos[1] = y_temp;
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
        try image.setPixel(@intFromFloat(lerp_res[0]), @intFromFloat(lerp_res[1]), laser.color);
        lerp_res += laser.direction;
        // lerp_res = std.math.lerp(laser.pos, laser.pos + @as(@Vector(2, f32), @splat(laser.length))*laser.vel, @as(@Vector(2, f32), @splat(t)));
    }    
}

// Draws the enemy sprite.
fn drawSquare(image: *graphics.Image, obj: *const game.GameObject) graphics.DrawError!void {
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
    
    // if (game_data.paused) {
        // out = std.fmt.bufPrint(&buf, "FPS: {d}\nSCORE: {d}\nPAUSED", .{game_data.curr_fps, game_data.score}) catch brk: {break :brk @constCast("ERROR");};
    // }
    // else {
    out = std.fmt.bufPrint(&buf, "FPS: {d}\nSCORE: {d}", .{game_data.curr_fps, game_data.score}) catch brk: {break :brk @constCast("ERROR");};
    // }

    try font_obj.renderString(image, out, @intFromFloat(text.pos[0]), @intFromFloat(text.pos[1]), graphics.Color {.r = 255, .g = 255, .b = 255});
    // TODO: PUT TEXT FOR IF DEAD
}

// Creates an enemy
fn createEnemy(self: *game.GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
    //var try_pos: @Vector(2, f32) = .{self.randomizer.float(f32)*self.window.image.width, self.randomizer.float(f32)*self.window.image.height};

    // Ensure that any spawned enemies are at least 100 units away from the player
    //while (@sqrt((self.player_pos[0] - try_pos[0])*(self.player_pos[0] - try_pos[0]) + (self.player_pos[1] - try_pos[1])*(self.player_pos[1] - try_pos[1])) < 100) {
    //    try_pos = .{self.randomizer.float(f32)*self.window.image.width, self.randomizer.float(f32)*self.window.image.height};
    //}

    self.object_list.append(game.GameObject {
        .pos = .{x, y},
        .length = length,
        .color = color,
        .update_func = &updateEnemy,
        .draw_func = &drawSquare
    });
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
        .velocity = 1,
        .length = 10,
        .parent = &data
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
    };
    try data.objects.append(&status);

    // Create target thingy
    // #2
    var target = game.GameObject {
        .pos = .{ WIDTH/2, HEIGHT/2 },
        .length = 5,
        .color = .{.r = 0, .g = 0, .b = 255},
        .update_func = &updateTarget,
        .draw_func = &drawSquare,
        .parent = &data
    };
    try data.objects.append(&target);
    
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
                        .direction = (target.pos - player.pos)/@as(@Vector(2, f32), @splat(std.math.hypot(target.pos[0] - player.pos[0], target.pos[1] - player.pos[1]))),
                        .velocity = 2,
                        .length = 10,
                        .color = .{.r = 0, .g = 0, .b = 255},
                        .update_func = &updateLaser,
                        .draw_func = &drawLaser,
                        .data = &mem_pool,
                        .parent = &data,
                        .dyn_alloc = true,
                    };
                    try data.objects.append(laser_ptr);
                },
                else => {},
            }

            if (data.keyboard['q']) {
                data.running = false; // You could just do a direct assignment here probably
            }

            if (data.keyboard[sdl.SDLK_ESCAPE]) {
                data.paused = !data.paused;
            }
        }

// </EVENT>

// <ITERATE>

        if (!data.paused) {
            // Clear the screen
            @memset(window.image.pixels, graphics.Color {.r = 0, .g = 0, .b = 0});
        
            // Draw and update all game objects
            for (data.objects.items) |object| {
                if (!data.paused and object.update_func != null) {
                    object.update_func.?(&data, object);
                }

                if (object.draw_func != null and !object.invalid) {
                    try object.draw_func.?(&window.image, object);
                }
            }

            // Remove invalid objects
            var i: usize = 0;
            while (i < data.objects.items.len) {
                if (data.objects.items[i].invalid) {
                    const obj = data.objects.swapRemove(i);
                    
                    if (obj.dyn_alloc) {
                        mem_pool.destroy(obj);
                    }
                    continue;
                }

                i += 1;
            }
       
            window.update(); // For some reason, a single call won't always suffice. Annoying.
            data.frames += 1;
        }
// </ITERATE>
    }
}
