const std = @import("std");
const game = @import("game.zig");

/// A type representing an RGBA value.
pub const Color = packed struct { b: u8, g: u8, r: u8, a: u8 = 255};

/// A struct representing an image.
pub const Image = struct {
    width: u16,
    height: u16,
    pixels: []Color,

    /// Returns the pixel at the requested coordinates.
    pub fn getPixel(self: *@This(), x: u16, y: u16) DrawError!Color {
        if (x < 0 or x >= self.width or y < 0 or y >= self.height) {
            return DrawError.OutOfBounds;
        }

        return self.pixels[y * self.width + x];
    }

    /// Uses Alpha-Blending to modify the pixel at the given position
    pub fn setPixelBlend(self: *@This(), x: u16, y: u16, new_color: Color) DrawError!void {
        if (x < 0 or x >= self.width or y < 0 or y >= self.height) {
            return DrawError.OutOfBounds;
        }

        const old_color = self.pixels[@as(usize, @truncate(@as(usize, y))) * @as(usize, @truncate(@as(usize, self.width))) + @as(usize, @truncate(@as(usize, x)))];
        const t = @as(f32, @floatFromInt(new_color.a))/@as(f32, @floatFromInt(old_color.a));
        const color = Color {
            .r = old_color.r + t*(new_color.r - old_color.r),
            .g = old_color.g + t*(new_color.g - old_color.g),
            .b = old_color.b + t*(new_color.b - old_color.b),
            .a = 255
        };

        self.pixels[@as(usize, @truncate(@as(usize, y))) * @as(usize, @truncate(@as(usize, self.width))) + @as(usize, @truncate(@as(usize, x)))] = color;
    }

    /// Modifies the pixel at the given position
    pub fn setPixel(self: *@This(), x: u16, y: u16, new_color: Color) DrawError!void {
        if (x < 0 or x >= self.width or y < 0 or y >= self.height) {
            return DrawError.OutOfBounds;
        }

        self.pixels[@as(usize, @truncate(@as(usize, y))) * @as(usize, @truncate(@as(usize, self.width))) + @as(usize, @truncate(@as(usize, x)))] = new_color;
    }

    /// Fills a row of pixels with the given color.
    pub fn fillRow(self: *@This(), x: u16, y: u16, length: u16, new_color: Color) DrawError!void {
        if ((x < 0 or x >= self.width or y < 0 or y >= self.height) and (x + length >= self.width)) {
            return DrawError.OutOfBounds;
        }

        const coord = @as(usize, @truncate(@as(usize, y))) * @as(usize, @truncate(@as(usize, self.width))) + @as(usize, @truncate(@as(usize, x)));

        @memset(self.pixels[coord..coord+length], new_color);
    }
};

pub const DrawError = error{
    OutOfBounds
};
