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
#include "SET_additions.h"
#include "SET_symid.h"
#include "SET_ast.h"
#include "SET_complaints.h"
#include "SET_scope.h"
#if DEBUG
#include "il.h"
#endif

/* Adapted from il.c.  Note that this will probably get out of sync. */
void dump_statement_kind(FILE *strm, a_statement_kind  kind)
{
  char *s;

  switch (kind) {
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
    case stmk_empty:           s = "empty";             break;
#endif /* REPRESENT_EMPTY_STATEMENTS_IN_IL */
    case stmk_expr:            s = "expr";              break;
    case stmk_if:              s = "if";                break;
    case stmk_while:           s = "while";             break;
    case stmk_goto:            s = "goto";              break;
    case stmk_label:           s = "label";             break;
    case stmk_return:          s = "return";            break;
    case stmk_block:           s = "block";             break;
    case stmk_end_test_while:  s = "end-test-while";    break;
    case stmk_for:             s = "for";               break;
    case stmk_switch:          s = "switch";            break;
    case stmk_init:            s = "init";              break;
    case stmk_asm:             s = "asm";               break;
#if ASM_FUNCTION_ALLOWED
    case stmk_asm_func_body:   s = "asm-func-body";     break;
#endif /* ASM_FUNCTION_ALLOWED */
    case stmk_try_block:       s = "try-block";         break;
#if MICROSOFT_EXTENSIONS_ALLOWED
    case stmk_microsoft_try:   s = "microsoft-try";     break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if GENERATE_SOURCE_SEQUENCE_LISTS
    case stmk_decl:            s = "decl";              break;
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
    case stmk_set_vla_size:    s = "set-vla-size";      break;
    case stmk_vla_decl:        s = "vla-decl";          break;
#if GNU_EXTENSIONS_ALLOWED
    case stmk_assigned_goto:   s = "assigned-goto";     break;
#endif /* GNU_EXTENSIONS_ALLOWED */
    default:                   s = "<bad stmt kind>";   break;
  }  /* switch */
  fputs(s, strm);
}  /* dump_statement_kind */

void dump_eptr(FILE *strm, char *identification, an_il_entry_kind kind, void *eptr)
{
#if DEBUG
    if (eptr != NULL) {
	fprintf(strm, "%s: %s [%lu] at %lx\n", identification, il_entry_kind_names[kind],
	    il_entry_prefix_of(eptr).SET_symid, (long)eptr);
    }
    else {
	fprintf(strm, "%s: %s at %lx\n", identification, il_entry_kind_names[kind],
	    (long)eptr);
    }
#endif /* DEBUG */
}


void dump_template_instance(FILE *strm, char *indent, a_template_instance_ptr inst)
{
    if (inst != NULL) {
	fprintf(strm, "%stemplate instance %lx for symbol %lx\n", indent, (long)inst, (long)inst->instance_sym);
	fprintf(strm, "%s    template_sym %lx\n", indent, (long)inst->template_sym);
	fprintf(strm, "%s    template_info %lx\n", indent, (long)inst->template_info);
    }
    else {
	fprintf(strm, "%stemplate instance %lx\n", indent, (long)inst);
    }
}

void dump_symbol(FILE *strm, a_symbol_ptr sym)
{
    fprintf(strm, "%s symbol at %lx\n", sym != NULL ? symbol_kind_names[sym->kind] : "NULL", (long)sym);
    if (sym != NULL) {
	fprintf(strm, "    %sreferenced, %sdefined, %s class member, %s template param, %s unknown function\n",
	      sym->referenced ? "" : "not ",
		sym->defined ? "" : "not ",
		sym->is_class_member ? "is": "not",
		sym->is_template_param ? "is": "not",
		sym->is_unknown_function ? "is": "not");
	if (sym->is_class_member) {
	    dump_eptr(strm, "    parent", iek_type, sym->parent.class_type);
	}
	else {
	    dump_eptr(strm, "    parent", iek_namespace, sym->parent.namespace_ptr);
	}
	switch (sym->kind) {
	case sk_type:
	    dump_eptr(strm, "    ptr", iek_type, sym->variant.type.ptr);
	    break;
	case sk_class_or_struct_tag:
	case sk_union_tag:
	    dump_eptr(strm, "    type", iek_type, sym->variant.class_struct_union.type);
	    break;
	case sk_variable:
	    dump_eptr(strm, "    ptr", iek_variable, sym->variant.variable.ptr);
	    break;
	case sk_field:
	    dump_eptr(strm, "    ptr", iek_field, sym->variant.field.ptr);
	    break;
	case sk_routine:
	case sk_member_function:
	    dump_eptr(strm, "    ptr", iek_routine, sym->variant.routine.ptr);
	    dump_template_instance(strm, "    ", sym->variant.routine.instance_ptr);
	    break;
	default:
	    break;
        }
    }
}

/* @@@ */

/* The match between a src seq entry pushed on the statement stack isn't
 * always an exact match with the corresponding src seq end of construct.
 * Pass this macro the entry that's on the stack, and the entry of the
 * end, to tell if there's a match. */
#if 0
#define stmt_stack_match(begin, end) ( \
	((begin).ptr == (end).ptr) \
     || ((begin).kind == iek_src_seq_secondary_decl \
	 && ((a_src_seq_secondary_decl_ptr)(begin).ptr)->entity.ptr == (end).ptr) \
   )
#else
a_boolean stmt_stack_match(a_tagged_pointer begin, a_tagged_pointer end) {
    return begin.ptr == end.ptr
	 || (begin.kind == iek_src_seq_secondary_decl
	     && ((a_src_seq_secondary_decl_ptr)begin.ptr)->entity.ptr == end.ptr);
}
#endif

a_boolean block_is_generated(a_statement_ptr block_stmt)
{
#if REPRESENT_EMPTY_STATEMENTS_IN_IL
   return    (((block_stmt)->variant.block.extra_info->final_position.seq == 0)
	      && !	(   (block_stmt)->variant.block.statements != NULL
		       && (block_stmt)->variant.block.statements->kind == stmk_empty
		       && (block_stmt)->variant.block.statements->next == NULL));
#else
   return block_stmt->variant.block.extra_info->final_position.seq == 0;
#endif
}

/* The following function checks to see if a source position is within
 * a specified range.
 */

a_boolean is_in_source_range(a_source_position_ptr pos,
			     a_source_range* range) {
    if ((pos->seq > range->start.seq ||
	 (pos->seq == range->start.seq &&
	  pos->mapped_column >= range->start.mapped_column)) &&
	(pos->seq < range->end.seq ||
	 (pos->seq == range->end.seq &&
	  pos->mapped_column <= range->end.mapped_column))) {
	return TRUE;
    }
    return FALSE;
}

/* The following function checks to see if a parameter variable has
 * a default argument or not.  (This is tricky because the variable
 * inherits a param_type with a default argument, even though the
 * the default occurs only in the secondary declaration, not in the
 * definition, so the source location has to be compared.)
 */

a_boolean param_var_has_default(a_variable_ptr var) {
    if (var && var->is_parameter && var->assoc_param_type &&
	var->assoc_param_type->default_arg_expr) {
	if (curr_fcn && curr_fcn->source_corresp.decl_pos_info) {
	    if (is_in_source_range(&var->assoc_param_type->
				   default_arg_expr->expr_range.start,
				   &curr_fcn->source_corresp.decl_pos_info->
				   variant.declarator_range)) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* @@@ */

a_source_sequence_entry_ptr find_src_seq_end_of_construct(
				   a_source_sequence_entry_ptr ssep,
				   a_tagged_pointer *construct_entry);

a_boolean recognize_src_seq_construct(a_tagged_pointer ent)
{
    an_il_entry_kind kind = (an_il_entry_kind)ent.kind;
    a_boolean is_construct = FALSE;
    if (kind == iek_type) {
        a_type_ptr type = (a_type_ptr)ent.ptr;
	a_boolean is_class_struct_union = (type->kind == tk_class || type->kind == tk_struct || type->kind == tk_union);
	a_boolean is_enum = (type->kind == tk_integer && type->variant.integer.enum_type);
#if CLASS_TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
        is_construct = (is_class_struct_union && !type->variant.class_struct_union.is_template_class) || is_enum;
#else
        is_construct = (is_class_struct_union && !(type->variant.class_struct_union.is_template_class
                                                   && type->variant.class_struct_union.extra_info->assoc_template != NULL
						   && type->variant.class_struct_union.extra_info->assoc_template->kind == templk_class))
                       || is_enum;
#endif
    }
    else if (kind == iek_namespace) {
	a_namespace_ptr ns = (a_namespace_ptr)ent.ptr;
	is_construct = !ns->is_namespace_alias;
    }
    else if (kind == iek_src_seq_secondary_decl) {
	a_src_seq_secondary_decl_ptr sseq =
		(a_src_seq_secondary_decl_ptr) ent.ptr;
	if (sseq->entity.kind == iek_namespace) {
	    a_namespace_ptr ns = (a_namespace_ptr)sseq->entity.ptr;
	    is_construct = !ns->is_namespace_alias;
	}
    }
    else if (kind == iek_statement) {
	a_statement_ptr stmt = (a_statement_ptr)ent.ptr;
	if (stmt->kind == stmk_if) {
	    is_construct = (stmt->variant.if_stmt.then_statement ||
		stmt->variant.if_stmt.else_statement);
	}
	else if (stmt->kind == stmk_while ||
		 stmt->kind == stmk_end_test_while) {
	    is_construct = (stmt->variant.loop_statement != NULL);
	}
	else if (stmt->kind == stmk_block) {
	    a_scope_ptr scope =
		    stmt->variant.block.extra_info->assoc_scope;
	    if (block_is_generated(stmt)) {
	    }
	    else if (scope && scope->variant.assoc_handler) {
	    }
	    else {
		is_construct = TRUE;
	    }
	}
	else if (stmt->kind == stmk_for) {
	    is_construct = (stmt->variant.for_loop.statement != NULL);
	}
	else if (stmt->kind == stmk_switch &&
		 stmt->variant.switch_stmt.body_statement) {
	    is_construct = TRUE;
	}
	else if (stmt->kind == stmk_try_block) {
	    is_construct = TRUE;
	}
	else if (stmt->kind == stmk_microsoft_try) {
	    is_construct = TRUE;
	}
    }
    else if (kind == iek_template) {
	a_template_ptr tplp = (a_template_ptr) ent.ptr;
	if (tplp->canonical_template != NULL
	      && tplp->canonical_template->definition_template == tplp
	      && (tplp->kind == templk_class || tplp->kind == templk_member_class)) {
	    is_construct = TRUE;
	}
    }
    return is_construct;
}

a_boolean is_src_seq_beginning_of_construct(a_source_sequence_entry_ptr seq)
{
    a_boolean is_construct = recognize_src_seq_construct(seq->entity);
#if DEBUG
    if (db_worry && is_construct != (find_src_seq_end_of_construct(seq, &seq->entity) != NULL)) {
	const char *entname = il_entry_kind_names[seq->entity.kind];
	if (is_construct) {
	    worry_str("Expected %s to have source sequence construct.\n", entname);
	}
	else {
	    worry_str("Expected %s to have no source sequence construct.\n", entname);
	    dump_src_seq_list(stdout, seq, seq->entity.ptr);
	}
    }
#endif
    return is_construct;
}

/* The following finds the source sequence entry for a given secondary
 * declaration of a template member (i.e., a declaration of a member
 * whose definition is given outside the body of the template class).
 */

a_source_sequence_entry_ptr get_secondary_src_seq(a_source_sequence_entry_ptr seq,
						  void* entity, an_il_entry_kind kind,
						  a_boolean dont_complain) {
    a_source_sequence_entry_ptr starting_seq = seq;
    while (seq &&
	   !(seq->entity.kind == iek_src_seq_secondary_decl &&
	     ((a_src_seq_secondary_decl_ptr) seq->entity.ptr)->
	     entity.ptr == entity)) {
	seq = seq->next;
    }
    if (!seq && !dont_complain) {
	unsigned long symid = SET_symid_of(entity, kind);
	complain_ulong(error_csev, "Cannot find source sequence entry for secondary declaration of symbol [$1].", symid);
    }
    return seq;
}

void dump_src_seq_list(FILE *strm, a_source_sequence_entry_ptr ssep,
				   char *construct_entry);
a_boolean is_auto_gen_member(a_source_correspondence_ptr sc);

/* Return whether the type is an automatically-generated class template instance.
 */
a_boolean is_auto_gen_class(a_type_ptr type)
{
    return (   (     type->kind == tk_class
	          || type->kind == tk_struct
	          || type->kind == tk_union)
            && type->variant.class_struct_union.is_template_class
            && !type->variant.class_struct_union.is_specialized
            && !type->variant.class_struct_union.is_prototype_instantiation)
	   || is_auto_gen_member(&type->source_corresp);
}

/* Return whether the routine is automatically-generated due to template instantiation,
 * whether of a template function directly, or as a member of an instantiated class.
 */
a_boolean is_auto_gen_routine(a_routine_ptr routine)
{
    a_template_ptr tmpl = routine->assoc_template;
    return (   tmpl != 0 
            && routine->is_template_function
	    && tmpl->kind == templk_function  /* not templk_member_function */
            && !routine->is_specialized
            && !routine->is_prototype_instantiation)
           || is_auto_gen_member(&routine->source_corresp);
}

/* Return whether the entity having this source correspondence
 * is a member of an automatically-generated class template instance.
 */
a_boolean is_auto_gen_member(a_source_correspondence_ptr sc)
{
    return    sc != NULL
           && sc->is_class_member
           && is_auto_gen_class(sc->parent.class_type);
}

a_boolean is_specialized(a_template_ptr template)
{
    return    ((template->kind == templk_class || template->kind == templk_member_class)
                  && template->prototype_instantiation.type != NULL
                  && template->prototype_instantiation.type->variant.class_struct_union.is_template_class
                  && template->prototype_instantiation.type->variant.class_struct_union.is_specialized)
	   || ((template->kind == templk_function || template->kind == templk_member_function)
		  && template->prototype_instantiation.routine != NULL
		  && template->prototype_instantiation.routine->is_template_function
                  && template->prototype_instantiation.routine->is_specialized)
	   || (template->kind == templk_static_data_member
		  && template->prototype_instantiation.variable != NULL
		  && template->prototype_instantiation.variable->is_template_static_data_member
                  && template->prototype_instantiation.variable->is_specialized);
}

/* @@@ */

/* The following function finds the "basetype" (i.e., the type after
 * stripping pointer, reference, and array modifiers), checks it to
 * see if it is a function, and returns both the basetype and the
 * result of the test.
 */

void check_for_fcn_basetype(a_type_ptr type,
			    a_type_ptr* basetype,
			    a_boolean* is_fcn) {
    while (type && type->kind == tk_pointer ||
	   type->kind == tk_array) {
	if (type->kind == tk_pointer) {
	    type = type->variant.pointer.type;
	}
	else type = type->variant.array.element_type;
    }
    *is_fcn = (type && type->kind == tk_routine &&
	       type->variant.routine.extra_info);
    *basetype = type;
}

/* The following function checks for a secondary decl that shares
 * a type and a source position with a defined function.  This
 * means that it's an in-class definition of a member function,
 * so that the secondary decl should be ignored.
 */

a_boolean is_in_class_def(a_src_seq_secondary_decl_ptr sseq) {
    if (sseq->entity.kind == iek_routine) {
	a_routine_ptr fcn = (a_routine_ptr) sseq->entity.ptr;
	if (fcn && fcn->defined) {
	    if (fcn->source_corresp.is_class_member) {
		a_type_ptr tp = fcn->source_corresp.parent.class_type;
		if (tp->source_corresp.decl_pos_info &&
		    is_in_source_range(&fcn->source_corresp.decl_position,
				       &tp->source_corresp.decl_pos_info->
				       specifiers_range)) {
		    return TRUE;
		}
	    }
	    if (fcn->defined_in_friend_decl) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* @@@ */

a_boolean sse_is_for_template_of(a_source_sequence_entry_ptr sse,
				 a_source_correspondence_ptr sc)
{
    if (sse->entity.kind == iek_template) {
	a_template_ptr template = (a_template_ptr)sse->entity.ptr;
	if (sc->decl_pos_info != NULL && template->source_corresp.decl_pos_info != NULL
	    && sc->decl_pos_info->identifier_range.start.seq != 0
            && template->source_corresp.decl_pos_info->identifier_range.start.seq != 0
	    && cmp_source_positions(sc->decl_pos_info->identifier_range.start,
				    template->source_corresp.decl_pos_info->identifier_range.start) == 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

a_boolean sse_ends_construct(a_source_sequence_entry_ptr sse,
			     an_il_entry_kind kind, void *ptr)
{
    if (sse->entity.ptr == ptr) {
	return TRUE;
    }
    if (sse->entity.kind == iek_type) {
	a_type_ptr type = (a_type_ptr)ptr;
	if (type->kind == tk_class || type->kind == tk_struct
	                           || type->kind == tk_union) {
	    a_class_type_supplement_ptr extra_info
		    = type->variant.class_struct_union.extra_info;
	}
    }
    return FALSE;
}

/* The following function checks to see if a function is a member
 * of a template class (prototype instantiation) and, if so, if the
 * function has its own template IL entry.  If so, it assigns the
 * symid of the template to the function and returns TRUE; otherwise
 * it returns FALSE.  This allows process_entry to avoid assigning
 * it a different symid and causes all references to the function
 * to be redirected to the template instead.
 */

a_template_ptr has_tpl_entry(a_routine_ptr fcn) {
    if (fcn->source_corresp.is_class_member) {
	a_symbol_ptr sym = (a_symbol_ptr) fcn->source_corresp.assoc_info;
	a_template_instance_ptr inst = sym ? sym->variant.routine.
		instance_ptr : NULL;
	a_template_symbol_supplement_ptr tssp = inst ? inst->template_info :
		NULL;
	a_template_ptr tplp = tssp ? tssp->il_template_entry : NULL;
	a_boolean mere_template;
	if (sym->kind == sk_member_function && sym->variant.routine.ptr != NULL && sym->variant.routine.ptr != fcn) {
	    attempt_same_symid(sym->variant.routine.ptr, iek_routine, fcn, iek_routine,
		     "Routine [$1] and member function [$2] have different symids.");
	}
	if (tplp) {
	    attempt_same_symid(tplp, iek_template, fcn, iek_routine,
		     "Template [$1] and function [$2] have different symids.");
	    return tplp;
	}
	mere_template = fcn->source_corresp.parent.class_type != NULL
	    && fcn->source_corresp.parent.class_type->variant.class_struct_union.is_template_class
	    && fcn->source_corresp.source_sequence_entry == NULL;
        if (mere_template) {
	    /* Search for corresponding template via parent. */
	    a_type_ptr parent = fcn->source_corresp.parent.class_type;
	    a_source_sequence_entry_ptr sse;
	    for (sse = parent->source_corresp.source_sequence_entry; sse != NULL; sse = sse->next) {
    		if (sse_is_for_template_of(sse, &fcn->source_corresp)) {
		    /* return TRUE; */
		    return NULL;
		}
		if (sse_ends_construct(sse, iek_type, parent)) {
		    break;
		}
	    }
	}
    }
    return NULL;
}

a_template_ptr variable_has_tpl_entry(a_variable_ptr var) {
    if (var->source_corresp.is_class_member) {
	a_symbol_ptr sym = (a_symbol_ptr) var->source_corresp.assoc_info;
	a_template_instance_ptr inst = sym && sym->kind == sk_static_data_member
                ? sym->variant.static_data_member.instance_ptr : NULL;
	a_template_symbol_supplement_ptr tssp = inst ? inst->template_info :
		NULL;
	a_template_ptr tplp = tssp ? tssp->il_template_entry : NULL;
	a_boolean mere_template;
	if (sym->kind == sk_static_data_member && sym->variant.static_data_member.variable != NULL
                                               && sym->variant.static_data_member.variable != var) {
	    attempt_same_symid(sym->variant.static_data_member.variable, iek_variable, var, iek_variable,
		     "Variable [$1] and static data member [$2] have different symids.");
	}
	if (tplp) {
	    attempt_same_symid(tplp, iek_template, var, iek_variable,
		     "Template [$1] and static data member [$2] have different symids.");
	    return tplp;
	}
	mere_template = var->source_corresp.parent.class_type != NULL
	    && var->source_corresp.parent.class_type->variant.class_struct_union.is_template_class
	    && var->source_corresp.source_sequence_entry == NULL;
        if (mere_template) {
	    /* Search for corresponding template via parent. */
	    a_type_ptr parent = var->source_corresp.parent.class_type;
	    a_source_sequence_entry_ptr sse;
	    for (sse = parent->source_corresp.source_sequence_entry; sse != NULL; sse = sse->next) {
    		if (sse_is_for_template_of(sse, &var->source_corresp)) {
		    /* return TRUE; */
		    return NULL;
		}
		if (sse_ends_construct(sse, iek_type, parent)) {
		    break;
		}
	    }
	}
    }
    return NULL;
}

/* Finds an index number of a symbol within its parent class.
 * Initially, this is part of a hack intended to reduce the collisions among
 * names of anonymous types in the MFC model.
 */
a_boolean find_child_index(a_tagged_pointer child,
			   a_boolean (*filter)(a_tagged_pointer),
			   int *index, int *count)
{
    an_il_entry_kind kind = (an_il_entry_kind)child.kind;
    a_source_correspondence_ptr scp = source_corresp_of_entry(child.ptr, kind);
    if (scp != NULL && scp->is_class_member) {

	/* Search for entity within parent. */
	a_type_ptr parent = scp->parent.class_type;
	a_source_sequence_entry_ptr sse;
	a_boolean found = FALSE;
	int i = 0;
	for (sse = parent->source_corresp.source_sequence_entry; sse != NULL; sse = sse->next) {
	    if (filter(sse->entity)) {
		if (sse->entity.ptr == child.ptr) {
		    found = TRUE;
		    *index = i;
		}
		i += 1;
	    }
	    if (sse->entity.kind == iek_src_seq_end_of_construct) {
		char *eoc_entry = ((a_src_seq_end_of_construct_ptr)sse->entity.ptr)->entity.ptr;
		if (eoc_entry == parent) {
		    break;
		}
	    }
	}
	*count = i;
	return found;
    }
    return FALSE;
}

/* The following function returns TRUE if the argument is an expression
 * whose type is nominally an lvalue and consequently represented in the
 * EDG IL as a pointer.
 */

a_boolean is_implicit_pointer(an_expr_node_ptr expr) {
    if (expr->kind == enk_operation &&
	(expr->variant.operation.kind == eok_field ||
	 expr->variant.operation.kind == eok_bit_field ||
	 expr->variant.operation.kind == eok_pm_field ||
	 expr->variant.operation.kind == eok_padd_subsc)) {
	return TRUE;
    }
    return FALSE;
}

/* Certain nodes reflect an implicit "address of" operation, either
 * directly or buried beneath one or more cast nodes.  (An example
 * is obj.f(), where the first argument to the function will be the
 * address of "obj".)  In GNU mode, an explicit generated "addr"
 * node must be added; this function returns the address of the node
 * that should be the operand of that operation (or NULL if the
 * situation does not apply).
 */

an_expr_node_ptr has_implicit_address_op(an_expr_node_ptr expr) {
    if (is_implicit_pointer(expr)) {
	return expr;
    }
    while (expr->kind == enk_operation &&
	   (expr->variant.operation.kind == eok_cast ||
	    expr->variant.operation.kind == eok_base_class_cast)) {
	expr = expr->variant.operation.operands;
    }
    if (expr->kind == enk_variable_address &&
	!expr->variant.variable.pos.seq) {
	/* An address operation, but the position of the "&" is
	 * null.
	 */
	return expr;
    }
    return NULL;
}

/* Check whether the given type should be treated as a class/struct/union.
 * Normally a class type is the same IF symbol as its class definition.
 * But nonreal classes are not handled that way.
 */
a_boolean type_is_class(a_type_ptr type_ptr) {
    a_type_kind kind = type_ptr->kind;
    if (kind == tk_class || kind == tk_struct || kind == tk_union) {
	return    !type_ptr->variant.class_struct_union.is_nonreal_class
	       || type_ptr->variant.class_struct_union.is_prototype_instantiation;
    }
    return FALSE;
}

a_boolean is_builtin_typedef(a_type_ptr type_ptr) {
    return type_ptr->kind == tk_typeref && type_ptr->source_corresp.name &&
	strcmp(type_ptr->source_corresp.name, "bool") == 0 &&
	type_ptr->variant.typeref.type->kind == tk_integer &&
	type_ptr->variant.typeref.type->variant.integer.bool_type;
}

/* Returns the scope associated with the given il entry.  May return NULL.
 * The IL walk allows nameless entities to be associated with the scope in
 * which they occur.  Unfortunately, this function supports scope only as
 * it relates to names.  For nameless entities including 'this' it returns
 * NULL.
 */
a_scope_ptr scope_of_entry(const void* entry_ptr, an_il_entry_kind entry_kind)
{
    a_source_correspondence_ptr scp = source_corresp_of_entry((char *)entry_ptr, entry_kind);
    if (scp != NULL) {
        a_symbol_ptr sym = (a_symbol_ptr)scp->assoc_info;
        if (sym != NULL && sym->decl_scope > 0) {
	    return lookup_scope(sym->decl_scope);
	}
    }
    return NULL;
}

a_source_correspondence_ptr source_corresp_of_entry(char* entry_ptr, an_il_entry_kind entry_kind)
{
    a_source_correspondence_ptr sc;
    switch(entry_kind) {
    case iek_constant: {
	    a_constant_ptr constant_ptr = (a_constant_ptr) entry_ptr;
	    sc = &constant_ptr->source_corresp;
        }
        break;
    case iek_type: {
	    a_type_ptr type_ptr = (a_type_ptr) entry_ptr;
	    sc = &type_ptr->source_corresp;
        }
        break;
    case iek_variable: {
	    a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	    sc = &variable_ptr->source_corresp;
        }
        break;
    case iek_field: {
	    a_field_ptr field_ptr = (a_field_ptr) entry_ptr;
	    sc = &field_ptr->source_corresp;
        }
        break;
    case iek_routine: {
	    a_routine_ptr routine_ptr = (a_routine_ptr) entry_ptr;
	    sc = &routine_ptr->source_corresp;
        }
        break;
    case iek_label: {
	    a_label_ptr label_ptr = (a_label_ptr) entry_ptr;
	    sc = &label_ptr->source_corresp;
        }
        break;
    case iek_namespace: {
	    a_namespace_ptr namespace_ptr = (a_namespace_ptr) entry_ptr;
	    sc = &namespace_ptr->source_corresp;
        }
        break;
    case iek_asm_entry: {
	    an_asm_entry_ptr asm_entry = (an_asm_entry_ptr) entry_ptr;
	    sc = &asm_entry->source_corresp;
        }
        break;
    case iek_template: {
	    a_template_ptr template = (a_template_ptr) entry_ptr;

	    sc = &template->source_corresp;
        }
        break;
    default:
        sc = NULL;
        break;
    }
    return sc;
}

/* Dump a source sequence entry for debugging.
 * If it's a sublist, dump the list's contents.
 */
void dump_src_seq_entry(FILE *strm, a_source_sequence_entry_ptr ssep)
{
  a_tagged_pointer entity = ssep->entity;
  dump_eptr(strm, "src seq entry for", entity.kind, entity.ptr);
  switch (entity.kind) {
    case iek_src_seq_secondary_decl:
      {
	  a_src_seq_secondary_decl_ptr dptr = (a_src_seq_secondary_decl_ptr)entity.ptr;
	  a_source_position pos = dptr->decl_position;
	  a_tagged_pointer dentity = dptr->entity;
	  dump_eptr(strm, "secondary decl for", dentity.kind, dentity.ptr);
	  fprintf(strm, "    at %d/%d %d\n", pos.seq, pos.column, pos.len);
      }
      break;
    case iek_src_seq_end_of_construct:
      {
	  a_src_seq_end_of_construct_ptr eptr = (a_src_seq_end_of_construct_ptr)entity.ptr;
	  a_source_position pos = eptr->position;
	  a_tagged_pointer eentity = eptr->entity;
	  dump_eptr(strm, "end of construct for", eentity.kind, eentity.ptr);
	  fprintf(strm, "    at %d/%d %d\n", pos.seq, pos.column, pos.len);
      }
      break;
    case iek_src_seq_sublist:
      dump_src_seq_list(strm, ((a_src_seq_sublist_ptr)entity.ptr)->source_sequence_list, NULL);
      break;
    default:
      {
	  a_source_correspondence_ptr sc = source_corresp_of_entry(entity.ptr, entity.kind);
	  if (sc != NULL) {
	      a_source_position pos = sc->decl_position;
	      fprintf(strm, "    at %d/%d %d", pos.seq, pos.column, pos.len);
	  }
	  else {
              fprintf(strm, "    at ?");
	  }
	  if (entity.kind == iek_statement) {
	      a_statement_ptr stmt = (a_statement_ptr)entity.ptr;
	      fprintf(strm, " (");
	      dump_statement_kind(strm, stmt->kind);
	      fprintf(strm, " statement)");
	  }
          fprintf(strm, "\n");
      }
      break;
  }
}

/* Dump a source sequence list for debugging.
 * If this is for a class or enum definition, a block, or a for-init
 * declaration, then construct_entry is used to identify the matching
 * a_src_seq_end_of_construct at which to stop dumping.
 */
void dump_src_seq_list(FILE *strm, a_source_sequence_entry_ptr ssep,
				   char *construct_entry)
{
  a_boolean satisfied = FALSE;
  a_source_sequence_entry_ptr p = ssep;
  fprintf(strm, "Dumping source sequence list %lx: begin.\n", (long)ssep);
  while (p != NULL) {
    dump_src_seq_entry(strm, p);
    if (p->entity.kind == iek_src_seq_end_of_construct) {
	char *eoc_entry = ((a_src_seq_end_of_construct_ptr)p->entity.ptr)->entity.ptr;
	if (satisfied = (eoc_entry == construct_entry)) {
            break;
	}
    }
    p = p->next;
  }
  if (construct_entry != NULL && !satisfied) {
    fprintf(strm, "! End of construct not found.\n");
  }
  fprintf(strm, "Dumping source sequence list %lx: end.\n", (long)ssep);
}

a_source_sequence_entry_ptr find_src_seq_end_of_construct(
				   a_source_sequence_entry_ptr ssep,
				   a_tagged_pointer *construct_entry)
{
  while (ssep != NULL) {
    if (ssep->entity.kind == iek_src_seq_end_of_construct) {
	a_tagged_pointer *eoc_entry = &((a_src_seq_end_of_construct_ptr)ssep->entity.ptr)->entity;
	if (stmt_stack_match(*construct_entry, *eoc_entry)) {
            return ssep;
	}
    }
    ssep = ssep->next;
  }
  return NULL;
}

a_boolean entry_is_prototype_instance_member(char* entry_ptr, an_il_entry_kind entry_kind)
{
    a_source_correspondence_ptr sc = source_corresp_of_entry(entry_ptr, entry_kind);
    if (sc != NULL && sc->is_class_member) {
	a_type_ptr parent_type = sc->parent.class_type;
	if (parent_type->kind == tk_class
	      || parent_type->kind == tk_struct
	      || parent_type->kind == tk_union) {
	    return /* parent_type->variant.class_struct_union.is_nonreal_class && */
                   parent_type->variant.class_struct_union.is_prototype_instantiation;
	}
    }
    return FALSE;
}
