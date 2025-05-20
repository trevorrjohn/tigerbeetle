# frozen_string_literal: true

require "fiddle"
require "fiddle/import"
require "fiddle/struct"

require_relative "shared_lib"

module TigerBeetle
  module TBFiddle
    extend Fiddle::Importer

    dlload SharedLib.path

    UInt128T = struct(['uint64_t lo', 'uint64_t hi'])

    Packet = struct([
      'void* user_data',
      'void* data',
      'uint32_t data_size',
      'uint16_t user_tag',
      'uint8_t operation',
      'uint8_t status',
      'uint8_t opaque[32]',
    ])

    Account2 = struct([
      { id: UInt128T },
      { debits_pending: UInt128T },
      { debits_posted: UInt128T },
      { credits_pending: UInt128T },
      { credits_posted: UInt128T },
      { user_data_128: UInt128T },
      'uint64_t user_data_64',
      'uint32_t user_data_32',
      'uint32_t reserved',
      'uint32_t ledger',
      'uint16_t code',
      'uint16_t flags',
      'uint64_t timestamp',
  ])
    # class Transfer < CStruct
    #   layout(
    #     id: :uint128,
    #     debit_account_id: :uint128,
    #     credit_account_id: :uint128,
    #     amount: :uint128,
    #     pending_id: :uint128,
    #     user_data_128: :uint128,
    #     user_data_64: :uint64,
    #     user_data_32: :uint32,
    #     timeout: :uint32,
    #     ledger: :uint32,
    #     code: :uint16,
    #     flags: TransferFlags,
    #     timestamp: :uint64,
    #   )
    # end
    #
    # class CreateAccountsResult < CStruct
    #   layout(
    #     index: :uint32,
    #     result: CreateAccountResult,
    #   )
    # end
    #
    # class CreateTransfersResult < CStruct
    #   layout(
    #     index: :uint32,
    #     result: CreateTransferResult,
    #   )
    # end
    #
    # class AccountFilter < CStruct
    #   layout(
    #     account_id: :uint128,
    #     user_data_128: :uint128,
    #     user_data_64: :uint64,
    #     user_data_32: :uint32,
    #     code: :uint16,
    #     reserved: [:uint8, 58],
    #     timestamp_min: :uint64,
    #     timestamp_max: :uint64,
    #     limit: :uint32,
    #     flags: AccountFilterFlags,
    #   )
    # end
    #
    # class AccountBalance < CStruct
    #   layout(
    #     debits_pending: :uint128,
    #     debits_posted: :uint128,
    #     credits_pending: :uint128,
    #     credits_posted: :uint128,
    #     timestamp: :uint64,
    #     reserved: [:uint8, 56],
    #   )
    # end
    #
    # class QueryFilter < CStruct
    #   layout(
    #     user_data_128: :uint128,
    #     user_data_64: :uint64,
    #     user_data_32: :uint32,
    #     ledger: :uint32,
    #     code: :uint16,
    #     reserved: [:uint8, 6],
    #     timestamp_min: :uint64,
    #     timestamp_max: :uint64,
    #     limit: :uint32,
    #     flags: QueryFilterFlags,
    #   )
    # end
  end
end
