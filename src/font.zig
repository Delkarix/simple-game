const std = @import("std");
const graphics = @import("graphics.zig");

pub const Font = struct {
    table: [256]u64,
    
    pub fn renderString(self: @This(), image: *graphics.Image, string: []const u8, x: u16, y:u16, color: graphics.Color) graphics.DrawError!void {
        // No multicolor for now
        const og_x = x;
        var curr_x = x;
        var curr_y = y;

        for (string) |char| {
            if (char == '\n') {
                curr_x = og_x;
                curr_y += 9;
            }
            else {
                try self.renderChar(image, char, curr_x, curr_y, color);
                curr_x += 9;
            }
        }
    }

    pub fn renderChar(self: @This(), image: *graphics.Image, char: u8, x: u16, y: u16, color: graphics.Color) graphics.DrawError!void {
        for (0..8) |_y| {
            for (0..8) |_x| {
                if ((self.table[char] >> @truncate(63 - (_y * 8 + _x))) & 1 == 1) {
                    try image.setPixel(@intCast(x+_x), @intCast(y+_y), color);
                }
            }
        }
    }

    pub fn init(comptime font_file: []const u8) !@This() {
        const bytes: [*]const u8 = @embedFile(font_file);
        const table_ptr: *[256]u64 = @constCast(@alignCast(@ptrCast(bytes)));// @ptrCast(bytes);
        const table: [256]u64 = table_ptr[0..256].*;
        return @This() {.table = table};
    }

    pub fn init2(font_file: []const u8) !@This() {
        var table: [256]u64 = undefined;
        var file: std.fs.File = try std.fs.cwd().openFile(font_file, std.fs.File.OpenFlags{ .mode = .read_only });
        defer file.close();

        const reader = file.reader();
        for (&table) |*item| {
            item.* = try reader.readInt(u64, std.builtin.Endian.big);
        }

        return @This() {.table = table};
    }
};
