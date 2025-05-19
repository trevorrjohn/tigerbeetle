require "fiddle"
require "fiddle/struct"

module TigerBeetle
  class Uint128  = struct(
    "uint64_t lo",
    "uint64_t hi",
  )
  end
end
