require 'ffi'
require_relative "ffi_struct_converter"

module TBClient
  module Types
    class UIINT128 < FFI::Struct
      include FFIStructConverter

      layout(low: :uint64, high: :uint64)

      class << self
        def to_native(value, context)
          case value
          when UINT128
            value.pointer
          when Integer
            obj = UINT128.new
            obj[:low] = value & 0xFFFFFFFFFFFFFFFF
            obj[:high] = (value >> 64) & 0xFFFFFFFFFFFFFFFF
            obj.pointer
          else
            raise TypeError, "can't convert #{value.class} to UINT128"
          end
        end
      end

      def to_i
        self[:high].to_i << 64 | self[:low].to_i
      end

      def to_s
        to_i.to_s
      end
    end
  end
end
