# frozen_string_literal: true

require_relative "tigerbeetle/version"
require_relative "tigerbeetle/result"

module TigerBeetle
  class Error < StandardError; end
end

require "tigerbeetle/tigerbeetle"
