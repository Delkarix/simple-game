const game = @import("game.zig");

pub const Color = packed struct {
    b: u8,
    g: u8,
    r: u8,
    a: u8
};

pub const Image = struct {
    width: u16,
    height: u16,
    pixels: []Color,

    pub fn getPixel(self: *@This(), x: u16, y: u16) DrawError!Color {
        if (x < 0 or x >= self.width or y < 0 or y >= self.height) {
            return DrawError.OutOfBounds;
        }

        return self.pixels[y*self.width + x];
    }

    pub fn setPixel(self: *@This(), x: u16, y: u16, new_color: Color) DrawError!void {
        if (x < 0 or x >= self.width or y < 0 or y >= self.height) {
            return DrawError.OutOfBounds;
        }

        self.pixels[@as(usize, @truncate(@as(usize, y)))*@as(usize, @truncate(@as(usize, self.width))) + @as(usize, @truncate(@as(usize, x)))] = new_color;
    }
};

pub const DrawError = error{
    OutOfBounds
};

//pub fn drawLaser(laser: game.GameObject) DrawError!void {

//}

//pub fn drawEnemy(enemy: game.GameObject) DrawError!void {

//}