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
#include "SET_process_entry.h"
#include "SET_names.h"
#include "SET_ast.h"
#include "SET_il.h"
#include "SET_additions.h"
#include "SET_symid.h"
#include "SET_symbol.h"
#include "SET_complaints.h"
#include "il_to_str.h"
#include "cmd_line.h"

/* The following function can discover that a node is
 * interesting after it was categorized as uninteresting.
 * This happens because a non-definition declaration occurs in
 * an uninteresting file but then the entity is defined in an
 * interesting file.  We can detect this case by looking at
 * whether the source sequence entry associated with the entity
 * is classified as interesting.
 */
static a_boolean is_interesting_after_all(char *entry_ptr, an_il_entry_kind entry_kind) {
    switch (entry_kind) {
    case iek_constant: {
	a_constant_ptr constant_ptr = (a_constant_ptr) entry_ptr;
	if (constant_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(constant_ptr->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(constant_ptr).full_SET_dump = il_entry_prefix_of(constant_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_type: {
	a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	a_boolean is_typedef = FALSE;
	const char* kind;
	if (type_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(type_ptr->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(type_ptr).full_SET_dump = il_entry_prefix_of(type_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_variable: {
	a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	if (variable_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(variable_ptr->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(variable_ptr).full_SET_dump = il_entry_prefix_of(variable_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_field: {
	a_field_ptr field_ptr = (a_field_ptr) entry_ptr;
	if (field_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(field_ptr->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(field_ptr).full_SET_dump = il_entry_prefix_of(field_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_routine: {
	a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	intr_info_ptr routine_is_interesting = NULL;
	if (routine_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(routine_ptr->
			     source_corresp.source_sequence_entry)) {
	    routine_is_interesting = il_entry_prefix_of(routine_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	}
	else if (routine_ptr->assoc_scope != NULL_region_number &&
		 interesting_node(il_header.region_scope_entry[routine_ptr->
							       assoc_scope])) {
	    routine_is_interesting = il_entry_prefix_of(il_header.region_scope_entry[routine_ptr->assoc_scope]).full_SET_dump;
	}
	if (routine_is_interesting) {
	    il_entry_prefix_of(routine_ptr).full_SET_dump = routine_is_interesting;
	    return TRUE;
	}
    }
	break;

    case iek_label: {
	a_label_ptr label_ptr = (a_label_ptr) entry_ptr;
	if (label_ptr->source_corresp.name) {
	    if (label_ptr->source_corresp.source_sequence_entry &&
		interesting_node(label_ptr->
				 source_corresp.source_sequence_entry)) {
		il_entry_prefix_of(label_ptr).full_SET_dump = il_entry_prefix_of(label_ptr->source_corresp.source_sequence_entry).full_SET_dump;
		return TRUE;
	    }
	}
    }
	break;

    case iek_namespace: {
	a_namespace_ptr namespace_ptr = (a_namespace_ptr) entry_ptr;
	if (namespace_ptr->source_corresp.source_sequence_entry &&
	    interesting_node(namespace_ptr->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(namespace_ptr).full_SET_dump = il_entry_prefix_of(namespace_ptr->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_source_sequence_entry: {
	a_source_sequence_entry_ptr seq = (a_source_sequence_entry_ptr)
		entry_ptr;
	if (interesting_node(seq->entity.ptr)) {
	    il_entry_prefix_of(seq).full_SET_dump = il_entry_prefix_of(seq->entity.ptr).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    case iek_template: {
	a_template_ptr template = (a_template_ptr) entry_ptr;
	const char* kind;
	if (template->source_corresp.source_sequence_entry &&
	    interesting_node(template->
			     source_corresp.source_sequence_entry)) {
	    il_entry_prefix_of(template).full_SET_dump = il_entry_prefix_of(template->source_corresp.source_sequence_entry).full_SET_dump;
	    return TRUE;
	}
    }
	break;

    default:
	break;
    } /* switch */
    return FALSE;
} /* is_interesting_after_all */

static a_boolean fully_interesting_node(char *entry_ptr, an_il_entry_kind entry_kind) {
    if (generate_multiple_IF_files) {
	switch(entry_kind) {
	case iek_type:
            {
		a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
		a_boolean is_typedef = FALSE;
		unsigned long type_symid = 0;
		if (type_ptr->kind == tk_class ||
		    type_ptr->kind == tk_struct ||
		    type_ptr->kind == tk_union) {

		    adjust_class_struct_union_output_buffer(type_ptr);
		}
	    }
	    break;
	case iek_variable:
            {
		a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
		adjust_variable_output_buffer(variable_ptr);
	    }
	    break;
	case iek_routine:
            {
		a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
		adjust_routine_output_buffer(routine_ptr);
	    }
	    break;
	default:
	    break;
	}
    }
    return interesting_node(entry_ptr) || is_interesting_after_all(entry_ptr, entry_kind);
}

/* For a given il entry, match it with other il entries which are related
 * so that they should share the same id in the IF.  If any such il entries
 * are found, group them in a "clique".
 */
void befriend_clique(char *entry_ptr, an_il_entry_kind entry_kind) {
    switch (entry_kind) {
    case iek_type: {
	a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	a_boolean is_class_struct_union = (type_ptr->kind == tk_class ||
					   type_ptr->kind == tk_struct ||
					   type_ptr->kind == tk_union);

	/* If this is a prototype instantiation with a class-type-supplement,
	   and with an associated template, then identify symbols. */
	if (is_class_struct_union && type_ptr->variant.class_struct_union.is_prototype_instantiation) {
	    a_class_type_supplement_ptr extra = type_ptr->variant.class_struct_union.extra_info;
	    if (extra != NULL && extra->assoc_template != NULL) {
		attempt_same_symid(extra->assoc_template, iek_template, type_ptr, iek_type,
		    "Associated template [$1] should have the same id as class type [$2].");
	    }
	}
    }
	break;

    case iek_routine: {
	a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	a_boolean is_prototype_instance_fcn = routine_ptr->is_template_function &&
		!routine_ptr->is_specialized && 
		routine_ptr->is_prototype_instantiation;

	if (!routine_ptr->defined &&
	    routine_ptr->source_corresp.source_sequence_entry &&
	    routine_ptr->source_corresp.source_sequence_entry->entity.kind ==
	    iek_src_seq_secondary_decl) {
	    /* use the declared_type from the secondary decl if the function
	     * is explicitly declared but not defined; the reason is that the
	     * type here and the declared_type in the declaration are
	     * independent and will be duplicates (apart from things like
	     * accumulated default arguments and parameter type adjustments),
	     * so the walk of the routine type in this case is suppressed to
	     * avoid multiplying type info in the IF.  We also copy the
	     * declared_type from the secondary decl as the type of the
	     * function, to prevent problems later on with references to
	     * the type.
	     */
	    a_src_seq_secondary_decl_ptr sseq = (a_src_seq_secondary_decl_ptr)
		    routine_ptr->source_corresp.source_sequence_entry->
		    entity.ptr;
	    ensure_same_symid(routine_ptr->type, iek_type, sseq->declared_type, iek_type);
	}
	if (is_prototype_instance_fcn && routine_ptr->assoc_template != NULL) {
	    /* instance of a function template or a member function template, so it
	     * has its own template IL entry:  Avoid processing its SYM line twice.
	     */
	    a_template_ptr assoc_template = routine_ptr->assoc_template;
	    attempt_same_symid(routine_ptr, iek_routine, assoc_template, iek_template,
		"Template function [$1] should have the same symid as its associated template [$2].");
	}
    }
	break;

    case iek_template: {
	a_template_ptr template = (a_template_ptr) entry_ptr;
	a_symbol_ptr sym = (a_symbol_ptr) template->
		source_corresp.assoc_info;
	a_boolean is_auto_gen = is_auto_gen_member(&template->source_corresp);

	a_boolean is_canonical = (template == template->canonical_template);
	a_template_ptr definition_template = template->canonical_template != NULL
	    ? template->canonical_template->definition_template
	    : NULL;
	a_boolean is_definition = (definition_template == template);

	if (is_auto_gen || is_specialized(template)) {

	    /* Can't identify this template with the canonical declaration. */
	}
	else {
	    /* Identify this template with the canonical declaration. */
	    if (!is_canonical && template->canonical_template != NULL) {
		attempt_same_symid(template, iek_type, template->canonical_template, iek_template,
			"Template declaration [$1] should have the same id as its canonical declaration [$2].");
	    }
	    if (!is_definition && definition_template != NULL) {
		attempt_same_symid(template, iek_type, definition_template, iek_template,
			"Template declaration [$1] should have the same id as its definition [$2].");
	    }

	    /* If this template has a prototype-instantiated entry, then identify them. */
	    switch (template->kind) {
	      case templk_class:
	      case templk_member_class:
		if (template->prototype_instantiation.type != NULL) {
		    attempt_same_symid(template->prototype_instantiation.type, iek_type, template, iek_template,
			    "Prototype instantiation [$1] should have the same id as template [$2].");
		}
		break;
	      case templk_function:
	      case templk_member_function:
		if (template->prototype_instantiation.routine != NULL) {
		    a_template_ptr external_tmpl = has_tpl_entry(template->prototype_instantiation.routine);
		    ensure_same_symid(template->prototype_instantiation.routine, iek_routine, template, iek_template);
		}
		break;
	      case templk_static_data_member:
		if (template->prototype_instantiation.variable != NULL) {
		    a_template_ptr external_tmpl = variable_has_tpl_entry(template->prototype_instantiation.variable);
		    ensure_same_symid(template->prototype_instantiation.variable, iek_variable, template, iek_template);
		}
		break;
	      default:
		break;
	    }
	}
	if (template->kind == templk_class && sym != NULL) {
	    a_template_symbol_supplement_ptr template_info = sym->variant.template_info;
	    if (template_info &&
		template_info->variant.class_template.prototype_instantiation &&
		template_info->variant.class_template.prototype_instantiation
			     ->variant.class_struct_union.extra_info) {

		/* Make references to the prototype instantiation look like
		 * references to the template itself.
		 */
		a_type_ptr tp = template_info->variant.class_template.prototype_instantiation
					     ->variant.class_struct_union.type;
		attempt_same_symid(tp, iek_type, template, iek_template,
		    "Prototype instantiation symbol [$1] differs from template symbol [$2].");
	    }
	}
    }
	break;
    default:
	break;
    } /* switch */
} /* befriend_clique */

a_boolean skip_entry(char *entry_ptr, an_il_entry_kind entry_kind) {
    /* The following variable is set TRUE to indicate that AST should be
     * created for class members of templates, even though they appear
     * to be members of template instances (in this case, the prototype
     * instantiation).
     */
    a_boolean prototype_instance_member = entry_is_prototype_instance_member(entry_ptr, entry_kind);

    switch (entry_kind) {
    case iek_constant: {
	a_constant_ptr constant_ptr = (a_constant_ptr) entry_ptr;
	if (generate_multiple_IF_files) {
	  if (is_auto_gen_member(&constant_ptr->source_corresp)) {
	    return TRUE;
	  }
	  /* Skip writing of SYM, REL, etc but do AST because the code 
	     to handle, for example, switch statements assumes that the 
	     case constants have ast_node_info objects. The SYM line
	     will be written when the constant gets referenced.
	   */
	}

	if (constant_ptr->source_corresp.is_local_to_function &&
	    constant_ptr->source_corresp.name && !fcn_name) {
	    /* This is an enumerator that is local to a function but
	     * which was forced to the global scope for EDG
	     * implementation reasons.  In order to decorate the
	     * name correctly, we defer processing it until we
	     * encounter its true scope.
	     */
	    return TRUE;
	}
	if (constant_ptr->kind == ck_address &&
	    constant_ptr->variant.address.kind == abk_constant &&
	    constant_ptr->variant.address.variant.constant->kind == ck_string) {
	    /* This is a string address constant.  When it's used, the AST
	     * reference will be manipulated to refer to the string itself,
	     * not to the address constant, so there's no need for this to
	     * be a symbol.
	     */
	    return TRUE;
	}
    }
	break;

    case iek_type: {
	a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	a_boolean is_typedef = FALSE;
	unsigned long type_symid = 0;
	a_boolean is_class_struct_union = (type_ptr->kind == tk_class ||
					   type_ptr->kind == tk_struct ||
					   type_ptr->kind == tk_union);

	if (generate_multiple_IF_files) {
	    /* Do not write SYM, REL, etc for types, pointer types,
	     * automatically-generated class and member template instance types
	     * yet. 
	     */
	    if ((type_ptr->kind == tk_typeref && !type_ptr->source_corresp.name) ||
		type_ptr->kind == tk_pointer ||
		(is_class_struct_union && is_auto_gen_class(type_ptr)) ||
		(is_auto_gen_member(&type_ptr->source_corresp))) {
		return TRUE;
	    }
	}

	if (prototype_instance_member && is_class_struct_union &&
	    type_ptr->source_corresp.assoc_info) {
	    a_symbol_ptr sym = (a_symbol_ptr)
		    type_ptr->source_corresp.assoc_info;
	    a_class_symbol_supplement_ptr cssp =
		    sym->variant.class_struct_union.extra_info;
	    if (cssp->template_info && cssp->template_info->il_template_entry) {
		/* This is a nested class of a template, and it has
		 * its own template IL entry; let the iek_template
		 * case handle it.
		 */
		return TRUE;
	    }
	}
	if (type_ptr->source_corresp.is_local_to_function &&
	    !fcn_name) {
	    /* This is a type that was declared inside a function,
	     * but for implementation reasons EDG "promotes" it into
	     * the global scope; since we don't know the full details
	     * of where it came from (we can find the function but
	     * not the block number), we'll defer processing it until
	     * we encounter its actual scope.
	     */
	    return TRUE;
	}
	if (is_builtin_typedef(type_ptr)) {
	    /* SYM record is emitted for the type, not the typedef. */
	    return TRUE;
	}
    }
	break;

    case iek_variable: {
	a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	a_boolean is_template_instance =
		variable_ptr->is_template_static_data_member &&
		!variable_ptr->is_specialized;
	if (generate_multiple_IF_files) {  
	    if (is_template_instance || 
		is_auto_gen_member(&variable_ptr->source_corresp) ||
		(fcn_name && curr_fcn && is_auto_gen_routine(curr_fcn)) ||
		(variable_ptr->is_this_parameter && !fcn_name)) {
		/* Do not write SYM, REL, etc for template instance  
		   variables and parameters of template instance routines yet.
		*/
		return TRUE;
	    }
	}

	if (variable_has_tpl_entry(variable_ptr) != NULL) {
	    /* This is a static data member of a template class (prototype
	     * instantiation), and this data member is defined outside
	     * its class, so it has its own template IL entry; we need to
	     * avoid processing it twice.
	     */
	    return TRUE;
	}
	if (variable_ptr->storage_class == sc_static &&
	    variable_ptr->source_corresp.is_local_to_function &&
	    !fcn_name) {
	    /* This is a local static variable, which was promoted into
	     * global scope for EDG implementation reasons; we can't
	     * decorate the name with the function/block name in
	     * global scope, so we'll defer processing it until we
	     * get to the appropriate local scope.
	     */
	    return TRUE;
	}
	if (prototype_instance_member &&
	    (!variable_ptr->source_corresp.source_sequence_entry ||
	     variable_ptr->source_corresp.source_sequence_entry->
	     entity.kind != iek_src_seq_secondary_decl)) {
	    /* This is a static data member of a template class that
	     * has a definition in this compilation unit; it thus
	     * has its own template IL entry, and we should defer
	     * processing to the iek_template case.
	     */
	    return TRUE;
	}
	if (variable_ptr->is_this_parameter && !fcn_declarator) {
	    /* This is the "this" parameter for a trivial default
	     * constructor; suppress it.
	     */
	    return TRUE;
	}
    }
	break;

    case iek_field: {
	a_field_ptr field_ptr = (a_field_ptr) entry_ptr;
	a_type_ptr parent = field_ptr->source_corresp.parent.class_type;

	if (generate_multiple_IF_files && is_auto_gen_class(parent)) {
	  /* Do not write SYM, REL, etc for fields of a template class
	   * instance yet. */
	  return TRUE;
	}
    }
	break;

    case iek_routine: {
	a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	a_type_ptr routine_type = routine_ptr->type;
	a_routine_type_supplement_ptr extra_info;
	a_boolean atr_required = FALSE;
	a_boolean is_prototype_instance_fcn = routine_ptr->is_template_function &&
		!routine_ptr->is_specialized && 
		routine_ptr->is_prototype_instantiation;
	a_boolean template_mbr = routine_ptr->source_corresp.is_class_member
	    && routine_ptr->source_corresp.parent.class_type != NULL
	    && routine_ptr->source_corresp.parent.class_type->variant.class_struct_union.is_template_class;
	a_boolean mere_template = template_mbr
	    && routine_ptr->source_corresp.source_sequence_entry == NULL;
	unsigned long return_symid;
	a_boolean is_auto_gen_fcn = routine_ptr->is_template_function &&
		    !routine_ptr->is_specialized && 
		    !routine_ptr->is_prototype_instantiation;

	if (generate_multiple_IF_files) {
	    if (is_auto_gen_routine(routine_ptr)) {
		/* Do not write SYM, REL, etc for template function instance yet.
		 */
		return TRUE;
	    }
	}
	if (has_tpl_entry(routine_ptr) != NULL) {
	    /* This is a member function of a template class (prototype
	     * instantiation), and this member function is defined outside
	     * its class, so it has its own template IL entry; we need to
	     * avoid processing it twice.
	     */
	    return TRUE;
	}
	if (routine_ptr->is_trivial_default_constructor) {
	    /* Just ignore these. */
	    return TRUE;
	}
    }
	break;

    case iek_template: {
	a_template_ptr template = (a_template_ptr) entry_ptr;
	a_boolean is_auto_gen = is_auto_gen_member(&template->source_corresp);
	if (generate_multiple_IF_files && is_auto_gen) {
	  /* Do not write SYM, REL, etc for template instances yet.
	   */
	  return TRUE;
	}
    }
	break;

    default:
	break;
    } /* switch */
    return FALSE;
} /* skip_entry */

/* If this entry was already referenced (i.e., has a nonzero
 * symid) before we had a chance to generate the sym_summary_info
 * for it, remember that fact and dump the SYM line now after
 * creating the appropriate info.
 */
static void process_boring_entry (char *entry_ptr, an_il_entry_kind entry_kind) {
    /* If this entry was already referenced (i.e., has a nonzero
     * symid) before we had a chance to generate the sym_summary_info
     * for it, remember that fact and dump the SYM line now after
     * creating the appropriate info.
     */
    a_boolean SYM_line_required =
	    il_entry_prefix_of(entry_ptr).SET_symid != 0;
    switch (entry_kind) {
    case iek_constant: {
	a_constant_ptr constant_ptr = (a_constant_ptr) entry_ptr;
	if (constant_ptr->source_corresp.name) {
	    form_name(&constant_ptr->source_corresp, iek_constant,
		      ocb_of_string_buf(&string_buf));
	}
	else constant_to_string(constant_ptr, &string_buf);
	decorate_name_and_finish(constant_ptr, iek_constant, /*add_block_qual=*/TRUE,
				 /*add_closing_quote=*/FALSE, &string_buf);
	if (constant_ptr->kind == ck_string ||
	    (constant_ptr->kind == ck_address &&
	     constant_ptr->variant.address.kind == abk_constant &&
	     constant_ptr->variant.address.variant.constant->
	     kind == ck_string)) {
	    add_summary_info(constant_ptr, "string", terminate_string(&string_buf));
	}
	else if (constant_ptr->kind == ck_integer &&
		 constant_ptr->source_corresp.name) {
	    add_summary_info(constant_ptr, "evalue", terminate_string(&string_buf));
	}
	else add_summary_info(constant_ptr, "constant",
			      terminate_string(&string_buf));
    }
	break;

    case iek_type: {
	a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	a_boolean is_typedef = FALSE;
	const char* kind;
	if (is_builtin_typedef(type_ptr)) {
	    /* "bool" is a predeclared typedef for the real type in
	     * Microsoft mode as of 2.39. In the IF it's synonymous
	     * with the associated "bool" type, which is the one whose
	     * SYM line is emitted.
	     */
	    SYM_line_required = FALSE;
	}
	if (type_ptr->kind == tk_integer && type_ptr->variant.integer.enum_type) {
	    kind = "enum";
	}
	else if (type_is_class(type_ptr)) {
	    if (type_ptr->kind == tk_class) {
		kind = "class";
	    }
	    else if (type_ptr->kind == tk_struct) {
		kind = "struct";
	    }
	    else {
		kind = "union";
	    }
	}
	else if (type_ptr->kind == tk_typeref &&
		 type_ptr->source_corresp.name) {
	    kind = "typedef";
	    is_typedef = TRUE;
	}
	else kind = "type";
	if (is_typedef) {
	    form_name(&type_ptr->source_corresp, iek_type,
                      ocb_of_string_buf(&string_buf));
	}
	else {
	    if (type_ptr->kind == tk_integer && type_ptr->variant.integer.bool_type) {
		/* To handle the builtin typedef "bool" in Microsoft mode,
		 * it is given the same symid as the "bool" type.
		 * "bool" will be reported as a type
		 * unconditionally, but it will be anyway in any real
		 * compilation unit (unless --no_bool is in effect).
		 */
		SYM_line_required = TRUE;
		if (!generate_multiple_IF_files) {
		  SET_symid_of(type_ptr, iek_type); /* ensure id */
		}
	    }
	    form_type(type_ptr, ocb_of_string_buf(&string_buf));
	}
	decorate_name_and_finish(type_ptr, iek_type, /*add_block_qual=*/TRUE,
				 /*add_closing_quote=*/FALSE, &string_buf);
	add_summary_info(type_ptr, kind, terminate_string(&string_buf));
    }
	break;

    case iek_variable: {
	a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	if (!(variable_ptr->is_parameter && !fcn_declarator)) {
	    /* ignore parameter of trivial default ctor */
	    if (variable_ptr->is_this_parameter) {
		add_to_string("this", &string_buf);
	    }
	    else form_name(&variable_ptr->source_corresp, iek_variable,
			   ocb_of_string_buf(&string_buf));
	    decorate_name_and_finish(variable_ptr, iek_variable, /*add_block_qual=*/TRUE,
				     /*add_closing_quote=*/FALSE, &string_buf);
	    add_summary_info(variable_ptr, "variable",
			     terminate_string(&string_buf));
	}
    }
	break;

    case iek_field: {
	a_field_ptr field_ptr = (a_field_ptr) entry_ptr;
	form_name(&field_ptr->source_corresp, iek_field,
		  ocb_of_string_buf(&string_buf));
	add_summary_info(field_ptr, "variable",
			 terminate_string(&string_buf));
    }
	break;

    case iek_routine: {
	a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	add_fcn_name_to_string(routine_ptr, &string_buf);
	if (routine_ptr->compiler_generated) {
	    add_summary_info(routine_ptr, "function:cg",
			     terminate_string(&string_buf));
	}
	else add_summary_info(routine_ptr, "function",
			      terminate_string(&string_buf));
    }
	break;

    case iek_label: {
	a_label_ptr label_ptr = (a_label_ptr) entry_ptr;
	if (label_ptr->source_corresp.name) {
	    form_name(&label_ptr->source_corresp, iek_label,
		      ocb_of_string_buf(&string_buf));
	    decorate_name_and_finish(label_ptr, iek_label, /*add_block_qual=*/FALSE,
				     /*add_closing_quote=*/FALSE, &string_buf);
	    add_summary_info(label_ptr, "label",
			     terminate_string(&string_buf));
	}
    }
	break;

    case iek_scope: {
	a_scope_ptr ptr = (a_scope_ptr) entry_ptr;
	if (ptr->kind == sck_function) {
	    fcn_name = NULL;
	    fcn_id = 0;
	    curr_fcn = NULL;
	    curr_fcn_scope = NULL;
	    in_template_function = FALSE;
	}
	else pop_block();
    }
	break;

    case iek_namespace: {
	a_namespace_ptr namespace_ptr = (a_namespace_ptr) entry_ptr;
	form_name(&namespace_ptr->source_corresp, iek_namespace,
		  ocb_of_string_buf(&string_buf));
	add_summary_info(namespace_ptr, "namespace",
			 terminate_string(&string_buf));
    }
	break;

    case iek_template: {
	a_template_ptr template = (a_template_ptr) entry_ptr;
	const char* kind;
	if (template->kind == templk_class ||
	    template->kind == templk_member_class) {
	    kind = "class";
	}
	else if (template->kind == templk_function ||
		 template->kind == templk_member_function) {
	    kind = "function";
	}
	else if (template->kind == templk_static_data_member) {
	    kind = "variable";
	}
	else {
	    complain_int(error_csev, "Unexpected template kind $1",
		    template->kind);
	    kind = "template";
	}
	form_name(&template->source_corresp, iek_template,
		  ocb_of_string_buf(&string_buf));
	add_summary_info(template, kind, terminate_string(&string_buf));
    }
	break;

    default:
	break;
    } /* switch */
    if (!generate_multiple_IF_files && SYM_line_required) {
	write_summary_sym_info(entry_ptr, entry_kind, &output_buf);
    }
} /* process_boring_entry */

/* The following function is called for each node encountered during
 * the IL walk.  For declarations, it outputs a SYM line to the IF
 * file; for references to non-local entities, it outputs a REL line
 * to the IF; and for blocks, expressions, and such, it adds to the
 * ast_node_info tree.
 */
void process_entry(char* entry_ptr, an_il_entry_kind entry_kind) {
#if DEBUG
    if (db_walk) {
        printf("Processing %s entry at %lx.\n", il_entry_kind_names[entry_kind], (long)entry_ptr);
    }
#endif
    befriend_clique(entry_ptr, entry_kind);
    if (fully_interesting_node(entry_ptr, entry_kind)) {
	if (in_src_seq_sublist) {
	    /* A source sequence entry that occurs in a sublist is part of */
	    /* a function scope that was artificially moved into the global */
	    /* scope because of the EDG memory management scheme.  Ignore */
	    /* it for now; we'll process it when we get to its real location */
	    /* inside the function. */
	    if (entry_kind == iek_src_seq_sublist) {
		/* finished with the sublist */
		in_src_seq_sublist = FALSE;
		return;	/* no further processing needed */
	    }
	    if (entry_kind == iek_source_sequence_entry) {
		/* do nothing now */
		return;
	    }
	    /* Other entry kinds are processed normally */
	}

	if (!skip_entry(entry_ptr, entry_kind)) {
	    if (get_buffer_from_il_node(entry_ptr) == NULL) {
		complain(error_csev, "Interesting node should have an associated output buffer.");
	    }
	    write_symbol_of_entry(entry_ptr, entry_kind, TRUE, TRUE);
	    if (entry_kind == iek_scope) {
		a_scope_ptr ptr = (a_scope_ptr)entry_ptr;
		if (ptr->kind == sck_function) {
		    fcn_name = NULL;
		    fcn_id = 0;
		    curr_fcn = NULL;
		    curr_fcn_scope = NULL;
		    in_template_function = FALSE;
		}
		else pop_block();
	    }
	    process_ast_of_entry(entry_ptr, entry_kind);
	}
    }
    else {
	process_boring_entry(entry_ptr, entry_kind);
    }
#if DEBUG
    if (db_walk) {
        printf("Done processing %s entry at %lx.\n", il_entry_kind_names[entry_kind], (long)entry_ptr);
    }
#endif
} /* process_entry */
