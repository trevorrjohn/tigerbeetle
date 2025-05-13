require_relative "tigerbeetle"
require "securerandom"

cluster_id = "0"
address = "3000"
callback = Proc.new do |msg|
  puts msg
end

client = TigerBeetle::Client.new(cluster_id, address, callback)
