# frozen_string_literal: true

require_relative "tigerbeetle/version"
require_relative "tigerbeetle/result"
require_relative "tigerbeetle/account"
require_relative "tigerbeetle/client"
require_relative "tigerbeetle/bindings"

require "tigerbeetle/tigerbeetle"

module TigerBeetle
  class Error < StandardError; end

  def create_account(client, accounts, &block)
    raise ArgumentError, "Block is required" unless block_given?

    client.submit(Bindings::Operation::CREATE_ACCOUNT, accounts, &block)
  end
end
