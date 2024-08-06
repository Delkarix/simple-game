const std = @import("std");
const graphics = @import("graphics.zig");
const sdl = @import("sdl.zig");
const game = @import("game.zig");
const font = @import("font.zig");

const WIDTH = 640;
const HEIGHT = 480;

// Updates the player.
fn updatePlayer(game_obj: *game.GameData, self: *game.GameObject) void {
    // Update player velocity
    if (game_obj.keys.w_down and self.pos[1] > self.length) {
        self.pos[1] -= self.vel[1];
    }
    else if (game_obj.keys.s_down and self.pos[1] < HEIGHT - self.length) {
        self.pos[1] += self.vel[1];
    }

    if (game_obj.keys.a_down and self.pos[0] > self.length) {
        self.pos[0] -= self.vel[0];
    }
    else if (game_obj.keys.d_down and self.pos[0] < WIDTH - self.length) {
        self.pos[0] += self.vel[0];        
    }
    // self.pos += self.vel;
}

// Updates the laser
fn updateLaser(game_data: *game.GameData, self: *game.GameObject) void {
    _ = game_data;
    self.pos += self.vel;
}

// Updates the enemy
fn updateEnemy(game_data: *game.GameData, self: *game.GameObject) void {
    const player = game_data.objects.items[0];
    self.pos += @as(@Vector(2, f32), @splat(game_data.enemy_speed))*(player.pos - self.pos) / @as(@Vector(2, f32), @splat(@sqrt((player.pos[0] - self.pos[0])*(player.pos[0] - self.pos[0]) + (player.pos[1] - self.pos[1])*(player.pos[1] - self.pos[1]))));
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

// Draws the laser sprite.
fn drawLaser(image: *graphics.Image, laser: *const game.GameObject) graphics.DrawError!void {
    _ = image;
    _ = laser;
}

// Draws the enemy sprite.
fn drawEnemy(image: *graphics.Image, enemy: *const game.GameObject) graphics.DrawError!void {
    for (@intFromFloat(enemy.pos[1]-enemy.length/2)..@intFromFloat(enemy.pos[1]+enemy.length/2)) |y| {
        try image.fillRow(@intFromFloat(enemy.pos[0] - enemy.length/2), @intCast(y), @intFromFloat(enemy.length), enemy.color);
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

// Creates a laser
fn createLaser(self: *game.GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
    const player = self.objects.items[0];
    
    self.object_list.append(game.GameObject {
        .pos = .{x, y},
        .vel = 10*(self.mouse_pos - player.pos)/@sqrt((self.mouse_pos[0] - player.pos[0])*(self.mouse_pos[0] - player.pos[0]) + (self.mouse_pos[1] - player.pos[1])*(self.mouse_pos[1] - player.pos[1])),
        .length = length,
        .color = color,
        .update_func = &updateLaser,
        .draw_func = &drawLaser
    });
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
        .draw_func = &drawEnemy
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
    defer _ = gpa.deinit();

    var data: game.GameData = game.GameData.init(gpa.allocator());
    defer data.deinit();

    // Init the font system
    const font_obj = try font.Font.init("FONT");
    
// </INITIALIZATION>

    // Create the player object
    // #0
    var player = game.GameObject {
        .pos = .{ WIDTH / 2, HEIGHT / 2 },
        .color = graphics.Color{ .r = 0, .g = 255, .b = 0 },
        .value = 100,
        .update_func = &updatePlayer,
        .draw_func = &drawEnemy,
        .vel = @splat(data.enemy_speed),
        .length = 10,
        .parent = &data
    };
    try data.objects.append(&player); // Player should always (theoretically) be item #0 in the list
    
    // Create status text game object (FPS, Score, Paused, Game Over)
    // #1
    var status = game.GameObject {
        .color = .{.r = 255, .g = 255, .b = 255},
        .update_func = &updateStatus,
        .draw_func = &drawStatus,
        .data = @constCast(@ptrCast(&font_obj)),
        .parent = &data,
    };
    try data.objects.append(&status);

    while (data.running) {
// <EVENT>
        // Event loop
        var event: sdl.SDL_Event = undefined;
        while (sdl.SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                sdl.SDL_EVENT_QUIT => {
                    data.running = false;
                },
                sdl.SDL_EVENT_KEY_DOWN => {
                    switch (event.key.key) {
                        sdl.SDLK_W => {
                            data.keys.w_down = true;
                        },
                        sdl.SDLK_A => {
                            data.keys.a_down = true;
                        },
                        sdl.SDLK_S => {
                            data.keys.s_down = true;
                        },
                        sdl.SDLK_D => {
                            data.keys.d_down = true;
                        },
                        sdl.SDLK_Q => {
                            data.running = false;
                        },
                        sdl.SDLK_ESCAPE => {
                            // Quit
                            data.paused = !data.paused;
                        },
                        // sdl.SDLK_o => {
                        //     // Open
                        //     // TODO
                        // },
                        else => {},
                    }
                },
                sdl.SDL_EVENT_KEY_UP => {
                    switch (event.key.key) {
                        sdl.SDLK_W => {
                            data.keys.w_down = false;
                        },
                        sdl.SDLK_A => {
                            data.keys.a_down = false;
                        },
                        sdl.SDLK_S => {
                            data.keys.s_down = false;
                        },
                        sdl.SDLK_D => {
                            data.keys.d_down = false;
                        },
                        else => {},
                    }
                },
                sdl.SDL_EVENT_MOUSE_BUTTON_DOWN => {
                },
                else => {},
            }
        }
// </EVENT>

// <ITERATE>
        // Clear the screen
        @memset(window.image.pixels, graphics.Color {.r = 0, .g = 0, .b = 0});
        
        // Draw and update all game objects
        for (data.objects.items) |object| {
            if (object.update_func != null and !data.paused) {
                object.update_func.?(&data, object);
            }

            if (object.draw_func != null) {
                try object.draw_func.?(&window.image, object);
            }
        }

        //try text.renderString(&window.image, "TEST\nTEST2", 0, 0, .{.r = 255, .g = 255, .b = 255});
        
        window.update(); // For some reason, a single call won't always suffice. Annoying.
        //std.debug.print("{s}\n", .{sdl.SDL_GetError()});

        data.frames += 1;
// </ITERATE>
    }
}
