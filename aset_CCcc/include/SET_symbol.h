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
/* generation of names, SYM, REL, and ATR lines for the IF
 *
 * This is separated from generation of the AST in SET_ast
 * because there is not always a one-to-one correspondence
 * between declarations and symbols.
 */

#ifndef SET_symbol_h
#define SET_symbol_h
#include "SET_names.h"
#include "il.h"

/* The following function creates an "<unnamed...>" string for a
 * nameless type. */
extern void form_unnamed(a_type_ptr, an_il_to_str_output_control_block_ptr);

/* Called after all AST generation, to write SYM lines for type symbols
 * that would otherwise have been missed. */
extern void retry_process_symbol(void);

/* Causes the SYM, REL, and ATR lines to be written for the given entry. */
extern void write_symbol_of_entry(char *entry_ptr, an_il_entry_kind entry_kind,
						   a_boolean in_il_walk,
						   a_boolean force_relations);

/* Writes a SYM line for the given symbol to the given output buffer,
 * and unless 'sym_line_only' is set, also REL and ATR lines. */
extern void write_referenced_symbol_info(char *entry_ptr, 
					 an_il_entry_kind entry_kind,
					 string_buffer_ptr output_buffer,
					 a_boolean sym_line_only);

/* Called early on to write the type REL line for the void * symbol. */
extern void write_type_rel_for_void_ptr(void* ilp, 
					an_il_entry_kind il_kind,
					string_buffer_ptr output_buffer);

extern void add_summary_info(void* ilp, const char* kind,
		             const char* name);

extern void write_summary_sym_info(void *ilp, 
				   an_il_entry_kind il_kind,
				   string_buffer_ptr output_buffer);

/* Simply add a the name of a function to a buffer. */
extern void add_fcn_name_to_string(a_routine_ptr routine_ptr,
				   string_buffer_ref bp);

/* Insert_sym_prefix and decorate_name_and_finish are used to gether
 * to write the SYM line for an entry. */
extern void insert_sym_prefix(const char* sym_kind, void* entry_ptr, an_il_entry_kind entry_kind,
			      a_boolean compiler_generated,
			      a_boolean from_template,
			      string_buffer_ref output_buffer);
extern void decorate_name_and_finish(const void* entry_ptr,
			             an_il_entry_kind entry_kind,
				     a_boolean add_block_qual,
				     a_boolean add_closing_quote,
				     string_buffer_ref bp);

/* Write a simple, complete REL line to the output buffer. */
extern void write_rel(unsigned long first_symid,
		      const char* rel_name,
		      unsigned long second_symid,
		      string_buffer_ptr output_buffer);

/* Write a simple, complete ATR line to the output buffer. */
extern void write_atr(unsigned long symid, 
		      const char* attrib,
		      string_buffer_ptr output_buffer);

/* Write a SYM line for the current function to the output buffer. */
extern void write_symbol_for_fcn_id(string_buffer_ptr bp);

/* Write specialize relations for instantations etc. of a template member. */
extern void write_tmpl_mbr_specialize_rels(a_symbol_ptr mbr,
					   unsigned long mbr_symid,
					   string_buffer_ptr output_buffer);

/* Write a SYM line for a source file to the given output buffer. */
extern void write_sym_line_for_source_file(a_source_file_ptr source, 
					   string_buffer_ptr output_buffer);

/* Write a SYM line and ATR for ellipsis. */
extern void write_ellipsis_symbol(unsigned long id, string_buffer_ptr output_buffer);

/* Write a context relation with the givne accessibility. */
extern void write_context_rel(unsigned long class_symid,
			      unsigned long member_symid,
			      int access,
			      string_buffer_ptr output_buffer);

/* Write a SYM line for the given il entry. */
extern void write_symbol_for_il_node(void *ilp,
				     an_il_entry_kind il_kind,
				     string_buffer_ptr output_buffer);

/* Write a complete specialize REL line, with attributes */
extern void write_specialize_rel(unsigned long inst_symid,
				 unsigned long template_symid,
				 const char* full_or_part,
				 a_boolean is_explicit,
				 string_buffer_ptr output_buffer);

/* Write an unterminated REL line. */
extern void format_rel(unsigned long, const char*, unsigned long,
		       string_buffer_ref);

/* Write a string representation fo the given contant
 * to the given output buffer. */
extern void constant_to_string(a_constant_ptr constant_ptr,
			       string_buffer_ref bp);

/* Dump memory usage information for symbol tracking. */
extern void dump_symbol_memory_meters(void);

#endif /* SET_symbol_h */
