const std = @import("std");
const uefi = std.os.uefi;

const W = std.unicode.utf8ToUtf16LeStringLiteral;

// Simple helper to - utilizing pre-allocated buffers - provide a slice of formatted utf16-string ready to pass to e.g. outputString()
fn formatToU16(buf8: []u8, buf16: []u16, comptime format: []const u8, comptime vars: anytype) [:0]const u16 {
    const formatted = std.fmt.bufPrint(buf8, format, vars) catch "";
    _ = std.unicode.utf8ToUtf16Le(buf16, formatted) catch 0;
    buf16[formatted.len] = 0;
    return buf16[0..formatted.len:0];
}

// Entry point. Zig handles assignment of system_table and uefi handle to uefi.system_table and uefi.handle respectively.
pub fn main() usize {
    const con_out = uefi.system_table.con_out.?;
    _ = con_out.reset(false);

    var scrap: [128]u8 = undefined;
    var scrap16: [128]u16 = undefined;

    _ = con_out.outputString(W("Hello NDC!\r\n"));
    _ = con_out.outputString(formatToU16(&scrap, &scrap16, "Look! We can do formatted strings now: {}", .{123}));

    const boot_services = uefi.system_table.boot_services.?;
    var event: u64 = undefined;
    var key: uefi.protocol.SimpleTextInput.Key.Input = undefined;
    
    while(true) {
        _ = boot_services.waitForEvent(1, @as([*]uefi.Event, @ptrCast(&uefi.system_table.con_in.?.wait_for_key)), &event);
        _ = uefi.system_table.con_in.?.readKeyStroke(&key);
        if(key.unicode_char == 13) break;
    }

    return 0;
}
