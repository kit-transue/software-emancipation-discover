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
/* Creation of the AST (abstract syntax tree) in its in-memory form.
 *
 * This is separated from writing of SYM lines etc. in SET_symbol
 * because there is not always a one-to-one correspondence
 * between declarations and symbols.
 */

#ifndef SET_ast_h
#define SET_ast_h

#include "SET_names.h"

/* The following enumeration specifies all the different names
 * of (parser) AST nodes.  (The preprocessor AST node names are
 * handled differently and are not part of this list.)  The names
 * are in alphabetical order.  If you add names to this list, be
 * sure that both the arrays (GNU_AST_COMPATIBILITY and the
 * converse) are updated accordingly.
 */

typedef enum a_node_name {
    NN_unknown_node,
    NN_add,
    NN_add_assign,
    NN_address,
    NN_address_of_ellipsis,
    NN_and,
    NN_and_assign,
    NN_asm,
    NN_asm_entry,
    NN_asm_function,
    NN_asm_pseudofunction,
    NN_assign,
    NN_assume,
    NN_base_name,
    NN_base_spec,
    NN_bit_field_ref,
    NN_block,
    NN_block_assign,
    NN_break,
    NN_call,
    NN_case_default,
    NN_case_label,
    NN_case_values,
    NN_cast,
    NN_cast_to_base,
    NN_cast_to_bool,
    NN_cast_to_derived,
    NN_catch,
    NN_class_def,
    NN_comma,
    NN_complement,
    NN_condition,
    NN_const_cast,
    NN_constant,
    NN_constant_enum,
    NN_constant_str,
    NN_constructor_call,
    NN_continue,
    NN_ctor_init,
    NN_decl_id_cnst,
    NN_decl_id_cls,
    NN_decl_id_enum,
    NN_decl_id_fld,
    NN_decl_id_fcn,
    NN_decl_id_lbl,
    NN_decl_id_ns,
    NN_decl_id_strct,
    NN_decl_id_tpl,
    NN_decl_id_typ,
    NN_decl_id_typd,
    NN_decl_id_un,
    NN_decl_id_var,
    NN_decl_specs,
    NN_declaration_no_dclrtr,
    NN_declaration_with_dclrtr,
    NN_declarator_data_mbr,
    NN_declarator_fcn,
    NN_declarator_typedef,
    NN_declarator_var,
    NN_default,
    NN_defined,
    NN_delete_sclr,
    NN_delete_vec,
    NN_divide,
    NN_divide_assign,
    NN_do_while,
    NN_dynamic_cast,
    NN_dynamic_init,
    NN_empty,
    NN_enum_def,
    NN_enumerator_decl,
    NN_eq,
    NN_error,
    NN_except,
    NN_expression,
    NN_expression_statement,
    NN_extract_bit_field,
    NN_field,
    NN_field_ref,
    NN_finally,
    NN_flt_add,
    NN_flt_add_assign,
    NN_flt_assign,
    NN_flt_divide,
    NN_flt_divide_assign,
    NN_flt_eq,
    NN_flt_ge,
    NN_flt_gt,
    NN_flt_le,
    NN_flt_lt,
    NN_flt_multiply_assign,
    NN_flt_multiply,
    NN_flt_ne,
    NN_flt_negate,
    NN_flt_post_decr,
    NN_flt_post_incr,
    NN_flt_pre_decr,
    NN_flt_pre_incr,
    NN_flt_subtract,
    NN_flt_subtract_assign,
    NN_for,
    NN_function_def,
    NN_ge,
    NN_generic_call,
    NN_goto,
    NN_gt,
    NN_if,
    NN_implicit_address,
    NN_indirect,
    NN_init,
    NN_int_add,
    NN_int_add_assign,
    NN_int_assign,
    NN_int_divide,
    NN_int_divide_assign,
    NN_int_eq,
    NN_int_ge,
    NN_int_gt,
    NN_int_le,
    NN_int_lt,
    NN_int_multiply,
    NN_int_multiply_assign,
    NN_int_ne,
    NN_int_negate,
    NN_int_post_decr,
    NN_int_post_incr,
    NN_int_pre_decr,
    NN_int_pre_incr,
    NN_int_subtract,
    NN_int_subtract_assign,
    NN_label,
    NN_le,
    NN_logical_and,
    NN_logical_or,
    NN_lshift,
    NN_lshift_assign,
    NN_lt,
    NN_lvalue,
    NN_lvalue_cast,
    NN_lvalue_dot_static,
    NN_lvalue_from_struct_rvalue,
    NN_microsoft_try,
    NN_multiply,
    NN_multiply_assign,
    NN_name,
    NN_name_clause,
    NN_namespace_def,
    NN_ne,
    NN_negate,
    NN_new_sclr,
    NN_new_vec,
    NN_not,
    NN_null_operand,
    NN_object_lifetime,
    NN_operation,
    NN_or,
    NN_or_assign,
    NN_parm_decl,
    NN_pm_arrow_field,
    NN_pm_assign,
    NN_pm_call,
    NN_pm_cast_to_base,
    NN_pm_cast_to_derived,
    NN_pm_dot_field,
    NN_pm_eq,
    NN_pm_field_ref,
    NN_pm_ne,
    NN_points_to_static,
    NN_post_decr,
    NN_post_incr,
    NN_pre_decr,
    NN_pre_incr,
    NN_ptr_add,
    NN_ptr_add_assign,
    NN_ptr_add_subscript,
    NN_ptr_assign,
    NN_ptr_diff,
    NN_ptr_eq,
    NN_ptr_ge,
    NN_ptr_gt,
    NN_ptr_le,
    NN_ptr_lt,
    NN_ptr_ne,
    NN_ptr_post_decr,
    NN_ptr_post_incr,
    NN_ptr_pre_decr,
    NN_ptr_pre_incr,
    NN_ptr_subtract,
    NN_ptr_subtract_assign,
    NN_question_colon,
    NN_reinterpret_cast,
    NN_remainder,
    NN_remainder_assign,
    NN_return,
    NN_routine_address,
    NN_rshift,
    NN_rshift_assign,
    NN_runtime_sizeof,
    NN_rvalue,
    NN_rvalue_dot_static,
    NN_set_vla_size,
    NN_statement,
    NN_static_cast,
    NN_struct_assign,
    NN_struct_def,
    NN_subscript,
    NN_subtract,
    NN_subtract_assign,
    NN_switch,
    NN_switch_clause,
    NN_temp_init,
    NN_template_body,
    NN_template_class,
    NN_template_fcn,
    NN_template_header,
    NN_template_static_data_mbr,
    NN_template_struct,
    NN_template_union,
    NN_throw,
    NN_try_block,
    NN_type,
    NN_typeid,
    NN_unary_plus,
    NN_union_def,
    NN_used_entity,
    NN_using,
    NN_using_decl,
    NN_vacuous_destructor_call,
    NN_value_bit_field_ref,
    NN_value_field_ref,
    NN_value_vacuous_destructor_call,
    NN_variable,
    NN_variable_address,
    NN_virtual_call,
    NN_virtual_function_ptr,
    NN_vla_decl,
    NN_while,
    NN_xor,
    NN_xor_assign,
/* MUST BE LAST: */
    NN_NUMBER_OF_NAMES
} a_node_name;

/* The following struct defines the information allowing a node in the
 * Abstract Syntax Tree (AST) to be printed to the IF.  Nodes are
 * created bottom-up while walking the IL tree; then, when the IL parent
 * is encountered during the walk, the ast_node_info objects corresponding
 * to the IL children are linked to each other via the next_sibling link
 * and to the ast_node_info object corresponding to the parent via the
 * first_child link. These nodes are linked to the corresponding IL nodes
 * using the an_il_entry_prefix::SET_info field.
 */
/* Note:
 * Fields start_pos and end_pos store a_source_position structure in the AST node.
 * These values get filled during AST creation parts of process_entry. Later
 * in walk_ast the actual line number and offsets are dumped to the output
 * buffer.
 */

typedef struct ast_node_info {
    ast_node_info_ptr first_child;
    ast_node_info_ptr next_sibling;
    const char* text;
    short node_name;
    a_tagged_pointer *entry;
    a_source_position start_pos;
    a_source_position end_pos;
    a_bit_field use_len:1;
    a_bit_field is_leaf:1;
    a_bit_field suppress_node:1;
    a_bit_field suppress_subtree:1;
} ast_node_info;

extern ast_node_info_ptr ast_info_of(void* ilp);

/* Process an il entry to create its portion of the AST. 
 * This is invoked in sequence of a walk of the EDG IL, with some
 * modifications. */
extern void process_ast_of_entry(char *entry_ptr, an_il_entry_kind entry_kind);

/* Called when entering a scope during the IL walk. */
extern void ast_notify_entering_scope(a_scope_ptr ptr);

/* Returns the node kind for the IF AST associated with the
 * given EDG IL expression. */
extern a_node_name expr_node_name(an_expr_node_ptr expr);

/* Sanity check some of the structures used to create the AST;
 * once per parse is appropriate. */
extern void validate_ast_structures(void);

/* Check the structures used to track the building of the AST,
 * to ensure that they properly reflect completion. */
extern void check_ast_consistent(void);

extern void dump_ast_memory_meters(void);

/* The IF AST node names, indexed by enum a_node_name. */
extern const char* node_name_str_gnu[];
extern const char* node_name_str[];

/* Set if a structural problem occurs while generating the AST tree.
 * A corrupted tree should not be traversed. */
extern a_boolean AST_is_corrupted;

/* If the IL walk is handling a function, this is the AST of
 * its declarator. */
extern ast_node_info_ptr fcn_declarator;

#endif /* SET_ast_h */
