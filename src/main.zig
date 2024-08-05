const std = @import("std");
const graphics = @import("graphics.zig");
const sdl = @import("sdl.zig");
const game = @import("game.zig");

const WIDTH = 640;
const HEIGHT = 480;

// Updates the player.
fn updatePlayer(game_obj: game.GameData, self: *game.GameObject) void {
    // Update player velocity
    if (game_obj.keys.w_down and self.pos[1] > self.length) {
        self.pos[1] -= self.vel[1];
    }
    else if (game_obj.keys.s_down and self.pos[1] <= HEIGHT - self.length) {
        self.pos[1] += self.vel[1];
    }

    if (game_obj.keys.a_down and self.pos[0] > self.length) {
        self.pos[0] -= self.vel[0];
    }
    else if (game_obj.keys.d_down and self.pos[0] <= WIDTH - self.length) {
        self.pos[0] += self.vel[0];        
    }
    // self.pos += self.vel;
}

// Updates the laser
fn updateLaser(game_obj: game.GameData, self: *game.GameObject) void {
    _ = game_obj;
    self.pos += self.vel;
}

// Updates the enemy
fn updateEnemy(game_obj: game.GameData, self: *game.GameObject) void {
    const player = game_obj.objects.items[0];
    self.pos += @as(@Vector(2, f32), @splat(game_obj.enemy_speed))*(player.pos - self.pos) / @as(@Vector(2, f32), @splat(@sqrt((player.pos[0] - self.pos[0])*(player.pos[0] - self.pos[0]) + (player.pos[1] - self.pos[1])*(player.pos[1] - self.pos[1]))));
}

// Draws the laser sprite.
pub fn drawLaser(image: *graphics.Image, laser: *const game.GameObject) graphics.DrawError!void {
    _ = image;
    _ = laser;
}

// Draws the enemy sprite.
pub fn drawEnemy(image: *graphics.Image, enemy: *const game.GameObject) graphics.DrawError!void {
    for (@intFromFloat(enemy.pos[1]-enemy.length/2)..@intFromFloat(enemy.pos[1]+enemy.length/2)) |y| {
        try image.fillRow(@intFromFloat(enemy.pos[0] - enemy.length/2), @intCast(y), @intFromFloat(enemy.length), enemy.color);
    }
}

// Creates a laser
pub fn createLaser(self: *game.GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
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
pub fn createEnemy(self: *game.GameData, x: f32, y: f32, length: f32, color: graphics.Color) !void {
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

    // Create the player object
    var player = game.GameObject {
        .pos = .{ WIDTH / 2, HEIGHT / 2 },
        .color = graphics.Color{ .r = 0, .g = 255, .b = 0 },
        .health = 100,
        .update_func = &updatePlayer,
        .draw_func = &drawEnemy,
        .vel = .{data.enemy_speed, data.enemy_speed},
        .length = 10,
    };
    
    try data.objects.append(&player); // Player should always (theoretically) be item #0 in the list

    while (data.running) {

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
        
            // Clear the screen
            @memset(window.image.pixels, graphics.Color {.r = 0, .g = 0, .b = 0});
            
            // Draw and update all game objects
            for (data.objects.items) |object| {
                object.update_func(data, object);
                try object.draw_func(&window.image, object);
            }
        }
        
        // for (0..640) |x| {
            // for (0..480) |y| {
                // try window.image.setPixel(@intCast(x), @intCast(y), graphics.Color{ .r = @intCast(data.randomizer.next() % 256), .g = @intCast(data.randomizer.next() % 256), .b = @intCast(data.randomizer.next() % 256), .a = 255 });
            // }
        // }
        window.update(); // For some reason, a single call won't always suffice. Annoying.
        //std.debug.print("{s}\n", .{sdl.SDL_GetError()});

        std.time.sleep(10000000);
    }
}
