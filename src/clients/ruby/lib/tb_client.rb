# frozen_string_literal: true

require_relative "tb_client/version"
require_relative "tb_client/bindings"

require "tb_client/tb_client"

module TBClient
  def connect(addresses, cluster_id)
    TBClient::Client.init(addresses, cluster_id)
  end

  def create_accounts(client, accounts)
    TBClient::Client.submit(client, Bindings::Operation::CREATE_ACCOUNTS, accounts) do |create_accounts_result|
      yield(create_accounts_result) if block_given?
    end
  end

  def lookup_accounts(client, account_ids)
    TBClient::Client.submit(client, Bindings::Operation::LOOKUP_ACCOUNTS, account_ids) do |accounts|
      yield(accounts) if block_given?
    end
  end
end
