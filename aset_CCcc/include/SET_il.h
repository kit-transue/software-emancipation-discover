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
/* Functions that analyze the EDG IL. */

#ifndef SET_il_h
#define SET_il_h

/* Functions that help parsing source sequence lists. */
extern a_source_sequence_entry_ptr find_src_seq_end_of_construct(
					   a_source_sequence_entry_ptr ssep,
					   a_tagged_pointer *construct_entry);
extern a_boolean recognize_src_seq_construct(a_tagged_pointer ent);
extern a_boolean is_src_seq_beginning_of_construct(a_source_sequence_entry_ptr seq);
extern a_source_sequence_entry_ptr get_secondary_src_seq(a_source_sequence_entry_ptr seq,
							  void* entity, an_il_entry_kind kind,
							  a_boolean dont_complain);
extern a_boolean sse_is_for_template_of(a_source_sequence_entry_ptr sse,
					a_source_correspondence_ptr sc);
extern a_boolean sse_ends_construct(a_source_sequence_entry_ptr sse,
				    an_il_entry_kind kind, void *ptr);

/* Compare two entries to tell whether the second entry completes the
 * source sequence construct which began with the first. */
extern a_boolean stmt_stack_match(a_tagged_pointer begin, a_tagged_pointer end);

/* Functions that categorize entities. */
extern a_boolean is_auto_gen_class(a_type_ptr type);
extern a_boolean is_auto_gen_routine(a_routine_ptr routine);
extern a_boolean is_auto_gen_member(a_source_correspondence_ptr sc);
extern a_boolean is_specialized(a_template_ptr template);
extern a_boolean is_in_class_def(a_src_seq_secondary_decl_ptr sseq);
extern a_template_ptr has_tpl_entry(a_routine_ptr fcn);
extern a_boolean find_child_index(a_tagged_pointer child,
			   a_boolean (*filter)(a_tagged_pointer),
			   int *index, int *count);
extern a_template_ptr variable_has_tpl_entry(a_variable_ptr var);
extern a_boolean is_implicit_pointer(an_expr_node_ptr expr);
extern an_expr_node_ptr has_implicit_address_op(an_expr_node_ptr expr);
extern a_boolean type_is_class(a_type_ptr type_ptr);
extern a_boolean is_builtin_typedef(a_type_ptr type_ptr);
extern a_boolean entry_is_prototype_instance_member(char* entry_ptr, an_il_entry_kind entry_kind);
extern a_boolean param_var_has_default(a_variable_ptr var);
extern a_boolean block_is_generated(a_statement_ptr block_stmt);

extern void check_for_fcn_basetype(a_type_ptr type,
				   a_type_ptr* basetype,
				   a_boolean* is_fcn);
extern a_scope_ptr scope_of_entry(const void* entry_ptr, an_il_entry_kind entry_kind);
extern a_source_correspondence_ptr source_corresp_of_entry(char* entry_ptr, an_il_entry_kind entry_kind);
extern a_boolean is_in_source_range(a_source_position_ptr pos,
				    a_source_range* range);

/* The following macro determines whether an entity is a member of
 * a class that is a prototype instantiation.
 */
#define entity_is_prototype_instance_member(p) \
    ((p)->source_corresp.is_class_member \
     && (p)->source_corresp.parent.class_type->variant.class_struct_union.is_prototype_instantiation)

/* Functions to dump various structures for debugging purposes. */
extern void dump_eptr(FILE *strm, char *identification, an_il_entry_kind kind, void *eptr);
extern void dump_template_instance(FILE *strm, char *indent, a_template_instance_ptr inst);
extern void dump_symbol(FILE *strm, a_symbol_ptr sym);
extern void dump_src_seq_entry(FILE *strm, a_source_sequence_entry_ptr ssep);
extern void dump_src_seq_list(FILE *strm, a_source_sequence_entry_ptr ssep,
			      char *construct_entry);
#endif /* SET_il_h */
