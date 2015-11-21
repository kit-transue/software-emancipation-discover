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
/* analyze preprocessing and creating its IF */

#ifndef SET_preprocess_h
#define SET_preprocess_h
#include "SET_names.h"
#include "preproc.h"

#ifndef MACRO_CACHE_ENTRY_PTR_DEFINED
#define MACRO_CACHE_ENTRY_PTR_DEFINED
typedef struct a_macro_cache_entry* a_macro_cache_entry_ptr;
#endif /* MACRO_CACHE_ENTRY_PTR_DEFINED */

/* Note a macro call, when its slmp is created, for "expands" relation. */
extern void record_nested_macro_call(a_source_line_modif_ptr slmp);

/* The following function registers a reference to a macro at
 * the specified source location and returns the macro's
 * macro_cache_entry_ptr.
 */

extern a_macro_cache_entry_ptr register_macro_ref(const char* name,
						  sizeof_t name_len,
						  a_macro_param_ptr params,
						  a_source_position_ptr pos);

/* The following variable identifies the location of the "#" in a
 * preprocessor directive, for use in the preprocessor AST.
 */

extern a_source_position start_of_dir_position;

/* The following function notifies the macro cache that the named
 * macro has been defined and inserts a reference into the
 * preprocessor event log.  It also adds the preprocessor AST node.
 */

extern a_macro_cache_entry_ptr register_macro_definition(const char* name,
							 sizeof_t name_len,
							 a_boolean object_like,
							 a_macro_param_ptr params,
							 a_source_position_ptr pos,
							 const char* replacement,
							 a_boolean variadic);

/* The following function notifies the macro cache that the named
 * macro has been undefined and inserts a reference into the
 * preprocessor event log.
 */

a_macro_cache_entry_ptr register_macro_undefinition(const char* name,
						    sizeof_t name_len,
						    a_macro_param_ptr params,
						    a_source_position_ptr pos);

/* The following function inserts a cpp_ifdef/cpp_ifndef node into the
 * preprocessor AST and records a reference to the associated macro
 * (even if it is not defined).
 */

extern void register_ifdef(a_boolean is_ifdef, const char* name,
			   sizeof_t name_len, a_macro_param_ptr params);

/* The following function records a #else directive */

extern void register_else(void);

/* The following function records the end of an if/ifdef/ifndef block */

extern void register_endif(void);

/* The following function is called when #if, #ifdef, or #ifndef is
 * encountered while a conditional compilation clause is being
 * skipped.
 */

extern void register_skipped_if(a_pp_directive_kind kind);

/* The following variable points to the most recently evaluated
 * constant expression in a #if or #elif.
 */

extern an_expr_node_ptr last_pp_if_expr;

/* The following function inserts a cpp_if node into the preprocessor
 * AST and dumps the expression represented by "last_pp_if_expr" as
 * the first operand of the cpp_if node.
 */

extern void register_if(void);

/* Ditto for #elif */

extern void register_elif(void);

/* The following function inserts a cpp_include node into the
 * preprocessor AST.  It saves the token positions but leaves the
 * symid of the file being included and the name of the file empty, to
 * be updated later after the file is actually found and updated (must
 * be done in this order to get the token references right).
 */

extern void register_include(a_source_position_ptr end_of_directive);
extern void register_end_of_include();

/* The following function updates the most recent cpp_include
 * preprocessor AST node to reflect the symid and full name of the
 * file that was opened.
 */

extern void update_include_pp_AST_node(a_source_file_ptr header_file);

/* The following variables record the starting points for MAP fixed
 * and MAP copy regions; when the corresponding end is found, the MAP
 * region will be recorded in the preprocessor event log.
 */

extern a_source_position start_of_map_fixed_region;
extern a_source_position start_of_map_copy_region;

/* The following functions add MAP regions to the preprocessor event
 * log.
 */

extern void record_map_copy(a_source_position_ptr from,
			    a_source_position_ptr to);

extern void record_map_fixed(a_source_position_ptr from,
			     a_source_position_ptr to);

/* The following variables are used to calculate the current offset in
 * the (imaginary) output line resulting from preprocessing.
 * base_of_cur_offset is a pointer to the beginning of the text line
 * or text of the current source modification; cur_base_offset gives
 * the offset associated with that location; ignored_macro_escapes is
 * the number of LE_END_OF_TOKEN and LE_INERT_MACRO escapes that have
 * been seen (they are inserted into processed macro arguments and
 * thus affect the mapping.  The offset associated with curr_char_loc,
 * therefore, is calculated by the formula:
 *
 *	cur_base_offset + (curr_char_loc - base_of_cur_offset) -
 *		ignored_macro_escapes
 */

extern const char* base_of_cur_offset;
extern size_t cur_base_offset;
extern size_t ignored_macro_escapes;

/* The following two functions are called when entering and exiting
 * source modifications.
 */

extern void register_source_modif_entry_conditional(a_source_line_modif_ptr slmp);
extern void register_source_modif_entry(a_source_line_modif_ptr slmp);
extern void register_source_modif_exit(a_source_line_modif_ptr slmp);

/* The following struct represents a single token from the original
 * source that appears inside the expansion of a macro invocation.
 * All tokens from a given argument are linked into a single list,
 * which is then recorded first in the source line modification
 * for that list (for ANSI-style preprocessing) and then into the
 * macro_argument structure.  When macro arguments are used in the
 * expansion, the maps for the argument are copied into the list
 * for the source line modification representing the expansion.
 * When the source line modifications are eventually read as the
 * expanded output, the maps for the source line modifications are
 * used to create "MAP copy" lines for each token from a macro
 * argument that appeared in the original source line that made
 * it into the expanded source.
 */

typedef struct a_macro_arg_map* a_macro_arg_map_ptr;
typedef struct a_macro_arg_map {
    a_macro_arg_map_ptr next;
    a_source_position pos;
    const char* mapped_loc;
} a_macro_arg_map;


/* The following function is called for each token that is scanned
 * inside a macro argument; it either creates a new macro_arg_map
 * on the list currently being created (if the source location
 * indicates that the current source line is being scanned) or
 * moves an existing macro_arg_map onto the list (if the source
 * location is inside a source line modification and that
 * modification contains a map for that position).  Moving a
 * macro_arg_map from a source line modification updates the
 * mapping to reflect the new buffer location of the string.
 * (This occurs when a macro argument contains a macro call
 * with arguments; the nested call's argument tokens are moved
 * from the top-level argument to the nested argument.)
 */

extern void new_macro_arg_map(a_source_position_ptr, const char* loc,
			      const char* target_loc);

/* The following function is called (from expand_top_level_pcc_macro)
 * to move macro arg maps from the source line modifications in which
 * they currently reside to the list currently being constructed (so
 * they can be appropriately adjusted when the original buffer is
 * overwritten by the macro-expanded version).
 */

extern void move_curr_token_macro_arg_map(const char* new_token_loc);

/* The following function is called when a macro argument has
 * been completely scanned to terminate the list of macro_arg_maps
 * for that argument; the head of the list is returned (so it can
 * be recorded in the macro_argument structure) and a new empty
 * list is started.
 */

extern a_macro_arg_map_ptr end_macro_arg_map_list(void);

/* The following function is called to copy and remap the
 * macro_arg_maps one list to another.  The result is built up in
 * the same fashion as for new_macro_arg_map and are accessed via
 * end_macro_arg_map_list().
 */

extern void clone_macro_arg_map_list(a_macro_arg_map_ptr list,
				     const char* old_base,
				     const char* new_base);

/* The preceding function is used when the source line modification
 * that will hold the new list is not yet available.  When the cloning
 * is from an existing source line modification (as opposed to a
 * macro argument) and the new source line modification has already
 * been created, the following function can be used to effect the copy
 * without disturbing the global list that may be in the process of
 * accumulation.
 */

extern void copy_source_modif_macro_arg_maps(a_source_line_modif_ptr old_slmp,
					     a_source_line_modif_ptr new_slmp);

/* The following function is called to free a macro_arg_map list
 * once it is no longer needed.
 */

extern void free_macro_arg_map_list(a_macro_arg_map_ptr);

/* The following function is called when a buffer is reallocated to
 * adjust the targeted positions from the old buffer to the new one.
 */

extern void adjust_macro_arg_maps_after_realloc(a_macro_arg_map_ptr list,
						const char* old_base,
						const char* old_after_end,
						const char* new_base);

/* The following function does the same as the preceding, except for
 * the global mapping pointer instead of the macro arg maps.
 */

extern void adjust_SET_mapping_ptr(const char* old_base,
				   const char* old_after_end,
				   const char* new_base);

/* The following functions are used when a macro replacement is
 * inserted somewhere other than the beginning of the macro
 * name (e.g., because the macro name is itself the result of
 * the expansion of a macro); this allows the mapping variables
 * to remain consistent.
 */

extern void register_start_of_macro(const char* macro_start);
extern void move_base_of_offset(const char* start_of_deletion);

/* The following functions are called while macro invocations are
 * being scanned to identify the arguments and processed arguments
 * used in the call.
 */

extern void register_macro_call(const char* name, sizeof_t name_len,
				a_macro_param_ptr params,
				a_source_position_ptr start_pos,
				const char* start_loc,
				a_boolean is_func_style_macro);

extern void register_macro_raw_arg(a_source_position_ptr start_pos,
				   a_source_position_ptr end_pos,
				   const char* orig_source_ptr,
				   const char* text, sizeof_t text_len);

extern void reset_macro_arg_locations(void);

extern void register_macro_processed_arg(const char* text, sizeof_t
					 text_len);

extern void register_unchanged_macro_arg(void);

extern void register_macro_arg_separator(const char* loc);

extern void finish_macro_call(a_source_position_ptr end_pos,
			      const char* end_loc);

extern a_boolean adjust_token_location(a_source_position_ptr pos,
				       const char* loc, size_t len);

/* The following flag is used to identify the case when skip_white_space
 * is being called just to look for the "(" in a macro call; in cases
 * where the identifier preceding the whitespace is the last token of
 * a source line modification, we need special handling (since the
 * token may or may not be rescanned, depending on whether the "(" is
 * found or not).
 */

extern a_boolean scanning_for_macro_paren;

/* The following flag is used to offset the fact that macro_invocation
 * is used recursively in pcc mode in a way that is not done in ANSI
 * mode preprocessing (i.e., the value of macro_depth is different),
 * since the calculation of the structure of the preprocessor AST is
 * based on the ANSI pattern.
 */

extern a_boolean SET_expanding_top_level_pcc_macro;

/* Write a preprocessing AST to the given output buffer. */
extern void write_pp_AST_info(struct pp_AST_node_info *pp, string_buffer_ptr);

extern void write_syms_for_undefined_macros(void);

/* Write the preprocessor IF for a file. */
extern void write_pp_IF_for(a_source_file_ptr);

/* Write an SMT file line to the given output buffer. */
extern void write_smt_file(const char* name, string_buffer_ptr output_buffer);

/* Dump memory usage analysis, preprocessing portion. */
extern void dump_pp_memory_meters(void);
#endif /* SET_preprocess_h */
