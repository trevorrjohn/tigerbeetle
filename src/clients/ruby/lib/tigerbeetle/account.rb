# frozen_string_literal: true

require_relative "bindings"

module TigerBeetle
  class Account < Bindings::Account

    def initialize(params = {})
      @params = params
    end

    def to_c_struct
    end
  end
end
