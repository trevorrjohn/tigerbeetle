# frozen_string_literal: true

require_relative "tigerbeetle/version"

require "tigerbeetle/tigerbeetle"

module TigerBeetle
  class Error < StandardError; end

  # def create_account(client, accounts, &block)
  #   raise ArgumentError, "Block is required" unless block_given?
  #
  #   client.submit(Bindings::Operation::CREATE_ACCOUNT, accounts, &block)
  # end
end
