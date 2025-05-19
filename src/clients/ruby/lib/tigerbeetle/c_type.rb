# frozen_string_literal: true

module TigerBeetle
  class CType
    C_TYPE_BYTESIZES = {
      uint8: 1,
      uint16: 2,
      uint32: 4,
      uint64: 8,
      tb_uint128_t: 16,
      pointer: 8,
    }.freeze

    def c_type
      raise NotImplementedError, "Subclasses must implement a to_c_type method"
    end

    def bytesize
      raise NotImplementedError, "Subclasses must implement a bytesize method"
    end
  end
end
