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
#include "macro.h"
#include "SET_preprocess.h"
#include "SET_ast.h"
#include "SET_symid.h"
#include "SET_additions.h"
#include "SET_complaints.h"
#include <stddef.h>
#include "SET_symbol.h"
#include "SET_dump_if.h"

/* The following variables are used for metering memory usage by the
 * functions in this file.
 */

static size_t macro_cache_meter;
static size_t pp_event_meter;
static size_t pp_AST_stack_meter;
static size_t pp_AST_node_meter;
static size_t file_lines_meter;
static size_t macro_call_meter;

/* The following function copies a supplied text argument into the
 * specified buffer, skipping over lexical escapes and walking
 * through source modifications.
 */

static void add_macro_text_to_string(const char* start,
				     const char* end,
				     string_buffer_ref bp) {
    const char* p;
    for (p = start; p < end; p++) {
	if (*p == LE_ESCAPE) {
	    p++;	/* just ignore */
	}
	else if (*p == ATTENTION_MARKER) {
	    a_source_line_modif_ptr slmp =
		    nested_source_line_modif((char*) p);
	    add_macro_text_to_string(slmp->inserted_text,
				     slmp->end_inserted_text,
				     bp);
	    p += slmp->num_chars_to_delete - 1;
	}
	else add_1_char_to_string(*p, bp);
    }
}

/* The following struct represents a single macro, either defined or
 * referenced in the expression of a #if or in a #ifdef/#ifndef.  The
 * symid is assigned when the first reference is dumped, not when it is
 * defined (which might never happen, or might happen after the
 * reference).  If the macro is #undefed, the undef_location is set
 * and subsequent lookups will ignore it, causing a new macro entry to
 * be inserted if the name and parameters are ever referenced or
 * defined later.
 */

typedef struct a_macro_cache_entry {
    a_macro_cache_entry_ptr next_in_bucket;
    unsigned long symid;
    const char* name;
    a_source_position def_pos;
    a_source_file_ptr def_file;
    a_source_position undef_pos;
    unsigned char *outbuf_bit_arr;
} a_macro_cache_entry;

/* The following define the macro cache hash table */

#define NUM_MACRO_CACHE_BUCKETS 17293

static a_macro_cache_entry_ptr macro_cache[NUM_MACRO_CACHE_BUCKETS];

/* The following variables are used to allocate blocks of
 * macro_cache_entry objects to avoid the overhead of allocating them
 * individually.
 */

static a_macro_cache_entry_ptr macro_cache_block;
static size_t next_free_macro_cache_entry;
#define MACRO_CACHE_ENTRY_BLOCK_COUNT 2000

/* The following function creates a new macro cache entry,
 * initializing its fields to good values.
 */

static a_macro_cache_entry_ptr new_macro_cache_entry(const char* name,
						     size_t bucket) {
    a_macro_cache_entry_ptr p;
    if (!macro_cache_block ||
	next_free_macro_cache_entry >= MACRO_CACHE_ENTRY_BLOCK_COUNT) {
	macro_cache_block = (a_macro_cache_entry_ptr)
		malloc(MACRO_CACHE_ENTRY_BLOCK_COUNT *
		       sizeof(a_macro_cache_entry));
	macro_cache_meter += MACRO_CACHE_ENTRY_BLOCK_COUNT *
		sizeof(a_macro_cache_entry);
	next_free_macro_cache_entry = 0;
    }
    p = macro_cache_block + next_free_macro_cache_entry++;
    p->next_in_bucket = macro_cache[bucket];
    macro_cache[bucket] = p;
    p->symid = 0;
    add_to_string(name, &string_buf);
    p->name = terminate_string(&string_buf);
    p->def_pos.seq = 0;		/* not #defined */
    p->def_file = NULL;
    p->undef_pos.seq = 0;	/* not #undefed */
    p->outbuf_bit_arr = NULL;
    return p;
}

/* The following function calculates the hash value for a given name,
 * i.e., the bucket into which the name will be inserted.
 */

static size_t macro_cache_hash(const char* name) {
    unsigned int accumulator = 0;
    while (name && *name) {
	accumulator *= 3;
	accumulator += *name++;
    }
    return accumulator % NUM_MACRO_CACHE_BUCKETS;
}

/* The following function either finds an existing macro cache entry
 * or creates a new one.  Existing macros with the same name that have
 * been #undefed are ignored.
 */

static a_macro_cache_entry_ptr get_macro_cache_entry(const char* name,
						     sizeof_t name_len,
						     a_macro_param_ptr params) {
    const char* canonical_name;
    a_macro_cache_entry_ptr cache_entry;
    size_t bucket;
    a_boolean first_param = TRUE;

    add_to_string_with_len(name, name_len, &trial_buf);
    params = NULL;	/* for now, at least, params are not part of
			 * the name; this may change, so I'm leaving
			 * the option to add them easily.
			 */
    while (params) {
	if (first_param) {
	    first_param = FALSE;
	    add_1_char_to_string('(', &trial_buf);
	}
	add_to_string(params->name, &trial_buf);
	if (params->next) {
	    add_1_char_to_string(',', &trial_buf);
	}
	else add_1_char_to_string(')', &trial_buf);
	params = params->next;
    }
    canonical_name = terminate_string(&trial_buf);

    bucket = macro_cache_hash(canonical_name);
    for (cache_entry = macro_cache[bucket]; cache_entry;
	 cache_entry = cache_entry->next_in_bucket) {
	if (strcmp(cache_entry->name, canonical_name) == 0 &&
	    cache_entry->undef_pos.seq == 0) {
	    break;
	}
    }

    if (!cache_entry) {
	cache_entry = new_macro_cache_entry(canonical_name, bucket);
    }

    return cache_entry;
}

static void write_macro_sym_line(a_macro_cache_entry_ptr p, 
				 string_buffer_ptr output_buffer)
{
  add_3_chars_to_string("SYM", output_buffer);
  add_symid_to_string(p->symid, output_buffer);
  add_to_string("macro ", output_buffer);
  add_quoted_str_to_string(p->name, /*add_quotes=*/TRUE,
			   output_buffer);
  terminate_string(output_buffer);
}

static void write_sym_for_macro_reference(a_macro_cache_entry_ptr macro, 
					  string_buffer_ptr output_buffer)
{
  if (!is_output_buffer_bit_set(&macro->outbuf_bit_arr, output_buffer)) { 

    write_macro_sym_line(macro, output_buffer);
    /* If macro has definition file, write macro SYM line and definition 
       file here. */
    if (macro->def_file && (get_buffer_of_source_file(macro->def_file) != 
	                    output_buffer)) {
      write_sym_line_for_source_file(macro->def_file, output_buffer);
      write_rel(macro->def_file->symid, "define", macro->symid, output_buffer);
    }

    set_output_buffer_bit(&macro->outbuf_bit_arr, output_buffer);
  }
}

/* The following function is called after all the preprocessor events
 * have been processed.  Since the SYM lines for macros are output
 * when the #define is seen (to put the SYM line in the correct file),
 * macros that were referenced but not defined cause references to
 * symids without SYM lines; this function loops through the entire
 * cache and writes SYM lines for undefined macros.
 */

void write_syms_for_undefined_macros(void) {
    a_boolean file_line_done = FALSE;
    size_t i;
    a_macro_cache_entry_ptr p;
    for (i = 0; i < NUM_MACRO_CACHE_BUCKETS; i++) {
	for (p = macro_cache[i]; p; p = p->next_in_bucket) {
	    if (p->symid && p->def_pos.seq == 0) {
		/* macro was referenced but not defined, must do the
		 * SYM line here to avoid undefined references.
		 */
		  if (!file_line_done) {
		    /* We identify the file as "" to avoid associating
		     * the SYM with the wrong file.
		     */
		    add_to_string("SMT file \"\"", &output_buf);
		    terminate_string(&output_buf);
		    file_line_done = TRUE;
		  }
		  write_macro_sym_line(p, &output_buf);
	    }
	}
    }
}

/* The following enumeration lists the different kinds of events that
 * can be registered in the preprocessor event log.
 */

typedef enum pp_event_kind {
    PP_MAP_FIXED,
    PP_MAP_COPY,
    PP_MACRO_REF,
    PP_NESTED_MACRO,
    PP_ARG_USE
} pp_event_kind;

/* The following structure represents a single preprocessor event. */

typedef struct pp_event* pp_event_ptr;

typedef struct pp_event {
    pp_event_kind kind;
    a_source_position pos;
    union {
	/* for MAP events: */
	a_source_position end;
	/* for macro references: */
	a_macro_cache_entry_ptr cache_entry;
	/* for nested macros: */
	struct nested_macro {
	    a_macro_cache_entry_ptr caller;
	    a_macro_cache_entry_ptr callee;
	} nested_macro;
	/* for use of macro arguments in expansions: */
	a_source_position target;
    } variant;
} pp_event;

/* As usual, to avoid allocation overhead memory is allocated in a
 * block and parceled out one event at a time.
 */

#define PP_EVENT_BLOCK_SIZE 2000

typedef struct pp_event_block* pp_event_block_ptr;

typedef struct pp_event_block {
    pp_event_block_ptr next_block;
    size_t next_available;
    pp_event event[PP_EVENT_BLOCK_SIZE];
} pp_event_block;
    
static pp_event_block_ptr first_pp_event_block;
static pp_event_block_ptr last_pp_event_block;

/* The following function returns a pointer to the next unused
 * preprocessor event log entry, handling all the allocation and
 * linking required.
 */

static pp_event_ptr new_pp_event(void) {
    if (!last_pp_event_block ||
	last_pp_event_block->next_available >= PP_EVENT_BLOCK_SIZE) {
	pp_event_block_ptr new_block = (pp_event_block_ptr)
		malloc(sizeof(pp_event_block));
	pp_event_meter += sizeof(pp_event_block);
	new_block->next_block = NULL;
	new_block->next_available = 0;
	if (last_pp_event_block) {
	    last_pp_event_block->next_block = new_block;
	}
	else first_pp_event_block = new_block;
	last_pp_event_block = new_block;
    }
    return &last_pp_event_block->
	    event[last_pp_event_block->next_available++];
}

/* The following variables and function form an iterator through the
 * preprocessor events.
 */

static pp_event_block_ptr cur_pp_event_block;
static size_t cur_pp_event_within_block;

static pp_event_ptr next_pp_event_in_range(a_seq_number first_seq,
					   a_seq_number last_seq) {
    for (;;) {
	pp_event_ptr ppe;
	if (!cur_pp_event_block) {
	    cur_pp_event_block = first_pp_event_block;
	    cur_pp_event_within_block = 0;
	}
	while (cur_pp_event_within_block >=
	       cur_pp_event_block->next_available) {
	    if (!cur_pp_event_block->next_block) {
		return NULL;	/* no more events */
	    }
	    cur_pp_event_block = cur_pp_event_block->next_block;
	    cur_pp_event_within_block = 0;
	}
	ppe = &cur_pp_event_block->event[cur_pp_event_within_block];
	if (ppe->pos.seq > last_seq) {
	    /* none in range */
	    return NULL;
	}
	cur_pp_event_within_block++;
	if (ppe->pos.seq < first_seq) {
	    /* print error and stay in loop for next event */
	    complain(error_csev, "Missed preprocessor event.");
	}
	else return ppe;
    }
}

/* The following struct defines a preprocessor AST entry.  Because
 * preprocessing is linear, no child/sibling links are necessary; the
 * nodes are sequential, with the tree structure indicated by the
 * "depth" field.
 */

typedef struct pp_AST_node* pp_AST_node_ptr;

typedef struct pp_AST_node {
    a_source_position start;
    a_source_position end;
    const char* name;
    unsigned long symid;
    a_macro_cache_entry_ptr cache_entry;
    const char* operand;
    pp_AST_node_ptr next;
    size_t depth;
    a_bit_field add_dummy_file_name:1;
} pp_AST_node;

/* The following structure represents a level of #if nesting.
 * "if_node" points to the pp_AST_node corresponding to the #if,
 * allowing it to be updated with the location of the #endif once it
 * is known.  "cur_block" points to the pp_AST_node representing the
 * block of text currently being processed, either the (imaginary)
 * "then" clause or a #elif or #else clause, again so that the ending
 * location can be inserted.  The "nesting_level" is relative to the
 * entire translation unit, not to the current file; since the AST
 * represents code inside an #if/#endif pair with two levels of
 * nesting (one for the #if, one for the then/#elif/#else), the
 * "nesting_level" is twice the stack depth.
 */

typedef struct pp_AST_stack_entry* pp_AST_stack_entry_ptr;

typedef struct pp_AST_stack_entry {
    pp_AST_stack_entry_ptr next;
    pp_AST_node_ptr if_node;
    pp_AST_node_ptr cur_block;
} pp_AST_stack_entry;

static pp_AST_stack_entry_ptr cur_pp_AST_stack_entry;
static pp_AST_stack_entry_ptr next_free_pp_AST_stack_entry;

/* The following function pushes a level of #if nesting onto the
 * preprocessor AST stack.
 */

static void push_pp_AST_stack_entry(pp_AST_node_ptr if_node) {
    pp_AST_stack_entry_ptr new_level;
    if (next_free_pp_AST_stack_entry) {
	new_level = next_free_pp_AST_stack_entry;
	next_free_pp_AST_stack_entry = new_level->next;
    }
    else {
	new_level = (pp_AST_stack_entry_ptr)
	    malloc(sizeof(pp_AST_stack_entry));
	pp_AST_stack_meter += sizeof(pp_AST_stack_entry);
    }
    new_level->next = cur_pp_AST_stack_entry;
    cur_pp_AST_stack_entry = new_level;
    new_level->if_node = if_node;
    new_level->cur_block = if_node;	/* until "then" is allocated */
}

/* The following function pops a level of #if nesting from the
 * preprocessor AST stack.
 */

static void pop_pp_AST_stack_entry(void) {
    pp_AST_stack_entry_ptr old_level = cur_pp_AST_stack_entry;
    if (old_level) {
	cur_pp_AST_stack_entry = old_level->next;
	old_level->next = next_free_pp_AST_stack_entry;
	next_free_pp_AST_stack_entry = old_level;
    }
    else complain(error_csev, "Popped more levels of #if than exist.");
}

/* The usual allocation strategy for pp_AST_nodes; the new node is
 * initialized with the supplied values and placed at the current
 * nesting level from the stack.
 */

static pp_AST_node_ptr pp_AST_node_block;
static size_t next_free_pp_AST_node;
#define PP_AST_NODE_BLOCK_COUNT 2000
static int pp_AST_depth = 0;

static pp_AST_node_ptr new_pp_AST_node(a_source_position_ptr start,
				       a_source_position_ptr end,
				       const char* name,
				       unsigned long symid,
				       a_macro_cache_entry_ptr cache_entry,
				       const char* operand) {
    pp_AST_node_ptr np;
    pp_AST_depth += 1;
    if (curr_ise) {
	/* only allocate if have a place to link it */
	a_source_file_ptr il_file;
	if (!pp_AST_node_block ||
	    next_free_pp_AST_node >= PP_AST_NODE_BLOCK_COUNT) {
	    pp_AST_node_block = (pp_AST_node_ptr)
		    malloc(PP_AST_NODE_BLOCK_COUNT *
			   sizeof(pp_AST_node));
	    pp_AST_node_meter += PP_AST_NODE_BLOCK_COUNT *
		    sizeof(pp_AST_node);
	    next_free_pp_AST_node = 0;
	}
	np = pp_AST_node_block + next_free_pp_AST_node++;
	np->start = *start;
	np->end = *end;
	np->name = name;
	np->symid = symid;
	np->cache_entry = cache_entry;
	np->operand = operand;
	np->next = NULL;
	np->add_dummy_file_name = FALSE;
	np->depth = pp_AST_depth;
	if (curr_ise->assoc_actual_il_file) {
	    il_file = curr_ise->assoc_actual_il_file;
	}
	else il_file = curr_ise->assoc_il_file;
	if (il_file) {
	    if (il_file->last_pp_AST_node) {
		il_file->last_pp_AST_node->next = np;
	    }
	    else il_file->first_pp_AST_node = np;
	    il_file->last_pp_AST_node = np;
	}
	else complain(error_csev, "No IL file for new pp AST node.");
	return np;
    }
    else {
	return NULL;
    }
}

static void pop_pp_AST_node() {
    pp_AST_depth -= 1;
}


/* The following function writes an "SMT file" line to the IF file. */

void write_smt_file(const char* name, string_buffer_ptr output_buffer) {
    if (output_buffer != NULL) {
	add_to_string("SMT file ", output_buffer);
	add_quoted_str_to_string(name, TRUE /*add quotes*/, output_buffer);
	terminate_string(output_buffer);
    }
}

/* The following struct encapsulates the mapping from sequence numbers
 * to line numbers for a segment of a given file.  Each invocation of
 * write_pp_IF_for keeps a local list of these objects to enable it to
 * translate the line references in that file's preprocessor AST.
 */

typedef struct file_lines* file_lines_ptr;

typedef struct file_lines {
    file_lines_ptr next;
    a_seq_number start_seq;
    a_seq_number last_seq;
    unsigned long start_lineno;
} file_lines;

static file_lines_ptr free_file_lines_list;

/* The following function allocates a file_lines object, initializes
 * its fields, and links it at the end of the list designated by the
 * supplied head and tail pointers.
 */

static void new_file_lines(file_lines_ptr* head, file_lines_ptr* tail,
			   a_seq_number start_seq,
			   a_seq_number last_seq,
			   unsigned long start_lineno) {
    file_lines_ptr flp;
    if (free_file_lines_list) {
	flp = free_file_lines_list;
	free_file_lines_list = flp->next;
    }
    else {
	flp = (file_lines_ptr) malloc(sizeof(file_lines));
	file_lines_meter += sizeof(file_lines);
    }
    if (*tail) {
	(*tail)->next = flp;
    }
    else *head = flp;
    *tail = flp;
    flp->next = NULL;
    flp->start_seq = start_seq;
    flp->last_seq = last_seq;
    flp->start_lineno = start_lineno;
}

/* The following function frees a chain of file_lines objects by
 * linking them to the free_file_lines list.  Because it is being
 * called at the end of the execution of write_pp_IF_for, which keeps
 * the head and tail pointers in local auto variables, the values of
 * the head and tail pointers do not need to be reset to NULL.
 */

static void free_file_lines(file_lines_ptr head, file_lines_ptr tail) {
    if (tail) {
	tail->next = free_file_lines_list;
	free_file_lines_list = head;
    }
}

/* The following function maps a position to its file-relative
 * equivalent, using the supplied file_lines chain, and adds the
 * mapped position to the specified string buffer.
 */

static void map_pos_to_string(a_source_position_ptr pos,
			      file_lines_ptr head,
			      file_lines_ptr tail,
			      string_buffer_ptr bp) {
    if (pos->seq < head->start_seq) {
	complain_ulong_ulong(error_csev, "Map sequence before start: $1, $2",
		pos->seq, head->start_seq);
    }
    else {
	while (head && head->last_seq < pos->seq) {
	    head = head->next;
	}
	if (!head) {
	    complain_ulong_ulong(error_csev, "Map sequence after end: $1, $2",
		    pos->seq, tail->last_seq);
	}
	else if (pos->seq < head->start_seq - 1) {
	    /* "-1" because sometimes we decrement a seq number (e.g.,
	     * to indicate the end of the "then" clause when we see a
	     * #else), and if the preceding line was a #include, the
	     * resulting seq number will be in the included file, not
	     * in the mapped range.  This case is handled by
	     * add_file_rel_pos_to_string.  It's only legitimate for
	     * an offset of 1 line, however, so we still want to complain
	     * if the delta is anything more than that.
	     */
	    complain_ulong_ulong_ulong(error_csev, "Map sequence between chunks: $1, $2-$3",
		    pos->seq, head->start_seq, head->last_seq);
	}
	else add_file_rel_pos_to_string(pos, head->start_seq,
					head->start_lineno, bp);
    }
}

/* The following function writes either a "MAP fixed" or "MAP copy"
 * line to the IF file.
 */

static void write_map_fixed_or_copy(const char* fixed_or_copy,
				    const char* filename,
				    pp_event_ptr ppe,
				    a_seq_number start_seq,
				    unsigned long start_lineno,
				    string_buffer_ref output_buffer) {
    if (*fixed_or_copy == 'c' && ppe->variant.end.seq == 1 &&
	ppe->variant.end.mapped_column == 0 && ppe->variant.end.column != 0) {
	/* This is the obscure case in which a macro was defined on
	 * the command line with empty replacement text and that
	 * macro occurs in the first character position of the
	 * file; in such a case, the "MAP copy" covering all the
	 * text before the first macro would have a target position
	 * of 0/0, which would cause the IF mapper heartburn.
	 * Suppress it.
	 */
	return;
    }

    add_to_string_with_len("MAP ", 4, output_buffer);
    add_to_string(fixed_or_copy, output_buffer);
    add_1_char_to_string(' ', output_buffer);
    add_quoted_str_to_string(filename, TRUE /*add quotes*/, output_buffer);
    add_2_chars_to_string(" {", output_buffer);

    /* The first two positions are file-relative. */

    add_file_rel_pos_to_string(&ppe->pos, start_seq, start_lineno,
			       output_buffer);
    add_file_rel_pos_to_string(&ppe->variant.end, start_seq,
			       start_lineno, output_buffer);
    add_1_char_to_string(',', output_buffer);

    /* The second two positions are relative to the phantom
     * cpp output file.
     */

    add_pos_to_string(&ppe->pos, output_buffer);
    add_pos_to_string(&ppe->variant.end, output_buffer);
    add_1_char_to_string('}', output_buffer);
    terminate_string(output_buffer);
}

/* The following function writes a MAP copy line to the IF file
 * describing a use of a macro argument in the expansion.  As such, it
 * indicates two different positions, unlike the preceding MAPs.
 */

static void write_map_copy_for_arg(const char* filename,
				   pp_event_ptr ppe,
				   a_seq_number start_seq,
				   unsigned long start_lineno,
				   string_buffer_ref output_buffer) {
    add_to_string("MAP copy ", output_buffer);
    add_quoted_str_to_string(filename, TRUE /*add quotes*/, output_buffer);
    add_2_chars_to_string(" {", output_buffer);

    /* The first position is file-relative */

    add_file_rel_pos_to_string(&ppe->pos, start_seq, start_lineno,
			       output_buffer);
    add_num_to_string(ppe->pos.len, NULL, ",", output_buffer);

    /* The second is relative to the phantom cpp output file */

    add_pos_to_string(&ppe->variant.target, output_buffer);
    add_1_char_to_string('}', output_buffer);
    terminate_string(output_buffer);
}

static a_source_position pos_of_suppressed_include;
static unsigned long lineno_of_suppressed_include;

/* The following function assigns a symid and writes a SYM line upon
 * the first reference to a macro.
 */

static void write_sym_for_macro(a_macro_cache_entry_ptr macro,
				a_seq_number start_seq,
				unsigned long start_lineno,
				string_buffer_ref output_buffer) {
    add_3_chars_to_string("SYM", output_buffer);
    add_symid_to_string(macro->symid, output_buffer);
    add_to_string("macro \"", output_buffer);
    add_to_string(macro->name, output_buffer);
    add_1_char_to_string('\"', output_buffer);
    if (macro->def_pos.seq) {
	if (pos_of_suppressed_include.seq) {
	    add_file_rel_pos_and_len_to_string(&pos_of_suppressed_include,
					       pos_of_suppressed_include.seq,
					       lineno_of_suppressed_include,
					       output_buffer);
	}
	else add_file_rel_pos_and_len_to_string(&macro->def_pos,
						start_seq,
						start_lineno,
						output_buffer);
        if (generate_multiple_IF_files) {
            set_output_buffer_bit(&macro->outbuf_bit_arr, output_buffer);
        }
    }
    terminate_string(output_buffer);
}

/* The following function writes a "MAP fixed" line for a header
 * file that is being suppressed because it has a use of a
 * macro-parameterized simulated template.  (Note: we can only
 * guess at the beginning location of the "#include" directive,
 * the column is assumed to be 1 but could be otherwise.)
 */

static void write_map_fixed_for_include(const char* including_filename,
					a_seq_number include_lineno,
					a_seq_number first_seq_in_included_file,
					a_seq_number last_seq_in_included_file,
					string_buffer_ref output_buffer) {
    add_to_string("MAP fixed ", output_buffer);
    add_quoted_str_to_string(including_filename, TRUE /*add quotes*/, output_buffer);
    add_num_to_string(include_lineno, " { ", "/1 ", output_buffer);
    add_num_to_string(include_lineno, NULL, "/0, ", output_buffer);
    add_num_to_string(first_seq_in_included_file, NULL, "/1 ", output_buffer);
    add_num_to_string(last_seq_in_included_file, NULL, "/0 }", output_buffer);
    terminate_string(output_buffer);
    pos_of_suppressed_include.seq = first_seq_in_included_file - 1;
    pos_of_suppressed_include.column = 1;
    pos_of_suppressed_include.mapped_column = 1;
    lineno_of_suppressed_include = include_lineno;
}

/* The following function traverses an include tree, calling the
 * preceding function for the suppressed portions.
 */

static void map_suppressed_include(a_source_file_ptr parent,
				   a_source_file_ptr f,
				   a_line_number include_lineno,
				   a_seq_number* start_seq,
				   a_boolean write_on_eof) {
    a_source_file_ptr child;
    a_source_file_ptr grandchild;
    string_buffer_ptr output_buffer = get_buffer_of_source_file(parent);
    if (output_buffer != NULL) {
	for (child = f->first_child_file; child; child = child->next) {
	    if (child->full_name) {
		if (child->has_template_macro_use) {
		    /* descend into child looking for a non-suppressed
		     * file.
		     */
		    map_suppressed_include(parent, child, include_lineno,
					   start_seq, FALSE);
		}
		else {
		    if (!generate_multiple_IF_files) {
			write_map_fixed_for_include(absolute_path_of(parent->full_name),
						    include_lineno,
						    *start_seq,
						    child->first_seq_number - 1,
						    output_buffer);
		    }
		    *start_seq = child->last_seq_number + 1;
		}
	    }
	    else for (grandchild = child->first_child_file; grandchild;
		      grandchild = grandchild->next) {
		if (grandchild->has_template_macro_use) {
		    map_suppressed_include(parent, grandchild, include_lineno,
					   start_seq, FALSE);
		}
		else {
		    if (!generate_multiple_IF_files) {
			write_map_fixed_for_include(absolute_path_of(parent->full_name),
						    include_lineno,
						    *start_seq,
						    grandchild->first_seq_number - 1,
						    output_buffer);
		    }
		    *start_seq = grandchild->last_seq_number + 1;
		}
	    }
	}
	if (!generate_multiple_IF_files && 
	    write_on_eof && *start_seq <= f->last_seq_number) {
	    write_map_fixed_for_include(absolute_path_of(parent->full_name),
					include_lineno, *start_seq,
					f->last_seq_number,
					output_buffer);
	}
    }
}

/* The following function writes a single MAP line to the IF file,
 * saves the mapping information in a new file_lines object, and
 * writes the preprocessor event log information up through the
 * line indicated by "last_seq."
 */

static void write_map_line_and_pp_event_log(a_source_file_ptr f,
					    a_seq_number start_seq,
					    a_seq_number last_seq,
					    unsigned long start_lineno,
					    file_lines_ptr* head,
					    file_lines_ptr* tail) {
    pp_event_ptr ppe;
    a_boolean suppress_MAPs = suppress_template_macro_files &&
	    f->has_template_macro_use;
    const char* absolute_path = absolute_path_of(f->full_name);
    string_buffer_ptr output_buffer = get_buffer_of_source_file(f);

    if (!generate_multiple_IF_files && !suppress_MAPs && output_buffer != NULL) {
	add_to_string("MAP lines ", output_buffer);
	add_quoted_str_to_string(absolute_path, /*add_quotes=*/TRUE,
				 output_buffer);
	add_num_to_string(start_lineno, " {", " ",output_buffer);
	add_num_to_string(start_lineno + last_seq - start_seq,
			  NULL, ", ", output_buffer);
	add_num_to_string(start_seq, NULL, "}", output_buffer);
	terminate_string(output_buffer);
    }

    new_file_lines(head, tail, start_seq, last_seq, start_lineno);

    if (output_buffer == NULL) {
	return;
    }

    while (ppe = next_pp_event_in_range(start_seq, last_seq)) {
	switch (ppe->kind) {
	case PP_MAP_FIXED:
	    if (!generate_multiple_IF_files && !suppress_MAPs) {
		write_map_fixed_or_copy("fixed", absolute_path, ppe,
					start_seq, start_lineno, output_buffer);
	    }
	    break;

	case PP_MAP_COPY:
	    if (!generate_multiple_IF_files && !suppress_MAPs) {
		write_map_fixed_or_copy("copy", absolute_path, ppe,
					start_seq, start_lineno, output_buffer);
	    }
	    break;

	case PP_MACRO_REF: {
	    a_macro_cache_entry_ptr macro = ppe->variant.cache_entry;
	    if (!macro->symid) {
		/* first reference -- assign a symid */
		macro->symid = next_symid++;
	    }
	    if (ppe->pos.seq == macro->def_pos.seq) {
		/* this is the definition -- write the SYM line and "define" REL */
		write_sym_for_macro(macro, start_seq, start_lineno, output_buffer);
		write_rel(f->symid, "define", macro->symid, output_buffer);
	    }
	    else if (generate_multiple_IF_files) {
	        write_sym_for_macro_reference(macro, output_buffer);
	    }
	    if (!suppress_MAPs) {
		add_3_chars_to_string("SMT", output_buffer);
		add_symid_to_string(macro->symid, output_buffer);
		add_file_rel_pos_and_len_to_string(&ppe->pos, start_seq,
					   start_lineno, output_buffer);
		terminate_string(output_buffer);
	    }
	}
	    break;

	case PP_NESTED_MACRO:
	    if (!ppe->variant.nested_macro.caller->symid) {
		ppe->variant.nested_macro.caller->symid = next_symid++;
	    }
	    if (!ppe->variant.nested_macro.callee->symid) {
		ppe->variant.nested_macro.callee->symid = next_symid++;
	    }
	    if (generate_multiple_IF_files) {
	      /* write SYM line before REL line */
	      write_sym_for_macro_reference(ppe->variant.nested_macro.caller,
					    output_buffer);
	      write_sym_for_macro_reference(ppe->variant.nested_macro.callee,
					    output_buffer);
	    }
	    write_rel(ppe->variant.nested_macro.caller->symid,
		      "expand",
		      ppe->variant.nested_macro.callee->symid,
		      output_buffer);
	    break;

	case PP_ARG_USE:
	    if (!generate_multiple_IF_files && !suppress_MAPs) {
		write_map_copy_for_arg(absolute_path, ppe, start_seq,
				       start_lineno, output_buffer);
	    }
	    break;

	default:
	    complain_int(error_csev, "Unknown preprocessor event $1",
		    ppe->kind);
	    break;
	}
    }
}


typedef struct pp_AST_node_info* pp_AST_node_info_ptr;

typedef struct pp_AST_node_info {
  pp_AST_node_ptr first_pp_AST_node;
  file_lines_ptr head;
  file_lines_ptr tail;
} pp_AST_node_info;


static void save_pp_AST_node_info_for(a_source_file_ptr f, file_lines_ptr head,
				      file_lines_ptr tail)
{
  if (!get_pp_ast_of_source_file(f)) {
    pp_AST_node_info_ptr pp = (pp_AST_node_info_ptr)malloc(sizeof(pp_AST_node_info));
    pp->first_pp_AST_node = f->first_pp_AST_node;
    pp->head = head;
    pp->tail = tail;
    
    set_pp_ast_of_source_file(f, pp);
  }
}

/* The following function writes the preprocessor AST.
 */

static void write_pp_AST(pp_AST_node_ptr first_pp_node, file_lines_ptr head,
			 file_lines_ptr tail, string_buffer_ref output_buffer)
{
  pp_AST_node_ptr node = first_pp_node;
  int nesting_offset = node->depth - 1; /* for AST of a file nested in #if */
  while (node) {
    int nestedness;
    if (indent_AST) {
      indent_to_depth(node->depth - nesting_offset, output_buffer);
    }
    add_to_string(node->name, output_buffer);
    if (node->symid) {
      add_symid_to_string(node->symid, output_buffer);
    }
    else if (node->cache_entry) {
      add_symid_to_string(node->cache_entry->symid, output_buffer);
    }
    if (node->start.seq) {
      if (node->add_dummy_file_name) {
	a_boolean good_loc;
        char *saved_output_filename;
	const char *loc_str;
	/* This node is part of the expression in a #if directive. */
	if (!generate_multiple_IF_files) {
	  /* Since these use the expanded version,
	   * positions must be reported relative to the
	   * "output" file and not mapped back to the input.
	   * This situation is flagged to the IF parser by
	   * the presence of "" as the node's file name.
	   */
	  add_3_chars_to_string(" \"\"", output_buffer);
	}
	else {
	  /* While generate_multiple_IF_files,
	   * positions are reported in the input file.
	   * We need add_pos_to_string to get the
	   * precise column.
	   */
	}
	/* Test whether the locations are good,
	 * making trial_buf look like a multiple-IF output buffer
	 * for the sake of checking that the output file corresponds to
	 * the operation's location.
	 */
	good_loc = add_pos_to_string_checking(&node->start, &trial_buf, output_buffer)
		   && (node->end.seq == 0
                       || add_pos_to_string_checking(&node->end, &trial_buf, output_buffer));
	loc_str = terminate_string(&trial_buf);
	if (good_loc) {
	  add_to_string(loc_str, output_buffer);
	}
	else {
          add_1_char_to_string(' ', output_buffer);
	}
      }
      else {
	map_pos_to_string(&node->start, head, tail, output_buffer);
	if (node->end.seq) {
	  map_pos_to_string(&node->end, head, tail, output_buffer);
	}
      }
    }
    if (node->next && (nestedness = node->next->depth - node->depth) > 0) {
      /* Check for skipped level. */
      if (nestedness > 1) {
	complain_pos(error_csev, "Error in nesting of preprocessing AST ($1: $2/$3).",
		     &node->start);
      }
      /* just do open brace */
      add_1_char_to_string('{', output_buffer);
      terminate_string(output_buffer);
    }
    else {
      /* self-contained -- either has string operand or no
       * operands.  We may need to provide some closing
       * braces if the next node is less deeply nested or
       * if this is the last node in the AST.
       */
      int close_braces = node->depth
			 - (node->next ? node->next->depth : nesting_offset);
      int i;
      if (node->operand) {
	/* string operand */
	add_1_char_to_string('{', output_buffer);
	terminate_string(output_buffer);
	if (indent_AST) {
	  indent_to_depth(node->depth - nesting_offset + 1,
			  output_buffer);
	}
	add_quoted_str_to_string(node->operand,
				 /*add_quotes=*/TRUE,
				 output_buffer);
	terminate_string(output_buffer);
	if (indent_AST) {
	  indent_to_depth(node->depth - nesting_offset,
			  output_buffer);
	}
	add_1_char_to_string('}', output_buffer);
	terminate_string(output_buffer);
      }
      else {
	/* no operand */
	add_2_chars_to_string("{}", output_buffer);
	terminate_string(output_buffer);
      }
      for (i = 1; i <= close_braces; i++) {
	if (indent_AST) {
	  indent_to_depth(node->depth - i - nesting_offset,
			  output_buffer);
	}
	add_1_char_to_string('}', output_buffer);
	terminate_string(output_buffer);
      }
    }
    node = node->next;
  }
  free_file_lines(head, tail);
}

void write_pp_AST_info(pp_AST_node_info_ptr pp, string_buffer_ref output_buffer)
{
  if (generate_multiple_IF_files && pp) {
    add_to_string("ast_root {", output_buffer);
    terminate_string(output_buffer);
    write_pp_AST(pp->first_pp_AST_node, pp->head, pp->tail,
		 output_buffer);
  }
}


/* The following function writes the preprocessor IF for a file and,
 * recursively, for all its children.
 */

void write_pp_IF_for(a_source_file_ptr f) {
    unsigned long lines_in_children = 0;
    a_seq_number next_seq = f->first_seq_number;
    a_source_file_ptr child;
    a_source_file_ptr grandchild;
    file_lines_ptr head = NULL;
    file_lines_ptr tail = NULL;
    static const char* filename = "";
    const char* saved_filename = filename;
    a_source_position include_pos_on_entry = pos_of_suppressed_include;
    a_source_position saved_include_pos;
    a_seq_number first_suppressed_seq;
    string_buffer_ptr output_buffer = get_buffer_of_source_file(f);

    /* write "SYM" and "SMT file" lines for this file */

    if (f->is_interesting && output_buffer != NULL) {
        if (generate_multiple_IF_files) {
	    add_SMT_language_to_string(output_buffer);
	}
	if (!(suppress_template_macro_files &&
	      f->has_template_macro_use)) {
	    filename = absolute_path_of(f->full_name);
	    pos_of_suppressed_include = null_source_position;
	    write_smt_file(filename, output_buffer);
	}
	add_3_chars_to_string("SYM", output_buffer);
	add_symid_to_string(f->symid, output_buffer);
	add_to_string_with_len("file ", 5, output_buffer);
	add_quoted_str_to_string(filename, TRUE /*add quotes*/, output_buffer);
	terminate_string(output_buffer);
        if (f->checksum != NULL) {
	    unsigned long checksum_symid = next_symid++;
            char buf[10];
            int i;
	    add_3_chars_to_string("SYM", output_buffer);
	    add_symid_to_string(checksum_symid, output_buffer);
	    add_to_string_with_len("checksum \"", 10, output_buffer);
	    for (i = 0; i < 16; i++) {
		sprintf (buf, "%02x", (*f->checksum)[i]);
		add_2_chars_to_string(buf, output_buffer);
	    }
	    add_1_char_to_string('\"', output_buffer);
	    terminate_string(output_buffer);

	    write_rel(checksum_symid, "checksum", f->symid, output_buffer);
	}
    }

    for (child = f->first_child_file; child; child = child->next) {
	saved_include_pos = pos_of_suppressed_include;
	if (child->full_name) {
	    /* This is an include file.  First write the entry
	     * for the lines of this file up to this point (since
	     * the last child, if any).
	     */
	    if (f->is_interesting) {
		write_map_line_and_pp_event_log(f, next_seq,
					    child->first_seq_number - 1,
					    next_seq - f->first_seq_number -
					    lines_in_children + 1,
					    &head, &tail);
	    }
	    else while(next_pp_event_in_range(next_seq,
					      child->first_seq_number - 1))
		       { }
	    if (suppress_template_macro_files &&
		!f->has_template_macro_use &&
		child->has_template_macro_use) {
		first_suppressed_seq = child->first_seq_number;
		map_suppressed_include(f, child, child->first_seq_number -
				       f->first_seq_number -
				       lines_in_children, &first_suppressed_seq,
				       TRUE);
	    }
	    write_pp_IF_for(child);
	    if (f->is_interesting && output_buffer != NULL) {
	        write_sym_line_for_source_file(child, output_buffer);
		format_rel(f->symid, "include", child->symid, output_buffer);
		if (child->included_by_preinclude) {
		    add_to_string("preinclude", output_buffer);
		}
		terminate_string(output_buffer);
	    }
	    if (f->is_interesting && output_buffer != NULL && !generate_multiple_IF_files) {
		write_smt_file(filename, output_buffer);
	    }
	    lines_in_children += child->last_seq_number -
		    child->first_seq_number + 1;
	    next_seq = child->last_seq_number + 1;
	}
	else {
	    /* This is an entry representing a #line block.  Since
	     * it's physically part of this file, and since its
	     * children represent only include files (a new #line
	     * becomes a sibling, not a child), we process the
	     * "grandchildren" directly here, writing MAP lines
	     * and tallying their contribution to the count of
	     * lines_in_children.
	     */
	    for (grandchild = child->first_child_file; grandchild;
		 grandchild = grandchild->next) {
		if (f->is_interesting) {
		    write_map_line_and_pp_event_log(f, next_seq,
						grandchild->first_seq_number - 1,
						next_seq - f->first_seq_number -
						lines_in_children + 1,
						&head, &tail);
		}
		else while (next_pp_event_in_range(next_seq,
						   grandchild->first_seq_number - 1))
			    { }
		if (suppress_template_macro_files &&
		    !f->has_template_macro_use &&
		    grandchild->has_template_macro_use) {
		    first_suppressed_seq = grandchild->first_seq_number;
		    map_suppressed_include(f, grandchild,
					   grandchild->first_seq_number-
					   f->first_seq_number -
					   lines_in_children,
					   &first_suppressed_seq,
					   TRUE);
		}
		write_pp_IF_for(grandchild);
		if (f->is_interesting && output_buffer != NULL) {
		    write_sym_line_for_source_file(grandchild, output_buffer);
		    write_rel(f->symid, "include", grandchild->symid,
			      output_buffer);
		}
		if (f->is_interesting && output_buffer != NULL && !generate_multiple_IF_files) {
		    write_smt_file(filename, output_buffer);
		}
		lines_in_children += grandchild->last_seq_number -
			grandchild->first_seq_number + 1;
		next_seq = grandchild->last_seq_number + 1;
	    }
	}
	pos_of_suppressed_include = saved_include_pos;
    }
    if (next_seq <= f->last_seq_number) {
	if (f->is_interesting) {
	    write_map_line_and_pp_event_log(f, next_seq,
					    f->last_seq_number,
					    next_seq - f->first_seq_number
					    - lines_in_children + 1,
					    &head, &tail);
	}
	else while(next_pp_event_in_range(next_seq, f->last_seq_number))
		   { }
    }
    if (f->first_pp_AST_node && f->is_interesting && output_buffer != NULL &&
	!(suppress_template_macro_files &&
	  f->has_template_macro_use)) {
        if (generate_multiple_IF_files) {
	    save_pp_AST_node_info_for(f, head, tail);
	}
	else {
	    /* dump the preprocessor AST for this file */
	    add_to_string("AST ", output_buffer);
	    add_quoted_str_to_string(filename, TRUE /*add quotes*/, output_buffer);
	    add_2_chars_to_string(" {", output_buffer);
	    terminate_string(output_buffer);
	    write_pp_AST(f->first_pp_AST_node, head, tail, output_buffer);
	}
    }
    filename = saved_filename;
    pos_of_suppressed_include = include_pos_on_entry;
}

/* The following variable identifies the start of a preprocessing
 * directive, i.e., the "#" character.
 */

a_source_position start_of_dir_position;

/* The following function returns the spelling of a given parameter
 * name (the number is 1-origined).
 */

static const char* macro_param_name(sizeof_t number,
				    a_macro_param_ptr pp) {
    for ( ; --number > 0; pp = pp->next) {}
    return pp->name;
}  /* macro_param_name */

/* The following function is called whenever a macro is defined.  It
 * adds a PP_MACRO_REF event to the preprocessor event log, inserting
 * the macro into the macro cache if it is not already there, and sets
 * the cache entry's def_pos to the specified position.  It also
 * adds an AST node for the #define.  (The code that copies the
 * replacement text was copied from make_il_macro_entry.)
 */

a_macro_cache_entry_ptr register_macro_definition(const char* name,
						  sizeof_t name_len,
						  a_boolean object_like,
						  a_macro_param_ptr params,
						  a_source_position_ptr pos,
						  const char* replacement,
						  a_boolean variadic) {
    a_repl_text_seq_kind rts_kind;
    sizeof_t             rts_number;
    const char*          ptr;
    a_macro_cache_entry_ptr macro = get_macro_cache_entry(name,
            name_len, params);
    a_macro_param_ptr parmp;
    pp_AST_node_ptr np;
    pp_event_ptr ppe = new_pp_event();
    a_source_position end_of_def = pos_curr_token;
    ppe->kind = PP_MACRO_REF;
    ppe->pos = *pos;
    ppe->variant.cache_entry = macro;
    if (!macro->def_pos.seq) {
	macro->def_pos = *pos;
	macro->def_file = curr_ise->assoc_actual_il_file;
    }
    end_of_def.column--;
    end_of_def.mapped_column--;
    new_pp_AST_node(&start_of_dir_position, &end_of_def,
		    "cpp_define", 0, macro, NULL);

    /* Add operands for the macro parameters */

    if (!object_like && !params) {
	/* A function-style macro with no parameters -- just output an
	 * empty cpp_parm node to distinguish it from object-style
	 * macros.
	 */
	np = new_pp_AST_node(&null_source_position, &null_source_position,
			     "cpp_parm ", 0, NULL, NULL);
    	pop_pp_AST_node();
    }
    else for (parmp = params; parmp; parmp = parmp->next) {
	const char* namep;
#if !SET_VARIADIC_MACROS
	if (variadic && parmp->next == NULL) {
#else /* SET_VARIADIC_MACROS */
	if ((variadic && parmp->next == NULL) || parmp->has_ellipsis) {
#endif /* SET_VARIADIC_MACROS */
	    add_to_string(parmp->name, &string_buf);
	    add_3_chars_to_string("...", &string_buf);
	    namep = terminate_string(&string_buf);
	}
	else namep = parmp->name;
	np = new_pp_AST_node(&null_source_position, &null_source_position,
			     "cpp_parm ", 0, NULL, namep);
	pop_pp_AST_node();
    }

    /* convert the replacement text from the internal form to a plain
       string. */

    for (ptr = replacement; *ptr != (int)rt_null;) {
	rts_kind = (a_repl_text_seq_kind)*(ptr++);
	/* Extract the section length or argument number. */
	get_macro_repl_text_number(rts_number, ptr);
	switch (rts_kind) {
	case rt_text:
#if SET_VARIADIC_MACROS
	case rt_text_if_last_arg_supplied:
#endif /* SET_VARIADIC_MACROS */
	    /* Raw text.  rts_number gives its length.  Copy the text, ignoring
	       end-of-token markers. */
	    for (; rts_number > 0; rts_number--) {
		char ch = *ptr++;
		if (ch == LE_ESCAPE) {
		    /* End of token marker. */
		    check_assertion_str(*ptr == LE_END_OF_TOKEN,
					"make_il_macro_entry: bad lexical escape");
		    ptr++;
		    rts_number--;
		} else {
		    add_1_char_to_string(ch, &string_buf);
		}  /* for */
	    }  /* for */
	    break;
	case rt_raw_argument:
	    /* parameter ## normal or parameter ## parameter, or pcc-mode
	       parameter. */
	    add_to_string(macro_param_name(rts_number, params), &string_buf);
	    break;
	case rt_paste:
	    /* ## placeholder */
	    add_2_chars_to_string("##", &string_buf);
	    break;
	case rt_stringized_raw_argument:
	    /* #parameter */
	    add_1_char_to_string('#', &string_buf);
	    add_to_string(macro_param_name(rts_number, params), &string_buf);
	    break;
	case rt_argument:
	    /* Simple parameter name. */
	    add_to_string(macro_param_name(rts_number, params), &string_buf);
	    break;
      default:
	    unexpected_condition_str("make_il_macro_entry: bad text section kind in macro def");
	}  /* switch */
    }  /* for */
    np = new_pp_AST_node(&null_source_position, &null_source_position,
			 "cpp_defn ", 0, NULL, terminate_string(&string_buf));
    pop_pp_AST_node();  /* cpp_defn */
    pop_pp_AST_node();  /* cpp_define */
    return macro;
}

/* The following function is called whenever a macro is #undefed.  It
 * adds a PP_MACRO_REF event to the preprocessor event log, inserting
 * the macro into the macro cache if it is not already there, and sets
 * the cache entry's undef_pos to the specified position.  It also
 * adds a cpp_undef node to the preprocessor AST.
 */

a_macro_cache_entry_ptr register_macro_undefinition(const char* name,
						    sizeof_t name_len,
						    a_macro_param_ptr params,
						    a_source_position_ptr pos) {
    a_source_position undef_end = pos_curr_token;
    a_macro_cache_entry_ptr macro = get_macro_cache_entry(name,
            name_len, params);
    pp_event_ptr ppe = new_pp_event();
    ppe->kind = PP_MACRO_REF;
    ppe->pos = *pos;
    ppe->variant.cache_entry = macro;
    macro->undef_pos = *pos;
    macro->def_file = NULL;
    undef_end.column += undef_end.len - 1;
    undef_end.mapped_column += undef_end.len - 1;
    new_pp_AST_node(&start_of_dir_position, &undef_end,
		    "cpp_undef", 0, macro, NULL);
    pop_pp_AST_node();
    return macro;
}

/* The following function is called for each reference in the source to
 * a macro (macro call or use in ifdef/ifndef/defined()).  It sets the
 * has_template_macro_use and potential_template_macro_file flags in the
 * files on the stack, as appropriate.
 */

static void check_for_template_macro_use(a_macro_cache_entry_ptr macro) {
    if (macro && macro->def_file && curr_ise &&
	macro->def_file != curr_ise->assoc_actual_il_file &&
	(is_file_on_stack(macro->def_file) ||
	 (curr_ise->assoc_actual_il_file->potential_template_macro_file &&
	  macro->def_file->has_template_macro_use))) {
	if (suppress_template_macro_files &&
	    !curr_ise->assoc_actual_il_file->has_template_macro_use) {

	}
	curr_ise->assoc_actual_il_file->has_template_macro_use = TRUE;
	mark_potential_template_macro_files(macro->def_file);
    }
}

/* The following function is called to register a macro use other than
 * in #ifdef/#ifndef lines and macro invocation (currently that means
 * use in a defined() preprocessor operator).
 */

a_macro_cache_entry_ptr register_macro_ref(const char* name,
					   sizeof_t name_len,
					   a_macro_param_ptr params,
					   a_source_position_ptr pos) {
    a_macro_cache_entry_ptr macro = get_macro_cache_entry(name,
	    name_len, params);
    pp_event_ptr ppe = new_pp_event();
    ppe->kind = PP_MACRO_REF;
    ppe->pos = *pos;
    ppe->variant.cache_entry = macro;
    check_for_template_macro_use(macro);
    return macro;
}

/* The following function adds a cpp_ifdef/cpp_ifndef node to the
 * preprocessor AST and registers a reference to the macro (defined or
 * not).  Note that a reference to an undefined macro that is later
 * defined as a function-like macro will not be handled correctly,
 * since the parameter list is part of the name but is not available
 * before it is defined.
 */

void register_ifdef(a_boolean is_ifdef, const char* name,
		    sizeof_t name_len, a_macro_param_ptr params) {
    a_macro_cache_entry_ptr macro = get_macro_cache_entry(name,
            name_len, params);
    pp_AST_node_ptr node = new_pp_AST_node(&start_of_dir_position,
            &pos_curr_token, (is_ifdef) ? "cpp_ifdef" : "cpp_ifndef",
	    0, macro, NULL);
    a_source_position then_pos;
    pp_event_ptr ppe = new_pp_event();
    ppe->kind = PP_MACRO_REF;
    ppe->pos = pos_curr_token;
    ppe->variant.cache_entry = macro;
    push_pp_AST_stack_entry(node);
    then_pos.seq = pos_curr_token.seq + 1;
    then_pos.column = 1;
    then_pos.mapped_column = 1;
    node = new_pp_AST_node(&then_pos, &then_pos, "cpp_then", 0,
			   NULL, NULL);
    cur_pp_AST_stack_entry->cur_block = node;
    check_for_template_macro_use(macro);
}

/* The following function adds a cpp_else node to the preprocessor
 * AST, adjusting the end of the preceding block to the last character
 * of the preceding line (indicated by a column number of 0)
 */

void register_else(void) {
    if (!cur_pp_AST_stack_entry) {
	complain(error_csev, "#else without #if");
    }
    else {
	a_source_position blk_end = start_of_dir_position;
	pp_AST_node_ptr node;
	pop_pp_AST_node(); /* for then */
	blk_end.seq--;
	blk_end.column = 0;	/* special code for "end of line" */
	blk_end.mapped_column = 0;
	cur_pp_AST_stack_entry->cur_block->end = blk_end;
	node = new_pp_AST_node(&start_of_dir_position,
			       &pos_curr_token, "cpp_else",
			       0,
			       NULL, NULL);
	cur_pp_AST_stack_entry->cur_block = node;
    }
}
			      
/* The following function terminates an if/ifdef/ifndef block.  It
 * uses the top of the AST stack to find the associated node, writes
 * the ending source position, and pops the stack.
 */

void register_endif(void) {
    if (!cur_pp_AST_stack_entry) {
	complain(error_csev, "#endif without #if");
    }
    else {
	a_source_position after_endif = pos_curr_token;
	after_endif.column += after_endif.len - 1;
	after_endif.mapped_column += after_endif.len - 1;
	cur_pp_AST_stack_entry->if_node->end = after_endif;
	cur_pp_AST_stack_entry->cur_block->end = after_endif;
	pop_pp_AST_stack_entry();
	pop_pp_AST_node(); /* for the/#elif/#else */
	pop_pp_AST_node(); /* for #if */
    }
}

/* The following function is called when #if, #ifdef, or #ifndef is
 * encountered while an unselected conditional compilation branch is
 * being skipped.  It is necessary because the #elif, #else, and
 * #endif directives in the skipped code are processed normally, which
 * means that there must be a preprocessor AST stack entry for them to
 * operate on.
 */

void register_skipped_if(a_pp_directive_kind kind) {
    const char* dir_name;
    pp_AST_node_ptr node;
    a_source_position then_pos;
    if (kind == ppd_if) {
	dir_name = "cpp_if";
    }
    else if (kind == ppd_ifdef) {
	dir_name = "cpp_ifdef";
    }
    else dir_name = "cpp_ifndef";
    node = new_pp_AST_node(&start_of_dir_position, &pos_curr_token,
			   dir_name, 0, NULL, NULL);
    push_pp_AST_stack_entry(node);
    if (kind == ppd_if) {
	new_pp_AST_node(&pos_curr_token, &pos_curr_token,
			"cpp_null_operand", 0, NULL, NULL);
	pop_pp_AST_node();
    }
    then_pos.seq = pos_curr_token.seq + 1;
    then_pos.column = 1;
    then_pos.mapped_column = 1;
    node = new_pp_AST_node(&then_pos, &then_pos, "cpp_then", 0,
			   NULL, NULL);
    cur_pp_AST_stack_entry->cur_block = node;
}

/* The following source position variable points nowhere (because a */
/* seq of 0 means "invalid," and static variables are initialized to */
/* all zeroes). */

static a_source_position null_pos;

/* The following function recursively adds preprocessor AST nodes to
 * describe a specified (integer constant) expression.
 */

static void dump_expr_to_pp_AST(an_expr_node_ptr expr) {
    const char* op_name;
    an_expr_node_ptr operand = NULL;
    const char* value = NULL;
    pp_AST_node_ptr node;
    a_boolean suppress_node = FALSE;
    a_source_position_ptr start_pos;
    a_source_position_ptr end_pos;
    a_macro_cache_entry_ptr macro = NULL;
    a_source_position local_end_pos;

    if (expr) {
	start_pos = &expr->expr_range.start;
	end_pos = &expr->expr_range.end;
	if (GNU_compatible_AST) {
	    op_name = node_name_str_gnu[expr_node_name(expr)];
	}
	else op_name = node_name_str[expr_node_name(expr)];
	if (expr->kind == enk_constant) {
	    a_constant_ptr constant = expr->variant.constant;
	    if (constant->macro_ref) {
		if (constant->defined_op_range.start.seq) {
		    /* defined() operator */
		    op_name = (GNU_compatible_AST) ?
			    node_name_str_gnu[NN_defined] :
			    node_name_str[NN_defined];
		    start_pos = &constant->defined_op_range.start;
		    end_pos = &constant->defined_op_range.end;
		    macro = constant->macro_ref;
		    constant_to_string(constant, &string_buf);
		    value = terminate_string(&string_buf);
		}
		else {
		    macro = constant->macro_ref;
		    constant_to_string(constant, &string_buf);
		    value = terminate_string(&string_buf);
		    if (!end_pos->seq) {
			local_end_pos = *start_pos;
			local_end_pos.column += start_pos->len - 1;
			local_end_pos.mapped_column += start_pos->len - 1;
			end_pos = &local_end_pos;
		    }
		}
	    }
	    else if (constant->const_expr) {
		operand = constant->const_expr;
		suppress_node = TRUE;
	    }
	    else {
		constant_to_string(constant, &string_buf);
		value = terminate_string(&string_buf);
	    }
	}
	else if (expr->kind == enk_operation) {
	    if (expr->variant.operation.compiler_generated) {
		add_to_string(op_name, &string_buf);
		add_3_chars_to_string(":cg", &string_buf);
		op_name = terminate_string(&string_buf);
	    }
	    operand = expr->variant.operation.operands;
	}
	if (!suppress_node) {
	    node = new_pp_AST_node(start_pos, end_pos, op_name, 0, macro, value);
	    node->add_dummy_file_name = TRUE;
	    if (operand) {
		dump_expr_to_pp_AST(operand);
	    }
	    pop_pp_AST_node();
	}
	else if (operand) {
	    dump_expr_to_pp_AST(operand);
	}
	if (expr->next) {
	    dump_expr_to_pp_AST(expr->next);
	}
    }
}

/* The following variable is set whenever a #if or #elif is
 * encountered to point to that directive's expression.
 */

an_expr_node_ptr last_pp_if_expr;

/* The following function adds a cpp_if node to the preprocessor AST,
 * then dumps the constant expression from last_pp_if_expr as its
 * first operand.
 */

void register_if(void) {
    pp_AST_node_ptr node = new_pp_AST_node(&start_of_dir_position,
            &pos_curr_token, "cpp_if", 0, NULL, NULL);
    a_source_position then_pos;
    push_pp_AST_stack_entry(node);
    if (last_pp_if_expr) {
	dump_expr_to_pp_AST(last_pp_if_expr);
	last_pp_if_expr = NULL;
    }
    else {
	new_pp_AST_node(&pos_curr_token, &pos_curr_token,
			"cpp_null_operand", 0, NULL, NULL);
	pop_pp_AST_node();
    }
    then_pos.seq = pos_curr_token.seq + 1;
    then_pos.column = 1;
    then_pos.mapped_column = 1;
    node = new_pp_AST_node(&then_pos, &then_pos, "cpp_then", 0,
			   NULL, NULL);
    cur_pp_AST_stack_entry->cur_block = node;
}

void register_elif(void) {
    if (!cur_pp_AST_stack_entry) {
	complain(error_csev, "#elif without #if");
    }
    else {
	a_source_position blk_end = start_of_dir_position;
	pp_AST_node_ptr node;
	pop_pp_AST_node(); /* for then */
	blk_end.seq--;
	blk_end.column = 0;	/* special code for "end of line" */
	blk_end.mapped_column = 0;
	cur_pp_AST_stack_entry->cur_block->end = blk_end;
	node = new_pp_AST_node(&start_of_dir_position,
                &pos_curr_token, "cpp_elif", 0, NULL, NULL);
	cur_pp_AST_stack_entry->cur_block = node;
	if (last_pp_if_expr) {
	    dump_expr_to_pp_AST(last_pp_if_expr);
	    last_pp_if_expr = NULL;
	}
	else {
	    new_pp_AST_node(&pos_curr_token, &pos_curr_token,
			    "cpp_null_operand", 0, NULL, NULL);
	    pop_pp_AST_node();
	}
    }
}

/* The following variable records the last cpp_include preprocessor
 * AST node that was added for later update.
 */

pp_AST_node_ptr last_include_pp_AST_node;

/* The following function adds a cpp_include node to the preprocessor
 * AST, leaving the symid and filename blank for later updating.
 */

void register_include(a_source_position_ptr end_of_directive) {
    a_source_position end_dir = *end_of_directive;
    if (end_dir.seq) {
	end_dir.column = 0;	/* end of line */
	end_dir.mapped_column = 0;
    }
    last_include_pp_AST_node = new_pp_AST_node(&start_of_dir_position,
					       &end_dir, "cpp_include",
					       0, NULL, NULL);
    /* Need not track the nesting, as we're in a new file. */
    pop_pp_AST_node();
}

void register_end_of_include() {
}

/* The following function updates the most recent cpp_include
 * preprocessor AST node to reflect the symid and full name of the
 * file that was opened.
 */

void update_include_pp_AST_node(a_source_file_ptr header_file) {
   if (!last_include_pp_AST_node) {
       if (header_file->full_name) {
	   complain_str(error_csev, "Update nonexistent cpp_include node for file $1",
			header_file->full_name);
       }
       else if (header_file->file_name) {
	   complain_str(error_csev, "Update nonexistent cpp_include node for file name $1",
			header_file->file_name);
       }
       else complain(error_csev, "Update nonexistent cpp_include node for file with NULL name");
   }
   else {
       last_include_pp_AST_node->symid = header_file->symid;
       last_include_pp_AST_node->operand =
	       absolute_path_of(header_file->full_name);
       last_include_pp_AST_node = NULL;
   }
}

/* The following variables are used to record the start of a region
 * for later use in adding a MAP copy or MAP fixed event to the
 * preprocessor event log.
 */

a_source_position start_of_map_fixed_region = { 1, 1, 1 };
a_source_position start_of_map_copy_region = { 1, 1, 1 };

/* The following function adds a MAP copy event to the preprocessor
 * event log.
 */

void record_map_copy(a_source_position_ptr from,
		     a_source_position_ptr to) {
    pp_event_ptr ppe = new_pp_event();
    ppe->kind = PP_MAP_COPY;
    ppe->pos = *from;
    ppe->variant.end = *to;
}

/* The following function adds a MAP fixed event to the preprocessor
 * event log.
 */

void record_map_fixed(a_source_position_ptr from,
		      a_source_position_ptr to) {
    pp_event_ptr ppe = new_pp_event();
    ppe->kind = PP_MAP_FIXED;
    ppe->pos = *from;
    ppe->variant.end = *to;
}

static a_macro_cache_entry_ptr get_cache_entry_for_macro_def(
					        a_macro_def_ptr mdp) {
    if (mdp == NULL) {
	return NULL;
    }
    else {
	a_macro_ptr mp = mdp->macro;
	return get_macro_cache_entry(mp->source_corresp.name,
				     strlen(mp->source_corresp.name),
				     mdp->param_list);
    }
}

static a_macro_cache_entry_ptr get_macro_entry_for_slm(
					        a_source_line_modif_ptr slmp) {
    a_macro_cache_entry_ptr macro = NULL;

    if (slmp == NULL || slmp->assoc_macro == NULL) {
    }
    else if (slmp->assoc_macro->macro) {
	macro = get_cache_entry_for_macro_def(slmp->assoc_macro);
    }
    else if (slmp->assoc_macro->object_like) {
	/* This is probably a reference to a built-in macro; we
	 * can reconstruct the spelling from the replaced text
	 * in the modification.
	 */
	add_1_char_to_string(slmp->orig_char, &trial_buf);
	add_to_string_with_len(loc_of_insert(slmp) + 1,
			       slmp->num_chars_to_delete - 1,
			       &trial_buf);
	macro = get_macro_cache_entry(terminate_string(&trial_buf),
				      slmp->num_chars_to_delete, NULL);
    }
    return macro;
}

/* Global variable used in macro.c to track the current macro
 * whose argument is being expanded in isolation.
 */

a_source_line_modif_ptr macro_arg_context = NULL;

/* The following function records a nested macro preprocessor
 * event, if this is not the top level. Uses the given
 * parent_macro if the slmp-based parent is NULL.  This is
 * so that the caller can indicate the context in which an
 * argument is expanded.
 */
void record_nested_macro_call(a_source_line_modif_ptr slmp) {
    a_macro_cache_entry_ptr macro_entry = get_macro_entry_for_slm(slmp);
    a_source_line_modif_ptr parent_slmp = parent_source_line_modif(slmp);
    if (parent_slmp != NULL) {
	/* If the parent slmp is isolated, then use the macro from the
	 * substitute slmp because an argument expanded in isolation is
	 * treated as coming from the macro body in which it occurs. */
        a_macro_cache_entry_ptr parent_entry
	  = get_macro_entry_for_slm(parent_slmp->is_isolated_text
							? macro_arg_context
				     			: parent_slmp);
	if (macro_entry != NULL && parent_entry != NULL) {
	    pp_event_ptr ppe = new_pp_event();
	    ppe->kind = PP_NESTED_MACRO;
	    ppe->pos = pos_curr_token;
	    ppe->variant.nested_macro.caller = parent_entry;
	    ppe->variant.nested_macro.callee = macro_entry;
	}
    }
}

/* The following variables are used to create a list of
 * macro_arg_maps (via new_macro_arg_map and
 * clone_macro_arg_map_list); the list is terminated by
 * end_macro_arg_map_list().  The two-element array is to keep
 * separate the list used by the top-level pcc macro expansion
 * and all other macro expansions, as distinguished by the
 * MACRO_CONTEXT macro.
 */

static a_macro_arg_map_ptr macro_arg_map_list_head[2];
static a_macro_arg_map_ptr macro_arg_map_list_tail[2];

#define MACRO_CONTEXT (SET_expanding_top_level_pcc_macro != 0 && macro_depth == 1)

/* The following variable keeps track of freed macro_arg_maps to
 * reduce allocation overhead.
 */

static a_macro_arg_map_ptr free_macro_arg_map;

/* The following function either recycles an existing macro arg
 * map from the free list or allocates space for a new one and
 * then links it at the end of the list currently being created.
 */

static a_macro_arg_map_ptr alloc_macro_arg_map(a_source_position_ptr pos,
					       const char* mapped_loc) {
    a_macro_arg_map_ptr map;
    if (free_macro_arg_map) {
	map = free_macro_arg_map;
	free_macro_arg_map = map->next;
    }
    else map = (a_macro_arg_map_ptr) malloc(sizeof(a_macro_arg_map));
    map->next = NULL;
    map->pos = *pos;
    map->mapped_loc = mapped_loc;
    if (macro_arg_map_list_tail[MACRO_CONTEXT]) {
	macro_arg_map_list_tail[MACRO_CONTEXT]->next = map;
    }
    else macro_arg_map_list_head[MACRO_CONTEXT] = map;
    macro_arg_map_list_tail[MACRO_CONTEXT] = map;
    return map;
}

/* The action of the following function depends on whether the
 * specified location is in the current source line.  If it is,
 * a new macro_arg_map is created and linked into the list being
 * created.  If not, the containing source line modification is
 * examined to see if it contains a macro_arg_map for that
 * location and, if so, the map is moved from the source line
 * modification to the list.
 */

void new_macro_arg_map(a_source_position_ptr pos, const char* loc,
		       const char* target_loc) {
    if (within_curr_source_line(loc)) {
	alloc_macro_arg_map(pos, target_loc);
    }
    else {
	a_macro_arg_map_ptr map;
	a_macro_arg_map_ptr prev_map = NULL;
	a_macro_arg_map_ptr next_map = NULL;
	a_source_line_modif_ptr slmp = assoc_source_line_modif((char*) loc);
	for (map = slmp ? slmp->macro_arg_map_list : NULL; map;
	     map = next_map) {
	    next_map = map->next;
	    if (ptr_in_range(map->mapped_loc, loc, loc + pos->len)) {
		/* This token is part of a macro argument in a
		 * nested macro call, so it can't be part of the
		 * final text of this modification; we move it
		 * from the list it's in currently to the one
		 * we're now creating and remap its location.
		 */
		ptrdiff_t offset = map->mapped_loc - loc;
		if (prev_map) {
		    prev_map->next = map->next;
		}
		else slmp->macro_arg_map_list = map->next;
		map->next = NULL;
		if (macro_arg_map_list_tail[MACRO_CONTEXT]) {
		    macro_arg_map_list_tail[MACRO_CONTEXT]->next = map;
		}
		else macro_arg_map_list_head[MACRO_CONTEXT] = map;
		macro_arg_map_list_tail[MACRO_CONTEXT] = map;
		map->mapped_loc = target_loc + offset;
	    }
	    else prev_map = map;
	}
    }
}

/* The following function removes any macro arg maps associated with
 * the current token and puts them on the current macro arg map list.
 * It is used during expand_top_level_pcc_macro processing, which
 * gloms all source line modifications together into a single buffer.
 */

void move_curr_token_macro_arg_map(const char* new_token_loc) {
    a_source_line_modif_ptr slmp =
	    assoc_source_line_modif((char*) start_of_curr_token);
    a_macro_arg_map_ptr map;
    a_macro_arg_map_ptr prev_map = NULL;
    a_macro_arg_map_ptr next_map = NULL;
    for (map = slmp ? slmp->macro_arg_map_list : NULL; map;
	 map = next_map) {
	next_map = map->next;
	if (ptr_in_range(map->mapped_loc, start_of_curr_token,
			 start_of_curr_token + pos_curr_token.len)) {
	    ptrdiff_t offset = map->mapped_loc - start_of_curr_token;
	    if (prev_map) {
		prev_map->next = map->next;
	    }
	    else slmp->macro_arg_map_list = map->next;
	    map->next = NULL;
	    if (macro_arg_map_list_tail[MACRO_CONTEXT]) {
		macro_arg_map_list_tail[MACRO_CONTEXT]->next = map;
	    }
	    else macro_arg_map_list_head[MACRO_CONTEXT] = map;
	    macro_arg_map_list_tail[MACRO_CONTEXT] = map;
	    map->mapped_loc = new_token_loc + offset;
	}
	else prev_map = map;
    }
}

/* The following function resets the macro arg map list pointers to
 * set up for creation of a new list and returns the head of the
 * previous list as the result.
*/

a_macro_arg_map_ptr end_macro_arg_map_list(void) {
    a_macro_arg_map_ptr map = macro_arg_map_list_head[MACRO_CONTEXT];
    macro_arg_map_list_head[MACRO_CONTEXT] = NULL;
    macro_arg_map_list_tail[MACRO_CONTEXT] = NULL;
    return map;
}

/* The following function clones a list of macro arg maps to the
 * list that is currently under construction, updating the mapped
 * location appropriately.
 */

void clone_macro_arg_map_list(a_macro_arg_map_ptr list,
			      const char* old_base,
			      const char* new_base) {
    while (list) {
	ptrdiff_t offset = list->mapped_loc - old_base;
	a_macro_arg_map_ptr new_map =
		alloc_macro_arg_map(&list->pos, new_base + offset);
	list = list->next;
    }
}

/* The following function is similar to the preceding one, but copies
 * directly from one source line modification to another.  It saves
 * and restores the global list that may be under construction.
 */

void copy_source_modif_macro_arg_maps(a_source_line_modif_ptr old_slmp,
				      a_source_line_modif_ptr new_slmp) {
    if (old_slmp->macro_arg_map_list) {
	a_macro_arg_map_ptr saved_first = macro_arg_map_list_head[MACRO_CONTEXT];
	a_macro_arg_map_ptr saved_last = macro_arg_map_list_tail[MACRO_CONTEXT];
	macro_arg_map_list_head[MACRO_CONTEXT] = NULL;
	macro_arg_map_list_tail[MACRO_CONTEXT] = NULL;
	clone_macro_arg_map_list(old_slmp->macro_arg_map_list,
				 old_slmp->inserted_text,
				 new_slmp->inserted_text);
	new_slmp->macro_arg_map_list = end_macro_arg_map_list();
	macro_arg_map_list_head[MACRO_CONTEXT] = saved_first;
	macro_arg_map_list_tail[MACRO_CONTEXT] = saved_last;
    }
    else new_slmp->macro_arg_map_list = NULL;
}

/* The following function frees a macro arg map list. */

void free_macro_arg_map_list(a_macro_arg_map_ptr list) {
    while (list) {
	a_macro_arg_map_ptr next = list->next;
	if (next == list) {
	    complain(error_csev, "Circular macro argument map list.");
	    next = NULL;
	}
	list->next = free_macro_arg_map;
	free_macro_arg_map = list;
	list = next;
    }
}

/* The following function is called when a buffer is reallocated,
 * invalidating the mapped_locs of a macro arg map list.
 */

void adjust_macro_arg_maps_after_realloc(a_macro_arg_map_ptr list,
					 const char* old_base,
					 const char* old_after_end,
					 const char* new_base) {
    while (list) {
	if (ptr_in_range(list->mapped_loc, old_base, old_after_end)) {
	    ptrdiff_t offset = list->mapped_loc - old_base;
	    list->mapped_loc = new_base + offset;
	}
	list = list->next;
    }
}

/* The following variables enable calculation of offsets within the
 * (nonexistent) output file.
 */

const char* base_of_cur_offset;
size_t cur_base_offset;
size_t ignored_macro_escapes;

/* The following function is called when a buffer is reallocated,
 * invalidating base_of_cur_offset.
 */

void adjust_SET_mapping_ptr(const char* old_base,
			    const char* old_after_end,
			    const char* new_base) {
    a_macro_arg_map_ptr map;
    ptrdiff_t offset;
    for (map = macro_arg_map_list_head[MACRO_CONTEXT]; map; map = map->next) {
	if (ptr_in_range(map->mapped_loc, old_base, old_after_end)) {
	    offset = map->mapped_loc - old_base;
	    map->mapped_loc = new_base + offset;
	}
    }
    if (ptr_in_range(base_of_cur_offset, old_base, old_after_end)) {
	offset = base_of_cur_offset - old_base;
	base_of_cur_offset = new_base + offset;
    }
}

/* The following function is called before register_source_modif_entry
 * when expanding a macro.
 */

void move_base_of_offset(const char* start_of_deletion) {
    if (macro_depth == 0) {
	base_of_cur_offset = start_of_deletion;
    }
}

/* The following function adds a MAP copy event to the preprocessor
 * event log for the token described by "map" and frees the map
 * object for later reuse.
 */

static a_macro_arg_map_ptr record_map_copies_for_macro_args
	(a_macro_arg_map_ptr list, const char* ending_loc) {
    const char* p = base_of_cur_offset;
    size_t escapes_seen = 0;
    while (list && ptr_in_range(list->mapped_loc, p, ending_loc)) {
	a_macro_arg_map_ptr next = list->next;
	pp_event_ptr ppe = new_pp_event();
	ppe->kind = PP_ARG_USE;
	ppe->pos = list->pos;
	conv_line_loc_to_source_pos((char*) list->mapped_loc,
				    &ppe->variant.target);
	/* Note: cannot just use conv_line_loc_to_source_pos here because
	 * escape characters may have been counted that occur after
	 * the point of use, which will throw off the calculation.
	 */
	while (p < list->mapped_loc) {
	    if (*p == LE_ESCAPE) {
		++p;
		escapes_seen += LE_ESCAPE_LEN;
	    }
	    p++;
	}
	ppe->variant.target.mapped_column = cur_base_offset +
		list->mapped_loc - base_of_cur_offset - escapes_seen;
	list->next = free_macro_arg_map;
	free_macro_arg_map = list;
	list = next;
    }
    return list;
}

/* The following function is called whenever we think we're about
 * to expand a macro; it adjusts the mapping variables to the
 * start of the macro call.
 */

void register_start_of_macro(const char* loc) {
    if (macro_depth == 0 && (IF_file_name || generate_multiple_IF_files)) {
	if (!within_curr_source_line(loc)) {
	    a_source_line_modif_ptr slmp = assoc_source_line_modif((char*) loc);
	    slmp->macro_arg_map_list =
		    record_map_copies_for_macro_args(slmp->macro_arg_map_list,
						     loc);
	}
	cur_base_offset += loc - base_of_cur_offset -
		ignored_macro_escapes;
	base_of_cur_offset = loc;
	ignored_macro_escapes = 0;
    }
}

void register_source_modif_entry_conditional(a_source_line_modif_ptr slmp) {
    /* Ignore text deletion, because we don't enter modification. */
    if (slmp->inserted_text != slmp->end_inserted_text) {
	/* Text replacement. */
	register_source_modif_entry(slmp);
    }
}

/* The following function is called when entering a source
 * modification.  It updates the offset-calculation variables, issues
 * a MAP copy for positions corresponding to macro arguments, and
 * adds a macro call to the macro invocation stack if the modification
 * is associated with a macro.
 */

void register_source_modif_entry(a_source_line_modif_ptr slmp) {
    if (macro_depth == 0 && (IF_file_name || generate_multiple_IF_files)) {
	/* ignore trial expansions for arguments and the like */
	a_source_line_modif_ptr parent_slmp =
		parent_source_line_modif(slmp);
	if (parent_slmp) {
	    /* Create "MAP copy" events for each use of a token from
	     * the source line in the part of the expansion preceding
	     * the location of this new sub-modification.  We have to
	     * do it here before changing the mapping state variables.
	     */
	    parent_slmp->macro_arg_map_list =
		    record_map_copies_for_macro_args(
		    parent_slmp->macro_arg_map_list, loc_of_insert(slmp));
	}
	slmp->registered = TRUE;
	cur_base_offset += loc_of_insert(slmp) - base_of_cur_offset -
		ignored_macro_escapes;
	base_of_cur_offset = slmp->inserted_text;
	ignored_macro_escapes = 0;
    }
    else slmp->registered = FALSE;
}

/* The following function is called when leaving a source
 * modification.  It updates the offset-calculation variables and pops
 * a level from the macro invocation stack if the modification is
 * associated with a macro.
 */

void register_source_modif_exit(a_source_line_modif_ptr slmp) {
    if (slmp->registered) {
	/* ignore trial expansions for arguments and the like */
	slmp->macro_arg_map_list = record_map_copies_for_macro_args(
		slmp->macro_arg_map_list, slmp->end_inserted_text);
	if (slmp->macro_arg_map_list) {
	    complain_pos(error_csev, "Leftover macro arg maps ($1: $2/$3).",
			 &slmp->macro_arg_map_list->pos);
	}
	if (macro_depth == 0 || (macro_depth == 1 &&
				 scanning_for_macro_paren)) {
	    cur_base_offset += slmp->end_inserted_text -
		    base_of_cur_offset - ignored_macro_escapes;
	    base_of_cur_offset = loc_of_insert(slmp) +
		    slmp->num_chars_to_delete;
	    ignored_macro_escapes = 0;
	}
	if (within_curr_source_line(base_of_cur_offset)) {
	    a_source_position end_of_map_fixed_region;
	    int saved_macro_depth = macro_depth;
	    macro_depth = 0;	/* force next call to calculate mapped_column */
	    conv_line_loc_to_source_pos((char*) base_of_cur_offset,
					 &start_of_map_copy_region);
	    macro_depth = saved_macro_depth;
	    end_of_map_fixed_region = start_of_map_copy_region;
	    end_of_map_fixed_region.column--;
	    end_of_map_fixed_region.mapped_column--;
	    record_map_fixed(&start_of_map_fixed_region,
			     &end_of_map_fixed_region);
	}
    }
    slmp->registered = FALSE;	/* modifs are sometimes popped twice for
				 * only one push (!).
				 */
}

/* The following function corrects the position of the starting and
 * ending tokens of a macro call to reflect their actual location in
 * the source code.  This is only significant in the case of a macro
 * call inside a macro argument; the position reported is inside the
 * buffer to which the containing macro argument has been copied, so
 * the macro_arg_map_list of the associated source line modification
 * is used to determine the original position of the call.  (The
 * macro_arg_maps are also removed from the list, since the entire
 * macro call will be replaced and those tokens cannot possibly
 * appear in the ultimate expansion.)  For macro calls occurring
 * directly within the source line, no adjustment is required.  For
 * macro arguments appearing within the expansions of other macro
 * invocations, there is no source position, so the reported position
 * is nulled.
 */

static void adjust_macro_call_location(a_source_position_ptr pos,
				       const char* loc, size_t len,
				       a_boolean is_func_macro,
				       a_boolean is_macro_beginning) {
    if (!within_curr_source_line(loc)) {
	a_source_line_modif_ptr slmp =
		assoc_source_line_modif((char*) loc);
	a_macro_arg_map_ptr map;
	a_macro_arg_map_ptr prev_map = NULL;
	for (map = slmp->macro_arg_map_list; map; map = map->next) {
	    if (ptr_in_range(map->mapped_loc, loc, loc + len)) {
		break;
	    }
	    else if (is_func_macro && is_macro_beginning && *loc == LE_ESCAPE &&
		     loc[LE_ESCAPE_LEN] == '(' &&
		     map->mapped_loc == loc + LE_ESCAPE_LEN) {
		/* This is a function style macro whose macro name is
		 * the result of the expansion of an object-like macro
		 * in a macro argument (really! :-); the map for the
		 * macro name is already gone, but we have to remove the
		 * left paren: set variables to simulate removing an
		 * object-like macro, which isn't this case but will have
		 * the effect we want.
		 */
		is_func_macro = FALSE;
		is_macro_beginning = FALSE;
		break;
	    }
	    prev_map = map;
	}
	if (map) {
	    /* This macro call comes from the source line (in a
	     * macro argument).
	     */
	    a_macro_arg_map_ptr next_map = map->next;
	    if (map->pos.len == len || !is_macro_beginning) {
		/* We need to suppress the mapping for cases in which the
		 * token that appears in the argument list is only a
		 * fragment of the macro name.
		 */
		*pos = map->pos;
	    }
	    else *pos = null_source_position;
	    while (next_map &&
		   ((is_func_macro && *next_map->mapped_loc == '(') ||
		    ptr_in_range(next_map->mapped_loc, loc, loc + len))) {
		/* This loop handles two disparate cases.  The first is
		 * when the macro name is the result of concatenation
		 * of two or more macro arguments.  In that case there
		 * will be more than one map and all but the first must
		 * be removed here (the first map may be removed below
		 * for a function-like macro or left for end-of-macro
		 * processing for an object-like macro).  The second
		 * case is a function-like macro; there will be a map
		 * for the "(" token, which must also be removed here.
		 */
		map->next = next_map->next;
		next_map->next = free_macro_arg_map;
		free_macro_arg_map = next_map;
		next_map = map->next;
	    }
	    if (is_macro_beginning && !is_func_macro) {
		/* Must adjust map and leave it in the list so
		 * end-of-macro mapping will work.
		 */
		map->pos.column += map->pos.len - 1;
		map->pos.mapped_column += map->pos.len - 1;
		map->mapped_loc = loc + len - 1;
	    }
	    else {
		/* Otherwise, remove the map so it won't interfere later */
		map->next = free_macro_arg_map;
		free_macro_arg_map = map;
		if (prev_map) {
		    prev_map->next = next_map;
		}
		else slmp->macro_arg_map_list = next_map;
	    }
	}
	else *pos = null_source_position;
    }
}

/* The following function corrects the position of a token
 * to reflect its actual location in
 * the source code.  This is only significant in the case of
 * inside a macro argument; the position reported is inside the
 * buffer to which the containing macro argument has been copied, so
 * the macro_arg_map_list of the associated source line modification
 * is used to determine the original position.  For tokens occurring
 * directly within the source line, no adjustment is required.
 *
 * Returns whether an adjustment was made. If TRUE, and no column
 * could be determined, then precise_column is set to 0.
 */
a_boolean adjust_token_location(a_source_position_ptr pos,
			        const char* loc, size_t len) {
    if (!within_curr_source_line(loc)) {
	a_source_line_modif_ptr slmp =
		assoc_source_line_modif((char*) loc);
	a_macro_arg_map_ptr map;
	a_macro_arg_map_ptr prev_map = NULL;
	for (map = slmp->macro_arg_map_list; map; map = map->next) {
	    if (ptr_in_range(map->mapped_loc, loc, loc + len)) {
		break;
	    }
	    prev_map = map;
	}
	if (map) {
	    /* This macro call comes from the source line (in a
	     * macro argument).
	     */
	    a_macro_arg_map_ptr next_map = map->next;
	    if (map->pos.len != len) {
		/* We need to suppress the mapping for cases in which the
		 * token that appears in the argument list is only a
		 * fragment of the adjusted token.
		 */
		pos->precise_column = 0;
	    }
	    else {
		pos->precise_column = map->pos.column;
	    }
	}
	else {
	    pos->precise_column = 0;
	}
        return TRUE;
    }
    return FALSE;
}

/* The following declarations form a stack of macro calls, enabling
 * the end position of a given call to be supplied after the
 * processing of all its arguments.  (This is different from the macro
 * stack above, which operates from source line modifications and
 * enables tracking of macro uses in the definition of a macro; this
 * stack is used to create the AST for macro parameters and the like
 * and is called from macro_invocation directly.  This overlap is a
 * direct result of the redundancy in the IF format.
 */

typedef struct a_macro_call* a_macro_call_ptr;

typedef struct a_macro_call {
    a_macro_call_ptr next;
    pp_AST_node_ptr call_node;
} a_macro_call;

static a_macro_call_ptr top_of_macro_call_stack;
static a_macro_call_ptr free_macro_call;

/* The following function creates a macro call node in the
 * preprocessor AST, pushes it on the stack of macro calls, and adds a
 * macro reference to the preprocessor event log.
 */

void register_macro_call(const char* name, sizeof_t name_len,
			 a_macro_param_ptr params,
			 a_source_position_ptr start_pos,
			 const char* start_loc,
			 a_boolean is_func_style_macro) {
    pp_AST_node_ptr node;
    pp_event_ptr ppe;
    a_macro_cache_entry_ptr macro;
    a_macro_call_ptr call;
    a_source_position adjusted_start_pos = *start_pos;

    adjust_macro_call_location(&adjusted_start_pos, start_loc,
			       name_len, is_func_style_macro, TRUE);
    adjusted_start_pos.len = name_len;
    macro = get_macro_cache_entry(name, name_len, params);
    node = new_pp_AST_node(&adjusted_start_pos, &adjusted_start_pos,
			   "cpp_macro_call", 0, macro, NULL);
    if (free_macro_call) {
	call = free_macro_call;
	free_macro_call = call->next;
    }
    else {
	call = (a_macro_call_ptr) malloc(sizeof(a_macro_call));
	macro_call_meter += sizeof(a_macro_call);
    }
    call->next = top_of_macro_call_stack;
    top_of_macro_call_stack = call;
    call->call_node = node;
    if (adjusted_start_pos.seq) {
	ppe = new_pp_event();
	ppe->kind = PP_MACRO_REF;
	ppe->pos = adjusted_start_pos;
	ppe->variant.cache_entry = macro;
    }
    check_for_template_macro_use(macro);
}

/* The following function creates a macro argument node and a macro
 * raw argument node in the preprocessor AST.  If the original source
 * is not in the current source line, we are processing arguments of
 * macros used in macro arguments and the positions are meaningless,
 * so they are ignored.
 */

void register_macro_raw_arg(a_source_position_ptr start_pos,
			    a_source_position_ptr end_pos,
			    const char* orig_source_ptr,
			    const char* text, sizeof_t text_len) {
    pp_AST_node_ptr node;

    add_macro_text_to_string(text, text + text_len, &string_buf);
    if (within_curr_source_line(orig_source_ptr)) {
	node = new_pp_AST_node(start_pos, end_pos, "cpp_macro_arg",
			       0, NULL, NULL);
    }
    else {
	node = new_pp_AST_node(&null_pos, &null_pos, "cpp_macro_arg",
			       0, NULL, NULL);
    }
    if (node) {
	node = new_pp_AST_node(&null_pos, &null_pos, "cpp_raw_arg",
			       0, NULL, terminate_string(&string_buf));
	pop_pp_AST_node();
    }
}

/* The following function creates a macro processed argument node in
 * the preprocessor AST.
 */

void register_macro_processed_arg(const char* text, sizeof_t text_len) {
    pp_AST_node_ptr node;

    add_macro_text_to_string(text, text + text_len, &string_buf);
    node = new_pp_AST_node(&null_pos, &null_pos, "cpp_processed_arg",
			   0, NULL, terminate_string(&string_buf));
    pop_pp_AST_node(); /* for cpp_processed_arg */
    pop_pp_AST_node(); /* for cpp_macro_arg */
}

/* The following function is called when a macro argument has no
 * further macro expansion in it; this avoids a redundant cpp_processed_arg
 * node, while still registering the end of the argument processing.
 */

void register_unchanged_macro_arg(void) {
    pop_pp_AST_node(); /* for cpp_macro_arg */
}

/* The following function is called just to remove the "," separating
 * macro arguments from the token map.
 */

void register_macro_arg_separator(const char* loc) {
    a_source_position pos;
    adjust_macro_call_location(&pos, loc, 1, FALSE, FALSE);
}

/* The following function sets the ending position (iff macro_depth
 * <=1) for the macro call at the top of the stack and pops it from
 * the stack.
 */

void finish_macro_call(a_source_position_ptr end_pos,
		       const char* end_loc) {
    a_source_position adjusted_end_pos = *end_pos;
    adjust_macro_call_location(&adjusted_end_pos, end_loc, 1, FALSE,
			       FALSE);
    if (!top_of_macro_call_stack) {
	complain(error_csev, "finish_macro_call with stack empty.");
    }
    else {
	a_macro_call_ptr call = top_of_macro_call_stack;
	top_of_macro_call_stack = call->next;
	call->call_node->end = adjusted_end_pos;
	call->next = free_macro_call;
	free_macro_call = call;
	pop_pp_AST_node();
    }
}

a_boolean scanning_for_macro_paren;

a_boolean SET_expanding_top_level_pcc_macro;

void dump_pp_memory_meters(void) {
    fprintf(stderr, "macro_cache_meter =\t\t%10lu\n", (unsigned long)macro_cache_meter);
    fprintf(stderr, "pp_event_meter =\t\t%10lu\n", (unsigned long)pp_event_meter);
    fprintf(stderr, "pp_AST_stack_meter =\t\t%10lu\n", (unsigned long)pp_AST_stack_meter);
    fprintf(stderr, "pp_AST_node_meter =\t\t%10lu\n", (unsigned long)pp_AST_node_meter);
    fprintf(stderr, "file_lines_meter =\t\t%10lu\n", (unsigned long)file_lines_meter);
    fprintf(stderr, "macro_call_meter =\t\t%10lu\n", (unsigned long)macro_call_meter);
}
