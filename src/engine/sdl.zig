const lib = @cImport(
    @cInclude("SDL3/SDL.h")
);

pub usingnamespace lib;

const graphics = @import("graphics.zig");

/// Represents a wrapper around the SDL library
pub const SDL = struct {
    /// Represents a wrapper around an SDL Window.
    pub const Window = struct {
        /// Represents the drawable region in the window
        image: graphics.Image,

        /// Represents the underlying SDL_Window pointer
        ptr: *lib.SDL_Window,

        /// Represents the SDL_Window's surface
        surface_ptr: *lib.SDL_Surface,

        /// Initializes a window.
        pub fn init(self: *const SDL, width: u16, height: u16) !Window {
            // We're forcing the user to first create an SDL struct and then pass it.
            _ = self;

            // Create the window
            const ptr: ?*lib.SDL_Window = lib.SDL_CreateWindow("Window", width, height, lib.SDL_WINDOW_OPENGL);
            if (ptr == null) {
                return error.SDLWindowFailedInit;
            }

            // Attempt to get the surface
            const surface_ptr: ?*lib.SDL_Surface = lib.SDL_GetWindowSurface(ptr.?);
            if (surface_ptr == null) {
                return error.SDLWindowFailedSurface;
            }
            _ = lib.SDL_LockSurface(surface_ptr.?); // Probably should do error testing on this one but ehh
            _ = lib.SDL_ShowWindow(ptr); // Show the window. This will cause windows to show themselves on default.

            // Create the image
            const img = graphics.Image {.width = width, .height = height, .pixels = @as([*]graphics.Color, @alignCast(@ptrCast(surface_ptr.?.pixels.?)))[0..@as(usize, @truncate(@as(usize, width)))*@as(usize, @truncate(@as(usize, height)))]};

            // Return the final window
            return Window {.image = img, .ptr = ptr.?, .surface_ptr = surface_ptr.?};
        }

        /// Destroys the window.
        pub fn deinit(self: *@This()) void {
            _ = lib.SDL_UnlockSurface(self.surface_ptr);
            lib.SDL_DestroyWindow(self.ptr);
            self.* = undefined;
        }

        pub fn update(self: *@This()) void {
            _ = lib.SDL_UpdateWindowSurface(self.ptr);
            //@import("std").debug.print("{}\n", .{val});
        }
    };
};

/// Initializes the SDL library.
pub fn init() !SDL {
    if (lib.SDL_Init(lib.SDL_INIT_VIDEO) != 0) {
        return error.SDLFailedInit;
    }

    return SDL {};
}

pub fn deinit() void {
    lib.SDL_Quit();
}