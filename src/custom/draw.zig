const std = @import("std");
const game = @import("../game.zig");
const graphics = @import("../graphics.zig");
const font = @import("../font.zig");

// Draws the laser sprite.
pub fn drawLaser(image: *graphics.Image, laser: *const game.GameObject) graphics.DrawError!void {
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
pub fn drawSquare(image: *graphics.Image, obj: *const game.GameObject) graphics.DrawError!void {
    // std.debug.print("OBJ: P={}, L={}, T={}\n", .{obj.pos, obj.length/2, @as(TYPES, @enumFromInt(obj.type_id))});
    for (@intFromFloat(obj.pos[1]-obj.length/2)..@intFromFloat(obj.pos[1]+obj.length/2)) |y| {
        try image.fillRow(@intFromFloat(obj.pos[0] - obj.length/2), @intCast(y), @intFromFloat(obj.length), obj.color);
    }
}

pub fn drawStatus(image: *graphics.Image, text: *const game.GameObject) graphics.DrawError!void {
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

