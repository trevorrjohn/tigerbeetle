require "mkmf"

dir_config("tigerbeetle")

have_library("tb_client")

create_makefile("tigerbeetle")
