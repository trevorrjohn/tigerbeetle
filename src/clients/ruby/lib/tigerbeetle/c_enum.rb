# frozen_string_literal: true

require_relative "c_type"

module TigerBeetle
  class CEnum < CType
  ALLOWED_C_TYPES = [:uint8, :uint16, :uint32, :uint64].freeze

    class << self
      attr_reader :enum_options, :enum_c_type

      def options(opts)
        @enum_options = opts.freeze

        opts.each do |name, value|
          const_set(name, value)

          define_method(:"#{name.downcase}?") do
            @value == value
          end
        end
      end

      def c_type(type)
        raise ArgumentError, "Invalid c_type: #{type}" unless ALLOWED_C_TYPES.include?(type)

        @enum_c_type = type
      end

      def name_from_value(value)
        @enum_options.key(value)
      end

      def value_from_name(name)
        @enum_options[name]
      end

      def bytesize
        CType::C_TYPE_BYTESIZES[@enum_c_type]
      end
    end

    attr_reader :name, :value

    # @param value [Symbol, Integer] The enum value or name.
    def initialize(value)
      if value.is_a?(Symbol)
        @name = value
        @value = self.class.value_from_name(@name)

        raise ArgumentError, "Invalid enum name: #{value}" unless @value
      elsif value.is_a?(Integer)
        @value = value
        @name = self.class.name_from_value(@value)

        raise ArgumentError, "Invalid enum value: #{value}" unless @name
      else
        raise ArgumentError, "Enum value must be #{self.class.options}"
      end
    end

    def to_sym = @name

    def to_i = @value

    def to_s = to_sym.to_s

    def bytesize
      C_TYPE_BYTESIZES[self.class.enum_c_type]
    end

    def c_type = self.class.enum_c_type

    def ==(other)
      case other
      when Integer
        @value == other
      when Symbol
        @name == other
      when self.class
        @value == other.to_i && @name == other.to_sym
      else
        false
      end
    end
  end
end
