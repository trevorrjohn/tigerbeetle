const std = @import("std");
// const vsr = @import("vsr");
// const exports = vsr.tb_client.exports;
// const type_mappings = vsr.tb_client.type_mappings;
// const assert = std.debug.assert;

const rb = @cImport({
    @cInclude("ruby.h");
});
// const tb_client = @cImport(@cInclude("./ext/tb_client.h"));

// const constants = vsr.constants;
// const IO = vsr.io.IO;
//
// const Tracer = vsr.trace.TracerType(vsr.time.Time);
// const Storage = vsr.storage.StorageType(IO, Tracer);
// const StateMachine = vsr.state_machine.StateMachineType(Storage, constants.state_machine_config);
// const tb = vsr.tigerbeetle;
//
// const mappings_vsr = .{
//     .{ exports.tb_operation, "Operation" },
//     .{ exports.tb_packet_status, "PacketStatus" },
//     .{ exports.tb_packet_t, "Packet", "tb_packet" },
//     .{ exports.tb_client_t, "Client", "tb_client" },
//     .{ exports.tb_init_status, "InitStatus" },
//     .{ exports.tb_client_status, "ClientStatus" },
//     .{ exports.tb_log_level, "LogLevel" },
//     .{ exports.tb_register_log_callback_status, "RegisterLogCallbackStatus" },
// };
//
// /// State machine specific mappings: in future, these should be pulled automatically from the state
// /// machine.
// const mappings_state_machine = .{
//     .{ tb.AccountFlags, "AccountFlags" },
//     .{ tb.TransferFlags, "TransferFlags" },
//     .{ tb.AccountFilterFlags, "AccountFilterFlags" },
//     .{ tb.QueryFilterFlags, "QueryFilterFlags" },
//     .{ tb.Account, "Account", "tb_account" },
//     .{ tb.Transfer, "Transfer", "tb_transfer" },
//     .{ tb.CreateAccountResult, "CreateAccountResult", "tb_create_accounts_result" },
//     .{ tb.CreateTransferResult, "CreateTransferResult", "tb_create_transfer_result" },
//     .{ tb.AccountFilter, "AccountFilter", "tb_account_filter" },
//     .{ tb.AccountBalance, "AccountBalance", "tb_account_balance" },
//     .{ tb.QueryFilter, "QueryFilter", "tb_query_filter" },
// };

// const mappings_all = mappings_vsr ++ mappings_state_machine;

export const ruby_abi_version = rb.RUBY_ABI_VERSION;

var gpa = std.heap.GeneralPurposeAllocator(.{}){};

pub fn Init_TigerBeetle() void {
    const mtb = rb.rb_define_module("TigerBeetle");
    const mClient = rb.rb_define_module_under(mtb, "Client");
    rb.define_const(mClient, "I_DID_IT", 1);
}
