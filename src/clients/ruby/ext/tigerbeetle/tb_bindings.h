#ifndef TB_BINDINGS_H
#define TB_BINDINGS_H

#include <ruby.h>
#include "tb_client.h"

void tb_define_enums_and_bitmasks(VALUE module);
void tb_define_ruby_accessors(VALUE module);

extern const rb_data_type_t rb_tb_packet_type;
extern const rb_data_type_t rb_tb_account_type;
extern const rb_data_type_t rb_tb_client_type;
extern const rb_data_type_t rb_tb_transfer_type;
extern const rb_data_type_t rb_tb_account_filter_type;
extern const rb_data_type_t rb_tb_account_balance_type;
extern const rb_data_type_t rb_tb_query_filter_type;
#endif
