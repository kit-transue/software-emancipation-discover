/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
/*
 * P L S Q L _ A S T _ E N U M . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * NOTE: This file is GENERATED from the ast_enum.sh script, and
 * therefore should not be modified directly as any changes will
 * be overwritten during the next build.  Any changes should be
 * made to the script.
 *
 * This file contains the lookup routine to translate
 * Abstract Syntax Tree (AST) node kinds into their text
 * representation for output to the IFF (Internal Format File).
 */

#include "sql_all.h"		// EXTERN_C
#include "sql_ast_enum.h"	// ast node kinds
#include "sql_sym_enum.h"	// symbol kinds

//
// ast_node_kind_lookup - return the text string for each ast node kind
//
// This is used to output the syntax tree built during parsing.
// Each tree node becomes an entry in the IFF output with the
// text below as the label. (see iff_ast() in sql_iff.C)
//
EXTERN_C const char * ast_node_kind_lookup (ast_node_kind ank)
{
   const char *s;

   switch (ank) {
      case ank_none:		s = ""; break;
      case ank_alias:		s = "alias"; break;
      case ank_assign_expr:		s = "assign_expr"; break;
      case ank_assign_stmt:		s = "assign_stmt"; break;
      case ank_ast_block:		s = "ast_block"; break;
      case ank_ast_declspec:		s = "ast_declspec"; break;
      case ank_ast_replace:		s = "ast_replace"; break;
      case ank_ast_root:		s = "ast_root"; break;
      case ank_bit_and_expr:		s = "bit_and_expr"; break;
      case ank_bit_ior_expr:		s = "bit_ior_expr"; break;
      case ank_bit_not_expr:		s = "bit_not_expr"; break;
      case ank_bit_xor_expr:		s = "bit_xor"; break;
      case ank_call_expr:		s = "call_expr"; break;
      case ank_column:		s = "column"; break;
      case ank_column_alias:		s = "column_alias"; break;
      case ank_constant:		s = "constant"; break;
      case ank_context:		s = "context"; break;
      case ank_cross_join_expr:		s = "cross_join_expr"; break;
      case ank_cursor:		s = "cursor"; break;
      case ank_cursor_decl:		s = "cursor_decl"; break;
      case ank_cursor_def:		s = "cursor_def"; break;
      case ank_dblink:		s = "dblink"; break;
      case ank_div_expr:		s = "div_expr"; break;
      case ank_elsif_clause:		s = "elsif_clause"; break;
      case ank_eq_expr:		s = "eq_expr"; break;
      case ank_error_mark:		s = "error_mark"; break;
      case ank_exception:		s = "exception"; break;
      case ank_exception_decl:		s = "exception_decl"; break;
      case ank_exception_handler:		s = "exception_handler"; break;
      case ank_exit_stmt:		s = "exit_stmt"; break;
      case ank_expon_expr:		s = "expon_expr"; break;
      case ank_expr_stmt:		s = "expr_stmt"; break;
      case ank_externref:		s = "externref"; break;
      case ank_field:		s = "field"; break;
      case ank_field_decl:		s = "field_decl"; break;
      case ank_file:		s = "file"; break;
      case ank_full_join_expr:		s = "full_join_expr"; break;
      case ank_function:		s = "function"; break;
      case ank_function_decl:		s = "function_decl"; break;
      case ank_function_def:		s = "function_def"; break;
      case ank_ge_expr:		s = "ge_expr"; break;
      case ank_goto_stmt:		s = "goto_stmt"; break;
      case ank_gt_expr:		s = "gt_expr"; break;
      case ank_if_stmt:		s = "if_stmt"; break;
      case ank_in_expr:		s = "in_expr"; break;
      case ank_inner_join_expr:		s = "inner_join_expr"; break;
      case ank_label:		s = "label"; break;
      case ank_label_decl:		s = "label_decl"; break;
      case ank_le_expr:		s = "le_expr"; break;
      case ank_left_join_expr:		s = "left_join_expr"; break;
      case ank_list:		s = "list"; break;
      case ank_list_decl:		s = "list_decl"; break;
      case ank_ljoin_expr:		s = "ljoin_expr"; break;
      case ank_loop_stmt:		s = "loop_stmt"; break;
      case ank_lt_expr:		s = "lt_expr"; break;
      case ank_minus_expr:		s = "minus_expr"; break;
      case ank_mult_expr:		s = "mult_expr"; break;
      case ank_ne_expr:		s = "ne_expr"; break;
      case ank_negate_expr:		s = "negate_expr"; break;
      case ank_package:		s = "package"; break;
      case ank_package_body:		s = "package_body"; break;
      case ank_parm_decl:		s = "parm_decl"; break;
      case ank_plsql_table:		s = "plsql_table"; break;
      case ank_plus_expr:		s = "plus_expr"; break;
      case ank_procedure:		s = "procedure"; break;
      case ank_record:		s = "record"; break;
      case ank_record_type:		s = "record_type"; break;
      case ank_ref_type:		s = "ref_type"; break;
      case ank_return_stmt:		s = "return_stmt"; break;
      case ank_right_join_expr:		s = "right_join_expr"; break;
      case ank_rjoin_expr:		s = "rjoin_expr"; break;
      case ank_savepoint:		s = "savepoint"; break;
      case ank_schema:		s = "schema"; break;
      case ank_sequence:		s = "sequence"; break;
      case ank_string:		s = "string"; break;
      case ank_symbol:		s = "symbol"; break;
      case ank_table:		s = "table"; break;
      case ank_table_type:		s = "table_type"; break;
      case ank_trigger:		s = "trigger"; break;
      case ank_trunc_mod_expr:		s = "trunc_mod_expr"; break;
      case ank_truth_and_expr:		s = "truth_and_expr"; break;
      case ank_truth_not_expr:		s = "truth_not_expr"; break;
      case ank_truth_or_expr:		s = "truth_or_expr"; break;
      case ank_type:		s = "type"; break;
      case ank_type_decl:		s = "type_decl"; break;
      case ank_var_decl:		s = "var_decl"; break;
      case ank_variable:		s = "variable"; break;
      case ank_where:		s = "where"; break;
      case ank_while_stmt:		s = "while_stmt"; break;
      default:			s = ""; break;
   }

   return (s);
}
