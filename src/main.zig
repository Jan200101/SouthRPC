const std = @import("std");

pub const PLUGIN_NAME = "Zig Plugin";
pub const LOG_NAME = "ZIGPLUGIN";
pub const DEPENDENCY_NAME = "ZigPlugin";

pub const std_options: std.Options = .{
    .logFn = @import("sys.zig").log,
    .log_level = .info,
};

comptime {
    _ = @import("interface.zig");
}
