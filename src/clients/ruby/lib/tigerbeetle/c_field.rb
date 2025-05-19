# frozen_string_literal: true

require_relative "c_type"
module TigerBeetle
  class CField < CType
    attr_reader :name, :type, :offset

    def initialize(name, type, offset: 0)
      @name = name
      @type = type
      @offset = offset
    end

    def c_type = type

    def bytesize
      if type.is_a?(CType)
        type.bytesize
      elsif type.is_a?(Symbol)
        CType::C_TYPE_BYTESIZES[type]
      end
    end
  end
end
