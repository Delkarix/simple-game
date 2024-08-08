const std = @import("std");
const graphics = @import("engine/graphics.zig");
const sdl = @import("engine/sdl.zig");
const game = @import("engine/game.zig");
const font = @import("engine/font.zig");
const collision = @import("collision.zig");
const update = @import("update.zig");
const draw = @import("draw.zig");

pub const WIDTH = 1280;//640;
pub const HEIGHT = 960;//480;

pub const TYPES = enum(u8) {
    PLAYER,
    TEXT,
    TARGET,
    OBJECT_MODIFIER,
    OBJECT_SPAWNER,
    COLLISION_DETECTOR,
    ENEMY,
    LASER
};

// IDEA: Right-click = shield

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
        .update_func = &update.updatePlayer,
        .draw_func = &draw.drawSquare,
        .velocity = 2,
        .length = 10,
        .parent = &data,
        .has_collision = true,
        .type_id = @intFromEnum(TYPES.PLAYER),
    };
    try data.objects.append(&player); // Player should always (theoretically) be item #0 in the list
    
    // Create status text game object (FPS, Score, Paused, Game Over)
    // #1
    var status = game.GameObject {
        .pos = .{0, 0},
        .color = .{.r = 255, .g = 255, .b = 255},
        .update_func = &update.updateStatus,
        .draw_func = &draw.drawStatus,
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
        .update_func = &update.updateTarget,
        .draw_func = &draw.drawLaser, //&drawSquare,
        .parent = &data,
        .type_id = @intFromEnum(TYPES.TARGET),
        .data = &player,
    };
    try data.objects.append(&target);

    // Create enemy spawner
    // #3
    var enemy_spawner = game.GameObject {
        .velocity = 1, // Enemy speed
        .length = 1000, // How long in milliseconds before spawning another enemy
        .color = .{ .r = 255, .g = 0, .b = 0 },
        .update_func = &update.updateSpawnEnemy,
        .data = &mem_pool, // Allocator
        .parent = &data,
        .value = std.time.milliTimestamp(), // last timestamp
        .type_id = @intFromEnum(TYPES.OBJECT_SPAWNER),
    };
    // enemy_spawner.length += 1;
    try data.objects.append(&enemy_spawner);

    // Create the collision detector. NOTE: OPTIMIZE IN THE FUTURE
    // #4
    var collision_detector = game.GameObject {
        .length = 2, // How many subdivisions to make
        .parent = &data,
        .update_func = &update.updateCollision,
        .type_id = @intFromEnum(TYPES.COLLISION_DETECTOR),
    };
    try data.objects.append(&collision_detector);

    // Create the enemy spawner speed increaser
    // #5
    var enemy_speed_increaser = game.GameObject {
        .velocity = 0.1, // The rate at which enemies should speed up
        .length = 10, // The rate at which the spawn rate should be increased (in milliseconds)
        .value = std.time.milliTimestamp(),
        .update_func = &update.updateSpawnRate,
        .data = &enemy_spawner,
        .parent = &data,
        .type_id = @intFromEnum(TYPES.OBJECT_MODIFIER),
    };
    try data.objects.append(&enemy_speed_increaser);
    
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
                    // Create laser
                    const laser_ptr = try mem_pool.create();
                    laser_ptr.* = game.GameObject {
                        .pos = player.pos,
                        .direction = target.direction,//(target.pos - player.pos)/@as(@Vector(2, f32), @splat(std.math.hypot(target.pos[0] - player.pos[0], target.pos[1] - player.pos[1]))),
                        .velocity = 5,
                        .length = 10,
                        .color = .{.r = 0, .g = 0, .b = 255},
                        .update_func = &update.updateLaser,
                        .draw_func = &draw.drawLaser,
                        .collision_data = &collision.laser_collision,
                        .has_collision = true,
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
                // std.debug.print("exited\n", .{});
                data.running = false; // You could just do a direct assignment here probably
            }

            if (data.keyboard[sdl.SDLK_ESCAPE]) {
                if (!data.game_over) {
                    data.paused = !data.paused;
                    // std.debug.print("paused\n", .{});

                    // Notify the player that the game is paused
                    update.updateStatus(&data, &status);
                    try draw.drawStatus(&window.image, &status);
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
