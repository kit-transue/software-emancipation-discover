/*
 * S Q L _ A S T _ E N U M . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * NOTE: This file is GENERATED from the ast_enum.sh script, and
 * therefore should not be modified directly as any changes will
 * be overwritten during the next build.  Any changes should be
 * made to the script.
 *
 * Include file with enum list of Abstract Syntax Tree node kinds.  These
 * can be either symbol types (e.g. variable, procedure, etc) or
 * parser non-terminals (assigment_statement, plus, function_body, etc).
 */

#ifndef SQL_AST_ENUM_H
#define SQL_AST_ENUM_H

#include "sql_all.h"
#include "sql_sym_enum.h"

enum ast_node_kind_enum {
   ank_none = 0,
   ank_alias,
   ank_assign_expr,
   ank_assign_stmt,
   ank_ast_block,
   ank_ast_declspec,
   ank_ast_replace,
   ank_ast_root,
   ank_bit_and_expr,
   ank_bit_ior_expr,
   ank_bit_not_expr,
   ank_bit_xor_expr,
   ank_call_expr,
   ank_column,
   ank_column_alias,
   ank_constant,
   ank_context,
   ank_cross_join_expr,
   ank_cursor,
   ank_cursor_decl,
   ank_cursor_def,
   ank_dblink,
   ank_div_expr,
   ank_elsif_clause,
   ank_eq_expr,
   ank_error_mark,
   ank_exception,
   ank_exception_decl,
   ank_exception_handler,
   ank_exit_stmt,
   ank_expon_expr,
   ank_expr_stmt,
   ank_externref,
   ank_field,
   ank_field_decl,
   ank_file,
   ank_function,
   ank_function_decl,
   ank_function_def,
   ank_full_join_expr,
   ank_ge_expr,
   ank_goto_stmt,
   ank_gt_expr,
   ank_if_stmt,
   ank_in_expr,
   ank_inner_join_expr,
   ank_label,
   ank_label_decl,
   ank_le_expr,
   ank_left_join_expr,
   ank_list,
   ank_list_decl,
   ank_ljoin_expr,
   ank_loop_stmt,
   ank_lt_expr,
   ank_minus_expr,
   ank_mult_expr,
   ank_ne_expr,
   ank_negate_expr,
   ank_package,
   ank_package_body,
   ank_parm_decl,
   ank_plsql_table,
   ank_plus_expr,
   ank_procedure,
   ank_record,
   ank_record_type,
   ank_ref_type,
   ank_return_stmt,
   ank_right_join_expr,
   ank_rjoin_expr,
   ank_savepoint,
   ank_schema,
   ank_sequence,
   ank_string,
   ank_symbol,
   ank_table,
   ank_table_type,
   ank_trigger,
   ank_trunc_mod_expr,
   ank_truth_and_expr,
   ank_truth_not_expr,
   ank_truth_or_expr,
   ank_type,
   ank_type_decl,
   ank_var_decl,
   ank_variable,
   ank_where,
   ank_while_stmt
};

typedef enum ast_node_kind_enum ast_node_kind;

EXTERN_C const char * ast_node_kind_lookup (ast_node_kind ank);

#endif /* SQL_AST_ENUM_H */
