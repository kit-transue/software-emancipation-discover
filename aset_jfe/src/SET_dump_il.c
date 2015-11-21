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
#include <stdio.h>
#include "SET_IF.h"

#define IND_INCR 1

/* Record of scope dumping. */
typedef struct dump_scope_node {
    a_scope_ptr this;
    a_boolean dumped;
    struct dump_scope_node *next;
} dump_scope_node;
static dump_scope_node *scopes_for_dumping;

/* Record of declaration dumping, for packages. */
typedef struct dump_declaration_node {
    a_declaration_ptr this;
    a_boolean dumped;
    struct dump_declaration_node *next;
} dump_declaration_node;
static dump_declaration_node *declarations_for_dumping;

/* Declarations of routines.  Each routine takes a pointer to its
 * associated data structure and an indentation level; all members
 * of the data structure are to be printed with at least that level
 * of indentation.  Mutual recursion is common.
 */

static void p_ind(int);
static void p_label_str(const char* str, int ind);
static void p_label_and_ptr(const char* str, const void* p, int ind);
static void p_scope(a_scope_ptr p, int ind);
static void p_boolean(const char* label, a_boolean flag, int ind, a_boolean last);
static void add_boolean(const char* label, a_boolean flag, a_boolean last);
static void p_declaration(a_declaration_ptr p, int ind);
static void p_name_reference(a_name_reference_ptr p, int ind);
static void p_full_name_reference(a_name_reference_ptr p, int ind);
static void p_unicode_string(const char* label, a_unicode_string str,
			     size_t len, int ind);
static void p_locator(const char* label, a_source_locator_ptr loc, int ind);
static void p_package(a_package_ptr p, int ind);
static void p_reference_type(a_reference_type_ptr p, int ind);
static void p_method(a_method_ptr p, int ind);
static void p_initialization_block(an_initialization_block_ptr p, int ind);
static void p_variable(a_variable_ptr p, int ind);
static void p_label(a_label_ptr p, int ind);
static void p_overload_set(an_overload_set_ptr p, int ind);
static void p_label_and_str(const char* label, const char* str, int ind);
static void p_name_reference_component(a_name_reference_component_ptr p,
				       int ind);
static void p_modifier_set(const char* label, a_modifier_set set, int ind);
static void p_class_or_interface_reference(a_class_or_interface_reference_ptr p,
					   int ind);
static void p_reference_type_addendum(a_reference_type_addendum_ptr p, int ind);
static void p_type_reference(a_type_reference_ptr p, int ind);
static void p_type(a_type_ptr p, int ind);
static void p_statement(a_statement_ptr p, int ind);
static void p_method_addendum(a_method_addendum_ptr p, int ind);
static void p_variable_initializer(a_variable_initializer_ptr p, int ind);
static void p_variable_addendum(a_variable_addendum_ptr p, int ind);
static void p_token_string(a_token_string_ptr p, int ind);
static void p_parse_modifier(a_parse_modifier_ptr p, int ind);
static void p_expr(an_expr_ptr p, int ind);
static void p_switch_block_statement_group(a_switch_block_statement_group_ptr p,
					   int ind);
static void p_label_reference(a_label_reference_ptr p, int ind);
static void p_catch_clause(a_catch_clause_ptr p, int ind);
static void p_constant(a_constant_ptr p, int ind);
static void p_literal(a_literal_ptr p, int ind);
static void p_unsigned_int(const char* label, unsigned int val, int ind);
static void p_switch_label(a_switch_label_ptr p, int ind);
static void p_compilation_unit(a_compilation_unit_ptr p, int ind);
static void p_package_reference(a_package_reference_ptr p, int ind);
static void p_import_declaration(an_import_declaration_ptr p, int ind);
static void p_file_list_entry(a_file_list_entry_ptr p, int ind);
static void p_SET_info(SET_info_t *p, int ind);

static void record_scope_dump(a_scope_ptr p);
static void record_scope_reference(a_scope_ptr p);
static a_scope_ptr find_undumped_scope();
static void record_declaration_dump(a_declaration_ptr p);
static void record_declaration_reference(a_declaration_ptr p);
static a_declaration_ptr find_undumped_declaration();

/* Routine implementations. */

static void p_ind(int ind) {
    int i;
    for (i = 0; i < ind; i++) {
	printf("| ");
    }
}

static void p_label_str(const char* str, int ind) {
    p_ind(ind);
    printf("%s\n", str);
}

static void p_label_and_ptr(const char* str, const void* p, int ind) {
    p_ind(ind);
    printf("%s %p\n", str, p);
}

static void p_scope(a_scope_ptr p, int ind) {
    a_declaration_ptr dp;
    a_name_reference_ptr rp;
    a_scope_ptr ssp;
    record_scope_dump(p);
    p_label_and_ptr("SCOPE @", p, ind);
    switch (p->kind) {
    case sk_root:
	p_label_str("kind = root", ind);
	break;
    case sk_compilation_unit:
	/* ?? no pointer ?? */
	p_label_str("kind = compilation_unit", ind);
	break;
    case sk_package:
	p_label_and_ptr("kind = package", p->assoc.package, ind);
	break;
    case sk_reference_type:
	p_label_and_ptr("kind = reference_type", p->assoc.reference_type, ind);
	break;
    case sk_method:
	p_label_and_ptr("kind = method", p->assoc.method, ind);
	break;
    case sk_block:
	p_label_and_ptr("kind = block", p->assoc.statement, ind);
	break;
    default:
	p_label_and_ptr("kind = ***unexpected***", NULL, ind);
	break;
    }
    p_label_and_ptr("parent_scope =", p->parent_scope, ind);
    record_scope_reference(p->parent_scope);
    p_label_and_ptr("method_or_init_block_decl =",
		    p->method_or_init_block_decl, ind);
    for (ssp = p->subscopes; ssp; ssp = ssp->next) {
	p_label_and_ptr("subscope =", ssp, ind);
        record_scope_reference(ssp);
    }
    p_label_and_ptr("next =", p->next, ind);
    p_label_str("declarations:", ind);
    for (dp = p->declarations; dp; dp = dp->next) {
	p_declaration(dp, ind + IND_INCR);
	if (dp == p->last_declaration) {
	    break;
	}
    }
    p_label_str("references:", ind);
    for (rp = p->references; rp; rp = rp->next) {
	p_full_name_reference(rp, ind + IND_INCR);
	if (rp == p->last_reference) {
	    break;
	}
    }
}

static void p_boolean(const char* label, a_boolean flag, int ind, a_boolean last) {
    p_ind(ind);
    if (flag) {
	printf("%s", label);
    }
    else printf("!%s", label);
    if (last) {
	printf("\n");
    }
}

static void add_boolean(const char* label, a_boolean flag, a_boolean last) {
    if (flag) {
	printf(", %s", label);
    }
    else printf(", !%s", label);
    if (last) {
	printf("\n");
    }
}

const char* decl_name[] = {
    "PACKAGE @", "CLASS @", "INTERFACE @", "ARRAY @", "METHOD @", 
    "INITIALIZATION_BLOCK @", "VARIABLE @", "LABEL @", "OVERLOAD_SET @",
    "UNKNOWN @"
};

static void p_declaration(a_declaration_ptr p, int ind) {
    record_declaration_dump(p);
    if (((int) p->kind) >= 0 && ((int) p->kind) < dk_last) {
	p_label_and_ptr(decl_name[p->kind], p, ind);
    }
    else p_label_and_ptr("***unexpected declaration kind***", p, ind);
    p_label_and_ptr("decl.next =", p->next, ind);
    p_boolean("decl.is_error", p->is_error, ind, TRUE);
    p_unicode_string("decl.name =", p->name, p->name_length, ind);
    p_label_and_ptr("decl.scope =", p->scope, ind);
    record_scope_reference(p->scope);
    p_locator("decl.location =", &p->location, ind);
    p_SET_info(&p->SET_info, ind);
    switch (p->kind) {
    case dk_package:
	p_package((a_package_ptr) p, ind);
	break;
    case dk_class:
    case dk_interface:
    case dk_array:
	p_reference_type((a_reference_type_ptr) p, ind);
	break;
    case dk_method:
	p_method((a_method_ptr) p, ind);
	break;
    case dk_initialization_block:
	p_initialization_block((an_initialization_block_ptr) p, ind);
	break;
    case dk_variable:
	p_variable((a_variable_ptr) p, ind);
	break;
    case dk_label:
	p_label((a_label_ptr) p, ind);
	break;
    case dk_overload_set:
	p_overload_set((an_overload_set_ptr) p, ind);
	break;
    default:
	break;
    }
}

static char* rk_name[] = {
    "unknown", "package_reference", "type_reference", "type_reference_array",
    "class_reference", "interface_reference", "class_or_interface_reference",
    "import_declaration", "label_reference", "expr_variable", "expr_method"
};

/* By the time we dump, a name reference is normally resolved to
 * a particular kind of name reference, and is dumped via 
 * p_class_or_interface_reference, p_package_reference, etc. 
 * Therefore, p_name_reference does not need to (and would run into trouble
 * if it did) call these functions to dump the associated specific reference.
 *
 * The exception to this is in scopes, where the name references are
 * retained as an undifferentiated list.  p_full_name_reference serves
 * to dump name references from scopes, by invoking the specific name
 * reference dumper that applies.
 */
static void p_full_name_reference(a_name_reference_ptr p, int ind) {
    a_boolean substitute = FALSE;
    switch (p->kind) {
    case rk_package_reference:
    	substitute = p->ref.package_ref != NULL;
    	break;
    case rk_type_reference:
    case rk_type_reference_array:
	substitute = p->ref.type_ref != NULL;
	break;
    case rk_class_reference:
    case rk_interface_reference:
    case rk_class_or_interface_reference:
	substitute = p->ref.class_or_interface_ref != NULL;
	break;
    case rk_import_declaration:
	substitute = p->ref.import_decl != NULL;
	break;
    case rk_label_reference:
	substitute = p->ref.label_ref != NULL;
	break;
    }
    if (substitute) {
        p_label_and_ptr("NAME_REFERENCE @", p, ind);
	switch (p->kind) {
	case rk_package_reference:
	    p_label_str("ref.package_ref:", ind);
    	    p_package_reference(p->ref.package_ref, ind + IND_INCR);
	    break;
	case rk_type_reference:
	case rk_type_reference_array:
	    p_label_str("ref.type_ref:", ind);
            p_type_reference(p->ref.type_ref, ind + IND_INCR);
	    break;
	case rk_class_reference:
	case rk_interface_reference:
	case rk_class_or_interface_reference:
	    p_label_str("ref.class_or_interface_ref:", ind);
	    p_class_or_interface_reference(p->ref.class_or_interface_ref,
                                           ind + IND_INCR);
	    break;
	case rk_import_declaration:
	    p_label_str("ref.import_decl:", ind);
            p_import_declaration(p->ref.import_decl, ind + IND_INCR);
	    break;
	case rk_label_reference:
	    p_label_str("ref.label_ref:", ind);
            p_label_reference(p->ref.label_ref, ind + IND_INCR);
	    break;
	}
    }
    else {
    	p_name_reference(p, ind);
    }
}

static void p_name_reference(a_name_reference_ptr p, int ind) {
    p_label_and_ptr("NAME_REFERENCE @", p, ind);
    if (p != NULL) {
	p_label_and_ptr("next =", p->next, ind);
	if (((int) p->kind) >= 0 && ((int) p->kind) <= rk_expr_method) {
	    p_label_and_str("kind =", rk_name[p->kind], ind);
	}
	else p_label_and_str("kind = ", "***unexpected***", ind);
	p_boolean("is_error", p->is_error, ind, FALSE);
	add_boolean("resolved", p->resolved, FALSE);
	add_boolean("resolution_pending", p->resolution_pending, FALSE);
	add_boolean("fully_qualified", p->fully_qualified, FALSE);
	add_boolean("defer_name_resolution", p->defer_name_resolution, TRUE);
	/* NOTE: some of the following will probably be changed to print the
	 * entire referenced entity rather than just the pointer, but the
	 * pointer is all we print at the moment to guarantee non-circularity
	 * until the IL structure is better understood.
	 */
	switch (p->kind) {
	case rk_package_reference:
	    /* Dump package reference here, because packages are the entities
	     * which are not dumped in any other way.  Don't do it by calling
	     * p_package_reference, because that calls p_name_reference and
	     * causes infinite recursion. */
	    p_label_str("ref.package_ref:", ind);
	    p_label_and_ptr("PACKAGE_REFERENCE @", p->ref.package_ref,
						   ind + IND_INCR);
	    p_label_and_ptr("package =", p->ref.package_ref->package,
					 ind + IND_INCR);
	    record_declaration_reference(&p->ref.package_ref->package->decl);
	    p_label_and_ptr("name_ref =", p->ref.package_ref->name_ref,
					  ind + IND_INCR);
	    break;
	case rk_type_reference:
	case rk_type_reference_array:
	    p_label_and_ptr("ref.type_ref =", p->ref.type_ref, ind);
	    break;
	case rk_class_reference:
	case rk_interface_reference:
	case rk_class_or_interface_reference:
	    p_label_and_ptr("ref.class_or_interface_ref =",
			    p->ref.class_or_interface_ref, ind);
	    break;
	case rk_import_declaration:
	    p_label_and_ptr("ref.import_decl =", p->ref.import_decl, ind);
	    break;
	case rk_label_reference:
	    p_label_and_ptr("ref.label_ref =", p->ref.label_ref, ind);
	    break;
	case rk_expr_variable:
	case rk_expr_method:
	    p_label_and_ptr("ref.expr =", p->ref.expr, ind);
	    break;
	default:
	    break;
	}
	p_label_str("name:", ind);
	p_name_reference_component(&p->name, ind + IND_INCR);
    }
}

static void p_unicode_string(const char* label, a_unicode_string str,
			     size_t len, int ind) {
    int i;
    a_unicode_string p;
    p_ind(ind);
    printf("%s \"", label);
    for (p = str, i = 0; i < len; i++, p++) {
	printf("%c", *p);
    }
    printf("\"\n");
    p_ind(ind);
    printf("    \"");
    for (p = str, i = 0; i < len; i++, p++) {
	printf("\\u%04x", *p);
    }
    printf("\"\n");
}

static void p_locator(const char* label, a_source_locator_ptr loc, int ind) {
    p_ind(ind);
    if (loc != NULL) {
	printf("%s %d/%d-%d/%d\n", label, loc->start_sequence,
               loc->start_column,
	       loc->end_sequence, loc->end_column);
    }
    else {
	printf("%s <null locator>\n", label);
    }
}

static void p_package(a_package_ptr p, int ind) {
    /* "decl" already printed. */
    p_label_and_ptr("scope =", p->scope, ind);
    record_scope_reference(p->scope);
}

static void p_reference_type(a_reference_type_ptr p, int ind) {
    a_class_or_interface_reference_ptr cirp;
    /* "decl" already printed. */
    p_locator("location =", &p->location, ind);
    p_modifier_set("modifiers =", p->modifiers, ind);
    p_boolean("is_top_level", p->is_top_level, ind, FALSE);
    add_boolean("is_local", p->is_local, FALSE);
    add_boolean("is_throwable", p->is_throwable, FALSE);
    add_boolean("is_checked_exception", p->is_checked_exception, TRUE);
    p_label_str("superclass:", ind);
    for (cirp = p->superclass; cirp; cirp = cirp->next) {
	p_class_or_interface_reference(cirp, ind + IND_INCR);
    }
    p_label_str("interfaces:", ind);
    for (cirp = p->interfaces; cirp; cirp = cirp->next) {
	p_class_or_interface_reference(cirp, ind + IND_INCR);
    }
    if (p->scope) {
	p_label_str("scope:", ind);
	p_scope(p->scope, ind + IND_INCR);
    }
    else p_label_and_ptr("scope =", NULL, ind);
    p_label_and_ptr("overload_sets <TBD> =", p->overload_sets, ind);
    p_label_and_ptr("constructors <TBD> =", p->constructors, ind);
    p_label_and_ptr("component_type =", p->component_type, ind);
    p_label_str("type:", ind);
    p_type(p->type, ind + IND_INCR);
    p_label_and_ptr("compilation_unit =", p->compilation_unit, ind);
    if (p->parse_info) {
	p_label_str("parse_info:", ind);
	p_reference_type_addendum(p->parse_info, ind + IND_INCR);
    }
    else p_label_and_ptr("parse_info =", NULL, ind);
}

static void p_method(a_method_ptr p, int ind) {
    a_class_or_interface_reference_ptr cirp;
    /* "decl" already printed */
    p_locator("location =", &p->location, ind);
    p_boolean("is_constructor", p->is_constructor, ind, FALSE);
    add_boolean("invokes_same_class_constructor",
		p->invokes_same_class_constructor, FALSE);
    add_boolean("compiler_generated", p->compiler_generated, TRUE);
    p_modifier_set("modifiers =", p->modifiers, ind);
    if (p->return_type){ 
	p_label_str("return_type:", ind);
	p_type_reference(p->return_type, ind + IND_INCR);
    }
    else p_label_and_ptr("return_type =", NULL, ind);
    if (p->scope) {
	p_label_str("scope:", ind);
	p_scope(p->scope, ind + IND_INCR);
    }
    else p_label_and_ptr("scope =", p->scope, ind);
    p_label_str("throws:", ind);
    for (cirp = p->throws; cirp; cirp = cirp->next) {
	p_class_or_interface_reference(cirp, ind + IND_INCR);
    }
    p_label_and_ptr("overload_set =", p->overload_set, ind);
    if (p->block) {
	p_label_str("block:", ind);
	p_statement(p->block, ind + IND_INCR);
    }
    else p_label_and_ptr("block =", NULL, ind);
    if (p->parse_info) {
	p_label_str("parse_info:", ind);
	p_method_addendum(p->parse_info, ind + IND_INCR);
    }
    else p_label_and_ptr("parse_info =", NULL, ind);
}

static void p_initialization_block(an_initialization_block_ptr p, int ind) {
    /* "decl" already printed */
    p_boolean("is_static", p->is_static, ind, TRUE);
    p_statement(p->block, ind + IND_INCR);
}

static void p_variable(a_variable_ptr p, int ind) {
    /* "decl" already printed */
    p_locator("location =", &p->location, ind);
    p_modifier_set("modifiers =", p->modifiers, ind);
    p_boolean("is_parameter", p->is_parameter, ind, FALSE);
    add_boolean("is_definitely_assigned", p->is_definitely_assigned, FALSE);
    add_boolean("decl_follows_comma", p->decl_follows_comma, TRUE);
    if (p->type) {
	p_label_str("type:", ind);
	p_type_reference(p->type, ind + IND_INCR);
    }
    else p_label_and_ptr("type =", NULL, ind);
    if (p->initializer) {
	p_label_str("initializer:", ind);
	p_variable_initializer(p->initializer, ind + IND_INCR);
    }
    else p_label_and_ptr("initializer =", NULL, ind);
    if (p->parse_info) {
	p_label_str("parse_info:", ind);
	p_variable_addendum(p->parse_info, ind + IND_INCR);
    }
    else p_label_and_ptr("parse_info =", NULL, ind);
}

static void p_label(a_label_ptr p, int ind) {
    /* "decl" already printed */
    p_label_and_ptr("next_on_statement =", p->next_on_statement, ind);
}

static void p_overload_set(an_overload_set_ptr p, int ind) {
    /* "decl" already printed */
    /* TBD */
}

static void p_label_and_str(const char* label, const char* str, int ind) {
    p_ind(ind);
    printf("%s %s\n", label, str != NULL ? str : "<NULL>");
}

static void p_name_reference_component(a_name_reference_component_ptr p,
				       int ind) {
    if (p->token_string) {
	p_label_str("token_string:", ind);
	p_token_string(p->token_string, ind + IND_INCR);
    }
    else p_label_and_ptr("token_string =", NULL, ind);
    p_locator("location =", &p->location, ind);
    if (p->qualifier) {
	p_label_str("qualifier:", ind);
	p_name_reference_component(p->qualifier, ind + IND_INCR);
    }
    else p_label_and_ptr("qualifier =", NULL, ind);
}

static void p_modifier_set(const char* label, a_modifier_set set, int ind) {
    p_ind(ind);
    printf("%s", label);
    if (set & MK_PUBLIC) { printf(" public"); }
    if (set & MK_PROTECTED) { printf(" protected"); }
    if (set & MK_PRIVATE) { printf(" private"); }
    if (set & MK_FINAL) { printf(" final"); }
    if (set & MK_STATIC) { printf(" static"); }
    if (set & MK_SYNCHRONIZED) { printf(" synchronized"); }
    if (set & MK_ABSTRACT) { printf(" abstract"); }
    if (set & MK_NATIVE) { printf(" native"); }
    if (set & MK_TRANSIENT) { printf(" transient"); }
    if (set & MK_VOLATILE) { printf(" volatile"); }
    printf("\n");
}

static void p_class_or_interface_reference(a_class_or_interface_reference_ptr p,
					   int ind) {
    p_label_and_ptr("CLASS_OR_INTERFACE_REFERENCE @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_label_and_ptr("class_or_interface =", p->class_or_interface, ind);
    record_declaration_reference(&p->class_or_interface->decl);
    if (p->name_ref) {
	p_label_str("name_ref:", ind);
	p_name_reference(p->name_ref, ind + IND_INCR);
    }
    else p_label_and_ptr("name_ref =", NULL, ind);
}

static void p_reference_type_addendum(a_reference_type_addendum_ptr p,
				      int ind) {
    a_parse_modifier_ptr pmp;
    p_label_and_ptr("REFERENCE_TYPE_ADDENDUM @", p, ind);
    p_label_str("modifiers:", ind);
    for (pmp = p->modifiers; pmp; pmp = pmp->next) {
	p_parse_modifier(pmp, ind + IND_INCR);
    }
    p_locator("extends_location =", &p->extends_location, ind);
    p_locator("implements_location =", &p->implements_location, ind);
    p_locator("body_location =", &p->body_location, ind);
    if (p->anon_class_constructor_throws) {
	p_label_str("anon_class_constructor_throws:", ind);
	p_class_or_interface_reference(p->anon_class_constructor_throws,
				   ind + IND_INCR);
    }
    else p_label_and_ptr("anon_class_constructor_throws =", NULL, ind);
}

static void p_type_reference(a_type_reference_ptr p, int ind) {
    p_label_and_ptr("TYPE_REFERENCE @", p, ind);
    p_label_str("type:", ind);
    p_type(p->type, ind + IND_INCR);
    if (p->component_type_ref) {
	p_label_str("component_type_ref:", ind);
	p_type_reference(p->component_type_ref, ind + IND_INCR);
    }
    if (is_reference_type_reference(p)) {
    	if (p->variant.name_ref) {
	    p_label_str("variant.name_ref:", ind);
	    p_name_reference(p->variant.name_ref, ind + IND_INCR);
	}
    	else p_label_and_ptr("variant.name_ref =", NULL, ind);
    }
    else p_locator("variant.location =", p->variant.location, ind);
}

static void p_type(a_type_ptr p, int ind) {
    a_boolean needs_reference_type = FALSE;

    p_label_and_ptr("TYPE @", p, ind);
    if (p != NULL) {
    	p_SET_info(&p->SET_info, ind);
    	switch (p->kind) {
    	case tk_none:
	    p_label_str("kind = root", ind);
    	    break;
    	case tk_error:
    	    p_label_str("kind = error", ind);
    	    break;
    	case tk_void:
    	    p_label_str("kind = void", ind);
    	    break;

    	case tk_boolean:
    	    p_label_str("kind = boolean", ind);
    	    break;
    	case tk_byte:
    	    p_label_str("kind = byte", ind);
    	    break;
    	case tk_short:
    	    p_label_str("kind = short", ind);
    	    break;
    	case tk_int:
    	    p_label_str("kind = int", ind);
    	    break;
    	case tk_long:
    	    p_label_str("kind = long", ind);
    	    break;
    	case tk_char:
    	    p_label_str("kind = char", ind);
    	    break;
    	case tk_float:
    	    p_label_str("kind = float", ind);
    	    break;
    	case tk_double:
    	    p_label_str("kind = double", ind);
    	    break;

    	case tk_class:
    	    p_label_str("kind = class", ind);
    	    needs_reference_type = TRUE;
    	    break;
    	case tk_interface:
    	    p_label_str("kind = interface", ind);
    	    needs_reference_type = TRUE;
    	    break;
    	case tk_array:
    	    p_label_str("kind = array", ind);
    	    needs_reference_type = TRUE;
    	    break;
    	case tk_null:
    	    p_label_str("kind = null", ind);
    	    break;

	default:
	    p_label_str("kind = ***unexpected***", ind);
	    break;
        } /* switch */
    	if (needs_reference_type) {
    	    p_label_and_ptr("reference_type =", p->reference_type, ind);
    	    if (p->reference_type != NULL) {
    	    	record_declaration_reference(&p->reference_type->decl);
	    }
	}
    	if (p->assoc_array_type != NULL) {
    	    p_label_and_ptr("assoc_array_type = ", p->assoc_array_type, ind);
        }
    }
}

static char* stmt_name[] = {
    "ERROR @", "BLOCK @", "DECL @", "EMPTY @", "EXPR @", "IF @", "SWITCH @",
    "WHILE @", "DO_WHILE @", "FOR @", "BREAK @", "CONTINUE @", "RETURN @",
    "THROW @", "SYNCHRONIZED @", "TRY @"
};

static void p_statement(a_statement_ptr p, int ind) {
    a_statement_ptr sp;
    a_declaration_ptr dp;
    a_switch_block_statement_group_ptr sbsgp;
    an_expr_ptr ep;
    a_catch_clause_ptr ccp;
    if (((int) p->kind) >= 0 && ((int) p->kind) <= stmk_try) {
	p_label_and_ptr(stmt_name[p->kind], p, ind);
    }
    else p_label_and_ptr("***unexpected stmt kind*** @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_locator("location =", &p->location, ind);
    p_label_and_ptr("label =", p->label, ind);
    p_boolean("reachable", p->reachable, ind, FALSE);
    add_boolean("can_complete_normally", p->can_complete_normally, TRUE);
    switch (p->kind) {
    case stmk_block:
	p_label_str("variant.block.statements:", ind);
	for (sp = p->variant.block.statements; sp; sp = sp->next) {
	    p_statement(sp, ind + IND_INCR);
	}
	if (p->variant.block.scope) {
	    p_label_str("variant.block.scope:", ind);
	    p_scope(p->variant.block.scope, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.block.scope =", NULL, ind);
	break;
    case stmk_decl:
	p_label_str("variant.decl:", ind);
	for (dp = p->variant.decl.first; dp; dp = dp->next) {
	    p_declaration(dp, ind + IND_INCR);
	    if (dp == p->variant.decl.last) {
		break;
	    }
	}
	break;
    case stmk_expr:
	p_label_str("variant.expr:", ind);
	p_expr(p->variant.expr, ind + IND_INCR);
	break;
    case stmk_if:
	p_label_str("variant.if_stmt.expr:", ind);
	p_expr(p->variant.if_stmt.expr, ind + IND_INCR);
	p_label_str("variant.if_stmt.then_statement:", ind);
	p_statement(p->variant.if_stmt.then_statement, ind + IND_INCR);
	if (p->variant.if_stmt.else_statement) {
	    p_label_str("variant.if_stmt.else_statement:", ind);
	    p_statement(p->variant.if_stmt.else_statement, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.if_stmt.else_statement =", NULL, ind);
	p_locator("variant.if_stmt.else_location =",
		  &p->variant.if_stmt.else_location, ind);
	break;
    case stmk_switch:
	p_label_str("variant.switch_stmt.expr:", ind);
	p_expr(p->variant.switch_stmt.expr, ind + IND_INCR);
	p_label_str("variant.switch_stmt.groups:", ind);
	for (sbsgp = p->variant.switch_stmt.groups; sbsgp; sbsgp = sbsgp->next) {
	    p_switch_block_statement_group(sbsgp, ind + IND_INCR);
	}
	p_label_str("variant.switch_stmt.block:", ind);
	p_statement(p->variant.switch_stmt.block, ind + IND_INCR);
	break;
    case stmk_while:
    case stmk_do_while:
	p_label_str("variant.while_stmt.expr:", ind);
	p_expr(p->variant.while_stmt.expr, ind + IND_INCR);
	p_label_str("variant.while_stmt.statement:", ind);
	p_statement(p->variant.while_stmt.statement, ind + IND_INCR);
	p_locator("variant.while_stmt.while_location",
		  &p->variant.while_stmt.while_location, ind);
	break;
    case stmk_for:
	p_boolean("variant.for_stmt.for_init_is_decl",
		  p->variant.for_stmt.for_init_is_decl, ind, TRUE);
	if (p->variant.for_stmt.for_init_is_decl) {
	    p_label_str("variant.for_stmt.init.decl.stmt:", ind);
	    p_statement(p->variant.for_stmt.init.decl.stmt, ind + IND_INCR);
	    p_label_and_ptr("variant.for_stmt.init.decl.scope =",
			    p->variant.for_stmt.init.decl.scope, ind);
            record_scope_reference(p->variant.for_stmt.init.decl.scope);
	}
	else {
	    p_label_str("variant.for_stmt.init.exprs:", ind);
	    for (ep = p->variant.for_stmt.init.exprs; ep; ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	p_label_str("variant.for_stmt.expr:", ind);
	p_expr(p->variant.for_stmt.expr, ind + IND_INCR);
	p_label_str("variant.for_stmt.updates:", ind);
	for (ep = p->variant.for_stmt.updates; ep; ep = ep->next) {
	    p_expr(ep, ind + IND_INCR);
	}
	p_label_str("variant.for_stmt.statement:", ind);
	p_statement(p->variant.for_stmt.statement, ind + IND_INCR);
	break;
    case stmk_break:
    case stmk_continue:
	if (p->variant.break_continue.label) {
	    p_label_str("variant.break_continue.label:", ind);
	    p_label_reference(p->variant.break_continue.label, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.break_continue.label =", NULL, ind);
	p_label_and_ptr("variant.break_continue.statement =",
			p->variant.break_continue.statement, ind);
	break;
    case stmk_return:
	if (p->variant.return_expr) {
	    p_label_str("variant.return_expr:", ind);
	    p_expr(p->variant.return_expr, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.return_expr =", NULL, ind);
	break;
    case stmk_throw:
	p_label_str("variant.throw_expr:", ind);
	p_expr(p->variant.throw_expr, ind + IND_INCR);
	break;
    case stmk_synchronized:
	p_label_str("variant.synchronized.expr:", ind);
	p_expr(p->variant.synchronized.expr, ind + IND_INCR);
	p_label_str("variant.synchronized.block:", ind);
	p_statement(p->variant.synchronized.block, ind + IND_INCR);
	break;
    case stmk_try:
	p_label_str("variant.try_stmt.block:", ind);
	p_statement(p->variant.try_stmt.block, ind + IND_INCR);
	p_label_str("variant.try_stmt.catch_clauses:", ind);
	for (ccp = p->variant.try_stmt.catch_clauses; ccp; ccp = ccp->next) {
	    p_catch_clause(ccp, ind + IND_INCR);
	}
	if (p->variant.try_stmt.finally_block) {
	    p_label_str("variant.try_stmt.finally_block:", ind);
	    p_statement(p->variant.try_stmt.finally_block, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.try_stmt.finally_block =", NULL, ind);
	p_locator("variant.try_stmt.finally_location =",
		  &p->variant.try_stmt.finally_location, ind);
	break;
    case stmk_error:
    case stmk_empty:
    default:
	break;
    }
}

static void p_method_addendum(a_method_addendum_ptr p, int ind) {
    a_parse_modifier_ptr pmp;
    p_label_and_ptr("METHOD_ADDENDUM @", p, ind);
    p_label_str("modifiers:", ind);
    for (pmp = p->modifiers; pmp; pmp = pmp->next) {
	p_parse_modifier(pmp, ind + IND_INCR);
    }
    p_locator("parameter_list_location =", &p->parameter_list_location, ind);
    p_locator("throws_location =", &p->throws_location, ind);
}

static void p_variable_initializer(a_variable_initializer_ptr p, int ind) {
    p_label_and_ptr("VARIABLE_INITIALIZER @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_boolean("is_array_initializer", p->is_array_initializer, ind, TRUE);
    if (p->is_array_initializer) {
	a_variable_initializer_ptr vip;
	p_label_str("variant.component_inits:", ind);
	for (vip = p->variant.component_inits; vip; vip = vip->next) {
	    p_variable_initializer(vip, ind + IND_INCR);
	}
    }
    else {
	p_label_str("variant.expr:", ind);
	p_expr(p->variant.expr, ind + IND_INCR);
    }
    p_locator("location =", p->location, ind);
    p = p->next;
}

static void p_variable_addendum(a_variable_addendum_ptr p, int ind) {
    a_parse_modifier_ptr pmp;
    p_label_and_ptr("VARIABLE_ADDENDUM @", p, ind);
    p_label_str("modifiers:", ind);
    for (pmp = p->modifiers; pmp; pmp = pmp->next) {
	p_parse_modifier(pmp, ind + IND_INCR);
    }
}

static void p_token_string(a_token_string_ptr p, int ind) {
    int i;
    a_unicode_string usp;
    p_label_and_ptr("TOKEN_STRING @", p, ind);
    p_unicode_string("unicode_string =", p->unicode_string,
		     p->unicode_length, ind);
    p_label_and_str("token_kind =", token_names[p->token_kind], ind);
    if (p->constant) {
	p_label_str("constant:", ind);
	p_constant(p->constant, ind + IND_INCR);
    }
    else p_label_and_ptr("constant =", NULL, ind);
}

static char* modifier_name[] = {
    "public", "protected", "private", "final", "static", "synchronized",
    "abstract", "native", "transient", "volatile"
};

static void p_parse_modifier(a_parse_modifier_ptr p, int ind) {
    p_locator(modifier_name[p->kind], &p->location, ind);
}

static char* expr_name[] = {
    "error", "constant", "variable", "this", "super", "class_new",
    "array_new", "cast", "call", "expl_constr_invoc", "instanceof",
    "operation", "class_literal"
};

static void p_expr(an_expr_ptr p, int ind) {
    an_expr_ptr ep;
    p_label_and_ptr("EXPR @", p, ind);
    p_label_and_str("kind = ", expr_name[p->kind], ind);
    p_label_str("type:", ind);
    p_type(p->type, ind + IND_INCR);
    p_locator("location =", p->location, ind);
    switch(p->kind) {
    case ek_constant:
	p_label_str("variant.constant.ptr:", ind);
	p_constant(p->variant.constant.ptr, ind + IND_INCR);
	if (p->variant.constant.expr) {
	    p_label_str("variant.constant.expr:", ind);
	    p_expr(p->variant.constant.expr, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.constant.expr =", NULL, ind);
	if (p->variant.constant.literal) {
	    p_label_str("variant.constant.literal:", ind);
	    p_literal(p->variant.constant.literal, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.constant.literal =", NULL, ind);
	break;
    case ek_variable:
	p_label_and_ptr("variant.variable.ptr =", p->variant.variable.ptr,
			ind);
	if (p->variant.variable.object) {
	    p_label_str("variant.variable.object:", ind);
	    p_expr(p->variant.variable.object, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.variable.object =", NULL, ind);
	p_label_str("variant.variable.name_ref:", ind);
	p_name_reference(p->variant.variable.name_ref, ind + IND_INCR);
	p_boolean("variant.variable.outer_local_variable_reference",
    	    	  p->variant.variable.outer_local_variable_reference,
                  ind + IND_INCR, TRUE);
	break;
    case ek_this:
        if (p->variant.this_expr.enclosing_class != NULL) {
            record_declaration_reference(
                &p->variant.this_expr.enclosing_class->decl);
	}
	p_label_and_ptr("variant.this_enclosing_class =",
                        p->variant.this_expr.enclosing_class, ind);
	if (p->variant.this_expr.enclosing_class_ref) {
	    p_label_str("variant.this_expr.enclosing_class_ref:", ind);
	    p_type_reference(p->variant.this_expr.enclosing_class_ref,
			     ind + IND_INCR);
	}
	else p_label_and_ptr("variant.this_enclosing_class_ref =", NULL, ind);
	break;
    case ek_class_new:
	if (p->variant.class_new.enclosing_object) {
	    p_label_str("variant.class_new.enclosing_object:", ind);
	    p_expr(p->variant.class_new.enclosing_object, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.class_new.enclosing_object =", NULL,
			     ind);
	if (p->variant.class_new.type) {
	    p_label_str("variant.class_new.type:", ind);
	    p_type_reference(p->variant.class_new.type, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.class_new.type =", NULL, ind);
	p_label_and_ptr("variant.class_new.constructor =",
			p->variant.class_new.constructor, ind);
	if (p->variant.class_new.argument_list) {
	    p_label_str("variant.class_new.argument_list:", ind);
	    for (ep = p->variant.class_new.argument_list; ep; ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	break;
    case ek_array_new:
	if (p->variant.array_new.element_type) {
	    p_label_str("variant.array_new.element_type:", ind);
	    p_type_reference(p->variant.array_new.element_type, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.array_new.element_type =", NULL, ind);
	p_unsigned_int("variant.array_new.num_dims =",
		       p->variant.array_new.num_dims, ind);
	if (p->variant.array_new.dim_exprs) {
	    p_label_str("variant.array_new.dim_exprs:", ind);
	    for (ep = p->variant.array_new.dim_exprs; ep; ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	else p_label_and_ptr("variant.array_new.dim_exprs =", NULL, ind);
	if (p->variant.array_new.initializer) {
	    p_label_str("variant.array_new.initializer:", ind);
	    p_variable_initializer(p->variant.array_new.initializer,
				   ind + IND_INCR);
	}
	else p_label_and_ptr("variant.array_new.initializer =", NULL, ind);
	break;
    case ek_cast:
	if (p->variant.cast.type) {
	    p_label_str("variant.cast.type:", ind);
	    p_type_reference(p->variant.cast.type, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.cast.type =", NULL, ind);
	p_label_str("variant.cast.operand:", ind);
	p_expr(p->variant.cast.operand, ind + IND_INCR);
	p_boolean("variant.cast.runtime_check_needed",
		    p->variant.cast.runtime_check_needed, ind, TRUE);
	break;
    case ek_call:
	p_label_and_ptr("variant.call.method =", p->variant.call.method, ind);
	if (p->variant.call.object) {
	    p_label_str("variant.call.object:", ind);
	    p_expr(p->variant.call.object, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.call.object =", NULL, ind);
	if (p->variant.call.argument_list) {
	    p_label_str("variant.call.argument_list:", ind);
	    for (ep = p->variant.call.argument_list; ep; ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	else p_label_and_ptr("variant.call.argument_list =", NULL, ind);
	p_label_str("variant.call.name_ref:", ind);
	p_name_reference(p->variant.call.name_ref, ind + IND_INCR);
	break;
    case ek_expl_constr_invoc:
	p_boolean("variant.expl_constr_invoc.is_super",
		  p->variant.expl_constr_invoc.is_super, ind, FALSE);
	add_boolean("variant.expl_constr_invoc.compiler_generated",
		    p->variant.expl_constr_invoc.compiler_generated, TRUE);
	if (p->variant.expl_constr_invoc.enclosing_object) {
	    p_label_str("variant.expl_constr_invoc.enclosing_object:", ind);
	    p_expr(p->variant.expl_constr_invoc.enclosing_object,
		   ind + IND_INCR);
	}
	else p_label_and_ptr("variant.expl_constr_invoc.enclosing_object =",
			     NULL, ind);
	p_label_and_ptr("variant.expl_constr_invoc.constructor =",
			p->variant.expl_constr_invoc.constructor, ind);
	if (p->variant.expl_constr_invoc.argument_list) {
	    p_label_str("variant.expl_constr_invoc.argument_list:", ind);
	    for (ep = p->variant.expl_constr_invoc.argument_list; ep;
		 ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	else p_label_and_ptr("variant.expl_constr_invoc.argument_list =",
			     NULL, ind);
	break;
    case ek_instanceof:
	p_label_str("variant.instanceof.operand:", ind);
	p_expr(p->variant.instanceof.operand, ind + IND_INCR);
	if (p->variant.instanceof.type) {
	    p_label_str("variant.instanceof.type:", ind);
	    p_type_reference(p->variant.instanceof.type, ind + IND_INCR);
	}
	else p_label_and_ptr("variant.instanceof.type =", NULL, ind);
	break;
    case ek_operation:
	p_label_and_str("variant.operation.op =",
			db_operator_names[p->variant.operation.op], ind);
	if (p->variant.operation.operands) {
	    p_label_str("variant.operation.operands:", ind);
	    for (ep = p->variant.operation.operands; ep; ep = ep->next) {
		p_expr(ep, ind + IND_INCR);
	    }
	}
	else p_label_and_ptr("variant.operation.operands =", NULL, ind);
	break;
    case ek_class_literal:
	if (p->variant.class_literal_type_ref) {
	    p_label_str("variant.class_literal_type_ref:", ind);
	    p_type_reference(p->variant.class_literal_type_ref,
			     ind + IND_INCR);
	}
	else p_label_and_ptr("variant.class_literal_type_ref =", NULL, ind);
	break;
    case ek_super:
    case ek_error:
    default:
	break;
    }
}

static void p_switch_block_statement_group(a_switch_block_statement_group_ptr p,
					   int ind) {
    a_switch_label_ptr slp;
    a_statement_ptr sp;
    p_label_str("labels:", ind);
    for (slp = p->labels; slp; slp = slp->next) {
	p_switch_label(slp, ind + IND_INCR);
    }
    p_label_str("statements:", ind);
    for (sp = p->statements; sp; sp = sp->next) {
	p_statement(sp, ind + IND_INCR);
    }
}

static void p_label_reference(a_label_reference_ptr p, int ind) {
    p_label_and_ptr("label =", p->label, ind);
    if (p->name_ref) {
	p_label_str("name_ref:", ind);
	p_name_reference(p->name_ref, ind + IND_INCR);
    }
    else p_label_and_ptr("name_ref =", NULL, ind);
}

static void p_catch_clause(a_catch_clause_ptr p, int ind) {
    p_label_and_ptr("CATCH_CLAUSE @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_locator("location =", &p->location, ind);
    p_label_and_ptr("param_variable =", p->param_variable, ind);
    p_label_str("block:", ind);
    p_statement(p->block, ind + IND_INCR);
}

static char* const_kind_name[] = {
    "error", "integer", "float", "boolean", "string", "null"
};

static char* type_kind_name[] = {
    "none", "error", "void", "boolean", "byte", "short", "int",
    "long", "char", "float", "double", "class", "interface", "array",
    "null"
};

static void p_constant(a_constant_ptr p, int ind) {
    int i;
    p_label_and_ptr("CONSTANT @", p, ind);
    if (((int) p->kind) >= 0 && ((int) p->kind) <= ck_null) {
	p_label_and_str("kind =", const_kind_name[p->kind], ind);
    }
    else p_label_and_str("kind =", "***unexpected const kind***", ind);
    if (((int) p->type_kind) >= 0 && ((int) p->type_kind) < tk_last) {
	p_label_and_str("type_kind =", type_kind_name[p->type_kind], ind);
    }
    else p_label_and_str("type_kind =", "***unexpected type kind***", ind);
    p_SET_info(&p->SET_info, ind);
    switch (p->kind) {
    case ck_integer:
	p_ind(ind);
	printf("variant.integer_value =");
	for (i = 0; i < 4; i++) {
	    printf("%04x", p->variant.integer_value.part[i]);
	}
	printf("\n");
	break;
    case ck_float:
	p_ind(ind);
	printf("variant.float_value =");
	for (i = 0; i < 8; i++) {
	    printf("%02x", p->variant.float_value.byte[i]);
	}
	printf("\n");
	break;
    case ck_boolean:
	p_label_and_str("variant.boolean_value =",
			(p->variant.boolean_value) ? "true" : "false",
			ind);
	break;
    case ck_string:
	p_unicode_string("variant.string =", p->variant.string.chars,
			 p->variant.string.length, ind);
	break;
    case ck_error:
    case ck_null:
    default:
	break;
    }
}

static void p_literal(a_literal_ptr p, int ind) {
    p_label_and_ptr("LITERAL @", p, ind);
    p_label_and_str("token_kind =", token_names[p->token_kind], ind);
    if (p->token_string) {
	p_label_str("token_string:", ind);
	p_token_string(p->token_string, ind + IND_INCR);
    }
    else p_label_and_ptr("token_string =", NULL, ind);
}

static void p_unsigned_int(const char* label, unsigned int val, int ind) {
    p_ind(ind);
    printf("%s %u\n", label, val);
}

static void p_switch_label(a_switch_label_ptr p, int ind) {
    p_locator("location =", &p->location, ind);
    if (p->expr) {
	p_label_str("expr:", ind);
	p_expr(p->expr, ind + IND_INCR);
    }
    else p_label_and_ptr("expr =", NULL, ind);
}

static void p_compilation_unit(a_compilation_unit_ptr p, int ind) {
    an_import_declaration_ptr idp;
    p_label_and_ptr("COMPILATION_UNIT @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_label_and_str("file_name =", p->file_name, ind);
    p_label_and_str("base_name =", p->base_name, ind);
    switch (p->kind) {
    case cuk_source_file:
    	p_label_str("kind = source file", ind);
    	break;
    case cuk_class_file:
    	p_label_str("kind = .class file", ind);
    	break;
    case cuk_jil_file:
    	p_label_str("kind = jil file", ind);
    	break;
    case cuk_stub_file:
    	p_label_str("kind = stub", ind);
    	break;
    case cuk_generated:
    	p_label_str("kind = generated", ind);
    	break;
    default:
	p_label_str("kind = ***unexpected***", ind);
        break;
    }
    p_boolean("specified_on_command_line", p->specified_on_command_line,
	      ind, TRUE);
    switch (p->kind) {
    case cuk_source_file:
	p_label_str("scope:", ind);
	p_scope(p->variant.source_file.scope, ind + IND_INCR);
	if (p->variant.source_file.package) {
	    p_label_str("package:", ind);
	    p_package_reference(p->variant.source_file.package,
                                ind + IND_INCR);
	}
	else p_label_and_ptr("package =", NULL, ind);
	p_label_str("imports:", ind);
	for (idp = p->variant.source_file.imports; idp; idp = idp->next) {
	    p_import_declaration(idp, ind + IND_INCR);
	}
        break;
    case cuk_class_file:
    case cuk_generated:
    	if (p->variant.class_file.reference_type != NULL) {
	    p_label_str("reference_type:", ind);
	    p_declaration(&p->variant.class_file.reference_type->decl, ind + IND_INCR);
	}
    	else {
    	    p_label_and_ptr("reference_type =", NULL, ind);
	}
    	break;
    default:
        break;
    }
    if (p->addendum) {
        if (p->addendum->file != NULL) {
    	    p_label_str("file:", ind);
    	    p_file_list_entry(p->addendum->file, ind + IND_INCR);
	}
    	else {
    	    p_label_and_ptr("file =", NULL, ind);
	}
    }
}

static void p_package_reference(a_package_reference_ptr p, int ind) {
    p_label_and_ptr("PACKAGE_REFERENCE @", p, ind);
    p_label_and_ptr("package =", p->package, ind);
    record_declaration_reference(&p->package->decl);
    if (p->name_ref) {
	p_label_str("name_ref:", ind);
	p_name_reference(p->name_ref, ind + IND_INCR);
    }
    else p_label_and_ptr("name_ref =", NULL, ind);
}

static void p_import_declaration(an_import_declaration_ptr p, int ind) {
    p_label_and_ptr("IMPORT_DECLARATION @", p, ind);
    p_label_and_ptr("next =", p->next, ind);
    p_boolean("import_on_demand", p->import_on_demand, ind, FALSE);
    add_boolean("duplicate", p->duplicate, TRUE);
    if (p->name_ref) {
	p_label_str("name_ref:", ind);
	p_name_reference(p->name_ref, ind + IND_INCR);
    }
    else p_label_and_ptr("name_ref =", NULL, ind);
    p_label_and_ptr("decl =", p->decl, ind);
}

static void p_file_list_entry(a_file_list_entry_ptr p, int ind) {
    p_label_and_ptr("FILE_LIST_ENTRY @", p, ind);
    p_SET_info(&p->SET_info, ind);
    p_boolean("virtual", p->virtual, ind, FALSE);
    p_label_and_str("file_name =", p->file_name, ind);
}

static void p_SET_info(SET_info_t *p, int ind) {
    static char *ks[] = { "declaration", "type", "constant", "file" };
    p_boolean("SET is_reference_type_decl", p->is_reference_type_decl, ind, TRUE);
    p_label_and_str("SET kind =", ks[p->kind], ind);
    p_ind(ind);
    printf("SET symid = [%lu]\n", p->symid);
}

static void record_scope_dump(a_scope_ptr p) {
    dump_scope_node *sp;
    record_scope_reference(p);
    sp = scopes_for_dumping;
    while (sp != NULL && sp->this != p) {
        sp = sp->next;
    }
    if (sp != NULL) {
        sp->dumped = TRUE;
    }
}

static void record_scope_reference(a_scope_ptr p) {
    dump_scope_node *sp = scopes_for_dumping;
    while (sp != NULL && sp->this != p) {
        sp = sp->next;
    }
    if (sp == NULL && p != NULL) {
        sp = scopes_for_dumping;
        scopes_for_dumping = (dump_scope_node *)malloc(sizeof(dump_scope_node));
        scopes_for_dumping->this = p;
        scopes_for_dumping->dumped = FALSE;
        scopes_for_dumping->next = sp;
    }
}

static a_scope_ptr find_undumped_scope() {
    dump_scope_node *sp = scopes_for_dumping;
    while (sp != NULL && sp->dumped) {
        sp = sp->next;
    }
    return (sp != NULL) ? sp->this : NULL;
}

static void record_declaration_dump(a_declaration_ptr p) {
    dump_declaration_node *sp;
    record_declaration_reference(p);
    sp = declarations_for_dumping;
    while (sp != NULL && sp->this != p) {
        sp = sp->next;
    }
    if (sp != NULL) {
        sp->dumped = TRUE;
    }
}

static void record_declaration_reference(a_declaration_ptr p) {
    dump_declaration_node *sp = declarations_for_dumping;
    while (sp != NULL && sp->this != p) {
        sp = sp->next;
    }
    if (sp == NULL && p != NULL) {
        sp = declarations_for_dumping;
        declarations_for_dumping = (dump_declaration_node *)malloc(sizeof(dump_declaration_node));
        declarations_for_dumping->this = p;
        declarations_for_dumping->dumped = FALSE;
        declarations_for_dumping->next = sp;
    }
}

static a_declaration_ptr find_undumped_declaration() {
    dump_declaration_node *sp = declarations_for_dumping;
    while (sp != NULL && sp->dumped) {
        sp = sp->next;
    }
    return (sp != NULL) ? sp->this : NULL;
}

void dump_il() {
    a_compilation_unit_ptr cup;
    a_scope_ptr sp;
    a_declaration_ptr dp;
    for (cup = compilation_units; cup; cup = cup->next) {
	if (is_being_emitted(cup)) {
	    p_compilation_unit(cup, 0);
	}
    	for (;;) {
    	    sp = find_undumped_scope();
    	    dp = find_undumped_declaration();
            if (sp == NULL && dp == NULL) {
    	    	break;
	    }
    	    if (dp != NULL) {
                p_declaration(dp, 0);
	    }
    	    else if (sp != NULL) {
    	    	p_scope(sp, 0);
	    }
	}
    }
    p_label_str("String_type:", 0);
    p_type(String_type(), IND_INCR);
    p_label_str("Object_type:", 0);
    p_type(Object_type(), IND_INCR);
}
