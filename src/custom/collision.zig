// const std = @import("std");
const game = @import("../game.zig");
const TYPES = @import("../main.zig").TYPES;

// pub fn BinaryTree(comptime DataType: type) type {
//     return struct {
//         left: ?*BinaryTree = null,
//         right: ?*BinaryTree = null,
//         data: DataType
//     };
// }

// pub const CollisionData = struct {
//     horizontal: bool = false,
//     objs: [2]?*game.GameObject
// };

// pub const CollisionDetector = struct {
    
// };

pub fn collisionDelete(game_data: *game.GameData, self: *game.GameObject, collider: *game.GameObject) void {
    _ = self;

    if (collider.type_id == @intFromEnum(TYPES.PLAYER)) {
        game_data.game_over = true;
    }
    else {
        collider.invalid = true;
    }
    
    if (collider.type_id == @intFromEnum(TYPES.ENEMY)) {
        game_data.score += 1;
    }
}

pub fn testEnemy(self: *game.GameObject, collider: *game.GameObject) bool {
    if (self.type_id != collider.type_id) {
        // If the enemy collides with a laser, do a Point-Square test
        if (collider.type_id == @intFromEnum(TYPES.LASER)) {
            return testPointSquare(collider.pos, self.pos, self.length);
        }
        // Otherwise, do a square
        else {
            return testSquareSquare(collider.pos, collider.length, self.pos, self.length);
        }
    }

    return false;
}

pub fn testLaser(self: *game.GameObject, collider: *game.GameObject) bool {
    if (collider.type_id == @intFromEnum(TYPES.ENEMY) and self.type_id != collider.type_id) {
        return testPointSquare(self.pos, collider.pos, collider.length);
    }

    return false;
}

fn testPointSquare(point: @Vector(2, f32), square: @Vector(2, f32), square_length: f32) bool {
    return  @reduce(.And, point >= square - @as(@Vector(2, f32), @splat(square_length/2)))
        and @reduce(.And, point <= square + @as(@Vector(2, f32), @splat(square_length/2)));
}

fn testSquareSquare(square1: @Vector(2, f32), square1_length: f32, square2: @Vector(2, f32), square2_length: f32) bool {
    return testPointSquare(square1 - @as(@Vector(2, f32), @splat(square1_length/2)), square2, square2_length)
        or testPointSquare(square1 + @as(@Vector(2, f32), @splat(square1_length/2)), square2, square2_length);
}

pub const laser_collision = game.CollisionData {
    .test_func = &testLaser,
    .collision_func = &collisionDelete
};

pub const enemy_collision = game.CollisionData {
    .test_func = &testEnemy,
    .collision_func = &collisionDelete
};
