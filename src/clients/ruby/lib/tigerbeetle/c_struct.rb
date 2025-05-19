require_relative "c_type"

module TigerBeetle
  class CStruct < CType
    ALLOWED_C_TYPES = [:uint8, :uint16, :uint32, :uint64].freeze

    class << self
      attr_reader :fields

      def layout(opts)
        @fields = opts.map do |name, value|
          if value.is_a?(Array)
            
          end

        end

        opts.each do |name, value|
          define_method(:"#{name}") do
            if value.is_a?(CType)
            elsif value.is_a?(Symbol)
            elsif value.is_a?(Array)
            end
          end
        end
      end

      def c_type(type)
        raise ArgumentError, "Invalid c_type: #{type}" unless ALLOWED_C_TYPES.include?(type)

        @struct_c_type = type
      end

      def bytesize
        @members.inject(0) do |size, (_name, c_type)|
          if c_type.is_a?(CType)
            size + c_type.bytesize
          elsif c_type.is_a?(Symbol)
            CType::C_TYPE_BYTESIZES[c_type] + size
          elsif c_type.is_a?(Array)
            c_type.second * CType::C_TYPE_BYTESIZES[c_type.first] + size
          else
            raise ArgumentError, "Invalid member type: #{member_type}"
          end
        end
      end
    end

    def initialize(params = {})
      params.each do |key, value|
        raise ArgumentError, "Invalid field: #{key}" unless self.class.members.key?(key)
      end
      @fields = params
    end

    def c_type = :struct

    def bytesize = self.class.bytesize
  end
end
