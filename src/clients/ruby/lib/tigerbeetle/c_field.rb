# frozen_string_literal: true

module TigerBeetle
  class CField
    C_BYTESIZES = {
      uint8: 1,
      uint16: 2,
      uint32: 4,
      uint64: 8,
      tb_uint128_t: 16,
      pointer: 8,
    }.freeze

    attr_reader :name, :type, :value

    def initialize(name:, type:, value: nil)
      @name = name
      @type = type
    end

    def bytesize
      case type
      when CEnum
        type.bytesize
      when Symbol
        C_BYTESIZES[type] || raise(ArgumentError, "Invalid c_type: #{type}")
      else
        raise ArgumentError, "Invalid c_type: #{type}"
      end
    end
  end
end
