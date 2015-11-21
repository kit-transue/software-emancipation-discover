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
#include "SET_symbol.h"
#include "SET_names.h"
#include "SET_symid.h"
#include "SET_il.h"
#include "SET_additions.h"
#include "SET_complaints.h"
#include "SET_ast.h"
#include "SET_process_entry.h"
#include "SET_scope.h"
#include "symbol_tbl.h"
#include "cmd_line.h"
#include "il_to_str.h"

/* The following variables are used for metering memory usage by the
 * functions in this file.
 */

static size_t sym_summary_meter;

/* The following function adds a function name (given its routine_ptr)
 * to the string.
 */

void add_fcn_name_to_string(a_routine_ptr routine_ptr,
			    string_buffer_ref bp) {
    a_type_ptr routine_type = skip_typerefs(routine_ptr->type);
    an_il_to_str_output_control_block_ptr ocb = ocb_of_string_buf(bp);
    form_name(&routine_ptr->source_corresp, iek_routine, ocb);
    if (routine_ptr->is_template_function) {
	form_template_args(routine_ptr->template_arg_list, ocb);
    }
    if (routine_type->variant.routine.extra_info &&
	il_header.source_language == sl_Cplusplus &&
	routine_ptr->source_corresp.name_linkage != nlk_external) {
	a_boolean saved_flag = ocb->suppress_all_typedefs;
	ocb->suppress_all_typedefs = TRUE;
	form_function_declarator(routine_type, ocb);
	ocb->suppress_all_typedefs = saved_flag;
    }
}

/* @@@ */

typedef struct a_retry *a_retry_ptr;
typedef struct a_retry {
    char *entry_ptr;
    an_il_entry_kind entry_kind;
    a_retry_ptr next;
} a_retry;
a_retry_ptr retry_list = NULL;
a_retry_ptr retry_list_last = NULL;

/* Add entry to the retry list, for processing at end if not otherwise done. */
static void process_symbol_of_entry_if_needed(char *entry_ptr, an_il_entry_kind entry_kind) {
    if (!generate_multiple_IF_files && !il_entry_prefix_of(entry_ptr).SET_SYM_written) {
	a_retry_ptr retry = (a_retry_ptr)malloc(sizeof(a_retry));
        retry->entry_ptr = entry_ptr;
        retry->entry_kind = entry_kind;
	retry->next = NULL;
	if (retry_list_last != NULL) {
	    retry_list_last->next = retry;
	}
	else {
    	    retry_list = retry;
	}
	retry_list_last = retry;
    }
}

static void process_symbol_of_entry(char *entry_ptr, an_il_entry_kind entry_kind);

/* Process entries on the retry list, for those which don't have SYMs written. */
void retry_process_symbol() {
    a_retry_ptr retry = retry_list;
    for (; retry != NULL; retry = retry->next) {
	if (!il_entry_prefix_of(retry->entry_ptr).SET_SYM_written) {
	    process_symbol_of_entry(retry->entry_ptr, retry->entry_kind);
	}
    }
}

/*
 * -------------------- FORMATTING UTILITIES --------------------
 *
 * functions:	insert_sym_prefix decorate_name_and_finish
 * 		string_constant_to_string constant_to_string
 * 		format_rel write_rel add_access_spec write_context_rel
 * 		format_atr write_atr add_attrib expr_node_name
 *
 * These functions provide consistency of output and conciseness of
 * processing code when writing the various kinds of lines in the main
 * section of the IF file.  In addition, the ...constant... functions
 * are used in creating AST operands.  expr_node_name is used in
 * creating both preprocessor and parser AST nodes.
 */

/* The following function inserts the standard SYM prefix into the output
 * string in preparation for the insertion of the name (which must be
 * done on a per-kind basis).  Note that the use of the macro
 * SET_symid_of has the side effect of assigning next_symid to the
 * entry and incrementing next_symid, if the entry does not already
 * have a symid.
 */

void insert_sym_prefix(const char* sym_kind, void* entry_ptr, an_il_entry_kind entry_kind,
		       a_boolean compiler_generated,
		       a_boolean from_template,
		       string_buffer_ref output_buffer) {
    unsigned long id = SET_symid_of(entry_ptr, entry_kind);
    add_3_chars_to_string("SYM", output_buffer);
    add_symid_to_string(id, output_buffer);
    fulfill_SYM(entry_ptr);
    add_to_string(sym_kind, output_buffer);
    if (from_template) {
	add_to_string_with_len("_tmpl", 5, output_buffer);
    }
    if (compiler_generated) {
	add_to_string_with_len(" :cg", 4, output_buffer);
    }
    add_2_chars_to_string(" \"", output_buffer);

    /* mark the output buffer bit as set */
    if (generate_multiple_IF_files) {
        set_outbuf_bit_of_clique(entry_ptr, output_buffer);
    }
}

/* The following function decorates the names of entities local to a
 * function with the function name and block number, if any; global
 * entities are left unchanged.  It then adds the closing quote.
 */

void decorate_name_and_finish(const void* entry_ptr,
			      an_il_entry_kind entry_kind,
			      a_boolean add_block_qual,
			      a_boolean add_closing_quote,
			      string_buffer_ref bp) {
    if (fcn_name) {
	add_3_chars_to_string(" @ ", bp);
	add_to_string(fcn_name, bp);
	if (add_block_qual) {
	    int *bstp = block_stack_top();
	    if (bstp != NULL) {
	        add_num_to_string(*bstp, " {", "}", bp);
	    }
	}
    }
    if (add_closing_quote) {
	add_1_char_to_string('\"', bp);
    }
}

/* The following function is parallel to decorate_name_and_finish,
 * for use when the block tracking is not valid.  It works fine except
 * for nameless entities (including 'this').
 */

static void finish_plain_name(const void* entry_ptr,
			      an_il_entry_kind entry_kind,
			      a_boolean add_block_qual,
			      a_boolean add_closing_quote,
			      string_buffer_ptr bp) {
    a_scope_ptr scp = scope_of_entry(entry_ptr, entry_kind);
    a_scope_ptr fscp = scp;
    while (fscp != NULL && (   fscp->kind == sck_func_prototype
                            || fscp->kind == sck_block
			    || fscp->kind == sck_condition)) {
	fscp = fscp->parent;
    }
    if (fscp != NULL && fscp->kind == sck_function) {
	add_3_chars_to_string(" @ ", bp);
	add_fcn_name_to_string(fscp->variant.routine.ptr, bp);
	if (add_block_qual) {
	    if (scp->kind == sck_block || scp->kind == sck_condition) {
		int id = scp->if_id;
		if (id < 0) {
		    complain(error_csev, "Can't put id of block into name.");
		}
		else {
	            add_num_to_string(id, " {", "}", bp);
	        }
	    }
	}
    }
    else if (scp != NULL && scp->kind == sck_block) {
	complain(error_csev, "Can't form name for block.");
    }
    if (add_closing_quote) {
	add_1_char_to_string('\"', bp);
    }
}

/* Track which buffers have the SYM line for ellipsis. */
static unsigned char *ellipsis_outbuf_bit_arr = NULL;

void write_ellipsis_symbol(unsigned long id, string_buffer_ptr output_buffer) {
    if (output_buffer != NULL
        && (!generate_multiple_IF_files || !is_output_buffer_bit_set(&ellipsis_outbuf_bit_arr, output_buffer))) {

	add_3_chars_to_string("SYM", output_buffer);
	add_symid_to_string(id, output_buffer);
	add_to_string("type \"...\"", output_buffer);
	terminate_string(output_buffer);
	write_atr(id, "ellipsis", output_buffer);

	/* mark the output buffer bit as set */
	if (generate_multiple_IF_files) {
	    set_output_buffer_bit(&ellipsis_outbuf_bit_arr, output_buffer);
	}
    }
}

void write_symbol_for_fcn_id(string_buffer_ptr bp)
{
    if (generate_multiple_IF_files && curr_fcn) { 
      write_symbol_for_il_node(curr_fcn, iek_routine, bp);
    }
}

void write_sym_line_for_source_file(a_source_file_ptr source, 
				    string_buffer_ptr output_buffer)
{
    if (generate_multiple_IF_files && output_buffer != NULL
				   && !is_output_buffer_bit_set(&source->SET_outbuf_bit_arr, output_buffer)) { 
	if (!source->is_interesting || (get_buffer_of_source_file(source) != output_buffer)) {
            /* assign symid here */
            if (!source->symid) {
	        source->symid = next_symid++;
	    }

            add_3_chars_to_string("SYM", output_buffer);
            add_symid_to_string(source->symid, output_buffer);
            add_to_string_with_len("file ", 5, output_buffer);
            add_quoted_str_to_string(absolute_path_of(source->full_name),
		TRUE /*add quotes*/, output_buffer);
            terminate_string(output_buffer);
	    
	    set_output_buffer_bit(&source->SET_outbuf_bit_arr, output_buffer);
	}
    }
}

void write_define_relation(a_seq_number seq, 
                           unsigned long symid,
                           string_buffer_ptr output_buffer)
{
    if (output_buffer != NULL) {
        a_line_number line_number;
        a_boolean at_end_of_source;
        unsigned long nesting_depth;
  
        a_source_file_ptr sfp = source_file_for_seq(seq, &line_number, 
					            &at_end_of_source,
                                                    TRUE);
        if (sfp) {
            write_sym_line_for_source_file(sfp, output_buffer);
	    write_rel(sfp->symid, "define", symid, output_buffer);
        }
    }    
}

void write_type_rel_for_void_ptr(void* ilp, 
				 an_il_entry_kind il_kind,
				 string_buffer_ptr output_buffer) {
  a_type_ptr type_ptr = (a_type_ptr)ilp;
  if (type_ptr->kind == tk_pointer && output_buffer != NULL) {
    a_type_ptr base_type = (a_type_ptr)type_ptr->variant.pointer.type;
    if (base_type->kind == tk_void) {
      format_rel(SET_symid_of(base_type, iek_type), 
		 "type", 
		 il_entry_prefix_of(type_ptr).SET_symid, 
		 output_buffer);
      add_to_string("pointer", output_buffer);
      terminate_string(output_buffer);
    }
  }
}

/* The following function adds a printable representation of a string
 * constant to the specified string_buffer.  The spelling of the
 * constant is simply enclosed in quotes and embedded quotes are only
 * singly-escaped, that is, the three-character string a"b will be
 * represented as "\"a\"b\"", not as "\"a\\\"b\"".
 */

#define TRUNCATED_MSG "...<truncated>"
#define MAX_STR_LEN_IN_IF (10000 - sizeof(TRUNCATED_MSG))

static void string_constant_to_string(a_constant_ptr constant_ptr,
				      string_buffer_ref bp) {
    int i;
    a_targ_size_t len = constant_ptr->variant.string.length;
    char* str = constant_ptr->variant.string.value;
    a_boolean truncated;
    if (len > MAX_STR_LEN_IN_IF) {
	len = MAX_STR_LEN_IN_IF;
	truncated = TRUE;
    }
    else truncated = FALSE;
    if (is_wide_string_constant(constant_ptr)) {
	unsigned long wc;
	add_3_chars_to_string("L\\\"", bp);
	for (i = 0; i < len; i += targ_sizeof_wchar_t) {
	    wc = extract_wide_char_from_string(str + i);
	    /* Suppress the last character if NUL */
	    if (i != (len - targ_sizeof_wchar_t) || wc != '\0') {
		form_wide_char(wc, ocb_of_string_buf(bp));
	    }
	}
    }
    else {
	add_2_chars_to_string("\\\"", bp);
	for (i = 0; i < len; i++) {
	    if (i != (len - 1) || str[i] != '\0') {
		form_char(str[i], ocb_of_string_buf(bp));
	    }
	}
    }
    if (truncated) {
	add_to_string(TRUNCATED_MSG, bp);
    }
    add_2_chars_to_string("\\\"", bp);
}

/* The following function adds a printable representation of a constant
 * to the specified string_buffer.
 *
 * This function uses the trial buffer.
 */

void constant_to_string(a_constant_ptr constant_ptr,
			string_buffer_ref bp) {
    if (constant_ptr->kind == ck_aggregate) {
	add_to_string("{...}", bp);
    }
    else if (constant_ptr->kind == ck_string) {
	string_constant_to_string(constant_ptr, bp);
    }
    else if (constant_ptr->kind == ck_address &&
	     constant_ptr->variant.address.kind == abk_constant &&
	     constant_ptr->variant.address.variant.constant->kind
	     == ck_string) {
	string_constant_to_string(constant_ptr->
				  variant.address.variant.constant, bp);
    }
    else if (constant_ptr->type && is_character_type(constant_ptr->type)) {
	a_boolean ofl;
	add_1_char_to_string('\'', bp);
	form_char((char) value_of_integer_constant(constant_ptr, &ofl),
		  ocb_of_string_buf(bp));
	add_1_char_to_string('\'', bp);
    }
    else if (bp == &trial_buf) {
	complain(error_csev, "Can't convert constant in trial_buf.");
    }
    else {
	/* Suppress enumerator name in output (so that, e.g.,
	 * "enum { a = 5 }; a;" will show up as "5" and not "a"
	 * in the AST, per Pero's spec).  Cases where the enum
	 * name is desired, as in the SYM line, are handled
	 * by calling form_name directly instead of this fcn.
	 */
        an_il_to_str_output_control_block *trial_ocb
		= ocb_of_string_buf(&trial_buf);
	a_boolean saved_flag = trial_ocb->suppress_enum_name;
	trial_ocb->suppress_enum_name = TRUE;
	form_constant(constant_ptr, /*need_parens=*/FALSE, trial_ocb);
	trial_ocb->suppress_enum_name = saved_flag;
	add_quoted_str_to_string(terminate_string(&trial_buf),
				 /*add_quotes=*/FALSE, bp);
    }
}

/* The following function creates an unterminated REL line (not
 * including any attributes on that line) in the output string_buffer.
 */

void format_rel(unsigned long first_symid,
		const char* rel_name,
		unsigned long second_symid,
		string_buffer_ref output_buffer) {
    add_3_chars_to_string("REL", output_buffer);
    add_symid_to_string(first_symid, output_buffer);
    add_1_char_to_string('\"', output_buffer);
    add_to_string(rel_name, output_buffer);
    add_1_char_to_string('\"', output_buffer);
    add_symid_to_string(second_symid, output_buffer);
}

/* The following function formats and terminates a REL line (without
 * attributes) in the output string_buffer.
 */

void write_rel(unsigned long first_symid,
	       const char* rel_name,
	       unsigned long second_symid,
	       string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	format_rel(first_symid, rel_name, second_symid, output_buffer);
	terminate_string(output_buffer);
    }
}

/* The following function formats and terminates a REL line for the
 * "specialize" relation, including full, partial, and explicit
 * attributes.
 */

void write_specialize_rel(unsigned long inst_symid,
			  unsigned long template_symid,
			  const char* full_or_part,
			  a_boolean is_explicit,
			  string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	format_rel(inst_symid, "specialize", template_symid, output_buffer);
	if (full_or_part) {
	    add_to_string(full_or_part, output_buffer);
	    if (is_explicit) {
		add_to_string(",explicit", output_buffer);
	    }
	}
	else if (is_explicit) {
	    add_to_string("explicit", output_buffer);
	}
	terminate_string(output_buffer);
    }
}

/* The following function adds an access qualifier to the current
 * string being constructed in the output string_buffer.
 */

static void add_access_spec(int access_spec, 
			    string_buffer_ref output_buffer) {
    if (access_spec == as_public) {
	add_to_string("public", output_buffer);
    }
    else if (access_spec == as_protected) {
	add_to_string("protected", output_buffer);
    }
    else if (access_spec == as_private) {
	add_to_string("private", output_buffer);
    }
    else complain_int(error_csev, "Unexpected access $1",
		 access_spec);
}

/* The following function formats and terminates a "context" REL line
 * in the output string_buffer, including the access specifier. */

void write_context_rel(unsigned long class_symid,
		       unsigned long member_symid,
		       int access,
		       string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	format_rel(class_symid, "context", member_symid, output_buffer);
	add_access_spec(access, output_buffer);
	terminate_string(output_buffer);
    }
}

/* The following function formats an ATR line for the specified
 * symbol and attribute.
 */

static void format_atr(unsigned long symid, 
		       const char* attrib,
		       string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	add_3_chars_to_string("ATR", output_buffer);
	add_symid_to_string(symid, output_buffer);
	add_to_string(attrib, output_buffer);
    }
}

/* The following function writes an ATR line to the IF file. */

void write_atr(unsigned long symid, 
	       const char* attrib,
	       string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	format_atr(symid, attrib, output_buffer);
	terminate_string(output_buffer);
    }
}


#if 0
static a_type_ptr parent_class_of_entry(char *entry_ptr, an_il_entry_kind entry_kind) {
    switch(entry_kind) {
    case iek_constant:
	return ((a_constant_ptr)entry_ptr)->source_corresp.parent.class_type;
    case 
}
#endif

static void write_specialize_rel_for_templ_inst(a_symbol_ptr inst_sym,
						unsigned long inst_symid,
						string_buffer_ptr output_buffer)
{
    if (output_buffer != NULL) {
      a_type_ptr inst_class = inst_sym->parent.class_type;
      a_symbol_ptr inst_class_sym = inst_class ?
		(a_symbol_ptr) inst_class->source_corresp.assoc_info : NULL;
      a_symbol_ptr tmpl_class = inst_class_sym ?
		inst_class_sym->variant.class_struct_union.extra_info->
		class_template : NULL;
	a_symbol_ptr sym;
	for (sym = inst_sym->header->inactive_symbols; sym; sym = sym->next) {
	    if (sym != inst_sym && sym->is_class_member) {
		a_type_ptr sym_tmpl_class = sym->parent.class_type;
		if (sym_tmpl_class &&
                    !is_auto_gen_class(sym_tmpl_class) &&
		    sym_tmpl_class->source_corresp.assoc_info && 
		    ((a_symbol_ptr) sym_tmpl_class->source_corresp.assoc_info)->
		    variant.class_struct_union.extra_info->class_template ==
		    tmpl_class && sym->kind == inst_sym->kind) {

		    unsigned long tmpl_member_symid;
		    if (sym->kind == sk_type) {
		      tmpl_member_symid = SET_symid_and_write_sym(sym->variant.type.ptr, 
								  iek_type, 
								  output_buffer);
		    }
		    else if (sym->kind == sk_enum_tag) {
		      tmpl_member_symid = SET_symid_and_write_sym(sym->variant.enumeration.type, 
								  iek_type, 
								  output_buffer);
		    }
		    else if (sym->kind == sk_constant) {
		      tmpl_member_symid = SET_symid_and_write_sym(sym->variant.constant,
								  iek_constant, 
								  output_buffer);
		    }
		    else if (sym->kind == sk_field) {
		      tmpl_member_symid = SET_symid_and_write_sym(sym->variant.field.ptr, 
								  iek_field, 
								  output_buffer);
		    }
		    else if (sym->kind == sk_projection) {
		      tmpl_member_symid = 0;
		    }
		    else {
		      complain_int(error_csev, "Unexpected template member kind $1",
				   sym->kind);
		      tmpl_member_symid = 0;
		    }
		    if (tmpl_member_symid) {
			write_specialize_rel(inst_symid, tmpl_member_symid,
					     NULL,
					     FALSE,
					     output_buffer);
		    }
		}
	    }
	} 
    }
}

/* The following function is called to add an attribute to the specified
 * string_buffer.  The atr_added parameter is used to keep track of
 * whether a comma is needed before adding the string.
 */

static void add_attrib(const char* attrib, a_boolean* atr_added,
		       string_buffer_ref bp) {
    if (*atr_added) {
	add_2_chars_to_string(", ", bp);
    }
    add_to_string(attrib, bp);
    *atr_added = TRUE;
}

static a_boolean is_anonymous(a_tagged_pointer ent)
{
    an_il_entry_kind kind = (an_il_entry_kind)ent.kind;
    a_source_correspondence_ptr scp = source_corresp_of_entry(ent.ptr, (an_il_entry_kind)ent.kind);
    return scp != NULL && unmangled_name_of(scp) == NULL;
}

/* The following function writes a string to the output_buf of the
 * form "<unnamed # var_name>" for a nameless type that is used in
 * the type of "var_name" (the first variable or field in the
 * declarator list of the declaration in which the nameless type
 * is contained).  Note that this differs from the treatment of
 * typedefs for nameless types, where the name is the first that
 * is a synonym of the type; here it is the first, regardless of
 * whether type modifiers are part of the declarator.
 */

void form_unnamed(a_type_ptr tp,
		  an_il_to_str_output_control_block_ptr ocb) {
    a_source_sequence_entry_ptr seq;
    (*ocb->output_str)("<unnamed");
    for (seq = tp->source_corresp.source_sequence_entry; seq;
	 seq = seq->next) {
	if (seq->entity.kind == iek_src_seq_end_of_construct) {
	    a_src_seq_end_of_construct_ptr eoc =
		    (a_src_seq_end_of_construct_ptr) seq->entity.ptr;
	    if (eoc->entity.ptr == (char*) tp) {
		break;
	    }
	}
    }
    if (seq && !seq->next) {
	a_source_sequence_entry_ptr parent_seq;
	for (parent_seq = orphaned_list_parent_seq_list; parent_seq;
	     parent_seq = parent_seq->next) {
	    if (parent_seq->entity.kind == iek_src_seq_sublist) {
		a_src_seq_sublist_ptr sublist =
			(a_src_seq_sublist_ptr) parent_seq->entity.ptr;
		a_source_sequence_entry_ptr subseq;
		for (subseq = sublist->source_sequence_list; subseq;
		     subseq = subseq->next) {
		    if (subseq->entity.kind == iek_type &&
			subseq->entity.ptr == (char*) tp) {
			seq = parent_seq;
			break;
		    }
		}
		if (seq == parent_seq) {
		    break;
		}
	    }
	}
    }
    if (seq && seq->next && !tp->autonomous_primary_tag_decl) {
	/* make seq point to entry for first declarator after type */
	a_source_sequence_entry_ptr seqn = seq->next;
	if (seqn->entity.kind == iek_variable) {
	    a_variable_ptr vp = (a_variable_ptr) seqn->entity.ptr;
	    (*ocb->output_str)(" # ");
	    form_name(&vp->source_corresp, iek_variable, ocb);
	    /*TBD: decorate_name_and_finish(...) */
	}
	else if (seqn->entity.kind == iek_field) {
	    a_field_ptr fp = (a_field_ptr) seqn->entity.ptr;
	    (*ocb->output_str)(" # ");
	    form_name(&fp->source_corresp, iek_field, ocb);
	    /*TBD: decorate_name_and_finish(...) */
	}
    }
    else {
	int index, count;
	a_tagged_pointer ent;
	ent.kind = iek_type;
	ent.ptr = (char *)tp;
	if (find_child_index(/* seq->entity */ ent, is_anonymous, &index, &count) && count > 1) {
	    char buffer[50];
	    (*ocb->output_str)(" # ");
	    sprintf(buffer, "%d", index + 1);
	    ocb->output_str(buffer);
	}
    }
    (*ocb->output_str)(">");
}


/* The following function will write a SYM line for a given il entry 
 * pointer to the given output file/buffer if it has not already been
 * written to it.  Associated relations and attributes are also written
 * unless they are to be handled in the tree walk.
 */

void write_symbol_for_il_node(void *ilp,
			      an_il_entry_kind il_kind,
			      string_buffer_ptr output_buffer)
{
  an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
  
  /* skip writing SYM line if il node already has an "error_symid" 
     assigned to it */
  if (ep->SET_symid == error_symid) {
     return;
  }

  if (output_buffer != NULL && !is_output_buffer_bit_set(&ep->SET_outbuf_bit_arr, output_buffer)) { 
    string_buffer_ptr def_buffer = get_buffer_from_il_node(ilp);

    write_referenced_symbol_info(ilp, il_kind, output_buffer, 
				 (def_buffer == output_buffer));       

    /* set output buffer bit */ 
    set_outbuf_bit_of_clique(ilp, output_buffer);
      
    /* write "pointer" REL line between "void" and "void*" */
    if (il_kind == iek_type) {
      write_type_rel_for_void_ptr(ilp, il_kind, output_buffer);
    }
  }
}

/* The following function is called for data members, nested
 * non-class types (typedefs and enums), and enumerators of template
 * classes (prototype instantiations, actually) to add the
 * "specialize" RELs with the corresponding members of instances
 * of the parent template class.
 */

void write_tmpl_mbr_specialize_rels(a_symbol_ptr mbr,
				    unsigned long mbr_symid,
				    string_buffer_ptr output_buffer) {
    a_type_ptr proto_class = mbr->parent.class_type;
    a_symbol_ptr proto_class_sym = proto_class ?
	    (a_symbol_ptr) proto_class->source_corresp.assoc_info : NULL;
    a_symbol_ptr tmpl_class = proto_class_sym ?
	    proto_class_sym->variant.class_struct_union.extra_info->
	    class_template : NULL;
    a_symbol_ptr sym;
    for (sym = mbr->header->inactive_symbols; sym; sym = sym->next) {
	if (sym != mbr && sym->is_class_member) {
	    a_type_ptr inst_class = sym->parent.class_type;
	    if (inst_class &&
		inst_class->variant.class_struct_union.is_template_class &&
		!inst_class->variant.class_struct_union.is_specialized &&
		inst_class->source_corresp.assoc_info && 
		((a_symbol_ptr) inst_class->source_corresp.assoc_info)->
		variant.class_struct_union.extra_info->class_template ==
		tmpl_class && sym->kind == mbr->kind) {
		/* This class is an instance of the template parent of
		 * mbr_sym; find the symid of the instance member and
		 * write the "specialize" REL.
		 */
		unsigned long inst_symid;
		if (sym->kind == sk_type) {
		    inst_symid = SET_symid_and_write_sym(sym->variant.type.ptr, 
							 iek_type, output_buffer);
                    process_symbol_of_entry_if_needed((char*)sym->variant.type.ptr, 
                                                      iek_type);
		}
		else if (sym->kind == sk_enum_tag) {
		    inst_symid = SET_symid_and_write_sym(sym->variant.enumeration.type, iek_type, output_buffer);
		}
		else if (sym->kind == sk_constant) {
		    inst_symid = SET_symid_and_write_sym(sym->variant.constant, iek_constant, output_buffer);
		}
		else if (sym->kind == sk_field) {
		    inst_symid = SET_symid_and_write_sym(sym->variant.field.ptr, iek_field, output_buffer);
		}
		else if (sym->kind == sk_projection) {
		    inst_symid = 0;
		}
		else {
		    complain_int(error_csev, "Unexpected template member kind $1",
				 sym->kind);
		    inst_symid = 0;
		}
		if (inst_symid) {
		    write_specialize_rel(inst_symid, mbr_symid,
					 /*full_or_part=*/NULL,
					 /*explicit=*/FALSE,
					 get_buffer_from_il_node(inst_class));
		}
	    }
	}
    }
}

/* -------------------- FILE SCREENING --------------------
 *
 * structs:	sym_summary_info
 *
 * variables:	SET_file_is_interesting sym_summary_info_block
 *		next_free_sym_summary_info
 *
 * functions:	SET_is_file_interesting add_summary_info
 *		write_summary_sym_info
 *
 * macros:	interesting_node SYM_SUMMARY_INFO_BLOK_COUNT
 *
 * Files are categorized as either "interesting" or "uninteresting,"
 * depending on whether information from that file is desired in the IF.
 * Each time input is switched from one file to another (for each
 * #include and end-of-file), the global flag SET_file_is_interesting
 * is set accordingly.  This flag is copied into each IL node that is
 * created as il_entry_prefix::full_SET_dump.  The value of this flag
 * is returned by the macro interesting_node.
 *
 * Nodes that are not intresting are processed separately from interesting
 * nodes: instead of immediately causing output to the IF file, the
 * function add_summary_info is called to capture the basic information
 * about the node (since IL nodes are not self-defining) in a
 * sym_summary_info object, which is then attached to the node via the
 * SET_info field of the node's an_il_entry_prefix.  If a reference to
 * the node is later encountered from an interesting node and the node
 * has no symid, write_summary_sym_info is called (from assign_symid as
 * a side_effect of the SET_symid_of macro) to add a SYM line to the IF
 * using the stored info.
 * 
 * The decision as to whether a file is interesting or not is made by
 * the function SET_is_file_interesting.
 */
/* The following structure is used to provide sufficient information
 * (the spelling of the kind and name of the entity) to allow a SYM
 * line to be printed for references to entities from "uninteresting"
 * files.  Such entities are handled specially by process_entry,
 * filling in a pointer to this structure in the SET_info of the
 * entity in place of the pointer to ast_node_info that is placed
 * there for "interesting" entities.
 */

typedef struct sym_summary_info {
    const char* kind;
    const char* name;
} sym_summary_info;

typedef sym_summary_info* sym_summary_info_ptr;

/* The usual block allocation scheme: */

static sym_summary_info_ptr sym_summary_info_block;
static size_t next_free_sym_summary_info;
#define SYM_SUMMARY_INFO_BLOCK_COUNT 2000

/* The following function creates a new sym_summary_info object,
 * initializes it with the specified strings, and inserts it as
 * the SET_info of the specified IL node.
 */

void add_summary_info(void* ilp, const char* kind,
		      const char* name) {
    sym_summary_info_ptr sip;
    if (!sym_summary_info_block ||
	next_free_sym_summary_info >= SYM_SUMMARY_INFO_BLOCK_COUNT) {
	sym_summary_info_block = (sym_summary_info_ptr)
		malloc(SYM_SUMMARY_INFO_BLOCK_COUNT *
		       sizeof(sym_summary_info));
	sym_summary_meter += SYM_SUMMARY_INFO_BLOCK_COUNT *
		sizeof(sym_summary_info);
	next_free_sym_summary_info = 0;
    }
    sip = sym_summary_info_block + next_free_sym_summary_info++;
    sip->kind = kind;
    sip->name = name;
    il_entry_prefix_of(ilp).SET_info = sip;
}

void write_summary_sym_info(void *ilp, 
			    an_il_entry_kind il_kind,
			    string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
	sym_summary_info_ptr sip = (sym_summary_info_ptr) ep->SET_info;
	add_3_chars_to_string("SYM", output_buffer);
	add_symid_to_string(ep->SET_symid, output_buffer);
	add_to_string(sip->kind, output_buffer);
	add_2_chars_to_string(" \"", output_buffer);
	add_to_string(sip->name, output_buffer);
	add_1_char_to_string('\"', output_buffer);
	terminate_string(output_buffer);
    }
    fulfill_SYM(ilp);

    /* write "pointer" REL line between "void" and "void*" */
    if (il_kind == iek_type) {
        write_type_rel_for_void_ptr(ilp, il_kind, output_buffer);
    }
}

/* The following function takes the symbol pointer of a template function
 * or static data member and returns its qualified name and the symid of
 * its parent class.  The qualified name is created in the trial_buf.
 */

static const char* template_qual_name_and_parent_symid(a_symbol_ptr sym,
						       a_tagged_pointer* parent_tag) {
    if (sym->is_class_member && sym->parent.class_type) {
	a_type_ptr parent_class = sym->parent.class_type;
	a_boolean is_member_of_real_instance = FALSE;
	if (parent_class->variant.class_struct_union.is_template_class &&
	    !parent_class->variant.class_struct_union.is_specialized) {
	    a_symbol_ptr parent_sym = (a_symbol_ptr)
		    parent_class->source_corresp.assoc_info;
	    a_class_symbol_supplement_ptr parent_info = (parent_sym) ?
		    parent_sym->variant.class_struct_union.extra_info : NULL;
	    a_template_symbol_supplement_ptr parent_template_info =
		    (parent_info) ? parent_info->template_info : NULL;
	    if (!parent_template_info) {
		/* We only set the parent symid here for members of real
		 * instances, not prototype instances, because the "context"
		 * REL will be output by create_ast_for_template_member
		 * for members of prototype instances; setting the
		 * parent symid here would cause a duplicate.
		 */
		SET_symid_of(parent_class, iek_type);
		parent_tag->ptr = (char *)parent_class;
		parent_tag->kind = iek_type;
		is_member_of_real_instance = TRUE;
	    }
	}
	else {
	    SET_symid_of(parent_class, iek_type);
	    parent_tag->ptr = (char *)parent_class;
	    parent_tag->kind = iek_type;
	}
	if (sym->kind != sk_class_template || is_member_of_real_instance) {
	    form_name(&parent_class->source_corresp, iek_type, ocb_of_string_buf(&trial_buf));
	    add_2_chars_to_string("::", &trial_buf);
	}
    }
    else if (sym->parent.namespace_ptr) {
        SET_symid_of(sym->parent.namespace_ptr, iek_namespace);
	parent_tag->ptr = (char *)sym->parent.namespace_ptr;
	parent_tag->kind = iek_namespace;
	if (sym->kind != sk_class_template) {
	    form_name(&sym->parent.namespace_ptr->source_corresp,
		      iek_namespace, ocb_of_string_buf(&trial_buf));
	    add_2_chars_to_string("::", &trial_buf);
	}
    }
    if (sym->kind == sk_class_template) {
	a_template_symbol_supplement_ptr tmpl_supp = sym->variant.template_info;
	a_symbol_ptr proto_sym = tmpl_supp ?
		tmpl_supp->variant.class_template.prototype_instantiation : NULL;
	a_type_ptr proto_class = (proto_sym && (proto_sym->kind == 
						sk_class_or_struct_tag ||
						proto_sym->kind ==
						sk_union_tag)) ?
	        proto_sym->variant.class_struct_union.type : NULL;
	if (proto_class) {
	    form_name(&proto_class->source_corresp, iek_type, ocb_of_string_buf(&trial_buf));
	}
	else if (sym->variant.template_info->prototype_template) {
	    /* This is a member template of a real instance, so there is
	     * no prototype for this one.  What we have to do is find the
	     * prototype for the member of the prototype instance and
	     * get its unqualified name (the qualification was added
	     * above under control of the is_member_of_real_instance
	     * flag).
	     */
	    tmpl_supp = sym->variant.template_info->prototype_template->
		    variant.template_info;
	    proto_sym = tmpl_supp ?
		    tmpl_supp->variant.class_template.prototype_instantiation :
		    NULL;
	    proto_class = (proto_sym && (proto_sym->kind == sk_class_or_struct_tag ||
					 proto_sym->kind == sk_union_tag)) ?
		    proto_sym->variant.class_struct_union.type : NULL;
	    if (proto_class) {
		form_unqualified_name(&proto_class->source_corresp, iek_type,
				      ocb_of_string_buf(&trial_buf));
	    }
	    else add_to_string(sym->header->identifier, &trial_buf);
	}
	else add_to_string(sym->header->identifier, &trial_buf);
    }
    else {
	add_to_string(sym->header->identifier, &trial_buf);
	if (sym->kind == sk_routine || sym->kind == sk_member_function) {
            an_il_to_str_output_control_block* ocb
			= ocb_of_string_buf(&trial_buf);
            a_boolean saved_flag = ocb->suppress_all_typedefs;
	    ocb->suppress_all_typedefs = TRUE;
	    form_function_declarator(skip_typerefs(sym->variant.routine.
						   ptr->type),
				     ocb);
            ocb->suppress_all_typedefs = saved_flag;
	}
	else if (sym->kind == sk_function_template) {
	    a_template_param_ptr tpl_parm;
            an_il_to_str_output_control_block* ocb;
	    a_boolean saved_flag;
	    add_1_char_to_string('<', &trial_buf);
	    if (sym->variant.template_info != NULL
		&& sym->variant.template_info->cache.decl_info != NULL) {
		for (tpl_parm = sym->variant.template_info->cache.decl_info->
		     parameters; tpl_parm; tpl_parm = tpl_parm->next) {
		    add_to_string(tpl_parm->param_symbol->header->identifier,
				  &trial_buf);
		    if (tpl_parm->next) {
			add_2_chars_to_string(", ", &trial_buf);
		    }
		}
	    }
	    add_1_char_to_string('>', &trial_buf);

            ocb = ocb_of_string_buf(&trial_buf);
            saved_flag = ocb->suppress_all_typedefs;
            ocb->suppress_all_typedefs = TRUE;
	    form_function_declarator(skip_typerefs(sym->variant.template_info->
						   variant.function.routine->
						   type),
				     ocb);
            ocb->suppress_all_typedefs = saved_flag;
	}
    }
    return terminate_string(&trial_buf);
}

/* The following function writes SYM line for a given il entry in the given
 * output buffer. It also writes REL and ATR lines if sym_line_only is not 
 * true. Most of the stuff here is grabbed from the process_entry function.
 * This function is used to write information of a symbol that is referenced
 * in this file(represented by output_buffer), regardless of whether it is
 * defined in this file. Thus, unlike process_entry, this function will not
 * write SMT lines or AST.
 */

static void write_symbol(char *entry_ptr, an_il_entry_kind entry_kind,
			 string_buffer_ptr output_buffer,
			 a_boolean sym_line_only,
			 a_boolean in_il_walk,
			 a_boolean write_noninvertible_relations) {

    a_boolean for_entry = FALSE;

    /* The following variable is set TRUE to indicate that AST should be
     * created for class members of templates, even though they appear
     * to be members of template instances (in this case, the prototype
     * instantiation).
     */
    a_boolean prototype_instance_member = entry_is_prototype_instance_member(entry_ptr, entry_kind);

    void (*finish_name)(const void *, an_il_entry_kind entry_kind, a_boolean, a_boolean, string_buffer_ptr)
	= in_il_walk ? decorate_name_and_finish : finish_plain_name;
    a_boolean write_parent_relations = generate_multiple_IF_files;
    a_boolean write_child_relations = !generate_multiple_IF_files;
    a_boolean mark_definitions = in_il_walk;

    if (write_noninvertible_relations && sym_line_only) {
        complain(error_csev, "Writing only SYM line.");
    }
    if (in_il_walk && !write_noninvertible_relations) {
	complain(error_csev, "IL walk is not writing all relations.");
    }
    if (output_buffer == NULL) {
	return;
    }

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
        unsigned long constant_symid;

	if (   in_il_walk
            && (   (   constant_ptr->const_expr
		    && (   !is_case_label
		        || constant_ptr->const_expr->kind == enk_constant)
		    && !constant_ptr->source_corresp.name)
		|| constant_ptr->kind == ck_dynamic_init
		|| (GNU_compatible_AST && constant_ptr->kind == ck_aggregate)
		|| (constant_ptr->from_asm && !emit_asm_strings))) {
	    break;
	}

	if (constant_ptr->kind == ck_string ||
	    (constant_ptr->kind == ck_address &&
	     constant_ptr->variant.address.kind == abk_constant &&
	     constant_ptr->variant.address.variant.constant->
	     kind == ck_string)) {
	    insert_sym_prefix("string", constant_ptr, iek_constant, FALSE, FALSE, output_buffer);
	}
	else if (constant_ptr->kind == ck_integer &&
		 constant_ptr->source_corresp.name) {
	    insert_sym_prefix("evalue", constant_ptr, iek_constant, FALSE,
			      prototype_instance_member,
			      output_buffer);
	}
	else insert_sym_prefix("constant", constant_ptr, iek_constant, FALSE,
			       prototype_instance_member, output_buffer);
	if (constant_ptr->source_corresp.name) {
	    form_name(&constant_ptr->source_corresp, iek_constant,
		      ocb_of_string_buf(output_buffer));
	    finish_name(constant_ptr, iek_constant, /*add_block_qual=*/TRUE,
			/*add_closing_quote=*/TRUE, output_buffer);
	}
	else {
	    constant_to_string(constant_ptr, output_buffer);
	    add_1_char_to_string('\"', output_buffer);
	}
	if (mark_definitions && constant_ptr->source_corresp.name &&
	    !is_template_member) {
	    add_sym_def_pos_to_string(&constant_ptr->source_corresp.decl_position,
			              output_buffer);
	}
	terminate_string(output_buffer);

        if (!sym_line_only || is_auto_gen_member(&constant_ptr->source_corresp)) {
            constant_symid = SET_symid_of(constant_ptr, iek_constant);
	    if (constant_ptr->type) {
	        write_rel(
		          SET_symid_and_write_sym(constant_ptr->type, iek_type, output_buffer), 
		          "type", 
		          constant_symid,
		          output_buffer);
	    }
            if (!in_il_walk) {
                /* write "define" relation for enum value reference */
                if (constant_ptr->kind == ck_integer && 
                    constant_ptr->source_corresp.name &&
                    !is_template_member) {
                    write_define_relation(constant_ptr->source_corresp.decl_position.seq, 
                                          constant_symid, 
                                          output_buffer);
	        }
            }
	    if (write_parent_relations) {
	        /* write "context" REL between constant and its parent 
	        class/struct/union or namespace */
	        if (constant_ptr->source_corresp.is_class_member) {
	            a_type_ptr parent = constant_ptr->source_corresp.parent.class_type;
	            write_context_rel(SET_symid_and_write_sym(parent, 
                                                              iek_type, 
						              output_buffer), 
			              constant_symid, 
			              constant_ptr->source_corresp.access,
			              output_buffer);
	        } 
	        else {
	            a_namespace_ptr namespace_ptr = 
	            constant_ptr->source_corresp.parent.namespace_ptr;
	            if (namespace_ptr) {
	                write_rel(SET_symid_and_write_sym(namespace_ptr, 
                                                          iek_namespace, 
						          output_buffer),
			          "context",
			          constant_symid,
			          output_buffer);
	            }
	        }
	    }
	}
    }
	break;

    case iek_param_type: {
	if (!in_il_walk) {
	    a_param_type_ptr param_ptr = (a_param_type_ptr) entry_ptr;
	    unsigned long param_symid;
	    insert_sym_prefix("variable", param_ptr, iek_param_type,
			      /*compiler_generated=*/FALSE,
			      prototype_instance_member,
			      output_buffer);
	    if (param_ptr->name) {
	      add_to_string(param_ptr->name, output_buffer);
	    }
	    else add_to_string("<unnamed>", output_buffer);
	    add_to_string_with_len(" @ \"", 4, output_buffer);
	    terminate_string(output_buffer);

	    param_symid = SET_symid_of(param_ptr, iek_param_type);
	    write_rel(SET_symid_and_write_sym(param_ptr->type, iek_type, output_buffer), 
		      "type", param_symid, output_buffer);
	    write_atr(param_symid, "auto", output_buffer);
	}
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
	a_boolean is_class_struct_union = (type_ptr->kind == tk_class ||
					   type_ptr->kind == tk_struct ||
					   type_ptr->kind == tk_union);
	a_boolean is_auto_gen_type = ((is_class_struct_union && 
				       is_auto_gen_class(type_ptr)) ||
				      (is_auto_gen_member(&type_ptr->source_corresp)));

	/* During il_walk do not write SYM, REL, etc for tk_routine types 
         * (but do the AST) in "multiple_iffs" mode. The SYM, REL, etc will 
         * be written if the type gets referenced.
	 */
	if (generate_multiple_IF_files && in_il_walk && 
            type_ptr->kind == tk_routine) {
            break;
        }

	if (!in_il_walk && is_class_struct_union) {
	  /* from "type" il node try to find its corresponding "template" il node */
	  a_source_correspondence *scp = source_corresp_for_il_entry(entry_ptr, iek_type);
	  a_symbol_ptr entry_sym = (scp) ? (a_symbol_ptr)scp->assoc_info : NULL;
	  a_type_ptr classp = (entry_sym) ? entry_sym->variant.class_struct_union.type : NULL;
	  a_class_symbol_supplement_ptr cssp = (entry_sym) ? entry_sym->variant.class_struct_union.extra_info : NULL;
	  a_template_symbol_supplement_ptr tssp = cssp ? cssp->template_info : NULL;
	  a_template_ptr tpl = tssp ? tssp->il_template_entry : NULL;
#if DEBUG
	  if (type_ptr != classp) {
	    worry("Symbol type differs from source sequence type.");
	  }
#endif
	  if (classp && tpl) {
            SET_symid_and_write_sym(tpl, iek_template, output_buffer); 
	    break;
	  }
	}

	if (in_il_walk && is_class_struct_union && type_ptr->variant.class_struct_union.is_nonreal_class
	    && type_ptr->variant.class_struct_union.is_prototype_instantiation) {
	    /* This is a prototype instantiation, which should not
	     * be handled here but processed by the code in the
	     * iek_template case.
	     */
	}
	else if (!in_il_walk && is_builtin_typedef(type_ptr)) {
	  /* SYM line is emitted for the "bool" type, not the typedef. */
	  SET_symid_and_write_sym(bool_type(), iek_type, output_buffer);
	  break;
	}
	else {
	    if (type_ptr->kind == tk_integer && type_ptr->variant.integer.enum_type) {
		insert_sym_prefix("enum", type_ptr, iek_type, FALSE,
				  prototype_instance_member, output_buffer);
	    }
	    else if (type_is_class(type_ptr)) {
		if (type_ptr->kind == tk_class) {
		    insert_sym_prefix("class", type_ptr, iek_type, FALSE,
				      prototype_instance_member, output_buffer);
		}
		else if (type_ptr->kind == tk_struct) {
		    insert_sym_prefix("struct", type_ptr, iek_type, FALSE,
				      prototype_instance_member, output_buffer);
		}
		else {
		    insert_sym_prefix("union", type_ptr, iek_type, FALSE,
				      prototype_instance_member, output_buffer);
		}
	    }
	    else if (type_ptr->kind == tk_typeref &&
		     type_ptr->source_corresp.name) {
		insert_sym_prefix("typedef", type_ptr, iek_type, FALSE,
				  prototype_instance_member, output_buffer);
		is_typedef = TRUE;
	    }
	    else insert_sym_prefix("type", type_ptr, iek_type, FALSE, FALSE, output_buffer);
	    if (is_typedef) {
		form_name(&type_ptr->source_corresp, iek_type,
                          ocb_of_string_buf(output_buffer));
	    }
	    else form_type(type_ptr, ocb_of_string_buf(output_buffer));
	    if (type_ptr->source_corresp.is_local_to_function) {
		finish_name(type_ptr, iek_type, /*add_block_qual=*/TRUE,
			    /*add_closing_quote=*/TRUE, output_buffer);
	    }
	    else add_1_char_to_string('\"', output_buffer);
	    if (!((type_ptr->kind == tk_class || type_ptr->kind == tk_struct ||
		   type_ptr->kind == tk_union) &&
		  (type_ptr->size == 0 ||
		   is_auto_gen_class(type_ptr))) &&
		!in_template_function &&
		mark_definitions) {
		/* Do not add position (indicating that the type is defined) if
		 * it's a class/struct/union that's incomplete or the result of
		 * automatic instantiation, or if the type (of any kind) is
		 * defined inside a template function.
		 */
		add_sym_def_pos_to_string(&type_ptr->source_corresp.decl_position,
				          output_buffer);
	    }
	    terminate_string(output_buffer);
	}
	if (!sym_line_only ||
	    (type_ptr->kind == tk_typeref && !type_ptr->source_corresp.name) ||
	    type_ptr->kind == tk_pointer ||
	    is_auto_gen_type || 
	    type_ptr->kind == tk_routine) {
	    unsigned long type_symid = SET_symid_and_write_sym(type_ptr, iek_type,
                                                               output_buffer);

	    if (type_ptr->kind == tk_integer &&
		     type_ptr->variant.integer.enum_type) {
		a_constant_ptr enumerator =
			type_ptr->variant.integer.enum_info.constant_list;
		if (!type_ptr->source_corresp.name) {
		    write_atr(type_symid, "nameless", output_buffer);
		}
                if (!in_il_walk) {
                    /* write "define" relation for enum reference */
                    if (!in_template_function) {
                        write_define_relation(type_ptr->source_corresp.decl_position.seq,
                                              type_symid,
                                              output_buffer);
		    }
		}
		if (write_parent_relations && is_auto_gen_type &&
                    type_ptr->source_corresp.assoc_info) {
		  a_symbol_ptr enum_sym = (a_symbol_ptr)type_ptr->source_corresp.assoc_info;
                  if (enum_sym) {
		      write_specialize_rel_for_templ_inst(enum_sym, type_symid,
						          output_buffer);
		  }
		}
		if (write_noninvertible_relations) {
		    while(enumerator) {
			write_rel(type_symid, "enumerate",
				  SET_symid_and_write_sym(enumerator, iek_constant, output_buffer), 
				  output_buffer);
			enumerator = enumerator->next;
		    }
		}
	    }
	    else if (type_ptr->kind == tk_routine) {
		a_routine_type_supplement_ptr extra_info =
			type_ptr->variant.routine.extra_info;
		a_param_type_ptr param_type;
		int argno = 1;
		if (extra_info->routine_name_linkage == nlk_external &&
		    il_header.source_language == sl_Cplusplus) {
		    write_atr(type_symid, "linkage(c)", output_buffer);
		}
		format_rel(SET_symid_and_write_sym(type_ptr->variant.routine.return_type, iek_type, output_buffer),
			   "type", type_symid, output_buffer);
		add_to_string("return", output_buffer);
		terminate_string(output_buffer);
		for (param_type = extra_info->param_type_list; param_type;
		     param_type = param_type->next) {
		    format_rel(SET_symid_and_write_sym(param_type->type, iek_type, output_buffer), 
			       "type", type_symid, output_buffer);
		    add_num_to_string(argno++, "argument(", ")", output_buffer);
		    terminate_string(output_buffer);
		    if (in_il_walk && !extra_info->assoc_routine &&
			param_type->decl_pos_info) {
			/* param is not part of a definition; must do
			 * entire SYM, type REL, auto ATR now.
			 */
			unsigned long parm_symid = SET_symid_of(param_type, iek_param_type);

			insert_sym_prefix("variable", param_type, iek_param_type,
					  /*compiler_generated=*/FALSE,
					  prototype_instance_member,
					  output_buffer);
			if (param_type->name) {
			    add_to_string(param_type->name, output_buffer);
			}
			else add_to_string("<unnamed>", output_buffer);
			add_to_string_with_len(" @ \"", 4, output_buffer);
			terminate_string(output_buffer);
			write_rel(SET_symid_and_write_sym(param_type->type, iek_type, output_buffer), "type",
				  parm_symid, output_buffer);
			write_atr(parm_symid, "auto", output_buffer);

		    }
		}
		if (extra_info->has_ellipsis) {
		    format_rel(symid_of_ellipsis(output_buffer), "type", type_symid, output_buffer);
		    add_num_to_string(argno++, "argument(", ")", output_buffer);
		    terminate_string(output_buffer);
		}
	    }
	    else if (type_ptr->kind == tk_array) {
		format_rel(SET_symid_and_write_sym(type_ptr->variant.array.element_type, iek_type, output_buffer),
			   "type", type_symid, output_buffer);
		add_to_string("array", output_buffer);
		if (!type_ptr->variant.array.is_variable_size_array) {
		    add_num_to_string(type_ptr->variant.array.variant.number_of_elements,
				      "(", ")", output_buffer);
		}
		terminate_string(output_buffer);
	    }
	    else if (type_ptr->kind == tk_class || type_ptr->kind ==
		     tk_struct || type_ptr->kind == tk_union) {
		a_field_ptr field;
		a_class_type_supplement_ptr extra_info =
			type_ptr->variant.class_struct_union.extra_info;
		a_scope_ptr class_scope = (extra_info) ? extra_info->assoc_scope :
			NULL;
		if (type_symid == 0) {
		    complain(error_csev, "type_symid was not set.");
		}

		if (!type_ptr->source_corresp.name) {
		    write_atr(type_symid, "nameless", output_buffer);
		}
		if (extra_info) {
		    a_base_class_ptr base;
		    for (base = extra_info->base_classes; base; base =
			 base->next) {
			if (base->direct) {
			    format_rel(type_symid, "subclass",
				       SET_symid_and_write_sym(base->type, iek_type, output_buffer), 
				       output_buffer);
			    if (base->is_virtual) {
				add_to_string("virtual", output_buffer);
			    }
			    else add_access_spec(base->derivation->access,
						 output_buffer);
			    terminate_string(output_buffer);
			}
		    }
		    if (type_ptr->variant.class_struct_union.is_template_class) {
			a_template_arg_ptr tp_arg;
			for (tp_arg = extra_info->template_arg_list; tp_arg;
			     tp_arg = tp_arg->next) {
			    if (tp_arg->kind == (a_templ_arg_kind)tak_type) {
				format_rel(SET_symid_and_write_sym(tp_arg->variant.type, iek_type, output_buffer),
					   "argument", type_symid, output_buffer);
				add_to_string("template", output_buffer);
				terminate_string(output_buffer);

				if (tp_arg->variant.type->kind == tk_pointer)
				    process_symbol_of_entry_if_needed (
					(char*)tp_arg->variant.type, 
					iek_type);
			    }
			    else if (tp_arg->kind == (a_templ_arg_kind)tak_template ) {
				format_rel(SET_symid_and_write_sym(tp_arg->variant.templ.ptr, iek_template, output_buffer),
					   "argument", type_symid, output_buffer);
				add_to_string("template", output_buffer);
				terminate_string(output_buffer);
			    }
			    else if (!tp_arg->is_array_bound_of_unknown_type) {
				if (tp_arg->variant.constant->kind == ck_address) {
				    /* This is a pointer non-type argument; make
				     * the relation with the entity pointed to by
				     * the address constant ("variable" will give
				     * the right address, even if it's not a
				     * variable), not the constant itself.
				     */
				    format_rel(SET_symid_and_write_sym(tp_arg->variant.constant->variant.address.variant.variable,
								       iek_variable, output_buffer),
					       "argument",
					       type_symid, output_buffer);
				}
				else {
				    process_symbol_of_entry_if_needed((char*) tp_arg->variant.constant,
								      iek_constant);
				    format_rel(SET_symid_and_write_sym(tp_arg->variant.constant, iek_constant, output_buffer),
					       "argument", type_symid,
					       output_buffer);
				}
				add_to_string("template", output_buffer);
				terminate_string(output_buffer);
			    }
			}
		    }
		    /* write "specialize" REL between instantiation and prototype template */
		    if (write_parent_relations && 
                        !type_ptr->variant.class_struct_union.is_prototype_instantiation && 
                        extra_info->assoc_template) {
		        write_specialize_rel(type_symid, 
					     SET_symid_and_write_sym(extra_info->assoc_template, 
                                                                     iek_template, 
                                                                     output_buffer), 
					     "full",
					     type_ptr->variant.class_struct_union.is_specialized,
					     output_buffer);
		    }

		    /* For an instantiated class write symbol info for all the instantiated
		       types(like enum, typedefs) defined within the template class.*/
		    if (is_auto_gen_type && class_scope) {
			a_type_ptr type;
			for (type = class_scope->types; type; type = type->next) { 
			    SET_symid_and_write_sym(type, iek_type, output_buffer);
			}
		    }
		}
		if (write_child_relations) {
		    for (field = type_ptr->variant.class_struct_union.field_list;
			 field; field = field->next) {
			write_context_rel(type_symid, 
					  SET_symid_and_write_sym(field, iek_field, output_buffer),
					  field->source_corresp.access,
					  output_buffer);
		    }
		}
		if (write_child_relations && class_scope) {
		    a_constant_ptr constant;
		    a_type_ptr type;
		    a_variable_ptr variable;
		    a_routine_ptr routine;
		    for (routine = class_scope->routines; routine;
			 routine = routine->next) {
			a_routine_type_supplement_ptr extra_info =
				skip_typerefs(routine->type)->
				variant.routine.extra_info;
			write_context_rel(type_symid, 
					  SET_symid_and_write_sym(routine, iek_routine, output_buffer),
					  routine->source_corresp.access,
					  output_buffer);
			if (!extra_info->this_class) {
			    write_atr(SET_symid_and_write_sym(routine, iek_routine, output_buffer), 
				      "static",
				      output_buffer);
			}
		    }
		    for (variable = class_scope->variables; variable;
			 variable = variable->next) {
			write_context_rel(type_symid, 
					  SET_symid_and_write_sym(variable, iek_variable, output_buffer),
					  variable->source_corresp.access,
					  output_buffer);
			write_atr(SET_symid_and_write_sym(variable, iek_variable, output_buffer), 
				  "static",
				  output_buffer);
		    }
		    for (type = class_scope->types; type; type = type->next) {
			write_context_rel(type_symid, 
					  SET_symid_and_write_sym(type, iek_type, output_buffer),
					  type->source_corresp.access,
					  output_buffer);
		    }
		    for (constant = class_scope->constants; constant;
			 constant = constant->next) {
			write_context_rel(type_symid, 
					  SET_symid_and_write_sym(constant, iek_constant, output_buffer),
					  constant->source_corresp.access,
					  output_buffer);
		    }
               }
               if (write_noninvertible_relations && class_scope) { 
		    a_using_decl_ptr using_decl;
		    for (using_decl = class_scope->using_decls; using_decl;
			 using_decl = using_decl->next) {
			format_rel(type_symid, "context",
				   SET_symid_and_write_sym(using_decl->entity.ptr, using_decl->entity.kind, output_buffer),
				   output_buffer);
			add_access_spec(using_decl->access, output_buffer);
			add_to_string(", using", output_buffer);
			terminate_string(output_buffer);
		    }
		}
		if (write_noninvertible_relations && extra_info && type_symid != 0) {
		    a_routine_list_entry_ptr friend_fcn;
		    a_class_list_entry_ptr friend_class;
		    for (friend_fcn = extra_info->friend_routines; friend_fcn;
			 friend_fcn = friend_fcn->next) {
			write_rel(SET_symid_and_write_sym(friend_fcn->routine, iek_routine, output_buffer), 
				  "friend",
				  type_symid, output_buffer);
		    }
		    for (friend_class = extra_info->friend_classes;
			 friend_class; friend_class = friend_class->next) {
			write_rel(SET_symid_and_write_sym(friend_class->class_type, iek_type, output_buffer),
				  "friend", type_symid, output_buffer);
		    }
		}
	    }
	    else if (type_ptr->kind == tk_typeref) {
		unsigned int type_quals =
			type_ptr->variant.typeref.qualifiers;
		unsigned int base_quals;
		a_boolean atr_added = FALSE;
		a_type_ptr typeref_ptr = type_ptr->variant.typeref.type;
	        write_define_relation(type_ptr->source_corresp.decl_position.seq,
                                      type_symid,
                                      output_buffer);
		format_rel(SET_symid_and_write_sym(typeref_ptr, iek_type, output_buffer), 
			   "type", type_symid, 
			   output_buffer);
		if (type_ptr->variant.typeref.type->kind == tk_typeref) {
		    base_quals = type_ptr->variant.typeref.type->
			    variant.typeref.qualifiers;
		}
		else base_quals = 0;
		if ((base_quals & TQ_CONST) ^
		    (type_quals & TQ_CONST)) {
		    add_attrib("const", &atr_added, output_buffer);
		}
		if ((base_quals & TQ_VOLATILE) ^
			 (type_quals & TQ_VOLATILE)) {
		    add_attrib("volatile", &atr_added, output_buffer);
		}
		if ((base_quals & TQ_RESTRICT) ^
			 (type_quals & TQ_RESTRICT)) {
		    add_attrib("restrict", &atr_added, output_buffer);
		}
#ifdef MICROSOFT_EXTENSIONS_ALLOWED
		if ((base_quals & TQ_UNALIGNED) ^
			 (type_quals & TQ_UNALIGNED)) {
		    add_attrib("unaligned", &atr_added, output_buffer);
		}
		if ((base_quals & TQ_NEAR) ^
			 (type_quals & TQ_NEAR)) {
		    add_attrib("near", &atr_added, output_buffer);
		}
		if ((base_quals & TQ_FAR) ^
			 (type_quals & TQ_FAR)) {
		    add_attrib("far", &atr_added, output_buffer);
		}
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
		terminate_string(output_buffer);

                /* Write "specialize" relations between instantiated "typedef" and
                   prototype template "typedef". */
                if (write_parent_relations && is_auto_gen_type &&
                    type_ptr->source_corresp.assoc_info) {
		  a_symbol_ptr typedef_sym = (a_symbol_ptr)type_ptr->source_corresp.assoc_info;
                  if (typedef_sym) {
		      write_specialize_rel_for_templ_inst(typedef_sym, type_symid,
						          output_buffer);
		  }
		}
	    }
	    else if (type_ptr->kind == tk_pointer) {
		format_rel(SET_symid_and_write_sym(type_ptr->variant.pointer.type, iek_type, output_buffer),
			   "type", type_symid, output_buffer);
		if (type_ptr->variant.pointer.is_reference) {
		    add_to_string("reference", output_buffer);
		}
		else add_to_string("pointer", output_buffer);
		terminate_string(output_buffer);
	    }
	    else if (type_ptr->kind == tk_ptr_to_member) {
		unsigned long class_symid =
			SET_symid_and_write_sym(type_ptr->variant.ptr_to_member.class_of_which_a_member, iek_type, output_buffer);
		process_symbol_of_entry_if_needed(
		    (char*)type_ptr->variant.ptr_to_member.class_of_which_a_member,
		    iek_type);
		format_rel(SET_symid_and_write_sym(type_ptr->variant.ptr_to_member.type, iek_type, output_buffer),
			   "type", type_symid, output_buffer);
		add_to_string("pointer(", output_buffer);
		add_symid_to_string(class_symid, output_buffer);
		add_1_char_to_string(')', output_buffer);
		terminate_string(output_buffer);
	    }

            if (write_parent_relations) {
		/* write "context" REL between type and its parent 
		   class/struct/union or namespace */
		if (type_ptr->source_corresp.is_class_member) {
		    a_type_ptr parent = type_ptr->source_corresp.parent.class_type;
		    write_context_rel(SET_symid_and_write_sym(parent, iek_type, 
							      output_buffer), 
				      type_symid, 
				      type_ptr->source_corresp.access,
				      output_buffer);
		}
		else {
		    a_namespace_ptr namespace_ptr = 
		        type_ptr->source_corresp.parent.namespace_ptr;
		    if (namespace_ptr) {
		        write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, 
						          output_buffer),
			          "context",
			          type_symid,
			          output_buffer);
		    }
		} 
            }
	} /* !sym_line_only */
    } /* case iek_type */
	break;

    case iek_variable: {
	a_variable_ptr variable_ptr = (a_variable_ptr) entry_ptr;
	a_boolean atr_required = FALSE;
	a_boolean has_default_expr = param_var_has_default(variable_ptr);
	a_boolean is_template_instance =
		variable_ptr->is_template_static_data_member &&
		!variable_ptr->is_specialized;
        a_boolean is_auto_gen_var = is_auto_gen_member(&variable_ptr->source_corresp);
        a_template_ptr templ_ptr = variable_has_tpl_entry(variable_ptr);

	if (!in_il_walk && templ_ptr) {
	    SET_symid_and_write_sym(templ_ptr, iek_template, output_buffer);
	    break;
	}

	insert_sym_prefix("variable", variable_ptr, iek_variable, FALSE,
			  prototype_instance_member, output_buffer);
	if (variable_ptr->is_this_parameter) {
	    add_to_string("this", output_buffer);
	}
	else {
	  form_name(&variable_ptr->source_corresp, iek_variable,
		    ocb_of_string_buf(output_buffer));
	}
	finish_name(variable_ptr, iek_variable, /*add_block_qual=*/TRUE,
		    /*add_closing_quote=*/TRUE, output_buffer);
	if (mark_definitions &&
	    variable_ptr->storage_class != sc_extern &&
	    !is_template_instance && !in_template_function &&
	    !(common_tentative_defs && il_header.source_language == sl_C &&
	      variable_ptr->storage_class == sc_unspecified &&
	      variable_ptr->init_kind == initk_none)) {
	    add_sym_def_pos_to_string(&variable_ptr->source_corresp.decl_position,
			              output_buffer);
	}
	terminate_string(output_buffer);

	if (!sym_line_only || is_auto_gen_var) {
            unsigned long variable_symid = SET_symid_of(variable_ptr, iek_variable);

	    write_rel(SET_symid_and_write_sym(variable_ptr->type, iek_type, output_buffer), 
		      "type", variable_symid, output_buffer);
	    add_3_chars_to_string("ATR", &trial_buf);
	    add_symid_to_string(variable_symid, &trial_buf);
	    switch(variable_ptr->storage_class) {
	    case sc_static:
		add_attrib("static", &atr_required, &trial_buf);
		break;
	    case sc_auto:
		add_attrib("auto", &atr_required, &trial_buf);
		break;
	    case sc_register:
		add_attrib("register", &atr_required, &trial_buf);
		break;
	    default:
		/* presumably "extern", the default */
		break;
	    }
	    if (variable_ptr->source_corresp.name_linkage == nlk_external &&
		il_header.source_language == sl_Cplusplus) {
		add_attrib("linkage(c)", &atr_required, &trial_buf);
	    }
	    if (atr_required) {
		add_to_string(terminate_string(&trial_buf), output_buffer);
		terminate_string(output_buffer);
	    }
	    else terminate_string(&trial_buf);	/* forget it */

            if (!in_il_walk) {
                /* write "define" relation for variable reference */
	        if (!variable_ptr->source_corresp.is_class_member &&
                    variable_ptr->storage_class != sc_extern &&
	            !is_template_instance && !in_template_function &&
	            !(common_tentative_defs && 
                      il_header.source_language == sl_C &&
	              variable_ptr->storage_class == sc_unspecified &&
	              variable_ptr->init_kind == initk_none)) {
                    write_define_relation(variable_ptr->source_corresp.decl_position.seq, 
                                          variable_symid, 
                                          output_buffer);
		}
            }
	    if (write_parent_relations) {
		/* write "context" REL between variable and its parent 
		   class/struct/union or namespace */
		if (variable_ptr->source_corresp.is_class_member) {
		  a_type_ptr parent = variable_ptr->source_corresp.parent.class_type;
		  write_context_rel(SET_symid_and_write_sym(parent, iek_type, 
							    output_buffer), 
				    variable_symid, 
				    variable_ptr->source_corresp.access,
				    output_buffer);
                  /* Also write "static" attribute for static data 
                     member. */
		  write_atr(variable_symid, "static", output_buffer);
                   
		}
		else {
		  a_namespace_ptr namespace_ptr = 
		    variable_ptr->source_corresp.parent.namespace_ptr;
		  if (namespace_ptr) {
		    write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, 
						      output_buffer),
			      "context",
			      variable_symid,
			      output_buffer);
		  }
		} 

		/* write "specialize" REL between instantiation and prototype template */
		if (is_auto_gen_var && variable_ptr->assoc_template) {
		  write_specialize_rel(variable_symid,
				       SET_symid_and_write_sym(variable_ptr->assoc_template, 
							       iek_template, 
							       output_buffer),
				       NULL,
				       variable_ptr->is_specialized,
				       output_buffer);
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
        a_boolean is_auto_gen_field = (parent && is_auto_gen_class(parent));

	insert_sym_prefix("field", field_ptr, iek_field, FALSE,
			  prototype_instance_member,
			  output_buffer);
	form_name(&field_ptr->source_corresp, iek_field,
		  ocb_of_string_buf(output_buffer));
	add_1_char_to_string('\"', output_buffer);
	if (mark_definitions && !is_template_member) {
	    add_sym_def_pos_to_string(&field_ptr->source_corresp.decl_position,
			              output_buffer);
	}
	terminate_string(output_buffer);
	if (!sym_line_only || is_auto_gen_field) {
	    unsigned long field_symid = SET_symid_of(field_ptr, iek_field);

	    write_rel(SET_symid_and_write_sym(field_ptr->type, iek_type, output_buffer), 
		      "type", field_symid, output_buffer);
	    if (field_ptr->is_mutable) {
		write_atr(field_symid, "mutable", output_buffer);
	    }
	    if (write_parent_relations) {
		/* write "context" REL between field and its parent 
		   class/struct/union or namespace */
		if (field_ptr->source_corresp.is_class_member) {
		  write_context_rel(SET_symid_and_write_sym(parent, iek_type, 
							    output_buffer), 
				    field_symid, 
				    field_ptr->source_corresp.access,
				    output_buffer);
		}
		else {
		  a_namespace_ptr namespace_ptr = 
		    field_ptr->source_corresp.parent.namespace_ptr;
		  if (namespace_ptr) {
		    write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, 
						      output_buffer),
			      "context",
			      field_symid,
			      output_buffer);
		  }
		}

		/* write "specialize" REL  between instantiation and prototype template */
		if (is_auto_gen_field) {
		  a_symbol_ptr field_sym = (a_symbol_ptr)field_ptr->source_corresp.assoc_info;
		  write_specialize_rel_for_templ_inst(field_sym, field_symid,
						      output_buffer);
		}
	    }
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
	a_type_ptr routine_type = routine_ptr->type;
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
        a_boolean is_auto_gen_rout = is_auto_gen_routine(routine_ptr);
        a_template_ptr templ_ptr = has_tpl_entry(routine_ptr);

	if (!in_il_walk && templ_ptr) {
	    SET_symid_and_write_sym(templ_ptr, iek_template, output_buffer);
	    break;
	}

	if (in_il_walk && is_prototype_instance_fcn && routine_ptr->assoc_template != NULL) {
	    /* instance of a function template or a member function template, so it
	     * has its own template IL entry:  Avoid processing its SYM line twice.
	     */
	}
#if 0
	Haven''t gotten this criterion correct.  For now, duplicate SYM lines may occur.

	else if (prototype_instance_member && template_mbr
		 && routine_ptr->source_corresp.parent.class_type->variant.class_struct_union.extra_info != NULL
		 && routine_ptr->source_corresp.parent.class_type->variant.class_struct_union.extra_info->assoc_template != NULL) {
	    /* instance of a function template or a member function template, so it
	     * has its own template IL entry:  Avoid processing its SYM line twice.
	     */
	    a_template_ptr assoc_template = routine_ptr->assoc_template;
	}
#endif
	else {
	    insert_sym_prefix("function", routine_ptr, iek_routine,
			      routine_ptr->compiler_generated,
			      prototype_instance_member || is_prototype_instance_fcn,
			      output_buffer);
	    add_fcn_name_to_string(routine_ptr, output_buffer);
	    add_to_string("\"", output_buffer);
	    if (mark_definitions &&
	        routine_ptr->assoc_scope != NULL_region_number && !is_auto_gen_rout &&
		!routine_ptr->compiler_generated) {
		a_scope_ptr scope_ptr =
			il_header.region_scope_entry[routine_ptr->assoc_scope];
		if (scope_ptr) {
		    a_statement_ptr assoc_block = scope_ptr->assoc_block;
		    if (assoc_block) {
			add_sym_def_pos_to_string(&routine_ptr->source_corresp.decl_position,
					          output_buffer);
		    }
		}
	    }
	    terminate_string(output_buffer);
	}

        if (!sym_line_only || is_auto_gen_rout) {
            unsigned long routine_symid = SET_symid_and_write_sym(routine_ptr, iek_routine, output_buffer);
	    a_routine_type_supplement_ptr extra_info
		    = (routine_type->kind == tk_routine) ?
		    routine_type->variant.routine.extra_info : NULL;
	    return_symid =
		    SET_symid_and_write_sym(routine_type->variant.routine.return_type, iek_type, output_buffer);

	    write_rel(SET_symid_and_write_sym(routine_type, iek_type, output_buffer), 
		      "type", routine_symid, output_buffer);
	    format_rel(return_symid, "type", routine_symid, output_buffer);
	    add_to_string("return", output_buffer);
	    terminate_string(output_buffer);
	    add_3_chars_to_string("ATR", &trial_buf);
	    add_symid_to_string(routine_symid, &trial_buf);
	    process_symbol_of_entry_if_needed((char *)routine_type, iek_type);
	    switch(routine_ptr->special_kind) {
	    case sfk_constructor:
		add_attrib("constructor", &atr_required, &trial_buf);
		if (routine_ptr->is_explicit_constructor) {
		    add_attrib("explicit", &atr_required, &trial_buf);
		}
		break;
	    case sfk_destructor:
		add_attrib("destructor", &atr_required, &trial_buf);
		break;
	    case sfk_conversion:
	    case sfk_operator:
		add_attrib("operator", &atr_required, &trial_buf);
		break;
	    default:
		/* add nothing */
		break;
	    }
	    if (routine_ptr->storage_class == sc_static &&
		!(extra_info && extra_info->this_class)) {
		add_attrib("static", &atr_required, &trial_buf);
	    }
	    else if (routine_ptr->is_virtual) {
		add_attrib("virtual", &atr_required, &trial_buf);
		if (routine_ptr->pure_virtual) {
		    add_attrib("pure", &atr_required, &trial_buf);
		}
	    }
	    if (routine_ptr->is_inline) {
		add_attrib("inline", &atr_required, &trial_buf);
	    }
	    if (routine_ptr->source_corresp.name_linkage == nlk_external &&
		il_header.source_language == sl_Cplusplus) {
		add_attrib("linkage(c)", &atr_required, &trial_buf);
	    }
	    if (il_header.source_language != sl_Cplusplus ||
		routine_ptr->source_corresp.name_linkage == nlk_external) {
		an_il_to_str_output_control_block* ocb
			= ocb_of_string_buf(&trial_buf);
		a_boolean saved_flag = ocb->suppress_all_typedefs;
		ocb->suppress_all_typedefs = TRUE;
		add_attrib("c_proto(\"", &atr_required, &trial_buf);
		form_function_declarator(skip_typerefs(routine_type),
					 ocb);
		add_2_chars_to_string("\")", &trial_buf);
		ocb->suppress_all_typedefs = saved_flag;
	    }
	    if (atr_required) {
		add_to_string(terminate_string(&trial_buf), output_buffer);
		terminate_string(output_buffer);
	    }
	    else terminate_string(&trial_buf);	/* forget it */
	    if (routine_ptr->is_template_function) {
		a_template_arg_ptr tp_arg;
		for (tp_arg = routine_ptr->template_arg_list; tp_arg;
		     tp_arg = tp_arg->next) {
		    if (tp_arg->kind == (a_templ_arg_kind)tak_type) {
			format_rel(SET_symid_and_write_sym(tp_arg->variant.type, iek_type, output_buffer),
				   "argument", routine_symid, output_buffer);
			add_to_string("template", output_buffer);
			terminate_string(output_buffer);
		    }
		    else if (tp_arg->kind == (a_templ_arg_kind)tak_template) {
			format_rel(SET_symid_and_write_sym(tp_arg->variant.templ.ptr, iek_template, output_buffer),
				   "argument", routine_symid, output_buffer);
			add_to_string("template", output_buffer);
			terminate_string(output_buffer);
		    }
		    else if (!tp_arg->is_array_bound_of_unknown_type) {
			if (tp_arg->variant.constant->kind == ck_address) {
			    /* Make relation with entity pointed to by the
			     * address constant, not the constant itself.
			     */
			    format_rel(SET_symid_and_write_sym(tp_arg->variant.constant->variant.address.variant.variable, iek_variable, output_buffer), 
				       "argument", routine_symid, output_buffer);
			}
			else {
			    /* The constant may have been bypassed during the normal
			     * tree walk; ensure at least a SYM line here.
			     */
			    process_symbol_of_entry_if_needed((char*) tp_arg->variant.constant,
							      iek_constant);
			    format_rel(SET_symid_and_write_sym(tp_arg->variant.constant, iek_constant, output_buffer),
				       "argument", routine_symid, output_buffer);
			}
			add_to_string("template", output_buffer);
			terminate_string(output_buffer);
		    }
		}
	    }
	    if (write_noninvertible_relations && extra_info && extra_info->exception_specification) {
		an_exception_specification_type_ptr throw_spec;
		for (throw_spec = extra_info->exception_specification->
		     exception_specification_type_list; throw_spec;
		     throw_spec = throw_spec->next) {
		    if (!throw_spec->redundant) {
			format_rel(routine_symid, "throw",
				   SET_symid_and_write_sym(throw_spec->type, iek_type, output_buffer),
				   output_buffer);
			add_to_string("declared", output_buffer);
			terminate_string(output_buffer);
		    }
		}
	    }
            if (!in_il_walk) {
                /* write "define" relation for non member function reference */
                if (!routine_ptr->source_corresp.is_class_member && 
                    routine_ptr->assoc_scope != NULL_region_number && 
                    !is_auto_gen_rout &&
		    !routine_ptr->compiler_generated) {
                    a_scope_ptr scope_ptr =
			il_header.region_scope_entry[routine_ptr->assoc_scope];
                    a_statement_ptr assoc_block = (scope_ptr) ? 
                                                  scope_ptr->assoc_block :
                                                  NULL;
                    if (assoc_block) {
                        write_define_relation(routine_ptr->source_corresp.decl_position.seq,
                                              routine_symid,
                                              output_buffer);
                    }    
                }
            }
	    if (write_parent_relations) {
		/* write "context" REL between routine and its parent 
		   class/struct/union or namespace */
		if (routine_ptr->source_corresp.is_class_member) {
		  a_type_ptr parent = routine_ptr->source_corresp.parent.class_type;
		  write_context_rel(SET_symid_and_write_sym(parent, iek_type, 
							    output_buffer), 
				    routine_symid, 
				    routine_ptr->source_corresp.access,
				    output_buffer);

                  /* Also write "static" attribute for static member 
                     routine. */
                  if (extra_info && !extra_info->this_class) {
                    write_atr(routine_symid, "static", output_buffer);
                  }
		}
		else {
		  a_namespace_ptr namespace_ptr = 
		    routine_ptr->source_corresp.parent.namespace_ptr;
		  if (namespace_ptr) {
		    write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, 
						      output_buffer),
			      "context",
			      routine_symid,
			      output_buffer);
		  }
		}

		/* write "specialize" REL between instantiation and prototype template */
		if (!routine_ptr->is_prototype_instantiation && routine_ptr->assoc_template) {
		    write_specialize_rel(routine_symid,
				         SET_symid_and_write_sym(routine_ptr->assoc_template, 
							         iek_template, 
							         output_buffer),
				         NULL,
				         routine_ptr->is_specialized,
				         output_buffer);
		}
	    }
	}
    }
	break;

    case iek_label: {
	a_label_ptr label_ptr = (a_label_ptr) entry_ptr;
	if (label_ptr->source_corresp.name) {
	    insert_sym_prefix("label", label_ptr, iek_label, FALSE, FALSE, output_buffer);
	    form_name(&label_ptr->source_corresp, iek_label,
		      ocb_of_string_buf(output_buffer));
	    finish_name(label_ptr, iek_label, /*add_block_qual=*/FALSE,
			/*add_closing_quote=*/TRUE, output_buffer);
	    if (mark_definitions && !in_template_function) {
		add_sym_def_pos_and_len_to_string(&label_ptr->source_corresp.decl_position,
					  output_buffer);
	    }
	    terminate_string(output_buffer);
	}
    }
	break;

    case iek_expr_node: {
	if (in_il_walk) {
	    an_expr_node_ptr expr = (an_expr_node_ptr) entry_ptr;
	    a_boolean symid_slot_taken = FALSE;
	    a_boolean compiler_generated = FALSE;
	    a_boolean use_only_range_start_and_len = FALSE;
	    switch(expr->kind) {
	    case enk_new_delete: {
		a_new_delete_supplement_ptr new_del = (a_new_delete_supplement_ptr)
			expr->variant.new_delete;
		if (new_del->routine) {
		    if (fcn_id && !(generate_multiple_IF_files && 
				    is_auto_gen_routine(curr_fcn))) {
			write_symbol_for_fcn_id(output_buffer);
			write_rel(fcn_id, "call", SET_symid_and_write_sym(new_del->routine, iek_routine, output_buffer), 
				  output_buffer);
		    }
		}
	    }
		break;
	    case enk_routine_address:
		if (!expr->variant.routine.ptr) {
		    /* NULL pointer to function: we have to manufacture a SYM. */
		    insert_sym_prefix("constant", expr, iek_expr_node, FALSE, FALSE, output_buffer);
		    add_to_string("0\"", output_buffer);
		    terminate_string(output_buffer);
		    write_rel(SET_symid_and_write_sym(expr->type, iek_type, output_buffer), 
			      "type", SET_symid_of(expr, iek_expr_node), output_buffer);
		}
		break;
	    case enk_operation:
		if (expr->kind == enk_operation &&
		    expr->variant.operation.kind == eok_call) {
		    an_expr_node_ptr callee =
			    expr->variant.operation.operands;
		    if (fcn_id && callee->kind == enk_routine_address &&
			!(generate_multiple_IF_files && 
			  is_auto_gen_routine(curr_fcn))) {
			write_symbol_for_fcn_id(output_buffer);
			write_rel(fcn_id, "call",
				  SET_symid_and_write_sym(callee->variant.routine.ptr, iek_routine, output_buffer),
				  output_buffer);
		    }
		}
		break;
	    default:
		break;
	    }
	}
    }
	break;

#ifdef CIL
    case iek_for_loop: {
    }
	break;

    case iek_switch_clause: {
    }
	break;

    case iek_handler: {
	if (in_il_walk) {
	    a_handler_ptr handler = (a_handler_ptr) entry_ptr;
	    write_symbol_for_fcn_id(output_buffer);
	    if (handler->parameter) {
		write_rel(fcn_id, "catch",
			  SET_symid_and_write_sym(handler->parameter->type, iek_type, output_buffer),
			  output_buffer);
	    }
	    else write_rel(fcn_id, "catch", symid_of_ellipsis(output_buffer),
			   output_buffer);
	}
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
    }
	break;

    case iek_object_lifetime: {
    }
	break;

    case iek_scope: {
	if (in_il_walk) {
	    a_scope_ptr ptr = (a_scope_ptr) entry_ptr;
	    if (ptr->kind == sck_function) {
		a_routine_ptr routine_ptr = ptr->variant.routine.ptr;
		if (routine_ptr && ast_info_of(routine_ptr)) {
		    a_variable_ptr parm;
		    unsigned int argno = 1;
		    if (generate_multiple_IF_files) {
			string_buffer_ptr alt_output_buffer
			    = get_buffer_from_il_node(routine_ptr);
			if (alt_output_buffer != NULL
                            && output_buffer != alt_output_buffer) {

			    output_buffer = alt_output_buffer;
			}
		    }
		    write_symbol_for_fcn_id(output_buffer);
		    for (parm = ptr->variant.routine.parameters;
			 parm; parm = parm->next) {
			format_rel(SET_symid_and_write_sym(parm, iek_variable, output_buffer), 
				   "argument", fcn_id, output_buffer);
			add_num_to_string(argno++, "order(", ")", output_buffer);
			terminate_string(output_buffer);
		    }
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
	a_namespace_ptr namespace_ptr = (a_namespace_ptr) entry_ptr;
	insert_sym_prefix("namespace", namespace_ptr, iek_namespace, FALSE, FALSE, output_buffer);
	form_name(&namespace_ptr->source_corresp, iek_namespace,
		  ocb_of_string_buf(output_buffer));
	add_1_char_to_string('\"', output_buffer);
	terminate_string(output_buffer);

    	if (!sym_line_only) {
	    unsigned long namespace_symid = SET_symid_of(namespace_ptr, iek_namespace);
	    if (namespace_ptr->is_namespace_alias) {
		write_rel(namespace_symid, 
			  "alias",
			  SET_symid_and_write_sym(namespace_ptr->variant.assoc_namespace, iek_namespace, output_buffer),
			  output_buffer);
	    }
	    else {
		a_scope_ptr scope = namespace_ptr->variant.assoc_scope;
		if (write_child_relations && scope) {
		    a_constant_ptr constant;
		    a_type_ptr type;
		    a_variable_ptr variable;
		    a_routine_ptr routine;
		    a_namespace_ptr nested_namespace;
		    a_using_decl_ptr using_decl;
		    string_buffer_ptr bp = output_buffer;
		    for (constant = scope->constants; constant;
			 constant = constant->next) {
			write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
				  "context",
				  SET_symid_and_write_sym(constant, iek_constant, bp), 
				  bp);
		    }
		    for (type = scope->types; type; type = type->next) {
			write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
				  "context",
				  SET_symid_and_write_sym(type, iek_type, bp), 
				  bp);
		    }
		    for (variable = scope->variables; variable;
			 variable = variable->next) {
			write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
				  "context",
				  SET_symid_and_write_sym(variable, iek_variable, bp), 
				  bp);
		    }
		    for (routine = scope->routines; routine;
			 routine = routine->next) {
			write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
				  "context",
				  SET_symid_and_write_sym(routine, iek_routine, bp), 
				  bp);
		    }
		    for (nested_namespace = scope->namespaces;
			 nested_namespace;
			 nested_namespace = nested_namespace->next) {
			write_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
			          "context",
				  SET_symid_and_write_sym(nested_namespace, iek_namespace, bp), 
				  bp);
		    }
                } 
                if (write_noninvertible_relations && scope) {
		    a_using_decl_ptr using_decl;
		    string_buffer_ptr bp = output_buffer;
		    for (using_decl = scope->using_decls; using_decl;
			 using_decl = using_decl->next) {
                        if (generate_multiple_IF_files) {
                            bp = get_buffer_from_il_node(using_decl); 
                        } 
                        if (bp) {
			    format_rel(SET_symid_and_write_sym(namespace_ptr, iek_namespace, bp), 
			               "context",
				       SET_symid_and_write_sym(using_decl->entity.ptr, using_decl->entity.kind, bp),
				       bp);
		            add_to_string("using", bp);
		            terminate_string(bp);
		        }
                    }
		}
		if (write_parent_relations && !namespace_ptr->source_corresp.is_class_member) {
		    a_namespace_ptr parent_namespace = 
		      namespace_ptr->source_corresp.parent.namespace_ptr;
		    /* write "context" REL between namespace and its parent namespace */
		    if (parent_namespace) {
		      write_rel(SET_symid_and_write_sym(parent_namespace, iek_namespace, 
							output_buffer),
				"context",
				namespace_symid,
				output_buffer);
		    }
		}
    	    }
    	}
    }
	break;

    case iek_using_decl: {
    }
	break;

    case iek_dynamic_init: {
	if (in_il_walk) {
	    a_dynamic_init_ptr init = (a_dynamic_init_ptr) entry_ptr;
	    if (!init->variable && init->kind != dik_none && init->destructor &&
		init->lifetime && !init->is_constructor_init) {
		insert_sym_prefix("variable", init, iek_dynamic_init, /*compiler_generated=*/TRUE,
				  /*from_template=*/FALSE, output_buffer);
		add_to_string("<unnamed>", output_buffer);
		finish_name(init, iek_dynamic_init, /*add_block_qual=*/TRUE,
			    /*add_closing_quote=*/TRUE, output_buffer);
		terminate_string(output_buffer);
		if (fcn_name) {
		    write_atr(SET_symid_of(init, iek_dynamic_init), 
			      "auto", output_buffer);
		}
	    }
	    if (init->kind == dik_constructor) {
		if (fcn_id && !(generate_multiple_IF_files && 
				is_auto_gen_routine(curr_fcn))) {
		    write_symbol_for_fcn_id(output_buffer);
		    write_rel(fcn_id, "call",
			      SET_symid_and_write_sym(init->variant.constructor.ptr, iek_routine, output_buffer),
			      output_buffer);
		}
	    }
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
		if (!is_base_dtor && !(generate_multiple_IF_files && 
				       is_auto_gen_routine(curr_fcn))) {
		    /* We suppress calls to base class destructors because
		     * the reference to the base class destructor from the
		     * derived class constructor, with a virtual destructor,
		     * makes the "where used" query useless: if you have
		     * D1 and D2 both derived from B, you get the ridiculous
		     * result that D1's destructor is called from D2's
		     * constructor!
		     */
		    write_symbol_for_fcn_id(output_buffer);
		    write_rel(fcn_id, "call",
			      SET_symid_and_write_sym(init->destructor, iek_routine, output_buffer), output_buffer);
		}
	    }
	}
    }
	break; 

    case iek_local_static_variable_init: {
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
    }
	break;

    case iek_asm_entry: {
    }
	break;

    case iek_template_arg: {
    }
	break;

    case iek_new_delete_supplement: {
    }
	break;

    case iek_throw_supplement: {
	if (in_il_walk) {
	    a_throw_supplement_ptr throw = (a_throw_supplement_ptr) entry_ptr;
	    write_symbol_for_fcn_id(output_buffer);
	    format_rel(fcn_id, "throw", SET_symid_and_write_sym(throw->type, iek_type, output_buffer),
		       output_buffer);
	    add_to_string("actual", output_buffer);
	    terminate_string(output_buffer);
	}
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
    }
	break;

    case iek_src_seq_secondary_decl: {
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
	if (in_il_walk) {
	    orphaned_list_parent_seq_list = NULL;
	}
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
	a_symbol_ptr sym = (a_symbol_ptr) template->
		source_corresp.assoc_info;
	a_template_symbol_supplement_ptr template_info;
	a_tagged_pointer parent_tag = {0, NULL};
	unsigned long parent_symid = 0;
	a_type_kind kw_kind = tk_class;
	a_boolean is_auto_gen = is_auto_gen_member(&template->source_corresp);
	a_boolean defined_externally = FALSE;

	a_boolean is_canonical = (template == template->canonical_template);
	a_template_ptr definition_template = template->canonical_template != NULL
	    ? template->canonical_template->definition_template
	    : NULL;
	a_boolean is_definition = (definition_template == template);
        a_type_ptr parent = NULL;

	switch (template->kind) {
	case templk_class:
	case templk_member_class:
	  if (template->prototype_instantiation.type != NULL) {
	    a_type_ptr inst_type = template->prototype_instantiation.type;
	    if (inst_type != NULL) {
	      parent = inst_type->source_corresp.parent.class_type;
	    }
	  }
	  break;
	case templk_function:
	case templk_member_function:
	  if (template->prototype_instantiation.routine != NULL) {
	    a_routine_ptr inst_rout = template->prototype_instantiation.routine;
	    if (inst_rout) {
	      parent = inst_rout->source_corresp.parent.class_type;
	    }
	  }
	  break;
	case templk_static_data_member:
	  if (template->prototype_instantiation.variable != NULL) {
	    a_variable_ptr inst_var = template->prototype_instantiation.variable;
	    if (inst_var) {
	      parent = inst_var->source_corresp.parent.class_type;
	    }
	  }
	  break;
	default:
	  break;
	}    


	if (is_auto_gen || is_specialized(template)) {
	    /* Can't identify this template with the canonical declaration. */
	}
	else {
	    a_template_ptr external_tmpl = NULL;
	    switch (template->kind) {
	      case templk_function:
	      case templk_member_function:
		if (template->prototype_instantiation.routine != NULL) {
		    external_tmpl = has_tpl_entry(template->prototype_instantiation.routine);
		}
		break;
	      case templk_static_data_member:
		if (template->prototype_instantiation.variable != NULL) {
		    external_tmpl = variable_has_tpl_entry(template->prototype_instantiation.variable);
		}
		break;
	      default:
		break;
	    }
	    defined_externally = (external_tmpl != NULL && external_tmpl->canonical_template != template->canonical_template);
	}
	if (is_definition && sym == NULL) {
	    sym = (a_symbol_ptr)template->canonical_template
					->source_corresp.assoc_info;
	}
	if (sym == NULL) {
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
	    if (!in_il_walk) {
    	    	/* Complain because this check wasn't here before the modularization,
		   and it could cause a change in results. */
		complain(error_csev, "Expected symbol for template.");
	    }
	    break;
	}


	if (in_il_walk && ast_info_of(template)) {
	    /* Must be a member template, already processed */
	    break;
	}

	if (template->kind == templk_class ||
	    template->kind == templk_member_class) {
	    if (template->kind == templk_class) {
		template_info = sym ? sym->variant.template_info :
			NULL;
		if (template_info) {
		    kw_kind = template_info->variant.class_template.type_kind;
		}
	    }
	    else kw_kind = sym->variant.class_struct_union.type->kind;
	}

	/* During il_walk output the SYM line & relations
	 * if this is the definition or the canonical declaration. */
	if ((definition_template != NULL ? is_definition : (is_canonical && !defined_externally)) ||
	    is_auto_gen ||
	    is_specialized(template) ||
	    !in_il_walk) {

	    if (template->kind == templk_class ||
		template->kind == templk_member_class) {
		if (kw_kind == tk_struct) {
		    insert_sym_prefix("struct", template, iek_template, FALSE, TRUE, output_buffer);
		}
		else if (kw_kind == tk_union) {
		    insert_sym_prefix("union", template, iek_template, FALSE, TRUE, output_buffer);
		}
		else insert_sym_prefix("class", template, iek_template, FALSE, TRUE, output_buffer);
	    }
	    else if (template->kind == templk_function ||
		     template->kind == templk_member_function) {
		insert_sym_prefix("function", template, iek_template, FALSE, TRUE, output_buffer);
	    }
	    else if (template->kind == templk_static_data_member) {
		insert_sym_prefix("variable", template, iek_template, FALSE, TRUE, output_buffer);
	    }
	    else {
		/* This might occur after a source error. */
		insert_sym_prefix("template", template, iek_template, FALSE, FALSE, output_buffer);
	    }
	    add_to_string(template_qual_name_and_parent_symid(sym, &parent_tag), 
			  output_buffer);
	    add_1_char_to_string('\"', output_buffer);
	    if (mark_definitions && (is_definition ||
		 template->kind == templk_static_data_member)) {
		/* We won't see a static data member template unless we
		 * see the definition; the declaration inside the class
		 * won't create a template node.  Otherwise, we check
		 * for the definition range to see if the template was
		 * defined and we should issue a position.
		 */
		add_sym_def_pos_to_string(&template->source_corresp.decl_position,
				          output_buffer);
	    }
	    terminate_string(output_buffer);

	    if (!sym_line_only || is_auto_gen_member(&template->source_corresp)) {
		unsigned long template_symid = SET_symid_of(template, iek_template);

		write_atr(template_symid, "template", output_buffer);
		if (parent_tag.ptr) {
		    parent_symid = SET_symid_and_write_sym((void*)parent_tag.ptr, 
							   parent_tag.kind,
							   output_buffer);
		}
		if (parent_symid) {
		    write_context_rel(parent_symid, template_symid,
				      template->source_corresp.access,
				      output_buffer);
		}
		if (write_parent_relations) {
                    if (parent) {
		        write_context_rel(SET_symid_and_write_sym(parent, iek_type, 
							          output_buffer), 
				          template_symid, 
				          template->source_corresp.access,
				          output_buffer);
		    }

                    /* write "static" attribute */
                    if (template->kind == templk_member_function) {
                        a_routine_ptr inst_rout = template->prototype_instantiation.routine;
                        if (inst_rout && inst_rout->source_corresp.is_class_member) {
                            a_type_ptr inst_rout_type = inst_rout->type;
                            a_routine_type_supplement_ptr extra_info
		                    = (inst_rout_type->kind == tk_routine) ?
		                    inst_rout_type->variant.routine.extra_info : NULL;
                            if (extra_info && !extra_info->this_class) {
                                write_atr(template_symid, "static", output_buffer);
                            }
	                }
                    }
                    else if (template->kind == templk_static_data_member) {
		        write_atr(template_symid, "static", output_buffer);
                    }
		}
		if (write_parent_relations &&  
		    (template->kind == templk_class || 
		     template->kind == templk_member_class)) {
		    /* Write "specialize" relation with "partial" attribute between
		     * a partially specialized template and the primary template.
		     */    
		    a_symbol_ptr primary_template_sym;
		    a_template_ptr primary_template;
		    a_template_symbol_supplement_ptr primary_supp;
		    a_symbol_ptr psym;
		    if (template->kind == templk_class) {
			template_info = sym->variant.template_info;
		    }
		    else {
			template_info = sym->variant.class_struct_union.extra_info
					->template_info;
		    }
		    if (template_info && sym->kind == sk_class_template) {
			/* For a partial specialization, primary_template_sym points 
			 * back to the primary template of which this is a partial
			 * specialization.
			 */ 
			primary_template_sym = template_info->variant.class_template.primary_template_sym;
			if (primary_template_sym &&
			    primary_template_sym->kind == sk_class_template) {
			    primary_supp = primary_template_sym->variant.template_info;
			    primary_template = primary_supp->il_template_entry;
			    /* Goes through all the partial specializations of the 
			     * primary template until one of them matches with the 
			     * current template.
			     */
			    psym = primary_supp ? primary_supp->variant.class_template.partial_specializations : NULL;
			    for (; psym; psym = psym->next) {
				a_template_ptr specialization = psym->variant.template_info->il_template_entry;
				if (specialization == template) {
				    /* The current template is a partial specialization,
				       write a "partial", "specialize" relation. */
				    write_specialize_rel(template_symid,
							 SET_symid_and_write_sym(primary_template, iek_template, output_buffer),
							 /*full_or_part=*/"partial",
							 /*explicit=*/TRUE,
							 output_buffer);
				    break;
				}
			    }
			}
		    }
		}
		if (write_child_relations && sym) {
		    if (template->kind == templk_class ||
			template->kind == templk_member_class) {
			if (template->kind == templk_class) {
			    template_info = sym->variant.template_info;
			}
			else {
			    template_info = sym->variant.class_struct_union.extra_info
					       ->template_info;
			}
			sym = template_info ? template_info->variant.class_template.instantiations
					    : NULL;
			for (; sym; sym = next_instance_sym(sym)) {
			    a_type_ptr type = sym->variant.class_struct_union.type;
			    if (type) {
				write_specialize_rel(SET_symid_and_write_sym(type, iek_type, output_buffer),
							  template_symid,
							  /*full_or_part=*/"full",
							  /*explicit=*/type->variant.
							  class_struct_union.is_specialized,
							  output_buffer);
				process_symbol_of_entry_if_needed((char *)type, iek_type);
			    }
			}
			sym = template_info ? template_info->variant.class_template.partial_specializations
					    : NULL;
			for (; sym; sym = sym->next) {
			    a_template_ptr specialization = 
				    sym->variant.template_info->il_template_entry;
			    if (specialization) {
				write_specialize_rel(SET_symid_and_write_sym(specialization, iek_template, output_buffer),
						     template_symid,
						     /*full_or_part=*/"partial",
						     /*explicit=*/TRUE,
						     output_buffer);
			    }
			}
		    }
		    else if (template->kind == templk_function ||
			     template->kind == templk_member_function) {
			a_template_instance_ptr inst;
			if (template->kind == templk_function) {
			    template_info = sym->variant.template_info;
			}
			else if (sym->variant.routine.instance_ptr != NULL) {
			    template_info = sym->variant.routine.instance_ptr->template_info;
			}
			for (inst = template_info
					? template_info->variant.function.instantiations
					: NULL;
			     inst; inst = inst->next) {
			    if (inst->instance_sym != inst->template_sym) {
				/* this is a "real" instance, not a member of
				 * a prototype instantiation.
				 */
				a_routine_ptr fcn = inst->instance_sym->
					variant.routine.ptr;
				write_specialize_rel(SET_symid_and_write_sym(fcn, iek_routine, output_buffer),
						     template_symid,
						     /*full_or_part=*/NULL,
						     /*explicit=*/fcn->is_specialized,
						     output_buffer);
			    }
			}
		    }
		    else if (template->kind == templk_static_data_member) {
			a_template_instance_ptr inst;
			inst = sym->variant.static_data_member.instance_ptr;
			if (inst && inst->template_info) {
			    a_template_instance_ptr definition;
			    for (definition = inst->template_info->variant.
				 static_data_member.definitions; definition;
				 definition = definition->next) {
				if (definition->instance_sym) {
				    a_variable_ptr var = definition->instance_sym->
					    variant.static_data_member.variable;
				    if (var) {
					write_specialize_rel(SET_symid_and_write_sym(var, iek_variable, output_buffer),
							     template_symid,
							     /*full_or_part=*/NULL,
							     /*explicit=*/var->is_specialized,
							     output_buffer);
				    }
				}
			    }
			}
		    }
		}
	    }
	} /* end of handling SYM line etc. for defn or canonical decl */
	else {
	    /* Not the primary occurrence of this template; don't generate SYM.
	     * e.g. for a member template of a class template, it recurs as
	     * a member of the prototype instantation, t165.C. */
	    if (!in_il_walk) {
		worry("Before modularization, would have written symbol info.");
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

        /* XXX: these enumerations need to be examined */
#if RECORD_FORM_OF_NAME_REFERENCE
    case iek_name_reference:
    case iek_name_qualifier:
#endif /* RECORD_FORM_OF_NAME_REFERENCE */
#if MICROSOFT_EXTENSIONS_ALLOWED
    case iek_ms_attribute:
    case iek_ms_attribute_arg:
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
    case iek_seq_number_lookup_entry:
#if MACRO_INVOCATION_TREE_IN_IL
    case iek_macro_invocation_record_block:
#endif /* MACRO_INVOCATION_TREE_IN_IL */
#if GENERATE_MICROSOFT_IF_EXISTS_ENTRIES
    case iek_ms_if_exists:
#endif /* GENERATE_MICROSOFT_IF_EXISTS_ENTRIES */
    case iek_local_expr_node_ref:
#if EXPR_RANGE_MODIFIERS_IN_IL
    case iek_expr_range_modifier:
#endif /* EXPR_RANGE_MODIFIERS_IN_IL */
#if GENERATE_SOURCE_SEQUENCE_LISTS
    case iek_static_assertion:
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
        break;
        /* /XXX: pending enumerations */

    default:
	complain_int(error_csev, "Unsupported entry kind $1", entry_kind);
	break;
    } /* switch */
} /* write_symbol */

void write_referenced_symbol_info(char *entry_ptr, 
				  an_il_entry_kind entry_kind,
				  string_buffer_ptr output_buffer,
				  a_boolean sym_line_only) {
    befriend_clique(entry_ptr, entry_kind);
    write_symbol(entry_ptr, entry_kind, output_buffer, sym_line_only, FALSE, FALSE);
}

void write_symbol_of_entry(char *entry_ptr, an_il_entry_kind entry_kind,
					    a_boolean in_il_walk,
					    a_boolean write_noninvertible_relations) {
    string_buffer_ptr output_buffer = get_buffer_from_il_node(entry_ptr);
    write_symbol(entry_ptr, entry_kind, output_buffer,
                 FALSE, in_il_walk, write_noninvertible_relations);
}

static void process_symbol_of_entry(char *entry_ptr, an_il_entry_kind entry_kind) {

    if (!skip_entry(entry_ptr, entry_kind)) {
	write_symbol_of_entry(entry_ptr, entry_kind, FALSE, TRUE);
    }
} /* process_symbol_of_entry */

/* The following function is called in response to the command line
 * option --SET_memory_metering; it shows the memory usage of all the
 * data structures managed in this file.
 */

void dump_symbol_memory_meters() {
    fprintf(stderr, "sym_summary_meter =\t\t%10lu\n", (unsigned long)sym_summary_meter);
}
