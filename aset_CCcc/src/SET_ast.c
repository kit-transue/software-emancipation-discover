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
#include "basic_hdrs.h"
#include "fe_common.h"
#include "SET_il.h"
#include "SET_ast.h"
#include "SET_symid.h"
#include "SET_additions.h"
#include "SET_complaints.h"
#include "SET_symbol.h"
#include "SET_dump_if.h"
#include "SET_process_entry.h"

/* -------------------- AST MANAGEMENT --------------------
 *
 * structs:	ast_node_info
 *
 * enums:	a_node_name
 *
 * variables:	node_name_str_gnu node_name_str expr_operand
 *		ast_node_head ast_node_tail ast_node_block next_free_ast_node_info
 *
 * functions:	new_ast_node_info make_null_ast_operand cycle_in_ast
 * 		make_string_ast_operand relink mirror_hierarchy indent_to_depth
 *		walk_ast dump_ast
 *
 * macros:	AST_NODE_INFO_BLOCK_COUNT ast_info_of parallelize_links
 *
 * Each ast_node_info object represents one node in the IF AST.  They
 * are allocated in a block and parceled out one at a time (by
 * new_ast_node_info) to reduce allocation overhead.  Because of the
 * bottom-up nature of the IL tree walk, nodes are created unattached
 * and moved to child status only when their parents are
 * encountered; the relink function is used to make that transition,
 * and the parallelize_links macro does the same for a linked list of
 * IL and AST nodes.  At some points, the IL has a NULL pointer to
 * indicate an omitted optional operand; that is represented in the IF
 * AST by a special node created by make_null_ast_operand.  The
 * ast_info_of macro gets the ast_node_info object corresponding to a
 * given IL node.  The AST is written to the IF file by dump_ast, which
 * traverses the list of top-level nodes and calls walk_ast for each;
 * walk_ast recursively dumps a node and its subtree.
 *
 * The parser AST can be dumped in one of two forms: one corresponding
 * closely to the structure and naming used in the IL, the other
 * compatible with the AST produced by the gcc-based parser, the
 * choice being based on the global variable GNU_compatible_AST.  To
 * facilitate this dual representation, node names are given in the
 * node by a value of the enumeration a_node_name, which is then
 * translated into the appropriate name at the time it is dumped using
 * the node_name_str arrays.  A major difference is the treatment of
 * operators that require an lvalue as an operand; such operators have
 * address operands in the native AST but variables in the gcc AST.
 * The expr_operand.first_is_address field is used to trigger the
 * special treatment of these nodes.
 *
 * Two fields in the ast_node_info structure are examined at dump time
 * to determine whether to output the node to the IF or not.
 * suppress_node causes the children of the node to be promoted in its
 * place; suppress_subtree causes the node and all its descendants to be
 * omitted.  The former is used to handle certain differences between the
 * two flavors of AST, as well as to suppress the constant nodes
 * corresponding to folded constant expressions (so that only the
 * expressions themsleves appear in the AST).  The latter is used when
 * insufficient information is known when a node is encountered during
 * the IL walk to decide whether AST should be dumped but the parent has
 * the ability to discriminate (e.g., bodies of template instantiations).
 *
 * IL nodes, once processed, are linked to their corresponding AST nodes
 * via the SET_info field of the il_entry_prefix.  Since this field is
 * used to point to the sym_summary_info object for nodes that belong
 * to "uninteresting" files (see below), the ast_info_of macro checks to
 * ensure that the node for which it is invoked is "interesting" and
 * complains if it is not.
 *
 * -------------------- STATEMENT STACK MANAGEMENT --------------------
 *
 * structs:	a_stmt_stack_entry
 *
 * variables:	fcn_parm_link fcn_declarator top_of_stmt_stack
 *		free_stmt_stack_entry
 *
 * functions:	push_stmt pop_stmt chain_at_curr_stmt_stack_level
 *		find_stmt_entity pop_specific_stmt pop_expected_stmt
 *
 * The top-level organization of the IF (file-scope declarations,
 * statements within function blocks) is provided by the
 * source sequence entries.  However, these provide no indication
 * of the nesting structure of the program, only the order in which
 * the various elements occur.  The statement stack is used to keep
 * track of the nesting.  A new level is pushed whenever a construct
 * is encountered that estabishes a new nesting level; otherwise,
 * the entities represented by the source sequence entries are just
 * chained as siblings at the current level (the stack entries
 * maintain a pointer to the place to link the next entity at that
 * level).
 *
 * The end of a level is indicated in one of two ways.  Some constructs
 * have an explicit end-of-construct indication in the intermediate
 * language; for the rest, the entity that ends the level is found
 * from the IL and recorded in the stack entry, and when that item is
 * encountered, the stack is popped.
 *
 * The fcn_parm_link and fcn_declarator variables are established
 * whenever a function scope is pushed to allow the parameters and
 * function body to be linked appropriately.
 *
 * -------------------- DECLARATION TREE GENERATION --------------------
 *
 * variables:	start_of_declarator_chain end_of_declarator_chain
 *		pending_declaration_head
 *
 * functions:	make_decl_specs_ast decl_pos_of scan_declarator_chain
 *		link_declarator_list make_declaration_tree
 *		create_ast_info_for_type_def create_ast_info_for_declarator
 *		check_for_fcn_basetype is_in_class_def
 *
 * Declarations are generally processed in two pieces: the declaration
 * header, including the declaration node itself and the declaration
 * specifiers, and the declarator.  The declarator is usually created
 * (using create_ast_info_for_declarator) by the code that processes
 * the entity's IL node in process_entry.  Since the IL proper has no
 * indication that various entities are part of the same declaration,
 * the creation of the declaration header is deferred until the source
 * sequence entries are processed.  At that point, the entities from a
 * single declaration are contiguous; entities with adjacent source
 * sequence entries that have the same source location for their
 * declaration specifiers are from a single declaration.  The function
 * scan_declarator_chain is responsible for this determination, setting
 * the start_of_declarator_chain and end_of_declarator_chain variables
 * to mark the source sequence entries corresponding to a given
 * declaration.  The function is called when the first declarator's
 * source sequence entry is seen; when the last declarator's source
 * sequence entry is seen, the declaration header is created (it needs
 * to indicate the source location of the last declarator that is part
 * of the declaration).
 *
 * The make_declaration_tree function is called when the last declarator
 * in a declaration is seen.  It functions differently depending on
 * whether the declaration speciifer contains a type definition or not.
 * If there is no type definition, it calls make_decl_specs_ast to
 * create the node for the declaration specifiers (whose only function
 * is to indicate the source location of same) and creates a declaration
 * node as its parent.  If there is a type definition in the specifiers,
 * the declaration header, including a specifier subtree for the
 * definition, is created (by create_ast_info_for_type_def) when
 * processing the type's source sequence entry, and the variable
 * pending_declaration_head is set so that make_declaration_tree will
 * simply reuse it rather than creating one from scratch.  In either case,
 * link_declarator_list is called to link the declarators of the
 * declaration together beneath the header.
 */

/* The following array gives the spellings of all the parser AST node
 * names in GNU-compatible form.
 */

const char* node_name_str_gnu[] = {
    "unknown_node",			/* NN_unknown_node */
    "plus_expr",			/* NN_add */
    "plus_assign_expr",			/* NN_add_assign */
    "addr_expr",			/* NN_address */
    "address_of_ellipsis",		/* NN_address_of_ellipsis */
    "bit_and_expr",			/* NN_and */
    "bit_and_assign_expr",		/* NN_and_assign */
    "asm_stmt",				/* NN_asm */
    "string",				/* NN_asm_entry */
    "ast_block",			/* NN_asm_function */
    "asm_pseudofunction",		/* NN_asm_pseudofunction */
    "assign_expr",			/* NN_assign */
    "assume",				/* NN_assume */
    "class",				/* NN_base_name */
    "ast_superclass",			/* NN_base_spec */
    "bit_field_ref",			/* NN_bit_field_ref */
    "ast_block",			/* NN_block */
    "block_assign",			/* NN_block_assign */
    "exit_stmt",			/* NN_break */
    "call_expr",			/* NN_call */
    "case_default",			/* NN_case_default */
    "case_label",			/* NN_case_label */
    "case_values",			/* NN_case_values */
    "convert_expr",			/* NN_cast */
    "convert_expr",			/* NN_cast_to_base */
    "convert_expr",			/* NN_cast_to_bool */
    "convert_expr",			/* NN_cast_to_derived */
    "catch",				/* NN_catch */
    "type_decl",			/* NN_class_def */
    "compound_expr",		  	/* NN_comma */
    "bit_not_expr",			/* NN_complement */
    "condition",			/* NN_condition */
    "convert_expr",			/* NN_const_cast */
    "constant",				/* NN_constant */
    "evalue",				/* NN_constant_enum */
    "string",				/* NN_constant_str */
    "call_expr",			/* NN_constructor_call */
    "exit_stmt",			/* NN_continue */
    "ctor_init",			/* NN_ctor_init */
    "constant",				/* NN_decl_id_cnst */
    "class",				/* NN_decl_id_cls */
    "enum",				/* NN_decl_id_enum */
    "field",				/* NN_decl_id_fld */
    "function",				/* NN_decl_id_fcn */
    "label",				/* NN_decl_id_lbl */
    "namespace",			/* NN_decl_id_ns */
    "struct",				/* NN_decl_id_strct */
    "template",				/* NN_decl_id_tpl */
    "type",				/* NN_decl_id_typ */
    "typedef",				/* NN_decl_id_typd */
    "union",				/* NN_decl_id_un */
    "variable",				/* NN_decl_id_var */
    "ast_declspec",			/* NN_decl_specs */
    "type_decl",			/* NN_declaration_no_dclrtr */
    "list_decl",			/* NN_declaration_with_dclrtr */
    "field_decl",			/* NN_declarator_data_mbr */
    "function_decl",			/* NN_declarator_fcn */
    "type_decl",			/* NN_declarator_typedef */
    "var_decl",				/* NN_declarator_var */
    "default",				/* NN_default */
    "defined",				/* NN_defined */
    "dl_expr",				/* NN_delete_sclr */
    "vec_dl_expr",			/* NN_delete_vec */
    "trunc_div_expr",			/* NN_divide */
    "trunc_div_assign_expr",		/* NN_divide_assign */
    "do_while_stmt",			/* NN_do_while */
    "convert_expr",			/* NN_dynamic_cast */
    "dynamic_init",			/* NN_dynamic_init */
    "statement",			/* NN_empty */
    "type_decl",			/* NN_enum_def */
    "const_decl",			/* NN_enumerator_decl */
    "eq_expr",				/* NN_eq */
    "error",				/* NN_error */
    "catch",				/* NN_except */
    "expression",			/* NN_expression */
    "expr_stmt",			/* NN_expression_statement */
    "component_ref",			/* NN_extract_bit_field */
    "field",				/* NN_field */
    "component_ref",			/* NN_field_ref */
    "catch",				/* NN_finally */
    "plus_expr",			/* NN_flt_add */
    "plus_assign_expr",			/* NN_flt_add_assign */
    "assign_expr",			/* NN_flt_assign */
    "rdiv_expr",			/* NN_flt_divide */
    "rdiv_assign_expr",			/* NN_flt_divide_assign */
    "eq_expr",				/* NN_flt_eq */
    "ge_expr",				/* NN_flt_ge */
    "gt_expr",				/* NN_flt_gt */
    "le_expr",				/* NN_flt_le */
    "lt_expr",				/* NN_flt_lt */
    "mult_assign_expr",			/* NN_flt_multiply_assign */
    "mult_expr",			/* NN_flt_multiply */
    "ne_expr",				/* NN_flt_ne */
    "negate_expr",			/* NN_flt_negate */
    "postdecrement_expr",		/* NN_flt_post_decr */
    "postincrement_expr",		/* NN_flt_post_incr */
    "predecrement_expr",		/* NN_flt_pre_decr */
    "preincrement_expr",		/* NN_flt_pre_incr */
    "minus_expr",			/* NN_flt_subtract */
    "minus_assign_expr",		/* NN_flt_subtract_assign */
    "for_stmt",				/* NN_for */
    "function_def",			/* NN_function_def */
    "ge_expr",				/* NN_ge */
    "call_expr",			/* NN_generic_call */
    "goto_stmt",			/* NN_goto */
    "gt_expr",				/* NN_gt */
    "if_stmt",				/* NN_if */
    "variable",				/* NN_implicit_address */
    "indirect_ref",			/* NN_indirect */
    "init",				/* NN_init */
    "plus_expr",			/* NN_int_add */
    "plus_assign_expr",			/* NN_int_add_assign */
    "assign_expr",			/* NN_int_assign */
    "trunc_div_expr",			/* NN_int_divide */
    "trunc_div_assign_expr",		/* NN_int_divide_assign */
    "eq_expr",				/* NN_int_eq */
    "ge_expr",				/* NN_int_ge */
    "gt_expr",				/* NN_int_gt */
    "le_expr",				/* NN_int_le */
    "lt_expr",				/* NN_int_lt */
    "mult_expr",			/* NN_int_multiply */
    "mult_assign_expr",			/* NN_int_multiply_assign */
    "ne_expr",				/* NN_int_ne */
    "negate_expr",			/* NN_int_negate */
    "postdecrement_expr",		/* NN_int_post_decr */
    "postincrement_expr",		/* NN_int_post_incr */
    "predecrement_expr",		/* NN_int_pre_decr */
    "preincrement_expr",		/* NN_int_pre_incr */
    "minus_expr",			/* NN_int_subtract */
    "minus_assign_expr",		/* NN_int_subtract_assign */
    "label_decl",			/* NN_label */
    "le_expr",				/* NN_le */
    "truth_andif_expr",			/* NN_logical_and */
    "truth_orif_expr",			/* NN_logical_or */
    "alshift_expr",			/* NN_lshift */
    "alshift_assign_expr",		/* NN_lshift_assign */
    "lt_expr",				/* NN_lt */
    "addr_expr",			/* NN_lvalue */
    "convert_expr",			/* NN_lvalue_cast */
    "component_ref",			/* NN_lvalue_dot_static */
    "lvalue_from_struct_rvalue",	/* NN_lvalue_from_struct_rvalue */
    "microsoft_try",			/* NN_microsoft_try */
    "mult_expr",			/* NN_multiply */
    "mult_assign_expr",			/* NN_multiply_assign */
    "class",				/* NN_name */
    "name_clause",			/* NN_name_clause */
    "namespace_def",			/* NN_namespace_def */
    "ne_expr",				/* NN_ne */
    "negate_expr",			/* NN_negate */
    "nw_expr",				/* NN_new_sclr */
    "vec_nw_expr",			/* NN_new_vec */
    "truth_not_expr",			/* NN_not */
    "no_operand",			/* NN_null_operand */
    "object_lifetime",			/* NN_object_lifetime */
    "operation",			/* NN_operation */
    "bit_ior_expr",			/* NN_or */
    "bit_ior_assign_expr",		/* NN_or_assign */
    "parm_decl",			/* NN_parm_decl */
    "component_ref",			/* NN_pm_arrow_field */
    "assign_expr",			/* NN_pm_assign */
    "call_expr",			/* NN_pm_call */
    "convert_expr",			/* NN_pm_cast_to_base */
    "convert_expr",			/* NN_pm_cast_to_derived */
    "component_ref",			/* NN_pm_dot_field */
    "eq_expr",				/* NN_pm_eq */
    "pm_field_ref",			/* NN_pm_field_ref */
    "ne_expr",				/* NN_pm_ne */
    "component_ref",			/* NN_points_to_static */
    "postdecrement_expr",		/* NN_post_decr */
    "postincrement_expr",		/* NN_post_incr */
    "predecrement_expr",		/* NN_pre_decr */
    "preincrement_expr",		/* NN_pre_incr */
    "plus_expr",			/* NN_ptr_add */
    "plus_assign_expr",			/* NN_ptr_add_assign */
    "array_ref",			/* NN_ptr_add_subscript */
    "assign_expr",			/* NN_ptr_assign */
    "minus_expr",			/* NN_ptr_diff */
    "eq_expr",				/* NN_ptr_eq */
    "ge_expr",				/* NN_ptr_ge */
    "gt_expr",				/* NN_ptr_gt */
    "le_expr",				/* NN_ptr_le */
    "lt_expr",				/* NN_ptr_lt */
    "ne_expr",				/* NN_ptr_ne */
    "postdecrement_expr",		/* NN_ptr_post_decr */
    "postincrement_expr",		/* NN_ptr_post_incr */
    "predecrement_expr",		/* NN_ptr_pre_decr */
    "preincrement_expr",		/* NN_ptr_pre_incr */
    "minus_expr",			/* NN_ptr_subtract */
    "minus_assign_expr",		/* NN_ptr_subtract_assign */
    "cond_expr",			/* NN_question_colon */
    "convert_expr",			/* NN_reinterpret_cast */
    "trunc_mod_expr",			/* NN_remainder */
    "trunc_mod_assign_expr",		/* NN_remainder_assign */
    "return_stmt",			/* NN_return */
    "function",				/* NN_routine_address */
    "arshift_expr",			/* NN_rshift */
    "arshift_assign_expr",		/* NN_rshift_assign */
    "runtime_sizeof",			/* NN_runtime_sizeof */
    "rvalue",				/* NN_rvalue */
    "component_ref",			/* NN_rvalue_dot_static */
    "set_vla_size",			/* NN_set_vla_size */
    "statement",			/* NN_statement */
    "convert_expr",			/* NN_static_cast */
    "assign_expr",			/* NN_struct_assign */
    "type_decl",			/* NN_struct_def */
    "array_ref",			/* NN_subscript */
    "minus_expr",			/* NN_subtract */
    "minus_assign_expr",		/* NN_subtract_assign */
    "case_stmt",			/* NN_switch */
    "switch_clause",			/* NN_switch_clause */
    "temp_init",			/* NN_temp_init */
    "template_body",			/* NN_template_body */
    "template_class",			/* NN_template_class */
    "template_fcn",			/* NN_template_fcn */
    "template_header",			/* NN_template_header */
    "template_static_data_mbr",		/* NN_template_static_data_mbr */
    "template_struct",			/* NN_template_struct */
    "template_union",			/* NN_template_union */
    "throw_expr",			/* NN_throw */
    "try_block",			/* NN_try_block */
    "type",				/* NN_type */
    "typeid",				/* NN_typeid */
    "unary_plus",			/* NN_unary_plus */
    "type_decl",			/* NN_union_def */
    "used_entity",			/* NN_used_entity */
    "using",				/* NN_using */
    "using_decl",			/* NN_using_decl */
    "call_expr",			/* NN_vacuous_destructor_call */
    "value_bit_field_ref",		/* NN_value_bit_field_ref */
    "component_ref",			/* NN_value_field_ref */
    "call_expr",			/* NN_value_vacuous_destructor_call */
    "variable",				/* NN_variable */
    "addr_expr",			/* NN_variable_address */
    "call_expr",			/* NN_virtual_call */
    "virtual_function_ptr",		/* NN_virtual_function_ptr */
    "vla_decl",				/* NN_vla_decl */
    "while_stmt",			/* NN_while */
    "bit_xor_expr",			/* NN_xor */
    "bit_xor_assign_expr"		/* NN_xor_assign */
};

/* The following array gives the node names in native form. */

const char* node_name_str[] = {
    "unknown_node",			/* NN_unknown_node */
    "add",				/* NN_add */
    "add_assign",			/* NN_add_assign */
    "address",				/* NN_address */
    "address_of_ellipsis",		/* NN_address_of_ellipsis */
    "and",				/* NN_and */
    "and_assign",			/* NN_and_assign */
    "asm",				/* NN_asm */
    "asm_entry",			/* NN_asm_entry */
    "asm_function",			/* NN_asm_function */
    "asm_pseudofunction",		/* NN_asm_pseudofunction */
    "assign",				/* NN_assign */
    "assume",				/* NN_assume */
    "base_name",			/* NN_base_name */
    "base_spec",			/* NN_base_spec */
    "bit_field_ref",			/* NN_bit_field_ref */
    "block",				/* NN_block */
    "block_assign",			/* NN_block_assign */
    "break",				/* NN_break */
    "call",				/* NN_call */
    "case_default",			/* NN_case_default */
    "case_label",			/* NN_case_label */
    "case_values",			/* NN_case_values */
    "cast",				/* NN_cast */
    "cast_to_base",			/* NN_cast_to_base */
    "cast_to_bool",			/* NN_cast_to_bool */
    "cast_to_derived",			/* NN_cast_to_derived */
    "catch",				/* NN_catch */
    "class_def",			/* NN_class_def */
    "comma",			  	/* NN_comma */
    "complement",			/* NN_complement */
    "condition",			/* NN_condition */
    "const_cast",			/* NN_const_cast */
    "constant",				/* NN_constant */
    "constant",				/* NN_constant_enum */
    "constant",				/* NN_constant_str */
    "constructor_call",			/* NN_constructor_call */
    "continue",				/* NN_continue */
    "ctor_init",			/* NN_ctor_init */
    "decl_id",				/* NN_decl_id_cnst */
    "decl_id",				/* NN_decl_id_cls */
    "decl_id",				/* NN_decl_id_enum */
    "decl_id",				/* NN_decl_id_fld */
    "decl_id",				/* NN_decl_id_fcn */
    "decl_id",				/* NN_decl_id_lbl */
    "decl_id",				/* NN_decl_id_ns */
    "decl_id",				/* NN_decl_id_strct */
    "decl_id",				/* NN_decl_id_tpl */
    "decl_id",				/* NN_decl_id_typ */
    "decl_id",				/* NN_decl_id_typd */
    "decl_id",				/* NN_decl_id_un */
    "decl_id",				/* NN_decl_id_var */
    "decl_specs",			/* NN_decl_specs */
    "declaration",			/* NN_declaration_no_dclrtr */
    "declaration",			/* NN_declaration_with_dclrtr */
    "declarator",			/* NN_declarator_data_mbr */
    "declarator",			/* NN_declarator_fcn */
    "declarator",			/* NN_declarator_typedef */
    "declarator",			/* NN_declarator_var */
    "default",				/* NN_default */
    "defined",				/* NN_defined */
    "delete",				/* NN_delete_sclr */
    "delete",				/* NN_delete_vec */
    "divide",				/* NN_divide */
    "divide_assign",			/* NN_divide_assign */
    "do_while",				/* NN_do_while */
    "dynamic_cast",			/* NN_dynamic_cast */
    "dynamic_init",			/* NN_dynamic_init */
    "empty",				/* NN_empty */
    "enum_def",				/* NN_enum_def */
    "enumerator_decl",			/* NN_enumerator_decl */
    "eq",				/* NN_eq */
    "error",				/* NN_error */
    "except",				/* NN_except */
    "expression",			/* NN_expression */
    "expression_statement",		/* NN_expression_statement */
    "extract_bit_field",		/* NN_extract_bit_field */
    "field",				/* NN_field */
    "field_ref",			/* NN_field_ref */
    "finally",				/* NN_finally */
    "flt_add",				/* NN_flt_add */
    "flt_add_assign",			/* NN_flt_add_assign */
    "flt_assign",			/* NN_flt_assign */
    "flt_divide",			/* NN_flt_divide */
    "flt_divide_assign",		/* NN_flt_divide_assign */
    "flt_eq",				/* NN_flt_eq */
    "flt_ge",				/* NN_flt_ge */
    "flt_gt",				/* NN_flt_gt */
    "flt_le",				/* NN_flt_le */
    "flt_lt",				/* NN_flt_lt */
    "flt_multiply_assign",		/* NN_flt_multiply_assign */
    "flt_multiply",			/* NN_flt_multiply */
    "flt_ne",				/* NN_flt_ne */
    "flt_negate",			/* NN_flt_negate */
    "flt_post_decr",			/* NN_flt_post_decr */
    "flt_post_incr",			/* NN_flt_post_incr */
    "flt_pre_decr",			/* NN_flt_pre_decr */
    "flt_pre_incr",			/* NN_flt_pre_incr */
    "flt_subtract",			/* NN_flt_subtract */
    "flt_subtract_assign",		/* NN_flt_subtract_assign */
    "for",				/* NN_for */
    "function_def",			/* NN_function_def */
    "ge",				/* NN_ge */
    "generic_call",			/* NN_generic_call */
    "goto",				/* NN_goto */
    "gt",				/* NN_gt */
    "if",				/* NN_if */
    "implicit_address",			/* NN_implicit_address */
    "indirect",				/* NN_indirect */
    "init",				/* NN_init */
    "int_add",				/* NN_int_add */
    "int_add_assign",			/* NN_int_add_assign */
    "int_assign",			/* NN_int_assign */
    "int_divide",			/* NN_int_divide */
    "int_divide_assign",		/* NN_int_divide_assign */
    "int_eq",				/* NN_int_eq */
    "int_ge",				/* NN_int_ge */
    "int_gt",				/* NN_int_gt */
    "int_le",				/* NN_int_le */
    "int_lt",				/* NN_int_lt */
    "int_multiply",			/* NN_int_multiply */
    "int_multiply_assign",		/* NN_int_multiply_assign */
    "int_ne",				/* NN_int_ne */
    "int_negate",			/* NN_int_negate */
    "int_post_decr",			/* NN_int_post_decr */
    "int_post_incr",			/* NN_int_post_incr */
    "int_pre_decr",			/* NN_int_pre_decr */
    "int_pre_incr",			/* NN_int_pre_incr */
    "int_subtract",			/* NN_int_subtract */
    "int_subtract_assign",		/* NN_int_subtract_assign */
    "label",				/* NN_label */
    "le",				/* NN_le */
    "logical_and",			/* NN_logical_and */
    "logical_or",			/* NN_logical_or */
    "lshift",				/* NN_lshift */
    "lshift_assign",			/* NN_lshift_assign */
    "lt",				/* NN_lt */
    "lvalue",				/* NN_lvalue */
    "lvalue_cast",			/* NN_lvalue_cast */
    "lvalue_dot_static",		/* NN_lvalue_dot_static */
    "lvalue_from_struct_rvalue",	/* NN_lvalue_from_struct_rvalue */
    "microsoft_try",			/* NN_microsoft_try */
    "multiply",				/* NN_multiply */
    "multiply_assign",			/* NN_multiply_assign */
    "name",				/* NN_name */
    "name_clause",			/* NN_name_clause */
    "namespace_def",			/* NN_namespace_def */
    "ne",				/* NN_ne */
    "negate",				/* NN_negate */
    "new",				/* NN_new_sclr */
    "new",				/* NN_new_vec */
    "not",				/* NN_not */
    "null_operand",			/* NN_null_operand */
    "object_lifetime",			/* NN_object_lifetime */
    "operation",			/* NN_operation */
    "or",				/* NN_or */
    "or_assign",			/* NN_or_assign */
    "parm_decl",			/* NN_parm_decl */
    "pm_arrow_field",			/* NN_pm_arrow_field */
    "pm_assign",			/* NN_pm_assign */
    "pm_call",				/* NN_pm_call */
    "pm_cast_to_base",			/* NN_pm_cast_to_base */
    "pm_cast_to_derived",		/* NN_pm_cast_to_derived */
    "pm_dot_field",			/* NN_pm_dot_field */
    "pm_eq",				/* NN_pm_eq */
    "pm_field_ref",			/* NN_pm_field_ref */
    "pm_ne",				/* NN_pm_ne */
    "points_to_static",			/* NN_points_to_static */
    "pre_incr",				/* NN_pre_incr */
    "pre_decr",				/* NN_pre_decr */
    "post_incr",			/* NN_post_incr */
    "post_decr",			/* NN_post_decr */
    "ptr_add",				/* NN_ptr_add */
    "ptr_add_assign",			/* NN_ptr_add_assign */
    "ptr_add_subscript",		/* NN_ptr_add_subscript */
    "ptr_assign",			/* NN_ptr_assign */
    "ptr_diff",				/* NN_ptr_diff */
    "ptr_eq",				/* NN_ptr_eq */
    "ptr_ge",				/* NN_ptr_ge */
    "ptr_gt",				/* NN_ptr_gt */
    "ptr_le",				/* NN_ptr_le */
    "ptr_lt",				/* NN_ptr_lt */
    "ptr_ne",				/* NN_ptr_ne */
    "ptr_post_decr",			/* NN_ptr_post_decr */
    "ptr_post_incr",			/* NN_ptr_post_incr */
    "ptr_pre_decr",			/* NN_ptr_pre_decr */
    "ptr_pre_incr",			/* NN_ptr_pre_incr */
    "ptr_subtract",			/* NN_ptr_subtract */
    "ptr_subtract_assign",		/* NN_ptr_subtract_assign */
    "question_colon",			/* NN_question_colon */
    "reinterpret_cast",			/* NN_reinterpret_cast */
    "remainder",			/* NN_remainder */
    "remainder_assign",			/* NN_remainder_assign */
    "return",				/* NN_return */
    "routine_address",			/* NN_routine_address */
    "rshift",				/* NN_rshift */
    "rshift_assign",			/* NN_rshift_assign */
    "runtime_sizeof",			/* NN_runtime_sizeof */
    "rvalue",				/* NN_rvalue */
    "rvalue_dot_static",		/* NN_rvalue_dot_static */
    "set_vla_size",			/* NN_set_vla_size */
    "statement",			/* NN_statement */
    "static_cast",			/* NN_static_cast */
    "struct_assign",			/* NN_struct_assign */
    "struct_def",			/* NN_struct_def */
    "subscript",			/* NN_subscript */
    "subtract",				/* NN_subtract */
    "subtract_assign",			/* NN_subtract_assign */
    "switch",				/* NN_switch */
    "switch_clause",			/* NN_switch_clause */
    "temp_init",			/* NN_temp_init */
    "template_body",			/* NN_template_body */
    "template_class",			/* NN_template_class */
    "template_fcn",			/* NN_template_fcn */
    "template_header",			/* NN_template_header */
    "template_static_data_mbr",		/* NN_template_static_data_mbr */
    "template_struct",			/* NN_template_struct */
    "template_union",			/* NN_template_union */
    "throw",				/* NN_throw */
    "try_block",			/* NN_try_block */
    "type",				/* NN_type */
    "typeid",				/* NN_typeid */
    "unary_plus",			/* NN_unary_plus */
    "union_def",			/* NN_union_def */
    "used_entity",			/* NN_used_entity */
    "using",				/* NN_using */
    "using_decl",			/* NN_using_decl */
    "vacuous_destructor_call",		/* NN_vacuous_destructor_call */
    "value_bit_field_ref",		/* NN_value_bit_field_ref */
    "value_field_ref",			/* NN_value_field_ref */
    "value_vacuous_destructor_call",	/* NN_value_vacuous_destructor_call */
    "variable",				/* NN_variable */
    "variable_address",			/* NN_variable_address */
    "virtual_call",			/* NN_virtual_call */
    "virtual_function_ptr",		/* NN_virtual_function_ptr */
    "vla_decl",				/* NN_vla_decl */
    "while",				/* NN_while */
    "xor",				/* NN_xor */
    "xor_assign"			/* NN_xor_assign */
};

/* The following array identifies the operators whose first operand
 * is an address (the IL represents "i=1" as if it were "&i=1").
 * In gcc-compatible mode, these operands are subjected to one level
 * of indirection to bring them back into conformity with the source
 * representation.
 */

struct {
    enum an_expr_operator_kind_tag code;
    a_byte_boolean first_is_address;
} expr_operand[] = {
eok_indirect,     FALSE,	
    eok_inegate,     FALSE,	
#if FIXED_POINT_ALLOWED
    eok_fxnegate,     FALSE,	
#endif /* FIXED_POINT_ALLOWED */
    eok_fnegate,     FALSE,	
    eok_unary_plus,     FALSE,	
    eok_not,     FALSE,	
    eok_cast,     FALSE,	
#ifdef CIL
    eok_base_class_cast,     FALSE,	
    eok_derived_class_cast,     FALSE,	
    eok_pm_base_class_cast,     FALSE,	
    eok_pm_derived_class_cast,     FALSE,	
    eok_lvalue_cast,     FALSE,	
    eok_dynamic_cast,     FALSE,	
    eok_bool_cast,     FALSE,	
    eok_complement,     FALSE,	
    eok_ipost_incr,     TRUE,	
    eok_ipost_decr,     TRUE,	
    eok_ipre_incr,     TRUE,	
    eok_ipre_decr,     TRUE,	
#if FIXED_POINT_ALLOWED
    eok_fxpost_incr, XXX,
    eok_fxpost_decr, XXX,
    eok_fxpre_incr, XXX,
    eok_fxpre_decr, XXX,
#endif /* FIXED_POINT_ALLOWED */
    eok_fpost_incr,     TRUE,	
    eok_fpost_decr,     TRUE,	
    eok_fpre_incr,     TRUE,	
    eok_fpre_decr,     TRUE,	
    eok_ppost_incr,     TRUE,	
    eok_ppost_decr,     TRUE,	
    eok_ppre_incr,     TRUE,	
    eok_ppre_decr,     TRUE,	
    eok_lvalue_from_struct_rvalue,     FALSE,	
#if MICROSOFT_EXTENSIONS_ALLOWED
    eok_assume,     FALSE,	
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#endif
#if defined(FIL) || C99_IL_EXTENSIONS_SUPPORTED
    eok_xnegate,     FALSE,	
#endif /* defined(FIL) || C99_IL_EXTENSIONS_SUPPORTED */
#ifdef FIL
    eok_char_length,     FALSE,	
    eok_address_of_value,     FALSE,	
    eok_loc,     FALSE,	
    eok_test_logical,     FALSE,	
#endif
    eok_iadd,     FALSE,	
    eok_isubtract,     FALSE,	
    eok_imultiply,     FALSE,	
    eok_idivide,     FALSE,	
    eok_ieq,     FALSE,	
    eok_ine,     FALSE,	
    eok_igt,     FALSE,	
    eok_ilt,     FALSE,	
    eok_ige,     FALSE,	
    eok_ile,     FALSE,
    eok_ignu_min,    FALSE,
    eok_ignu_max,    FALSE,
    eok_iassign,     TRUE,	
    eok_fadd,     FALSE,	
    eok_fsubtract,   FALSE,	
    eok_fmultiply,   FALSE,	
    eok_fdivide,     FALSE,	
    eok_feq,     FALSE,	
    eok_fne,     FALSE,	
    eok_fgt,     FALSE,	
    eok_flt,     FALSE,	
    eok_fge,     FALSE,	
    eok_fle,     FALSE,
    eok_fgnu_min, FALSE,
    eok_fgnu_max, FALSE,
    eok_fassign,     TRUE,	
    eok_padd,     FALSE,	
    eok_psubtract,     FALSE,	
    eok_passign,     TRUE,	
#if defined(FIL) || C99_IL_EXTENSIONS_SUPPORTED
    eok_xadd,     FALSE,	
    eok_xsubtract,     FALSE,	
    eok_xmultiply,     FALSE,	
    eok_xdivide,     FALSE,	
    eok_xeq,     FALSE,	
    eok_xne,     FALSE,	
    eok_xassign,     TRUE,	
#endif /* defined(FIL) || C99_IL_EXTENSIONS_SUPPORTED */
#if C99_IL_EXTENSIONS_SUPPORTED
    eok_xadd_assign,     TRUE,	
    eok_xsubtract_assign,     TRUE,	
    eok_xmultiply_assign,     TRUE,	
    eok_xdivide_assign,     TRUE,	
    eok_jmultiply,     FALSE,	
    eok_jdivide,     FALSE,
    eok_fjadd, FALSE,
    eok_jfadd, FALSE,
    eok_fjsubtract, FALSE,
    eok_jfsubtract, FALSE,
#endif /* C99_IL_EXTENSIONS_SUPPORTED */
#if GNU_COMPLEX_EXTENSIONS_ALLOWED
    eok_xconj,           FALSE,
    eok_real_part,       FALSE,
    eok_imag_part,       FALSE,
    eok_lvalue_real_part, FALSE,
    eok_lvalue_imag_part, FALSE,
#endif /* GNU_COMPLEX_EXTENSIONS_ALLOWED */
#ifdef FIL
    eok_complex,     FALSE,	
    eok_ceq,     FALSE,	
    eok_cne,     FALSE,	
    eok_cgt,     FALSE,	
    eok_clt,     FALSE,	
    eok_cge,     FALSE,	
    eok_cle,     FALSE,	
    eok_cassign,     FALSE,	
    eok_concat,     FALSE,	
    eok_i_to_i_expon,     FALSE,	
    eok_f_to_i_expon,     FALSE,	
    eok_x_to_i_expon,     FALSE,	
    eok_f_to_f_expon,     FALSE,	
    eok_x_to_x_expon,     FALSE,	
#endif
#ifdef CIL
    eok_remainder,     FALSE,	
    eok_padd_subsc,     FALSE,	
    eok_pdiff,     FALSE,	
    eok_peq,     FALSE,	
    eok_pne,     FALSE,	
    eok_pgt,     FALSE,	
    eok_plt,     FALSE,	
    eok_pge,     FALSE,	
    eok_ple,     FALSE,
    eok_pgnu_min,  FALSE,
    eok_pgnu_max,  FALSE,
    eok_pmeq,     FALSE,	
    eok_pmne,     FALSE,	
    eok_sassign,     TRUE,	
    eok_bassign,     FALSE,	
    eok_pmassign,     TRUE,	
    eok_iadd_assign,     TRUE,	
    eok_isubtract_assign,     TRUE,	
    eok_imultiply_assign,     TRUE,	
    eok_idivide_assign,     TRUE,	
    eok_remainder_assign,     TRUE,	
    eok_fadd_assign,     TRUE,	
    eok_fsubtract_assign,     TRUE,	
    eok_fmultiply_assign,     TRUE,	
    eok_fdivide_assign,     TRUE,	
    eok_padd_assign,     TRUE,	
    eok_psubtract_assign,     TRUE,	
    eok_shiftl_assign,     TRUE,	
    eok_shiftr_assign,     TRUE,	
    eok_and_assign,     TRUE,	
    eok_or_assign,     TRUE,	
    eok_xor_assign,     TRUE,	
    eok_subscript,     FALSE,	
    eok_field,     TRUE,	
    eok_value_field,     FALSE,	
    eok_bit_field,     TRUE,	
    eok_value_bit_field,     FALSE,	
    eok_extract_bit_field,     TRUE,	
    eok_pm_field,     TRUE,	
    eok_points_to_static,     FALSE,	
    eok_lvalue_dot_static,     FALSE,	
    eok_rvalue_dot_static,     FALSE,	
    eok_shiftl,     FALSE,	
    eok_shiftr,     FALSE,	
    eok_and,     FALSE,	
    eok_or,     FALSE,	
    eok_xor,     FALSE,	
    eok_comma,     FALSE,	
    eok_virtual_function_ptr,     FALSE,	
    eok_vacuous_destructor_call,     FALSE,	
    eok_value_vacuous_destructor_call,     FALSE,	
#endif
    eok_land,     FALSE,	
    eok_lor,     FALSE,	
#ifdef FIL
    eok_neqv,     FALSE,	
    eok_eqv,     FALSE,	
#endif
#ifdef CIL
    eok_question,     FALSE,	
#endif
#ifdef FIL
    eok_substring,     FALSE,	
    eok_value_substring,     FALSE,	
#endif
    eok_call,     FALSE,	
#ifdef CIL
    eok_virtual_call,     FALSE,	
    eok_pm_call,     FALSE,	
#endif
#ifdef FIL
    eok_fsubscript,     FALSE,	
    eok_value_fsubscript,     FALSE,	
#endif
    eok_va_start,     FALSE,	
    eok_va_arg,     FALSE,	
    eok_va_end,     FALSE,	
    eok_va_copy,     TRUE,
    eok_va_start_single_operand, FALSE,
#ifdef CIL
    eok_negate,     FALSE,	
    eok_post_incr,     TRUE,	
    eok_post_decr,     TRUE,	
    eok_pre_incr,     TRUE,	
    eok_pre_decr,     TRUE,	
    eok_add,     FALSE,	
    eok_subtract,     FALSE,	
    eok_multiply,     FALSE,	
    eok_divide,     FALSE,	
    eok_eq,     FALSE,	
    eok_ne,     FALSE,	
    eok_gt,     FALSE,	
    eok_lt,     FALSE,	
    eok_ge,     FALSE,	
    eok_le,     FALSE,
    eok_gnu_min, FALSE,
    eok_gnu_max, FALSE,
    eok_assign,     TRUE,	
    eok_add_assign,     TRUE,	
    eok_subtract_assign,     TRUE,	
    eok_multiply_assign,     TRUE,	
    eok_divide_assign,     TRUE,	
    eok_address,     FALSE,	
    eok_pm_dot_field,     TRUE,	
    eok_pm_arrow_field,     TRUE,	
    eok_static_cast,     FALSE,	
    eok_const_cast,     FALSE,	
    eok_reinterpret_cast,     FALSE,	
    eok_lvalue,     FALSE,	
    eok_rvalue,     FALSE,	
    eok_generic_call,     FALSE,	
    eok_generic_member_call,     FALSE,	
#endif /* ifdef CIL */
    eok_error,     FALSE	
};

/* The following variables are used for metering memory usage by the
 * functions in this file.
 */

static size_t ast_node_meter;
static size_t stmt_stack_meter;

void dump_ast_memory_meters()
{
    fprintf(stderr, "ast_node_meter =\t\t%10lu\n", (unsigned long)ast_node_meter);
    fprintf(stderr, "stmt_stack_meter =\t\t%10lu\n", (unsigned long)stmt_stack_meter);
}

#if DEBUG
static char *app(char *to, const char *from)
{
    strcpy(to, from);
    return to + strlen(from);
}

static char *entry_id(a_tagged_pointer ent)
{
    static char buffer[200];
    unsigned long id = il_entry_prefix_of(ent.ptr).SET_symid;
    char *p = app(app(buffer, "iek_"), il_entry_kind_names[ent.kind]);
    if (id != 0) {
	sprintf(p, " [%lu] %lx", id, (unsigned long)ent.ptr);
    }
    else {
	sprintf(p, " %lx", (unsigned long)ent.ptr);
    }
    return buffer;
}
#endif /* DEBUG */

/* The following variables are used to link parameter declarations and
 * the outer block of a function definition to the appropriate places
 * in the ast.
 */

static ast_node_info_ptr* fcn_parm_link;
ast_node_info_ptr fcn_declarator;

/* The following structure allows postponing a link in the AST until
 * after a given IL node has been processed (for cases that violate
 * the normal leaf-first processing of the IL tree).  When a new
 * ast_node_info is created for the given IL node, the stored link
 * is updated to point to the newly-created ast_node_info and the
 * pending_link is removed from the list.
 */

typedef struct pending_link* pending_link_ptr;

typedef struct pending_link {
    pending_link_ptr next;
    void* il_node;
    ast_node_info_ptr* link;
} pending_link;

static pending_link_ptr pending_links;
static pending_link_ptr free_pending_links;

/* The following function adds a new link to the list of pending links */

static void add_pending_link(void* il_node, ast_node_info_ptr* link) {
    pending_link_ptr p;
    if (free_pending_links) {
	p = free_pending_links;
	free_pending_links = p->next;
    }
    else p = (pending_link_ptr) malloc(sizeof(pending_link));
    p->next = pending_links;
    pending_links = p;
    p->il_node = il_node;
    p->link = link;
}

/* The following variables are used to allocate blocks of
 * ast_node_info objects which are then used one at a time rather than
 * allocating them one at a time, which is expensive.
 */

static ast_node_info_ptr ast_node_info_block;
static size_t next_free_ast_node_info;
#define AST_NODE_INFO_BLOCK_COUNT 2000

/* The following function creates a new ast_node_info object,
 * initializing its fields.  It also links the
 * newly-created node with its corresponding IL node.
 */

static ast_node_info_ptr new_ast_node_info(void* il_node) {
    ast_node_info_ptr p;
    if (!ast_node_info_block ||
	next_free_ast_node_info >= AST_NODE_INFO_BLOCK_COUNT) {
	ast_node_info_block = (ast_node_info_ptr)
		malloc(AST_NODE_INFO_BLOCK_COUNT *
		       sizeof(ast_node_info));
	ast_node_meter += AST_NODE_INFO_BLOCK_COUNT *
		sizeof(ast_node_info);
	next_free_ast_node_info = 0;
    }
    p = ast_node_info_block + next_free_ast_node_info++;
    p->first_child = NULL;
    p->next_sibling = NULL;
    p->text = "";
    p->node_name = NN_unknown_node;
    p->entry = NULL;
    p->start_pos = null_source_position;
    p->end_pos = null_source_position;
    p->use_len = FALSE;
    p->is_leaf = FALSE;
    p->suppress_node = FALSE;
    p->suppress_subtree = FALSE;
    if (il_node) {
	pending_link_ptr prev_plp = NULL;
	pending_link_ptr plp;
	il_entry_prefix_of(il_node).SET_info = p;
#if DEBUG
	if (db_ast) {
	    printf("Creating ast for entry %lx at %lx.\n", (long)il_node, (long)p);
	}
#endif
	for (plp = pending_links; plp; plp = plp->next) {
	    if (plp->il_node == il_node) {
		*plp->link = p;
		if (prev_plp) {
		    prev_plp->next = plp->next;
		}
		else pending_links = plp->next;
		plp->next = free_pending_links;
		free_pending_links = plp;
		break;
	    }
	    prev_plp = plp;
	}
    }
    return p;
}

/* The following function creates an ast_node_info object to represent
 * a nonexistent operand (e.g., a missing "then" clause or a missing
 * test in a "for" statement).  (Note that
 * trailing missing operands, such as the expression in a no-value
 * "return" statement or a missing "else" clause, are simply omitted
 * rather than represented by one of these nodes.)
 */

static ast_node_info_ptr make_null_ast_operand() {
    ast_node_info_ptr p = new_ast_node_info(NULL);
    p->node_name = NN_null_operand;
    p->text = "";
    p->is_leaf = FALSE;
    return p;
}

static void add_pos_to_ast(a_source_position_ptr start,
			   a_source_position_ptr end,
			   ast_node_info_ptr astp)
{ 
    if (!generate_multiple_IF_files) {
	/* This probably could work more sensibly, the way
	 * the multiple iffs case does, but it is like this
	 * to avoid any damage to the way things used to work.
	 */
	if (!astp->start_pos.seq) {
	   astp->start_pos = *start;
	}
	else {
	   astp->end_pos = *start;
	}
	if (end == NULL) {
	}
	else if (!astp->start_pos.seq) {
	   astp->start_pos = *end;
	}
	else {
	   astp->end_pos = *end;
	}
    }
    else {
	/* generating multiple iffs */
    	if (end != NULL) {
	    if (start->precise_column != 0 && end->precise_column != 0) {
	        astp->start_pos = *start;
	        astp->end_pos = *end;
	    }
	    else {
	        astp->start_pos.seq = 0;
	        astp->end_pos.seq = 0;
	    }
	}
	else {
	    if (start->precise_column != 0) {
	        astp->start_pos = *start;
	    }
	    else {
	        astp->start_pos.seq = 0;
	    }
	    astp->end_pos.seq = 0;
	}
    }
}

static void add_pos_and_len_to_ast(a_source_position_ptr pos,
                                   ast_node_info_ptr astp)
{ 
    astp->start_pos = *pos;
    astp->use_len = TRUE;
}

static void add_range_to_ast(a_source_range *range,
                             ast_node_info_ptr astp)
{ 
    if (range->end.seq) {
        add_pos_to_ast(&range->start, &range->end, astp);
    }
    else add_pos_and_len_to_ast(&range->start, astp);
}

#if DEBUG
static char *db_ast_text(ast_node_info_ptr astp)
{
    if (astp == NULL) {
	return "<NULL>";
    }
    else if (!astp->suppress_node) {
    	static char buf[2000];
	const char *nn = (GNU_compatible_AST ? node_name_str_gnu : node_name_str)[astp->node_name];
	if (nn == NULL) {
	    nn = "<NULL>";
	}
        sprintf(buf, "%lx (%s%s)", (long)astp, nn, astp->text != NULL ? astp->text : "<NULL>");
	return buf;
    }
    else {
    	static char buf[2000];
    	static char child[2000];
	const char *nn = (GNU_compatible_AST ? node_name_str_gnu : node_name_str)[astp->node_name];
	if (nn == NULL) {
	    nn = "<NULL>";
	}
	if (astp->first_child != NULL) {
	    child[0] = ' ';
            strcpy(child + 1, db_ast_text(astp->first_child));
	}
	else {
	    child[0] = '\0';
	}
        sprintf(buf, "(%lx (%s%s)) %s", (long)astp, nn, astp->text != NULL ? astp->text : "<NULL>", child);
	return buf;
    }
}
#endif

/* The following function modifies the link
 * pointer provided to point to the specified ast node..
 */

static void relink(ast_node_info_ptr* link, ast_node_info_ptr p) {
#if DEBUG
    if (db_ast) {
	printf("Relinking slot at %lx to point to %s.\n",
	       (long)link, db_ast_text(p));
    }
    if (p != NULL && link == &p->next_sibling || link == &p->first_child) {
        complain(error_csev, "Creating a sibling link to itself.");
    }
#endif
    *link = p;
}

/* The following variable is incremented for each declared entity in
 * the compilation unit to provide a unique symbol ID for the IF.  Its
 * initial value is "1" to allow the detection of unassigned values.
 */

unsigned long next_symid = 1;

/* The following macro extracts the SET_info field from the entry
 * prefix of a supplied IL node, if any; if the IL node pointer is
 * NULL, the value of the macro is also NULL.  It complains and
 * returns NULL if AST info is requested for a node from an
 * "uninteresting" file, since the SET_info field is used to point
 * to a sym_summary_info object, if anything, and not an
 * ast_node_info object.
 */

#if 0
/* fast macro version: */
#define ast_info_of(ilp) (ast_node_info_ptr) ( \
	/* if */ (ilp) ? \
	    /* then if */ ((il_entry_prefix_of(ilp).full_SET_dump) ? \
		/* then */ (il_entry_prefix_of(ilp).SET_info) : \
	    /* else */ (worry("Request for AST info of uninteresting node."), \
			(void*) NULL)) : \
	/* else */ (void*) NULL)
#else
/* slow function version (for easier debugging): */
ast_node_info_ptr ast_info_of(void* ilp) {
    if (ilp) {
	an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
	if (ep->full_SET_dump) {
	    return (ast_node_info_ptr) ep->SET_info;
	}
	else worry("Request for AST info of uninteresting node.");
    }
    return NULL;
}
#endif

/* The following function takes pointers to two IL nodes that are
 * linked as siblings in the IL.  (It takes both the current and next
 * nodes as parameters because the "next" field is located at
 * different positions in different IL nodes, so it cannot get the
 * "next" pointer in a generic fashion.)  It uses the
 * an_il_entry_prefix::SET_info field to get the corresponding
 * ast_node_info objects and link them also together as siblings.
 */

static void* mirror_hierarchy(void* cur, void* next) {
    ast_node_info_ptr cur_ast = ast_info_of(cur);
    ast_node_info_ptr next_ast = NULL;
    if (next) {
	next_ast = ast_info_of(next);
	if (cur_ast) {
	    relink(&cur_ast->next_sibling, next_ast);
	}
	if (!cur_ast || !next_ast) {
	    complain(error_csev, "Missing AST info!");
#if DEBUG
	    if (db_ast) {
		if (cur_ast == NULL) {
		    printf("Missing AST info from %lx.\n", (long)cur);
		}
		if (next_ast == NULL) {
		    printf("Missing AST info from %lx.\n", (long)next);
		}
	    }
#endif
	}
    }
    return next;
}

/* The following macro is used to form a list among
 * ast_node_info objects, parallel to that of the IL nodes.  It is
 * invoked with a pointer to an IL node that is the head of a list
 * linked by "next" fields.
 */

#define parallelize_links(astp_link, ilp, il_type) \
    { \
	if (ilp) { \
	    il_type p = ilp; /* make scratch local copy */ \
	    if (ast_info_of(p)) { \
                relink(&astp_link, (ast_node_info_ptr) \
		       ast_info_of(p)); \
	    } \
	    else { \
		complain(error_csev, "Missing link from IL to ast_node_info."); \
		relink(&astp_link, make_null_ast_operand()); \
	    } \
	    while (p = (il_type) mirror_hierarchy(p, p->next)) { } \
	} \
    }


/* The following function returns TRUE if any of the operands does not have
 * AST information and FALSE otherwise.
 */

static a_boolean missing_ast_for_operands(an_expr_node_ptr first_operand)
{
    a_boolean missing = FALSE;
    an_expr_node_ptr operand = first_operand;
    while(operand) {
        if (!ast_info_of(operand)) {
	    missing = TRUE;
	    break;
	}
	operand = operand->next;
    }
    return missing;
}

/* The following function creates an ast_node_info object whose body
 * is a supplied NUL-terminated string.  It copies the supplied
 * string as the node's operand, quoting and escaping embedded quotes,
 * and links the node at the tail of the top-level list.
 */

static ast_node_info_ptr make_string_ast_operand(const char* text) {
    ast_node_info_ptr np = new_ast_node_info(NULL);
    add_quoted_str_to_string(text, /*add_quotes=*/TRUE, &string_buf);
    np->text = terminate_string(&string_buf);
    np->is_leaf = TRUE;
    return np;
}

/* The following two variables denote the beginning and end of a group
 * of source sequence entries that are all declarators in a single
 * declaration.
 */

static a_source_sequence_entry_ptr start_of_declarator_chain;
static a_source_sequence_entry_ptr end_of_declarator_chain;


/* The following function makes a decl_specs ast node, based on the
 * position information passed in.  If the sequence number of the
 * specifiers position is 0, it creates and returns a null-operand ast
 * node info object.
 */

static ast_node_info_ptr make_decl_specs_ast(
        a_decl_position_supplement_ptr decl_pos) {
    a_node_name specs_name = NN_decl_specs;
    ast_node_info_ptr specs;
    ast_node_info_ptr name = NULL;
    if (start_of_declarator_chain && start_of_declarator_chain ==
	end_of_declarator_chain && end_of_declarator_chain->
	entity.kind == iek_src_seq_secondary_decl) {
	a_src_seq_secondary_decl_ptr sseq =
		(a_src_seq_secondary_decl_ptr) end_of_declarator_chain->
		entity.ptr;
	if (sseq->entity.kind == iek_type) {
	    a_type_ptr type = (a_type_ptr) sseq->entity.ptr;
	    if (type->kind == tk_class) {
		specs_name = NN_class_def;
	    }
	    else if (type->kind == tk_struct) {
		specs_name = NN_struct_def;
	    }
	    else if (type->kind == tk_union) {
		specs_name = NN_union_def;
	    }
	    else if (type->kind == tk_integer &&
		     type->variant.integer.enum_type) {
		specs_name = NN_enum_def;
	    }
	    if (specs_name != NN_decl_specs) {
		name = new_ast_node_info(NULL);
		name->node_name = NN_name_clause;
		add_symid_to_string(SET_symid_and_add_to_ast(type, iek_type, name), &string_buf);
		if (decl_pos && decl_pos->identifier_range.start.seq) {
		    add_range_to_ast(&decl_pos->identifier_range,
				     name);
		}
		name->text = terminate_string(&string_buf);
	    }
	}
    }
    if ((decl_pos && decl_pos->specifiers_range.start.seq) ||
	specs_name != NN_decl_specs) {
	specs = new_ast_node_info(NULL);
	specs->node_name = specs_name;
	if (decl_pos && decl_pos->specifiers_range.start.seq) {
	    add_range_to_ast(&decl_pos->specifiers_range,
			     specs);
	}
	specs->text = terminate_string(&string_buf);
	if (name) {
	    relink(&specs->first_child, name);
	}
    }
    else specs = make_null_ast_operand();
    return specs;
}

/* The following function returns the AST node name corresponding to
 * the kind of expression represented by the supplied IL expression
 * node.
 */

a_node_name expr_node_name(an_expr_node_ptr expr) {
    switch(expr->kind) {
    case enk_error:
	return NN_error;
    case enk_operation:
	switch(expr->variant.operation.kind) {
	case eok_indirect:
	    return NN_indirect;
	case eok_inegate:
	    return NN_int_negate;
	case eok_fnegate:
	    return NN_flt_negate;
	case eok_unary_plus:
	    return NN_unary_plus;
	case eok_not:
	    return NN_not;
	case eok_cast:
	    return NN_cast;
#ifdef CIL
	case eok_base_class_cast:
	    return NN_cast_to_base;
	case eok_derived_class_cast:
	    return NN_cast_to_derived;
	case eok_pm_base_class_cast:
	    return NN_pm_cast_to_base;
	case eok_pm_derived_class_cast:
	    return NN_pm_cast_to_derived;
	case eok_lvalue_cast:
	    return NN_lvalue_cast;
	case eok_dynamic_cast:
	    return NN_dynamic_cast;
	case eok_bool_cast:
	    return NN_cast_to_bool;
	case eok_complement:
	    return NN_complement;
	case eok_ipost_incr:
	    return NN_int_post_incr;
	case eok_ipost_decr:
	    return NN_int_post_decr;
	case eok_ipre_incr:
	    return NN_int_pre_incr;
	case eok_ipre_decr:
	    return NN_int_pre_decr;
	case eok_fpost_incr:
	    return NN_flt_post_incr;
	case eok_fpost_decr:
	    return NN_flt_post_decr;
	case eok_fpre_incr:
	    return NN_flt_pre_incr;
	case eok_fpre_decr:
	    return NN_flt_pre_decr;
	case eok_ppost_incr:
	    return NN_ptr_post_incr;
	case eok_ppost_decr:
	    return NN_ptr_post_decr;
	case eok_ppre_incr:
	    return NN_ptr_pre_incr;
	case eok_ppre_decr:
	    return NN_ptr_pre_decr;
	case eok_lvalue_from_struct_rvalue:
	    return NN_lvalue_from_struct_rvalue;
#if MICROSOFT_EXTENSIONS_ALLOWED
	case eok_assume:
	    return NN_assume;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#endif /* ifdef CIL */
#ifdef FIL
	case eok_xnegate:
	    return "";
	case eok_char_length:
	    return "";
	case eok_address_of_value:
	    return "";
	case eok_loc:
	    return "";
	case eok_test_logical:
	    return "";
#endif /* ifdef FIL */
	case eok_iadd:
	    return NN_int_add;
	case eok_isubtract:
	    return NN_int_subtract;
	case eok_imultiply:
	    return NN_int_multiply;
	case eok_idivide:
	    return NN_int_divide;
	case eok_ieq:
	    return NN_int_eq;
	case eok_ine:
	    return NN_int_ne;
	case eok_igt:
	    return NN_int_gt;
	case eok_ilt:
	    return NN_int_lt;
	case eok_ige:
	    return NN_int_ge;
	case eok_ile:
	    return NN_int_le;
	case eok_iassign:
	    return NN_int_assign;
	case eok_fadd:
	    return NN_flt_add;
	case eok_fsubtract:
	    return NN_flt_subtract;
	case eok_fmultiply:
	    return NN_flt_multiply;
	case eok_fdivide:
	    return NN_flt_divide;
	case eok_feq:
	    return NN_flt_eq;
	case eok_fne:
	    return NN_flt_ne;
	case eok_fgt:
	    return NN_flt_gt;
	case eok_flt:
	    return NN_flt_lt;
	case eok_fge:
	    return NN_flt_ge;
	case eok_fle:
	    return NN_flt_le;
	case eok_fassign:
	    return NN_flt_assign;
	case eok_padd:
	    return NN_ptr_add;
	case eok_psubtract:
	    return NN_ptr_subtract;
	case eok_passign:
	    return NN_ptr_assign;
#ifdef FIL
	case eok_xadd:
	    return "";
	case eok_xsubtract:
	    return "";
	case eok_xmultiply:
	    return "";
	case eok_xdivide:
	    return "";
	case eok_xeq:
	    return "";
	case eok_xne:
	    return "";
	case eok_xassign:
	    return "";
	case eok_complex:
	    return "";
	case eok_ceq:
	    return "";
	case eok_cne:
	    return "";
	case eok_cgt:
	    return "";
	case eok_clt:
	    return "";
	case eok_cge:
	    return "";
	case eok_cle:
	    return "";
	case eok_cassign:
	    return "";
	case eok_concat:
	    return "";
	case eok_i_to_i_expon:
	    return "";
	case eok_f_to_i_expon:
	    return "";
	case eok_x_to_i_expon:
	    return "";
	case eok_f_to_f_expon:
	    return "";
	case eok_x_to_x_expon:
	    return "";
#endif /* ifdef FIL */
#ifdef CIL
	case eok_remainder:
	    return NN_remainder;
	case eok_padd_subsc:
	    return NN_ptr_add_subscript;
	case eok_pdiff:
	    return NN_ptr_diff;
	case eok_peq:
	    return NN_ptr_eq;
	case eok_pne:
	    return NN_ptr_ne;
	case eok_pgt:
	    return NN_ptr_gt;
	case eok_plt:
	    return NN_ptr_lt;
	case eok_pge:
	    return NN_ptr_ge;
	case eok_ple:
	    return NN_ptr_le;
	case eok_pmeq:
	    return NN_pm_eq;
	case eok_pmne:
	    return NN_pm_ne;
	case eok_sassign:
	    return NN_struct_assign;
	case eok_bassign:
	    return NN_block_assign;
	case eok_pmassign:
	    return NN_pm_assign;
	case eok_iadd_assign:
	    return NN_int_add_assign;
	case eok_isubtract_assign:
	    return NN_int_subtract_assign;
	case eok_imultiply_assign:
	    return NN_int_multiply_assign;
	case eok_idivide_assign:
	    return NN_int_divide_assign;
	case eok_remainder_assign:
	    return NN_remainder_assign;
	case eok_fadd_assign:
	    return NN_flt_add_assign;
	case eok_fsubtract_assign:
	    return NN_flt_subtract_assign;
	case eok_fmultiply_assign:
	    return NN_flt_multiply_assign;
	case eok_fdivide_assign:
	    return NN_flt_divide_assign;
	case eok_padd_assign:
	    return NN_ptr_add_assign;
	case eok_psubtract_assign:
	    return NN_ptr_subtract_assign;
	case eok_shiftl_assign:
	    return NN_lshift_assign;
	case eok_shiftr_assign:
	    return NN_rshift_assign;
	case eok_and_assign:
	    return NN_and_assign;
	case eok_or_assign:
	    return NN_or_assign;
	case eok_xor_assign:
	    return NN_xor_assign;
	case eok_subscript:
	    return NN_subscript;
	case eok_field:
	    return NN_field_ref;
	case eok_value_field:
	    return NN_value_field_ref;
	case eok_bit_field:
	    return NN_bit_field_ref;
	case eok_value_bit_field:
	    return NN_value_bit_field_ref;
	case eok_extract_bit_field:
	    return NN_extract_bit_field;
	case eok_pm_field:
	    return NN_pm_field_ref;
	case eok_points_to_static:
	    return NN_points_to_static;
	case eok_lvalue_dot_static:
	    return NN_lvalue_dot_static;
	case eok_rvalue_dot_static:
	    return NN_rvalue_dot_static;
	case eok_shiftl:
	    return NN_lshift;
	case eok_shiftr:
	    return NN_rshift;
	case eok_and:
	    return NN_and;
	case eok_or:
	    return NN_or;
	case eok_xor:
	    return NN_xor;
	case eok_comma:
	    return NN_comma;
	case eok_virtual_function_ptr:
	    return NN_virtual_function_ptr;
	case eok_vacuous_destructor_call:
	    return NN_vacuous_destructor_call;
	case eok_value_vacuous_destructor_call:
	    return NN_value_vacuous_destructor_call;
#endif /* ifdef CIL */
	case eok_land:
	    return NN_logical_and;
	case eok_lor:
	    return NN_logical_or;
#ifdef FIL
	case eok_neqv:
	    return "";
	case eok_eqv:
	    return "";
#endif /* ifdef FIL */
#ifdef CIL
	case eok_question:
	    return NN_question_colon;
#endif /* ifdef CIL */
#ifdef FIL
	case eok_substring:
	    return "";
	case eok_value_substring:
	    return "";
#endif /* ifdef FIL */
	case eok_call:
	    return NN_call;
#ifdef CIL
	case eok_virtual_call:
	    return NN_virtual_call;
	case eok_pm_call:
	    return NN_pm_call;
#endif /* ifdef CIL */
#ifdef FIL
	case eok_fsubscript:
	    return "";
	case eok_value_fsubscript:
	    return "";
#endif /* ifdef FIL */
#ifdef CIL /* generic operators */
	case eok_negate:
	    return NN_negate;
	case eok_post_incr:
	    return NN_post_incr;
	case eok_post_decr:
	    return NN_post_decr;
	case eok_pre_incr:
	    return NN_pre_incr;
	case eok_pre_decr:
	    return NN_pre_decr;
	case eok_add:
	    return NN_add;
	case eok_subtract:
	    return NN_subtract;
	case eok_multiply:
	    return NN_multiply;
	case eok_divide:
	    return NN_divide;
	case eok_eq:
	    return NN_eq;
	case eok_ne:
	    return NN_ne;
	case eok_gt:
	    return NN_gt;
	case eok_lt:
	    return NN_lt;
	case eok_ge:
	    return NN_ge;
	case eok_le:
	    return NN_le;
	case eok_assign:
	    return NN_assign;
	case eok_add_assign:
	    return NN_add_assign;
	case eok_subtract_assign:
	    return NN_subtract_assign;
	case eok_multiply_assign:
	    return NN_multiply_assign;
	case eok_divide_assign:
	    return NN_divide_assign;
	case eok_address:
	    return NN_address;
	case eok_pm_dot_field:
	    return NN_pm_dot_field;
	case eok_pm_arrow_field:
	    return NN_pm_arrow_field;
	case eok_static_cast:
	    return NN_static_cast;
	case eok_const_cast:
	    return NN_const_cast;
	case eok_reinterpret_cast:
	    return NN_reinterpret_cast;
	case eok_lvalue:
	    return NN_lvalue;
	case eok_rvalue:
	    return NN_rvalue;
	case eok_generic_call:
	    return NN_generic_call;
#endif /* ifdef CIL */
	default:
	    complain_int(error_csev, "Unexpected operation kind $1",
		    expr->variant.operation.kind);
	    return NN_operation;
	}
    case enk_constant:
	if (expr->variant.constant->kind == ck_string ||
	    (expr->variant.constant->kind == ck_address &&
	     expr->variant.constant->variant.address.kind == abk_constant &&
	     expr->variant.constant->variant.address.variant.constant->
	     kind == ck_string)) {
	    return NN_constant_str;
	}
	else if (expr->variant.constant->kind == ck_integer &&
		 expr->variant.constant->source_corresp.name) {
	    return NN_constant_enum;
	}
	else return NN_constant;
    case enk_variable:
	return NN_variable;
    case enk_variable_address:
	return NN_variable_address;
#ifdef CIL
    case enk_field:
	return NN_field;
    case enk_temp_init:
	return NN_temp_init;
    case enk_new_delete: {
	a_new_delete_supplement_ptr new_del = (a_new_delete_supplement_ptr)
		expr->variant.new_delete;
	if (new_del->is_new) {
	    if (new_del->type->kind == tk_array) {
		return NN_new_vec;
	    }
	    return NN_new_sclr;
	}
	else if (new_del->array_delete) {
	    return NN_delete_vec;
	}
	else return NN_delete_sclr;
    }
	break;
    case enk_throw:
	return NN_throw;
    case enk_condition:
	return NN_condition;
    case enk_object_lifetime:
	return NN_object_lifetime;
    case enk_asm:
	return NN_asm_pseudofunction;
    case enk_typeid:
	return NN_typeid;
    case enk_runtime_sizeof:
	return NN_runtime_sizeof;
    case enk_address_of_ellipsis:
	return NN_address_of_ellipsis;
#endif /* ifdef CIL */
#ifdef FIL
    case enk_stmt_label_value:
	return "";
    case enk_char_variable_length:
	return "";
    case enk_data_implied_do_var:
	return "";
#endif /* ifdef FIL */
    case enk_routine_address:
	return NN_routine_address;
    default:
	complain_int(error_csev, "Unexpected expr kind $1",
		expr->kind);
	return NN_expression;
    }
}

/* The following variable points to the information needed to call
 * the destructor for the most-recently-constructed auto variable
 * (and, tracing backward from it using the lifetime and
 * next_in_destruction_list fields, all other destructible auto
 * variables and temporaries).
 */

static a_dynamic_init_ptr most_recent_destructible_auto;

/* The following declarations form a stack that is used to keep track
 * of nesting while going through the source sequence list.  Stackable
 * entities include classes, structs, and if, while, end_test_while,
 * for, try_block, and block statements.
 */

typedef struct a_class_template_body {
    a_source_sequence_entry_ptr old_seq;
    a_decl_position_supplement_ptr last_pos;
    a_decl_position_supplement_ptr this_pos;
    ast_node_info_ptr first_dclrtr_ast;
    ast_node_info_ptr last_dclrtr_ast;
    ast_node_info_ptr pending_class_def;
    a_symbol_ptr prev_member_sym;
} a_class_template_body, *a_class_template_body_ptr;

typedef struct a_stmt_stack_entry* a_stmt_stack_entry_ptr;

typedef struct a_stmt_stack_entry {
    a_stmt_stack_entry_ptr next;
    a_tagged_pointer stmt;
    a_source_sequence_entry_ptr
	ending_stmt;	/* the IL entity that ends this construct
			 * (for things that don't have end-of-
			 * construct markers).
			 */
    void* entity;	/* the IL entity that constitutes this construct
			 * (for checking & matching)
			 */
    an_object_lifetime_ptr lifetime;	/* If the ending_stmt has been
					 * mapped to the last enclosed
					 * statement of a compiler-generated
					 * block and the block had an
					 * associated lifetime that has
					 * destructions to be processed,
					 * this points to that lifetime;
					 * otherwise, NULL.
					 */
    a_dynamic_init_ptr destructible;	/* The value of most_recent_destructible_auto
					 * at the time this block was entered; when
					 * the block is popped, the value is restored,
					 * discarding all the nested destructible
					 * autos.
					 */
    ast_node_info_ptr* link;	/* where the next one will be linked */
				/* for this level of the stack. */
    a_boolean do_pop;	/* TRUE => pop before processing next stmt,
			 * i.e., the ending_stmt has been seen.
			 */
    a_class_template_body template; /* For handling declarations within
                                     * template body.
                                     */
} a_stmt_stack_entry;

a_stmt_stack_entry_ptr top_of_stmt_stack;
a_stmt_stack_entry_ptr free_stmt_stack_entry;

static int depth_of_stmt_stack() {
    int depth = 0;
    a_stmt_stack_entry_ptr ent = top_of_stmt_stack;
    for (; ent != NULL; ent = ent->next) {
	depth += 1;
    }
    return depth;
}

static void determine_stack_ending_stmt(a_statement_ptr stmt);

static void push_stmt(a_tagged_pointer stmt, ast_node_info_ptr node) {
    a_stmt_stack_entry_ptr ent;
#if DEBUG
    if (db_statement_stack) {
        dump_eptr(stdout, "push_stmt of", stmt.kind, stmt.ptr);
    }
#endif
    if (free_stmt_stack_entry) {
	ent = free_stmt_stack_entry;
	free_stmt_stack_entry = ent->next;
    }
    else {
	ent = (a_stmt_stack_entry_ptr)
	    malloc(sizeof(a_stmt_stack_entry));
	stmt_stack_meter += sizeof(a_stmt_stack_entry);
    }
    ent->next = top_of_stmt_stack;
    top_of_stmt_stack = ent;
    ent->stmt = stmt;
    ent->ending_stmt = NULL;
    ent->entity = NULL;
    ent->lifetime = NULL;
    ent->destructible = most_recent_destructible_auto;
    ent->link = (node != NULL ? &node->first_child : NULL);
    ent->do_pop = FALSE;
#if DEBUG
    if (db_statement_stack) {
        printf("depth of stack is %d\n", depth_of_stmt_stack());
    }
#endif
    if (stmt.kind == iek_statement) {
	determine_stack_ending_stmt((a_statement_ptr)stmt.ptr);
    }
}

static a_tagged_pointer pop_stmt() {
    a_stmt_stack_entry_ptr ent = top_of_stmt_stack;
#if DEBUG
    if (db_statement_stack) {
        printf("depth of stack is %d\n", depth_of_stmt_stack());
    }
#endif
    if (ent) {
#if DEBUG
	if (db_statement_stack) {
	    dump_eptr(stdout, "pop_stmt of", ent->stmt.kind, ent->stmt.ptr);
	}
#endif
	top_of_stmt_stack = ent->next;
	ent->next = free_stmt_stack_entry;
	most_recent_destructible_auto = ent->destructible;
	free_stmt_stack_entry = ent;
	return ent->stmt;
    }
    else {
	a_tagged_pointer error_val;
	complain(error_csev, "Pop of empty statement stack.");
	error_val.ptr = NULL;
	return error_val;
    }
}

static a_tagged_pointer pop_specific_stmt(a_tagged_pointer stmt,
					  a_boolean expected) {
    a_stmt_stack_entry_ptr ent = top_of_stmt_stack;
    if (ent != NULL && stmt_stack_match(ent->stmt, stmt)) {
	if (!expected) {
	    worry("Popping unexpected specific statement from stack.");
	}
        return pop_stmt();
    }
    else {
	a_tagged_pointer val;
	if (expected) {
	    worry("Mismatch in attempt to pop statement stack.");
	}
	val.ptr = NULL;
	return val;
    }
}

/* Like the above, except it goes ahead and pops a mismatched
 * statement, unless 'expected' is false. */
static a_tagged_pointer pop_expected_stmt(a_tagged_pointer stmt,
					  a_boolean expected) {
    a_stmt_stack_entry_ptr ent = top_of_stmt_stack;
    if (expected) {
	if (ent != NULL && !stmt_stack_match(ent->stmt, stmt)) {
	    worry("Popping statement from stack, not the expected.");
	}
	return pop_stmt();
    }
    else if (ent != NULL && stmt_stack_match(ent->stmt, stmt)) {
	worry("Popping surprising statement from stack.");
	return pop_stmt();
    }
    else {
	a_tagged_pointer val;
	val.ptr = NULL;
	return val;
    }
}

/* The following function sets the ending statement for the current
 * level on the statement stack, "looking through" compiler-generated
 * blocks (since they won't have source sequence entries to trigger
 * the stack pop, we need to transfer the end check to the outermost
 * contained statement that does.
 */

static void set_stack_ending_stmt(a_statement_ptr sp) {
    a_source_sequence_entry_ptr ending_stmt
		= sp != NULL ? sp->source_sequence_entry : NULL;
    if (ending_stmt == NULL) {
	while (sp && sp->kind == stmk_block && !sp->source_sequence_entry) {
	    sp = sp->variant.block.statements;
	    if (sp) {
		do {
		    if (sp->source_sequence_entry) {
			ending_stmt = sp->source_sequence_entry;
		    }
		} while (sp->next && (sp = sp->next));
	    }
	}
    }
    if (ending_stmt) {
	top_of_stmt_stack->ending_stmt = ending_stmt;
    }
    else top_of_stmt_stack->do_pop = TRUE;
#if DEBUG
    if (db_statement_stack) {
	fprintf(stdout, "Setting end of this statement using %s.\n",
		      sp != NULL && ending_stmt == sp->source_sequence_entry
				? "source sequence"
		    : ending_stmt != NULL ? "found outermost"
		    : "automatic pop");
	if (ending_stmt != NULL) {
	    fprintf(stdout, "To ");
	    dump_src_seq_entry(stdout, ending_stmt);
	}
    }
#endif
}

/* Depending on the structure of the given statement, find the source
 * sequence entry which indicates its end, and set the top_of_stmt_stack
 * accordingly.  The results can be:
 *  1. Set ending_stmt to the source sequence entry of a subsequent
 *     (nested) statement.
 *  2. Set entity when a matching src seq end of construct is expected.
 *  3. Set do_pop TRUE, if this statement contains no further entries.
 */
static void determine_stack_ending_stmt(a_statement_ptr stmt) {
    if (stmt == NULL) {
	/* is a fake block */
	top_of_stmt_stack->do_pop = TRUE;
    }
    else if (stmt->kind == stmk_if) {
	if (stmt->variant.if_stmt.else_statement) {
	    set_stack_ending_stmt(stmt->variant.if_stmt.else_statement);
	}
	else set_stack_ending_stmt(stmt->variant.if_stmt.then_statement);
    }
    else if (stmt->kind == stmk_while ||
	     stmt->kind == stmk_end_test_while) {
	set_stack_ending_stmt(stmt->variant.loop_statement);
    }
    else if (stmt->kind == stmk_block) {
	top_of_stmt_stack->entity = stmt;
    }
    else if (stmt->kind == stmk_for) {
	set_stack_ending_stmt(stmt->variant.for_loop.statement);
    }
    else if (stmt->kind == stmk_switch) {
	set_stack_ending_stmt(stmt->variant.switch_stmt.body_statement);
    }
    else if (stmt->kind == stmk_try_block) {
	a_try_supplement_ptr tsp = stmt->variant.try_block;
	if (tsp->handlers) {
	    a_handler_ptr hp;
	    for (hp = tsp->handlers; hp->next; hp = hp->next)
		    { }
	    set_stack_ending_stmt(hp->statement);
	}
	else set_stack_ending_stmt(tsp->statement);
    }
#ifdef MICROSOFT_EXTENSIONS_ALLOWED
    else if (stmt->kind == stmk_microsoft_try) {
	a_microsoft_try_supplement_ptr try_info =
		stmt->variant.microsoft_try;
	set_stack_ending_stmt(try_info->cleanup_statement);
    }
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
}

/* Search the statement stack for the entity.  Return -1 if it's not
 * in a stack entry, otherwise the count to where found.
 */
static int find_stmt_entity(void *ent) {
    a_stmt_stack_entry_ptr p = top_of_stmt_stack;
    int count = 0;
    while (p != NULL && p->entity != ent) {
    	count += 1;
	p = p->next;
    }
    return p == NULL ? -1 : count;
}

a_boolean AST_is_corrupted = FALSE;

static void check_for_circularity(ast_node_info_ptr astp) {
    ast_node_info_ptr p;
    for (p = astp->next_sibling; p != NULL; p = p->next_sibling) {
	if (p == astp) {
	    complain(catastrophe_csev, "Formed circular chain of siblings in AST.");
	    AST_is_corrupted = TRUE;
	    break;
	}
    }
}

/* The following function links an ast node at the end of the chain at
 * the current level of statement stack.
 */

static void chain_at_curr_stmt_stack_level(ast_node_info_ptr astp,
					   intr_info_ptr intrp) {
    if (astp) {
#if DEBUG
        a_boolean is_list = (astp->next_sibling != NULL);
#endif
	if (top_of_stmt_stack) {
	    ast_node_info_ptr *link = top_of_stmt_stack->link;
	    if (link != NULL) {
	        relink(link, astp);
	        top_of_stmt_stack->link = &astp->next_sibling;
	    }
	    else {
#if DEBUG
		if (db_ast) {
		    printf("Not relinking %lx because top of statement stack won't accept.\n", (long)astp);
		}
#endif
	    }
	}
	else if (intrp) {
	    if (!intrp->first_ast_node) {
#if DEBUG
		if (db_ast) {
		    printf("Relinking slot at first ast node to point to %s.\n",
			   db_ast_text(astp));
		}
#endif
		intrp->first_ast_node = astp;
	    }
	    else relink(&intrp->last_ast_node->next_sibling, astp);
	    intrp->last_ast_node = astp;
	}
        else complain(error_csev, "Invalid interesting information structure.");
#if DEBUG
	if (astp->next_sibling != NULL) {
	    if (is_list) {
	        worry("Chained a list.");
	    }
	    check_for_circularity(astp);
	}
#endif
    }
}

static ast_node_info_ptr create_tpl_mbr_decl_ast(a_decl_position_supplement_ptr pos,
						 ast_node_info_ptr class_def,
						 ast_node_info_ptr first_ast);

static void chain_pending_template_member_decl(intr_info_ptr intrp) {
    if (top_of_stmt_stack != NULL && top_of_stmt_stack->stmt.kind == iek_template) {
	a_class_template_body_ptr body = &top_of_stmt_stack->template;
	if (body->last_dclrtr_ast) {
	    /* Form new decl to hold the first-to-last list of declarators,
	     * link the decl into the declaration list,
	     * and start a new list of declarators. */
	    ast_node_info_ptr decl =
		    create_tpl_mbr_decl_ast(body->last_pos, body->pending_class_def,
					    body->first_dclrtr_ast);
	    chain_at_curr_stmt_stack_level(decl, intrp);
	}
	else if (body->pending_class_def) {
	    /* Class definition stands alone with no declarators.
	     * Link it into the declaration list. */
	    chain_at_curr_stmt_stack_level(body->pending_class_def, intrp);
	}
	body->last_dclrtr_ast = NULL;
	body->pending_class_def = NULL;
    }
}

/* The following function returns the extended declaration position
 * information for the declaration identified by the specified source
 * sequence entry, or NULL if the source sequence entry does not
 * identify a declaration.
 */

static a_decl_position_supplement_ptr
        decl_pos_of(a_source_sequence_entry_ptr seq) {
    if (seq) {
	if (seq->entity.kind == iek_type) {
	    return ((a_type_ptr) seq->entity.ptr)->
		    source_corresp.decl_pos_info;
	}
	else if (seq->entity.kind == iek_variable) {
	    return ((a_variable_ptr) seq->entity.ptr)->
		    source_corresp.decl_pos_info;
	}
	else if (seq->entity.kind == iek_field) {
	    return ((a_field_ptr) seq->entity.ptr)->
		    source_corresp.decl_pos_info;
	}
	else if (seq->entity.kind == iek_routine) {
	    return ((a_routine_ptr) seq->entity.ptr)->
		    source_corresp.decl_pos_info;
	}
	else if (seq->entity.kind == iek_src_seq_secondary_decl) {
	    a_src_seq_secondary_decl_ptr sseq =
		    (a_src_seq_secondary_decl_ptr) seq->entity.ptr;
	    if (!sseq->decl_pos_info && sseq->entity.kind == iek_type) {
		/* write around a bug for "enum E;" */
		a_type_ptr type = (a_type_ptr) sseq->entity.ptr;
		if (type->source_corresp.decl_position.seq ==
		    sseq->decl_position.seq &&
		    type->source_corresp.decl_position.mapped_column ==
		    sseq->decl_position.mapped_column) {
		    return type->source_corresp.decl_pos_info;
		}
	    }
	    if (sseq->entity.kind == iek_routine) {
		a_routine_ptr fcn = (a_routine_ptr) sseq->entity.ptr;
		if (fcn->source_corresp.source_sequence_entry == seq &&
		    fcn->source_corresp.decl_pos_info) {
		    /* entity's position is more accurate than
		     * secondary decl's -- e.g., including "=0"
		     * (pure virtual) specifier.
		     */
		    return fcn->source_corresp.decl_pos_info;
		}
	    }
	    return sseq->decl_pos_info;
	}
    }
    return NULL;
}

/* The following function sets the chain variables by scanning
 * from the indicated source sequence entry until it falls off the
 * end of the list or finds a source sequence entry that is not a
 * declarator in the same declaration.
 */

static void scan_declarator_chain(a_source_sequence_entry_ptr first_seq) {
    a_source_sequence_entry_ptr seq;
    a_decl_position_supplement_ptr first_seq_pos =
	    decl_pos_of(first_seq);
    a_decl_position_supplement_ptr seq_pos;
    start_of_declarator_chain = first_seq;
    end_of_declarator_chain = first_seq;
    for (seq = first_seq->next; seq; seq = seq->next) {
	seq_pos = decl_pos_of(seq);
	if (first_seq_pos && seq_pos &&
	    seq_pos->specifiers_range.end.seq &&
	    first_seq_pos->specifiers_range.end.seq ==
	    seq_pos->specifiers_range.end.seq &&
	    first_seq_pos->specifiers_range.end.mapped_column ==
	    seq_pos->specifiers_range.end.mapped_column) {
	    end_of_declarator_chain = seq;
	}
	else break;
    }
}

/* The following function links the ast nodes for the declarators that
 * share a common declaration specifiers sequence.  It also stores the
 * ending position of the last declarator linked, so that the parent
 * declaration's source range can include it.
 */

static void link_declarator_list(ast_node_info_ptr* link,
				 a_source_position_ptr end_pos) {
    a_source_sequence_entry_ptr seq;
    for (seq = start_of_declarator_chain; seq; seq = seq->next) {
	a_boolean is_incomplete_type = FALSE;
	a_decl_position_supplement_ptr seq_pos =
		decl_pos_of(seq);
	ast_node_info_ptr seq_ast = ast_info_of(seq->entity.ptr);
	if (!seq_ast && il_entry_prefix_of(seq).il_walk_flag !=
	    il_entry_prefix_of(seq->entity.ptr).il_walk_flag) {
	    /* This is a situation where the sequence entry appears in
	     * the tree walk but the entity it references does not.
	     * This can occur as the result of a source error; it's
	     * not exactly an "incomplete type," but setting that
	     * flag will let us proceed without further error.
	     */
	    is_incomplete_type = TRUE;
	    if (!il_entry_prefix_of(seq->entity.ptr).SET_symid) {
		/* It may still show up in the cross reference; make
		 * sure it doesn't get printed there.
		 */
		assign_error_symid(seq->entity.ptr);
	    }
	}
	else if (seq->entity.kind == iek_src_seq_secondary_decl) {
	    a_src_seq_secondary_decl_ptr sseq =
		    (a_src_seq_secondary_decl_ptr) seq->entity.ptr;
	    if (sseq->entity.kind == iek_type &&
		!(((a_type_ptr) sseq->entity.ptr)->kind == tk_typeref &&
		  ((a_type_ptr) sseq->entity.ptr)->source_corresp.name)) {
		/* incomplete type declaration, may not have declarator */
		is_incomplete_type = TRUE;
		seq_ast = NULL; /* to be added in make_declaration_tree */
	    }
	}
	if (seq_ast) {
	    relink(link, seq_ast);
	    link = &seq_ast->next_sibling;
	    il_entry_prefix_of(seq).SET_info = seq_ast;	/* mark as processed */
	}
	else if (!is_incomplete_type) {
	    complain(error_csev, "No ast info for node from sequence entry.");
	}
	if (seq_pos && !(seq == start_of_declarator_chain &&
			 is_incomplete_type)) {
	    *end_pos = seq_pos->variant.declarator_range.end;
	}
	if (seq->entity.kind == iek_variable) {
	    a_variable_ptr var = (a_variable_ptr) seq->entity.ptr;
	    if (var->init_kind == initk_static ||
		var->init_kind == initk_dynamic) {
		if (var->initializer_range.start.seq) {
		    *end_pos = var->initializer_range.end;
		}
	    }
	}
	if (seq == end_of_declarator_chain) {
	    break;
	}
    }
    start_of_declarator_chain = NULL;
    end_of_declarator_chain = NULL;
}

/* The following variable is used when a class, struct, union, or
 * enum definition is followed by declarators.  The code processing
 * the end-of-construct for the definition sets it to remember the
 * AST subtree for the declaration header until after the final
 * declarator is encountered; it is then used (instead of creating
 * a new declaration header from scratch) by make_declaration_tree.
 */

static ast_node_info_ptr pending_declaration_head;

/* The following function creates a declaration subtree for the
 * declarator chain from start_of_declarator_chain to
 * end_of_declarator_chain.  If pending_declaration_head is
 * non-NULL, it is used as the root for the declarator chain
 * rather than creating a declaration header from scratch.  If
 * do_chain is true, the resulting AST will be chained at the
 * current level in the statement stack (the normal case;
 * however, declarations in conditions are done when the
 * expression node is seen, which is before the source sequence
 * entry traversal begins, so the statement stack is not set up
 * correctly yet).  (A non-NULL pending_declaration_head also
 * suppresses the chaining, since the head was chained at the
 * time it was created.)
 */

static ast_node_info_ptr make_declaration_tree(a_boolean do_chain,
					       intr_info_ptr intrp) {
    a_decl_position_supplement_ptr pos =
	    decl_pos_of(start_of_declarator_chain);
    ast_node_info_ptr decl;
    ast_node_info_ptr specs;
    a_source_position end_pos;
    if (pending_declaration_head) {
	decl = pending_declaration_head;
	specs = decl->first_child;
	link_declarator_list(&specs->next_sibling, &end_pos);
    }
    else {
	decl = new_ast_node_info(NULL);
	specs = make_decl_specs_ast(pos);
	if (pos) {
	    /* initialize end_pos to handle the case of a declaration
	     * like "struct S;".  Cases with declarators will overwrite
	     * the end_pos with that of the last declarator.
	     */
	    end_pos = pos->specifiers_range.end;
	}
	link_declarator_list(&specs->next_sibling, &end_pos);
	if (specs->next_sibling) {
	    decl->node_name = NN_declaration_with_dclrtr;
	}
	else {
	    decl->node_name = NN_declaration_no_dclrtr;
	    if (GNU_compatible_AST) {
		decl->suppress_node = TRUE;
	    }
	}
	if (pos) {
	    add_pos_to_ast(  pos->specifiers_range.start.seq
			   ? &pos->specifiers_range.start
			   : &pos->variant.declarator_range.start, 
			   &end_pos,
			   decl);
	}
	decl->text = terminate_string(&string_buf);
	relink(&decl->first_child, specs);
    }
    if (do_chain && !pending_declaration_head) {
	chain_at_curr_stmt_stack_level(decl, intrp);
    }
    pending_declaration_head = NULL;
    return decl;
}

/* The following function is called to create the ast node info
 * subtree for a class, struct, union, or enum definition, consisting
 * of a <type>_def node (name reflecting the specific kind of type)
 * and a name_clause subtree.  The name clause is either just the
 * name or the class name and base class list.  The text for the
 * <type>_def node is left to be provided when the "}" is seen so
 * that the source range will be correct.
 */

static ast_node_info_ptr create_ast_info_for_type_def(a_type_ptr type,
						      ast_node_info_ptr name) {
    ast_node_info_ptr def = new_ast_node_info(type);
    if (!name) {
	a_decl_position_supplement_ptr pos =
		type->source_corresp.decl_pos_info;
	name = new_ast_node_info(NULL);
	name->node_name = NN_name_clause;
	add_symid_to_string(SET_symid_and_add_to_ast(type, iek_type, name), 
			    &string_buf);
	if (pos) {
	    add_range_to_ast(&pos->identifier_range,
			     name);
	}
	else add_pos_and_len_to_ast(&type->source_corresp.decl_position,
				    name);
	name->text = terminate_string(&string_buf);
    }
    relink(&def->first_child, name);
    return def;
}

/* The following function is called to create the ast node info
 * subtree for a variable, function, field, or typedef declarator.
 */

static ast_node_info_ptr create_ast_info_for_declarator(void* il_node,
        an_il_entry_kind kind) {
    ast_node_info_ptr dclrtr = new_ast_node_info(il_node);
    ast_node_info_ptr name;
    a_decl_position_supplement_ptr pos;
    a_source_position id_pos = null_source_position;
    a_node_name node_name = NN_unknown_node;
    if (kind == iek_routine) {
	a_routine_ptr fcn = (a_routine_ptr) il_node;
	a_boolean is_definition = FALSE;
	a_boolean is_comp_gen = fcn->compiler_generated;
	dclrtr->node_name = NN_declarator_fcn;
	if (fcn->assoc_scope != NULL_region_number) {
	    a_scope_ptr scope = il_header.region_scope_entry[fcn->assoc_scope];
	    if (scope && scope->assoc_block) {
		is_definition = TRUE;
	    }
	}
	if (is_definition) {
	    add_to_string_with_len(":def", 4, &string_buf);
	    if (is_comp_gen) {
		add_3_chars_to_string(",cg", &string_buf);
	    }
	}
	else if (is_comp_gen) {
	    add_3_chars_to_string(":cg", &string_buf);
	}
    }
    else if (kind == iek_field) {
	dclrtr->node_name = NN_declarator_data_mbr;
	add_to_string_with_len(":def", 4, &string_buf);
    }
    else if (kind == iek_type) {
	dclrtr->node_name = NN_declarator_typedef;
	add_to_string_with_len(":def", 4, &string_buf);
    }
    else {
	a_variable_ptr var = (a_variable_ptr) il_node;
	dclrtr->node_name = NN_declarator_var;
	if (var->storage_class != sc_extern) {
	    add_to_string_with_len(":def", 4, &string_buf);
	}
    }
    add_symid_to_string(SET_symid_and_add_to_ast(il_node, kind, dclrtr),
			&string_buf);
    if (kind == iek_variable) {
	a_variable_ptr var = (a_variable_ptr) il_node;
	a_source_position_ptr start_pos = NULL;
	a_source_position_ptr end_pos = NULL;
	node_name = NN_decl_id_var;
	pos = var->source_corresp.decl_pos_info;
	if (var->initializer_range.end.seq) {
	    end_pos = &var->initializer_range.end;
	}
	else if (var->is_parameter && param_var_has_default(var)) {
	    end_pos = &var->assoc_param_type->default_arg_expr->expr_range.end;
	}
	else if (pos) {
	    end_pos = &pos->variant.declarator_range.end;
	}
	if (pos && pos->variant.declarator_range.start.seq) {
	    add_pos_to_ast(&pos->variant.declarator_range.start,
			   end_pos, dclrtr);
	}
	else if (var->is_parameter && param_var_has_default(var)) {
	    add_pos_to_ast(&var->assoc_param_type->
			   default_arg_expr->expr_range.start,
			   end_pos, dclrtr);
	}
	else {
	    id_pos = var->source_corresp.decl_position;
	    if (var->initializer_range.end.seq) {
		add_pos_to_ast(&id_pos, end_pos, dclrtr);
	    }
	    else add_pos_and_len_to_ast(&id_pos, dclrtr);
	}
    }
    else if (kind == iek_field) {
	a_field_ptr field = (a_field_ptr) il_node;
	node_name = NN_decl_id_fld;
	pos = field->source_corresp.decl_pos_info;
	if (pos && pos->variant.declarator_range.start.seq) {
	    add_range_to_ast(&pos->variant.declarator_range,
			     dclrtr);
	}
	else {
	    id_pos = field->source_corresp.decl_position;
	    add_pos_and_len_to_ast(&id_pos, dclrtr);
	}
    }
    else if (kind == iek_routine) {
	a_routine_ptr fcn = (a_routine_ptr) il_node;
	node_name = NN_decl_id_fcn;
	if (fcn->compiler_generated) {
	    pos = NULL;
	}
	else {
	    pos = fcn->source_corresp.decl_pos_info;
	    if (pos && pos->variant.declarator_range.start.seq) {
		add_range_to_ast(&pos->variant.declarator_range,
				 dclrtr);
	    }
	    else {
		id_pos = fcn->source_corresp.decl_position;
		add_pos_and_len_to_ast(&id_pos, dclrtr);
	    }
	}
    }
    else if (kind == iek_type) {
	a_type_ptr type = (a_type_ptr) il_node;
	node_name = NN_decl_id_typd;
	pos = type->source_corresp.decl_pos_info;
	if (pos && pos->variant.declarator_range.start.seq) {
	    add_range_to_ast(&pos->variant.declarator_range,
			     dclrtr);
	}
	else {
	    id_pos = type->source_corresp.decl_position;
	    add_pos_and_len_to_ast(&id_pos, dclrtr);
	}
    }
    else complain_int(error_csev, "Unexpected declarator kind $1",
		 kind);
    dclrtr->text = terminate_string(&string_buf);
    if (node_name != NN_unknown_node) {
	name = new_ast_node_info(NULL);
        add_symid_to_string(SET_symid_and_add_to_ast(il_node, kind, name), 
			    &string_buf);
        if (pos && pos->identifier_range.start.seq) {
            add_range_to_ast(&pos->identifier_range, name);
        }
        else if (id_pos.seq) {
	    add_pos_and_len_to_ast(&id_pos, name);
        }
	name->text = terminate_string(&string_buf);
	name->node_name = node_name;
    }
    else {
	terminate_string(&string_buf);
	name = make_null_ast_operand();
    }
    relink(&dclrtr->first_child, name);
    return dclrtr;
}

/* The following function is called
 * to create a declaration tree once a sequence of declarators have been
 * recognized as all belonging to a single declaration.  It is similar
 * to make_declaration_tree except without benefit of the source
 * sequence list.
 */

static ast_node_info_ptr create_tpl_mbr_decl_ast(a_decl_position_supplement_ptr pos,
						 ast_node_info_ptr class_def,
						 ast_node_info_ptr first_ast) {
    ast_node_info_ptr decl = new_ast_node_info(NULL);
    ast_node_info_ptr specs = make_decl_specs_ast(pos);
    if (class_def) {
	class_def->suppress_node = TRUE;
	relink(&specs->first_child, class_def);
    }
    decl->node_name = NN_declaration_with_dclrtr;
    if (pos) {
	add_pos_to_ast(  pos->specifiers_range.start.seq
		       ? &pos->specifiers_range.start
		       : &pos->variant.declarator_range.start,
		       &pos->variant.declarator_range.end,
		       decl);
	decl->text = terminate_string(&string_buf);
    }
    else decl->text = "";
    relink(&decl->first_child, specs);
    relink(&specs->next_sibling, first_ast);
    return decl;
}

static void write_SMT_for_parameters(a_type_ptr type, string_buffer_ptr output_buffer)
{
    if (type != NULL && type->kind == tk_routine) {
	a_routine_type_supplement_ptr extra_info =
		type->variant.routine.extra_info;
	a_param_type_ptr param_type;
	for (param_type = extra_info->param_type_list; param_type;
	     param_type = param_type->next) {
	    if (!extra_info->assoc_routine &&
		param_type->decl_pos_info) {

		a_decl_position_supplement_ptr pos =
			param_type->decl_pos_info;
		a_boolean good_loc = add_pos_and_len_to_string_checking(&pos->identifier_range.start,
									&trial_buf, output_buffer);
		const char *loc_str = terminate_string(&trial_buf);
		if (good_loc) {
		    unsigned long parm_symid = SET_symid_and_write_sym(param_type, iek_param_type, output_buffer);
		    add_3_chars_to_string("SMT", output_buffer);
		    add_symid_to_string(parm_symid, output_buffer);
		    add_to_string(loc_str, output_buffer);
		    terminate_string(output_buffer);
		}
	    }
	}
    }
}

/* The following function creates the AST for a member of a template class.
 * Along the way, it also writes context and specialize rels for the
 * members. 
 */
static a_boolean create_ast_for_template_member(a_symbol_ptr const member_sym,
					   const a_source_sequence_entry_ptr member_seq,
					   a_source_range *const parent_range,
					   const unsigned long parent_symid,
					   a_source_sequence_entry_ptr *const old_seq,
					   a_boolean nested_class,
					   a_decl_position_supplement_ptr *const this_pos_param,
					   ast_node_info_ptr *const this_dclrtr_ast_param,
					   ast_node_info_ptr *const class_def_param,
					   intr_info_ptr intrp
					   ) {
    a_decl_position_supplement_ptr this_pos = NULL;
    ast_node_info_ptr this_dclrtr_ast = NULL;
    ast_node_info_ptr class_def = NULL;
    ast_node_info_ptr tpl_body = NULL;
    int access;
    unsigned long member_symid = 0;
    a_source_sequence_entry_ptr this_seq;
    a_src_seq_secondary_decl_ptr sseq;
    a_src_seq_secondary_decl_ptr orig_sseq = NULL;
    a_boolean do_push_stmt = FALSE;
    a_tagged_pointer member_entity = member_seq->entity;
    string_buffer_ptr output_buffer = get_buffer_from_intr_info(intrp);
    if (member_entity.kind == iek_src_seq_secondary_decl) {
        orig_sseq = ss_entry_ptr(member_seq, a_src_seq_secondary_decl_ptr);
	member_entity = orig_sseq->entity;
    }
    if (member_entity.kind == iek_type) {
	a_type_ptr type = (a_type_ptr) member_entity.ptr;
	if (is_auto_gen_member(&type->source_corresp)) {
	    /* Do nothing, this type is a member of an
	     * automatically-generated class template instance.
	     */
	}
	else if (type->kind == tk_class || type->kind == tk_struct
	    || type->kind == tk_union ||
	    (type->kind == tk_integer &&
	     type->variant.integer.enum_type)) {
	    /* push a level; leave allocation of ast nodes */
	    /* until doing the corresponding end seq */
	    if (is_src_seq_beginning_of_construct(member_seq)) {
		do_push_stmt = TRUE;
	    }
	    chain_pending_template_member_decl(intrp);
	}
    }
    switch (member_entity.kind) {
    case iek_type:
        {
	    a_type_ptr type_ptr = (a_type_ptr)member_entity.ptr;
	    if (type_ptr->kind == tk_class
		    || type_ptr->kind == tk_struct
		    || type_ptr->kind == tk_union) {
		a_type_ptr classp = member_sym->variant.class_struct_union.type;
		a_class_symbol_supplement_ptr cssp = member_sym->variant.
			class_struct_union.extra_info;
		a_template_symbol_supplement_ptr tssp = cssp ?
			cssp->template_info : NULL;
		a_template_ptr tpl = tssp ? tssp->il_template_entry : NULL;
#if DEBUG
		if (type_ptr != classp) {
		    worry("Symbol type differs from source sequence type.");
		}
#endif
		access = classp->source_corresp.access;
		if (is_in_source_range(&member_sym->variant.class_struct_union.type->
				       source_corresp.decl_position, parent_range)) {
		    if (!tpl) {
			a_symbol_ptr sym;
			ast_node_info_ptr tpl_hdr;
			a_source_range body_range;
			a_source_position name_clause_end =
				classp->source_corresp.decl_pos_info->identifier_range.end;
			a_base_class_ptr base;
			a_class_type_supplement_ptr extra_info =
				classp->variant.class_struct_union.extra_info;
			a_boolean no_definition = FALSE;
			walk_entry_and_subtree_for_IF((char*) classp, iek_type);
			member_symid = SET_symid_and_write_sym(classp, iek_type, 
							       output_buffer);
			this_pos = classp->source_corresp.decl_pos_info;
			if (!member_sym->is_class_member) {
			    /* no template attribute for a friend */
			}
			else {
			    write_atr(member_symid, "template", output_buffer);
			}
			if (!generate_multiple_IF_files) {
			  for (sym = tssp ? tssp->variant.class_template.instantiations :
			     NULL; sym; sym = sym->next) {
			    a_type_ptr type = sym->variant.class_struct_union.type;
			    if (type && sym->kind != sk_projection &&
				sym->kind != sk_type &&
				sym->variant.class_struct_union.extra_info) {
				write_specialize_rel(SET_symid_and_write_sym(type, iek_type, output_buffer), 
						     member_symid,
						     /*full_or_part=*/"full",
						     /*explicit=*/type->variant.
						     class_struct_union.is_specialized,
						     output_buffer);
			    }
			  }
			  for (sym = tssp ? tssp->variant.class_template.partial_specializations
			     : NULL; sym; sym = sym->next) {
			    a_template_ptr specialization =
				    sym->variant.template_info->il_template_entry;
			    if (specialization) {
				write_specialize_rel(SET_symid_and_write_sym(specialization, iek_template, output_buffer),
						     member_symid,
						     /*full_or_part=*/"partial",
						     /*explicit=*/TRUE,
						     output_buffer);
			    }
			  }
			}
			for (base = extra_info ? extra_info->base_classes : NULL;
			     base; base = base->next) {
			    if (base->direct) {
				name_clause_end = base->base_specifier_range.end;
			    }
			}
			if (classp->variant.class_struct_union.lbrace_pos.seq) {
			    a_source_sequence_entry_ptr members_of_member = NULL;
			    class_def = new_ast_node_info(NULL);
			    class_def->node_name = NN_declaration_no_dclrtr;
			    add_symid_to_string(member_symid, &string_buf);
			    add_range_to_ast(&this_pos->specifiers_range, class_def);
			    class_def->text = terminate_string(&string_buf);
			    tpl_hdr = ast_info_of(classp);
			    if (!tpl_hdr) {
				ast_node_info_ptr def_node = new_ast_node_info(NULL);
				no_definition = TRUE;
				tpl_hdr = new_ast_node_info(NULL);
				if (classp->kind == tk_struct) {
				    def_node->node_name = NN_struct_def;
				}
				else if (classp->kind == tk_union) {
				    def_node->node_name = NN_union_def;
				}
				else def_node->node_name = NN_class_def;
				add_range_to_ast(&this_pos->specifiers_range, def_node);
				def_node->text = terminate_string(&string_buf);
				if (GNU_compatible_AST) {
				    def_node->suppress_node = TRUE;
				}
				relink(&tpl_hdr->first_child, def_node);
			    }
			    tpl_body = new_ast_node_info(NULL);
			    if (classp->kind == tk_struct) {
				tpl_hdr->node_name = NN_template_struct;
			    }
			    else if (classp->kind == tk_union) {
				tpl_hdr->node_name = NN_template_union;
			    }
			    else tpl_hdr->node_name = NN_template_class;
			    if (!no_definition) {
				add_to_string_with_len(":def", 4, &string_buf);
			    }
			    add_symid_to_string(member_symid, &string_buf);
			    add_range_to_ast(&this_pos->specifiers_range, tpl_hdr);
			    tpl_hdr->text = terminate_string(&string_buf);
			    tpl_body->node_name = NN_template_body;
			    body_range.start = classp->variant.class_struct_union.lbrace_pos;
			    body_range.end = classp->variant.class_struct_union.rbrace_pos;
			    add_range_to_ast(&body_range, tpl_body);
			    tpl_body->text = terminate_string(&string_buf);
			    relink(&tpl_hdr->first_child->next_sibling, tpl_body);

#if CLASS_TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
			    /* Determine source sequence list for members of this member. */
			    if (members_of_member == NULL && tssp != NULL) {
				members_of_member = tssp->variant.class_template.source_sequence_list;
			    }
			    if (members_of_member == NULL) {
				worry_ulong("Missing source list for members of symbol $1", member_symid);
			    }
#endif
			    worry_ulong("Not handling template members of [$1].", member_symid);
#if CLASS_TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
#endif
			    relink(&class_def->first_child, tpl_hdr);
			}
			else {
			    /* Not defined in this compilation unit */
			    this_seq = get_secondary_src_seq(*old_seq, classp, iek_type, FALSE);
			    if (this_seq) {
				sseq = (a_src_seq_secondary_decl_ptr) this_seq->entity.ptr;
				process_entry((char*) sseq, iek_src_seq_secondary_decl);
				if (ast_info_of(sseq)) {
				    class_def = ast_info_of(sseq);
				    this_pos = sseq->decl_pos_info;
				    member_symid = SET_symid_and_write_sym(classp, iek_type,
								output_buffer);
				}
				*old_seq = this_seq->next;
			    }
			    else if (orig_sseq != NULL) {
				sseq = orig_sseq;
				process_entry((char*) sseq, iek_src_seq_secondary_decl);
				if (ast_info_of(sseq)) {
				    class_def = ast_info_of(sseq);
				    this_pos = sseq->decl_pos_info;
				    member_symid = SET_symid_and_write_sym(classp, iek_type,
								output_buffer);
				}
				*old_seq = this_seq->next;
			    }
			}
		    }
		    else if (tpl && interesting_node(tpl)) {
			if (!ast_info_of(tpl)) {
			    walk_entry_and_subtree_for_IF((char*) tpl, iek_template);
			}
			if (ast_info_of(tpl)) {
			    class_def = ast_info_of(tpl);
			    this_pos = classp->source_corresp.decl_pos_info;
			    member_symid = SET_symid_and_write_sym(tpl, iek_template,
							output_buffer);
			}
		    }
		}
		else {
		    /* Defined outside of template class */
		    if (tpl && classp) {
			ensure_same_symid(tpl, iek_template, classp, iek_type);
		    }
		    if (tpl != NULL && il_entry_prefix_of(tpl).SET_symid == parent_symid) {
			/* Class name injection -- ignore */
			return FALSE;
		    }
		    /*
		     * Here we tell get_secondary_src_seq() not to complain because
		     * it is not finding the secondary declaration. This does not
		     * seem to be a big problem, but it does indicate that there
		     * is a bug somewhere else that we haven't fixed. This acts as
		     * a temporary fix to one of the bugs related to bug 19471.
		     */
		    this_seq = get_secondary_src_seq(*old_seq, classp, iek_type, TRUE);
		    if (this_seq) {
			sseq = (a_src_seq_secondary_decl_ptr) this_seq->entity.ptr;
			process_entry((char*) sseq, iek_src_seq_secondary_decl);
			if (ast_info_of(sseq)) {
			    class_def = ast_info_of(sseq);
			    this_pos = sseq->decl_pos_info;
			    member_symid = SET_symid_and_write_sym(classp, iek_type,
								   output_buffer);
			}
			*old_seq = this_seq->next;
		    }
		}
	    }
	    else if (type_ptr->kind == tk_integer && type_ptr->variant.integer.enum_type) {
		a_type_ptr enump = member_sym->variant.enumeration.type;
		ast_node_info_ptr enum_node;
		ast_node_info_ptr* const_link;
		ast_node_info_ptr const_node;
		a_constant_ptr cp;
		a_symbol_ptr new_member_sym;
		walk_entry_and_subtree_for_IF((char*) enump, iek_type);
		member_symid = SET_symid_and_write_sym(enump, iek_type, output_buffer);
		access = enump->source_corresp.access;
		this_pos = enump->source_corresp.decl_pos_info;
		if (!generate_multiple_IF_files) {
		    write_tmpl_mbr_specialize_rels(member_sym, member_symid, output_buffer);
		}
		enum_node = ast_info_of(enump);
		if (!enum_node) {
		    enum_node = new_ast_node_info(NULL);
		    enum_node->first_child = new_ast_node_info(NULL);
		    enum_node->first_child->node_name = NN_name_clause;
		    add_symid_to_string(SET_symid_and_add_to_ast(enump, iek_type, enum_node->first_child), 
					&string_buf);
		    enum_node->first_child->text = terminate_string(&string_buf);
		}
		enum_node->node_name = NN_enum_def;
		add_to_string_with_len(":def", 4, &string_buf);
		add_symid_to_string(member_symid, &string_buf);
		add_range_to_ast(&this_pos->specifiers_range, enum_node);
		enum_node->text = terminate_string(&string_buf);
		class_def = new_ast_node_info(NULL);
		class_def->node_name = NN_declaration_no_dclrtr;
		add_symid_to_string(member_symid, &string_buf);
		add_range_to_ast(&this_pos->specifiers_range, class_def);
		class_def->text = terminate_string(&string_buf);
		relink(&class_def->first_child, enum_node);
		const_link = &enum_node->first_child->next_sibling;
		new_member_sym = member_sym;
		while (new_member_sym->next_in_scope &&
		       new_member_sym->next_in_scope->kind == sk_constant) {
		    new_member_sym = new_member_sym->next_in_scope;
		    cp = new_member_sym->variant.constant;
		    if (!generate_multiple_IF_files) {
		        write_tmpl_mbr_specialize_rels(new_member_sym,
						     SET_symid_and_write_sym(cp, iek_constant, output_buffer),
						     output_buffer);
		    }
		    const_node = ast_info_of(cp);
		    if (const_node) {
			relink(const_link, const_node);
			const_link = &const_node->next_sibling;
		    }
		}
	    }
	    else {
		a_type_ptr tp = member_sym->variant.type.ptr;
		this_pos = tp->source_corresp.decl_pos_info;
		walk_entry_and_subtree_for_IF((char*) tp, iek_type);
		this_dclrtr_ast = ast_info_of(tp);
		member_symid = SET_symid_and_write_sym(tp, iek_type, output_buffer);
		if (member_symid == parent_symid) {
		    /* Class name injection -- ignore */
		    return FALSE;
		}
		access = tp->source_corresp.access;
		if (!generate_multiple_IF_files) {
		    write_tmpl_mbr_specialize_rels(member_sym, member_symid, output_buffer);
		}
	    }
	}
        break;
    case iek_routine:
	{
	    a_routine_ptr routine = (a_routine_ptr)member_entity.ptr;
	    if (routine->source_corresp.is_class_member) {
		a_routine_ptr fcn = member_sym->variant.routine.ptr;
		a_template_symbol_supplement_ptr tssp
		    = member_sym->variant.routine.instance_ptr != NULL
			? member_sym->variant.routine.instance_ptr->template_info
			: NULL;
		a_template_ptr tpl = tssp ? tssp->il_template_entry : NULL;
		if (fcn != routine) {
		    worry("Inconsistent sequence of member function of template.");
		}
		access = fcn->source_corresp.access;
		if (is_in_source_range(&member_sym->variant.routine.ptr->
				       source_corresp.decl_position, parent_range)) {
		    a_template_instance_ptr inst;
		    a_symbol_ptr sym;
		    this_pos = fcn->source_corresp.decl_pos_info;
		    if (!fcn->defined && fcn->source_corresp.source_sequence_entry) {
			walk_entry_and_subtree_for_IF((char*) fcn->source_corresp.
					       source_sequence_entry,
					       iek_source_sequence_entry);
		    }
		    walk_entry_and_subtree_for_IF((char*) fcn, iek_routine);
		    if (this_dclrtr_ast != NULL) {
			worry("Setting template member function AST when it already has an AST.");
		    }
		    this_dclrtr_ast = ast_info_of(fcn);
		    if (this_dclrtr_ast) {
			if (fcn->assoc_scope != NULL_region_number) {
			    a_scope_ptr scope = il_header.
				    region_scope_entry[fcn->assoc_scope];
			    a_statement_ptr blk = scope->assoc_block;
			    if (blk) {
				ast_node_info_ptr body = new_ast_node_info(NULL);
				body->node_name = NN_template_body;
				add_pos_to_ast(&blk->position,
					       &blk->variant.block.extra_info->
					       final_position, body);
				body->text = terminate_string(&string_buf);
				relink(&this_dclrtr_ast->next_sibling, body);
			    }
			}
			else {
			    a_source_position blk_start = null_source_position;
			    a_source_position blk_end;
			    a_cached_token_ptr tp;
			    for (tp = tssp ? tssp->cache.tokens.first_token : NULL;
				 tp; tp = tp->next) {
				if (tp->token == tok_lbrace &&
				    !blk_start.seq) {
				    blk_start = tp->source_position;
				}
				else if (!tp->next ||
					 tp->next->token == tok_end_of_source) {
				    blk_end = tp->end_source_position;
				    break;
				}
			    }
			    if (blk_start.seq) {
				ast_node_info_ptr body = new_ast_node_info(NULL);
				body->node_name = NN_template_body;
				add_pos_to_ast(&blk_start, &blk_end, body);
				body->text = terminate_string(&string_buf);
				relink(&this_dclrtr_ast->next_sibling, body);
			    }
			    {
				/* Normally the AST for a function's declared
				 * parameters are handled in conjunction with
				 * its body, when the scope is processed, but
				 * when a template is not parsed, it does not
				 * have a scope, so chain them now. */
				a_type_ptr routine_type = routine->declared_type;
				a_routine_type_supplement_ptr extra_info
				    = (routine_type != NULL && routine_type->kind == tk_routine) ?
				       routine_type->variant.routine.extra_info : NULL;
				if (extra_info != NULL) {
				    if (this_dclrtr_ast->first_child != NULL) {
					parallelize_links(this_dclrtr_ast->first_child->next_sibling,
							  extra_info->param_type_list,
							  a_param_type_ptr);
				    }
				}
			    }
			}
		    }
		    member_symid = SET_symid_and_write_sym(fcn, iek_routine, output_buffer);
		    write_atr(member_symid, "template", output_buffer);
		    if (!generate_multiple_IF_files) {
		      for (inst = tssp ? tssp->variant.function.instantiations : NULL;
			 inst; inst = inst->next) {
			if (inst->instance_sym != inst->template_sym) {
			    /* "real," not the prototype instantiation */
			    a_routine_ptr fcn_inst = inst->instance_sym->
				    variant.routine.ptr;
			    write_specialize_rel(SET_symid_and_write_sym(fcn_inst, 
									 iek_routine,
									 output_buffer),
						 member_symid,
						 /*full_or_part=*/NULL,
						 /*explicit=*/fcn_inst->is_specialized,
						 output_buffer);
			}
		      }
		    }
		    if (orig_sseq != NULL) {
		        write_SMT_for_parameters(orig_sseq->declared_type, output_buffer);
		    }
		    else {
		        write_SMT_for_parameters(fcn->declared_type, output_buffer);
		    }
		}
		else {
		    if (tpl && fcn) {
			ensure_same_symid(tpl, iek_template, fcn, iek_routine);
		    }
		    this_seq = get_secondary_src_seq(*old_seq, fcn, iek_routine, FALSE);
		    if (this_seq) {
			sseq = (a_src_seq_secondary_decl_ptr) this_seq->entity.ptr;
			walk_entry_and_subtree_for_IF((char*) sseq, iek_src_seq_secondary_decl);
			if (ast_info_of(sseq)) {
			    this_dclrtr_ast = ast_info_of(sseq);
			    this_pos = sseq->decl_pos_info;
			    member_symid = SET_symid_and_write_sym(fcn, iek_routine,
								   output_buffer);
			}
			*old_seq = this_seq->next;
		        if (orig_sseq != NULL) {
		            write_SMT_for_parameters(orig_sseq->declared_type, output_buffer);
			}
		        else {
		            write_SMT_for_parameters(fcn->declared_type, output_buffer);
			}
		    }
		}
	    }
        }
        break;
    case iek_variable:
	{
	    a_variable_ptr var = member_sym->variant.static_data_member.variable;
	    a_template_symbol_supplement_ptr tssp
		= member_sym->variant.static_data_member.instance_ptr != NULL
		    ? member_sym->variant.static_data_member.instance_ptr->template_info
		    : NULL;
	    a_template_ptr tpl = tssp ? tssp->il_template_entry : NULL;
	    if (tpl && var) {
		ensure_same_symid(tpl, iek_template, var, iek_variable);
	    }
	    this_seq = get_secondary_src_seq(*old_seq, var, iek_variable, FALSE);
	    if (this_seq) {
		sseq = (a_src_seq_secondary_decl_ptr) this_seq->entity.ptr;
		process_entry((char*) sseq, iek_src_seq_secondary_decl);
		if (ast_info_of(sseq)) {
		    this_dclrtr_ast = ast_info_of(sseq);
		    this_pos = sseq->decl_pos_info;
		    member_symid = SET_symid_and_write_sym(var, iek_variable, 
							   output_buffer);
		    access = var->source_corresp.access;
		}
		*old_seq = this_seq->next;
	    }
	}
        break;
    case iek_field:
	{
	    a_field_ptr fp = member_sym->variant.field.ptr;
	    this_pos = fp->source_corresp.decl_pos_info;
	    walk_entry_and_subtree_for_IF((char*) fp, iek_field);
	    if (this_dclrtr_ast != NULL) {
		worry("Setting template field AST when it already has an AST.");
	    }
	    this_dclrtr_ast = ast_info_of(fp);
	    member_symid = SET_symid_and_write_sym(fp, iek_field, output_buffer);
	    access = fp->source_corresp.access;
	    if (!generate_multiple_IF_files) {
	      write_tmpl_mbr_specialize_rels(member_sym, member_symid, output_buffer);
	    }
	}
        break;
    case iek_template:
	if (member_sym->variant.template_info->il_template_entry &&
	    is_in_source_range(&member_sym->variant.template_info->
			       il_template_entry->source_corresp.decl_position,
			       parent_range)) {
	    a_template_ptr tplp = (a_template_ptr)
		    member_sym->variant.template_info->il_template_entry;
	    this_pos = tplp->source_corresp.decl_pos_info;
	    walk_entry_and_subtree_for_IF((char*) tplp, iek_template);
	    class_def = ast_info_of(tplp);
	    member_symid = SET_symid_and_write_sym(tplp, iek_template,
						   output_buffer);
	    access = tplp->source_corresp.access;
	}
        break;
    default:
        break;
    }

    if (do_push_stmt) {
        push_stmt(member_seq->entity, tpl_body != NULL ? tpl_body : class_def);
    }

    if (member_symid && !nested_class && member_sym->is_class_member) {
	write_context_rel(parent_symid, member_symid, access, output_buffer);
    }
    *this_pos_param = this_pos;
    *this_dclrtr_ast_param = this_dclrtr_ast;
    *class_def_param = class_def;
    return TRUE;
}

/* The following routine is used to create the AST for the compiler-
 * generated calls to destructors at the end of blocks and full
 * expressions and following gotos, breaks, continues, and returns
 * and chains the call at the current statement level. In addition,
 * it writes a "call" REL from the current function to the destructor.
 */

static ast_node_info_ptr create_destructor_call_ast(a_dynamic_init_ptr init,
						    a_boolean do_chain,
						    intr_info_ptr intrp) {
    ast_node_info_ptr call = NULL;
    if (init && init->destructor && !init->is_freeing_of_storage_on_exception) {
	ast_node_info_ptr fcn = new_ast_node_info(NULL);
	ast_node_info_ptr var = new_ast_node_info(NULL);
	call = new_ast_node_info(NULL);
	call->node_name = NN_call;
	add_3_chars_to_string(":cg", &string_buf);
	add_symid_to_string(SET_symid_and_add_to_ast(init->destructor->type->variant.routine.return_type, iek_type, call),
			    &string_buf);
	call->text = terminate_string(&string_buf);
	fcn->node_name = NN_routine_address;
	add_symid_to_string(SET_symid_and_add_to_ast(init->destructor, 
						     iek_routine, fcn), 
			    &string_buf);
	fcn->text = terminate_string(&string_buf);
	var->node_name = NN_variable;
	if (init->variable) {
	    add_symid_to_string(SET_symid_and_add_to_ast(init->variable, 
							 iek_variable, var),
				&string_buf);
	}
	else add_symid_to_string(SET_symid_and_add_to_ast(init, 
							  iek_dynamic_init, var), 
				 &string_buf);
	var->text = terminate_string(&string_buf);
	relink(&call->first_child, fcn);
	relink(&fcn->next_sibling, var);
	if (do_chain) {
	    chain_at_curr_stmt_stack_level(call, intrp);
	}
	if (intrp && fcn_id && !(generate_multiple_IF_files && 
			       is_auto_gen_routine(curr_fcn))) {
	    string_buffer_ref output_buffer = get_buffer_from_intr_info(intrp);
	    write_symbol_for_fcn_id(output_buffer);
	    write_rel(fcn_id, "call", SET_symid_and_write_sym(init->destructor, iek_routine, output_buffer), 
		      output_buffer);
	}
    }
    return call;
}

/* The following function processes destructor calls for some portion
 * of the destructor chain: it creates calls for each auto object
 * from the most recently constructed one up to (but not including)
 * the specified target lifetime.
 */

static void process_lifetime_window(an_object_lifetime_ptr targ_lifetime,
				    intr_info_ptr intrp) {
    a_dynamic_init_ptr init = most_recent_destructible_auto;
    if (init) {
	an_object_lifetime_ptr lifetime = init->lifetime;
	while (init && lifetime && lifetime != targ_lifetime) {
	    if (!init->is_constructor_init) {
		/* destructors for base and member objects are linked
		 * onto the destructors at the end of the constructor
		 * block if there might be an exception thrown in the
		 * body of the constructor; we'll process those
		 * separately in the destructor, so ignore them here.
		 */
		create_destructor_call_ast(init, /*do_chain=*/TRUE, intrp);
	    }
	    if (!init->next_in_destruction_list) {
		init = lifetime->parent_destruction_sublist;
		lifetime = lifetime->parent_lifetime;
	    }
	    else init = init->next_in_destruction_list;
	}
    }
}

void process_ast_of_entry(char *entry_ptr, an_il_entry_kind entry_kind) {
    ast_node_info_ptr astp;

    /* The following variable is used when process_entry is called */
    /* recursively to handle a source sequence sublist; it is set to */
    /* the source sequence entry that references the sublist, to allow */
    /* end of construct processing to find the true "next" source */
    /* sequence entry and allow, for instance, local class */
    /* declarations with declarators to be combined into a single */
    /* declaration, as they should be. */
    static a_source_sequence_entry_ptr subsequence_of = NULL;

    /* The following variable is set TRUE to indicate that AST should be
     * created for class members of templates, even though they appear
     * to be members of template instances (in this case, the prototype
     * instantiation).
     */
    a_boolean prototype_instance_member = entry_is_prototype_instance_member(entry_ptr, entry_kind);

    switch (entry_kind) {
    case iek_none: {
    }
	break;

    case iek_source_file: {
    }
	break;

    case iek_constant: {
	a_constant_ptr constant_ptr = (a_constant_ptr) entry_ptr;
	a_boolean is_case_label =
		(constant_ptr->source_corresp.decl_pos_info &&
		 constant_ptr->source_corresp.decl_pos_info->
		 specifiers_range.start.seq);
	a_type_ptr parent = (constant_ptr->source_corresp.is_class_member) ?
		constant_ptr->source_corresp.parent.class_type : NULL;
	a_boolean is_template_member = in_template_function ||
		(parent && parent->variant.class_struct_union.is_template_class &&
		 !parent->variant.class_struct_union.is_specialized &&
		 !prototype_instance_member);

	if (constant_ptr->from_huge_init) {
	    /*
	     * This constant is part of a huge initializer, that's why
	     * we don't need to generate anything for it (in order to
	     * avoid slowing down the model build unnecessarily)
	     */
	    break;
	}
	if (constant_ptr->from_asm && !emit_asm_strings) {
	    assign_error_symid(constant_ptr);
	    break;	/* no AST */
	}
	if (!in_file_scope(constant_ptr) ||
	    constant_ptr->kind == ck_aggregate ||
	    constant_ptr->kind == ck_dynamic_init ||
	    (constant_ptr->kind == ck_integer &&
	     constant_ptr->source_corresp.name)) {
	    /* make an ast_node_info object -- it's probably a case
	     * label or some such and thus not shared (or an enum
	     * constant, which will be in the enum's AST); the code to
	     * handle switch statements below assumes that the case
	     * constants have ast_node_info objects.
	     */
	    ast_node_info_ptr opnd = NULL;
	    unsigned long node_symid = 0;
	    astp = new_ast_node_info(constant_ptr);
	    if (is_case_label) {
		astp->node_name = NN_case_label;
	    }
	    else if (constant_ptr->kind == ck_string ||
		     (constant_ptr->kind == ck_address &&
		      constant_ptr->variant.address.kind == abk_constant &&
		      constant_ptr->variant.address.variant.constant->
		      kind == ck_string)) {
		astp->node_name = NN_constant_str;
	    }
	    else if (constant_ptr->kind == ck_integer &&
		     constant_ptr->source_corresp.name) {
		astp->node_name = NN_constant_enum;
	    }
	    else astp->node_name = NN_constant;
	    if (constant_ptr->const_expr &&
		interesting_node(constant_ptr->const_expr) &&
		ast_info_of(constant_ptr->const_expr)) {
		opnd = ast_info_of(constant_ptr->const_expr);
		if (opnd && (opnd->node_name == NN_constant ||
			     opnd->node_name == NN_constant_enum) &&
		    is_case_label) {
		    opnd->suppress_node = TRUE;
		    node_symid = SET_symid_and_add_to_ast(constant_ptr->const_expr->variant.constant, iek_constant, astp);
		}
		if (!constant_ptr->source_corresp.name &&
		    !is_case_label) {
		    astp->suppress_node = TRUE;
		}
	    }
	    else if (constant_ptr->kind == ck_aggregate) {
		a_constant_ptr subconst;
		ast_node_info_ptr subnode;
		ast_node_info_ptr* link = NULL;
		if (GNU_compatible_AST && constant_ptr->type) {
		    node_symid = SET_symid_and_add_to_ast(constant_ptr->type, iek_type, astp);
		}
		for (subconst = constant_ptr->variant.aggregate.first_constant;
		     subconst; subconst = subconst->next) {
		    subnode = ast_info_of(subconst);
		    if (!subnode) {
			ast_node_info_ptr subopnd;
			subnode = new_ast_node_info(subconst);
			if (subconst->const_expr) {
			    subopnd = ast_info_of(subconst->const_expr);
			}
			else {
			    subopnd = new_ast_node_info(NULL);
			    add_1_char_to_string('\"', &string_buf);
			    constant_to_string(subconst, &string_buf);
			    add_1_char_to_string('\"', &string_buf);
			    subopnd->text = terminate_string(&string_buf);
			    subopnd->is_leaf = TRUE;
			}
			if (subconst->kind == ck_string ||
			    (subconst->kind == ck_address &&
			     subconst->variant.address.kind == abk_constant &&
			     subconst->variant.address.variant.constant->
			     kind == ck_string)) {
			    subnode->node_name = NN_constant_str;
			}
			else if (subconst->kind == ck_integer &&
				 subconst->source_corresp.name) {
			    subnode->node_name = NN_constant_enum;
			}
			else subnode->node_name = NN_constant;
			if (subconst->const_expr &&
			    !subconst->source_corresp.name) {
			    subnode->suppress_node = TRUE;
			    subnode->text = "";
			}
			else {
			    add_symid_to_string(SET_symid_and_add_to_ast(subconst, iek_constant, subnode),
						&string_buf);
			    add_pos_and_len_to_ast(&subconst->
						   source_corresp.decl_position,
						   subnode);
			    subnode->text = terminate_string(&string_buf);
			}
			relink(&subnode->first_child, subopnd);
		    }
		    if (link) {
			relink(link, subnode);
		    }
		    else opnd = subnode;
		    link = &subnode->next_sibling;
		}
	    }
	    else if (constant_ptr->kind == ck_dynamic_init &&
		     interesting_node(constant_ptr->variant.dynamic_init)) {
		opnd = ast_info_of(constant_ptr->variant.dynamic_init);
		astp->suppress_node = TRUE;
		if (GNU_compatible_AST) {
		    opnd->suppress_node = TRUE;
		}
	    }
	    if (!opnd) {
		opnd = new_ast_node_info(NULL);
		add_1_char_to_string('\"', &string_buf);
		constant_to_string(constant_ptr, &string_buf);
		add_1_char_to_string('\"', &string_buf);
		opnd->text = terminate_string(&string_buf);
		opnd->is_leaf = TRUE;
	    }
	    if (astp->suppress_node) {
		astp->text = "";
	    }
	    else {
		if (!node_symid) {
		    node_symid = SET_symid_and_add_to_ast(constant_ptr, iek_constant, astp);
		}
		add_symid_to_string(node_symid, &string_buf);
		if (constant_ptr->source_corresp.decl_pos_info &&
		    constant_ptr->source_corresp.decl_pos_info->
		    variant.enum_value_range.end.seq) {
		    add_pos_to_ast(&constant_ptr->source_corresp.decl_pos_info->
				   identifier_range.start,
				   &constant_ptr->source_corresp.decl_pos_info->
				   variant.enum_value_range.end, astp);
		}
		else if (is_case_label) {
		    add_range_to_ast(&constant_ptr->source_corresp.
				     decl_pos_info->specifiers_range,
				     astp);
		}
		else add_pos_and_len_to_ast(&constant_ptr->
					    source_corresp.decl_position,
					    astp);
		astp->text = terminate_string(&string_buf);
	    }
	    relink(&astp->first_child, opnd);
	}
    }
	break;

    case iek_param_type: {
    }
	break;

    case iek_routine_type_supplement: {
    }
	break;

    case iek_based_type_list_member: {
    }
	break;

    case iek_type: {
	a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	a_boolean is_typedef = FALSE;
	unsigned long type_symid = 0;
	a_boolean is_class_struct_union = (type_ptr->kind == tk_class ||
					   type_ptr->kind == tk_struct ||
					   type_ptr->kind == tk_union);

	if (type_ptr->kind == tk_integer &&
		 type_ptr->variant.integer.enum_type) {
	    if (type_ptr->variant.integer.enum_info.constant_list) {
		/* Only do AST if is definition, not just
		 * elaborated type specifier (illegal, but only
		 * flagged in pedantic mode).
		 */
		create_ast_info_for_type_def(type_ptr, NULL);
	    }
	}
	else if (type_ptr->kind == tk_routine) {
	    a_routine_type_supplement_ptr extra_info =
		    type_ptr->variant.routine.extra_info;
	    a_param_type_ptr param_type;
	    a_type_ptr return_basetype;
	    a_boolean returns_function;
	    for (param_type = extra_info->param_type_list; param_type;
		 param_type = param_type->next) {
		if (!extra_info->assoc_routine &&
		    param_type->decl_pos_info) {
		    /* param is not part of a definition; must do
		     * entire SYM, type REL, auto ATR now.
		     */
		    a_decl_position_supplement_ptr pos =
			    param_type->decl_pos_info;
		    an_expr_node_ptr dft_expr = param_type->default_arg_expr;
		    a_type_ptr parm_basetype;
		    a_boolean parm_is_fcn_type;
		    ast_node_info_ptr* nested_parm_link;
		    check_for_fcn_basetype(param_type->type, &parm_basetype,
					   &parm_is_fcn_type);

		    /* Construct AST for parameter. */
		    {
			ast_node_info_ptr parm_ast =
				new_ast_node_info(param_type);
			ast_node_info_ptr specs =
				make_decl_specs_ast(pos);
			ast_node_info_ptr dclrtr;
			ast_node_info_ptr name;
			ast_node_info_ptr dft;
			parm_ast->node_name = NN_parm_decl;
			add_symid_to_string(SET_symid_and_add_to_ast(param_type,
								     iek_param_type,
								     parm_ast),
					    &string_buf);
			if (param_type->decl_pos_info) {
			    add_pos_to_ast(
				param_type->decl_pos_info->specifiers_range.start.seq
				    ? &param_type->decl_pos_info->specifiers_range.start
				    : &param_type->decl_pos_info->variant.declarator_range.start,
				dft_expr
				    ? &dft_expr->expr_range.end
				    : param_type->decl_pos_info->variant.declarator_range.end.seq
					  ? &param_type->decl_pos_info->variant.declarator_range.end
					  : &param_type->decl_pos_info->specifiers_range.end,
				parm_ast);

			}
			parm_ast->text = terminate_string(&string_buf);
			relink(&parm_ast->first_child, specs);
			if (pos->variant.declarator_range.start.seq || dft_expr) {
			    dclrtr = new_ast_node_info(NULL);
			    dclrtr->node_name = NN_declarator_var;
			    if (GNU_compatible_AST) {
				dclrtr->suppress_node = TRUE;
			    }
			    add_symid_to_string(SET_symid_and_add_to_ast(param_type, iek_param_type, dclrtr),
						&string_buf);
			    add_pos_to_ast(
				pos->variant.declarator_range.start.seq
				    ? &pos->variant.declarator_range.start
                                    : &dft_expr->expr_range.start,
			        dft_expr
				    ? &dft_expr->expr_range.end
                                    : &pos->variant.declarator_range.end,
			        dclrtr);
			    dclrtr->text = terminate_string(&string_buf);
			    if (pos->identifier_range.start.seq) {
				name = new_ast_node_info(NULL);
				name->node_name = NN_decl_id_var;
				add_symid_to_string(SET_symid_and_add_to_ast(param_type, iek_param_type, name), 
						    &string_buf);
				add_range_to_ast(&pos->identifier_range,
						 name);
				name->text = terminate_string(&string_buf);
				nested_parm_link = &name->next_sibling;
			    }
			    else if (dft_expr || parm_is_fcn_type) {
				name = make_null_ast_operand();
				nested_parm_link = &name->next_sibling;
			    }
			    else name = NULL;
			    if (parm_is_fcn_type) {
				a_param_type_ptr ptp;
				for (ptp = parm_basetype->variant.routine.
				     extra_info->param_type_list; ptp;
				     ptp = ptp->next) {
				    ast_node_info_ptr nested_parm_ast =
					    ast_info_of(ptp);
				    if (nested_parm_ast == NULL) {
					break;
				    }
				    relink(nested_parm_link, nested_parm_ast);
				    nested_parm_link = &nested_parm_ast->
					    next_sibling;
				}
			    }
			    if (dft_expr) {
				ast_node_info_ptr dft = new_ast_node_info(NULL);
				dft->node_name = NN_default;
				add_range_to_ast(&dft_expr->expr_range,
						 dft);
				dft->text = terminate_string(&string_buf);;
				relink(&dft->first_child, ast_info_of(dft_expr));
				relink(nested_parm_link, dft);
				if (GNU_compatible_AST) {
				    dft->suppress_node = TRUE;
				}
			    }
			    if (name) {
				relink(&dclrtr->first_child, name);
			    }
			    relink(&specs->next_sibling, dclrtr);
			}
		    }
		}
	    }
	    /* If the basetype of the return type is a function type, we
	     * created AST subtrees for the parameters of that type;
	     * however, there's no place in the AST structure to plug
	     * them in, so we need to suppress them.
	     */
	    check_for_fcn_basetype(type_ptr->variant.routine.return_type,
				   &return_basetype, &returns_function);
	    if (returns_function) {
		for (param_type = return_basetype->variant.routine.
		     extra_info->param_type_list; param_type;
		     param_type = param_type->next) {
		    ast_node_info_ptr parm_ast =
			    ast_info_of(param_type);
		    if (parm_ast) {
			parm_ast->suppress_subtree = TRUE;
		    }
		}
	    }
	}
	else if (type_ptr->kind == tk_array) {
	}
	else if (type_ptr->kind == tk_class || type_ptr->kind ==
		 tk_struct || type_ptr->kind == tk_union) {
	    ast_node_info_ptr name_clause = NULL;
	    ast_node_info_ptr* name_clause_link;
	    a_source_position name_clause_end;
	    a_field_ptr field;
	    a_class_type_supplement_ptr extra_info =
		    type_ptr->variant.class_struct_union.extra_info;
	    a_scope_ptr class_scope = (extra_info) ? extra_info->assoc_scope :
		    NULL;
	    a_base_class_ptr base;
	    a_constant_ptr constant;
	    a_type_ptr type;
	    a_variable_ptr variable;
	    a_routine_ptr routine;
	    a_using_decl_ptr using_decl;
	    a_routine_list_entry_ptr friend_fcn;
	    a_class_list_entry_ptr friend_class;
	    if (extra_info) {
		for (base = extra_info->base_classes; base; base =
		     base->next) {
		    if (base->direct) {
			ast_node_info_ptr base_clause;
			if (!name_clause) {
			    ast_node_info_ptr name = new_ast_node_info(NULL);
			    a_decl_position_supplement_ptr pos =
				    type_ptr->source_corresp.decl_pos_info;
			    name->node_name = NN_name;
			    add_symid_to_string(SET_symid_and_add_to_ast(
							type_ptr, iek_type, name),
                                                &string_buf);
			    if (pos) {
				add_range_to_ast(&pos->identifier_range,
						 name);
			    }
			    else add_pos_and_len_to_ast(
				    &type_ptr->source_corresp.decl_position,
				    name);
			    name->text = terminate_string(&string_buf);
			    name_clause = new_ast_node_info(NULL);
			    relink(&name_clause->first_child, name);
			    name_clause_link = &name_clause->first_child->
				    next_sibling;
			}
			base_clause = new_ast_node_info(base);
			base_clause->node_name = NN_base_spec;
			add_symid_to_string(SET_symid_and_add_to_ast(base->type, iek_type, base_clause), 
					    &string_buf);
			add_range_to_ast(&base->base_specifier_range,
					 base_clause);
			base_clause->text = terminate_string(&string_buf);
			relink(name_clause_link, base_clause);
			if (GNU_compatible_AST) {
			    ast_node_info_ptr base_name =
				    new_ast_node_info(NULL);
			    base_name->node_name = NN_base_name;
			    add_symid_to_string(SET_symid_and_add_to_ast(base->type, iek_type, base_name),
						&string_buf);
			    base_name->text = terminate_string(&string_buf);
			    relink(&base_clause->first_child, base_name);
			}
			name_clause_link = &base_clause->next_sibling;
			name_clause_end = base->base_specifier_range.end;
		    }
		}
	    }
	    if (name_clause) {
		name_clause->node_name = NN_name_clause;
		add_symid_to_string(SET_symid_and_add_to_ast(
					type_ptr, iek_type, name_clause), &string_buf);
		if (type_ptr->source_corresp.decl_pos_info) {
		    add_pos_to_ast(&type_ptr->source_corresp.decl_pos_info->
				   identifier_range.start,
				   &name_clause_end,
				   name_clause);
		}
		name_clause->text = terminate_string(&string_buf);
	    }
	    if ((class_scope && !is_auto_gen_class(type_ptr)) ||
		il_header.source_language == sl_C) {
		/* do no AST for incomplete type or automatically-
		 * generated template instance.
		 */
		create_ast_info_for_type_def(type_ptr, name_clause);
	    }
	}
	else if (type_ptr->kind == tk_typeref) {
	    unsigned int type_quals =
		    type_ptr->variant.typeref.qualifiers;
	    unsigned int base_quals;
	    a_boolean atr_added = FALSE;
	    a_line_number line_number;
	    a_boolean     at_end_of_source;
	    a_source_file_ptr source;

	    a_type_ptr typeref_ptr = type_ptr->variant.typeref.type;
	    a_seq_number seq =  type_ptr->source_corresp.decl_position.seq;
	    source = source_file_for_seq(seq,
                                         &line_number,
                                         &at_end_of_source,
                                         /*physical_line*/TRUE);
	    if (type_ptr->source_corresp.name) {
		a_type_ptr typedef_basetype;
		a_boolean typedef_is_fcn_type;
		ast_node_info_ptr dclrtr =
			create_ast_info_for_declarator(type_ptr, iek_type);
		check_for_fcn_basetype(type_ptr->variant.typeref.type,
				       &typedef_basetype,
				       &typedef_is_fcn_type);
		if (typedef_is_fcn_type) {
		    parallelize_links(dclrtr->first_child->next_sibling,
				      typedef_basetype->variant.routine.
				      extra_info->param_type_list,
				      a_param_type_ptr);
		}
	    }
	}
    }
	break;

    case iek_variable: {
	a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	a_type_ptr var_basetype;
	a_boolean var_is_fcn_type;
	a_boolean has_default_expr = param_var_has_default(variable_ptr);
	a_boolean is_template_instance =
		variable_ptr->is_template_static_data_member &&
		!variable_ptr->is_specialized;

	check_for_fcn_basetype(variable_ptr->type, &var_basetype,
			       &var_is_fcn_type);

	/* Create AST info if appropriate. */
	if (is_template_instance) {
	    /* Don't create AST info for template instance. */
	}
	else if (variable_ptr->is_this_parameter) {
	    /* Don't create AST info for 'this' parameter. */
	}
	else if (variable_ptr->is_parameter &&
	      !variable_ptr->source_corresp.name &&
	      !has_default_expr && !var_is_fcn_type) {
	    /* Don't create AST info for a parameter, unless
	     * it has a name, a default expression, or a function type. */
	}
	else if (variable_ptr->is_handler_param &&
	      !variable_ptr->source_corresp.name &&
	      !var_is_fcn_type) {
	    /* Don't create AST info for a handler parameter, unless
	     * it has a name or a function type. */
	}
	else {
	    /* Create AST info. */
	    ast_node_info_ptr* parm_link;
	    a_param_type_ptr ptp;
	    astp = create_ast_info_for_declarator(variable_ptr,
						  iek_variable);
	    parm_link = &astp->first_child->next_sibling;
	    if (var_is_fcn_type) {
		a_param_type_ptr ptp;
		for (ptp = var_basetype->variant.routine.extra_info->
		     param_type_list; ptp; ptp = ptp->next) {
		    ast_node_info_ptr parm_ast = ast_info_of(ptp);
		    if (parm_ast != NULL) {
			relink(parm_link, parm_ast);
			parm_link = &parm_ast->next_sibling;
		    }
		}
	    }
	    if (variable_ptr->init_kind == initk_static ||
		variable_ptr->init_kind == initk_dynamic) {
		ast_node_info_ptr init;
		ast_node_info_ptr init_val;
		if (variable_ptr->init_kind == initk_static) {
		    init_val = ast_info_of(variable_ptr->initializer.constant);
		    if (!init_val) {
			if (variable_ptr->initializer.constant->const_expr) {
			    init_val = ast_info_of(variable_ptr->
				    initializer.constant->const_expr);
			}
		    }
		    if (!init_val) {
			a_constant_ptr constant = variable_ptr->initializer.constant;
			ast_node_info_ptr const_val = new_ast_node_info(NULL);
			init_val = new_ast_node_info(constant);
			if (constant->kind == ck_string ||
			    (constant->kind == ck_address &&
			     constant->variant.address.kind == abk_constant &&
			     constant->variant.address.variant.constant->
			     kind == ck_string)) {
			    init_val->node_name = NN_constant_str;
			}
			else if (constant->kind == ck_integer &&
				 constant->source_corresp.name) {
			    init_val->node_name = NN_constant_enum;
			}
			else init_val->node_name = NN_constant;
			add_symid_to_string(SET_symid_and_add_to_ast(constant, iek_constant, init_val), 
					    &string_buf);
			add_pos_and_len_to_ast(&constant->source_corresp.
					       decl_position, init_val);
			init_val->text = terminate_string(&string_buf);
			add_1_char_to_string('\"', &string_buf);
			constant_to_string(constant, &string_buf);
			add_1_char_to_string('\"', &string_buf);
			const_val->text = terminate_string(&string_buf);
			const_val->is_leaf = TRUE;
			relink(&init_val->first_child, const_val);
		    }
		}
		else {
		    init_val = ast_info_of(variable_ptr->initializer.dynamic);
		    if (init_val && init_val->node_name == NN_dynamic_init) {
			/* NOTE: in the (rare) case where the dynamic init has
			 * not yet been processed, the following manipulations
			 * on the initial value will not be done.  Hopefully
			 * that will not cause major problems.
			 */
			ast_node_info_ptr ctor_call = init_val->first_child;
			if (GNU_compatible_AST) {
			    init_val->suppress_node = TRUE;
			}
			if (ctor_call && ctor_call->node_name ==
			    NN_constructor_call &&
			    !variable_ptr->initializer_range.start.seq) {
			    /* implicit constructor call, must flag it as
			     * compiler-generated.
			     */
			    add_3_chars_to_string(":cg", &string_buf);
			    add_to_string(ctor_call->text, &string_buf);
			    ctor_call->text = terminate_string(&string_buf);
			}
		    }
		}
		init = new_ast_node_info(NULL);
		init->node_name = NN_init;
		add_range_to_ast(&variable_ptr->initializer_range,
				 init);
		init->text = terminate_string(&string_buf);
		relink(parm_link, init);
		if (init_val) {
		    relink(&init->first_child, init_val);
		}
		else add_pending_link(variable_ptr->initializer.dynamic,
				      &init->first_child);
		if (GNU_compatible_AST) {
		    init->suppress_node = TRUE;
		}
	    }
	    else if (variable_ptr->is_parameter) {
		a_param_type_ptr param_type = variable_ptr->assoc_param_type;
		if (has_default_expr) {
		    ast_node_info_ptr dft = new_ast_node_info(NULL);
		    dft->node_name = NN_default;
		    add_range_to_ast(&param_type->default_arg_expr->
				     expr_range, dft);
		    dft->text = terminate_string(&string_buf);
		    relink(parm_link, dft);
		    relink(&dft->first_child,
			   ast_info_of(param_type->default_arg_expr));
		    if (GNU_compatible_AST) {
			dft->suppress_node = TRUE;
		    }
		}
	    }
	}
    }
	break;

#ifdef CIL 
    case iek_field: {
	a_field_ptr field_ptr = (a_field_ptr) entry_ptr;
	a_type_ptr parent = field_ptr->source_corresp.parent.class_type;
	a_boolean is_template_member = in_template_function ||
		(parent->variant.class_struct_union.is_template_class &&
		 !parent->variant.class_struct_union.is_specialized &&
		 !prototype_instance_member);

	if (!is_template_member) {
	    create_ast_info_for_declarator(field_ptr, iek_field);
	}
    }
	break;

    case iek_exception_specification: {
    }
	break;

    case iek_exception_specification_type: {
    }
	break;
#endif /* ifdef CIL */

    case iek_routine: {
	a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	if (!routine_ptr->compiler_generated ||
	    routine_ptr->assoc_scope != NULL_region_number) {
	    create_ast_info_for_declarator(routine_ptr, iek_routine);
	}
    }
	break;

    case iek_label: {
    }
	break;

    case iek_expr_node: {
	an_expr_node_ptr expr = (an_expr_node_ptr) entry_ptr;
	intr_info_ptr intrp = get_intr_info_from_il_node(entry_ptr);
	a_boolean symid_slot_taken = FALSE;
	a_boolean compiler_generated = FALSE;
	a_boolean use_only_range_start_and_len = FALSE;
	a_type_ptr type = NULL;
	ast_node_info_ptr* link;
	astp = new_ast_node_info(expr);
	astp->node_name = expr_node_name(expr);
	switch(expr->kind) {
	case enk_error:
	    break;
	case enk_operation:
	    if (!(generate_multiple_IF_files && 
		  missing_ast_for_operands(expr->variant.operation.
						 operands))) {
		parallelize_links(astp->first_child,
				  expr->variant.operation.operands,
				  an_expr_node_ptr);
	    }
	    if (GNU_compatible_AST) {
		if (expr->variant.operation.kind == eok_indirect) {
		    an_expr_node_ptr op1 = expr->variant.operation.operands;
		    if (is_implicit_pointer(op1)) {
			/* This indirection is an artifact of the conventional
			 * type of these operations and must be suppressed.
			 * The position on this node is correct, so just
			 * steal the node_name from the operand and suppress
			 * the operand node.
			 */
			ast_node_info_ptr opnd = ast_info_of(op1);
			astp->node_name = opnd->node_name;
			opnd->suppress_node = TRUE;
		    }
		}
		else if (expr_operand[expr->variant.operation.kind].first_is_address) {
		    /* This node requires an address, so a level of
		     * indirection must be applied to restore the first
		     * operand to the form it had in the source.
		     */
		    an_expr_node_ptr op1 = expr->variant.operation.operands;
		    ast_node_info_ptr opnd = ast_info_of(op1);
		    if (opnd->node_name == NN_variable_address) {
			if (op1->kind == enk_variable ||
			    op1->kind == enk_variable_address) {
			    /* We already converted this from a single
			     * variable_address node to a subtree with
			     * addr_expr and a variable node.  Now we
			     * just replace the whole thing.
			     */
			    ast_node_info_ptr var = new_ast_node_info(op1);
			    var->node_name = NN_variable;
			    add_symid_to_string(SET_symid_and_add_to_ast(op1->variant.variable.ptr, iek_variable, var),
						&string_buf);
			    add_range_to_ast(&op1->expr_range, var);
			    var->text = terminate_string(&string_buf);
			    relink(&var->next_sibling, opnd->next_sibling);
			    relink(&opnd->next_sibling, var);
			    opnd->suppress_subtree = TRUE;
			}
		    }
		    else if (opnd->node_name == NN_implicit_address) {
			/* already fixed, nothing to do */
		    }
		    else {
			/* Some expression.  Need to add an indirect node. */
			a_type_ptr op1_type = op1->type;
			ast_node_info_ptr ind = new_ast_node_info(NULL);
			if (op1_type->kind == tk_pointer) {
			    /* This should always be the case.  Get the
			     * type pointed to as the result type of the
			     * indirection.
			     */
			    op1_type = op1_type->variant.pointer.type;
			}
			add_symid_to_string(SET_symid_and_add_to_ast(op1_type, iek_type, ind),
					    &string_buf);
			add_range_to_ast(&op1->expr_range, ind);
			ind->text = terminate_string(&string_buf);
			relink(&astp->first_child, ind);
			relink(&ind->first_child, opnd);
			relink(&ind->next_sibling, opnd->next_sibling);
			opnd->next_sibling = NULL;
			if (is_implicit_pointer(op1)) {
			    /* The node was only formally a pointer in the
			     * EDG IL; nominally it was not indirect in the
			     * source, so make opnd a clone of op1 instead
			     * of an explicit indirection and suppress op1.
			     */
			    ind->node_name = opnd->node_name;
			    opnd->suppress_node = TRUE;
			}
			else ind->node_name = NN_indirect;
		    }
		    if (is_implicit_pointer(expr) &&
			expr->operator_position.seq &&
			expr->operator_position.seq ==
			expr->expr_range.start.seq &&
			expr->operator_position.mapped_column ==
			expr->expr_range.start.mapped_column) {
			/* The "implicit pointer" represents an explicit
			 * "&" operator (at expr->operator_position); we
			 * will create a new node just to represent the
			 * "&", which will become astp, and make the old
			 * astp its operand, with its own ending position
			 * but starting at the position of its first
			 * operand (to skip over the "&").
			 */
			ast_node_info_ptr new_astp = new_ast_node_info(expr);
			if (expr->type->kind == tk_pointer) {
			    /* Use the base type (before application of "&") */
			    add_symid_to_string(SET_symid_and_add_to_ast(expr->type->variant.pointer.type, iek_type, astp),
						&string_buf);
			}
			else /* shouldn't happen */
				add_symid_to_string(SET_symid_and_add_to_ast(expr->type, iek_type, astp),
						    &string_buf);
			
			if (op1 && op1->expr_range.start.seq) {
			    /* Use operand start pos */
			    add_pos_to_ast(&op1->expr_range.start, &expr->expr_range.end, astp);
			}
			else /* shouldn't happen */
				add_range_to_ast(&expr->expr_range, astp);
			astp->text = terminate_string(&string_buf);
			new_astp->node_name = NN_variable_address;
			relink(&new_astp->first_child, astp);
			astp = new_astp;
			use_only_range_start_and_len = TRUE;
			/* Must write type symid now; otherwise, this node will
			 * be treated as if it were only a pointer by virtual of
			 * being an lvalue and the type implicitly dereferenced.
			 */
			if (compiler_generated) {
			    add_3_chars_to_string(":cg", &string_buf);
			}
			add_symid_to_string(SET_symid_and_add_to_ast(expr->type, iek_type, astp), 
					    &string_buf);
			symid_slot_taken = TRUE;
		    }
		}
		else if (expr->variant.operation.kind == eok_call ||
			 expr->variant.operation.kind == eok_virtual_call ||
			 expr->variant.operation.kind == eok_pm_call) {
		    an_expr_node_ptr op1 = expr->variant.operation.operands;
		    a_type_ptr op1_type = (op1 && op1->kind == enk_routine_address

					   && op1->variant.routine.ptr->type) ?
			    skip_typerefs(op1->variant.routine.ptr->type) : NULL;
		    an_expr_node_ptr op2 = op1 ? op1->next : NULL;
		    an_expr_node_ptr addr_opnd;
		    if (op1 && op1->kind == enk_routine_address &&
			op1_type != NULL &&
			op1_type->variant.routine.extra_info &&
			op1_type->variant.routine.extra_info->
			this_class && op2 &&
			(addr_opnd = has_implicit_address_op(op2)) != NULL) {
			/* This is a call of a nonstatic member function
			 * whose object expression is an object, not a
			 * pointer (o.f(), not p->f()).  The implicit
			 * address operation must be made explicit.
			 */
			ast_node_info_ptr op1_ast = ast_info_of(op1);
			ast_node_info_ptr opnd_ast = ast_info_of(addr_opnd);
			ast_node_info_ptr addr_op = new_ast_node_info(NULL);
			addr_op->node_name = NN_variable_address;
			add_3_chars_to_string(":cg", &string_buf);
			add_symid_to_string(SET_symid_and_add_to_ast(addr_opnd->type, iek_type, addr_op),
					    &string_buf);
			addr_op->text = terminate_string(&string_buf);
			relink(&addr_op->first_child, opnd_ast);
			relink(&addr_op->next_sibling, opnd_ast->next_sibling);
			opnd_ast->next_sibling = NULL;
			if (addr_opnd == op2) {
			    relink(&op1_ast->next_sibling, addr_op);
			}
			else {
			    an_expr_node_ptr parent;
			    for(parent = op2; parent; parent = parent->
				variant.operation.operands) {
				if (parent->variant.operation.operands ==
				    addr_opnd) {
				    ast_info_of(parent)->first_child =
					    addr_op;
				    break;
				}
			    }
			}
		    }
		}
	    }
	    if (expr->variant.operation.compiler_generated ||
		expr->variant.operation.implicit_in_member_naming) {
		compiler_generated = TRUE;
		if ((expr->variant.operation.kind == eok_cast ||
		     expr->variant.operation.kind == eok_bool_cast) &&
		    types_are_compatible(expr->type,
					 expr->variant.operation.operands->type)) {
		    /* no-op: suppress */
		    astp->suppress_node = TRUE;
		}
	    }
	    if (GNU_compatible_AST && !symid_slot_taken &&
		expr_operand[expr->variant.operation.kind].first_is_address &&
		expr->variant.operation.kind != eok_extract_bit_field &&
		expr->type->kind == tk_pointer) {
		/* The type of this node is an lvalue, represented by a
		 * pointer type; we need to provide the indirected type
		 * in GNU mode.
		 */
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(expr->type->variant.pointer.type, iek_type, astp),
				    &string_buf);
		symid_slot_taken = TRUE;
	    }
	    break;
	case enk_constant:
	    /* For constants representing constant expressions, they
	     * will have a corresponding ast node, either directly or
	     * for the constant expr.  For shared constants that are
	     * not folded expressions, we have to create an ast node
	     * on the fly for each use; in order not to disturb the
	     * partially-complete creation of the main node for this
	     * expression, the operand creation is deferred to the
	     * bottom of the switch.  (If the node has a name, we
	     * just use it and ignore the associated const expr.)
	     */
	    if (expr->variant.constant->from_huge_init) {
		/* 
		 * Don't generate a reference, since we didn't generate
		 * a symbol for this constant 
		 */
		break;
	    }
	    if (expr->variant.constant->const_expr &&
		!expr->variant.constant->source_corresp.name &&
		(interesting_node(expr->variant.constant) ||
		 interesting_node(expr->variant.constant->const_expr))) {
		ast_node_info_ptr const_ast;
		if (interesting_node(expr->variant.constant)) {
		    const_ast = ast_info_of(expr->variant.constant);
		    if (const_ast && compiler_generated) {
			/* Clone node with added ":cg" */
			ast_node_info_ptr unsuppressed;
			for (unsuppressed = const_ast;
			     unsuppressed && unsuppressed->suppress_node;
			     unsuppressed = unsuppressed->first_child)
				{ }
			if (unsuppressed && !unsuppressed->suppress_subtree) {
			    const_ast = new_ast_node_info(expr);
			    *const_ast = *unsuppressed;
			    add_3_chars_to_string(":cg", &string_buf);
			    add_to_string(unsuppressed->text, &string_buf);
			    const_ast->text = terminate_string(&string_buf);
			}
		    }
		}
		else const_ast = NULL;
		if (!const_ast) {
		    const_ast =
			    new_ast_node_info(expr->variant.constant);
		    if (expr->variant.constant->kind == ck_string ||
			(expr->variant.constant->kind == ck_address &&
			 expr->variant.constant->variant.address.kind == abk_constant &&
			 expr->variant.constant->variant.address.variant.constant->
			 kind == ck_string)) {
			const_ast->node_name = NN_constant_str;
		    }
		    else if (expr->variant.constant->kind == ck_integer &&
			     expr->variant.constant->source_corresp.name) {
			const_ast->node_name = NN_constant_enum;
		    }
		    else const_ast->node_name = NN_constant;
		    const_ast->text = "";
		    relink(&const_ast->first_child,
			   ast_info_of(expr->variant.constant->const_expr));
		    const_ast->suppress_node = TRUE;
		}
		relink(&astp->first_child, const_ast);
		astp->suppress_node = TRUE;
	    }
	    else {
		a_constant_ptr cp = expr->variant.constant;
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		if (cp->kind == ck_address &&
		    cp->variant.address.kind == abk_constant &&
		    cp->variant.address.variant.constant->kind == ck_string) {
		    /* suppress the address constant from the AST and just use the
		     * string constant itself (it doesn't matter in dumping the
		     * operand below because the dump routine does the right thing).
		     */
		    add_symid_to_string(SET_symid_and_add_to_ast(cp->variant.address.variant.constant, iek_constant, astp),
					&string_buf);
		}
		else add_symid_to_string(SET_symid_and_add_to_ast(cp, iek_constant, astp), &string_buf);
		symid_slot_taken = TRUE;
	    }
	    break;
	case enk_variable:
	    {
		a_variable_ptr variable_ptr = expr->variant.variable.ptr;
		unsigned long orig_id = il_entry_prefix_of(variable_ptr).SET_symid;
                a_boolean ok = FALSE;
		if (orig_id == 0) {
		    if (variable_ptr->is_this_parameter && !fcn_declarator) {
			/* A symid has not been assigned, because we have the
			 * 'this' parameter of a trivial contructor, and no SYM
			 * line is written.
			 */
		    }
		    else {
                        if (generate_multiple_IF_files) {
                            /* If generate_multiple_IF_files is set it is possible
                             * that SYM line has been written yet so do not complain.
                             */
                            ok = TRUE;
			}
			else {
                            worry("Symid expected to have been pre-assigned.");
			}
		    }
		}
                else ok = TRUE;
		if (ok) {
		    if (compiler_generated) {
			add_3_chars_to_string(":cg", &string_buf);
		    }
		    add_symid_to_string(SET_symid_and_add_to_ast(variable_ptr, iek_variable, astp), &string_buf);
		    symid_slot_taken = TRUE;
		}
	    }
	    break;
	case enk_variable_address:
	    if (GNU_compatible_AST) {
		if (expr->variant.variable.pos.seq) {
		    /* This is an explicit address expression ("&var").  Make a
		     * new node for the variable, beginning at the variable pos.
		     */
		    ast_node_info_ptr var = new_ast_node_info(NULL);
		    var->node_name = NN_variable;
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.variable.ptr, iek_variable, var),
					&string_buf);
		    if (expr->expr_range.start.seq) {
			add_pos_to_ast(&expr->variant.variable.pos, &expr->expr_range.end, var);
		    }
		    var->text = terminate_string(&string_buf);
		    relink(&astp->first_child, var);
		}
		else {
		    /* This is an implicit address expression (e.g., an array
		     * converted to to a pointer).  The source range is already
		     * correct, so just change the name to "variable" and use
		     * the symid slot to reference the variable.  (We use the
		     * node name NN_implicit_address, which is output as "variable,"
		     * so we can tell the difference between this case and a
		     * pointer variable when doing an assignment or other
		     * operation that takes a pointer in the EDG IL; if it's
		     * a pointer variable, we must add a level of indirection, but
		     * if it's just the result of an implicit address, nothing
		     * needs to be done.
		     */
		    astp->node_name = NN_implicit_address;
		    if (compiler_generated) {
			add_3_chars_to_string(":cg", &string_buf);
		    }
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.variable.ptr, iek_variable, astp),
					&string_buf);
		    symid_slot_taken = TRUE;
		}
	    }
	    else {
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.variable.ptr, iek_variable, astp),
				    &string_buf);
		symid_slot_taken = TRUE;
	    }
	    break;
#ifdef CIL
	case enk_field:
	    if (GNU_compatible_AST) {
		if (expr->variant.field.pos.seq) {
		    /* This is an explicit address expression ("&X::f").  Make
		     * a new node for the field, beginning at the indicated
		     * position.
		     */
		    ast_node_info_ptr field = new_ast_node_info(NULL);
		    field->node_name = NN_field;
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.field.ptr, iek_field, field),
					&string_buf);
		    if (expr->expr_range.start.seq) {
			add_pos_to_ast(&expr->variant.field.pos,
				       &expr->expr_range.end,
				       field);
		    }
		    field->text = terminate_string(&string_buf);
		    relink(&astp->first_child, field);
		    astp->node_name = NN_variable_address;
		}
		else {
		    if (compiler_generated) {
			add_3_chars_to_string(":cg", &string_buf);
		    }
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.field.ptr, iek_field, astp),
					 &string_buf);
		    symid_slot_taken = TRUE;
		}
	    }
	    else {
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.field.ptr, iek_field, astp),
				    &string_buf);
		symid_slot_taken = TRUE;
	    }
	    break;
	case enk_temp_init:
	    relink(&astp->first_child, (ast_node_info_ptr)
		   ast_info_of(expr->variant.init.dynamic_init));
	    if (GNU_compatible_AST) {
		astp->suppress_node = TRUE;
	    }
	    break;
	case enk_new_delete: {
	    a_new_delete_supplement_ptr new_del = (a_new_delete_supplement_ptr)
		    expr->variant.new_delete;
	    ast_node_info_ptr op_call = new_ast_node_info(NULL);
	    ast_node_info_ptr func;
	    op_call->node_name = NN_call;
	    if (new_del->routine) {
		add_symid_to_string(SET_symid_and_add_to_ast(new_del->routine->type->variant.routine.return_type, iek_type, op_call), 
				    &string_buf);
		op_call->text = terminate_string(&string_buf);
		func = new_ast_node_info(NULL);
		func->node_name = NN_routine_address;
		add_symid_to_string(SET_symid_and_add_to_ast(new_del->routine, iek_routine, func), 
				    &string_buf);
		func->text = terminate_string(&string_buf);
	    }
	    else {
		op_call->text = "";
		func = make_null_ast_operand();
	    }
	    relink(&op_call->first_child, func);
	    parallelize_links(func->next_sibling, new_del->arg,
			      an_expr_node_ptr);
	    relink(&astp->first_child, op_call);
	    if (new_del->dynamic_init) {
		relink(&op_call->next_sibling,
		       ast_info_of(new_del->dynamic_init));
	    }
	    else relink(&op_call->next_sibling, make_null_ast_operand());
	    if (new_del->freeing_of_storage_on_exception) {
		ast_node_info_ptr op_del =
			ast_info_of(new_del->freeing_of_storage_on_exception);
		relink(&op_call->next_sibling->next_sibling, op_del);
		if (GNU_compatible_AST && op_del) {
		    op_del->suppress_subtree = TRUE;
		}
	    }
	}
	    break;
	case enk_throw:
	    if (expr->variant.throw_info) {
		relink(&astp->first_child, ast_info_of(expr->
		       variant.throw_info->dynamic_init));
	    }
	    break;
	case enk_condition: {
	    a_condition_supplement_ptr cond = (a_condition_supplement_ptr)
		    expr->variant.condition;
	    if (cond->dynamic_init) {
		if (cond->dynamic_init->variable) {
		    a_source_sequence_entry_ptr sseq =
			    cond->dynamic_init->variable->
			    source_corresp.source_sequence_entry;
		    ast_node_info_ptr dcl;
		    scan_declarator_chain(sseq);
		    dcl = make_declaration_tree(/*do_chain=*/FALSE, intrp);
		    relink(&astp->first_child, dcl);
		    /* flag as already processed: */
		    il_entry_prefix_of(sseq).SET_info = dcl;
		}
		else relink(&astp->first_child, ast_info_of(cond->dynamic_init));
	    }
	    else relink(&astp->first_child, make_null_ast_operand());
	    relink(&astp->first_child->next_sibling,
		   ast_info_of(cond->expr));
	}
	    break;
	case enk_object_lifetime: {
	    a_dynamic_init_ptr init;
	    relink(&astp->first_child, (ast_node_info_ptr)
		   ast_info_of(expr->variant.object_lifetime.expr));
	    link = &astp->first_child->next_sibling;
	    for (init = expr->variant.object_lifetime.ptr->destructions;
		 init; init = init->next_in_destruction_list) {
		if (!init->is_freeing_of_storage_on_exception) {
		    /* a "freeing of storage on exception" init cannot
		     * be processed by create_destructor_call_ast
		     * because the "destructor" is actually an
		     * operator delete(); there's no variable to
		     * pass as the first operand, so we just ignore
		     * those here.
		     */
		    ast_node_info_ptr dtor_call =
			    create_destructor_call_ast(init, 
						       /*do_chain=*/FALSE,
						       intrp);
		    if (dtor_call) {
			relink(link, dtor_call);
			link = &dtor_call->next_sibling;
		    }
		}
	    }
	    if (GNU_compatible_AST) {
		astp->suppress_node = TRUE;
	    }
	}
	    break;
	case enk_asm: {
	    an_asm_entry_ptr asm_entry;
	    link = &astp->first_child;
	    for (asm_entry = expr->variant.asm_entry; asm_entry;
		 asm_entry = asm_entry->next_asm_string) {
		ast_node_info_ptr entry_ast = ast_info_of(asm_entry);
		if (entry_ast) {
		    relink(link, entry_ast);
		    link = &entry_ast->next_sibling;
		}
	    }
	}
	    break;
	case enk_typeid:
	    if (expr->variant.typeid_info.expr) {
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(expr->variant.typeid_info.expr));
	    }
	    else type = expr->variant.typeid_info.type;
	    break;
	case enk_runtime_sizeof:
	    if (!expr->variant.runtime_sizeof.is_type) {
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(expr->variant.runtime_sizeof.variant.expr));
	    }
	    else type = expr->variant.runtime_sizeof.variant.type;
	    break;
	case enk_address_of_ellipsis:
	    break;
#endif /* ifdef CIL */
#ifdef FIL
	case enk_stmt_label_value:
	    break;
	case enk_char_variable_length:
	    break;
	case enk_data_implied_do_var:
	    break;
#endif /* ifdef FIL */
	case enk_routine_address:
	    symid_slot_taken = TRUE;
	    if (!expr->variant.routine.ptr) {
		/* NULL pointer to function: we have to manufacture a SYM */
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(expr, iek_expr_node, astp), &string_buf);
	    }
	    else if (GNU_compatible_AST) {
		if (expr->variant.routine.pos.seq) {
		    /* This is an explicit address expression ("&fcn").  Make a
		     * new node for the function, beginning at the indicated
		     * position.
		     */
		    ast_node_info_ptr fcn = new_ast_node_info(NULL);
		    fcn->node_name = NN_routine_address;
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.routine.ptr, iek_routine, fcn),
					&string_buf);
		    if (expr->expr_range.start.seq) {
			add_pos_to_ast(&expr->variant.routine.pos, &expr->expr_range.end, fcn);
		    }
		    fcn->text = terminate_string(&string_buf);
		    relink(&astp->first_child, fcn);
		    astp->node_name = NN_variable_address;
		    symid_slot_taken = FALSE;
		}
		else {
		    if (compiler_generated) {
			add_3_chars_to_string(":cg", &string_buf);
		    }
		    add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.routine.ptr, iek_routine, astp),
					 &string_buf);
		}
	    }
	    else {
		if (compiler_generated) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(expr->variant.routine.ptr, iek_routine, astp),
				     &string_buf);
	    }
	    break;
	default:
	    /* do nothing: error already reported by expr_node_name */
	    break;
	}
	if (!symid_slot_taken) {
	    if (compiler_generated) {
		add_3_chars_to_string(":cg", &string_buf);
	    }
	    add_symid_to_string(SET_symid_and_add_to_ast(expr->type, iek_type, astp), &string_buf);
	}
	if (expr->expr_range.start.seq) {
	    if (use_only_range_start_and_len) {
		add_pos_and_len_to_ast(&expr->expr_range.start,
				       astp);
	    }
	    else {
	    	add_range_to_ast(&expr->expr_range, astp);
	    }
	}
	astp->text = terminate_string(&string_buf);
	if (expr->kind == enk_constant && !astp->first_child) {
	    /* constant may have been shared; if so, we clone an ast
	     * for it here.
	     */
	    ast_node_info_ptr ast_op = new_ast_node_info(NULL);
	    add_1_char_to_string('\"', &string_buf);
	    constant_to_string(expr->variant.constant, &string_buf);
	    add_1_char_to_string('\"', &string_buf);
	    ast_op->text = terminate_string(&string_buf);
	    ast_op->is_leaf = TRUE;
	    relink(&astp->first_child, ast_op);
	}
	else if (type) {
	    /* must clone operands for type-based typeid and
	     * runtime_sizeof expressions, since types are shared and
	     * ast_node_info objects cannot be created for them during
	     * the IL walk.
	     */
	     ast_node_info_ptr ast_op = new_ast_node_info(NULL);
	     ast_op->node_name = NN_type;
	     add_symid_to_string(SET_symid_and_add_to_ast(type, iek_type, ast_op), &string_buf);
	     ast_op->text = terminate_string(&string_buf);
	     relink(&astp->first_child, ast_op);
	}
    }
	break;

#ifdef CIL
    case iek_for_loop: {
    }
	break;

    case iek_switch_clause: {
	a_switch_clause_ptr switch_clause = (a_switch_clause_ptr) entry_ptr;
	ast_node_info_ptr values = new_ast_node_info(NULL);
	values->node_name = NN_case_values;
	if (!switch_clause->constant_list) {
	    /* a default label */
	    ast_node_info_ptr dflt = new_ast_node_info(NULL);
	    dflt->node_name = NN_case_default;
	    add_pos_to_ast(&switch_clause->default_position,
			   &switch_clause->default_end_position, dflt);
	    dflt->text = terminate_string(&string_buf);

	    add_pos_to_ast(&switch_clause->default_position,
			   &switch_clause->default_end_position, values);
	    values->text = terminate_string(&string_buf);
	    relink(&values->first_child, dflt);
	}
	else {
	    values->text = "";
	    parallelize_links(values->first_child, switch_clause->constant_list,
			      a_constant_ptr);
	}
	astp = new_ast_node_info(switch_clause);
	astp->node_name = NN_switch_clause;
	astp->text = "";
	relink(&astp->first_child, values);
    }
	break;

    case iek_handler: {
	a_handler_ptr handler = (a_handler_ptr) entry_ptr;
	astp = new_ast_node_info(handler);
	astp->node_name = NN_catch;
	if (handler->parameter) {
	    add_symid_to_string(SET_symid_and_add_to_ast(handler->parameter, iek_variable, astp),
				&string_buf);
	}
	add_pos_to_ast(&handler->catch_position,
		       &handler->statement->variant.block.extra_info->final_position,
		       astp);
	astp->text = terminate_string(&string_buf);
	if (handler->dynamic_init) {
	    relink(&astp->first_child,
		   ast_info_of(handler->dynamic_init));
	}
	else relink(&astp->first_child,
		    make_null_ast_operand());
    }
	break;

    case iek_try_supplement: {
    }
	break;

#if MICROSOFT_EXTENSIONS_ALLOWED
    case iek_microsoft_try_supplement: {
    }
	break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#endif /* ifdef CIL */

    case iek_block: {
    }
	break;

    case iek_statement: {
	a_statement_ptr statement_ptr = (a_statement_ptr) entry_ptr;
	if (statement_ptr->kind != stmk_init &&
	    statement_ptr->kind != stmk_decl &&
	    !((statement_ptr->kind == stmk_goto ||
	       statement_ptr->kind == stmk_label) &&
	      !statement_ptr->source_sequence_entry) &&
	    !(statement_ptr->kind == stmk_block &&
	      block_is_generated(statement_ptr))) {
	    astp = new_ast_node_info(statement_ptr);
	    switch (statement_ptr->kind) {
	    case stmk_expr:
		astp->node_name = NN_expression_statement;
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(statement_ptr->expr));
		break;
	    case stmk_if:
		astp->node_name = NN_if;
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(statement_ptr->expr));
		break;
	    case stmk_while:
		astp->node_name = NN_while;
		relink(&astp->first_child, (ast_node_info_ptr) 
		       ast_info_of(statement_ptr->expr));
		break;
	    case stmk_goto: {
		a_label_ptr targ = statement_ptr->variant.label.ptr;
		if (targ->break_label) {
		    astp->node_name = NN_break;
		}
		else if (targ->continue_label) {
		    astp->node_name = NN_continue;
		}
		else {
		    astp->node_name = NN_goto;
		    add_symid_to_string(SET_symid_and_add_to_ast(targ, iek_label, astp), &string_buf);
		}
	    }
		break;
	    case stmk_label:
		astp->node_name = NN_label;
		add_symid_to_string(SET_symid_and_add_to_ast(statement_ptr->variant.label.ptr, iek_label, astp),
				    &string_buf);
		break;
	    case stmk_return:
		astp->node_name = NN_return;
		if (!statement_ptr->source_sequence_entry) {
		    add_3_chars_to_string(":cg", &string_buf);
		}
		if (statement_ptr->expr && curr_fcn &&
		    curr_fcn->special_kind != sfk_constructor) {
		    if (statement_ptr->variant.return_stmt.implicit_indirection) {
			relink(&astp->first_child,
			       ast_info_of(statement_ptr->variant.return_stmt.
					   implicit_indirection));
		    }
		    else relink(&astp->first_child, 
				ast_info_of(statement_ptr->expr));
		}
		else if (statement_ptr->variant.return_stmt.dynamic_init) {
		    relink(&astp->first_child, (ast_node_info_ptr)
			   ast_info_of(statement_ptr->variant.return_stmt.
				       dynamic_init));
		}
		if (!statement_ptr->source_sequence_entry) {
		    astp->suppress_subtree = TRUE;
		}
		break;
	    case stmk_block:
		astp->node_name = NN_block;
		break;
#ifdef CIL
	    case stmk_end_test_while:
		astp->node_name = NN_do_while;
		relink(&astp->first_child, (ast_node_info_ptr) 
		       ast_info_of(statement_ptr->expr));
		break;
	    case stmk_for: {
		astp->node_name = NN_for;
	    }
		break;
	    case stmk_switch: {
		a_statement_ptr body = statement_ptr->variant.switch_stmt.body_statement;
		ast_node_info_ptr body_ast = ast_info_of(body);
		astp->node_name = NN_switch;
		relink(&astp->first_child, (ast_node_info_ptr) 
		       ast_info_of(statement_ptr->expr));
	    }
		break;
	    case stmk_init:
		break;
	    case stmk_asm:
		astp->node_name = NN_asm;
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(statement_ptr->variant.asm_entry));
		break;
#if ASM_FUNCTION_ALLOWED
	    case stmk_asm_func_body:
		astp->node_name = NN_asm_function;
		if (!GNU_compatible_AST) {
		    relink(&astp->first_child,
			   make_string_ast_operand(statement_ptr->
						   variant.asm_func_body));
		}
		break;
#endif /* ASM_FUNCTION_ALLOWED */
	    case stmk_try_block: {
		astp->node_name = NN_try_block;
	    }
		break;
#if MICROSOFT_EXTENSIONS_ALLOWED
	    case stmk_microsoft_try:
		astp->node_name = NN_microsoft_try;
		break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if GENERATE_SOURCE_SEQUENCE_LISTS
	    case stmk_decl:
		break;
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
	    case stmk_set_vla_size:
		astp->node_name = NN_set_vla_size;
		add_symid_to_string(SET_symid_and_add_to_ast(statement_ptr->variant.vla_dimension->type, iek_type, astp), 
				    &string_buf);
		relink(&astp->first_child, (ast_node_info_ptr)
		       ast_info_of(statement_ptr->variant.vla_dimension->dimension_expr));
		break;
	    case stmk_vla_decl:
		astp->node_name = NN_vla_decl;
		if (statement_ptr->variant.vla.is_typedef_decl) {
		    add_symid_to_string(SET_symid_and_add_to_ast(statement_ptr->variant.vla.variant.typedef_type, iek_type, astp), 
					&string_buf);
		}
		else {
		    add_symid_to_string(SET_symid_and_add_to_ast(statement_ptr->variant.vla.variant.variable, iek_variable, astp), 
					&string_buf);
		}
		break;
#endif /* ifdef CIL */
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
	    case stmk_empty:
		astp->node_name = NN_empty;
		break;
#endif
#ifdef FIL
	    case stmk_fentry:
	    case stmk_ido:
	    case stmk_fdo:
	    case stmk_iarith_if:
	    case stmk_farith_if:
	    case stmk_computed_goto:
	    case stmk_assigned_goto:
	    case stmk_alt_return:
	    case stmk_stop:
	    case stmk_pause:
	    case stmk_set_array_shape:
	    case stmk_input_output:
#endif /* ifdef FIL */
	    default:
		complain_int(error_csev, "Unexpected statement kind $1",
			statement_ptr->kind);
		astp->node_name = NN_statement;
		break;
	    }
	    {
		a_source_position_ptr end = NULL;
		if (statement_ptr->kind == stmk_block) {
		    end = &statement_ptr->variant.block.extra_info->final_position;
		}
		else if (statement_ptr->kind == stmk_try_block) {
		    a_handler_ptr hp = statement_ptr->variant.try_block->
			    handlers;
		    while (hp && hp->next) {
			hp = hp->next;
		    }
		    if (hp) {
			a_statement_ptr hdlr_stmt = hp->statement;
			if (hdlr_stmt) {
			    if (hdlr_stmt->kind == stmk_block) {
				end = &hdlr_stmt->variant.block.extra_info->final_position;
			    }
			    else if (hdlr_stmt->end_position.seq) {
				end = &hdlr_stmt->end_position;
			    }
			}
		    }
		}
		else if (statement_ptr->kind == stmk_microsoft_try) {
		    a_microsoft_try_supplement_ptr try_info =
			    statement_ptr->variant.microsoft_try;
		    a_statement_ptr cleanup = try_info->cleanup_statement;
		    if (cleanup) {
			ast_node_info_ptr except_or_finally =
				new_ast_node_info(NULL);
			ast_node_info_ptr cleanup_ast = ast_info_of(cleanup);
			if (cleanup->kind == stmk_block) {
			    end = &cleanup->variant.block.extra_info->final_position;
			}
			else if (cleanup->end_position.seq) {
			    end = &cleanup->end_position;
			}
			if (try_info->except_expr) {
			    except_or_finally->node_name = NN_except;
			    relink(&except_or_finally->first_child,
				   ast_info_of(try_info->except_expr));
			}
			else {
			    except_or_finally->node_name = NN_finally;
			    relink(&except_or_finally->first_child,
				   make_null_ast_operand());
			    if (!GNU_compatible_AST) {
				except_or_finally->first_child->suppress_node = TRUE;
			    }
			}
			except_or_finally->text = "";
			relink(&except_or_finally->first_child->next_sibling,
			       cleanup_ast);
			il_entry_prefix_of(try_info->cleanup_statement).SET_info =
				except_or_finally;
		    }
		}
		else if (statement_ptr->end_position.seq) {
		    end = &statement_ptr->end_position;
		}
	        add_pos_to_ast(&statement_ptr->position, end, astp);
	    }
	    astp->text = terminate_string(&string_buf);
	}
    }
	break;

    case iek_object_lifetime: {
    }
	break;

    case iek_scope: {
	a_scope_ptr ptr = (a_scope_ptr) entry_ptr;
	if (ptr->kind == sck_function) {
	    a_routine_ptr routine_ptr = ptr->variant.routine.ptr;
	    if (routine_ptr && ast_info_of(routine_ptr)) {
		a_statement_ptr assoc_block = ptr->assoc_block;
		ast_node_info_ptr block_ast = assoc_block ?
			ast_info_of(assoc_block) : NULL;
		ast_node_info_ptr fcn_dclrtr = ast_info_of(routine_ptr);
		ast_node_info_ptr* ctor_init_link =
			&fcn_dclrtr->next_sibling;
		a_constructor_init_ptr ctor_init;
		if (!ptr->source_sequence_list && fcn_declarator != NULL
					       && !is_auto_gen_routine(routine_ptr)) {
		    /* This is a compiler-generated function,
		     * so set up its AST here instead of during the 
		     * processing of its source sequence entry.
		     */
		    ast_node_info_ptr def = new_ast_node_info(NULL);
		    ast_node_info_ptr specs;
		    ast_node_info_ptr* parm_link =
			    &fcn_declarator->first_child->next_sibling;
		    a_variable_ptr parm;
		    def->node_name = NN_function_def;
		    add_symid_to_string(SET_symid_and_add_to_ast(routine_ptr, iek_routine, def),
					&string_buf);
		    def->text = terminate_string(&string_buf);
		    if (routine_ptr->special_kind == sfk_constructor ||
			routine_ptr->special_kind == sfk_destructor) {
			specs = make_null_ast_operand();
		    }
		    else {
			specs = new_ast_node_info(NULL);
			specs->text = "";
			specs->node_name = NN_decl_specs;
		    }
		    relink(&specs->next_sibling, fcn_dclrtr);
		    relink(&def->first_child, specs);
		    while (*parm_link) {
			/* skip over "this" parameter, if present */
			parm_link = &(*parm_link)->next_sibling;
		    }
		    for (parm = ptr->variant.routine.parameters; parm;
			 parm = parm->next) {
			ast_node_info_ptr parm_decl =
				new_ast_node_info(NULL);
			ast_node_info_ptr specs = new_ast_node_info(NULL);
			parm_decl->node_name = NN_parm_decl;
			add_symid_to_string(SET_symid_and_add_to_ast(parm, iek_variable, parm_decl), &string_buf);
			parm_decl->text = terminate_string(&string_buf);
			specs->node_name = NN_decl_specs;
			specs->text = "";
			relink(&parm_decl->first_child, specs);
			if (ast_info_of(parm)) {
			    relink(&specs->next_sibling, ast_info_of(parm));
			}
			else {
			    ast_node_info_ptr var = new_ast_node_info(NULL);
			    var->node_name = NN_variable;
			    add_symid_to_string(SET_symid_and_add_to_ast(parm, iek_variable, var), &string_buf);
			    var->text = terminate_string(&string_buf);
			    relink(&specs->next_sibling, var);
			}
			relink(parm_link, parm_decl);
			parm_link = &parm_decl->next_sibling;
		    }
		    for (ctor_init = ptr->variant.routine.constructor_inits;
			 ctor_init; ctor_init = ctor_init->next) {
			relink(ctor_init_link, ast_info_of(ctor_init));
			ctor_init_link = &ast_info_of(ctor_init)->next_sibling;
		    }
		    if (ptr->assoc_block) {
			a_statement_ptr stmts = (ptr->assoc_block->kind == stmk_block) ?
				ptr->assoc_block->variant.block.statements : NULL;
			if (!block_ast) {
			    block_ast = new_ast_node_info(ptr->assoc_block);
			    block_ast->node_name = NN_block;
			    block_ast->text = "";
			}
			relink(ctor_init_link, block_ast);
			if (stmts && ast_info_of(stmts)) {
			    /* might be just the "return" resulting from
			     * falling off the bottom of the function; in
			     * this case, the AST for the statement was
			     * suppresses, so we only attach the statements
			     * if they have an AST.
			     */
			    parallelize_links(block_ast->first_child, stmts,
					  a_statement_ptr);
			}
		    }
		    if (routine_ptr->source_corresp.is_class_member) {
			a_type_ptr tp = routine_ptr->source_corresp.parent.
				class_type;
			ast_node_info_ptr class_ast = interesting_node(tp) ?
				ast_info_of(tp) : NULL;
			if (class_ast) {
			    relink(&def->next_sibling, class_ast->first_child->
				    next_sibling);
			    relink(&class_ast->first_child->next_sibling,
				   def);
			}
		    }
		}
		else {
		    /* just link in constructor inits */
		    a_type_ptr basetype;
		    a_boolean is_fcn;
		    check_for_fcn_basetype(routine_ptr->type, &basetype, &is_fcn);
		    if (is_fcn && 
			!basetype->variant.routine.extra_info->prototyped) {
			/* There may be K&R parameters already linked as
			 * siblings; we have to skip over those before we
			 * link in the block_ast.
			 */
			ast_node_info_ptr node;
			for (node = fcn_dclrtr; node->next_sibling; node =
			     node->next_sibling) {
			}
			ctor_init_link = &node->next_sibling;
		    }
		    else for (ctor_init = ptr->variant.routine.constructor_inits;
			      ctor_init; ctor_init = ctor_init->next) {
			relink(ctor_init_link, ast_info_of(ctor_init));
			ctor_init_link = &ast_info_of(ctor_init)->next_sibling;
		    }
		    relink(ctor_init_link, block_ast);
		}
	    }
	}
    }
	break;

    case iek_id_name: {
    }
	break;

    case iek_string_text: {
    }
	break;

    case iek_other_text: {
    }
	break;

#ifdef FIL
    case iek_internal_complex_value: {
    }
	break;

    case iek_bound_info_entry: {
    }
	break;

    case iek_do_loop: {
    }
	break;

    case iek_label_list_entry: {
    }
	break;

    case iek_io_specifier: {
    }
	break;

    case iek_io_list_item: {
    }
	break;

    case iek_namelist_group_member: {
    }
	break;

    case iek_namelist_group: {
    }
	break;

    case iek_input_output_description: {
    }
	break;

    case iek_entry_param: {
    }
	break;

    case iek_entry_description: {
    }
	break;

#endif /* ifdef FIL */
#ifdef CIL
    case iek_namespace: {
    }
	break;

    case iek_using_decl: {
	a_using_decl_ptr using_decl = (a_using_decl_ptr) entry_ptr;
	a_boolean is_auto_gen_decl = (using_decl->is_class_member &&
	      is_auto_gen_class(using_decl->qualifier.class_type));

	if (generate_multiple_IF_files && is_auto_gen_decl) {
	  break;
	}
	if (!is_auto_gen_decl) {
	    astp = new_ast_node_info(using_decl);
	    if (GNU_compatible_AST) {
		if (using_decl->entity.kind == iek_type) {
		    a_type_ptr tp = (a_type_ptr) using_decl->entity.ptr;
		    if (tp->kind == tk_integer &&
			tp->variant.integer.enum_type) {
			astp->node_name = NN_decl_id_enum;
		    }
		    else if (tp->kind == tk_class) {
			astp->node_name = NN_decl_id_cls;
		    }
		    else if (tp->kind == tk_struct) {
			astp->node_name = NN_decl_id_strct;
		    }
		    else if (tp->kind == tk_union) {
			astp->node_name = NN_decl_id_un;
		    }
		    else if (tp->kind == tk_typeref &&
			     tp->source_corresp.name) {
			astp->node_name = NN_decl_id_typd;
		    }
		    else astp->node_name = NN_decl_id_typ;
		}
		else if (using_decl->entity.kind == iek_variable) {
		    astp->node_name = NN_decl_id_var;
		}
		else if (using_decl->entity.kind == iek_field) {
		    astp->node_name = NN_decl_id_fld;
		}
		else if (using_decl->entity.kind == iek_routine) {
		    astp->node_name = NN_decl_id_fcn;
		}
		else if (using_decl->entity.kind == iek_namespace) {
		    astp->node_name = NN_decl_id_ns;
		}
		else if (using_decl->entity.kind == iek_template) {
		    astp->node_name = NN_decl_id_tpl;
		}
		else if (using_decl->entity.kind == iek_constant &&
			 ((a_constant_ptr) using_decl->entity.ptr)->
			 kind == ck_integer) {
		    astp->node_name = NN_constant_enum;
		}
		else astp->node_name = NN_unknown_node;
	    }
	    else astp->node_name = NN_used_entity;
	    add_symid_to_string(SET_symid_and_add_to_ast(using_decl->entity.ptr, using_decl->entity.kind, astp),
				&string_buf);
	    astp->text = terminate_string(&string_buf);
	}
    }
	break;

    case iek_dynamic_init: {
	a_dynamic_init_ptr init = (a_dynamic_init_ptr) entry_ptr;
	ast_node_info_ptr opnd;
	astp = new_ast_node_info(init);
	astp->node_name = NN_dynamic_init;
	if (!init->variable && init->kind != dik_none && init->destructor &&
	    init->lifetime && !init->is_constructor_init) {
	    add_symid_to_string(SET_symid_and_add_to_ast(init, iek_dynamic_init, astp), &string_buf);
	    astp->text = terminate_string(&string_buf);
	}
	else astp->text = "";
	if (init->kind == dik_constant ||
	    init->kind == dik_nonconstant_aggregate) {
	    a_constant_ptr cp = init->variant.constant;
	    if (cp->kind == ck_address &&
		cp->variant.address.kind == abk_constant &&
		cp->variant.address.variant.constant->kind == ck_string) {
		/* Use string constant rather than its associated address
		 * constant.
		 */
		opnd = ast_info_of(cp->variant.address.variant.constant);
		if (!opnd) {
		    /* Have to create it on the fly */
		    ast_node_info_ptr ast_op = new_ast_node_info(NULL);
		    add_1_char_to_string('\"', &string_buf);
		    constant_to_string(cp, &string_buf);
		    add_1_char_to_string('\"', &string_buf);
		    ast_op->text = terminate_string(&string_buf);
		    ast_op->is_leaf = TRUE;
		    opnd = new_ast_node_info(NULL);
		    add_symid_to_string(SET_symid_and_add_to_ast(cp->variant.address.variant.constant, iek_constant, opnd),
					&string_buf);
		    if (cp->const_expr) {
			add_range_to_ast(&cp->const_expr->expr_range, opnd);
		    }
		    opnd->text = terminate_string(&string_buf);
		    opnd->node_name = NN_constant_str;
		    relink(&opnd->first_child, ast_op);
		}
	    }
	    else opnd = ast_info_of(cp);
	    if (init->kind == dik_nonconstant_aggregate && init->destructor) {
		/* This is a call to the destructor in an array delete;
		 * generate some semblance of this by turning it into a
		 * call and passing the aggregate constant as a parameter
		 * (normal destructor calls have no parameters).
		 */
		ast_node_info_ptr call = new_ast_node_info(NULL);
		ast_node_info_ptr fcn = new_ast_node_info(NULL);
		a_type_ptr dtor_return_type =
			init->destructor->type->variant.routine.return_type;
		call->node_name = NN_call;
		add_symid_to_string(SET_symid_and_add_to_ast(dtor_return_type, iek_type, call), &string_buf);
		call->text = terminate_string(&string_buf);
		fcn->node_name = NN_routine_address;
		add_symid_to_string(SET_symid_and_add_to_ast(init->destructor, iek_routine, fcn), &string_buf);
		fcn->text = terminate_string(&string_buf);
		relink(&fcn->next_sibling, opnd);
		relink(&call->first_child, fcn);
		opnd = call;
	    }
	}
	else if (init->kind == dik_expression ||
		 init->kind == dik_call_returning_class_via_cctor) {
	    if (init->variant.expression.implicit_indirection) {
		opnd = ast_info_of(init->variant.expression.implicit_indirection);
	    }
	    else opnd = ast_info_of(init->variant.expression.ptr);
	}
	else if (init->kind == dik_constructor) {
	    ast_node_info_ptr ctor_opnd = new_ast_node_info(NULL);
	    a_routine_ptr ctor = (a_routine_ptr)
		    init->variant.constructor.ptr;
	    a_type_ptr ctor_return_type =
		    ctor->type->variant.routine.return_type;
	    opnd = new_ast_node_info(NULL);
	    opnd->node_name = NN_constructor_call;
	    add_symid_to_string(SET_symid_and_add_to_ast(ctor_return_type, iek_type, opnd),
				&string_buf);
	    opnd->text = terminate_string(&string_buf);
	    ctor_opnd->node_name = NN_routine_address;
	    add_symid_to_string(SET_symid_and_add_to_ast(ctor, iek_routine, ctor_opnd), &string_buf);
	    ctor_opnd->text = terminate_string(&string_buf);
	    relink(&opnd->first_child, ctor_opnd);
	    parallelize_links(ctor_opnd->next_sibling,
			      init->variant.constructor.args,
			      an_expr_node_ptr);
	}
	else if (init->kind == dik_none && init->destructor) {
	    ast_node_info_ptr dtor_opnd = new_ast_node_info(NULL);
	    a_type_ptr dtor_return_type =
		    init->destructor->type->variant.routine.return_type;
	    opnd = new_ast_node_info(NULL);
	    opnd->node_name = NN_call;
	    add_symid_to_string(SET_symid_and_add_to_ast(dtor_return_type, iek_type, opnd),
				&string_buf);
	    opnd->text = terminate_string(&string_buf);
	    dtor_opnd->node_name = NN_routine_address;
	    add_symid_to_string(SET_symid_and_add_to_ast(init->destructor, iek_routine, dtor_opnd), &string_buf);
	    dtor_opnd->text = terminate_string(&string_buf);
	    relink(&opnd->first_child, dtor_opnd);
	}
	else opnd = NULL;
	if (opnd) {
	    relink(&astp->first_child, opnd);
	}
	else relink(&astp->first_child, make_null_ast_operand());
	if (fcn_id && init->destructor) {
	    a_boolean is_base_dtor = FALSE;
	    if (init->is_constructor_init && curr_fcn &&
		curr_fcn->special_kind == sfk_constructor) {
		a_type_ptr derived_class =
			curr_fcn->source_corresp.parent.class_type;
		a_type_ptr base_class =
			init->destructor->source_corresp.parent.class_type;
		if (derived_class && base_class &&
		    find_direct_base_class_of(derived_class, base_class)) {
		    is_base_dtor = TRUE;
		}
	    }
	}
    }
	break; 

    case iek_local_static_variable_init: {
	a_local_static_variable_init_ptr lsv_init =
		(a_local_static_variable_init_ptr) entry_ptr;
	if (lsv_init->variable && interesting_node(lsv_init->variable) &&
	    ast_info_of(lsv_init->variable)) {
	    ast_node_info_ptr init_val;
	    if (lsv_init->init_kind == initk_static) {
		init_val = ast_info_of(lsv_init->initializer.constant);
		if (!init_val) {
		    if (lsv_init->initializer.constant->const_expr) {
			init_val = ast_info_of(lsv_init->
				initializer.constant->const_expr);
		    }
		}
		if (!init_val) {
		    a_constant_ptr constant = lsv_init->initializer.constant;
		    ast_node_info_ptr const_val = new_ast_node_info(NULL);
		    init_val = new_ast_node_info(constant);
		    if (constant->kind == ck_string ||
			(constant->kind == ck_address &&
			 constant->variant.address.kind == abk_constant &&
			 constant->variant.address.variant.constant->
			 kind == ck_string)) {
			init_val->node_name = NN_constant_str;
		    }
		    else if (constant->kind == ck_integer &&
			     constant->source_corresp.name) {
			init_val->node_name = NN_constant_enum;
		    }
		    else init_val->node_name = NN_constant;
		    add_symid_to_string(SET_symid_and_add_to_ast(constant, iek_constant, init_val), &string_buf);
		    add_pos_and_len_to_ast(&constant->source_corresp.
					   decl_position, init_val);
		    init_val->text = terminate_string(&string_buf);
		    add_1_char_to_string('\"', &string_buf);
		    constant_to_string(constant, &string_buf);
		    add_1_char_to_string('\"', &string_buf);
		    const_val->text = terminate_string(&string_buf);
		    const_val->is_leaf = TRUE;
		    relink(&init_val->first_child, const_val);
		}
	    }
	    else {
		init_val = ast_info_of(lsv_init->initializer.dynamic);
		if (init_val && init_val->node_name == NN_dynamic_init) {
		    ast_node_info_ptr ctor_call = init_val->first_child;
		    if (GNU_compatible_AST) {
			init_val->suppress_node = TRUE;
		    }
		    if (ctor_call && ctor_call->node_name ==
			NN_constructor_call &&
			!lsv_init->variable->initializer_range.start.seq) {
			/* implicit constructor call, must flag it as
			 * compiler-generated.
			 */
			add_3_chars_to_string(":cg", &string_buf);
			add_to_string(ctor_call->text, &string_buf);
			ctor_call->text = terminate_string(&string_buf);
		    }
		}
	    }
	    if (init_val) {
		ast_node_info_ptr decl_id = ast_info_of(lsv_init->variable)->
			first_child;
		ast_node_info_ptr init = new_ast_node_info(NULL);
		init->node_name = NN_init;
		add_range_to_ast(&lsv_init->variable->initializer_range,
				 init);
		init->text = terminate_string(&string_buf);
		relink(&decl_id->next_sibling, init);
		relink(&init->first_child, init_val);
		if (GNU_compatible_AST) {
		    init->suppress_node = TRUE;
		}
	    }
	}
    }
	break;

    case iek_vla_dimension: {
    }
	break;

    case iek_overriding_virtual_function: {
    }
	break;

    case iek_derivation_step: {
    }
	break;

    case iek_base_class_derivation: {
    }
	break;

    case iek_base_class: {
    }
	break;

    case iek_class_list_entry: {
    }
	break;

    case iek_routine_list_entry: {
    }
	break;

    case iek_class_type_supplement: {
    }
	break;

    case iek_template_param_type_supplement: {
    }
	break;

    case iek_constructor_init: {
	a_constructor_init_ptr ctor_init = (a_constructor_init_ptr)
		entry_ptr;
	unsigned long targ_symid = 0;
	astp = new_ast_node_info(ctor_init);
	if (ctor_init->kind == cik_virtual_base_class ||
	    ctor_init->kind == cik_direct_base_class) {
	    targ_symid = SET_symid_and_add_to_ast(ctor_init->variant.base_class->type, iek_type, astp);
	}
	else if (ctor_init->kind == cik_field) {
	    targ_symid = SET_symid_and_add_to_ast(ctor_init->variant.field, iek_field, astp);
	}
	else complain_int(error_csev, "Unexpected kind $1 of constructor init.",
			  ctor_init->kind);
	astp->node_name = NN_ctor_init;
	if (ctor_init->compiler_generated) {
	    add_3_chars_to_string(":cg", &string_buf);
	}
	add_symid_to_string(targ_symid, &string_buf);
	add_range_to_ast(&ctor_init->ctor_init_range, astp);
	astp->text = terminate_string(&string_buf);
	if (GNU_compatible_AST && ctor_init->kind == cik_field) {
	    ast_node_info_ptr field = new_ast_node_info(NULL);
	    field->node_name = NN_field;
	    add_symid_to_string(SET_symid_and_add_to_ast(ctor_init->variant.field, iek_field, field),
				&string_buf);
	    field->text = terminate_string(&string_buf);
	    relink(&field->next_sibling, ast_info_of(ctor_init->initializer));
	    relink(&astp->first_child, field);
	}
	else relink(&astp->first_child, ast_info_of(ctor_init->initializer));
    }
	break;

    case iek_asm_entry: {
	if (emit_asm_strings) {
	    an_asm_entry_ptr asm_entry = (an_asm_entry_ptr) entry_ptr;
	    ast_node_info_ptr opnd = new_ast_node_info(NULL);
	    add_1_char_to_string('\"', &string_buf);
	    constant_to_string(asm_entry->asm_string, &string_buf);
	    add_1_char_to_string('\"', &string_buf);
	    opnd->text = terminate_string(&string_buf);
	    opnd->is_leaf = TRUE;
	    astp = new_ast_node_info(asm_entry);
	    astp->node_name = NN_asm_entry;
	    add_symid_to_string(SET_symid_and_add_to_ast(asm_entry->asm_string, iek_constant, astp),
				&string_buf);
	    if (asm_entry->end_pos.seq) {
		add_pos_to_ast(&asm_entry->source_corresp.decl_position,
			       &asm_entry->end_pos,
			       astp);
	    }
	    else add_pos_and_len_to_ast(&asm_entry->source_corresp.decl_position,
					astp);
	    astp->text = terminate_string(&string_buf);
	    relink(&astp->first_child, opnd);
	}
    }
	break;

    case iek_template_arg: {
    }
	break;

    case iek_new_delete_supplement: {
    }
	break;

    case iek_throw_supplement: {
    }
	break;

    case iek_condition_supplement: {
    }
	break;

#if !ABI_CHANGES_FOR_RTTI
    case iek_accessible_base_class: {
    }
	break;
#endif /* !ABI_CHANGES_FOR_RTTI */

#if DO_IL_LOWERING && !DO_FULL_PORTABLE_EH_LOWERING
    case iek_eh_prologue_supplement: {
    }
	break;
#endif /* DO_IL_LOWERING && !DO_FULL_PORTABLE_EH_LOWERING */
#endif /* ifdef CIL */

#if GENERATE_SOURCE_SEQUENCE_LISTS
    case iek_source_sequence_entry: {
	a_source_sequence_entry_ptr seq =
		(a_source_sequence_entry_ptr) entry_ptr;
	intr_info_ptr intrp = (interesting_node(seq->entity.ptr) ?
			       get_intr_info_from_il_node(seq->entity.ptr):
			       get_intr_info_from_il_node(entry_ptr));
	a_symbol_ptr entry_sym = NULL;
	a_template_ptr parent_template = NULL;
	a_class_template_body_ptr template_body = NULL;
	an_il_entry_kind kind;
#if DEBUG
	if (db_source_sequence) {
	    fprintf(stdout, "processing source sequence entry: %s\n",
		entry_id(seq->entity));
	    dump_src_seq_entry(stdout, seq);
	}
	if (db_statement_stack) {
	    fprintf(stdout, "popping any completed statements from the stack\n");
	}
#endif /* DEBUG */
	/* First pop already-completed compound statements off the stack */
	while (top_of_stmt_stack && top_of_stmt_stack->do_pop) {
	    if (top_of_stmt_stack->stmt.kind == iek_switch_clause) {
		/* "breaks" that end switch clauses don't appear in
		 * the IL nor in the source sequence list, so we
		 * have to synthesize one here if there was one in
		 * the source.
		 */
		a_switch_clause_ptr scp = (a_switch_clause_ptr)
			top_of_stmt_stack->stmt.ptr;
		if (scp->implied_break_at_end) {
		    ast_node_info_ptr break_ast = new_ast_node_info(NULL);
		    break_ast->node_name = NN_break;
		    add_pos_to_ast(&scp->break_position,
				   &scp->break_end_position, break_ast);
		    break_ast->text = terminate_string(&string_buf);
		    relink(top_of_stmt_stack->link, break_ast);
		}
	    }
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
	    /* Try to do without this. */
#else /* !REPRESENT_EMPTY_STATEMENTS_IN_IL */
	    else if (top_of_stmt_stack->stmt.kind == iek_statement &&
		     ((a_statement_ptr) top_of_stmt_stack->stmt.ptr)->
		     kind == stmk_if) {
		 a_statement_ptr if_stmt =
			 (a_statement_ptr) top_of_stmt_stack->stmt.ptr;
		 a_boolean has_empty_else_clause =
			 if_stmt->has_empty_else_clause;
		 if (!has_empty_else_clause &&
		     if_stmt->variant.if_stmt.else_statement &&
		     il_header.source_language == sl_Cplusplus) {
		     a_statement_ptr else_stmt =
			     if_stmt->variant.if_stmt.else_statement;
		     if (else_stmt->kind == stmk_block &&
			 !else_stmt->variant.block.extra_info->
			 final_position.seq &&
			 !else_stmt->variant.block.statements) {
			 has_empty_else_clause = TRUE;
		     }
		 }
		 if (has_empty_else_clause) {
		     /* Has "else;", which has an empty else_statement; we
		      * need to manufacture a null_operand here to indicate
		      * that it wasn't an omitted else clause instead.
		      */
		     chain_at_curr_stmt_stack_level(make_null_ast_operand(),
						    intrp);
		 }
	    }
#endif /* REPRESENT_EMPTY_STATEMENTS_IN_IL */
	    if (top_of_stmt_stack->lifetime) {
		process_lifetime_window(top_of_stmt_stack->lifetime->
					parent_lifetime, intrp);
	    }
	    pop_stmt();
	}
	if (top_of_stmt_stack && top_of_stmt_stack->ending_stmt &&
	    top_of_stmt_stack->ending_stmt == seq) {
	    /* This is the end of the construct; pop when completed
	     * (except for compiler-generated blocks; these don't
	     * have end-of-construct entries, so we transfer the
	     * check to the last contained statement.
	     */
	    a_boolean do_pop = TRUE;
	    if (seq->entity.kind == iek_statement) {
		a_statement_ptr stmt = (a_statement_ptr)
			seq->entity.ptr;
		if (stmt->kind == stmk_block &&
		    !stmt->variant.block.extra_info->
		    final_position.seq) {
		    a_statement_ptr substmt;
		    a_source_sequence_entry_ptr the_substmt;
		    for (the_substmt = NULL, 
			 substmt = stmt->variant.block.statements;
			 substmt;
			 substmt = substmt->next)
		    { 
			if (substmt->source_sequence_entry != NULL)
			    the_substmt = substmt->source_sequence_entry;
		    }
		    if (the_substmt) {
			do_pop = FALSE;
			top_of_stmt_stack->ending_stmt = the_substmt;
#if DEBUG
			if (db_statement_stack) {
			    dump_eptr(stdout, "Revising ending_stmt of", top_of_stmt_stack->stmt.kind,
									 top_of_stmt_stack->stmt.ptr);
			    fprintf(stdout, "Revised ending_stmt to ");
			    dump_src_seq_entry(stdout, the_substmt);
			}
#endif
			if (stmt->variant.block.extra_info->assoc_scope) {
			    top_of_stmt_stack->lifetime = stmt->variant.block.
				    extra_info->assoc_scope->lifetime;
			}
		    }
		}
	    }
#if DEBUG
	    if (top_of_stmt_stack->do_pop != do_pop) {
		if (!do_pop) {
		    worry("Turning off do_pop during a check for turning on do_pop.");
		}
		if (db_statement_stack) {
		    fprintf(stdout, "Turning %s do_pop ", do_pop ? "on" : "off");
		    if (top_of_stmt_stack->stmt.ptr != NULL) {
			dump_eptr(stdout, "at end of", top_of_stmt_stack->stmt.kind,
						       top_of_stmt_stack->stmt.ptr);
		    }
		    else {
			fprintf(stdout, "at end of construct on top of statement stack.\n");
		    }
		}
	    }
#endif
	    top_of_stmt_stack->do_pop = do_pop;
	}
	if (top_of_stmt_stack != NULL && top_of_stmt_stack->stmt.kind == iek_template) {
	    template_body = &top_of_stmt_stack->template;
	    parent_template = (a_template_ptr)top_of_stmt_stack->stmt.ptr;
	}
	kind = ss_entry_kind(seq);
	if (kind == (an_il_entry_kind)iek_src_seq_end_of_construct) {
	    a_src_seq_end_of_construct_ptr p = ss_entry_ptr(seq, a_src_seq_end_of_construct_ptr);
	}
	else if (ss_entry_ptr(seq, void *) == NULL) {
	}
	else {
	    a_source_correspondence *scp;
	    if (kind == (an_il_entry_kind)iek_src_seq_secondary_decl) {
		a_src_seq_secondary_decl_ptr sssdp
		    = ss_entry_ptr(seq, a_src_seq_secondary_decl_ptr);
		scp = source_corresp_for_il_entry(
					sssdp->entity.ptr,
					(an_il_entry_kind)sssdp->entity.kind);
	    }
	    else {
		scp = source_corresp_for_il_entry(seq->entity.ptr, kind);
	    }
	    entry_sym = (scp != NULL ? (a_symbol_ptr)scp->assoc_info : NULL);
	}
	if (ast_info_of(seq)) {
	    /* already processed */
	} 
	else if (!interesting_node(seq->entity.ptr)) {
	}
	else {
	    /* not already processed */
	    if (seq->entity.kind == iek_routine &&
		!il_entry_prefix_of(seq->entity.ptr).SET_symid &&
		!((a_routine_ptr) seq->entity.ptr)->is_trivial_default_constructor) {
		/* Wasn't processed during traversal of global scope;
		 * probably there was a source error that prevented
		 * its entry there.  We need the symid and AST, though;
		 * process it now.
		 */
		walk_entry_and_subtree_for_IF((char*) seq->entity.ptr, iek_routine);
	    }
	    astp = ast_info_of(seq->entity.ptr);
	    if (seq->entity.kind == iek_template) {
		a_template_ptr tplp = (a_template_ptr) seq->entity.ptr;
		ast_node_info_ptr body = NULL;
		intrp = get_intr_info_from_source_seq(tplp->source_corresp.decl_position.seq);
		if (is_auto_gen_member(&tplp->source_corresp)) {
		    /* do nothing, member of template instance */
		}
		else {
		    chain_pending_template_member_decl(intrp);
		    chain_at_curr_stmt_stack_level(astp, intrp);
		    /* Find the block node,
		     * within which to chain the template members. */
		    if (astp != NULL
			    && astp->node_name == NN_declaration_no_dclrtr
			    && astp->first_child != NULL
			    && (astp->first_child->node_name == NN_template_struct
				|| astp->first_child->node_name == NN_template_union
				|| astp->first_child->node_name == NN_template_class)
			    && astp->first_child->first_child != NULL) {
			body = astp->first_child->first_child->next_sibling;
		    }
		}
		if (is_src_seq_beginning_of_construct(seq)) {
		    /* Link sequence entries within the template construct to the template body.
		     * This needs to be a class template. */
		    push_stmt(seq->entity, body);
		    top_of_stmt_stack->template.old_seq = seq;  /* parent_class->source_corresp.source_sequence_entry, */
		    top_of_stmt_stack->template.last_pos = NULL;
		    top_of_stmt_stack->template.this_pos = NULL;
		    top_of_stmt_stack->template.first_dclrtr_ast = NULL;
		    top_of_stmt_stack->template.last_dclrtr_ast = NULL;
		    top_of_stmt_stack->template.pending_class_def = NULL;
		    top_of_stmt_stack->template.prev_member_sym = NULL;
		}
	    }
	    else if (template_body != NULL && entry_sym != NULL) {
		/* template member not already processed */
		a_class_template_body_ptr body = template_body;
		a_type_ptr parent_class = parent_template->prototype_instantiation.type;

		ast_node_info_ptr this_dclrtr_ast = NULL;
		ast_node_info_ptr class_def = NULL;

		a_boolean created = parent_class != NULL &&
		                    create_ast_for_template_member(
					entry_sym,
					seq,
					&parent_template->definition_range,
					SET_symid_and_write_sym(parent_class, iek_type, 
								get_buffer_from_intr_info(intrp)),
					&body->old_seq,
					FALSE /* ?? nested class */,
					&body->this_pos,
					&this_dclrtr_ast,
					&class_def,
					intrp);

		if (created) {
		    /* Integration of new AST depends on whether it is within the same
		     * decl as the previous declarator.  Also, a class definition needs
		     * to be strung into the declaration list, not declarator list. */
		    a_class_template_body_ptr body = template_body;
		    if ((body->last_dclrtr_ast || body->pending_class_def) && 
			    this_dclrtr_ast && body->last_pos && body->this_pos &&
			    body->last_pos->specifiers_range.end.seq &&
			    body->last_pos->specifiers_range.end.seq ==
			    body->this_pos->specifiers_range.end.seq &&
			    body->last_pos->specifiers_range.end.mapped_column ==
			    body->this_pos->specifiers_range.end.mapped_column) {
			/* In same decl; add to declarator chain. */
			if (body->last_dclrtr_ast) {
			    if (body->last_dclrtr_ast != this_dclrtr_ast) {
				relink(&body->last_dclrtr_ast->next_sibling, this_dclrtr_ast);
			    }        
			    else worry("Relinking ignored to avoid circularity."); 
			}
			else body->first_dclrtr_ast = this_dclrtr_ast;
			}
		    else {
			chain_pending_template_member_decl(intrp);
			/* Start a new list of declarators. */
			body->first_dclrtr_ast = this_dclrtr_ast;
			body->pending_class_def = NULL;
		    }
		    body->last_dclrtr_ast = this_dclrtr_ast;
		    body->last_pos = body->this_pos;

		    /* If the member is a class definition, defer its integration into the
		     * AST because it might become part of a declaration. */
		    if (class_def) {
			body->pending_class_def = class_def;
		    }
		}
	    }
	    else if (seq->entity.kind == iek_constant) {
		a_constant_ptr cp = (a_constant_ptr) seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(cp->source_corresp.decl_position.seq);
		if (is_auto_gen_member(&cp->source_corresp)) {
		    /* do nothing, member of template instance */
		}
		else if (GNU_compatible_AST) {
		    /* This is an enumerator, need a const_decl for the
		     * gcc version of the AST.
		     */
		    a_constant_ptr constant = (a_constant_ptr)
			    seq->entity.ptr;
		    ast_node_info_ptr const_decl =
			    new_ast_node_info(NULL);
		    const_decl->node_name = NN_enumerator_decl;
		    add_to_string_with_len(":def", 4, &string_buf);
		    add_symid_to_string(SET_symid_and_add_to_ast(constant, iek_constant, const_decl),
					&string_buf);
		    if (constant->source_corresp.decl_pos_info) {
			a_source_position_ptr start
				= &constant->source_corresp.
				  decl_pos_info->identifier_range.start;
			if (constant->source_corresp.decl_pos_info->
			    variant.enum_value_range.end.seq) {
			    add_pos_to_ast(start,
					   &constant->source_corresp.
					   decl_pos_info->variant.
					   enum_value_range.end, const_decl);
			}
			else add_pos_to_ast(start,
					    &constant->source_corresp.
					    decl_pos_info->identifier_range.end,
					    const_decl);
		    }
		    const_decl->text = terminate_string(&string_buf);
		    relink(&const_decl->first_child, astp);
		    chain_at_curr_stmt_stack_level(const_decl, intrp);
		}
		else chain_at_curr_stmt_stack_level(astp, intrp);
	    }
	    else if (seq->entity.kind == iek_label ||
		     seq->entity.kind == iek_asm_entry) {
		chain_at_curr_stmt_stack_level(astp, intrp);
	    }
	    else if (seq->entity.kind == iek_type) {
		a_type_ptr type = (a_type_ptr) seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(type->source_corresp.decl_position.seq);
		if (is_auto_gen_member(&type->source_corresp)) {
		    /* Do nothing, this type is a member of an
		     * automatically-generated class template instance.
		     */
		}
		else if (type->kind == tk_class || type->kind == tk_struct
		    || type->kind == tk_union ||
		    (type->kind == tk_integer &&
		     type->variant.integer.enum_type)) {
		    /* push a level; leave allocation of ast nodes */
		    /* until doing the corresponding end seq, when we */
		    /* will know the true source range, including any */
		    /* storage class specifiers or cv qualifiers */
		    /* applying to the declarators, if any. */
		    a_class_type_supplement_ptr extra_info = NULL;
		    a_template_ptr template = NULL;
		    a_tagged_pointer entity = seq->entity;
		    if (type->kind != tk_integer) {
			extra_info = type->variant.class_struct_union.extra_info;
		    }
		    if (extra_info != NULL) {
			template = extra_info->assoc_template;
		    }
		    push_stmt(entity, astp);
		    if (astp != NULL) {
			if (GNU_compatible_AST) {
			    ast_node_info_ptr block = new_ast_node_info(NULL);
			    a_source_position lbrace;
			    a_source_position rbrace;
			    if (type->kind == tk_integer) {
				lbrace = type->variant.integer.lbrace_pos;
				rbrace = type->variant.integer.rbrace_pos;
			    }
			    else {
				lbrace = type->variant.
					class_struct_union.lbrace_pos;
				rbrace = type->variant.
					class_struct_union.rbrace_pos;
			    }
			    block->node_name = NN_block;
			    if (!lbrace.seq) {
				/* We don't know where the "{" of this block is,
				 * so we'll try to simulate it by taking the
				 * last token of the name clause and adding 1.
				 */
				if (type->source_corresp.decl_pos_info) {
				    lbrace = type->source_corresp.decl_pos_info->
					    identifier_range.end;
				    rbrace = type->source_corresp.decl_pos_info->
					    specifiers_range.end;
				}
				else {
				    lbrace = type->source_corresp.decl_position;
				    lbrace.mapped_column += lbrace.len;
				    rbrace = null_source_position;
				}
				if (extra_info) {
				    a_base_class_ptr base;
				    for (base = extra_info->base_classes; base;
					 base = base->next) {
					if (base->direct) {
					    lbrace = base->base_specifier_range.end;
					}
				    }
				}
				lbrace.mapped_column++;
			    }
			    add_pos_to_ast(&lbrace, &rbrace, block);
			    block->text = terminate_string(&string_buf);
			    relink(&astp->first_child->next_sibling,
				   block);
			    top_of_stmt_stack->link = &block->first_child;
			}
			else top_of_stmt_stack->link = &astp->first_child->
				next_sibling;	/* skip name_clause */
		    }
		}
		else if (type->kind == tk_typeref &&
			 type->source_corresp.name) {
		    /* typedef */
		    if (!end_of_declarator_chain) {
			/* first of possible chain of declarators */
			scan_declarator_chain(seq);
		    }
		    if (seq == end_of_declarator_chain) {
			make_declaration_tree(/*do_chain=*/TRUE, intrp);
		    }
		}
	    }
	    else if (seq->entity.kind == iek_variable) {
		a_variable_ptr var = (a_variable_ptr) seq->entity.ptr;
		a_boolean is_template_instance =
			var->is_template_static_data_member &&
			!var->is_specialized;
		intrp = get_intr_info_from_source_seq(var->source_corresp.decl_position.seq);
		if (var->init_kind == initk_dynamic && (   var->storage_class == sc_auto
							|| var->storage_class == sc_register)) {
		    most_recent_destructible_auto = var->initializer.dynamic;
		}
		if (var->is_parameter) {
		    ast_node_info_ptr dcl = new_ast_node_info(NULL);
		    ast_node_info_ptr specs;
		    a_param_type_ptr parm_type = var->assoc_param_type;
		    dcl->node_name = NN_parm_decl;
		    add_symid_to_string(SET_symid_and_add_to_ast(var, iek_param_type, dcl), &string_buf);
		    if (var->source_corresp.decl_pos_info) {
			a_decl_position_supplement_ptr info
				= var->source_corresp.decl_pos_info;
			add_pos_to_ast(
			    info->specifiers_range.start.seq
				? &info->specifiers_range.start
				: &info->variant.declarator_range.start,
			    param_var_has_default(var)
                                ? &parm_type->default_arg_expr->expr_range.end
			        : info->variant.declarator_range.end.seq
			            ? &info->variant.declarator_range.end
			            : &info->specifiers_range.end,
			    dcl);
		    }
		    dcl->text = terminate_string(&string_buf);
		    specs = make_decl_specs_ast(
			    var->source_corresp.decl_pos_info);
		    if (astp) {
			/* nameless parameters don't have declarators */
			relink(&specs->next_sibling, astp);
			if (GNU_compatible_AST) {
			    astp->suppress_node = TRUE;
			}
		    }
		    relink(&dcl->first_child, specs);
		    if (fcn_parm_link) {
			relink(fcn_parm_link, dcl);
			fcn_parm_link = &dcl->next_sibling;
		    }
		}
		else if (!var->is_handler_param && !is_template_instance) {
		    /* Suppress handler params because unnamed ones
		     * do not appear in the sequence list and so have
		     * to be treated specially; we will just do all
		     * catch parameters when we hit the handler.
		     * (Besides, this is the wrong place, even for
		     * named ones, as we want to make them the first
		     * operand of the associated "catch" node.)
		     */
		    if (!end_of_declarator_chain) {
			/* first of possible chain */
			scan_declarator_chain(seq);
		    }
		    if (seq == end_of_declarator_chain) {
			make_declaration_tree(/*do_chain=*/TRUE, intrp);
		    }
		}
	    }
	    else if (seq->entity.kind == iek_field) {
		a_field_ptr fp = (a_field_ptr) seq->entity.ptr;
		a_type_ptr parent = fp->source_corresp.parent.class_type;
		intrp = get_intr_info_from_source_seq(fp->source_corresp.decl_position.seq);
		if (!is_auto_gen_class(parent)) {
		    if (!end_of_declarator_chain) {
			scan_declarator_chain(seq);
		    }
		    if (seq == end_of_declarator_chain) {
			make_declaration_tree(/*do_chain=*/TRUE, intrp);
		    }
		}
	    }
	    else if (seq->entity.kind == iek_routine) {
		/* This is a function definition (function */
		/* declarations are classified as secondary */
		/* declarations, not as routines, for the purposes of */
		/* the source sequence list).  The parameters and body */
		/* of the function will be added later when we get to */
		/* the function's scope in the tree walk (this is in */
		/* the global scope). */
		if (astp) {
		    a_routine_ptr fcn = (a_routine_ptr) seq->entity.ptr;
		    ast_node_info_ptr def;
		    ast_node_info_ptr specs = NULL;
		    unsigned long fcn_symid;
		    intrp = get_intr_info_from_source_seq(fcn->source_corresp.decl_position.seq);
		    if (pending_declaration_head) {
			/* We'll need to overwrite the original contents
			 * (which were set up thinking this would be a
			 * variable), but we'll reuse the structure.
			 */
			def = pending_declaration_head;
			pending_declaration_head = NULL;
			specs = def->first_child;
		    }
		    else {
			def = new_ast_node_info(NULL);
			chain_at_curr_stmt_stack_level(def, intrp);
		    }
		    if (fcn->is_template_function && !fcn->is_specialized) {
			/* Don't include ASTs from template instances */
			def->suppress_subtree = TRUE;
		    }
		    def->node_name = NN_function_def;
		    fcn_symid = SET_symid_and_add_to_ast(fcn, iek_routine, def);
		    add_symid_to_string(fcn_symid, &string_buf);
		    {
			a_source_position_ptr end = NULL;
			if (fcn->assoc_scope != NULL_region_number) {
			    a_scope_ptr scope_ptr =
				    il_header.region_scope_entry[fcn->assoc_scope];
			    if (scope_ptr) {
				a_statement_ptr assoc_block =
					scope_ptr->assoc_block;
				if (assoc_block) {
				    if (assoc_block->kind == stmk_block) {
					end = &assoc_block->
						       variant.block.extra_info->
						       final_position;
				    }
				    else if (assoc_block->kind == stmk_asm_func_body) {
					end = &assoc_block->end_position;
				    }
				}
			    }
			}
			if (fcn->source_corresp.decl_pos_info) {
			    if (fcn->source_corresp.decl_pos_info->
				specifiers_range.start.seq) {
				add_pos_to_ast(&fcn->source_corresp.decl_pos_info->
					       specifiers_range.start,
					       end, def);
			    }
			    else add_pos_to_ast(&fcn->source_corresp.decl_pos_info->
						variant.declarator_range.start,
						end, def);
			}
			else add_pos_to_ast(&fcn->source_corresp.decl_position,
					    end, def);
		    }
		    if (fcn->assoc_scope == NULL_region_number) {
			/* We were counting on there being a scope, in order
			 * to handle the function parameters later. */
			worry_ulong("Function definition for [$1] has no scope.", fcn_symid);
		    }
		    def->text = terminate_string(&string_buf);
		    if (!specs) {
			specs = make_decl_specs_ast(
				    fcn->source_corresp.decl_pos_info);
			relink(&def->first_child, specs);
		    }
		    relink(&specs->next_sibling, astp);
		    il_entry_prefix_of(seq).SET_info = def;
		    start_of_declarator_chain = NULL;
		    end_of_declarator_chain = NULL;
		}
	    }
	    else if (seq->entity.kind == iek_switch_clause) {
		if (astp) {
		    a_source_sequence_entry_ptr ending_stmt = NULL;
		    a_switch_clause_ptr swcl = (a_switch_clause_ptr)
			    seq->entity.ptr;
		    chain_at_curr_stmt_stack_level(astp, intrp);
		    if (swcl->statements) {
			a_statement_ptr sp;
			for (sp = swcl->statements; sp != NULL; sp = sp->next) {
			    /* For example, if this statement
			     * is a compiler-generated
			     * goto to implement a fall-through case, it
			     * won't appear in the sequence list, so make
			     * the statement before the goto the one to
			     * check for the end of the grouping.
			     */
			    if (sp->source_sequence_entry != NULL) {
				ending_stmt = sp->source_sequence_entry;
			    }
			}
		    }
		    if (ending_stmt != NULL) {
			push_stmt(seq->entity, astp);
			top_of_stmt_stack->link = &astp->first_child->
				next_sibling;	/* skip case_values */
			top_of_stmt_stack->ending_stmt = ending_stmt;
		    }
		    else if (swcl->implied_break_at_end) {
			ast_node_info_ptr break_ast = new_ast_node_info(NULL);
			break_ast->node_name = NN_break;
			add_pos_to_ast(&swcl->break_position,
				       &swcl->break_end_position, break_ast);
			break_ast->text = terminate_string(&string_buf);
			relink(&astp->first_child->next_sibling, break_ast);
		    }
		}
	    }
	    else if (seq->entity.kind == iek_statement) {
		a_statement_ptr stmt = (a_statement_ptr)
			seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(stmt->position.seq);
		if (astp && stmt->kind != stmk_decl) {
		    if (stmt->kind == stmk_block) {
			a_block_ptr bp = stmt->variant.block.extra_info;
			a_scope_ptr sp = bp->assoc_scope;
			if (fcn_declarator) {
			    /* This is the body of a function; don't
			     * chain it because it's handled specially.
			     */
			}
			else if (!bp->final_position.seq) {
			    /* Compiler-generated, ignore */
			}
			else if (sp && sp->variant.assoc_handler) {
			    /* This is the body of a catch clause.  We
			     * need to push the handler, create and link
			     * the declaration of its parameter, if any,
			     * and then link the block as the second
			     * operand of the handler.
			     */
			    a_handler_ptr hp = sp->variant.assoc_handler;
			    ast_node_info_ptr hp_ast = ast_info_of(hp);
			    a_tagged_pointer hp_entity;
			    chain_at_curr_stmt_stack_level(hp_ast, intrp);
			    hp_entity.kind = iek_handler;
			    hp_entity.ptr = (char*) hp;
			    push_stmt(hp_entity, hp_ast);
			    /* This is the only child, so set the stack
			     * to pop as soon as this block completes.
			     */
#if DEBUG
			    if (db_statement_stack && !top_of_stmt_stack->do_pop) {
				dump_eptr(stdout, "Setting do_pop on", hp_entity.kind, hp_entity.ptr);
			    }
#endif
			    top_of_stmt_stack->do_pop = TRUE;
			    if (hp->parameter) {
				/* We deferred creating the declaration node
				 * for this variable so we could put it here.
				 */
				ast_node_info_ptr dcl = new_ast_node_info(NULL);
				ast_node_info_ptr specs;
				ast_node_info_ptr dclrtr = ast_info_of(hp->parameter);
				a_decl_position_supplement_ptr pos = hp->parameter->
					source_corresp.decl_pos_info;
				dcl->node_name = NN_declaration_with_dclrtr;
				if (pos) {
				    if (pos->variant.declarator_range.end.seq) {
					add_pos_to_ast(&pos->specifiers_range.start,
						       &pos->variant.declarator_range.end,
						       dcl);
				    }
				    else add_pos_to_ast(&pos->specifiers_range.start,
							&pos->specifiers_range.end,
							dcl);
				}
				else add_pos_to_ast(&hp->parameter->
						    source_corresp.decl_position,
						    NULL, dcl);
				dcl->text = terminate_string(&string_buf);
				specs = make_decl_specs_ast(pos);
				relink(&dcl->first_child, specs);
				if (dclrtr) {
				    relink(&specs->next_sibling, dclrtr);
				}
				chain_at_curr_stmt_stack_level(dcl, intrp);
			    }
			    else chain_at_curr_stmt_stack_level(make_null_ast_operand(), intrp);
			    if (hp->dynamic_init && ast_info_of(hp->dynamic_init)) {
				/* The dynamic_init for a catch clause doesn't
				 * add any information, so suppress it.
				 */
				ast_info_of(hp->dynamic_init)->suppress_subtree = TRUE;
			    }
			    chain_at_curr_stmt_stack_level(astp, intrp);
			}
			else chain_at_curr_stmt_stack_level(astp, intrp);
		    }
		    else if (stmt->kind == stmk_goto ||
			     (stmt->kind == stmk_return &&
			      curr_fcn_scope->lifetime)) {
			/* goto and return cause destructor invocation for
			 * local automatic objects that are going out of scope.
			 * We need to process the indicated lifetime; we also
			 * need to create a compiler-generated block to contain
			 * both the destructions and the goto/return (in case of
			 * something like "if (x) return;") -- the block will be
			 * suppressed if there are no destructor calls.
			 */
			a_tagged_pointer fake_block = { iek_statement, NULL };
			ast_node_info_ptr* saved_link;
			ast_node_info_ptr fake_block_ast = new_ast_node_info(NULL);
			fake_block_ast->node_name = NN_block;
			fake_block_ast->text = ":cg";
			chain_at_curr_stmt_stack_level(fake_block_ast,
						       intrp);
			push_stmt(fake_block, fake_block_ast);
			saved_link = top_of_stmt_stack->link;
			if (stmt->kind == stmk_goto) {
			    process_lifetime_window(stmt->variant.label.lifetime, intrp);
			}
			else process_lifetime_window(curr_fcn_scope->lifetime->
						     parent_lifetime, intrp);
			if (top_of_stmt_stack->link == saved_link) {
			    fake_block_ast->suppress_node = TRUE;
			}
			chain_at_curr_stmt_stack_level(astp, intrp);
			pop_expected_stmt(fake_block, TRUE);
		    }
		    else chain_at_curr_stmt_stack_level(astp, intrp);
		    if (stmt->kind == stmk_if) {
			if (stmt->variant.if_stmt.then_statement ||
			    stmt->variant.if_stmt.else_statement) {
			    a_statement_ptr then_stmt =
				    stmt->variant.if_stmt.then_statement;
			    push_stmt(seq->entity, astp);
			    top_of_stmt_stack->link = &astp->first_child->
				    next_sibling;	/* skip over expr */
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
			    /* Try to do without this. */
#else
			    if (then_stmt && then_stmt->kind == stmk_block &&
				block_is_generated(then_stmt) &&
				!then_stmt->variant.block.statements) {
				/* The "then" statement was omitted in the source.
				 * It is represented in the IL by a compiler-generated
				 * block (no ending position) with no enclosed
				 * statements.  Compiler-generated blocks are
				 * suppressed, so there's nothing to link in
				 * the "then" spot in the tree.
				 */
				then_stmt = NULL;
			    }
#endif
			    if (!then_stmt) {
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
				/* Try this, too, for EDG 2.44. */
				worry("if statement with no then in IL");
#endif
				chain_at_curr_stmt_stack_level(make_null_ast_operand(), intrp);
			    }
			}
			else {
			    relink(&astp->first_child->next_sibling,
				   make_null_ast_operand());
			}
		    }
		    else if (stmt->kind == stmk_while ||
			     stmt->kind == stmk_end_test_while) {
			if (stmt->variant.loop_statement) {
			    push_stmt(seq->entity, astp);
			    top_of_stmt_stack->link = &astp->first_child->
				    next_sibling;	/* skip over expr */
			}
		    }
		    else if (stmt->kind == stmk_block) {
			a_scope_ptr scope =
				stmt->variant.block.extra_info->assoc_scope;
			if (block_is_generated(stmt)) {
			    /* ignore -- compiler-generated */
			}
			else if (scope && scope->variant.assoc_handler) {
			    /* This block is the body of a catch clause. */
			    /* Link the node for the handler and skip over */
			    /* the block for linking (the block is already */
			    /* the child of the handler). */
			    push_stmt(seq->entity,
				      ast_info_of(scope->variant.assoc_handler));
			    top_of_stmt_stack->link = &astp->first_child;
			}
			else {
			    if (fcn_declarator) {
				/* This will be linked in later when we see
				 * the end of the scope, when we know about
				 * ctor_inits that have to be linked before it.
				 */
				fcn_declarator = NULL;
			    }
			    push_stmt(seq->entity, astp);
			    if (astp && (astp->node_name == NN_except ||
					 astp->node_name == NN_finally)) {
				/* skip over expression and into body of block */
				top_of_stmt_stack->link = &astp->first_child->
					next_sibling->first_child;
			    }
			}
		    }
		    else if (stmt->kind == stmk_for) {
			a_for_loop_ptr for_loop =
				stmt->variant.for_loop.extra_info;
			push_stmt(seq->entity, astp);
			if (!for_loop->initialization) {
			    relink(&astp->first_child, make_null_ast_operand());
			}
			else {
			    /* recursively process the next sequence entry(ies), so */
			    /* we can link the test expr in the correct place */

			    /* Hold of on popping this for statement just yet. */
			    a_boolean save_do_pop = top_of_stmt_stack->do_pop;
			    top_of_stmt_stack->do_pop = FALSE;
			    if (seq->next->entity.kind == iek_variable) {
				/* do declarator chain so we'll have a whole
				 * declaration to link in.
				 */
				a_source_sequence_entry_ptr scan_seq;
				scan_declarator_chain(seq->next);
				for (scan_seq = start_of_declarator_chain;
				     end_of_declarator_chain;
				     scan_seq = scan_seq->next) {
				    /* end_of_declarator_chain will be NULLed
				     * when the last declarator is processed.
				     */
				    process_ast_of_entry((char*) scan_seq,
						  iek_source_sequence_entry);
				}
			    }
			    else {
				/* handle simple expression statement case. */
				process_ast_of_entry((char*) seq->next,
					      iek_source_sequence_entry);
				if (!ast_info_of(seq->next)) {
				    /* make sure it's not processed twice */
				    il_entry_prefix_of(seq->next).SET_info =
					    astp->first_child;
				}
			    }
			    top_of_stmt_stack->do_pop = save_do_pop;
			    if (!astp->first_child) {
				/* oops, something went wrong; at least we can
				 * prevent a crash from dereferencing a NULL ptr
				 */
				relink(&astp->first_child, make_null_ast_operand());
			    }
			}
			if (stmt->expr) {
			    relink(&astp->first_child->next_sibling,
				  (ast_node_info_ptr) ast_info_of(stmt->expr));
			}
			else relink(&astp->first_child->next_sibling,
				    make_null_ast_operand());
			if (for_loop->increment) {
			    relink(&astp->first_child->next_sibling->next_sibling,
				   (ast_node_info_ptr)
				   ast_info_of(for_loop->increment));
			}
			else relink(&astp->first_child->next_sibling->next_sibling,
				    make_null_ast_operand());
			if (stmt->variant.for_loop.statement) {
			    top_of_stmt_stack->link = &astp->first_child->
				    next_sibling->	/* init */
				    next_sibling->	/* condition */
				    next_sibling;	/* increment */
			}
			else pop_expected_stmt(seq->entity, TRUE);
		    }
		    else if (stmt->kind == stmk_switch &&
			     stmt->variant.switch_stmt.body_statement) {
			push_stmt(seq->entity, astp);
			top_of_stmt_stack->link = &astp->first_child->
				next_sibling;	/* skip over expr */
		    }
		    else if (stmt->kind == stmk_try_block) {
			push_stmt(seq->entity, astp);
		    }
#ifdef MICROSOFT_EXTENSIONS_ALLOWED
		    else if (stmt->kind == stmk_microsoft_try) {
			push_stmt(seq->entity, astp);
		    }
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
		}
	    }
	    else if (seq->entity.kind == iek_namespace) {
		a_namespace_ptr nmsp = (a_namespace_ptr)seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(nmsp->source_corresp.decl_position.seq);
		if(!nmsp->is_namespace_alias)
		{
		    astp = new_ast_node_info(seq->entity.ptr);
		    chain_at_curr_stmt_stack_level(astp, intrp);
		    push_stmt(seq->entity, astp);
		    /* leave text until after we know the ending location */
		}
	    }
	    else if (seq->entity.kind == iek_using_decl) {
		a_using_decl_ptr using_decl = (a_using_decl_ptr)
			seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(using_decl->position.seq);
		if (!(using_decl->is_class_member &&
		      is_auto_gen_class(using_decl->qualifier.class_type))) {
		    ast_node_info_ptr node = new_ast_node_info(NULL);
		    a_using_decl_ptr ilp;
		    ast_node_info_ptr* ast_link;
		    node->node_name = NN_using_decl;
		    add_pos_to_ast(&using_decl->position, NULL, node);
		    node->text = terminate_string(&string_buf);
		    for (ilp = using_decl, ast_link = &node->first_child;
			 ilp; ilp = ilp->next_in_overload_set,
			 ast_link = &(*ast_link)->next_sibling) {
			relink(ast_link, ast_info_of(ilp));
		    }
		    chain_at_curr_stmt_stack_level(node, intrp);
		}
	    }
	    else if (seq->entity.kind == iek_src_seq_secondary_decl) {
		a_src_seq_secondary_decl_ptr sseq =
			(a_src_seq_secondary_decl_ptr) seq->entity.ptr;
		a_source_sequence_entry_ptr main_seq = NULL;
		a_boolean is_defined = FALSE;
		a_boolean is_template_instance = FALSE;
		intrp = get_intr_info_from_source_seq(sseq->decl_position.seq);

		if (sseq->entity.kind == iek_variable) {
		    a_variable_ptr var = (a_variable_ptr) sseq->entity.ptr;
		    main_seq = var->source_corresp.source_sequence_entry;
		    is_defined = (var->storage_class != sc_extern);
		    if (var->is_template_static_data_member && !var->is_specialized) {
			is_template_instance = TRUE;
		    }
		    else if (is_auto_gen_member(&var->source_corresp)) {
			is_template_instance = TRUE;
		    }
		}
		else if (sseq->entity.kind == iek_routine) {
		    a_routine_ptr fcn = (a_routine_ptr) sseq->entity.ptr;
		    is_defined = fcn->defined;
		    main_seq = fcn->source_corresp.source_sequence_entry;
		    if (fcn->is_template_function && !fcn->is_specialized) {
			is_template_instance = TRUE;
		    }
		    else if (is_auto_gen_member(&fcn->source_corresp)) {
			is_template_instance = TRUE;
		    }
		}
		else if (sseq->entity.kind == iek_type) {
		    if (is_auto_gen_class((a_type_ptr) sseq->entity.ptr)) {
			is_template_instance = TRUE;
		    }
		}
		if (sseq->entity.kind == iek_namespace) {
		    if(!((a_namespace_ptr)sseq->entity.ptr)->is_namespace_alias)
		    {
			astp = new_ast_node_info(sseq);
			chain_at_curr_stmt_stack_level(astp, intrp);
			push_stmt(seq->entity, astp);
			/* leave text until we know the ending location */
		    }
		}
		else if (is_template_instance) {
		    /* suppress template instances from AST */
		    ast_node_info_ptr dclrtr = ast_info_of(sseq->entity.ptr);
		    if (dclrtr && !entry_is_prototype_instance_member(sseq->entity.ptr, sseq->entity.kind)) {
			dclrtr->suppress_subtree = TRUE;
		    }
		}
		else if (is_in_class_def(sseq)) {
		    /* transplant definition in place of secondary decl */
		    process_ast_of_entry((char*) main_seq,
				  iek_source_sequence_entry);
		    il_entry_prefix_of(seq).SET_info =
			    ast_info_of(main_seq);	/* mark as processed */
		}
		else if (sseq->entity.kind == iek_template) {
		    /* The AST is in the src seq secondary decl. */
		    ast_node_info_ptr tpl_ast = ast_info_of(sseq->entity.ptr);
		    chain_at_curr_stmt_stack_level(tpl_ast, intrp);
		}
		else {
		    if ((main_seq != seq || !is_defined) &&
			     !sseq->implicit_decl) {
			if (!end_of_declarator_chain) {
			    scan_declarator_chain(seq);
			}
			if (seq == end_of_declarator_chain) {
			    make_declaration_tree(/*do_chain=*/TRUE, intrp);
			}
		    }
		    if (sseq->entity.kind == iek_routine) {
			a_routine_ptr fcn = (a_routine_ptr)sseq->entity.ptr;
			write_SMT_for_parameters(sseq->declared_type, 
						 get_buffer_from_intr_info(intrp));
		    }
		}
	    }
	    else if (seq->entity.kind == iek_src_seq_sublist) {
		/* process deferred source sequence entries */
		/* recursively now. */
		a_src_seq_sublist_ptr sublist =
			(a_src_seq_sublist_ptr) seq->entity.ptr;
		a_source_sequence_entry_ptr subseq;
		subsequence_of = seq;
		for (subseq = sublist->source_sequence_list; subseq;
		     subseq = subseq->next) {
		    process_ast_of_entry((char*) subseq,
				  iek_source_sequence_entry);
		}
		subsequence_of = NULL;
	    }
	    else if (seq->entity.kind == iek_src_seq_end_of_construct) {
		a_src_seq_end_of_construct_ptr eoc =
			(a_src_seq_end_of_construct_ptr)
			seq->entity.ptr;
		intrp = get_intr_info_from_source_seq(eoc->position.seq);
		if (eoc->entity.kind == iek_statement) {
		    a_statement_ptr stmt = (a_statement_ptr)
			    eoc->entity.ptr;
		    if (stmt->kind == stmk_decl) {
			/* for-init decl; do nothing */
		    }
		    else if (stmt->kind == stmk_block) {
			if (!block_is_generated(stmt)) {
			    int level;
			    /* ignore compiler-generated blocks */
			    a_statement_ptr substmt;
			    /* look for compiler-generated return with expr
			     * (implicit "return 0" in main) and link it in
			     * if it's present.
			     */
			    for (substmt = stmt->variant.block.statements;
				 substmt; substmt = substmt->next) {
				if (!substmt->next &&
				    substmt->kind == stmk_return &&
				    !substmt->source_sequence_entry &&
				    ast_info_of(substmt)) {
				    chain_at_curr_stmt_stack_level(ast_info_of(substmt), intrp);
				}
			    }
			    if (stmt->variant.block.extra_info->assoc_scope &&
				stmt->variant.block.extra_info->assoc_scope->lifetime) {
				process_lifetime_window(stmt->variant.block.extra_info->
							assoc_scope->lifetime->
							parent_lifetime, intrp);
			    }
			    else if (!(top_of_stmt_stack != NULL && top_of_stmt_stack->next) &&
                                     curr_fcn_scope &&
				     curr_fcn_scope->lifetime) {
				process_lifetime_window(curr_fcn_scope->lifetime->
							parent_lifetime, intrp);
			    }
			    level = find_stmt_entity(stmt);
			    if (level < 0) {
				complain(error_csev, "Block statement not matched.");
			    }
			    else if (level > 0) {
				complain_int(error_csev, "$1 unmatched statement levels in block statement.", level);
				for (; level > 0; level -= 1) {
				}
			    }
			    pop_expected_stmt(eoc->entity, TRUE);
			}
		    }
		    else complain(error_csev, "Unexpected end of statement construct.");
		}
		else if (eoc->entity.kind == iek_type) {
		    a_type_ptr type = (a_type_ptr) eoc->entity.ptr;
		    if (type->kind == tk_class || type->kind ==
			tk_struct || type->kind == tk_union ||
			(type->kind == tk_integer &&
			 type->variant.integer.enum_type)) {
			/* We have now reached the end of a class or */
			/* enum declaration.  We need to supply the */
			/* declaration header, but we can't do that */
			/* until we check for the presence of */
			/* declarators following this and having the */
			/* same ending point of the declaration */
			/* specifiers (the starting point may not be */
			/* the same because of storage class and cv */
			/* qualifiers). */
			a_decl_position_supplement_ptr this_pos =
				type->source_corresp.decl_pos_info;
			a_source_position start_pos;
			a_source_position end_pos = eoc->position;
			a_boolean do_pop = !is_auto_gen_member(&type->source_corresp);
			/* If popping reveals a template body, then the AST is
			 * handled by special-case template code, later. */
			a_boolean of_template = top_of_stmt_stack != NULL
			    && (!do_pop ? top_of_stmt_stack->stmt.kind == iek_template
					: (top_of_stmt_stack->next != NULL
					   && top_of_stmt_stack->next->stmt.kind == iek_template));
			ast_node_info_ptr def = of_template ? NULL : ast_info_of(type);
			if (def && !do_pop) {
			    /* This type is a member of an automatically-
			     * instantiated class and thus should not
			     * have an AST.
			     */
			    def->suppress_subtree = TRUE;
			    def = NULL;
			}
			if (def) {
			    ast_node_info_ptr dcl = new_ast_node_info(NULL);
			    ast_node_info_ptr specs = new_ast_node_info(NULL);
			    a_source_sequence_entry_ptr next_seq;
			    a_decl_position_supplement_ptr next_pos;
			    a_node_name dcl_node_name = NN_declaration_no_dclrtr;
			    if (seq->next) {
				next_seq = seq->next;
			    }
			    else if (subsequence_of) {
				next_seq = subsequence_of->next;
			    }
			    else next_seq = NULL;
			    next_pos = decl_pos_of(next_seq);
			    /* Now we know where the "}" is, so we can construct */
			    /* the text of the def node. */
			    if (type->kind == tk_class) {
				def->node_name = NN_class_def;
			    }
			    else if (type->kind == tk_struct) {
				def->node_name = NN_struct_def;
			    }
			    else if (type->kind == tk_union) {
				def->node_name = NN_union_def;
			    }
			    else def->node_name = NN_enum_def;
			    if (type->kind == tk_integer ||
				type->size != 0) {
				add_to_string_with_len(":def", 4, &string_buf);
			    }
			    add_symid_to_string(SET_symid_and_add_to_ast(type, iek_type, def), &string_buf);
			    if (this_pos) {
				add_pos_to_ast(&this_pos->specifiers_range.start,
					       &this_pos->specifiers_range.end,
					       def);
				end_pos = this_pos->specifiers_range.end;
			    }
			    else add_pos_to_ast(&type->source_corresp.decl_position,
						&eoc->position,
						def);
			    def->text = terminate_string(&string_buf);
			    specs->node_name = NN_decl_specs;
			    add_symid_to_string(SET_symid_and_add_to_ast(type, iek_type, specs), &string_buf);
			    if (this_pos && next_pos &&
				this_pos->specifiers_range.end.seq ==
				next_pos->specifiers_range.end.seq &&
				this_pos->specifiers_range.end.mapped_column ==
				next_pos->specifiers_range.end.mapped_column) {
				a_decl_position_supplement_ptr end_seq_pos;
				dcl_node_name = NN_declaration_with_dclrtr;
				start_pos = next_pos->specifiers_range.start;
				add_pos_to_ast(&start_pos,
					       &next_pos->specifiers_range.end,
					       specs);
				scan_declarator_chain(next_seq);
				pending_declaration_head = dcl;
				end_seq_pos = decl_pos_of(end_of_declarator_chain);
				end_pos = end_seq_pos->
					variant.declarator_range.end;
			    }
			    else if (this_pos) {
				start_pos = this_pos->specifiers_range.start;
				add_pos_to_ast(&start_pos,
					       &this_pos->specifiers_range.end,
					       specs);
			    }
			    else {
				start_pos = type->source_corresp.decl_position;
				add_pos_to_ast(&start_pos, &end_pos, specs);
			    }
			    specs->text = terminate_string(&string_buf);
			    dcl->node_name = dcl_node_name;
			    if (GNU_compatible_AST && 
				dcl_node_name == NN_declaration_no_dclrtr) {
				dcl->suppress_node = TRUE;
				specs->suppress_node = TRUE;
			    }
			    add_pos_to_ast(&start_pos, &end_pos, dcl);
			    dcl->text = terminate_string(&string_buf);
			    relink(&dcl->first_child, specs);
			    relink(&specs->first_child, def);
			    pop_expected_stmt(eoc->entity, do_pop);
			    chain_at_curr_stmt_stack_level(dcl, intrp);
			}
			else {
			    pop_specific_stmt(eoc->entity, do_pop);
			}
		    }
		    else complain(error_csev, "Unexpected type in end of construct.");
		}
		else if (eoc->entity.kind == iek_namespace) {
		    a_tagged_pointer top_of_stack = pop_expected_stmt(eoc->entity, TRUE);
		    a_namespace_ptr ns = (a_namespace_ptr) eoc->entity.ptr;
		    astp = ast_info_of(top_of_stack.ptr);
		    if (astp) {
			astp->node_name = NN_namespace_def;
			add_symid_to_string(SET_symid_and_add_to_ast(ns, iek_namespace, astp), &string_buf);
			if (top_of_stack.kind == iek_src_seq_secondary_decl) {
			    a_src_seq_secondary_decl_ptr sseq =
				    (a_src_seq_secondary_decl_ptr) top_of_stack.ptr;
			    add_pos_to_ast(&sseq->decl_position, &eoc->position, astp);
		    }
		    else add_pos_to_ast(&ns->source_corresp.decl_position,
					&eoc->position,
					astp);
			astp->text = terminate_string(&string_buf);
		    }
		}
		else if (eoc->entity.kind == iek_template) {
		    a_template_ptr template = (a_template_ptr)eoc->entity.ptr;
		    if (top_of_stmt_stack != NULL && top_of_stmt_stack->stmt.ptr == (char *)template) {
			chain_pending_template_member_decl(intrp);
			pop_specific_stmt(eoc->entity, TRUE);
		    }
		}
		else {
#if !DEBUG
		    worry("Unexpected kind in end of construct");
#else /* DEBUG */
		    worry_str("Unexpected kind ($1) in end of construct",
			       il_entry_kind_names[eoc->entity.kind]);
#endif /* DEBUG */
		}
	    }
	}
    }
	break;

    case iek_src_seq_secondary_decl: {
	a_src_seq_secondary_decl_ptr sseq =
		(a_src_seq_secondary_decl_ptr) entry_ptr;
	a_type_ptr sseq_type = sseq->declared_type;
	a_boolean is_template_instance = FALSE;
	if (!ast_info_of(sseq)) {
	    a_source_sequence_entry_ptr main_seq;
	    if (sseq->entity.kind == iek_variable) {
		a_variable_ptr var = (a_variable_ptr) sseq->entity.ptr;
		main_seq = var->source_corresp.source_sequence_entry;
		is_template_instance = var->is_template_static_data_member &&
			!var->is_specialized;
	    }
	    else if (sseq->entity.kind == iek_routine) {
		a_routine_ptr fcn = (a_routine_ptr) sseq->entity.ptr;
		main_seq = fcn->source_corresp.source_sequence_entry;
		is_template_instance = fcn->is_template_function &&
			!fcn->is_specialized;
	    }
	    else main_seq = NULL;
	    if (is_template_instance
		&& !entry_is_prototype_instance_member(sseq->entity.ptr, sseq->entity.kind)) {
		/* suppress AST for template-generated declarations */
	    }
	    else if (main_seq && main_seq->entity.ptr == (char*) sseq) {
		/* This secondary decl refers to the main declaration */
		/* of the entity in this translation unit; the ast */
		/* subtree was already allocated, so we should reuse */
		/* that instead of allocating a new one and just link */
		/* the parameters in. */
		ast_node_info_ptr dclrtr = ast_info_of(sseq->entity.ptr);
		if (dclrtr) {
		    il_entry_prefix_of(sseq).SET_info = dclrtr;
		    if (sseq_type && sseq_type->kind == tk_routine) {
			parallelize_links(dclrtr->first_child->next_sibling,
					  sseq_type->variant.routine.extra_info->
					  param_type_list,
					  a_param_type_ptr);
		    }
		}
	    }
	    else if (is_in_class_def(sseq)) {
		/* This is an in-class definition of a member function.
		 * If it had default arguments, the types of the secondary
		 * declaration and the main routine are distinct, with the
		 * secondary decl's type not having an assoc_routine.
		 * Because this type is indistinguishable from the types
		 * used for function pointers and the like, AST nodes were
		 * generated for the parameters when the type was
		 * processed and will be orphaned, because this secondary
		 * decl will be ignored in favor of the actual definition.
		 * Consequently, we have to suppress the parameters here.
		 */
		a_type_ptr type = sseq->declared_type;
		if (type && type->kind == tk_routine &&
		    !type->variant.routine.extra_info->assoc_routine) {
		    a_param_type_ptr param_type;
		    for (param_type = type->variant.routine.extra_info->
			 param_type_list; param_type; param_type =
			 param_type->next) {
			ast_node_info_ptr parm_ast =
				ast_info_of(param_type);
			if (parm_ast) {
			    parm_ast->suppress_subtree = TRUE;
			}
		    }
		}
	    }
	    else if (sseq->entity.kind == iek_namespace) {
		/* do nothing, all handled in source sequence entry */
	    }
	    else if (sseq->entity.kind == iek_template) {
		/* An AST was generated for the template. */
		a_template_ptr tpl = (a_template_ptr)sseq->entity.ptr;
		ast_node_info_ptr dclrtr = ast_info_of(tpl);
		if (dclrtr) {
		    il_entry_prefix_of(sseq).SET_info = dclrtr;
		}
	    }
	    else if (sseq->entity.kind != iek_type ||
		     (((a_type_ptr) sseq->entity.ptr)->kind == tk_typeref &&
		      ((a_type_ptr) sseq->entity.ptr)->source_corresp.name)) {
		ast_node_info_ptr name = new_ast_node_info(NULL);
		astp = new_ast_node_info(sseq);
		if (sseq->entity.kind == iek_routine) {
		    astp->node_name = NN_declarator_fcn;
		}
		else astp->node_name = NN_declarator_var;
		add_symid_to_string(SET_symid_and_add_to_ast(sseq->entity.ptr, sseq->entity.kind, astp),
				    &string_buf);
		if (sseq->decl_pos_info) {
		    add_range_to_ast(&sseq->decl_pos_info->
				     variant.declarator_range,
				     astp);
		}
		astp->text = terminate_string(&string_buf);
		if (sseq->entity.kind == iek_field) {
		    name->node_name = NN_decl_id_fld;
		}
		else if (sseq->entity.kind == iek_routine) {
		    name->node_name = NN_decl_id_fcn;
		}
		else if (sseq->entity.kind == iek_variable) {
		    name->node_name = NN_decl_id_var;
		}
		else if (sseq->entity.kind == iek_type) {
		    name->node_name = NN_decl_id_typd;
		}
		else {
		    complain_int(error_csev, "Unexpected kind $1 for secondary decl.",
				 sseq->entity.kind);
		    name->node_name = NN_unknown_node;
		}
		add_symid_to_string(SET_symid_and_add_to_ast(sseq->entity.ptr, sseq->entity.kind, name),
				    &string_buf);
		if (sseq->decl_pos_info) {
		    add_range_to_ast(&sseq->decl_pos_info->identifier_range,
				     name);
		}
		else add_pos_and_len_to_ast(&sseq->decl_position,
					    name);
		name->text = terminate_string(&string_buf);
		relink(&astp->first_child, name);
		if (sseq_type && sseq_type->kind == tk_routine) {
		    if (ast_info_of(sseq_type->variant.routine.extra_info->
				    param_type_list)) {
			/* May not have AST for template fcns */
			parallelize_links(name->next_sibling,
					  sseq_type->variant.routine.
					  extra_info->param_type_list,
					  a_param_type_ptr);
		    }
		}
	    }
	    else if (sseq->entity.kind == iek_type /* && prototype_instance_member */) {
		/* Can't tell just from the secondary declaration whether the AST
		 * will be needed ... rather than check for prototype_instance_member,
		 * leave it up to the point where it's referenced. */
		a_node_name nn;
		a_type_ptr tp = (a_type_ptr) sseq->entity.ptr;
		if (tp->kind == tk_struct) {
		    nn = NN_struct_def;
		}
		else if (tp->kind == tk_union) {
		    nn = NN_union_def;
		}
		else if (tp->kind == tk_class) {
		    nn = NN_class_def;
		}
		else nn = NN_unknown_node;
		if (nn != NN_unknown_node) {
		    ast_node_info_ptr dcl = new_ast_node_info(sseq);
		    ast_node_info_ptr def = new_ast_node_info(NULL);
		    ast_node_info_ptr name = new_ast_node_info(NULL);
		    dcl->node_name = NN_declaration_no_dclrtr;
		    if (sseq->decl_pos_info != NULL) {
			add_range_to_ast(&sseq->decl_pos_info->specifiers_range,
					 dcl);
		    }
		    dcl->text = terminate_string(&string_buf);
		    def->node_name = nn;
		    if (GNU_compatible_AST) {
			def->text = "";
			def->suppress_node = TRUE;
		    }
		    else {
			if (sseq->decl_pos_info != NULL) {
			    add_range_to_ast(&sseq->decl_pos_info->specifiers_range,
					     def);
			}
			def->text = terminate_string(&string_buf);
		    }
		    name->node_name = NN_name_clause;
		    add_symid_to_string(SET_symid_and_add_to_ast(tp, iek_type, name), &string_buf);
		    if (sseq->decl_pos_info != NULL ) {
			add_range_to_ast(&sseq->decl_pos_info->identifier_range,
					 name);
		    }
		    name->text = terminate_string(&string_buf);
		    relink(&def->first_child, name);
		    relink(&dcl->first_child, def);
		}
	    }
	}
    }
	break;

    case iek_src_seq_end_of_construct: {
    }
	break;

    case iek_src_seq_sublist: {
    }
	break;

#if COMMENTS_IN_SOURCE_SEQUENCE_LISTS
    case iek_comment: {
    }
	break;
#endif /* COMMENTS_IN_SOURCE_SEQUENCE_LISTS */

    case iek_instantiation_directive: {
    }
	break;
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */

#if SCOPE_ORPHANED_LIST_PROCESSING_NEEDED
    case iek_scope_orphaned_list_header: {
    }
	break;
#endif /* SCOPE_ORPHANED_LIST_PROCESSING_NEEDED */

#if RECORD_HIDDEN_NAMES_IN_IL
    case iek_hidden_name: {
    }
	break;
#endif /* RECORD_HIDDEN_NAMES_IN_IL */

    case iek_pragma: {
    }
	break;

    case iek_template: {
	a_template_ptr template = (a_template_ptr) entry_ptr;
	unsigned long template_symid;
	a_symbol_ptr sym = (a_symbol_ptr) template->
		source_corresp.assoc_info;
	a_template_symbol_supplement_ptr template_info;
	a_type_kind kw_kind = tk_class;

	a_boolean is_canonical = (template == template->canonical_template);
	a_template_ptr definition_template = template->canonical_template != NULL
	    ? template->canonical_template->definition_template
	    : NULL;
	a_boolean is_definition = (definition_template == template);

	if (is_definition && sym == NULL) {
	    sym = (a_symbol_ptr)template->canonical_template
					->source_corresp.assoc_info;
	}
	if( sym == NULL )
	{
	    /*
	     * The parser did not consider this template as 'interesting',
	     * probably because it is a forward declaration for an already
	     * declared template. Previously to EDG 2.43, it used to 
	     * report it anyway, but now this causes a crash, since sym
	     * is used further down in this code without checking its 
	     * value. This means that we can safely 'break' in this case,
	     * since sym was probably never NULL before (otherwise, of 
	     * course, we would've had a crash)
	     */
	    if (is_definition) {
		worry("Expected symbol for template definition.");
	    }
	    if (definition_template == NULL && is_canonical) {
		worry("Expected symbol for canonical template declaration.");
	    }		    
	    break;
	}


	if (ast_info_of(template)) {
	    /* Must be a member template, already processed */
	    break;
	}

	if (template->kind == templk_class ||
	    template->kind == templk_member_class) {
	    if (template->kind == templk_class) {
		template_info = (sym) ? sym->variant.template_info :
			NULL;
		if (template_info) {
		    kw_kind = template_info->variant.class_template.type_kind;
		}
	    }
	    else kw_kind = sym->variant.class_struct_union.type->kind;
	}

	/* Write AST for template definition/declaration, and handle members. */
	if (template->source_corresp.decl_pos_info &&
	    (template->source_corresp.decl_pos_info->identifier_range.start.seq ||
	     template->source_corresp.decl_pos_info->specifiers_range.start.seq)) {
	    ast_node_info_ptr decl = new_ast_node_info(template);
	    ast_node_info_ptr tpl = new_ast_node_info(NULL);
	    ast_node_info_ptr tpl_header = new_ast_node_info(NULL);
	    ast_node_info_ptr decl_id = new_ast_node_info(NULL);
	    ast_node_info_ptr body_or_init;
	    a_source_position_ptr end_pos;
	    if (template->definition_range.end.seq) {
		end_pos = &template->definition_range.end;
	    }
	    else if (template->source_corresp.decl_pos_info->
		     variant.declarator_range.end.seq) {
		end_pos = &template->source_corresp.decl_pos_info->
			variant.declarator_range.end;
	    }
	    else end_pos = &template->source_corresp.decl_pos_info->
		    specifiers_range.end;
	    add_symid_to_string(SET_symid_and_add_to_ast(template, iek_template, decl), &string_buf);
	    add_pos_to_ast(&template->source_corresp.decl_position,
			   end_pos,
			   decl);
	    decl->text = terminate_string(&string_buf);
	    tpl_header->node_name = NN_template_header;
	    relink(&tpl->first_child, tpl_header);
	    relink(&decl->first_child, tpl);
	    if (GNU_compatible_AST) {
		decl->suppress_node = TRUE;
	    }
	    add_symid_to_string(SET_symid_and_add_to_ast(template, iek_template, decl_id), &string_buf);
	    add_range_to_ast(&template->source_corresp.decl_pos_info->
			     identifier_range, decl_id);
	    decl_id->text = terminate_string(&string_buf);
	    if (template->definition_range.start.seq) {
		body_or_init = new_ast_node_info(NULL);
		add_range_to_ast(&template->definition_range, body_or_init);
		body_or_init->text = terminate_string(&string_buf);
	    }
	    else body_or_init = NULL;
	    if (template->kind == templk_class ||
		template->kind == templk_member_class) {
		decl->node_name = NN_declaration_no_dclrtr;
		if (kw_kind == tk_struct) {
		    tpl->node_name = NN_template_struct;
		    decl_id->node_name = NN_decl_id_strct;
		}
		else if (kw_kind == tk_union) {
		    tpl->node_name = NN_template_union;
		    decl_id->node_name = NN_decl_id_un;
		}
		else {
		    tpl->node_name = NN_template_class;
		    decl_id->node_name = NN_decl_id_cls;
		}
		if (body_or_init) {
		    add_to_string_with_len(":def", 4, &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(template, iek_template, tpl), &string_buf);
		add_pos_to_ast(&template->source_corresp.decl_position,
			       end_pos,
			       tpl);
		tpl->text = terminate_string(&string_buf);
		if (template->source_corresp.decl_pos_info->
				  identifier_range.end.seq != 0) {
		    /* Use end of identifier as end of template header, for lack
		     * of something better. */
		    add_pos_to_ast(&template->source_corresp.decl_position,
				   &template->source_corresp.decl_pos_info->
				   identifier_range.end, tpl_header);
		}
		else {
		    /* a poor fallback */
		    add_pos_to_ast(&template->source_corresp.decl_position,
				   &template->source_corresp.decl_pos_info->
				   specifiers_range.end, tpl_header);
		}
		tpl_header->text = terminate_string(&string_buf);
		relink(&tpl_header->first_child, decl_id);
		if (body_or_init) {
		    body_or_init->node_name = NN_template_body;
		    relink(&tpl_header->next_sibling, body_or_init);
		}
	    }
	    else if (template->kind == templk_function ||
		     template->kind == templk_member_function) {
		ast_node_info_ptr specs = new_ast_node_info(NULL);
		ast_node_info_ptr dclrtr = new_ast_node_info(NULL);
		decl->node_name = NN_declaration_with_dclrtr;
		tpl->node_name = NN_template_fcn;
		decl_id->node_name = NN_decl_id_fcn;
		if (body_or_init) {
		    add_to_string_with_len(":def", 4, &string_buf);
		}
		add_symid_to_string(SET_symid_and_add_to_ast(template, iek_template, tpl), &string_buf);
		add_pos_to_ast(&template->source_corresp.decl_position,
			       end_pos, tpl);
		tpl->text = terminate_string(&string_buf);
		add_pos_to_ast(&template->source_corresp.decl_position,
			       &template->source_corresp.decl_pos_info->
			       variant.declarator_range.end, tpl_header);
		tpl_header->text = terminate_string(&string_buf);
		add_range_to_ast(&template->source_corresp.decl_pos_info->
				 specifiers_range, specs);
		specs->text = terminate_string(&string_buf);
		specs->node_name = NN_decl_specs;
		add_range_to_ast(&template->source_corresp.decl_pos_info->
				 variant.declarator_range, dclrtr);
		dclrtr->text = terminate_string(&string_buf);
		dclrtr->node_name = NN_declarator_fcn;
		relink(&dclrtr->first_child, decl_id);
		relink(&specs->next_sibling, dclrtr);
		relink(&tpl_header->first_child, specs);
		if (body_or_init) {
		    body_or_init->node_name = NN_template_body;
		    relink(&tpl_header->next_sibling, body_or_init);
		}
	    }
	    else if (template->kind == templk_static_data_member) {
		ast_node_info_ptr specs = new_ast_node_info(NULL);
		ast_node_info_ptr dclrtr = new_ast_node_info(NULL);
		decl->node_name = NN_declaration_with_dclrtr;
		tpl->node_name = NN_template_static_data_mbr;
		decl_id->node_name = NN_decl_id_var;
		add_to_string_with_len(":def", 4, &string_buf);
		add_symid_to_string(SET_symid_and_add_to_ast(template, iek_template, tpl), &string_buf);
		add_pos_to_ast(&template->source_corresp.decl_position,
			       end_pos, tpl);
		tpl->text = terminate_string(&string_buf);
		add_pos_to_ast(&template->source_corresp.decl_position,
			       end_pos, tpl_header);
		tpl_header->text = terminate_string(&string_buf);
		add_range_to_ast(&template->source_corresp.decl_pos_info->
				 specifiers_range, specs);
		specs->text = terminate_string(&string_buf);
		specs->node_name = NN_decl_specs;
		if (body_or_init) {
		    add_pos_to_ast(&template->source_corresp.decl_pos_info->
				   variant.declarator_range.start,
				   &template->definition_range.end,
				   dclrtr);
		}
		else add_pos_to_ast(&template->source_corresp.decl_pos_info->
				    variant.declarator_range.start, 
				    &template->source_corresp.decl_pos_info->
				    variant.declarator_range.end, dclrtr);
		dclrtr->text = terminate_string(&string_buf);
		dclrtr->node_name = NN_declarator_var;
		if (body_or_init) {
		    body_or_init->node_name = NN_init;
		    if (GNU_compatible_AST) {
			body_or_init->suppress_node = TRUE;
		    }
		    relink(&decl_id->next_sibling, body_or_init);
		}
		relink(&dclrtr->first_child, decl_id);
		relink(&specs->next_sibling, dclrtr);
		relink(&tpl_header->first_child, specs);
	    }
	    else {
		/* This is probably a source error, don't put anything out */
		decl->suppress_subtree = TRUE;
		decl_id->suppress_subtree = TRUE;
		if (body_or_init) {
		    body_or_init->suppress_subtree = TRUE;
		}
	    }
	}
    }
	break;

#if RECORD_MACROS_IN_IL
    case iek_macro:
	break;
#endif /* RECORD_MACROS_IN_IL */

#if ONE_INSTANTIATION_PER_OBJECT
    case iek_per_instantiation_needed_flags_entry:
	break;
#endif /* ONE_INSTANTIATION_PER_OBJECT */
#if EXTRA_SOURCE_POSITIONS_IN_IL
    case iek_decl_position_supplement:
	break;
#endif /* EXTRA_SOURCE_POSITIONS_IN_IL */
#if PROTOTYPE_INSTANTIATIONS_IN_IL
    case iek_template_decl:
	break;
    case iek_template_parameter:
	break;
#endif /* PROTOTYPE_INSTANTIATIONS_IN_IL */
    case iek_seq_number_lookup_entry:  /* XXX: default OK? */
        break;
    case iek_macro_invocation_record_block:
        /* XXX: move register_macro_call here? */
        break;

    default:
	complain_int(error_csev, "Unsupported entry kind $1", entry_kind);
	break;
    } /* switch */
#if DEBUG
    if (db_ast) {
	astp = ast_info_of(entry_ptr);
	if (astp != NULL) {
	    printf("Done processing %s entry %lx, ast %s.\n",
		il_entry_kind_names[entry_kind], (long)entry_ptr,
		db_ast_text(astp));
	}
    }
#endif
} /* process_ast_of_entry */

static void link_this_declarator_ast(a_scope_ptr ptr)
{
    /* link in ast for "this" here, since it doesn't appear
     * in the source sequence list.
     */
    ast_node_info_ptr parm_decl = new_ast_node_info(NULL);
    ast_node_info_ptr specs = new_ast_node_info(NULL);
    ast_node_info_ptr dclrtr =
	    new_ast_node_info(ptr->variant.routine.this_param_variable);
    dclrtr->node_name = NN_variable;
    add_symid_to_string(SET_symid_and_add_to_ast(ptr->variant.routine.this_param_variable, iek_variable, dclrtr),
			&string_buf);
    dclrtr->text = terminate_string(&string_buf);
    specs->node_name = NN_decl_specs;
    specs->text = "";
    relink(&specs->next_sibling, dclrtr);
    parm_decl->node_name = NN_parm_decl;
    add_3_chars_to_string(":cg", &string_buf);
    add_symid_to_string(SET_symid_and_add_to_ast(ptr->variant.routine.this_param_variable, iek_variable, parm_decl),
			&string_buf);
    parm_decl->text = terminate_string(&string_buf);
    relink(&parm_decl->first_child, specs);
    relink(fcn_parm_link, parm_decl);
    fcn_parm_link = &parm_decl->next_sibling;
}

void ast_notify_entering_scope(a_scope_ptr ptr)
{
    if (interesting_node(curr_fcn) &&
	ast_info_of(curr_fcn)) {
	/* trivial default ctor has scope but not routine
	 * IL entry; no arguments to worry about, so just
	 * ignore the declarator and parm link.
	 */
	a_type_ptr basetype;
	a_boolean is_fcn;
	fcn_declarator = ast_info_of(curr_fcn);
	check_for_fcn_basetype(curr_fcn->type, &basetype, &is_fcn);
	if (!is_fcn) {
	    fcn_parm_link = NULL;
	}
	else if (!basetype->variant.routine.extra_info->prototyped &&
	    !GNU_compatible_AST) {
	    /* K&R-style definition, parameter declarations are
	     * linked as siblings to the function declarator, not
	     * children.
	     */
	    fcn_parm_link = &fcn_declarator->next_sibling;
	}
	else fcn_parm_link = &fcn_declarator->first_child->next_sibling;
    } 
    else {
	fcn_parm_link = NULL; 
    }
    if (fcn_parm_link && ptr->variant.routine.this_param_variable) {
	link_this_declarator_ast(ptr);
    }
}

void validate_ast_structures() {
    unsigned i;
    if (sizeof(node_name_str_gnu) != NN_NUMBER_OF_NAMES * sizeof(const char*) ||
	sizeof(node_name_str) != NN_NUMBER_OF_NAMES * sizeof(const char*)) {
	complain(error_csev, "AST node name lists are inconsistent.");
    }
    if (sizeof(expr_operand)/sizeof(expr_operand[0]) != eok_last) {
	complain(error_csev, "Operand address table is inconsistent.");
    }
    for (i = 0; i < sizeof(expr_operand)/sizeof(expr_operand[0]); ++i) {
        if (i != expr_operand[i].code) {
            complain(error_csev, "Operand address table is inconsistent.");
        }
    }
}

void check_ast_consistent() {
    if (pending_links) {
	complain(error_csev, "Pending links left over.");
    }
    if (top_of_stmt_stack != NULL) {
	complain(error_csev, "Statements stranded.");
    }
}
