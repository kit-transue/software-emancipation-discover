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
/* Software Emancipation Technology code to dump the Java IL. */

#include "basic_hdrs.h"
#include "fe_types.h"
#include "util.h"
#include "fe_util.h"
#include "jil_def.h"
#include "cmd_line.h"
#include <stdio.h>
#include "jil_util.h"

#include "SET_IF.h"
#include "SET_strings.h"
#include "SET_IF_fmt.h"
#include "SET_checksum.h"
#include "SET_jfe_msg.h"

#define MAX_DIR_SIZE 1024

/* GLOBAL STATIC DATA */

static a_compilation_unit_ptr current_compilation_unit;
static unsigned long next_symid = 1;
static unsigned long current_compilation_unit_symid;
static a_boolean current_compilation_unit_is_source;
static unsigned long current_package_symid;
static unsigned long current_named_block_symid;
static a_file_list_entry_ptr current_cmd_line_file;
static a_boolean in_initialization_block;
static int cg_level = 0;
static const char *defining_file_name;
static a_cu_info_ptr origin_cu_list = NULL;
static char current_working_dir[MAX_DIR_SIZE];

/*Go through the linked list of origin_cu_info (origin_cu_list) to find a 
 *compilation_unit with the specified file name. Create a new one if is does 
 *not exist and add to the beginning of the list, otherwise return the 
 *existing one.
 */
a_cu_info_ptr create_a_cu_info_for(char *filename)
{
    a_cu_info_ptr ocp = origin_cu_list;
    while(ocp) {
        if (ocp->cup && strcmp(ocp->cup->file_name, filename) == 0) {
            break;
        }
        ocp = ocp->next;
    }
    if (!ocp) {
        a_compilation_unit_ptr new_cup = create_compilation_unit(filename, cuk_stub_file);
        /* create new a_cu_info and initialize values*/
        ocp = (a_cu_info_ptr)malloc(sizeof(a_cu_info));
        ocp->cup = new_cup;
        ocp->symid = 0;
        ocp->next = NULL;

        if (!origin_cu_list) {
            origin_cu_list = ocp;
        } else {
            ocp->next = origin_cu_list;
            origin_cu_list = ocp; 
        }
    }
    return ocp;
}

static const char* declaration_kind_name[] = {
    "package",
    "class",
    "interface",
    "type",
    "function",
    "initialization_block",
    "variable",
    "label",
    "overload_set",	/* never used */
    "unknown"		/* never used */
};
 
/* TREE WALKING ROUTINES */

static void walk_scope(a_scope_ptr p, a_boolean do_syms, a_boolean do_smts);
static void walk_declaration(a_declaration_ptr p);
static void walk_name_reference_for_sym(a_name_reference_ptr p);
static void walk_name_reference_for_smt(a_name_reference_ptr p);
static void walk_package(a_package_ptr p);
static void walk_reference_type(a_reference_type_ptr p);
static void walk_method(a_method_ptr p);
static void walk_initialization_block(an_initialization_block_ptr p);
static void walk_variable(a_variable_ptr p);
static void walk_label(a_label_ptr p);
static void walk_overload_set(an_overload_set_ptr p);
static void walk_statement(a_statement_ptr p, a_boolean for_initialization_block, a_boolean option);
static void walk_variable_initializer(a_variable_initializer_ptr p);
static void walk_expr(an_expr_ptr p);
static void walk_switch_block_statement_group(a_switch_block_statement_group_ptr p);
static void walk_catch_clause(a_catch_clause_ptr p);
static void walk_constant(a_constant_ptr p);
static void walk_literal(a_literal_ptr p);
static void walk_switch_label(a_switch_label_ptr p);
static void walk_compilation_unit(a_compilation_unit_ptr p);
static void walk_import_declaration(an_import_declaration_ptr p);
static void walk_all(void);

/* STRING and OUTPUT ROUTINES */

static void write_smt_line(unsigned long symid, a_source_locator_ptr loc);
static const char *get_declaration_kind_name(a_declaration_ptr decl);
static unsigned char get_declaration_kind_node_name(a_declaration_ptr decl);
static void write_sym_for_declaration(a_declaration_ptr decl);
static void write_sym_for_foreign_declaration(a_declaration_ptr decl);
static void write_class_rels_and_attribs(a_reference_type_ptr rtp);
static void write_foreign_declaration_attribs(a_declaration_ptr decl);
static void write_declaration_attribs(a_declaration_ptr decl);
static a_compilation_unit_ptr declaration_is_native(a_declaration_ptr decl);
static void write_stub_ast(const char *file_name);
static a_boolean write_ast_for_constant(an_expr_ptr p,
				        unsigned char nn,
				        a_source_locator *loc,
    	    	    	    	    	a_boolean force);
static void insert_sym_prefix(const char* sym_kind, unsigned long symid,
			      a_boolean compiler_generated);
static a_scope_ptr add_qualified_name_to_string(a_declaration_ptr dp, string_buffer_ptr bp);
static void add_unicode_char_to_string(a_unicode_char, char, string_buffer_ptr);
static void add_unicode_str_to_string(a_unicode_string ustring, size_t length,
    	    	    	    	      char quote,
                                      a_boolean add_quotes, string_buffer_ptr buf);
static a_boolean add_source_pos_to_string(a_source_locator *location, string_buffer_ptr buf);
static void add_source_pos_to_ast(a_source_locator *location);
static void add_method_sig_to_string(a_method_ptr meth, string_buffer_ptr bp);
static void add_native_mangled_name_to_string(a_method_ptr meth,
                                              string_buffer_ptr bp);
static void add_type_name_to_string(a_type_ptr tp, a_boolean add_quotes,
				    string_buffer_ptr bp);
static void add_modifier_attribs(a_modifier_set mods, a_boolean* atr_added,
				 string_buffer_ptr bp);
static void write_context_relation(a_declaration_ptr p, a_modifier_set mods,
                                   unsigned long context_symid);
static unsigned long write_function_type_of_method(a_method_ptr p);
static unsigned long error_type_symid();
static void add_name_decoration_to_string(a_scope_ptr scope_qual,
    	    	    	    	    	  string_buffer_ptr buf);
static void update_current_file(a_compilation_unit_ptr p);


/* AST SUPPORT */

static unsigned short ast_nesting_depth;

typedef struct ast_node* ast_node_ptr;

typedef struct ast_node {
    const char* text;
    unsigned short depth;
    unsigned char name;
    int leaf:1;
} ast_node;

typedef enum an_ast_node_name {
    NN_AST,
    NN_addr_expr,
    NN_alshift_assign_expr,
    NN_alshift_expr,
    NN_anonymous_class,
    NN_array_ref,
    NN_arshift_assign_expr,
    NN_arshift_expr,
    NN_assign_expr,
    NN_ast_block,
    NN_ast_declspec,
    NN_ast_root,
    NN_ast_superclass,
    NN_bit_and_assign_expr,
    NN_bit_and_expr,
    NN_bit_ior_assign_expr,
    NN_bit_ior_expr,
    NN_bit_not_expr,
    NN_bit_xor_assign_expr,
    NN_bit_xor_expr,
    NN_block_assign,
    NN_call_expr,
    NN_case_label,
    NN_case_stmt,
    NN_case_values,
    NN_catch,
    NN_class,
    NN_class_literal,
    NN_component_ref,
    NN_cond_expr,
    NN_condition,
    NN_constant,
    NN_convert_expr,
    NN_ctor_init,
    NN_default,
    NN_dimensions,
    NN_do_while_stmt,
    NN_dynamic_init,
    NN_easy_new,
    NN_eq_expr,
    NN_error,
    NN_exit_stmt,
    NN_expr_stmt,
    NN_expression,
    NN_field,
    NN_field_decl,
    NN_finally,
    NN_for_stmt,
    NN_function,
    NN_function_decl,
    NN_function_def,
    NN_ge_expr,
    NN_gt_expr,
    NN_if_stmt,
    NN_implements,
    NN_indirect_ref,
    NN_init,
    NN_initialization_block,
    NN_instanceof,
    NN_interface,
    NN_java_vec_new,
    NN_label,
    NN_label_decl,
    NN_le_expr,
    NN_list_decl,
    NN_lrshift_expr,
    NN_lrshift_assign_expr,
    NN_lt_expr,
    NN_minus_assign_expr,
    NN_minus_expr,
    NN_mult_assign_expr,
    NN_mult_expr,
    NN_name_clause,
    NN_ne_expr,
    NN_negate_expr,
    NN_no_operand,
    NN_null,
    NN_nw_expr,
    NN_object_lifetime,
    NN_operation,
    NN_package,
    NN_parens,
    NN_parm_decl,
    NN_plus_assign_expr,
    NN_plus_expr,
    NN_postdecrement_expr,
    NN_postincrement_expr,
    NN_predecrement_expr,
    NN_preincrement_expr,
    NN_rdiv_assign_expr,
    NN_rdiv_expr,
    NN_return_stmt,
    NN_set_vla_size,
    NN_statement,
    NN_string,
    NN_struct,
    NN_super,
    NN_switch_clause,
    NN_synchronized,
    NN_temp_init,
    NN_this,
    NN_throw_expr,
    NN_throw_spec,
    NN_trunc_div_assign_expr,
    NN_trunc_div_expr,
    NN_trunc_mod_assign_expr,
    NN_trunc_mod_expr,
    NN_truth_andif_expr,
    NN_truth_not_expr,
    NN_truth_orif_expr,
    NN_try_block,
    NN_type,
    NN_type_decl,
    NN_type_decl_modified,
    NN_typespec,
    NN_unary_plus,
    NN_unknown_node,
    NN_used_entity,
    NN_using,
    NN_using_decl,
    NN_var_decl,
    NN_variable,
    NN_vec_nw_expr,
    NN_virtual_function_ptr,
    NN_vla_decl,
    NN_while_stmt
} an_ast_node_name;

static unsigned char declaration_kind_node_name[] = {
    NN_package,
    NN_class,
    NN_interface,
    NN_unknown_node,	/* array has no corresponding node name */
    NN_function,
    NN_initialization_block,
    NN_variable,
    NN_label,
    NN_unknown_node,	/* overload_set never used */
    NN_unknown_node	/* unknown never used */
};

static ast_node_ptr new_ast_node(an_ast_node_name name);

static an_ast_node_name op_node_name[] = {
    NN_error,			/* eok_error */
    NN_parens,			/* eok_parens */
    NN_array_ref,		/* eok_array_access */
    NN_postincrement_expr,	/* eok_post_incr */
    NN_postdecrement_expr,	/* eok_post_decr */
    NN_preincrement_expr,	/* eok_pre_incr */
    NN_predecrement_expr,	/* eok_pre_decr */
    NN_unary_plus,		/* eok_unary_plus */
    NN_negate_expr,		/* eok_negate */
    NN_bit_not_expr,		/* eok_complement */
    NN_truth_not_expr,		/* eok_not */
    NN_mult_expr,		/* eok_multiply */
    NN_trunc_div_expr,		/* eok_divide (int; rdiv_expr for FP) */
    NN_trunc_mod_expr,		/* eok_remainder */
    NN_plus_expr,		/* eok_add */
    NN_minus_expr,		/* eok_subtract */
    NN_plus_expr,		/* eok_concat */
    NN_alshift_expr,		/* eok_lshift */
    NN_arshift_expr,		/* eok_signed_rshift */
    NN_lrshift_expr,		/* eok_unsigned_rshift */
    NN_eq_expr,			/* eok_eq */
    NN_ne_expr,			/* eok_ne */
    NN_gt_expr,			/* eok_gt */
    NN_lt_expr,			/* eok_lt */
    NN_ge_expr,			/* eok_ge */
    NN_le_expr,			/* eok_le */
    NN_bit_and_expr,		/* eok_and */
    NN_bit_ior_expr,		/* eok_or */
    NN_bit_xor_expr,		/* eok_xor */
    NN_truth_andif_expr,	/* eok_land */
    NN_truth_orif_expr,		/* eok_lor */
    NN_cond_expr,		/* eok_question */
    NN_assign_expr,		/* eok_assign */
    NN_mult_assign_expr,	/* eok_multiply_assign */
    NN_trunc_div_assign_expr,	/* eok_divide_assign (int; rdiv_assign_expr for FP) */
    NN_trunc_mod_assign_expr,	/* eok_remainder_assign */
    NN_plus_assign_expr,	/* eok_add_assign */
    NN_minus_assign_expr,	/* eok_subtract_assign */
    NN_plus_assign_expr,	/* eok_concat_assign */
    NN_alshift_assign_expr,	/* eok_lshift_assign */
    NN_arshift_assign_expr,	/* eok_signed_rshift_assign */
    NN_lrshift_assign_expr,	/* eok_unsigned_rshift_assign */
    NN_bit_and_assign_expr,	/* eok_and_assign */
    NN_bit_ior_assign_expr,	/* eok_or_assign */
    NN_bit_xor_assign_expr	/* eok_xor_assign */
};

/* MISCELLANEOUS UTILITIES */

static a_boolean auto_sym_upon_assign_symid(a_declaration_ptr);
static unsigned long assign_symid(void* entity);
static void write_sym_for_entity(void* entity);
static void modifier_source_range(a_parse_modifier_ptr mods,
				  a_source_locator_ptr range);
static void name_reference_source_range(a_name_reference_component_ptr comp,
					a_source_locator_ptr range);
static void extend_pos_left(a_source_locator_ptr master,
			    a_source_locator_ptr test);
static void extend_pos_right(a_source_locator_ptr master,
			     a_source_locator_ptr test);
static void extend_pos_for_declarator(a_source_locator_ptr master,
				      a_variable_ptr);
static void extend_pos_right_for_expr(a_source_locator_ptr master,
				      an_expr_ptr);
static a_variable_ptr last_declarator_in_decl(a_variable_ptr);
static a_variable_ptr next_declarator_in_decl(a_variable_ptr);
static a_declaration_ptr context_of(a_declaration_ptr);
    
#define EMIT_COMPILER_GENERATED_METHODS 1

#define start_pos_is_less(a,b) \
    ((a).start_sequence < (b).start_sequence || \
     ((a).start_sequence == (b).start_sequence && \
      (a).start_column < (b).start_column))

#define end_pos_is_greater(a,b) \
    ((a).end_sequence > (b).end_sequence || \
     ((a).end_sequence == (b).end_sequence && \
      (a).end_column > (b).end_column))

/* Routine implementations. */

/* Get the entity's symid for emission to the IF file.  As a side effect,
 * assigns symid and emits SYM record to the IF file as needed.
 */
#define symid_of(p) ( \
    /* if */ (p) ? \
        /* then if */ ((a_constant_ptr)(p))->SET_info.symid ? \
            /* then */ (check_defn_of(p), \
                       ((a_constant_ptr)(p))->SET_info.symid) : \
        /* else */ assign_symid(p) : \
    /* else */ 0)

/* Return the entity's symid (no side effects). */
#define inspect_symid_of(p) ((p) ? ((a_constant_ptr)(p))->SET_info.symid : 0)

#define set_symid_of(p,v) (((a_constant_ptr)(p))->SET_info.symid = (v))

/* Checks for a symbol whose SYM record was deferred, emitting as needed. */
#define check_defn_of(p) ( \
  ((a_constant_ptr)(p))->SET_info.file_where_to_define \
                                                == current_cmd_line_file \
  ? (write_sym_for_entity(p), 0) \
  : 0)

/* Generally, we need to emit SYM nodes before SMT.  Walking a scope
 * does not necessarily emit all of the SMT that references symbols,
 * because the statements of a block scope are processed in elsewhere.
 * In that case, we need to break the scope walking up into 2 parts.
 */
static void walk_scope(a_scope_ptr p, a_boolean do_syms, a_boolean do_smts) {
    a_declaration_ptr dp;
    a_name_reference_ptr rp;
    if (do_syms) {
	/* It's necessary to do the references first; otherwise, the
	 * symids will get filled in by the expressions and such and we won't
	 * know that we have to do SYM lines for references to foreign
	 * entities (the check is only done when walking name references to
	 * minimize overhead; expressions and such just use the symid
	 * blindly).
	 */
	for (rp = p->references; rp; rp = rp->next) {
	    walk_name_reference_for_sym(rp);
	    if (rp == p->last_reference) {
		break;
	    }
	}
	if (p->kind != sk_block) {
	    /* Don't do declarations for block scopes; the declarations can
	     * be spread around among other statements, so the only way to
	     * get the AST in the correct order is to handle them as statements,
	     * not here.
	     */
	    for (dp = p->declarations; dp; dp = dp->next) {
		walk_declaration(dp);
		if (dp == p->last_declaration) {
		    break;
		}
	    }
	}
    }
    if (do_smts) {
	/* It's also necessary to do the references (SMT) last;
	 * otherwise, the symbols declared in the declarations won't have
	 * their SYM nodes emitted in time.
	 */
	for (rp = p->references; rp; rp = rp->next) {
	    walk_name_reference_for_smt(rp);
	    if (rp == p->last_reference) {
		break;
	    }
	}
    }
}

static void walk_declaration(a_declaration_ptr p) {
    if (p->kind == dk_initialization_block || p->kind == dk_overload_set) {
    }
    else {
    	/* Force SYM record out. */
    	unsigned long symid = symid_of(p);
    	a_boolean comp_gen_method = (   p->kind == dk_method
                                     && ((a_method_ptr) p)->compiler_generated);
	if (!comp_gen_method && current_compilation_unit_is_source) {
	    write_smt_line(symid_of(p), &p->location);
	}
    }

    switch (p->kind) {
    case dk_package:
	walk_package((a_package_ptr) p);
	break;
    case dk_class:
    case dk_interface:
    case dk_array:
	walk_reference_type((a_reference_type_ptr) p);
	break;
    case dk_method:
	walk_method((a_method_ptr) p);
	break;
    case dk_initialization_block:
	walk_initialization_block((an_initialization_block_ptr) p);
	break;
    case dk_variable:
	walk_variable((a_variable_ptr) p);
	break;
    case dk_label:
	walk_label((a_label_ptr) p);
	break;
    case dk_overload_set:
	walk_overload_set((an_overload_set_ptr) p);
	break;
    default:
	break;
    }
}

/* Note: if the name reference refers to an undeclared type, e.g. int,
 * then get_referenced will return a_type_ptr cast to a_declaration_ptr.
 * It's up to the caller to test a_declaration.SET_info.kind.
 */
static a_declaration_ptr get_referenced(a_name_reference_ptr ref) {
    a_declaration_ptr decl = NULL;
    switch (ref->kind) {
    case rk_package_reference:
	decl = (a_declaration_ptr) ref->ref.package_ref->package;
	break;
    case rk_type_reference:
    case rk_type_reference_array:
	{
	    a_type_ptr type_targ = ref->ref.type_ref->type;
	    if (type_targ && is_reference_type_reference(ref->ref.type_ref)) {
		decl = (a_declaration_ptr) type_targ->reference_type;
	    }
	    else {
		decl = (a_declaration_ptr)type_targ;
	    }
	}
	break;
    case rk_class_reference:
    case rk_interface_reference:
    case rk_class_or_interface_reference:
	decl = (a_declaration_ptr) ref->ref.class_or_interface_ref->
		class_or_interface;
	break;
    case rk_import_declaration:
	decl = (a_declaration_ptr) ref->ref.import_decl->decl;
	break;
    case rk_label_reference:
	decl = (a_declaration_ptr) ref->ref.label_ref->label;
	break;
    case rk_expr_variable:
    	{
    	    an_expr_ptr e = ref->ref.expr;
	    while (e != NULL && e->kind == ek_cast || e->kind == ek_constant) {
		if (e->kind == ek_cast) {
		    e = e->variant.cast.operand;
		}
		else if (e->kind == ek_constant) {
		    e = e->variant.constant.expr;
		}
	    }
    	    if (e != NULL && e->kind == ek_variable) {
	        decl = (a_declaration_ptr) e->variant.variable.ptr;
	    }
	    break;
	}
    case rk_expr_method:
    	{
    	    an_expr_ptr e = ref->ref.expr;
    	    if (e->kind == ek_cast) {
    	    	e = e->variant.cast.operand;
	    }
    	    if (e != NULL && e->kind == ek_call) {
		decl = (a_declaration_ptr) e->variant.call.method;
	    }
	    break;
	}
    default:
	complain_int(error_csev, "Unexpected reference kind $1.", ref->kind);
	break;
    }
    return decl;
}

static void walk_name_reference_for_sym(a_name_reference_ptr ref) {
    a_declaration_ptr decl = get_referenced(ref);
    if (decl != NULL && !auto_sym_upon_assign_symid(decl)) {
    	/* Reference types are emitted when first referenced in the IF. */
	if (!inspect_symid_of(decl)) {
	    write_sym_for_foreign_declaration(decl);
	}
    }
}

static void walk_name_reference_for_smt(a_name_reference_ptr ref) {
    a_declaration_ptr decl = get_referenced(ref);
    if (decl == NULL) {
	/* Do nothing. */
    }
    else if (decl->SET_info.kind == etk_declaration) {
	a_name_reference_component_ptr comp = &ref->name;
	a_scope_ptr scope = decl->scope;
	while (comp && decl) {
	    a_source_locator extended_loc;
	    a_source_locator_ptr loc;
	    if (comp->qualifier && !scope) {
		/* Name reference components have higher resolution
		 * than package scopes, i.e., "a.b.c" will have three
		 * name reference components but only one layer of
		 * scope nesting if "a.b.c" is a package name; we
		 * have to synthesize the source location from the
		 * first component to the current one.
		 */
		name_reference_source_range(comp, &extended_loc);
		loc = &extended_loc;
		comp = NULL;
	    }
	    else {
		loc = &comp->location;
		comp = comp->qualifier;
	    }
	    if (   decl->kind != dk_initialization_block
    	    	&& decl->kind != dk_overload_set) {

	        write_smt_line(symid_of(decl), loc);
	    }
	    if (scope) {
		/* Not necessarily a package, but all are declarations: */
		decl = (a_declaration_ptr) scope->assoc.package;
		scope = scope->parent_scope;
	    }
	    else decl = NULL;
	}
    }
    else if (decl->SET_info.kind == etk_type) {
	a_type_ptr type_targ = (a_type_ptr)decl;
	write_smt_line(symid_of(type_targ), &ref->name.location);
    }
}

static void walk_package(a_package_ptr p) {
}

static void walk_reference_type(a_reference_type_ptr p) {
    if (p->decl.kind == dk_array) {
	/* Array types are handled elsewhere */
    }
    else {
	a_class_or_interface_reference_ptr cirp;
	a_reference_type_ptr rp;
	a_reference_type_addendum_ptr rtap = p->parse_info;
	ast_node_ptr np;
	a_source_locator range;
	a_boolean ast_done = FALSE;
	unsigned long saved_symid = current_named_block_symid;
    	unsigned long symid = symid_of(p);
    	a_boolean in_source = current_compilation_unit_is_source;
	current_named_block_symid = symid;
        write_class_rels_and_attribs(p);
    	if (in_source) {
	    np = new_ast_node(NN_type_decl_modified);
	    add_to_string_with_len(":def", 4, &string_buf);
	    add_symid_to_string(symid, &string_buf);
	    add_source_pos_to_ast(&p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* type_decl_modified */
	    if (rtap->modifiers) {
		np = new_ast_node(NN_ast_declspec);
		modifier_source_range(rtap->modifiers, &range);
		add_source_pos_to_ast(&range);
		np->text = terminate_string(&string_buf);
	    }
	    else new_ast_node(NN_no_operand);
	    np = new_ast_node(NN_name_clause);
	    range = p->decl.location;
	    extend_pos_right(&range, &rtap->extends_location);
	    extend_pos_right(&range, &rtap->implements_location);
	    add_source_pos_to_ast(&range);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* name_clause */
	    if (p->decl.kind == dk_class) {
		np = new_ast_node(NN_class);
	    }
	    else np = new_ast_node(NN_interface);
	    add_symid_to_string(symid_of(p), &string_buf);
	    add_source_pos_to_ast(&p->decl.location);
	    np->text = terminate_string(&string_buf);
	}
	for (cirp = p->superclass; cirp; cirp = cirp->next) {
	    rp = cirp->class_or_interface;
	    if (in_source && cirp->name_ref != NULL) {
		if (!ast_done) {
		    ast_done = TRUE;
		    np = new_ast_node(NN_ast_superclass);
		    add_source_pos_to_ast(&rtap->extends_location);
    	    	    np->text = terminate_string(&string_buf);
		    ast_nesting_depth++;	/* ast_superclass */
		}
		np = new_ast_node(NN_class);
		add_symid_to_string(symid_of(rp), &string_buf);
		name_reference_source_range(&cirp->name_ref->name,
					    &range);
		add_source_pos_to_ast(&range);
		np->text = terminate_string(&string_buf);
	    }
	}
	if (ast_done) {
	    ast_nesting_depth--;	/* ast_superclass */
	}
    	ast_done = FALSE;
	for (cirp = p->interfaces; cirp; cirp = cirp->next) {
	    rp = cirp->class_or_interface;
	    if (in_source && cirp->name_ref != NULL) {
		if (!ast_done) {
		    ast_done = TRUE;
    	    	    if (p->decl.kind == dk_class) {
		        np = new_ast_node(NN_implements);
			add_source_pos_to_ast(&rtap->implements_location);
		    }
    	    	    else {
    	    	    	np = new_ast_node(NN_ast_superclass);
			add_source_pos_to_ast(&rtap->extends_location);
		    }
    	    	    np->text = terminate_string(&string_buf);
		    ast_nesting_depth++;	/* implements */
		}
		np = new_ast_node(NN_interface);
		add_symid_to_string(symid_of(rp), &string_buf);
		name_reference_source_range(&cirp->name_ref->name,
					    &range);
		add_source_pos_to_ast(&range);
		np->text = terminate_string(&string_buf);
	    }
	}
	if (ast_done) {
	    ast_nesting_depth--;	/* implements */
	}
	if (in_source) 
    	    ast_nesting_depth--;	/* name_clause */
    	SET_push_main_block();  /* for proper counting of init blocks in nested classes*/
    	if (in_source) {
	    np = new_ast_node(NN_ast_block);
	    add_source_pos_to_ast(&rtap->body_location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* ast_block */
	}
    	if (p->scope != NULL) {
	    walk_scope(p->scope, TRUE, TRUE);
    	}
	current_named_block_symid = saved_symid;
	if (in_source)
    	    ast_nesting_depth--;	/* ast_block */
    	SET_pop_block();
    	if (in_source)
	    ast_nesting_depth--;	/* type_decl_modified */
    }
}

static a_declaration_ptr named_entity_of_scope(a_scope_ptr sp) {
    if (sp != NULL) {
        switch (sp->kind) {
    	case sk_reference_type:
	    {
		a_reference_type_ptr p = sp->assoc.reference_type;
		if (p == NULL) {
		    return NULL;
		}
		return &p->decl;
	    }
	case sk_method:
    	    return sp->assoc.method ? &sp->assoc.method->decl : NULL;
	case sk_package:
    	    return sp->assoc.package ? &sp->assoc.package->decl : NULL;
	}
    }
    return FALSE;
}

/* The following function decorates the names of entities local to a
 * function with the function name and block number, if any; global
 * entities are left unchanged.  It then adds the closing quote.
 *
 * Call this after adding the qualified name, and determining the
 * scope in which the top-level qualifier (or lacking that, the name)
 * is declared.
 */

static void add_name_decoration_to_string(a_scope_ptr scope_qual,
    	    	    	    	    	  string_buffer_ptr buf) {
    while (scope_qual && (   scope_qual->kind == sk_reference_type /* for anonymous class */
			  || scope_qual->kind == sk_method
			  || scope_qual->kind == sk_block)) {
    	a_scope_ptr a_scope = scope_qual;
    	a_declaration_ptr named_entity = NULL;
    	while (a_scope != NULL) {
    	    named_entity = named_entity_of_scope(a_scope);
    	    if (named_entity != NULL) {
    	    	break;
	    }
    	    a_scope = a_scope->parent_scope;
	}
    	if (named_entity == NULL) {
    	    complain(error_csev, "Failed to find name of type, method, or block.");
    	    break;
    	}
    	else {
	    int id = scope_qual->SET_id;
	    add_3_chars_to_string(" @ ", buf);
	    scope_qual = add_qualified_name_to_string(named_entity, buf);
	    if (id != 0) {
		add_num_to_string(id, " {", "}", buf);
	    }
	}
    }
}

/* test for destructor name
 */
static a_boolean has_destructor_name(a_declaration_ptr decl) {
    static int destructor_name_length = 8;
    static a_unicode_char destructor_name[8] = {'f','i','n','a','l','i','z','e'};
    return    decl->name != NULL
	   && identical_unicode_strings(decl->name,
					decl->name_length,
					destructor_name,
					destructor_name_length);
}

/* test for initializer name
 */
static a_boolean has_initializer_name(a_declaration_ptr decl) {
    static int initializer_name_length = 8;
    static a_unicode_char initializer_name[8] = {'<','c','l','i','n','i','t','>'};
    return    decl->name != NULL
	   && identical_unicode_strings(decl->name,
					decl->name_length,
					initializer_name,
					initializer_name_length);
}

static void write_declspec(a_parse_modifier_ptr pmp,
			   a_source_locator *type_pos,
			   a_source_locator *declspec_pos,
			   a_type_ptr type) {
    /* In writing the declspec, checks are performed because errors in the
     * source can cause odd mixtures of information in the jil.
     */
    if (type != NULL && type_pos->end_sequence != 0) {
	declspec_pos->end_sequence = type_pos->end_sequence;
	declspec_pos->end_column = type_pos->end_column;
    }
    else {
	while (pmp && pmp->next) {
	    pmp = pmp->next;
	}
	if (pmp) {
	    declspec_pos->end_sequence = pmp->location.end_sequence;
	    declspec_pos->end_column = pmp->location.end_column;
	}
	else *declspec_pos = null_source_locator;
    }
    if (type != NULL || pmp != NULL) {
	ast_node_ptr np = new_ast_node(NN_ast_declspec);
	add_source_pos_to_ast(declspec_pos);
	np->text = terminate_string(&string_buf);
	if (type != NULL) {
	    ast_nesting_depth++;	/* ast_declspec */
	    np = new_ast_node(NN_typespec);
	    add_symid_to_string(symid_of(type), &string_buf);
	    if (!is_error_type(type)) {
		add_source_pos_to_ast(type_pos);
	    }
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth--;	/* ast_declspec */
	}
    }
    else new_ast_node(NN_no_operand);
}

static void walk_method(a_method_ptr p) {
    a_class_or_interface_reference_ptr cirp;
    unsigned long saved_symid = current_named_block_symid;
    unsigned long symid = symid_of(p);
    current_named_block_symid = symid;
    write_declaration_attribs(&p->decl);
    if (p->return_type) {
    	/* Write return type relation. */
    	unsigned long type_symid;
	format_rel(symid_of(p->return_type->type), "type", symid_of(p));
	add_to_string("return", &output_buf);
	terminate_string(&output_buf);

    	type_symid = write_function_type_of_method(p);
    	if (type_symid > 0) {
    	    write_rel(type_symid, "type", symid_of(p));
	}
    }
    /* Write throw relations. */
    for (cirp = p->throws; cirp; cirp = cirp->next) {
	format_rel(symid_of(p), "throw", symid_of(cirp->class_or_interface));
	add_to_string("declared", &output_buf);
	terminate_string(&output_buf);
    }
    if (p->modifiers & MK_NATIVE) {
        /* Write call relation to C/C++ name. */
        unsigned long c_symid = next_symid++;
    	insert_sym_prefix("function", c_symid, FALSE);
    	add_native_mangled_name_to_string(p, &output_buf);
        add_1_char_to_string('\"', &output_buf);
        terminate_string(&output_buf);

	add_3_chars_to_string("ATR", &output_buf);
	add_symid_to_string(c_symid, &output_buf);
	add_to_string("linkage(c)", &output_buf);
	terminate_string(&output_buf);

        write_rel(symid_of(p), "call", c_symid);
    }
    if (!current_compilation_unit_is_source) {
    	/* Skip AST, etc. */
    }
    else if (p->parse_info == NULL) {
    	complain(error_csev, "Method has null parse_info.");
    }
    else if (!p->compiler_generated || EMIT_COMPILER_GENERATED_METHODS) {
    	/* Write AST, walk scope, and walk statement. */
	a_source_locator range = p->decl.location;
	a_source_locator ret_type_pos;
	a_source_locator declspec_pos;
    	a_source_locator throws_pos;
	ast_node_ptr np;
	a_type_ptr return_type = NULL;
    	if (p->compiler_generated) {
    	    cg_level += 1;
	}
	if (p->return_type) {
	    set_locator_from_type_reference(&ret_type_pos, p->return_type);
    	    return_type = p->return_type->type;
	}
	else ret_type_pos = null_source_locator;
	np = new_ast_node(NN_function_def);
	add_symid_to_string(symid_of(p), &string_buf);
	if (p->parse_info->modifiers) {
	    extend_pos_left(&range, &p->parse_info->modifiers->location);
	}
	else extend_pos_left(&range, &ret_type_pos);
	declspec_pos = range;
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* function_def */

    	write_declspec(p->parse_info->modifiers, &ret_type_pos,
						 &declspec_pos,
						 return_type);

	np = new_ast_node(NN_function_decl);

	/* Mark every declaration as a definition.  Abstract methods and
         * methods of interfaces need a point of definition, so that
         * the SMT_fdef will have a def_dd_of_smt relation, and
         * the "where declared" query will work. */
	add_to_string_with_len(":def", 4, &string_buf);
	if (p->compiler_generated) {
	    add_to_string_with_len(",cg", 3, &string_buf);
	}
	add_symid_to_string(symid_of(p), &string_buf);
	/* position of declarator, including argument list and throws */
	range = p->decl.location;
    	throws_pos = null_source_locator;
    	if (p->parse_info != NULL) {
    	    extend_pos_right(&range, &p->parse_info->parameter_list_location);
	}
    	if (p->throws != NULL) {
    	    if (p->parse_info != NULL) {
    	    	throws_pos = p->parse_info->throws_location;
    	    	extend_pos_right(&range, &throws_pos);
	    }
    	    else {
    	    	throws_pos = null_source_locator;
	    }
	}
	add_source_pos_to_ast(&range);

    	SET_push_main_block();
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* function_decl */

	np = new_ast_node(NN_function);
	add_symid_to_string(symid_of(p), &string_buf);
	add_source_pos_to_ast(&p->decl.location);
	np->text = terminate_string(&string_buf);
	/* scope contains parameter declarations: */
	if (p->scope) {
	    walk_scope(p->scope, TRUE, TRUE);
	}
    	/* Throw clause follows parameter declarations,
    	 * is operand of function_decl in AST. */
    	if (p->throws != NULL) {
    	    np = new_ast_node(NN_throw_spec);
    	    add_source_pos_to_ast(&throws_pos);
    	    np->text = terminate_string(&string_buf);
    	    ast_nesting_depth++;   /* throw_spec */
	    for (cirp = p->throws; cirp; cirp = cirp->next) {
    	    	if (cirp->name_ref == NULL) {
    	    	    /* implicitly generated */
    	    	}
    	    	else {
		    np = new_ast_node(NN_type);
		    add_symid_to_string(symid_of(cirp->class_or_interface),
					&string_buf);
		    add_source_pos_to_ast(&cirp->name_ref->name.location);
		    np->text = terminate_string(&string_buf);
		}
	    }
    	    ast_nesting_depth--;   /* throw_spec */
    	}
	ast_nesting_depth--;	/* function_decl */
	if (p->block) {
	    walk_statement(p->block, FALSE, FALSE);
	}
    	SET_pop_block();
	ast_nesting_depth--;	/* function_def */
    	if (p->compiler_generated) {
    	    cg_level -= 1;
	}
    }
    current_named_block_symid = saved_symid;
}

static void walk_initialization_block(an_initialization_block_ptr p) {
    in_initialization_block = TRUE;
    if (p->block != NULL) {
        walk_statement(p->block, TRUE, FALSE);
    }
    in_initialization_block = FALSE;
}

static void walk_variable(a_variable_ptr p) {
    if (!p->processed) {
	a_source_locator range = p->decl.location;
	a_source_locator declspec_pos;
	a_variable_ptr last_declarator = last_declarator_in_decl(p);
	ast_node_ptr np;
    	a_boolean is_method_parameter = p->is_parameter && p->decl.scope
                                          && p->decl.scope->kind == sk_method;
    	a_boolean is_catch_parameter =  p->is_parameter && p->decl.scope
                                          && p->decl.scope->kind == sk_block;
    	a_boolean is_source = current_compilation_unit_is_source;
    	if (is_source && p->parse_info == NULL) {
    	    complain(error_csev, "Variable has null parse_info.");
    	    is_source = FALSE;
	}
    	if (is_source) {
	    a_source_locator type_pos = null_source_locator;
	    a_type_ptr type = NULL;
	    if (p->type) {
		type = p->type->type;
		if (is_reference_type_reference(p->type)) {
		    a_type_reference_ptr trp = p->type;
		    while (trp && trp->type->kind == tk_array &&
			   trp->variant.name_ref != NULL &&
			   end_pos_is_greater(trp->variant.name_ref->name.location,
					      p->decl.location)) {
			trp = trp->component_type_ref;
		    }
		    if (trp) {
			type = trp->type;
			set_locator_from_type_reference(&type_pos, trp);
		    }
		    else {
			type = NULL;
		    }
		}
		else if (p->type->variant.location != NULL) {
		    type_pos = *p->type->variant.location;
		}
	    }
	    if (is_method_parameter) {
		np = new_ast_node(NN_parm_decl);
	    }
	    else {
		np = new_ast_node(NN_list_decl);
	    }
	    if (p->parse_info->modifiers) {
		extend_pos_left(&range, &p->parse_info->modifiers->location);
	    }
	    else extend_pos_left(&range, &type_pos);
	    declspec_pos = range;
	    add_source_pos_to_ast(&p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* list_decl, etc. */

	    write_declspec(p->parse_info->modifiers, &type_pos,
                           &declspec_pos, type);
	}
	do {
    	    write_declaration_attribs(&p->decl);
	    if (p->type) {
		write_rel(symid_of(p->type->type), "type", symid_of(p));
	    }
    	    if (is_method_parameter) {
    	    	a_declaration_ptr param;
    	    	long param_num = 0;
                for (param = p->decl.scope->declarations; param != NULL; param = param->next) {
    	    	    param_num += 1;
    	    	    if (param == &p->decl) {
    	    	    	format_rel(symid_of(p), "argument", symid_of(p->decl.scope->assoc.method));
    	    	    	add_num_to_string(param_num, "order(", ")", &output_buf);
    	    	    	terminate_string(&output_buf);
    	    	    	break;
		    }
		}
    	    }
    	    if (is_catch_parameter && p->type) {
    	    	/* parameter to a catch block */
		if (current_named_block_symid) {
		    write_rel(current_named_block_symid, "catch",
			      symid_of(p->type->type));
		}
    	    }
    	    if (is_source) {
		if (is_method_parameter) {
		    np = new_ast_node(NN_variable);
		}
		else {
		    a_boolean in_class = p->decl.scope
				      && p->decl.scope->kind == sk_reference_type;
		    if (in_class) {
			np = new_ast_node(NN_field_decl);
		    }
		    else np = new_ast_node(NN_var_decl);
		    add_to_string_with_len(":def", 4, &string_buf);
		    add_symid_to_string(symid_of(p), &string_buf);
		    range = p->decl.location;
		    extend_pos_for_declarator(&range, p);
		    add_source_pos_to_ast(&range);
		    np->text = terminate_string(&string_buf);
		    ast_nesting_depth++;	/* XXX_decl */
		    if (in_class) {
			np = new_ast_node(NN_field);
		    }
		    else np = new_ast_node(NN_variable);
		}
		add_symid_to_string(symid_of(p), &string_buf);
		add_source_pos_to_ast(&p->decl.location);
		np->text = terminate_string(&string_buf);
		if (!is_method_parameter) {
		    if (p->initializer) {
			walk_variable_initializer(p->initializer);
		    }
		    ast_nesting_depth--;	/* XXX_decl */
		}
	    }
	    p->processed = TRUE;
	} while (p = next_declarator_in_decl(p));
    	if (is_source)
	    ast_nesting_depth--;	/* list_decl, etc. */
    }
}

static void walk_label(a_label_ptr p) {
    if (current_compilation_unit_is_source) {
	ast_node_ptr np = new_ast_node(NN_label_decl);
	add_symid_to_string(symid_of(p), &string_buf);
	add_source_pos_to_ast(&p->decl.location);
	np->text = terminate_string(&string_buf);
    }
}

static void walk_overload_set(an_overload_set_ptr p) {
}

/* If 'option' is TRUE then the statement may safely be omitted, e.g. a compiler-
 * generated constructor invocation.
 */
static void walk_statement(a_statement_ptr p, a_boolean for_initialization_block,
    	    	    	    	    	      a_boolean option) {
    a_statement_ptr sp;
    a_declaration_ptr dp;
    a_switch_block_statement_group_ptr sbsgp;
    an_expr_ptr ep;
    a_catch_clause_ptr ccp;
    a_label_ptr lp;
    ast_node_ptr np;
    a_boolean is_main;
    a_boolean is_compiler_generated;
    for (lp = p->label; lp; lp = lp->next_on_statement) {
	/* These are always block-local, so the declaration is not
	 * processed as part of the scope, only here.
	 */
	walk_declaration(&lp->decl);
    }
    switch (p->kind) {
    case stmk_block:
	/* handle references first so foreign references will get SYM lines
	 * (only references, not declarations, are processed in a block
	 * scope).
	 */
    	/* Don't push a block for the main scope of a function, because
    	 * it's already been pushed for the parameters.
    	 */
    	is_main = p->variant.block.scope && p->variant.block.scope->parent_scope
    	    	    && p->variant.block.scope->parent_scope->kind == sk_method;
	if (!is_main) {
    	    SET_push_block();
	}
	if (p->variant.block.scope) {
    	    p->variant.block.scope->SET_id = SET_get_block_num();
	    walk_scope(p->variant.block.scope, TRUE, FALSE);
	}
	np = new_ast_node(for_initialization_block ? NN_initialization_block
                                                   : NN_ast_block);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* ast_block */
	for (sp = p->variant.block.statements; sp; sp = sp->next) {
	    walk_statement(sp, FALSE, TRUE);
	}
	if (p->variant.block.scope) {
	    walk_scope(p->variant.block.scope, FALSE, TRUE);
	}
	ast_nesting_depth--;	/* ast_block */
	if (!is_main) {
	    SET_pop_block();
    	}
	break;
    case stmk_decl:
	for (dp = p->variant.decl.first; dp; dp = dp->next) {
	    walk_declaration(dp);
	    if (dp == p->variant.decl.last) {
		break;
	    }
	}
	break;
    case stmk_expr:
    	ep = p->variant.expr;
    	is_compiler_generated =    ep->kind == ek_expl_constr_invoc
		                && ep->variant.expl_constr_invoc.compiler_generated;
    	if (    is_compiler_generated
             && option
    	     && !EMIT_COMPILER_GENERATED_METHODS) {
    	    /* Omit this compiler-generated constructor invocation. */
    	}
    	else {
    	    if (is_compiler_generated) {
    	    	cg_level += 1;
	    }
	    np = new_ast_node(NN_expr_stmt);
	    add_source_pos_to_ast(&p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* expr_stmt */
	    walk_expr(p->variant.expr);
	    ast_nesting_depth--;	/* expr_stmt */
    	    if (is_compiler_generated) {
    	    	cg_level -= 1;
	    }
        }
	break;
    case stmk_if:
	np = new_ast_node(NN_if_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* if_stmt */
	walk_expr(p->variant.if_stmt.expr);
	if (p->variant.if_stmt.then_statement->kind != stmk_empty) {
	    walk_statement(p->variant.if_stmt.then_statement, FALSE, FALSE);
	}
	else new_ast_node(NN_no_operand);
	if (p->variant.if_stmt.else_statement) {
	    walk_statement(p->variant.if_stmt.else_statement, FALSE, FALSE);
	}
	ast_nesting_depth--;	/* if_stmt */
	break;
    case stmk_switch:
	np = new_ast_node(NN_case_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* case_stmt */
	walk_expr(p->variant.switch_stmt.expr);
	walk_statement(p->variant.switch_stmt.block, FALSE, FALSE);
	np = new_ast_node(NN_ast_block);
	add_source_pos_to_ast(&p->variant.switch_stmt.block->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* ast_block */
	for (sbsgp = p->variant.switch_stmt.groups; sbsgp; sbsgp = sbsgp->next) {
	    walk_switch_block_statement_group(sbsgp);
	}
	ast_nesting_depth--;	/* ast_block */
	ast_nesting_depth--;	/* case_stmt */
	break;
    case stmk_while:
	np = new_ast_node(NN_while_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* while_stmt */
	walk_expr(p->variant.while_stmt.expr);
	walk_statement(p->variant.while_stmt.statement, FALSE, FALSE);
	ast_nesting_depth--;	/* while_stmt */
	break;
    case stmk_do_while:
	np = new_ast_node(NN_do_while_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* do_while_stmt */
	walk_expr(p->variant.while_stmt.expr);
	walk_statement(p->variant.while_stmt.statement, FALSE, FALSE);
	ast_nesting_depth--;	/* do_while_stmt */
	break;
    case stmk_for:
	np = new_ast_node(NN_for_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* for_stmt */
	if (p->variant.for_stmt.for_init_is_decl) {
	    /* do scope first for references */
	    walk_scope(p->variant.for_stmt.init.decl.scope, TRUE, TRUE);
	    walk_statement(p->variant.for_stmt.init.decl.stmt, FALSE, FALSE);
	}
	else if (p->variant.for_stmt.init.exprs) {
	    if (p->variant.for_stmt.init.exprs->next) {
		/* Create artificial expression statement to contain the
		 * list of inits.
		 */
		a_source_locator init_range;
		set_locator_from_expr(&init_range,
                                      p->variant.for_stmt.init.exprs);
		for (ep = p->variant.for_stmt.init.exprs; ep && ep->next;
		     ep = ep->next)
			{ }
		if (ep) {
		    extend_pos_right_for_expr(&init_range, ep);
		}
		np = new_ast_node(NN_expr_stmt);
		add_source_pos_to_ast(&init_range);
		np->text = terminate_string(&string_buf);
		ast_nesting_depth++;	/* expr_stmt */
		for (ep = p->variant.for_stmt.init.exprs; ep; ep = ep->next) {
		    walk_expr(ep);
		}
		ast_nesting_depth--;	/* expr_stmt */
	    }
	    else walk_expr(p->variant.for_stmt.init.exprs);
	}
	else new_ast_node(NN_no_operand);
	if (p->variant.for_stmt.expr) {
	    walk_expr(p->variant.for_stmt.expr);
	}
	else new_ast_node(NN_no_operand);
	if (p->variant.for_stmt.updates) {
	    if (p->variant.for_stmt.updates->next) {
		a_source_locator update_range;
		set_locator_from_expr(&update_range,
                                      p->variant.for_stmt.updates);
		for (ep = p->variant.for_stmt.updates; ep && ep->next;
		     ep = ep->next)
			{ }
		if (ep) {
		    extend_pos_right_for_expr(&update_range, ep);
		}
		np = new_ast_node(NN_expr_stmt);
		add_source_pos_to_ast(&update_range);
		np->text = terminate_string(&string_buf);
		ast_nesting_depth++;	/* expr_stmt */
		for (ep = p->variant.for_stmt.updates; ep; ep = ep->next) {
		    walk_expr(ep);
		}
		ast_nesting_depth--;	/* expr_stmt */
	    }
	    else walk_expr(p->variant.for_stmt.updates);
	}
	else new_ast_node(NN_no_operand);
	walk_statement(p->variant.for_stmt.statement, FALSE, FALSE);
	ast_nesting_depth--;	/* for_stmt */
	break;
    case stmk_break:
    case stmk_continue:
	np = new_ast_node(NN_exit_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	if (p->variant.break_continue.label) {
	    a_source_locator label_pos;
	    ast_nesting_depth++;	/* exit_stmt */
	    np = new_ast_node(NN_label);
	    add_symid_to_string(symid_of(p->variant.break_continue.label->label),
				&string_buf);
	    name_reference_source_range(&p->variant.break_continue.label->
				      name_ref->name, &label_pos);
	    add_source_pos_to_ast(&label_pos);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth--;	/* exit_stmt */
	}
	break;
    case stmk_return:
	np = new_ast_node(NN_return_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	if (p->variant.return_expr) {
	    ast_nesting_depth++;	/* return_stmt */
	    walk_expr(p->variant.return_expr);
	    ast_nesting_depth--;	/* return_stmt */
	}
	break;
    case stmk_throw:
	/* In C++, the expr_stmt includes the semicolon, while
         * the throw_expr does not.  In Java, there's no
         * syntactic entity in between the statement and the
    	 * thrown exception expression.
    	 */
	np = new_ast_node(NN_expr_stmt);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
    	ast_nesting_depth++;    /* expr_stmt */
	np = new_ast_node(NN_throw_expr);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* throw_expr */
	walk_expr(p->variant.throw_expr);
	ast_nesting_depth--;	/* throw_expr */
    	ast_nesting_depth--;    /* expr_stmt */
	if (current_named_block_symid) {
	    format_rel(current_named_block_symid, "throw",
		       symid_of(p->variant.throw_expr->type));
	    add_to_string("actual", &output_buf);
	    terminate_string(&output_buf);
	}
	break;
    case stmk_synchronized:
	np = new_ast_node(NN_synchronized);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* synchronized */
	walk_expr(p->variant.synchronized.expr);
	walk_statement(p->variant.synchronized.block, FALSE, FALSE);
	ast_nesting_depth--;	/* synchronized */
	break;
    case stmk_try:
	np = new_ast_node(NN_try_block);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* try_block */
	walk_statement(p->variant.try_stmt.block, FALSE, FALSE);
	for (ccp = p->variant.try_stmt.catch_clauses; ccp; ccp = ccp->next) {
	    walk_catch_clause(ccp);
	}
	if (p->variant.try_stmt.finally_block) {
	    a_source_locator finally_range = p->variant.try_stmt.finally_location;
	    extend_pos_right(&finally_range, &p->variant.try_stmt.finally_block->
			     location);
	    np = new_ast_node(NN_finally);
	    add_source_pos_to_ast(&finally_range);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* finally */
	    walk_statement(p->variant.try_stmt.finally_block, FALSE, FALSE);
	    ast_nesting_depth--;	/* finally */
	}
	ast_nesting_depth--;	/* try_block */
	break;
    case stmk_error:
	np = new_ast_node(NN_error);
	add_source_pos_to_ast(&p->location);
	np->text = terminate_string(&string_buf);
	break;
    case stmk_empty:
    default:
	break;
    }
}

static void walk_variable_initializer(a_variable_initializer_ptr p) {
    if (p->is_array_initializer) {
	a_variable_initializer_ptr vip;
	ast_node_ptr np;
	np = new_ast_node(NN_ast_block);
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* ast_block */
	for (vip = p->variant.component_inits; vip; vip = vip->next) {
	    walk_variable_initializer(vip);
	}
	ast_nesting_depth--;	/* ast_block */
    }
    else walk_expr(p->variant.expr);
}

/* Return the implemented type of an anonymous class.
 */
static a_type_ptr implemented_of_anonymous(a_reference_type_ptr rtp) {
    if (rtp != NULL) {
    	a_class_or_interface_reference_ptr p = rtp->interfaces;
    	if (p == NULL || p->name_ref == NULL) {
    	    p = rtp->superclass;
    	    if (p != NULL && p->name_ref == NULL) {
    	    	p = NULL;
	    }
	}
    	if (p != NULL) {
    	    return p->class_or_interface->type;
	}
    	
    }
    return 0;
}

static void walk_expr(an_expr_ptr p) {
    static a_source_locator expr_range;
    an_expr_ptr ep;
    a_source_locator range;
    ast_node_ptr np;
    a_reference_type_ptr crtp;
    a_boolean is_anonymous;
    a_boolean is_compiler_generated;
    const char* saved_name;
    unsigned long saved_symid;
    switch(p->kind) {
    case ek_constant:
	if (p->variant.constant.expr) {
	    walk_expr(p->variant.constant.expr);
	}
	else {
    	    write_ast_for_constant(p, NN_constant, p->location, FALSE);
	    walk_constant(p->variant.constant.ptr);
	}
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_variable:
	if (p->variant.variable.object) {
	    np = new_ast_node(NN_component_ref);
	    if (p->type) {
		add_symid_to_string(symid_of(p->type), &string_buf);
	    }
	    add_source_pos_to_ast(p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* component_ref */
	    walk_expr(p->variant.variable.object);
	    np = new_ast_node(NN_field);
	    add_symid_to_string(symid_of(p->variant.variable.ptr),
				&string_buf);
	    name_reference_source_range(&p->variant.variable.name_ref->name,
					&range);
	    add_source_pos_to_ast(&range);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth--;	/* component_ref */
	}
	else {
	    np = new_ast_node(NN_variable);
	    add_symid_to_string(symid_of(p->variant.variable.ptr),
				&string_buf);
	    add_source_pos_to_ast(p->location);
	    np->text = terminate_string(&string_buf);
	}
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_this:
	if (p->variant.this_expr.enclosing_class_ref) {
	    a_type_reference_ptr trp = p->variant.this_expr.enclosing_class_ref;
	    np = new_ast_node(NN_component_ref);
	    if (p->type) {
		add_symid_to_string(symid_of(p->type), &string_buf);
	    }
	    add_source_pos_to_ast(p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* component_ref */
	    np = new_ast_node(NN_class);
	    add_symid_to_string(symid_of(trp->type), &string_buf);
	    name_reference_source_range(&trp->variant.name_ref->name, &range);
	    add_source_pos_to_ast(&range);
	    np->text = terminate_string(&string_buf);
	    np = new_ast_node(NN_this);
	    add_symid_to_string(symid_of(p->type), &string_buf);
	    /* We don't have the position of "this" in the expression,
	     * so we construct it by backing up from the end.
	     */
	    range = *p->location;
	    range.start_sequence = range.end_sequence;
	    range.start_column = range.end_column - 3;
	    add_source_pos_to_ast(&range);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth--;	/* component_ref */
	}
	else {
	    np = new_ast_node(NN_this);
	    add_symid_to_string(symid_of(p->type), &string_buf);
	    add_source_pos_to_ast(p->location);
	    np->text = terminate_string(&string_buf);
	}
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_class_new:
    	is_anonymous = p->variant.class_new.type->variant.name_ref == NULL;
    	if (is_anonymous) {
    	    /* anonymous class */
    	    a_type_ptr ctp = p->variant.class_new.type->type;
    	    crtp = (ctp != NULL) ? ctp->reference_type : NULL;
    	    /* emit SYM */
    	    symid_of(crtp);
    	    write_class_rels_and_attribs(crtp);
	}
	np = new_ast_node(NN_easy_new);
    	if (p->type != NULL) {
	    add_symid_to_string(symid_of(p->type), &string_buf);
	}
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* easy_new */
    	if (is_anonymous) {
    	    np = new_ast_node(NN_anonymous_class);
    	    if (crtp != NULL) {
    	    	add_symid_to_string(symid_of(crtp), &string_buf);
	        add_source_pos_to_ast(&crtp->location);
	    }
	    np->text = terminate_string(&string_buf);
    	    ast_nesting_depth++;    /* anonymous_class */
	}

	np = new_ast_node(NN_call_expr);
	if (p->variant.class_new.type->variant.name_ref &&
	    p->location) {
	    /* construct source range for call: from the type
	     * reference up through the end of the expression.
	     */
	    name_reference_source_range(&p->variant.class_new.type->
					variant.name_ref->name,
					&range);
	    range.end_sequence = p->location->end_sequence;
	    range.end_column = p->location->end_column;
	    add_source_pos_to_ast(&range);
	    np->text = terminate_string(&string_buf);
	}
	ast_nesting_depth++;	/* call_expr */
	np = new_ast_node(NN_function);
	if (p->variant.class_new.constructor) {
	    add_symid_to_string(symid_of(p->variant.class_new.constructor),
				&string_buf);
	    if (current_named_block_symid) {
		write_rel(current_named_block_symid, "call",
			  symid_of(p->variant.class_new.constructor));
	    }
	}
	if (p->variant.class_new.type->variant.name_ref) {
	    name_reference_source_range(&p->variant.class_new.type->
					variant.name_ref->name,
					&range);
	    add_source_pos_to_ast(&range);
	}
	np->text = terminate_string(&string_buf);
	for (ep = p->variant.class_new.argument_list; ep; ep = ep->next) {
	    walk_expr(ep);
	}
	ast_nesting_depth--;	/* call_expr */
    	if (is_anonymous) {
    	    SET_push_main_block();
    	    if (crtp->scope != NULL) {
    	    	crtp->scope->SET_id = SET_get_block_num();
	    }
	    np = new_ast_node(NN_ast_block);
    	    if (crtp != NULL) {
	        add_to_string_with_len(":def", 4, &string_buf);
    	    	add_symid_to_string(symid_of(crtp), &string_buf);
	        add_source_pos_to_ast(&crtp->parse_info->body_location);
    	    	np->text = terminate_string(&string_buf);
	    }
	    ast_nesting_depth++;	/* ast_block */
    	    if (crtp->scope != NULL) {
	        walk_scope(crtp->scope, TRUE, TRUE);
	    }
	    ast_nesting_depth--;	/* ast_block */
    	    SET_pop_block();
    	    ast_nesting_depth--;    /* anonymous_class */
	}
	ast_nesting_depth--;	/* easy_new */
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_array_new:
	np = new_ast_node(NN_java_vec_new);
	add_symid_to_string(symid_of(p->type), &string_buf);
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* java_vec_new */
	np = new_ast_node(NN_type);
	add_symid_to_string(symid_of(p->variant.array_new.element_type->type),
			    &string_buf);
    	if (p->variant.array_new.element_type != NULL) {
	    set_locator_from_type_reference(&range,
                                            p->variant.array_new.element_type);
	}
	add_source_pos_to_ast(&range);
	np->text = terminate_string(&string_buf);
	if (p->variant.array_new.dim_exprs) {
	    np = new_ast_node(NN_dimensions);
	    ast_nesting_depth++;	/* dimensions */
	    for (ep = p->variant.array_new.dim_exprs; ep; ep = ep->next) {
		walk_expr(ep);
	    }
	    ast_nesting_depth--;	/* dimensions */
	}
	if (p->variant.array_new.initializer) {
	    walk_variable_initializer(p->variant.array_new.initializer);
	}
	ast_nesting_depth--;	/* java_vec_new */
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_cast:
	if (p->variant.cast.type != NULL) {
	    np = new_ast_node(NN_convert_expr);
	    ast_nesting_depth++;	/* convert_expr */
	}
	walk_expr(p->variant.cast.operand);
	if (p->variant.cast.type != NULL) {
	    add_symid_to_string(symid_of(p->type), &string_buf);
	    if (p->location) {
		extend_pos_left(&expr_range, p->location);
		add_source_pos_to_ast(&expr_range);
	    }
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth--;	/* convert_expr */
	}
	break;
    case ek_call:
        {
	    ast_node_ptr cnp = new_ast_node(NN_call_expr);
	    ast_nesting_depth++;	/* call_expr */
	    np = new_ast_node(NN_function);
	    add_symid_to_string(symid_of(p->variant.call.method),
				&string_buf);
	    if (current_named_block_symid) {
		write_rel(current_named_block_symid, "call",
			  symid_of(p->variant.call.method));
	    }
	    if (p->variant.call.name_ref) {
		name_reference_source_range(&p->variant.call.name_ref->
					    name, &range);
		add_source_pos_to_ast(&range);
	    }
	    np->text = terminate_string(&string_buf);
	    range = (p->location) ? *p->location : null_source_locator;
	    if (p->variant.call.object) {
    	    	expr_range = null_source_locator;
		walk_expr(p->variant.call.object);
		extend_pos_left(&range, &expr_range);
	    }
	    for (ep = p->variant.call.argument_list; ep; ep = ep->next) {
		walk_expr(ep);
	    }
	    ast_nesting_depth--;	/* call_expr */
	    if (p->type) {
		add_symid_to_string(symid_of(p->type), &string_buf);
	    }
	    add_source_pos_to_ast(&range);
	    cnp->text = terminate_string(&string_buf);
	}
	expr_range = range;
	break;
    case ek_expl_constr_invoc:
    	is_compiler_generated = p->variant.expl_constr_invoc.compiler_generated;
	if (!is_compiler_generated || EMIT_COMPILER_GENERATED_METHODS) {

	    np = new_ast_node(NN_call_expr);
    	    if (is_compiler_generated) {
    	    	cg_level += 1;
    	    	add_to_string_with_len(":cg", 3, &string_buf);
	    }
	    if (p->type) {
		add_symid_to_string(symid_of(p->type), &string_buf);
	    }
	    add_source_pos_to_ast(p->location);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;	/* call_expr */
	    np = new_ast_node(NN_function);
	    if (p->variant.expl_constr_invoc.constructor) {
		add_symid_to_string(symid_of(p->variant.expl_constr_invoc.
					     constructor),
				    &string_buf);
		if (current_named_block_symid) {
		    write_rel(current_named_block_symid, "call",
			      symid_of(p->variant.expl_constr_invoc.constructor));
		}
	    }
	    np->text = terminate_string(&string_buf);
	    if (p->variant.expl_constr_invoc.enclosing_object) {
    	    	/* !! This is a dubious use for NN_component_ref. !! */
		np = new_ast_node(NN_component_ref);
		ast_nesting_depth++;	/* component_ref */
		walk_expr(p->variant.expl_constr_invoc.enclosing_object);
		if (p->variant.expl_constr_invoc.is_super) {
		    np = new_ast_node(NN_super);
		}
		else np = new_ast_node(NN_this); /* !! Should not occur; see jil_def.h */
		ast_nesting_depth--;	/* component_ref */
	    }
	    else if (p->variant.expl_constr_invoc.is_super) {
		np = new_ast_node(NN_super);
	    }
	    else np = new_ast_node(NN_this);
	    for (ep = p->variant.expl_constr_invoc.argument_list; ep;
		 ep = ep->next) {
		walk_expr(ep);
	    }
	    ast_nesting_depth--;	/* call_expr */
    	    if (is_compiler_generated) {
    	    	cg_level -= 1;
	    }
	}
    	else {
    	    complain(error_csev, "Expression was expected, but compiler-generated constructor call omitted.");
    	}
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_instanceof:
	np = new_ast_node(NN_instanceof);
	if (p->type) {
	    add_symid_to_string(symid_of(p->type), &string_buf);
	}
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth++;	/* instanceof */
	walk_expr(p->variant.instanceof.operand);
	np = new_ast_node(NN_type);
	add_symid_to_string(symid_of(p->variant.instanceof.type->type),
			    &string_buf);
        if (p->variant.instanceof.type != NULL) {
	    set_locator_from_type_reference(&range, p->variant.instanceof.type);
	}
	add_source_pos_to_ast(&range);
	np->text = terminate_string(&string_buf);
	ast_nesting_depth--;	/* instanceof */
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_operation:
	range = (p->location) ? *p->location : null_source_locator;
	np = new_ast_node(op_node_name[p->variant.operation.op]);
	ast_nesting_depth++;	/* <operation> */
	for (ep = p->variant.operation.operands; ep; ep = ep->next) {
	    walk_expr(ep);
	    if (ep == p->variant.operation.operands) {
		extend_pos_left(&range, &expr_range);
	    }
	    if (!ep->next) {
		extend_pos_right(&range, &expr_range);
	    }
	}
	ast_nesting_depth--;	/* <operation> */
	if (p->type) {
	    add_symid_to_string(symid_of(p->type), &string_buf);
	}
	add_source_pos_to_ast(&range);
	np->text = terminate_string(&string_buf);
    	expr_range = range;
	break;
    case ek_class_literal:
	np = new_ast_node(NN_class_literal);
    	{
    	    a_type_reference_ptr trp = p->variant.class_literal_type_ref;
    	    if (trp != NULL) {
	        add_symid_to_string(symid_of(trp->type), &string_buf);
	    }
    	    else {
    	        /* Type reference used to be absent in case of void. */
    	    	complain(error_csev, "Type reference absent from class literal.");
    	    }
	}
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_super:
	np = new_ast_node(NN_super);
	if (p->type) {
	    add_symid_to_string(symid_of(p->type), &string_buf);
	}
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    case ek_error:
	np = new_ast_node(NN_error);
    	add_symid_to_string(error_type_symid(), &string_buf);
	add_source_pos_to_ast(p->location);
	np->text = terminate_string(&string_buf);
	if (p->location) {
	    expr_range = *p->location;
	}
	break;
    default:
	break;
    }
}

static void walk_switch_block_statement_group(a_switch_block_statement_group_ptr p) {
    a_switch_label_ptr slp;
    a_statement_ptr sp;
    ast_node_ptr np;
    np = new_ast_node(NN_switch_clause);
    ast_nesting_depth++;	/* switch_clause */
    np = new_ast_node(NN_case_values);
    ast_nesting_depth++;	/* case_values */
    for (slp = p->labels; slp; slp = slp->next) {
	walk_switch_label(slp);
    }
    ast_nesting_depth--;	/* case_values */
    for (sp = p->statements; sp; sp = sp->next) {
	walk_statement(sp, FALSE, FALSE);
    }
    ast_nesting_depth--;	/* switch_clause */
}

static void walk_catch_clause(a_catch_clause_ptr p) {
    a_source_locator catch_range = p->location;
    ast_node_ptr np;
    np = new_ast_node(NN_catch);
    add_symid_to_string(symid_of(p->param_variable), &string_buf);
    extend_pos_right(&catch_range, &p->block->location);
    add_source_pos_to_ast(&catch_range);
    np->text = terminate_string(&string_buf);
    ast_nesting_depth++;	/* catch */
    walk_declaration(&p->param_variable->decl);
    walk_statement(p->block, FALSE, FALSE);
    ast_nesting_depth--;	/* catch */
}

static char* const_kind_name[] = {
    "error", "integer", "float", "boolean", "string", "null"
};

static char* type_kind_name[] = {
    "none", "error", "boolean", "byte", "short", "int", "long",
    "char", "float", "double", "class", "interface", "array",
    "null"
};

static void walk_constant(a_constant_ptr p) {
    int i;
    switch (p->kind) {
    case ck_integer:
	break;
    case ck_float:
	break;
    case ck_boolean:
	break;
    case ck_string:
	break;
    case ck_error:
    case ck_null:
    default:
	break;
    }
}

static void walk_literal(a_literal_ptr p) {
}

static void walk_switch_label(a_switch_label_ptr p) {
    ast_node_ptr np;
    a_source_locator case_range = p->location;
    if (p->expr) {
	extend_pos_right_for_expr(&case_range, p->expr);
    }
    if (p->expr) {
    	if (write_ast_for_constant(p->expr, NN_case_label, &case_range, TRUE)) {
	    walk_constant(p->expr->variant.constant.ptr);
    	}
    	else {
    	    /* Non-constant case label is error. */
	    walk_expr(p->expr);
	}
    }
}

/* The following function creates ERR lines for each saved message. */
static void add_ERR_message(SET_list_node_ptr vmsg) {
    a_source_locator pos;
    a_compilation_unit_ptr cup;
    a_line_number start_lineno;
    a_boolean at_end_of_file;
    a_saved_message_ptr msg = (a_saved_message_ptr)vmsg;

    /* Check compilation unit, and emit SMT file if needed. */
    conv_sequence_to_comp_unit_and_line(msg->pos.start_sequence,
					&cup, &start_lineno,
					&at_end_of_file);
    update_current_file(cup);
    set_point_locator(pos, msg->pos); /* because ifext can't digest a range */

    if (!add_source_pos_to_string(&pos, &trial_buf)) {
	/* ERR syntax requires a pos, so skip it. */
    }
    else {
	add_3_chars_to_string("ERR", &output_buf);
	add_to_string(terminate_string(&trial_buf), &output_buf);
	add_1_char_to_string(' ', &output_buf);
	add_quoted_str_to_string(msg->text, /*add_quotes=*/TRUE,
				 &output_buf);
	terminate_string(&output_buf);
    }
}

static void write_smt_for_keyword(SET_list_node_ptr vkw) {
    a_saved_keyword_ptr kw = (a_saved_keyword_ptr)vkw;
    add_to_string_with_len("SMT keyword \"", 13, &output_buf);
    add_to_string(token_names[(int)kw->keyword], &output_buf);
    add_1_char_to_string('\"', &output_buf);
    add_source_pos_to_string(&kw->source_location, &output_buf);
    terminate_string(&output_buf);
}

/* Use our own conversion rather than fp_to_string defined in const_fp.h,
 * because the latter is rigid and verbose, rarely producing a string
 * that would appear in the source code.
 */
static void convert_floating_point_value(a_floating_point_value_ptr p,
    	    	    	    	    	 a_type_kind tk,
    	    	    	    	    	 char *buf)
{
    /* This isn't right yet !!! 
     * For portability, we need an IEEE->printable converter.
     * The following code assumes that we're running on hardware
     * that uses IEEE format.
     */

    if (tk != tk_float) {
	double dval;
    	memcpy((char *)&dval, (char *)p, sizeof(double));
	sprintf(buf, "%g", dval);
    }
    else {
    	float fval;
    	memcpy((char *)&fval, (char *)p, sizeof(float));
    	sprintf(buf, "%g", (double)fval);
    }

    /* for (i = 0; i < 8; i++) {
     *  printf("%02x", p->byte[i]);
     * }
     */
}

/* Reverses an array of characters. bp points to the beginning
 * of the array, and ep one past the last character in the array. 
 */
static void reverse_chars(char *bp, char *ep)
{
    while (ep > bp) {
        char ch;
    	ep -= 1;
	ch = *ep;
	*ep = *bp;
	*bp = ch;
    	bp += 1;
    }
}

/* Divide an unbounded integer by 10. The integer is stored in an
 * array of sig_parts parts, with the least significant pointed
 * to by p0.  Each element in the array is an unsigned short, so that
 * the radix of the representation is USHRT_MAX + 1.
 * Returns the remainder.
 */
static int divide_by_10(unsigned short *p0, int sig_parts) {
    unsigned int carry = 0;
    static unsigned long radix = 1UL << (sizeof(short) * CHAR_BIT);
    while (sig_parts > 0) {
    	unsigned long n;
    	sig_parts -= 1;
    	n = carry * radix + p0[sig_parts];
    	p0[sig_parts] = n / 10UL;
    	carry = (unsigned int)(n % 10UL);
    }
    return (int)carry;
}

static void convert_char_value(an_integer_value *p, char *buf) {
    /* Find the unicode value. */
    /* We don't need the following because only the least
     * significant part will affect uch.
     * int i;
     * a_unicode_char uch = 0;
     * for (i = 0; i < 4; i += 1) {
     * 	uch <<= (CHAR_BIT * sizeof(short));
     * 	uch += p->part[i];
     * }
     */
    a_unicode_char uch = p->part[3];

    /* Represent it in chars. */
    add_1_char_to_string('\'', &trial_buf);
    add_unicode_char_to_string(uch, '\'', &trial_buf);
    add_1_char_to_string('\'', &trial_buf);
    strcpy(buf, terminate_string(&trial_buf));
}

static void convert_integer_value(an_integer_value *p, char *buf) {
    unsigned short part[4];
    int i;
    char *bp = buf;

    /* Copy the value to a local array, finding the most significant part.
     * The local array has part[0] as the least significant.
     */
    int sig_parts = 0;
    for (i = 1; i <= 4; i += 1) {
    	if ((part[i - 1] = p->part[4 - i]) != 0) {
   	    sig_parts = i;
	}
    }
    while (sig_parts > 0) {
    	int rem = divide_by_10(part, sig_parts);
    	*bp++ = "0123456789"[rem];
    	if (part[sig_parts - 1] == 0) {
    	    sig_parts -= 1;
	}
    }
    reverse_chars(buf, bp);
    if (bp == buf) {
    	*bp++ = '0';
    }
    *bp++ = '\0';
}

/* For a class, interface, variable, or method, return its
   containing class, interface, or package.
   The package "" doesn't count as a context, and methods
   are not returned, because currently this function is
   used only to describe a context relation for the IF.
*/
static a_declaration_ptr context_of(a_declaration_ptr decl) {
    a_declaration_ptr con = NULL;
    a_scope_ptr sp = (decl != NULL) ? decl->scope : NULL;
    if (sp != NULL) {
	switch (sp->kind) {
	case sk_package:
	    con = &sp->assoc.package->decl;
    	    if (con != NULL && con->name == NULL) {
    	    	con = NULL;
	    }
            break;
        case sk_reference_type:
            con = &sp->assoc.reference_type->decl;
            break;
	}
    }
    return con;
}

static void write_sym_for_constant(a_constant_ptr conp, unsigned long symid) {
    /* Don't report constants that are compiler-generated or that
     * are the result of folding.
     */
    if (symid != 0) {
	insert_sym_prefix(conp->kind == ck_string ? "string" : "constant",
			  symid, FALSE);
	/* introductory quote has been added */
    	switch(conp->kind) {
    	case ck_integer:
	    {
    	    	an_integer_value *p = &conp->variant.integer_value;
    	    	char buf[1000];
    	    	if (conp->type_kind != tk_char) {
    	    	    if (conp->type_kind == tk_byte) {
    	    	    	/* occurs as a result of constant folding */
    	    	    	add_to_string("(byte)", &output_buf);
    	    	    }
    	    	    if (conp->type_kind == tk_short) {
    	    	    	/* occurs as a result of constant folding */
    	    	    	add_to_string("(short)", &output_buf);
    	    	    }
    	    	    convert_integer_value(p, buf);
        	    add_to_string(buf, &output_buf);
		}
    	    	else {
    	    	    convert_char_value(p, buf);
    	    	    add_quoted_str_to_string(buf, FALSE, &output_buf);
		}
    	    	switch (conp->type_kind) {
    	    	case tk_int:
    	    	case tk_char:
    	    	case tk_byte:
    	    	case tk_short:
    	    	    break;
    	    	case tk_long:
    	    	    add_1_char_to_string('L', &output_buf);
    	    	    break;
    	    	default:
    	    	    add_1_char_to_string('?', &output_buf);
	    	    complain(error_csev, "Unexpected type for integer constant.");
    	    	    break;
    	    	}
    	    }
    	    break;
    	case ck_float:
	    {
    	    	a_floating_point_value_ptr p = &conp->variant.float_value;
    	    	char buf[1000];
    	    	convert_floating_point_value(p, conp->type_kind, buf);
    	    	add_to_string(buf, &output_buf);
    	    	switch (conp->type_kind) {
    	    	case tk_double:
    	    	    if (strchr(buf, '.') == NULL) {
    	    	    	add_1_char_to_string('d', &output_buf);
		    }
    	    	    break;
    	    	case tk_float:
    	    	    add_1_char_to_string('f', &output_buf);
    	    	    break;
    	    	default:
    	    	    add_1_char_to_string('?', &output_buf);
	    	    complain(error_csev, "Unexpected type for floating point constant.");
    	    	    break;
    	    	}
    	    }
    	    break;
    	case ck_boolean:
	    {
    	    	a_byte_boolean val = conp->variant.boolean_value; /* 0 or 1 */
    	    	add_to_string(val ? "true" : "false", &output_buf);
    	    }
    	    break;
    	case ck_string:
	    {
    	    	a_unicode_string chars = conp->variant.string.chars;
    	        size_t length = conp->variant.string.length;
    	    	add_unicode_str_to_string(chars, length, '\"', TRUE, &trial_buf);
    	    	add_quoted_str_to_string(terminate_string(&trial_buf), FALSE, &output_buf);
    	    }
    	    break;
    	case ck_null:
	    {
    	    	add_to_string("null", &output_buf);
    	    }
    	    break;
    	case ck_error:
	    {
    	    	add_to_string("error", &output_buf);
    	    }
    	    break;
    	}
    	add_1_char_to_string('\"', &output_buf);
	terminate_string(&output_buf);

	/* Write REL lines for type of constant. */
    	{
	    /* A string literal is represented with tk_class and
	       no reference_type. Its type is java.lang.String. */
    	    a_type_ptr tp =   conp->kind == ck_string
    	    	    	    ? String_type()
    	    	    	    : shared_type_of_kind(conp->type_kind);
	    format_rel(symid_of(tp), "type", symid);
	    terminate_string(&output_buf);
	}
    }
}

static void update_current_file(a_compilation_unit_ptr p) {
    if (p != NULL && p->addendum->file != NULL
        && current_cmd_line_file != p->addendum->file) {

	current_cmd_line_file = p->addendum->file;
	add_to_string("SMT file ", &output_buf);
	add_quoted_str_to_string(p->addendum->file->file_name,
				 TRUE, &output_buf);
	terminate_string(&output_buf);
    }
}

static void write_checksum_sym_and_rel(void* csi, unsigned long file_symid) {
    unsigned char *checksum = get_final_checksum(csi);
    if (checksum) {
        int i;
        char buf[10];
        unsigned long checksum_symid = next_symid++;  
        add_3_chars_to_string("SYM", &output_buf);
        add_symid_to_string(checksum_symid, &output_buf);
        add_to_string_with_len("checksum \"", 10, &output_buf);
        for (i = 0; i < 16; i++) {
            sprintf (buf, "%02x", checksum[i]);
            add_2_chars_to_string(buf, &output_buf);
        }
        add_1_char_to_string('\"', &output_buf);
        terminate_string(&output_buf);

        write_rel(checksum_symid, "checksum", file_symid);
    }
}

static void walk_compilation_unit(a_compilation_unit_ptr p) {
    ast_node_ptr np;
    const char *file_name = p->file_name ? p->file_name : "";

    if (db_flag_is_set("if")) {
        printf("Walking compilation unit for file named %s.\n", file_name);
    }

    update_current_file(p);

    /* Track current entities, and write SYM record for file. */
    current_compilation_unit_symid = symid_of(p->addendum->file);
    current_compilation_unit_is_source = !current_cmd_line_file->virtual;

    if (p->checksum_info) write_checksum_sym_and_rel(p->checksum_info, current_compilation_unit_symid);

    switch (p->kind) {
    case cuk_source_file:
        {
	    an_import_declaration_ptr idp;

	    if (db_flag_is_set("if")) {
		printf("Compilation unit is source. Walking IL.\n");
	    }

	    np = new_ast_node(NN_AST);
	    add_1_char_to_string(' ', &string_buf);
	    add_quoted_str_to_string(file_name, TRUE, &string_buf);
	    np->text = terminate_string(&string_buf);

	    traverse_SET_list(&p->addendum->keywords, &write_smt_for_keyword);

	    /* Walk IL */
	    ast_nesting_depth++;	/* AST */
	    np = new_ast_node(NN_ast_root);
	    add_1_char_to_string(' ', &string_buf);
	    np->text = terminate_string(&string_buf);
	    ast_nesting_depth++;    /* ast_root */
	    if (   p->variant.source_file.package
		&& p->variant.source_file.package->name_ref) {

		a_source_locator range;
		a_package_ptr pp = p->variant.source_file.package->package;
		current_package_symid = symid_of(pp);
		np = new_ast_node(NN_package);
		add_symid_to_string(current_package_symid, &string_buf);
		add_source_pos_to_ast(&p->variant.source_file.package->location);
		np->text = terminate_string(&string_buf);
		name_reference_source_range(
		    &p->variant.source_file.package->name_ref->name,
		    &range);
		write_smt_line(current_package_symid, &range);
    	    	write_rel(current_package_symid, "context",
    	    	    	  current_compilation_unit_symid);
	    }
	    else current_package_symid = 0;
	    for (idp = p->variant.source_file.imports; idp; idp = idp->next) {
		walk_import_declaration(idp);
	    }
	    walk_scope(p->variant.source_file.scope, TRUE, TRUE);
	    ast_nesting_depth--;	/* ast_root */
	    ast_nesting_depth--;	/* AST */
	}
        break;
    case cuk_stub_file:
	if (db_flag_is_set("if")) {
	    printf("Compilation unit is stub.\n");
	}
    	write_stub_ast(p->addendum->file->file_name);
        break;
    case cuk_class_file:
    	{
	    a_reference_type_ptr rtp = p->variant.class_file.reference_type;
	    if (db_flag_is_set("if")) {
		printf("Compilation unit is class file.\n");
	    }
	    walk_reference_type(rtp);
	}
        break;
    }
    traverse_SET_list(&p->addendum->errors, &add_ERR_message);
    current_compilation_unit_symid = 0;
    current_compilation_unit_is_source = FALSE;
}

static void walk_import_declaration(an_import_declaration_ptr p) {
    ast_node_ptr np;
    if (p->decl->kind != dk_package && inspect_symid_of(p->decl) == 0
    	    	    	    	    && !auto_sym_upon_assign_symid(p->decl)) {
	write_sym_for_foreign_declaration(p->decl);
    }
    if (!p->duplicate) {
	write_rel(current_compilation_unit_symid, "import", symid_of(p->decl));
    }
    np = new_ast_node(NN_using_decl);
    add_source_pos_to_ast(&p->location);
    np->text = terminate_string(&string_buf);
    ast_nesting_depth++;	/* using_decl */
    np = new_ast_node(get_declaration_kind_node_name(p->decl));
    add_symid_to_string(symid_of(p->decl), &string_buf);
    np->text = terminate_string(&string_buf);
    ast_nesting_depth--;	/* using_decl */
}

static void write_stub_ast(const char *file_name) {
    ast_node_ptr np = new_ast_node(NN_AST);
    add_1_char_to_string(' ', &string_buf);
    add_quoted_str_to_string(file_name, TRUE, &string_buf);
    np->text = terminate_string(&string_buf);

    ast_nesting_depth++;	/* AST */
    np = new_ast_node(NN_ast_root);
    add_1_char_to_string(' ', &string_buf);
    np->text = terminate_string(&string_buf);
    ast_nesting_depth++;    /* ast_root */
    ast_nesting_depth--;	/* ast_root */
    ast_nesting_depth--;	/* AST */
}

/* ========================================================================= */

/* The following function writes an SMT line for the specified entity. 
 * If loc does not specify a full position, e.g. if a parse error is involved,
 * does not write.
 */
static void write_smt_line(unsigned long symid, a_source_locator_ptr loc) {
    /* An SMT line is erroneous without a full position. */
    if (loc->start_sequence != 0 && loc->end_sequence != 0) {
	add_3_chars_to_string("SMT", &output_buf);
	add_symid_to_string(symid, &output_buf);
	add_source_pos_to_string(loc, &output_buf);
	terminate_string(&output_buf);
    }
}

static const char *get_declaration_kind_name(a_declaration_ptr p) {
    const char* kind = declaration_kind_name[p->kind];
    if (   p->kind == dk_variable
        && p->scope && p->scope->kind == sk_reference_type
        && !(((a_variable_ptr)p)->modifiers & MK_STATIC)) {
	kind = "field";
    }
    return kind;
}

static unsigned char get_declaration_kind_node_name(a_declaration_ptr p) {
    unsigned char name = declaration_kind_node_name[p->kind];
    if (   p->kind == dk_variable
        && p->scope && p->scope->kind == sk_reference_type
        && !(((a_variable_ptr)p)->modifiers & MK_STATIC)) {
	name = NN_field;
    }
    return name;
}

/* Writes the SYM record to IF for a jil declaration.
 * The symid is assumed to be already assigned.
 */
static void write_sym_for_declaration(a_declaration_ptr decl) {
    a_type_ptr array_type = NULL;
    a_reference_type_ptr rtp = NULL;
    unsigned long this_symid = inspect_symid_of(decl);
    a_file_list_entry_ptr native_file = decl->SET_info.file_where_to_define;
    a_boolean is_compiler_generated
	=    has_initializer_name(decl)
	  || (decl->kind == dk_method &&
	      ((a_method_ptr)decl)->compiler_generated);
    if (decl->SET_info.is_reference_type_decl) {
	rtp = (a_reference_type_ptr)decl;
	if (rtp->decl.kind == dk_array && rtp->component_type) {
	    /* This is the first reference to an array type;
	     * allow code below to print its SYM line and a
	     * REL line relating it to its element type.
	     */
	    array_type = rtp->type;
	}
    	if (rtp->decl.kind == dk_class || rtp->decl.kind == dk_interface) {
    	    /* If this class was mentioned in a .class file loaded
             * from the command line, then its own .class file might not
             * be loaded yet, in which case its information must be updated
             * to avoid inaccuracies. */
    	    class_definition_needed(rtp)
	}
    }
    insert_sym_prefix(get_declaration_kind_name(decl), this_symid,
    	    	                                       is_compiler_generated);
    if (array_type != NULL) {
	add_type_name_to_string(array_type, /*add_quotes=*/FALSE,
				&output_buf);
	add_1_char_to_string('\"', &output_buf);
	terminate_string(&output_buf);
    }
    else {
	a_scope_ptr scope = add_qualified_name_to_string(decl, &output_buf);
    	add_name_decoration_to_string(scope, &output_buf);
	add_1_char_to_string('\"', &output_buf);
    	if (native_file != NULL) {
    	    if (native_file->virtual) {
    	    	/* Hack the source position, so model build treats this
    	    	 * as a definition. */
	        add_to_string(" 0/1 0", &output_buf);
	    }
    	    else {
	        add_source_pos_to_string(&decl->location, &output_buf);
	    }
	}
        terminate_string(&output_buf);
    }
    if (native_file == NULL) {
        /* Write required RELs and ATRs for entities that are not
           going to be handled as declarations. */
        write_foreign_declaration_attribs(decl);
        if (rtp && rtp->origin_compilation_unit_info) {
            unsigned long origin_symid = rtp->origin_compilation_unit_info->symid;
            /* write "SYM" line for file once */
            if (origin_symid == 0) {
                a_compilation_unit_ptr cup = rtp->origin_compilation_unit_info->cup;
                char full_filename[1024] = "";
                char *dir_name = NULL;
                char *dir_end = NULL;
                size_t dir_len;
                origin_symid = next_symid++;
                if (generate_file_checksum && !cup->checksum_info) {
                    checksum_of_archive(cup->file_name, &cup->checksum_info);
                }
                directory_name_of(cup->file_name, &dir_name, &dir_len);
                dir_end = cup->file_name + dir_len;
                if (dir_name && strncmp(dir_name, ".", dir_len) == 0) {
                    if (strcmp(current_working_dir, "") == 0) {
                        getcwd(current_working_dir, MAX_DIR_SIZE);
                    }
                    if (current_working_dir) strcpy(full_filename, current_working_dir);
                    if (dir_end) strcat(full_filename, dir_end);
                } 
                else strcpy(full_filename, cup->file_name);

                insert_sym_prefix("file", origin_symid, FALSE);
                add_quoted_str_to_string(full_filename, FALSE, &output_buf);
                add_1_char_to_string('\"', &output_buf);
                terminate_string(&output_buf);
                if (cup->checksum_info) write_checksum_sym_and_rel(cup->checksum_info, origin_symid);
                /* save symid */
                rtp->origin_compilation_unit_info->symid = origin_symid;
	    }
            write_rel(origin_symid, "define", this_symid);
        }

    }

    /* Mark this declaration as not needing further emission. */
    decl->SET_info.file_where_to_define = NULL;
    if (decl->SET_info.is_reference_type_decl) {
	a_reference_type_ptr rtp = (a_reference_type_ptr)decl;
	if (rtp->type != NULL) {
	    rtp->type->SET_info.file_where_to_define = NULL;
	}
    }
}

/* For a class or interface, write its basic attributes as well
 * as its relation with its superclasses.
 */
static void write_class_rels_and_attribs(a_reference_type_ptr rtp) {
    /* Note: no distinction is made between the "extends" and
     * "implements" relations, since the source and target of the
     * relation unambiguously determine which relation is
     * involved.
     */
    a_class_or_interface_reference_ptr cirp;
    write_declaration_attribs(&rtp->decl);
    for (cirp = rtp->superclass; cirp; cirp = cirp->next) {
	a_reference_type_ptr rp = cirp->class_or_interface;
	write_rel(symid_of(rtp), "subclass", symid_of(rp));
    }
    for (cirp = rtp->interfaces; cirp; cirp = cirp->next) {
	a_reference_type_ptr rp = cirp->class_or_interface;
	write_rel(symid_of(rtp), "subclass", symid_of(rp));
    }
}

/* For "foreign" entities, i.e. those which do not have declarations that
 * we're going to walk, put out their required attributes and relations.
 */
static void write_foreign_declaration_attribs(a_declaration_ptr decl) {
    unsigned long this_symid = symid_of(decl);
    if (decl->kind == dk_array) {
	/* Write REL line for array type. */
	a_reference_type_ptr rtp = (a_reference_type_ptr)decl;
    	a_type_ptr array_type = rtp->component_type ? rtp->type : NULL;

        if (array_type != NULL) {
	    format_rel(symid_of(array_type->reference_type->component_type),
		       "type", this_symid);
	    add_to_string("array", &output_buf);
	    terminate_string(&output_buf);
	}
    }
    else if (decl->kind == dk_package) {
	/* Write REL for context of containing package.
	   EDG keeps decl->scope null, so SET tracks creating scope. */
	a_scope_ptr scope = package_for_decl(decl)->creating_scope;
	if (scope != NULL && scope->kind == sk_package) {
	    a_package_ptr parent = scope->assoc.package;
	    if (parent->decl.name_length > 0) {
		unsigned long parid = symid_of(parent);
		add_3_chars_to_string("REL", &output_buf);
		add_symid_to_string(parid, &output_buf);
		add_to_string("\"context\"", &output_buf);
		add_symid_to_string(this_symid, &output_buf);
		terminate_string(&output_buf);
	    }
	}
    }
#if 0
    /*  MG says information on classes that aren't defined in this
     *  compilation unit don't belong in the IF.
     */
    else if (decl->kind == dk_class || decl->kind == dk_interface) {
    	write_class_rels_and_attribs((a_reference_type_ptr)decl);
    }
#endif
    write_declaration_attribs(decl);
}

static void write_declaration_attribs(a_declaration_ptr decl) {
    a_boolean atr_needed = FALSE;
    a_modifier_set modifiers = 0;
    if (decl->kind == dk_method) {
	a_method_ptr p = (a_method_ptr)decl;
        modifiers = p->modifiers;
	if (p->is_constructor) {
	    add_attrib("constructor", &atr_needed, &trial_buf);
	}
    	/*
    	 * Destructor attribute disabled, because the DISCOVER model
    	 * combines it with the constructor attribute, and then can't
    	 * tell that this is a destructor.  I suggested an explcit test
    	 * within discover.dis for the name "finalize", but MG didn't like
    	 * the incorrectness.  He suggested changing the name to ~finalize,
    	 * but I didn't like that.  Java culture doesn't consider "finalize"
    	 * to be a destructor, anyway.
    	 */
	if (FALSE && has_destructor_name(decl)) {
	    add_attrib("destructor", &atr_needed, &trial_buf);
	}
	if (!(   p->is_constructor 
              || ((modifiers & MK_FINAL) && p->overridden_methods == NULL)
              || (modifiers & MK_STATIC))) {
	    add_attrib("virtual", &atr_needed, &trial_buf);
	}
    }
    else if (decl->kind == dk_variable) {
        modifiers = ((a_variable_ptr)decl)->modifiers;
    }
    else if (decl->kind == dk_class || decl->kind == dk_interface || decl->kind == dk_array) {
        modifiers = ((a_reference_type_ptr)decl)->modifiers;
    }
    add_modifier_attribs(modifiers, &atr_needed, &trial_buf);
    if (atr_needed) {
	unsigned long symid = symid_of(decl);
	add_3_chars_to_string("ATR", &output_buf);
	add_symid_to_string(symid, &output_buf);
	add_to_string(terminate_string(&trial_buf), &output_buf);
	terminate_string(&output_buf);
    }
    /* Skip writing of "context" relation if it is an error declaration. */
    if (!decl->is_error) {
        write_context_relation(decl, modifiers, symid_of(context_of(decl)));
    }
}

/* The following function checks to see if a given declaration is from
 * a compilation unit that was not specified on the command line and, if
 * so, prints a SYM line for it.
 */

static void write_sym_for_foreign_declaration(a_declaration_ptr decl) {
    if (declaration_is_native(decl) == NULL) {
	if (decl->kind == dk_initialization_block || decl->kind == dk_overload_set) {
	}
    	else {
	    /* A reference to the symid forces SYM line. */
	    unsigned long symid = symid_of(decl);
	}
    }
}

a_compilation_unit_ptr stub_compilation_unit_of(const char *filename)
{
    a_compilation_unit_ptr stub_cup = NULL;
    if (filename) {
        a_compilation_unit_ptr cup;
        for (cup = compilation_units; cup; cup = cup->next) {
            if (cup->kind == cuk_stub_file && strcmp(cup->file_name, filename) == 0) {
                /* found a stub compilation_unit with the same name */
                stub_cup = cup;
                break;
            }
	}
    }
    return stub_cup;
}

int is_being_emitted(a_compilation_unit_ptr cup) {
    switch (cup->kind) {
    case cuk_source_file:
        /* See declaration_is_native below, and keep synchronized. */
	return cup->specified_on_command_line;
    case cuk_stub_file:
	return TRUE;
    default:
	return cup->addendum->file != NULL;
    }
}

/* The following function checks to see if a given declaration is from
 * a compilation unit that was specified on the command line.
 * If not it returns NULL.  It returns the declaration's compilation unit.
 * Arrays are considered foreign.  Packages are considered foreign.
 */
static a_compilation_unit_ptr declaration_is_native(a_declaration_ptr decl) {
    a_reference_type_ptr rtp = NULL;
    a_compilation_unit_ptr found_cup = NULL;
    if (decl->kind != dk_array) {
	if (decl->kind == dk_class || decl->kind == dk_interface) {
	    rtp = (a_reference_type_ptr) decl;
	}
	else {
	    a_scope_ptr sp = decl->scope;
	    while (sp && sp->kind != sk_reference_type) {
		sp = sp->parent_scope;
	    }
	    if (sp) {
		rtp = sp->assoc.reference_type;
	    }
	}
	if (rtp) {
	    a_compilation_unit_ptr cup = rtp->compilation_unit;
    	    if (rtp->own_compilation_unit != NULL) {
    	    	if (cup != NULL && rtp->own_compilation_unit != cup) {
		    complain(error_csev, "Inconsistent compilation unit for reference type.");
    	    	}
    	    	cup = rtp->own_compilation_unit;
    	    }
    	    if (cup != NULL) {
		switch (cup->kind) {
		case cuk_source_file:
	            /* See is_being_emitted above, and keep synchronized. */
		    if (cup->specified_on_command_line) {
    	    	    	found_cup = cup;
		    }
    	    	    break;
		default:
		    found_cup = cup;
    	    	    break;
		}
    	    	if ((cup->kind == cuk_source_file) != (rtp->own_compilation_unit == NULL)) {
		    complain(error_csev, "Inconsistent compilation unit info for reference type.");
		}
	    }
	}
    }
    return found_cup;
}

static a_boolean write_ast_for_constant(an_expr_ptr p,
				        unsigned char nn,
				        a_source_locator *loc,
    	    	    	    	    	a_boolean force) {
    if (p->kind == ek_constant) {
	write_smt_line(symid_of(p->variant.constant.ptr),
		       p->location);
    	if (p->variant.constant.expr == NULL) {
	    ast_node_ptr np;
	    a_token_string_ptr tokp = NULL;
	    a_token_kind token_kind = tok_none;
	    if (p->variant.constant.literal != NULL) {
		tokp = p->variant.constant.literal->token_string;
		token_kind = p->variant.constant.literal->token_kind;
	    }
	    np = new_ast_node(token_kind == tok_string_literal ? NN_string
							       : nn);
	    add_symid_to_string(symid_of(p->variant.constant.ptr),
				&string_buf);
	    add_source_pos_to_ast(loc);
	    if (tokp != NULL) {
		add_2_chars_to_string(" {", &string_buf);
		/* Convert Unicode without escaping quotes the usual
		 * way here, because we're looking for an ASCII source
		 * representation, and the token string already contains
		 * the quotes.  This makes a difference for 
		 *   \u000a   versus   \n    in the source token.
		 * Both should come out as \n before quoting and
		 * as \\n after quoting.
		 */
		add_unicode_str_to_string(tokp->unicode_string,
					  tokp->unicode_length,
					  '\0',
					  FALSE,
					  &trial_buf);
		add_quoted_str_to_string(terminate_string(&trial_buf),
					 TRUE,
					 &string_buf);
		add_1_char_to_string('}', &string_buf);
		np->leaf = TRUE;
	    }
	    np->text = terminate_string(&string_buf);
	}
    	else if (force) {
	    ast_node_ptr np;
    	    if (p->variant.constant.ptr == NULL) {
    	    	complain(error_csev, "Cannot form AST as required for constant.");
    	    }
	    np = new_ast_node(nn);
	    add_symid_to_string(symid_of(p->variant.constant.ptr),
				&string_buf);
	    add_source_pos_to_ast(loc);
	    np->text = terminate_string(&string_buf);
    	    ast_nesting_depth++;    /* nn */
    	    walk_expr(p->variant.constant.expr);
    	    ast_nesting_depth--;    /* nn */
	}
	return TRUE;
    }
    return FALSE;
}

/* The following function inserts the standard SYM prefix into the output
 * string in preparation for the insertion of the name (which must be
 * done differently for declarations and constants).
 */

static void insert_sym_prefix(const char* sym_kind, unsigned long symid,
			      a_boolean compiler_generated) {
    add_3_chars_to_string("SYM", &output_buf);
    add_symid_to_string(symid, &output_buf);
    add_to_string(sym_kind, &output_buf);
    if (compiler_generated) {
	add_to_string_with_len(" :cg", 4, &output_buf);
    }
    add_2_chars_to_string(" \"", &output_buf);
}

/* Returns the scope to which the name's qualifier belongs, or
 * lacking that, the scope to which the name belongs.
 * Caller guard against NULL.
 */
static a_scope_ptr add_qualified_name_to_string(a_declaration_ptr dp,
					        string_buffer_ptr bp) {
    a_declaration_ptr parent = NULL;
    a_scope_ptr scope = dp->scope;
    if (scope) {
	if (scope->kind == sk_reference_type) {
	    parent = (a_declaration_ptr)
		    scope->assoc.reference_type;
	}
	else if (scope->kind == sk_package &&
		 scope->assoc.package->decl.name_length > 0) {
	    parent = (a_declaration_ptr)
		    scope->assoc.package;
	}
    }
    if (parent != NULL) {
	scope = add_qualified_name_to_string(parent, bp);
	add_1_char_to_string('.', bp);
    }
    if (dp->name != NULL) {
        add_unicode_str_to_string(dp->name, dp->name_length, '\"', FALSE, bp);
    }
    else {
    	/* The symbol is nameless, but it's still going out to the IF.
    	 * C++ uses "<unnamed>", but we use "<anonymous>" because the
    	 * expected use is for anonymous classes.  The use of the same
    	 * substitute name could eventually cause a problem.
    	 */
    	if (dp->kind == dk_class) {
    	    add_to_string("<anonymous>", bp);
	}
    	else if (   dp->kind == dk_method
                 && ((a_method_ptr)dp)->is_constructor
    	    	 && dp->scope && dp->scope->kind == sk_reference_type
    	    	 && dp->scope->assoc.reference_type
    	    	 && dp->scope->assoc.reference_type->decl.name == NULL) {
    	    add_to_string("<anonymous>", bp);
	}
    	else {
    	    a_boolean have_complaint = TRUE;
    	    if (dp->kind == dk_array) {
    	    	/* Happens if the 'length' member is called for an array. */
    	    	have_complaint = FALSE;
    	    }
    	    else if (   dp->kind == dk_variable
                     && ((a_variable_ptr)dp)->is_parameter
                     && dp->scope && dp->scope->kind == sk_method) {
    	    	a_method_ptr containing = (a_method_ptr)
                                          dp->scope->method_or_init_block_decl;
                if (   containing != NULL
    	    	    && containing->is_constructor
                    && containing->compiler_generated) {

    	    	    /* May have a generated parameter to pass the
                     * enclosing object. */
    	    	    have_complaint = FALSE;
		}
    	    }
    	    if (have_complaint) {
    	        complain(error_csev, "Unexpected nameless symbol.");
	    }
    	    add_to_string("<unnamed>", bp);
    	}
    }
    if (dp->kind == dk_method) {
	add_method_sig_to_string((a_method_ptr) dp, bp);
    }
    return scope;
}

/* Convert one Unicode character to a source text ASCII representation.
 * Uses backslash and \u escapes for characters not printable in ASCII.
 * If quote is nonzero, then escapes quote and backslash using backslash.
 * ! Do DISCOVER's criteria for printability match isprint?
 */
static void add_unicode_char_to_string(a_unicode_char uch,
    	    	    	    	       char quote,
                                       string_buffer_ptr buf) {
    char ch = (char)uch;
    /* If the character is a printable ASCII character, use the original
       character.  Otherwise, use an escape (\uxxxx or other). */
    if (uch == '\011')
	add_2_chars_to_string("\\t", buf);
    else if (uch == '\012')
	add_2_chars_to_string("\\n", buf);
    else if (uch == '\015')
	add_2_chars_to_string("\\r", buf);
    else if (uch == '\010')
	add_2_chars_to_string("\\b", buf);
    else if (uch == '\014')
	add_2_chars_to_string("\\f", buf);
    else if (quote && (uch == quote || uch == '\\')) {
	add_1_char_to_string('\\', buf);
	add_1_char_to_string(ch, buf);
    }
    else if (uch <= 0xff && isprint(uch)) {
	add_1_char_to_string(ch, buf);
    }
    else {
	/* Convert the Unicode character to a \uxxxx escape. */
	char	num_buffer[10];
	sprintf(num_buffer, "\\u%04x", (int)uch);
	add_to_string_with_len(num_buffer, 6, buf);
    }
}

/* Convert "length" characters of the Unicode string specified by
 * "ustring" to a source text string. The converted string retains the full
 * ustring using backslash escapes for characters not printable in ASCII
 * as well as for quote and backslash. The outer pair of quotation marks may be
 * omitted by passing add_quotes==0, but in that case they should be added
 * by the caller.
 *
 * The character used for quotation is 'quote', normally ".
 * If quote is 0 then no escaping of a quote character or backslashes
 * is performed.
 */
static void add_unicode_str_to_string(a_unicode_string ustring, size_t length,
    	    	    	    	      char quote,
                                      a_boolean add_quotes, string_buffer_ptr buf)
{
    /* ! Not using conv_unicode_to_text which does not escape backslashes.
     * If the result is "\u2345" then you can't tell whether that came from
     * 1 Unicode character or 6 ASCII characters. 
     * textp = conv_unicode_to_text(str, len); 
     * It might make sense to do the quoting first as a Unicode transformation
     * (optional depending on the 'quote' parameter),
     * then call conv_unicode_to_text.  But conv_unicode_to_text doesn't
     * handle \t, \n, etc. nicely and would have ugly inefficiency if used
     * to convert a single character as we need with
     * add_unicode_char_to_string. */
    if (add_quotes) {
	add_1_char_to_string(quote, buf);
    }
    for (; length > 0; length--) {
    	add_unicode_char_to_string(*ustring++, quote, buf);
    }  /* for */
    if (add_quotes) {
	add_1_char_to_string(quote, buf);
    }
}

/* Add the source position text for the given location for an AST node
 * in the buffer 'string_buf'.  Suppressed within compiler-generated
 * expressions.
 */
static void add_source_pos_to_ast(a_source_locator *location)
{
    if (cg_level == 0) {
	add_source_pos_to_string(location, &string_buf);
    }
}

/* If 'location' does not indicate a source position (NULL or 0)
 * then nothing is added to the string and false is returned.
 * An error is reported if 'location' is in a different compilation unit from
 * the one currently being emitted.
 */
static a_boolean add_source_pos_to_string(a_source_locator *location, string_buffer_ptr buf)
{
    if (location != NULL && location->start_sequence != 0) {
	a_compilation_unit_ptr cup;
	a_line_number start_lineno;
    	a_boolean at_end_of_file;
	conv_sequence_to_comp_unit_and_line(location->start_sequence,
                                            &cup, &start_lineno,
    	    	    	    	    	    &at_end_of_file);
	if (cup->addendum->file == current_cmd_line_file) {
	    add_num_to_string(start_lineno, " ", "/", buf);
	    add_num_to_string(location->start_column, NULL, NULL, buf);
	    if (location->end_sequence != 0) {
		a_line_number end_lineno;
		conv_sequence_to_comp_unit_and_line(location->end_sequence,
						    &cup, &end_lineno,
    	    	    	    	    	    	    &at_end_of_file);
		if (end_lineno != start_lineno) {
		    add_num_to_string(end_lineno, " ", "/", buf);
		    add_num_to_string(location->end_column, NULL, NULL, buf);
		}
		else {
		    add_num_to_string(location->end_column - location->start_column + 1, " ", NULL, buf);
		}
	    }
	    else {
		/* Wmm changed this to add a partial pos if the end location is missing. */
		/* We don't complain("Missing end location") because the model build
		 * deals with a missing end location satisfactorily, and because
		 * the end location can be missing if the construct is an ERROR node,
		 * or is compiler-generated, or is an erroneous expression.
		 */
	    }
	}
    	else {
    	    complain(error_csev, "Cannot produce a valid source location.");
	    return FALSE;
    	}
	return TRUE;
    }
    return FALSE;
}

static void add_method_sig_to_string(a_method_ptr meth, string_buffer_ptr bp) {
    a_scope_ptr scope = meth->scope;
    a_declaration_ptr parm;
    a_boolean first_parm = TRUE;
    add_1_char_to_string('(', bp);
    for (parm = scope->declarations; parm; parm = parm->next) {
	if (parm->kind == dk_variable) {
	    a_type_reference_ptr trp = ((a_variable_ptr) parm)->type;
	    if (trp) {
		if (first_parm) {
		    first_parm = FALSE;
		}
		else add_2_chars_to_string(", ", bp);
		add_type_name_to_string(trp->type, FALSE, bp);
	    }
	}
    }
    add_1_char_to_string(')', bp);
}

/* The following functions mangle names to generate the Java Native
 * Interface corresponding C/C++ name from the Java declaration
 * of a native method.
 */
static void add_mangled_unicode_char_to_string(a_unicode_char uch,
    	    	    	    	    	       string_buffer_ptr bp) {
    char ch = (char)uch;
    /* If the character is an alphanumeric ASCII character, use the original
       character.  Otherwise, use an escape (_0xxxx or other). */
    if (uch == '.') { /* expected only in package names */
        add_1_char_to_string('_', bp);
    }
    else if (uch == '_') {
        add_2_chars_to_string("_1", bp);
    }
    else if (uch == ';') { /* for use in signatures (not yet) */
        add_2_chars_to_string("_2", bp);
    }
    else if (uch == '[') { /* for use in signatures (not yet) */
        add_2_chars_to_string("_3", bp);
    }
    else if (uch <= 0xff && isprint(uch) && isalnum(uch)) {
    	/* !! The above is not based on the official mangling criterion.
         * It should work because the characters in identifiers aren't
         * terribly tricky.
    	 */
	add_1_char_to_string(ch, bp);
    }
    else {
	/* Convert the Unicode character to a _0xxxx escape. */
	char	num_buffer[10];
	sprintf(num_buffer, "_0%04x", (int)uch);
	add_to_string_with_len(num_buffer, 6, bp);
    }
}

static void add_mangled_unicode_str_to_string(a_unicode_string ustring,
    	    	    	    	    	      size_t length,
    	    	    	    	    	      string_buffer_ptr bp) {
    for (; length>0; length--) {
        add_mangled_unicode_char_to_string(*ustring++, bp);
    }
}

static void add_mangled_qualified_name_to_string(a_declaration_ptr dp,
                                                 string_buffer_ptr bp) {
    if (dp->name != NULL) {
	a_declaration_ptr parent = NULL;
	a_scope_ptr scope = dp->scope;
	if (scope) {
	    if (scope->kind == sk_reference_type) {
		parent = (a_declaration_ptr)
			scope->assoc.reference_type;
	    }
	    else if (scope->kind == sk_package &&
		     scope->assoc.package->decl.name_length > 0) {
		parent = (a_declaration_ptr)
			scope->assoc.package;
	    }
	}
	if (parent != NULL) {
	    add_mangled_qualified_name_to_string(parent, bp);
	    add_1_char_to_string('_', bp);
	}
	add_mangled_unicode_str_to_string(dp->name, dp->name_length, bp);
    }
}

static void add_native_mangled_name_to_string(a_method_ptr meth,
                                              string_buffer_ptr bp) {
    add_to_string("Java_", bp);
    add_mangled_qualified_name_to_string(&meth->decl, bp);
}

static void add_type_name_to_string(a_type_ptr tp, a_boolean add_quotes,
				    string_buffer_ptr bp) {
    if (tp) {
	if (add_quotes) {
	    add_1_char_to_string('\"', bp);
	}
	switch (tp->kind) {
	case tk_none:
	    add_to_string("<no-type>", bp);
	    break;
	case tk_error:
	    add_to_string("<error-type>", bp);
	    break;
	case tk_void:
	    add_to_string("void", bp);
	    break;
	case tk_boolean:
	    add_to_string("boolean", bp);
	    break;
	case tk_byte:
	    add_to_string("byte", bp);
	    break;
	case tk_short:
	    add_to_string("short", bp);
	    break;
	case tk_int:
	    add_to_string("int", bp);
	    break;
	case tk_long:
	    add_to_string("long", bp);
	    break;
	case tk_char:
	    add_to_string("char", bp);
	    break;
	case tk_float:
	    add_to_string("float", bp);
	    break;
	case tk_double:
	    add_to_string("double", bp);
	    break;
	case tk_class:
	    if (tp->reference_type) {
		a_reference_type_ptr rtp = tp->reference_type;
		add_unicode_str_to_string(rtp->decl.name,
					  rtp->decl.name_length,
					  '\"', /*add_quotes=*/FALSE,
					  bp);
	    }
	    else {
		complain(error_csev, "Unexpected typeless class.");
		add_to_string("<class>", bp);
	    }
	    break;
	case tk_interface:
	    if (tp->reference_type) {
		a_reference_type_ptr rtp = tp->reference_type;
		add_unicode_str_to_string(rtp->decl.name,
					  rtp->decl.name_length,
					  '\"', /*add_quotes=*/FALSE,
					  bp);
	    }
	    else {
		complain(error_csev, "Unexpected typeless interface.");
		add_to_string("<interface>", bp);
	    }
	    break;
	case tk_array:
	    if (tp->reference_type) {
		/* print array type recursively */
		add_type_name_to_string(tp->reference_type->component_type,
					/*add_quotes=*/FALSE, bp);
		add_2_chars_to_string("[]", bp);
	    }
	    else {
		complain(error_csev, "Unexpected typeless array.");
		add_to_string("<array>", bp);
	    }
	    break;
	case tk_null:
	    add_to_string("null", bp);
	    break;
	default:
	    complain(error_csev, "Unexpected type.");
	    add_to_string("<unknown-type>", bp);
	    break;
	}
	if (add_quotes) {
	    add_1_char_to_string('\"', bp);
	}
    }
}

static void add_modifier_attribs(a_modifier_set mods, a_boolean* atr_added,
				 string_buffer_ptr bp) {
/*
 * These attributes are handled as part of the context relation.
 *     if (mods & MK_PUBLIC) {
 * 	add_attrib("public", atr_added, bp);
 *     }
 *     if (mods & MK_PROTECTED) {
 * 	add_attrib("protected", atr_added, bp);
 *     }
 *     if (mods & MK_PRIVATE) {
 * 	add_attrib("private", atr_added, bp);
 *     }
 *     if (!(mods & (MK_PUBLIC | MK_PROTECTED | MK_PRIVATE))) {
 * 	add_attrib("package", atr_added, bp);
 *     }
 */
    if (mods & MK_FINAL) {
	add_attrib("final", atr_added, bp);
    }
    if (mods & MK_STATIC) {
	add_attrib("static", atr_added, bp);
    }
    if (mods & MK_SYNCHRONIZED) {
	add_attrib("synchronized", atr_added, bp);
    }
    if (mods & MK_ABSTRACT) {
	add_attrib("abstract", atr_added, bp);
    }
    if (mods & MK_NATIVE) {
	add_attrib("native", atr_added, bp);
    }
    if (mods & MK_TRANSIENT) {
	add_attrib("transient", atr_added, bp);
    }
    if (mods & MK_VOLATILE) {
	add_attrib("volatile", atr_added, bp);
    }
}

static void write_context_relation(a_declaration_ptr p, a_modifier_set mods,
                                   unsigned long context_symid) {
    unsigned long this_symid = symid_of(p);
    if (context_symid != 0 && this_symid != 0) {
	add_3_chars_to_string("REL", &output_buf);
	add_symid_to_string(context_symid, &output_buf);
	add_to_string("\"context\"", &output_buf);
    	add_symid_to_string(this_symid, &output_buf);
	if (mods & MK_PUBLIC) {
	    add_to_string("public", &output_buf);
	}
	if (mods & MK_PROTECTED) {
	    add_to_string("protected", &output_buf);
	}
	if (mods & MK_PRIVATE) {
	    add_to_string("private", &output_buf);
	}
	if (!(mods & MK_ACCESS_MODIFIERS)) {
	    add_to_string("package", &output_buf);
	}
	terminate_string(&output_buf);
    }
}

static unsigned long write_function_type_of_method(a_method_ptr p)
{
    if (p->return_type != NULL) {
	a_scope_ptr scope = p->scope;
	a_declaration_ptr parm;
	unsigned long parm_num = 0;
        unsigned long function_type_symid = next_symid++;

	/* Write the function type. */
    	insert_sym_prefix("type", function_type_symid, FALSE);
    	add_type_name_to_string(p->return_type->type, FALSE, &output_buf);
    	add_method_sig_to_string(p, &output_buf);
    	add_1_char_to_string('\"', &output_buf);
    	terminate_string(&output_buf);
    	
	/* Write the function type's return type. */
    	format_rel(symid_of(p->return_type->type), "type", function_type_symid);
    	add_to_string("return", &output_buf);
    	terminate_string(&output_buf);

	/* Write the parameter types. */
	for (parm = scope->declarations; parm; parm = parm->next) {
    	    parm_num += 1;
	    if (parm->kind == dk_variable) {
		a_type_reference_ptr trp = ((a_variable_ptr) parm)->type;
		if (trp) {
		    format_rel(symid_of(trp->type), "type", function_type_symid);
    	    	    add_num_to_string(parm_num, "argument(", ")", &output_buf);
    	    	    terminate_string(&output_buf);
		}
	    }
	}
    	return function_type_symid;
    }
    return 0;
}

/* Write a SYM record to the output file (once), for an error-type
 * symbol, and return its symid.
 */
static unsigned long error_type_symid() {
    static unsigned long symid = 0;
    if (symid == 0) {
    	symid = next_symid++;
    	insert_sym_prefix("type", symid, FALSE);
    	add_to_string("<error-type>\"", &output_buf);
    	terminate_string(&output_buf);
    }
    return symid;
}

/* AST SUPPORT */

const char* ast_node_name_str[] = {
    "AST", 			/* NN_AST */
    "addr_expr", 		/* NN_addr_expr */
    "alshift_assign_expr", 	/* NN_alshift_assign_expr */
    "alshift_expr", 		/* NN_alshift_expr */
    "anonymous_class",	    	/* NN_anonymous_class */
    "array_ref", 		/* NN_array_ref */
    "arshift_assign_expr", 	/* NN_arshift_assign_expr */
    "arshift_expr", 		/* NN_arshift_expr */
    "assign_expr", 		/* NN_assign_expr */
    "ast_block", 		/* NN_ast_block */
    "ast_declspec", 		/* NN_ast_declspec */
    "ast_root",			/* NN_ast_root */
    "ast_superclass", 		/* NN_ast_superclass */
    "bit_and_assign_expr", 	/* NN_bit_and_assign_expr */
    "bit_and_expr", 		/* NN_bit_and_expr */
    "bit_ior_assign_expr", 	/* NN_bit_ior_assign_expr */
    "bit_ior_expr", 		/* NN_bit_ior_expr */
    "bit_not_expr", 		/* NN_bit_not_expr */
    "bit_xor_assign_expr", 	/* NN_bit_xor_assign_expr */
    "bit_xor_expr", 		/* NN_bit_xor_expr */
    "block_assign", 		/* NN_block_assign */
    "call_expr", 		/* NN_call_expr */
    "case_label", 		/* NN_case_label */
    "case_stmt", 		/* NN_case_stmt */
    "case_values", 		/* NN_case_values */
    "catch", 			/* NN_catch */
    "class", 			/* NN_class */
    "class_literal", 		/* NN_class_literal */
    "java_component_ref", 	/* NN_component_ref */
    "cond_expr", 		/* NN_cond_expr */
    "condition", 		/* NN_condition */
    "constant", 		/* NN_constant */
    "convert_expr", 		/* NN_convert_expr */
    "ctor_init", 		/* NN_ctor_init */
    "default", 			/* NN_default */
    "dimensions", 		/* NN_dimensions */
    "do_while_stmt", 		/* NN_do_while_stmt */
    "dynamic_init", 		/* NN_dynamic_init */
    "easy_new", 		/* NN_easy_new */
    "eq_expr", 			/* NN_eq_expr */
    "error", 			/* NN_error */
    "exit_stmt", 		/* NN_exit_stmt */
    "expr_stmt", 		/* NN_expr_stmt */
    "expression", 		/* NN_expression */
    "field", 			/* NN_field */
    "field_decl", 		/* NN_field_decl */
    "finally", 			/* NN_finally */
    "for_stmt", 		/* NN_for_stmt */
    "function", 		/* NN_function */
    "function_decl", 		/* NN_function_decl */
    "function_def", 		/* NN_function_def */
    "ge_expr", 			/* NN_ge_expr */
    "gt_expr", 			/* NN_gt_expr */
    "if_stmt", 			/* NN_if_stmt */
    "implements", 		/* NN_implements */
    "indirect_ref", 		/* NN_indirect_ref */
    "init", 			/* NN_init */
    "initialization_block", 	/* NN_initialization_block */
    "instanceof", 		/* NN_instanceof */
    "interface", 		/* NN_interface */
    "java_vec_new", 		/* NN_java_vec_new */
    "label", 			/* NN_label */
    "label_decl", 		/* NN_label_decl */
    "le_expr", 			/* NN_le_expr */
    "list_decl", 		/* NN_list_decl */
    "lrshift_expr", 		/* NN_lrshift_expr */
    "lrshift_assign_expr", 	/* NN_lrshift_assign_expr */
    "lt_expr", 			/* NN_lt_expr */
    "minus_assign_expr", 	/* NN_minus_assign_expr */
    "minus_expr", 		/* NN_minus_expr */
    "mult_assign_expr", 	/* NN_mult_assign_expr */
    "mult_expr", 		/* NN_mult_expr */
    "name_clause", 		/* NN_name_clause */
    "ne_expr", 			/* NN_ne_expr */
    "negate_expr", 		/* NN_negate_expr */
    "no_operand", 		/* NN_no_operand */
    "null", 			/* NN_null */
    "nw_expr", 			/* NN_nw_expr */
    "object_lifetime", 		/* NN_object_lifetime */
    "operation", 		/* NN_operation */
    "package", 			/* NN_package */
    "parens", 			/* NN_parens */
    "parm_decl", 		/* NN_parm_decl */
    "plus_assign_expr", 	/* NN_plus_assign_expr */
    "plus_expr", 		/* NN_plus_expr */
    "postdecrement_expr", 	/* NN_postdecrement_expr */
    "postincrement_expr", 	/* NN_postincrement_expr */
    "predecrement_expr", 	/* NN_predecrement_expr */
    "preincrement_expr", 	/* NN_preincrement_expr */
    "rdiv_assign_expr", 	/* NN_rdiv_assign_expr */
    "rdiv_expr", 		/* NN_rdiv_expr */
    "return_stmt", 		/* NN_return_stmt */
    "set_vla_size", 		/* NN_set_vla_size */
    "statement", 		/* NN_statement */
    "string", 			/* NN_string */
    "struct", 			/* NN_struct */
    "super", 			/* NN_super */
    "switch_clause", 		/* NN_switch_clause */
    "synchronized", 		/* NN_synchronized */
    "temp_init", 		/* NN_temp_init */
    "this", 			/* NN_this */
    "throw_expr", 		/* NN_throw_expr */
    "throw_spec", 		/* NN_throw_spec */
    "trunc_div_assign_expr", 	/* NN_trunc_div_assign_expr */
    "trunc_div_expr", 		/* NN_trunc_div_expr */
    "trunc_mod_assign_expr", 	/* NN_trunc_mod_assign_expr */
    "trunc_mod_expr", 		/* NN_trunc_mod_expr */
    "truth_andif_expr", 	/* NN_truth_andif_expr */
    "truth_not_expr", 		/* NN_truth_not_expr */
    "truth_orif_expr", 		/* NN_truth_orif_expr */
    "try_block", 		/* NN_try_block */
    "type", 			/* NN_type */
    "type_decl", 		/* NN_type_decl */
    "type_decl_modified", 	/* NN_type_decl_modified */
    "typespec", 		/* NN_typespec */
    "unary_plus", 		/* NN_unary_plus */
    "unknown_node", 		/* NN_unknown_node */
    "used_entity", 		/* NN_used_entity */
    "using", 			/* NN_using */
    "using_decl", 		/* NN_using_decl */
    "var_decl", 		/* NN_var_decl */
    "variable", 		/* NN_variable */
    "vec_nw_expr", 		/* NN_vec_nw_expr */
    "virtual_function_ptr", 	/* NN_virtual_function_ptr */
    "vla_decl", 		/* NN_vla_decl */
    "while_stmt" 		/* NN_while_stmt */
};

typedef struct ast_block* ast_block_ptr;

#define AST_NODE_BLOCK_COUNT 32000

typedef struct ast_block {
    ast_block_ptr next;
    size_t next_node;
    ast_node node[AST_NODE_BLOCK_COUNT];
} ast_block;

static ast_block_ptr first_ast_block;
static ast_block_ptr last_ast_block;

static ast_node_ptr new_ast_node(an_ast_node_name name) {
    ast_node_ptr np;
    if (!last_ast_block || last_ast_block->next_node >= AST_NODE_BLOCK_COUNT) {
	ast_block_ptr bp = (ast_block_ptr) malloc(sizeof(ast_block));
	if (last_ast_block) {
	    last_ast_block->next = bp;
	}
	else first_ast_block = bp;
	last_ast_block = bp;
	bp->next = NULL;
	bp->next_node = 0;
    }
    np = &last_ast_block->node[last_ast_block->next_node++];
    np->text = "";
    np->depth = ast_nesting_depth;
    np->name = name;
    np->leaf = FALSE;
    return np;
}

static void print_error_nodes_error (const char *filename, unsigned int num_errors) {
    if (filename) {
        char file[4096] = "";
        char *start = NULL;
        char *end = NULL;
        a_boolean sent = FALSE;
        char err_msg[4096];
        /* The following error message is used by the dismb scripts to figure 
         * out if a file is forgiven. So any changes to the message should be 
         * accompanied by appropriate changes to the scripts. Also the last 
         * parameter to ParserMsg() function is "TRUE" indicating that this 
         * message should be sent to stdout so that dismb scripts can see it.
         */ 
        sprintf(err_msg, "Number of error nodes in IF for file%s : %d", filename, num_errors);
        if ((start = strchr(filename, '"')) != NULL && 
	    (end = strrchr(filename, '"')) != NULL && start != end) {
	    a_file_list_entry_ptr flep = NULL; 

	    strncpy(file, start+1, end-start-1); 
            flep = lookup_file_list_entry(file);
            if (flep && flep->mgroup_idstr) {
                /* If a message group exists for the file for which error
                   is being reported, then send the message to that group.
                 */
                fwd_activate_message_group((const char *)flep->mgroup_idstr);
                ParserMsg(err_msg, es_error, TRUE); 
                fwd_deactivate_message_group();
                sent = TRUE;
            }
        }
        if (!sent) {
            /* Send message to the default group if it was not sent to the
               message group of the appropriate file. 
             */
            ParserMsg(err_msg, es_error, TRUE);
        }
    }
}

static void dump_ast() {
    ast_block_ptr bp = first_ast_block;
    int cur_node = 0;
    int last_depth = 0;
    int this_depth;
    a_boolean first_time = TRUE;
    a_boolean prev_was_leaf = FALSE;

    /* The following variables are used to give some diagnostics to the
       model build process about errors during parsing. */
    const char *filename = NULL;
    unsigned int error_nodes_in_current_file = 0;

    while (bp) {
	ast_node_ptr np;
	if (cur_node >= bp->next_node) {
	    bp = bp->next;
	    cur_node = 0;
	}
	if (bp) {
	    np = &bp->node[cur_node++];
	    this_depth = np->depth;
	}
	else {
	    np = NULL;
	    this_depth = 0;
	}
	if (this_depth > last_depth) {
	    if (this_depth > last_depth + 1) {
		complain(catastrophe_csev, "Depth increment > 1 in AST dump.");
		this_depth = last_depth + 1;
	    }
	    add_2_chars_to_string(" {", &output_buf);
	}
	else {
	    if (!first_time && !prev_was_leaf) {
		add_3_chars_to_string(" {}", &output_buf);
	    }
	    while (this_depth < last_depth--) {
		terminate_string(&output_buf);
		if (indent_AST) {
		    indent_to_depth(last_depth);
		}
		add_1_char_to_string('}', &output_buf);
	    }
	}
	if (first_time) {
	    first_time = FALSE;
	}
	else terminate_string(&output_buf);
	if (np) {
	    if (indent_AST) {
		indent_to_depth(this_depth);
	    }
	    add_to_string(ast_node_name_str[np->name], &output_buf);
	    add_to_string(np->text, &output_buf);
	    prev_was_leaf = np->leaf;

            /* start of ast for a file */
            if (np->name == NN_AST) {
	        if (error_nodes_in_current_file > 0) {
		    print_error_nodes_error(filename, error_nodes_in_current_file);
                }
	        /* reset file name and error counter for current file */
                filename = np->text;
	        error_nodes_in_current_file = 0;
            }
            if (np->name == NN_error) {
	        error_nodes_in_current_file++;
            }

	}
	last_depth = this_depth;
    }

    if (error_nodes_in_current_file > 0) {
        print_error_nodes_error(filename, error_nodes_in_current_file);
    }
}

/* MISCELLANEOUS UTILITIES */

/* Determine whether the entity gets its SYM line emitted automatically
 * upon first reference.
 */
static a_boolean auto_sym_upon_assign_symid(a_declaration_ptr entity) {
    SET_info_t* infop = &entity->SET_info;
    return infop->kind != etk_declaration || infop->is_reference_type_decl
    	                                  || entity->kind == dk_package;
}

/* Assigns an id number to a symbol that is being emitted to IF.
 * The entity is a struct whose initial part is a SET_info_t.
 * Does nothing if the entity's symid is already assigned (nonzero).
 * Provides that a reference type decl and its associated type symbol
 * have identical symids.
 *
 * Writes the symbol's SYM line immediately, unless the file currently
 * being processed from the command line is not the symbol's defining file.
 */
static unsigned long assign_symid(void* entity) {
    SET_info_t* infop = (SET_info_t*)entity;
    unsigned long symid = infop->symid;
    if (symid != 0) {
    }
    else if (infop->kind == etk_declaration && infop->is_reference_type_decl) {
	a_reference_type_ptr rtp = (a_reference_type_ptr)entity;
	if (rtp->type != NULL) {
	    symid = inspect_symid_of(rtp->type);
	}
    }
    else if (infop->kind == etk_type) {
	a_type_ptr tp = (a_type_ptr)entity;
	if (is_reference_type_kind(tp->kind) && tp->reference_type != NULL) {
	    symid = inspect_symid_of(tp->reference_type);
	}
    }
    if (symid == 0) {
    	a_declaration_ptr decl = NULL;
	a_type_ptr tp = NULL;
    	a_file_list_entry_ptr defining_file = NULL;

	symid = next_symid++;
	set_symid_of(entity, symid);

    	/* Set symid for reference type <--> decl correspondence. */
	if (infop->kind == etk_declaration) {
	    decl = (a_declaration_ptr)entity;
	    if (infop->is_reference_type_decl) {
		a_reference_type_ptr rtp = (a_reference_type_ptr)entity;
                tp = rtp->type;
		if (tp != NULL) {
		    set_symid_of(tp, symid);
		}
	    }
	}
	else if (infop->kind == etk_type) {
	    tp = (a_type_ptr)entity;
	    if (is_reference_type_kind(tp->kind) && tp->reference_type != NULL) {
		decl = &tp->reference_type->decl;
		set_symid_of(tp->reference_type, symid);
	    }
	}
    	if (decl != NULL) {
    	    a_compilation_unit_ptr defining_cup = declaration_is_native(decl);
    	    if (defining_cup != NULL) {
    	        defining_file = defining_cup->addendum->file;
	    }
    	    decl->SET_info.file_where_to_define = defining_file;
	    if (tp != NULL) {
		tp->SET_info.file_where_to_define = defining_file;
	    }
	}
        if (defining_file == NULL || defining_file == current_cmd_line_file) {
            write_sym_for_entity(entity);
	}
    }
    return symid;
}

/* Writes the SYM record to the IF file for any and all IF symbols which
 * arise from jil decls, types, and constants.
 * The symid is assumed to be already assigned.
 */
static void write_sym_for_entity(void* entity) {
    SET_info_t* infop = (SET_info_t*)entity;
    unsigned long symid = infop->symid;
    if (infop->kind == etk_declaration) {
	a_declaration_ptr decl = (a_declaration_ptr)entity;
	write_sym_for_declaration(decl);
    }
    else if (infop->kind == etk_type) {
	a_type_ptr tp = (a_type_ptr)entity;
	if (is_reference_type_kind(tp->kind) && tp->reference_type != NULL) {
	    write_sym_for_declaration(&tp->reference_type->decl);
	}
	else {
	    /* This is the first reference to a fundamental type; write
	     * the SYM line now.
	     */
	    insert_sym_prefix("type", symid, FALSE);
	    add_type_name_to_string(tp, /*add_quotes=*/FALSE, &output_buf);
	    add_1_char_to_string('\"', &output_buf);
	    terminate_string(&output_buf);
	}
    }
    else if (infop->kind == etk_constant) {
	/* This is the first reference to a constant; write
	 * the SYM line now.
	 */
	a_constant_ptr cp = (a_constant_ptr) entity;
	write_sym_for_constant(cp, symid);
    }
    else if (infop->kind == etk_file) {
	/* This is the first reference to a file;
	 * write the SYM line now.
	 */
	a_file_list_entry_ptr flep = (a_file_list_entry_ptr)entity;
	insert_sym_prefix("file", symid, FALSE);
	add_quoted_str_to_string(flep->file_name, FALSE, &output_buf);
	add_1_char_to_string('\"', &output_buf);
	terminate_string(&output_buf);

	if (flep->virtual) {
	    a_boolean atr_needed;
	    atr_needed = FALSE;
	    add_attrib("virtual", &atr_needed, &trial_buf);
	    if (atr_needed) {
		add_3_chars_to_string("ATR", &output_buf);
		add_symid_to_string(symid, &output_buf);
		add_to_string(terminate_string(&trial_buf), &output_buf);
		terminate_string(&output_buf);
	    }
	}
    }
}

static void modifier_source_range(a_parse_modifier_ptr mods,
				  a_source_locator_ptr range) {
    range->start_sequence = mods->location.start_sequence;
    range->start_column = mods->location.start_column;
    while (mods->next) {
	mods = mods->next;
    }
    range->end_sequence = mods->location.end_sequence;
    range->end_column = mods->location.end_column;
}

static void name_reference_source_range(a_name_reference_component_ptr comp,
					a_source_locator_ptr range) {
    range->end_sequence = comp->location.end_sequence;
    range->end_column = comp->location.end_column;
    while (comp->qualifier) {
	comp = comp->qualifier;
    }
    range->start_sequence = comp->location.start_sequence;
    range->start_column = comp->location.start_column;
}

static void extend_pos_left(a_source_locator_ptr master,
			    a_source_locator_ptr test) {
    if (!test->start_sequence) {
    }
    else if (!master->start_sequence) {
	*master = *test;
    }
    else if (test->start_sequence < master->start_sequence ||
	     (test->start_sequence == master->start_sequence &&
	      test->start_column < master->start_column)) {
	master->start_sequence = test->start_sequence;
	master->start_column = test->start_column;
    }
}

static void extend_pos_right(a_source_locator_ptr master,
			     a_source_locator_ptr test) {
    if (!test->start_sequence) {
    }
    else if (!master->start_sequence) {
	*master = *test;
    }
    else if (test->end_sequence > master->end_sequence ||
	     (test->end_sequence == master->end_sequence &&
	      test->end_column > master->end_column)) {
	master->end_sequence = test->end_sequence;
	master->end_column = test->end_column;
    }
}

static void extend_pos_for_declarator(a_source_locator_ptr master,
				      a_variable_ptr var) {
    if (var->initializer) {
	a_variable_initializer_ptr init = var->initializer;
	if (init->is_array_initializer) {
	    extend_pos_right(master, init->location);
	}
	else extend_pos_right_for_expr(master, init->variant.expr);
    }
    else if (var->type && var->type->type && var->type->type->kind == tk_array &&
	     end_pos_is_greater(var->type->variant.name_ref->name.location,
				var->decl.location)) {
	extend_pos_right(master, &var->type->variant.name_ref->name.location);
    }
    else extend_pos_right(master, &var->decl.location);
}

static void extend_pos_right_for_expr(a_source_locator_ptr master,
				      an_expr_ptr expr) {
    while (expr && (expr->kind == ek_cast || expr->kind == ek_operation)) {
	if (expr->kind == ek_cast) {
	    expr = expr->variant.cast.operand;
	}
	else if (expr->variant.operation.op == eok_parens ||
		 expr->variant.operation.op == eok_array_access ||
		 expr->variant.operation.op == eok_post_incr ||
		 expr->variant.operation.op == eok_post_decr) {
	    extend_pos_right(master, expr->location);
	    return;
	}
	else for (expr = expr->variant.operation.operands; expr && expr->next;
		  expr = expr->next)
		{ }
    }
    if (expr) {
	extend_pos_right(master, expr->location);
    }
}

static a_variable_ptr last_declarator_in_decl(a_variable_ptr p) {
    a_variable_ptr last_var = p;
    while (p = next_declarator_in_decl(p)) {
	last_var = p;
    }
    return last_var;
}

/* If this variable is followed by another in the same declaration,
 * return the next variable.
 */
static a_variable_ptr next_declarator_in_decl(a_variable_ptr this_var) {
    if (this_var && this_var->decl.next &&
	this_var->decl.next->kind == dk_variable) {
	a_variable_ptr next_var = (a_variable_ptr) this_var->decl.next;
    	if (next_var->decl_follows_comma) {
	    return next_var;
	}
    }
    return NULL;
}

void set_defining_file_name(const char *fname)
{
    defining_file_name = fname;
}

void clear_defining_file_name()
{
    defining_file_name = NULL;
}

const char *get_defining_file_name()
{
    return defining_file_name;
}

/* MAIN ENTRY */

void dump_SET_IF() {
    if (IF_file_name) {
        /* Check if atleast one file listed on command line is valid,
           otherwise generate error. */
        a_boolean good = FALSE;
	for (current_compilation_unit = compilation_units;
	     current_compilation_unit;
	     current_compilation_unit = current_compilation_unit->next) {
            if (is_being_emitted(current_compilation_unit) &&
                !current_compilation_unit->invalid) {
                good = TRUE;
                /* stop as soon as we hit the first valid unit */
                break;
            }
        }
        if (!good) {
            complain(error_csev, "Failed to generate IF for any of the compilation files.");
        }
	else if (initialize_string_buffers(IF_file_name, NULL, NULL, NULL)) {
            a_file_list_entry_ptr flep;
	    add_to_string("SMT language \"java\"", &output_buf);
	    terminate_string(&output_buf);
	    for (current_compilation_unit = compilation_units;
		 current_compilation_unit;
		 current_compilation_unit = current_compilation_unit->next) {
		if (is_being_emitted(current_compilation_unit) &&
                    !current_compilation_unit->invalid) {
		    walk_compilation_unit(current_compilation_unit);
		}
	    }
	    dump_ast();
	    close_output_buf();
	}
	else {
    	    complain_str(catastrophe_csev, "Unable to open $1 for output.", IF_file_name);
	}
    }
}
