# frozen_string_literal: true

module TigerBeetle
  class CType
    def c_type
      raise NotImplementedError, "Subclasses must implement a to_c_type method"
    end

    def bytes
      raise NotImplementedError, "Subclasses must implement a bytes method"
    end

    def bytesize
      raise NotImplementedError, "Subclasses must implement a bytesize method"
    end
  end
end
