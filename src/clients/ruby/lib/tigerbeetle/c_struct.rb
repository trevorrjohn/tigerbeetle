require_relative "c_type"
require_relative "c_field"

module TigerBeetle
  class CStruct < CType
    class << self
      attr_reader :field_types

      def layout(field_types)
        @field_types = field_types
      end

      def c_type(type)
        raise ArgumentError, "Invalid c_type: #{type}" unless ALLOWED_C_TYPES.include?(type)

        @struct_c_type = type
      end
    end

    def initialize(params = {})
      raise ArgumentError, "invalid key" if (self.class.field_types.keys - params.keys).any?

      @members = self.class.field_types.map do |key, type|
        value = params[key]

        CField.new(
          name: key,
          type: type,
          value: value
        )
      end
    end

    def c_type = :struct

    def bytesize = self.class.bytesize
  end
end
