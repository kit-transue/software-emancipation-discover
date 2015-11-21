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
#ifndef SET_additions_h
#define SET_additions_h

/* This file contains declarations for the routines added by Software
 * Emancipation Technology for management and dumping of information
 * to the External Language Support Intermediate Format file.
 */

#include "symbol_tbl.h"
#include "SET_names.h"

/* The following function is called to initialize output buffers
 * before the parse begins.
 */

extern void init_SET_data(void);

/* The following function is called whenever a scope node is
 * encountered during the IL walk (at the beginning, before walking
 * down to enclosed declarations, statements, etc.).
 */

extern void notify_entering_scope(a_scope_ptr);

/* The following function is called whenever a source sequence
 * sublist is entered during the IL walk, allowing the entries in
 * the list to be ignored until the reference to the sublist in
 * its logical location is encountered.
 */

void notify_entering_src_seq_sublist(a_src_seq_sublist_ptr sublist);

/* The following function is called whenever an orphaned list header
 * is encountered during the IL walk, allowing subsequent calls to
 * notify_entering_src_seq_sublist to identify the source sequence
 * list to which the sublist belongs.
 */

void notify_entering_orphaned_list(a_scope_orphaned_list_header_ptr);

/* The following flag is maintained by open_file_and_push_input_stack
 * and pop_input_stack.  It is set to indicate whether the entities in
 * the file at the top of the stack should be reported fully in the IF
 * or only in summary form and only when referenced from an "interesting"
 * file.
 */

extern intr_info_ptr SET_file_is_interesting;

/* The following function provides the intelligence for determining
 * whether a file is interesting or not.
 */

extern intr_info_ptr SET_is_file_interesting(const char* filename,
					     a_boolean is_header_file);

/* Returns a pointer to the current block number. */
extern int *block_stack_top(void);

/* Note leaving a block during the il walk. */
extern void pop_block(void);

/* variables that track current function during il walk */
extern const char *fcn_name;
extern a_routine_ptr curr_fcn;
extern a_scope_ptr curr_fcn_scope;
extern a_boolean in_template_function;
extern unsigned long fcn_id;

/* more variables needed to track state of il walk */
extern a_boolean in_src_seq_sublist;
extern a_source_sequence_entry_ptr orphaned_list_parent_seq_list;

/* The following function returns its argument, converted into an
 * absolute path if necessary.  If the argument is already an
 * absolute path, the result is just the argument; otherwise, the
 * function constructs an absolute path (in the trial_buf) using
 * the relative path base passed in as a command line argument.
 */

const char* absolute_path_of(const char* filename);

/* Given a SET_outbuf_bit_arr for a symbol, and an output buffer,
 * returns whether its SYM line has been written. */
extern a_boolean is_output_buffer_bit_set(unsigned char **, 
					  string_buffer_ref);

/* Given a SET_outbuf_bit_arr for a symbol, and an output buffer,
 * sets the bit to indicate that its SYM line has been written. */
extern a_boolean set_output_buffer_bit(unsigned char **, 
			               string_buffer_ref);

extern an_il_to_str_output_control_block *ocb_of_string_buf(string_buffer_ref);

extern void set_string_buffer_terminator(char, string_buffer_ptr);

/* Stuff an absolute path for the given file into the buffer 'bp',
 * terminating the string and returning the result. */
extern char* get_absolute_path(const char* filename, string_buffer_ref bp);

#define MAX_FILENAME_SIZE 4096

extern const char* terminate_string(string_buffer_ref bp);
extern void handle_output_overflow(string_buffer_ref bp);
extern void flush_all_output_buffers(void);

extern void add_num_to_string(unsigned long num, const char* pre_str,
			      const char* post_str, string_buffer_ref bp);
extern a_boolean add_file_rel_pos_and_len_to_string(a_source_position_ptr pos,
				                    a_seq_number start_seq,
				                    unsigned long start_lineno,
				                    string_buffer_ref bp);
extern a_boolean add_file_rel_pos_to_string(a_source_position_ptr pos,
				            a_seq_number start_seq,
				            unsigned long start_lineno,
				            string_buffer_ref bp);
extern void add_SMT_language_to_string(string_buffer_ref output_buffer);
extern void add_symid_to_string(unsigned long id, string_buffer_ref bp);
extern void add_quoted_str_to_string(const char* str,
				     a_boolean add_quotes,
				     string_buffer_ref bp);
extern void add_1_char_to_string(char ch, string_buffer_ref bp);
extern void add_to_string_with_len(const char* text, size_t len,
				   string_buffer_ref bp);
extern a_boolean add_pos_to_string(a_source_position_ptr pos,
			           string_buffer_ref bp);
extern a_boolean add_pos_to_string_checking(a_source_position_ptr pos,
					    string_buffer_ref bp,
					    string_buffer_ptr ckbuf);
extern void add_2_chars_to_string(char* str, string_buffer_ref bp);
extern void add_3_chars_to_string(char* str, string_buffer_ref bp);
extern a_boolean add_pos_and_len_to_string(a_source_position_ptr pos,
			                   string_buffer_ref bp);
extern a_boolean add_pos_and_len_to_string_checking(a_source_position_ptr pos,
						    string_buffer_ref bp,
						    string_buffer_ptr ckbuf);
extern void add_to_string(const char* text, string_buffer_ref bp);


/* Called after preprocessing to account for preprocessing usage
 * versus later usage. */
extern void reset_meters_after_preprocessing(void);

/* Called prior to termination to dump memory usage analysis. */
extern void dump_memory_meters(void);

extern void init_db(void);

#if DEBUG
extern a_boolean db_ast;
extern a_boolean db_expected_syms;
extern a_boolean db_statement_stack;
extern a_boolean db_source_sequence;
extern a_boolean db_symid;
extern a_boolean db_same_symid;
extern a_boolean db_walk;
extern a_boolean db_worry;
#endif /* DEBUG */

/* ** access to tables of source file information ** */

/* Returns the output buffer (if applicable) for the IF file associated
 * with the given source file. */
extern string_buffer_ptr get_buffer_of_source_file(a_source_file_ptr sfp);

/* Returns the info object (if applicable) for the given source file. */
extern intr_info_ptr get_intr_info_from_source_file(a_source_file_ptr sfp);

/* Notes in the given source file's info object, the preprocessing AST. */
extern void set_pp_ast_of_source_file(a_source_file_ptr sfp,
				      struct pp_AST_node_info *);

/* Returns the preprocessing AST for the given source file. */
extern struct pp_AST_node_info *get_pp_ast_of_source_file(a_source_file_ptr);

/* Returns the source file info object associated with the given il node. 
 * May return NULL, even if ilp is not. */
extern intr_info_ptr get_intr_info_from_il_node(const void *ilp);

/* Returns the output buffer (if applicable) for the IF file
 * associated with the given il node. */
extern string_buffer_ptr get_buffer_from_il_node(const void *ilp);

/* Returns the output buffer (if applicable) for the IF file
 * associated with the given sequence number in the source. */
extern string_buffer_ptr get_buffer_from_source_seq(a_seq_number seq);

/* Returns the source file info object
 * associated with the given sequence number in the source. 
 * May return NULL, even if seq is valid. */
extern intr_info_ptr get_intr_info_from_source_seq(a_seq_number seq);

/* Returns the output buffer for the IF file of the compilation
 * unit's primary source file. */
extern string_buffer_ptr get_primary_source_buffer(void);

extern a_boolean adjust_routine_output_buffer(a_routine_ptr routine_ptr);
extern a_boolean adjust_variable_output_buffer(a_variable_ptr variable_ptr);
extern a_boolean adjust_class_struct_union_output_buffer(a_type_ptr); 

extern void set_outbuf_bit_of_clique(void *, string_buffer_ref);

#define interesting_node(ilp) ((ilp) ? (il_entry_prefix_of(ilp).full_SET_dump) : NULL)

extern const char* get_filename_from_hash_bucket(void *);

/* Structure for interesting file information. 
 */

typedef struct intr_info {
  string_buffer_ref buf_ptr; /* a string buffer to hold IF, never NULL */

  ast_node_info_ptr first_ast_node; /* first ast node for this file */
  ast_node_info_ptr last_ast_node; /* last ast node for this file */

  struct pp_AST_node_info *pp_ast; /* preprocessor AST node info */

  char *filename; /* name of the interesting file this represents */ 
} intr_info;

/* List of info objects for all interesting files. */
extern intr_info_ref *intr_info_array;
extern unsigned int intr_info_count;

/* Returns the output buffer (if applicable) for the IF file
 * associated with the source file whose info object is given. */
#define get_buffer_from_intr_info(nd) ((nd) ? \
				       nd->buf_ptr : NULL)

#endif /* SET_additions_h */
