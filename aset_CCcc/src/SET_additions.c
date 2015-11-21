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
/* This code is not part of the EDG distribution; it contains the
 * routines added by Software Emancipation to extract information from
 * the EDG IL and the SET-supplied data structures (xref info, error
 * message list) and dump it in the format required by the External
 * Language Support Intermediate Format.
 */
 
/* The root from which most of the functions in this file are called is
 * dump_SET_IF (the exceptions are the functions that are called to track
 * the preprocessor AST and events).  This is called from fe_wrapup after
 * all the other processing in the front end is complete.  (The reason for
 * adding this call there instead of defining it as the back end is to
 * allow for dumping the IL directly; the most convenient way to do that
 * is to pretend that the back end is a separate process and that the IL
 * must be saved for it.  This allows the display program edgcpdisp to
 * read the .cil file and produce a readable version of the IL, which
 * can be useful in debugging situations.)
 *
 * The IF produced by the routines in this file is structured into
 * two principal sections, corresponding to the hypothetical division
 * between the preprocessor and the parser.  Each section is introduced
 * by the appropriate "SMT language" line.
 *
 * The preprocessor section contains information regarding files, macros,
 * and conditional compilation, including the information required to map
 * locations in the (imaginary) cpp output file to locations in the
 * original source files.  The organization of this information is not
 * constrained by the IF specification, so it is output in the order in
 * which it is most convenient.  The only order constraint is that the
 * line number references following a given "SMT file" line are relative
 * to that file.  The order in which information is dumped in this section
 * corresponds to a top-down left-right traversal of the inclusion tree,
 * with the (preprocessor) AST for a given file being dumped at the end
 * of the processing for that file.
 *
 * The information in the second ("parser") section of the IF file is
 * organized into several sections.  The first section includes all the
 * SYM, REL, and ATR lines for parser entities.  This information results
 * from a tree walk of the file scope and routine scope ILs.  The
 * function process_entry is called for each IL node encountered during
 * these walks, and it dumps the various lines for each node as
 * appropriate.  In addition, it constructs the AST tree to parallel the
 * IL nodes.
 *
 * Next, the SMT lines are dumped; the information for these lines was
 * saved during the parsing by capturing the arguments passed to the
 * EDG routines that normally create the cross reference file.
 * Similarly the ERR lines are dumped from saved info.
 *
 * Finally, the AST that was built during the IL tree walk is dumped
 * by a left-right top-down traversal.  The reason for a separate tree
 * structure and dump pass is that the IL tree walk is bottom-up,
 * while the structure of the AST section of the IF is top-down.
 *
 * This file is organized into groupings of functionally-related entities
 * in roughly the order described below:
 *
 * -------------------- STRING MANAGEMENT --------------------
 *
 * structs:	string_buffer
 *
 * variables:	output_cb output_buf trial_cb trial_buf string_cb
 * 		string_buf
 *
 * functions:	init_string_buffer handle_output_overflow
 * 		handle_trial_overflow handle_string_overflow
 * 		add_to_string_with_len add_to_string add_to_string_buf
 * 		add_to_output_buf add_to_trial_buf add_num_to_string
 * 		add_symid_to_string add_pos_to_string
 * 		add_pos_and_len_to_string add_range_to_string
 *		add_file_rel_pos_to_string add_file_rel_pos_and_len_to_string
 *		add_fcn_name_to_string add_macro_text_to_string
 * 		terminate_string add_quoted_str_to_string
 *
 * macros:	STRING_BUFFER_SIZE
 *
 * String handling is done using the string_buffer data structure and
 * a number of routines that initialize a buffer, add text of various
 * sorts, and terminate a string.  A string_buffer is a block of
 * storage large enough to hold many strings, some state information
 * identifying the current string, and some information that
 * parameterizes the string buffer for various uses.  When the storage
 * block gets full, a handler is called to process the accumulated
 * strings and set up the buffer to accept more strings.
 *
 * Strings are used for two main purposes in this file: for immediate
 * output to the IF file and for the text of AST nodes.  These two
 * uses have different requirements, and the string_buffer can be
 * parameterized to handle both.  The various "...output..." variables
 * and routines serve the first purpose, and the corresponding
 * "...string..." entities serve the second.  For output, strings are
 * terminated with \n and overflow results in flushing the accumulated
 * text to the IF file; for AST text, strings are terminated with \0
 * and overflow results in allocating a new buffer (since the AST
 * nodes maintain pointers to their associated strings, the old buffer
 * is simply orphaned).  (The "...trial..." entities are for
 * constructing strings that may or may not be used, so they are
 * either copied to another string_buffer when complete or discarded.)
 *
 * This file relies upon utility routines in the EDG compiler proper
 * to create human-readable versions of various names, types, and
 * constants; these EDG routines are parameterized with the ..._cb
 * control blocks, which direct the output of the EDG routines to the
 * associated string_buffers here; similarly, a string_buffer
 * identifies the associated control block.
 *
 * -------------------- SCOPE MANAGEMENT --------------------
 *
 * variables:	num_blocks block_depth block_stack block_stack_size
 * 		fcn_name fcn_id doing_IF_dump
 *
 * functions:	push_block pop_block notify_entering_scope
 *
 * macros:	INIT_BLOCK_STACK_SIZE
 *
 * The names of declarations that are local to a function must be
 * decorated in the IF with the function name and an indication of the
 * block containing the declaration in order to prevent unwanted
 * aliasing of distinct declarations.  Because the IL tree walk is
 * inherently bottom-up, the tree walking code in the EDG parser
 * proper was modified slightly to call a notification function
 * whenever a scope is entered; the end of the scope is detected by
 * process_entry when the scope is encountered on the tree walk.
 * Scopes are numbered according to the order of their opening "{"
 * (not all blocks are treated as scopes, only those containing
 * declarations), and the stack is used to keep track of the sequence
 * number of the containing block when popping a block.  The fcn_name
 * variable is set when a function scope is entered and zeroed when it
 * is exited.
 *
 * -------------------- Misc dumping routines --------------------
 *
 * functions:	dump_xref dump_err_msgs indent_to_depth walk_ast
 *		dump_ast
 *
 * dump_xref and dump_err_msgs take information that was stored during the
 * compilation (cross-references and error messages, respectively)
 * and dump it to the IF file in the requisite format.
 *
 * dump_ast dumps the parser AST (not the preprocessor AST), using
 * walk_ast to traverse the AST recursively, dumping as it goes.
 */

#include "basic_hdrs.h"
#include "fe_common.h"
#include "lexical.h"
#include "macro.h"
#include "SET_names.h"
#include "SET_additions.h"
#include "SET_file_table.h"
#include "SET_multiple_iffs.h"
#include "SET_process_entry.h"
#include "SET_ast.h"
#include "SET_symbol.h"
#include "SET_symid.h"
#include "SET_preprocess.h"
#include "SET_il.h"
#include "SET_dump_if.h"
#include "SET_complaints.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

/* debugging flags */
#if DEBUG
a_boolean db_ast = FALSE;
a_boolean db_expected_syms = FALSE;
a_boolean db_statement_stack = FALSE;
a_boolean db_source_sequence = FALSE;
a_boolean db_symid = FALSE;
a_boolean db_sync = FALSE;		/* Synchronize output buffer. */
a_boolean db_same_symid = FALSE;
a_boolean db_walk = FALSE;
a_boolean db_worry = FALSE;
a_boolean db_line = FALSE;

void init_db(void)
{
    db_ast = db_flag_is_set("ast");
    db_expected_syms = db_flag_is_set("xsym");
    db_statement_stack = db_flag_is_set("smstk");
    db_source_sequence = db_flag_is_set("sseq");
    db_symid = db_flag_is_set("ifsym");
    db_sync = db_flag_is_set("sync");
    db_same_symid = db_flag_is_set("samesym");
    db_walk = db_flag_is_set("walk");
    db_worry = db_flag_is_set("worry");
    db_line = db_flag_is_set("line");
}
#endif

static void complain_invalid_buffer(void) {
    complain(catastrophe_csev, "Invalid buffer.");
}


/* The following variables are used for metering memory usage by the
 * functions in this file.
 */

static size_t string_buffer_reallocs;
static size_t string_buffer_meter;
static size_t pp_string_buffer_meter;
static size_t block_stack_reallocs;
static size_t block_stack_meter;
static size_t sym_summary_meter;


#ifdef FORCE_INTR_MACROS
#define get_intr_info_from_il_node(ilp) ((ilp) ? interesting_node(ilp) : NULL)
#endif

#ifdef FORCE_INTR_MACROS
#define get_buffer_from_il_node(ilp) ((interesting_node(ilp)) ? \
				      interesting_node(ilp)->buf_ptr : NULL) 
#endif

#ifdef FORCE_INTR_MACROS
#define get_buffer_of_source_file(sfp) ((sfp && sfp->is_interesting) ? \
				     sfp->is_interesting->buf_ptr : NULL)
#endif

#ifdef FORCE_INTR_MACROS
#define get_intr_info_from_source_file(sfp) ((sfp) ? sfp->is_interesting : NULL)
#endif

#define get_intr_info_from_buffer(bp) ((bp) ? intr_info_array[bp->id] : NULL)

/* The following declarations implement an extensible string buffer.
 * The initial size of the buffer is given by the macro
 * STRING_BUFFER_SIZE, which is intended to be large enough to hold
 * many average strings and all but the very largest strings.
 * The buffer structure is parameterized by a function pointer,
 * handle_overflow, an il_to_str output control block, and a
 * character, string_terminator.  These fields allow the buffer to
 * be used either as a collection of persistent information (as for
 * AST node text) or as a buffer that will be flushed to an output
 * file and reused.
 */

#define STRING_BUFFER_SIZE 500000

typedef struct string_buffer {
    char* buf;		/* pointer to beginning of buffer */
    size_t buf_size;	/* number of bytes that the buffer can hold */
    char* cur_str;	/* pointer to beginning of string under construction */
    char* next_free_byte; /* points to where to add new text in the buffer */
    char* last_byte_of_buf; /* points to end of buffer */
    void (*handle_overflow)(string_buffer_ptr);
    an_il_to_str_output_control_block* ocb; /* Watch out for dummy_cb! */
    char string_terminator;
    size_t init_buf_size;  /* number of bytes that the buffer should be 
			     initiazed with */
    char* output_filename; /* the name of the file into which buffer contents 
			      will be flushed.*/
    FILE* fd;              /* File descriptor of output_filename */
    unsigned int id;
} string_buffer;

/* The following function initializes a string buffer by allocating
 * the buffer storage; if the existing buffer has a partial string in
 * it, the partial string is copied to the new buffer storage.  The
 * current string pointer is adjusted to point to the newly-allocated
 * buffer.  This routine can thus be used either to set up a string
 * buffer for the first time or to recover from overflow.  If there
 * is already a buffer and the current string begins at that location,
 * the buffer is extended before being replaced.
 */

static void init_string_buffer(string_buffer_ref bp) {
    char* new_buf;
    a_boolean free_old_buf = FALSE;
    size_t cur_str_len = bp->next_free_byte - bp->cur_str;
    if (!bp->buf) {
	bp->buf_size = bp->init_buf_size;
    }
    else if (bp->cur_str == bp->buf) {
	/* This is the case where a single string won't fit in the
	 * buffer; we need to allocate a bigger one, copy the entire
	 * old contents (the single string under construction), and
	 * free the old one.  (Normally we leave the old one around
	 * since there will be pointers into it, but there are no
	 * pointers to the buffer contents created until the current
	 * string is terminated, which hasn't happened yet.
	 */
	bp->buf_size *= 2;
	free_old_buf = TRUE;
	string_buffer_reallocs++;
    }
    else if (cur_str_len < bp->init_buf_size / 2) {
	/* No need to use a large buffer size for every buffer just
	 * because one string needed it.
	 */
	bp->buf_size = bp->init_buf_size;
    }
    new_buf = (char*) malloc(bp->buf_size);
    string_buffer_meter += bp->buf_size;
    if (cur_str_len) {
	memcpy(new_buf, bp->cur_str, cur_str_len);
    }
    if (free_old_buf) {
	free(bp->buf);
    }
    bp->buf = new_buf;
    bp->cur_str = new_buf;
    bp->next_free_byte = new_buf + cur_str_len;
    bp->last_byte_of_buf = bp->buf + bp->buf_size - 1;
}

static void add_to_active_buf(char*);

/* This control block is shared by all output buffers for multiple IF files.
 * Should be accessed only via ocb_of_string_buf.
 */
static an_il_to_str_output_control_block dummy_cb = { &add_to_active_buf };

/* The following declarations implement the string buffer for doing
 * buffered output; overflow handling consists of dumping the contents
 * to the IF file (including any partial contents of the cur_str) and
 * resetting the cur_str pointer to the beginning of the buffer.  The
 * end-of-string character is a newline.
 */

void handle_output_overflow(string_buffer_ref bp) {
    if (bp) {
        a_boolean err = (write(fileno(IF_file), bp->buf, bp->next_free_byte - bp->buf) == -1);
        if (err) {
	    complain_str(catastrophe_csev, "Failure writing to IF file $1", IF_file_name);
            term_compilation(catastrophe_csev);
        }
        bp->cur_str = bp->buf;
        bp->next_free_byte = bp->buf;
    }
    else complain_invalid_buffer();
}

static void add_to_output_buf(char*);

static an_il_to_str_output_control_block output_cb = { &add_to_output_buf };

/* Note the string terminator may be modified to '\r', to signal
 * that CRLF should be output as the terminator.
 */
string_buffer output_buf = { NULL, 0, NULL, NULL, NULL,
			     &handle_output_overflow,
			     &output_cb, '\n', 
			     STRING_BUFFER_SIZE, NULL, 
			     NULL, 0 };

/* The following declarations implement a string buffer for trial
 * strings; it is just like the output_buf except that the termination
 * character is a NUL instead of a newline and the overflow handling
 * just wraps around to the beginning of the buffer again.  It is used
 * to construct a string that might or might not be output or saved
 * (e.g., an ATR line when it is not known if any attributes will be
 * added).
 */

static void handle_trial_overflow(string_buffer_ptr bp) {
    if (!bp || bp->cur_str == bp->buf) {
	init_string_buffer(bp);
    }
    else {
	size_t cur_str_len = bp->next_free_byte - bp->cur_str;
	if (cur_str_len) {
	    memcpy(bp->buf, bp->cur_str, cur_str_len);
	}
	bp->cur_str = bp->buf;
	bp->next_free_byte = bp->buf + cur_str_len;
    }
}

static void add_to_trial_buf(char*);

static an_il_to_str_output_control_block trial_cb = { &add_to_trial_buf };

string_buffer trial_buf = { NULL, 0, NULL, NULL, NULL,
			    &handle_trial_overflow,
			    &trial_cb, '\0',
			    STRING_BUFFER_SIZE, NULL, 
			    NULL, 0 };

/* The following declarations implement the string buffer for doing
 * string heap management; overflow handling consists of allocating a
 * new buffer and copying the string currently under construction to
 * the beginning of the new buffer.  The end-of-string character is a
 * NUL.
 */

static void handle_string_overflow(string_buffer_ptr bp) {
    init_string_buffer(bp);
}

static void add_to_string_buf(char*);

an_il_to_str_output_control_block string_cb = { &add_to_string_buf };

string_buffer string_buf = { NULL, 0, NULL, NULL, NULL,
			     &handle_string_overflow,
			     &string_cb, '\0',
			     STRING_BUFFER_SIZE, NULL,
			     NULL, 0 };

static string_buffer_ptr active_string_buffer = NULL;
an_il_to_str_output_control_block *ocb_of_string_buf(string_buffer_ref p) {
    an_il_to_str_output_control_block *ocb = p ? p->ocb : NULL;
    if (ocb == &dummy_cb) {
	active_string_buffer = p;
    }
    return ocb;
}

void set_string_buffer_terminator(char ch, string_buffer_ptr p) {
    if (p != NULL) {
	p->string_terminator = ch;
    }
}

#ifndef FORCE_INTR_MACROS
intr_info_ptr get_intr_info_from_il_node(const void *ilp) {
    return ilp ? interesting_node(ilp) : NULL;
}
string_buffer_ptr get_buffer_from_il_node(const void *ilp) {
    return interesting_node(ilp)
      ? interesting_node(ilp)->buf_ptr
      : NULL;
}
string_buffer_ptr get_buffer_of_source_file(a_source_file_ptr sfp) {
    return  (sfp && sfp->is_interesting)
	   ? sfp->is_interesting->buf_ptr
	   : NULL;
}
intr_info_ptr get_intr_info_from_source_file(a_source_file_ptr sfp) {
    return sfp ? sfp->is_interesting : NULL;
}
struct pp_AST_node_info *get_pp_ast_of_source_file(a_source_file_ptr sfp) {
    return  (sfp && sfp->is_interesting)
           ? sfp->is_interesting->pp_ast
      	   : NULL;
}
void set_pp_ast_of_source_file(a_source_file_ptr sfp,
			       struct pp_AST_node_info *pp) {
    if (sfp != NULL && sfp->is_interesting) {
        sfp->is_interesting->pp_ast = pp;
    }
}
#endif

/* The following function handles output buffer overflow by writing the
 * the buffer contents in to the output file. This function is used in
 * intr_node structure. Since the IF output could be to many files
 * (dependending on the number of header files that do not already have an 
 * IF) we open a file, write buffer contents and close the file. We do not 
 * keep open file handles since we have the risk of exceeding maximum number 
 * of open file handles allowed for a process.  
 */

static void handle_intrfile_overflow(string_buffer_ref bp) {
  a_boolean err = FALSE;
  a_boolean already_opened = FALSE;
  FILE * iffp = NULL;

  if (!bp) {
    complain_invalid_buffer();
    return;
  }

  if (bp->fd) {
    /* there is an open file descriptor */
    already_opened = TRUE;
    iffp = bp->fd;
  }
  else {
    /* open file now */
#ifdef _WIN32
    iffp = fopen(bp->output_filename, "ab");
#else
    iffp = fopen(bp->output_filename, "a");
#endif
  }

  if (iffp) {
    err = (write(fileno(iffp), bp->buf, bp->next_free_byte - bp->buf) == -1);
    if (err) {
      complain_str(catastrophe_csev, "Failure writing to IF file $1", bp->output_filename);
    }
    /* close the file only if it was opened here */
    if (!already_opened) {
      fclose(iffp);
    }
  }
  else {
    complain_str(catastrophe_csev, "Failure opening IF file $1", bp->output_filename);
  }
  bp->cur_str = bp->buf;
  bp->next_free_byte = bp->buf;
}

/* Macros for interesting files */

#define MAX_INTR_FILES 1024
#define INTR_BUF_SIZE 10000
#define MAX_OPEN_FILES 10

/* The following variables are used to maintain a list of all intr_info 
 * structures (one structure for every interesting file) created till now. 
 */
intr_info_ref *intr_info_array = NULL;
unsigned int intr_info_count = 0;

/* The following function adds a new intr_info structure to the list.
 */

static void add_new_intr_info_to_arr(intr_info_ptr ip)
{
  if (ip) {
    /* Initialize the array the first time */
    if (!intr_info_array) {
      intr_info_array = (intr_info_ptr*)malloc(MAX_INTR_FILES *
					       sizeof(intr_info_ptr));
    }
    /* add new intr_info_ptr to array */
    intr_info_array[intr_info_count++] = ip;
  }
}

/* The following function goes through every intr_node created and 
 * flushes the output buffer contents in to its file by calling 
 * handle_intrfile_overflow function.
 */

void flush_all_output_buffers(void)
{
  int i;
  if (intr_info_array) {
    intr_info_ptr iip;
    string_buffer_ref bp;
    for (i = 0; i < intr_info_count; i++) {
      iip = intr_info_array[i];
      bp = get_buffer_from_intr_info(iip);

      /* flush buffer contents */
      handle_intrfile_overflow(bp);

      /* close open file */
      if (bp->fd) {
	fclose(bp->fd);
      }
      /* Print source file name and its IF file name that is considered 
         "interesting" and generated IF.
      */ 
      fprintf(stdout, "parsed: %s  %s\n", iip->filename, bp->output_filename);

      /* Remove lock file indicating completion of IF generation for the file */
      remove_lock_file_for(bp->output_filename);  
    }
  }
  close_generated_IF_files_file();
}

/* The following function creates a new intr_info node and adds it to the
 * global array. The string_buffer it creates gets the size of 10K
 * (from INTR_BUF_SIZE). Since many files could be opened at a time this
 * size is considerably lower than for other string_buffer sizes (500K). 
 */

static intr_info_ref create_new_intr_info(const char* file, const char* iffilename,
					  a_boolean is_header)
{
  intr_info_ptr intr_ptr = NULL;
  string_buffer_ptr bp = NULL;

  intr_ptr = (intr_info_ptr) malloc(sizeof(intr_info));
  if (intr_ptr) {

    bp = (string_buffer_ptr) malloc(sizeof(string_buffer));
    if (bp) {
      bp->buf = NULL;
      bp->buf_size = 0;
      bp->cur_str = NULL;
      bp->next_free_byte = NULL;
      bp->last_byte_of_buf = NULL;
      bp->handle_overflow = &handle_intrfile_overflow;
      bp->ocb = &dummy_cb; /* impossible to create a correct cb on the fly */
#ifdef _WIN32
      bp->string_terminator = '\r';
#else
      bp->string_terminator = '\n';
#endif
      /* initialize buffer size to 10K */
      bp->init_buf_size = INTR_BUF_SIZE;
      bp->output_filename = strdup(iffilename);

      /* Open output file for the source file*/
      if (!is_header || intr_info_count < MAX_OPEN_FILES) {
#ifdef _WIN32
          bp->fd = fopen(bp->output_filename, "ab");
#else
          bp->fd = fopen(bp->output_filename, "a");
#endif
	  if (!bp->fd) {
	      complain_str(catastrophe_csev, "Failure opening IF file $1", bp->output_filename);
	  }
      }
      else bp->fd = NULL;
      bp->id = intr_info_count;

      init_string_buffer(bp);
    }

    intr_ptr->buf_ptr = bp;
    intr_ptr->first_ast_node = NULL;
    intr_ptr->last_ast_node = NULL;
    intr_ptr->pp_ast = NULL;
    intr_ptr->filename = strdup(file);

    add_new_intr_info_to_arr(intr_ptr);
  }
  return intr_ptr;
}

/* The following function returns intr_info from a source sequence
 * number. This is done by first getting the source file for a given
 * sequence and then by getting intr_info of the source file.
 */

intr_info_ptr get_intr_info_from_source_seq(a_seq_number seq)
{
  intr_info_ptr retval = NULL;
  a_line_number line_number;
  a_boolean at_end_of_source;
  
  a_source_file_ptr sfp = source_file_for_seq(seq, &line_number, 
					      &at_end_of_source,
					      FALSE);
  retval = get_intr_info_from_source_file(sfp);
  return retval;
}

/* Similar to the previous function, except this one returns output 
   buffer.
 */

string_buffer_ptr get_buffer_from_source_seq(a_seq_number seq)
{
  intr_info_ptr iip =  get_intr_info_from_source_seq(seq);
  return (iip) ? get_buffer_from_intr_info(iip) : NULL;
}


/* Holds primary source file's interesting info structure.
 */
static intr_info_ptr primary_source_intr_info = NULL;

/* Returns buffer of primary source file */
string_buffer_ptr get_primary_source_buffer(void)
{
  return (primary_source_intr_info) ? 
    get_buffer_from_intr_info(primary_source_intr_info) : NULL;
}


/* The following function copies the input argument to the specified
 * string buffer, handling overflow as necessary.
 */

void add_to_string_with_len(const char* text, size_t len,
			    string_buffer_ref bp) {
    if (bp) {
        register char* new_next_free_byte = bp->next_free_byte + len;
        if (new_next_free_byte > bp->last_byte_of_buf) {
        	(*bp->handle_overflow)(bp);
	    while (len > bp->buf_size) {
	        init_string_buffer(bp);
	    }
	    new_next_free_byte = bp->next_free_byte + len;
        }
        memcpy(bp->next_free_byte, text, len);
        bp->next_free_byte = new_next_free_byte;
    }
    else complain_invalid_buffer();
}

/* The following functions are fixed-length versions of the preceding
 * (it's more efficient to copy a known number of characters than to
 * call memcpy, at least for a small number of characters, and many
 * of the calls are for 1, 2, or 3 characters).
 */

void add_1_char_to_string(char ch, string_buffer_ref bp) {
    if (bp) {
        register char* free_byte = bp->next_free_byte;
        if (free_byte + 1 > bp->last_byte_of_buf) {
	    (*bp->handle_overflow)(bp);
	    free_byte = bp->next_free_byte;
        }
        free_byte[0] = ch;
        bp->next_free_byte = free_byte + 1;
    }
    else complain_invalid_buffer();
}

void add_2_chars_to_string(char* str, string_buffer_ref bp) {
    if (bp) {
        register char* free_byte = bp->next_free_byte;
        if (free_byte + 2 > bp->last_byte_of_buf) {
            (*bp->handle_overflow)(bp);
	    free_byte = bp->next_free_byte;
        }
        free_byte[0] = str[0];
        free_byte[1] = str[1];
        bp->next_free_byte = free_byte + 2;
    }
    else complain_invalid_buffer();
}

void add_3_chars_to_string(char* str, string_buffer_ref bp) {
    if (bp) {
        register char* free_byte = bp->next_free_byte;
        if (free_byte + 3 > bp->last_byte_of_buf) {
            (*bp->handle_overflow)(bp);
	    free_byte = bp->next_free_byte;
        }
        free_byte[0] = str[0];
        free_byte[1] = str[1];
        free_byte[2] = str[2];
        bp->next_free_byte = free_byte + 3;
    }
    else complain_invalid_buffer();
}

/* The following function adds a NUL-terminated string to the
 * specified buffer.
 */

void add_to_string(const char* text, string_buffer_ref bp) {
    add_to_string_with_len(text, strlen(text), bp);
}

/* The following functions are callbacks that are registered in the
 * il_to_str_output_control_blocks to redirect output to the
 * respective string buffers
 */

static void add_to_string_buf(char* text) {
    add_to_string(text, &string_buf);
}

static void add_to_output_buf(char* text) {
    add_to_string(text, &output_buf);
}

static void add_to_trial_buf(char* text) {
    add_to_string(text, &trial_buf);
}

/* This callback is shared by all of the string buffers for
 * multiple IF files.  It relies on the global variable
 * active_string_buffer set by ocb_of_string_buf.
 */
static void add_to_active_buf(char* text) {
    if (active_string_buffer != NULL) {
        add_to_string(text, active_string_buffer);
    }
    else {
	complain(error_csev, "Shunting a few bytes to the bit bucket.");
    }
}

/* The following function adds a number to the string (with optional
 * bracketing strings).  Because metering showed sprintf to be the single
 * largest time sink in IF dumping, an alternative method is provided here.
 * It assumes that unsigned longs are at least 32 bits long and that no value
 * greater than 0xffffffff will ever be printed (if this is run on
 * an architecture where unsigned long is >32 bits).  If these
 * assumptions are not correct, use of sprintf can be restored by
 * defining the macro USE_SPRINTF_FOR_NUMS.  (The assumption is also made
 * that the total length of the bracketing strings and the converted
 * number will be <= 32.)
 */

void add_num_to_string(unsigned long num, const char* pre_str,
		       const char* post_str, string_buffer_ref bp) {
  if (bp) {
#ifdef USE_SPRINTF_FOR_NUMS
    char buff[32];
    size_t len;
#ifdef __BSD__
    sprintf(buff, "%s%lu%s", (pre_str) ? pre_str : "", num,
	    (post_str) ? post_str : "");
    len = strlen(buff);
#else /* !defined(__BSD__) */
    len = sprintf(buff, "%s%lu%s", (pre_str) ? pre_str : "", num,
		  (post_str) ? post_str : "");
#endif /* __BSD__ */
    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
  register char* buf = bp->next_free_byte;
  if (buf + 32 > bp->last_byte_of_buf) {
    (*bp->handle_overflow)(bp);
    buf = bp->next_free_byte;
  }
  if (pre_str) {
    while (*pre_str) {
      *buf++ = *pre_str++;
    }
  }
  if (num >= 10UL) {
    if (num >= 100UL) {
      if (num >= 1000UL) {
	if (num >= 10000UL) {
	  if (num >= 100000UL) {
	    if (num >= 1000000UL) {
	      if (num >= 10000000UL) {
		if (num >= 100000000UL) {
		  if (num >= 1000000000UL) {
		    if (num >= 3000000000UL) {
		      if (num >= 4000000000UL) {
			num -= 4000000000UL;
			*buf++ = '4';
		      }
		      else {
			num -= 3000000000UL;
			*buf++ = '3';
		      }
		    }
		    else if (num >= 2000000000UL) {
		      num -= 2000000000UL;
		      *buf++ = '2';
		    }
		    else {
		      num -= 1000000000UL;
		      *buf++ = '1';
		    }
		  }
		  if (num >= 500000000UL) {
		    if (num >= 800000000UL) {
		      if (num >= 900000000UL) {
			num -= 900000000UL;
			*buf++ = '9';
		      }
		      else {
			num -= 800000000UL;
		        *buf++ = '8';
		      }
		    }
		    else if (num >= 600000000UL) {
		      if (num >= 700000000UL) {
			num -= 700000000UL;
			*buf++ = '7';
		      }
		      else {
			num -= 600000000UL;
			*buf++ = '6';
		      }
		    }
		    else {
		      num -= 500000000UL;
		      *buf++ = '5';
		    }
		  }
		  else if (num >= 200000000UL) {
		    if (num >= 400000000UL) {
		      num -= 400000000UL;
		      *buf++ = '4';
		    }
		    else if (num >= 300000000UL) {
		      num -= 300000000UL;
		      *buf++ = '3';
		    }
		    else {
		      num -= 200000000UL;
		      *buf++ = '2';
		    }
		  }
		  else if (num >= 100000000UL) {
		    num -= 100000000UL;
		    *buf++ = '1';
		  }
		  else *buf++ = '0';
		}
		if (num >= 50000000UL) {
		  if (num >= 80000000UL) {
		    if (num >= 90000000UL) {
		      num -= 90000000UL;
		      *buf++ = '9';
		    }
		    else {
		      num -= 80000000UL;
		      *buf++ = '8';
		    }
		  }
		  else if (num >= 60000000UL) {
		    if (num >= 70000000UL) {
		      num -= 70000000UL;
		      *buf++ = '7';
		    }
		    else {
		      num -= 60000000UL;
		      *buf++ = '6';
		    }
		  }
		  else {
		    num -= 50000000UL;
		    *buf++ = '5';
		  }
		}
		else if (num >= 20000000UL) {
		  if (num >= 40000000UL) {
		    num -= 40000000UL;
		    *buf++ = '4';
		  }
		  else if (num >= 30000000UL) {
		    num -= 30000000UL;
		    *buf++ = '3';
		  }
		  else {
		    num -= 20000000UL;
		    *buf++ = '2';
		  }
		}
		else if (num >= 10000000UL) {
		  num -= 10000000UL;
		  *buf++ = '1';
		}
		else *buf++ = '0';
	      }
	      if (num >= 5000000UL) {
		if (num >= 8000000UL) {
		  if (num >= 9000000UL) {
		    num -= 9000000UL;
		    *buf++ = '9';
		  }
		  else {
		    num -= 8000000UL;
		    *buf++ = '8';
		  }
		}
		else if (num >= 6000000UL) {
		  if (num >= 7000000UL) {
		    num -= 7000000UL;
		    *buf++ = '7';
		  }
		  else {
		    num -= 6000000UL;
		    *buf++ = '6';
		  }
		}
		else {
		  num -= 5000000UL;
		  *buf++ = '5';
		}
	      }
	      else if (num >= 2000000UL) {
		if (num >= 4000000UL) {
		  num -= 4000000UL;
		  *buf++ = '4';
		}
		else if (num >= 3000000UL) {
		  num -= 3000000UL;
		  *buf++ = '3';
		}
		else {
		  num -= 2000000UL;
		  *buf++ = '2';
		}
	      }
	      else if (num >= 1000000UL) {
		num -= 1000000UL;
		*buf++ = '1';
	      }
	      else *buf++ = '0';
	    }
	    if (num >= 500000UL) {
	      if (num >= 800000UL) {
		if (num >= 900000UL) {
		  num -= 900000UL;
		  *buf++ = '9';
		}
		else {
		  num -= 800000UL;
		  *buf++ = '8';
		}
	      }
	      else if (num >= 600000UL) {
		if (num >= 700000UL) {
		  num -= 700000UL;
		  *buf++ = '7';
		}
		else {
		  num -= 600000UL;
		  *buf++ = '6';
		}
	      }
	      else {
		num -= 500000UL;
		*buf++ = '5';
	      }
	    }
	    else if (num >= 200000UL) {
	      if (num >= 400000UL) {
		num -= 400000UL;
		*buf++ = '4';
	      }
	      else if (num >= 300000UL) {
		num -= 300000UL;
		*buf++ = '3';
	      }
	      else {
		num -= 200000UL;
		*buf++ = '2';
	      }
	    }
	    else if (num >= 100000UL) {
	      num -= 100000UL;
	      *buf++ = '1';
	    }
	    else *buf++ = '0';
	  }
	  if (num >= 50000UL) {
	    if (num >= 80000UL) {
	      if (num >= 90000UL) {
		num -= 90000UL;
		*buf++ = '9';
	      }
	      else {
		num -= 80000UL;
		*buf++ = '8';
	      }
	    }
	    else if (num >= 60000UL) {
	      if (num >= 70000UL) {
		num -= 70000UL;
		*buf++ = '7';
	      }
	      else {
		num -= 60000UL;
		*buf++ = '6';
	      }
	    }
	    else {
	      num -= 50000UL;
	      *buf++ = '5';
	    }
	  }
	  else if (num >= 20000UL) {
	    if (num >= 40000UL) {
	      num -= 40000UL;
	      *buf++ = '4';
	    }
	    else if (num >= 30000UL) {
	      num -= 30000UL;
	      *buf++ = '3';
	    }
	    else {
	      num -= 20000UL;
	      *buf++ = '2';
	    }
	  }
	  else if (num >= 10000UL) {
	    num -= 10000UL;
	    *buf++ = '1';
	  }
	  else *buf++ = '0';
	}
	if (num >= 5000UL) {
	  if (num >= 8000UL) {
	    if (num >= 9000UL) {
	      num -= 9000UL;
	      *buf++ = '9';
	    }
	    else {
	      num -= 8000UL;
	      *buf++ = '8';
	    }
	  }
	  else if (num >= 6000UL) {
	    if (num >= 7000UL) {
	      num -= 7000UL;
	      *buf++ = '7';
	    }
	    else {
	      num -= 6000UL;
	      *buf++ = '6';
	    }
	  }
	  else {
	    num -= 5000UL;
	    *buf++ = '5';
	  }
	}
	else if (num >= 2000UL) {
	  if (num >= 4000UL) {
	    num -= 4000UL;
	    *buf++ = '4';
	  }
	  else if (num >= 3000UL) {
	    num -= 3000UL;
	    *buf++ = '3';
	  }
	  else {
	    num -= 2000UL;
	    *buf++ = '2';
	  }
	}
	else if (num >= 1000UL) {
	  num -= 1000UL;
	  *buf++ = '1';
	}
	else *buf++ = '0';
      }
      if (num >= 500UL) {
	if (num >= 800UL) {
	  if (num >= 900UL) {
	    num -= 900UL;
	    *buf++ = '9';
	  }
	  else {
	    num -= 800UL;
	    *buf++ = '8';
	  }
	}
	else if (num >= 600UL) {
	  if (num >= 700UL) {
	    num -= 700UL;
	    *buf++ = '7';
	  }
	  else {
	    num -= 600UL;
	    *buf++ = '6';
	  }
	}
	else {
	  num -= 500UL;
	  *buf++ = '5';
	}
      }
      else if (num >= 200UL) {
	if (num >= 400UL) {
	  num -= 400UL;
	  *buf++ = '4';
	}
	else if (num >= 300UL) {
	  num -= 300UL;
	  *buf++ = '3';
	}
	else {
	  num -= 200UL;
	  *buf++ = '2';
	}
      }
      else if (num >= 100UL) {
	num -= 100UL;
	*buf++ = '1';
      }
      else *buf++ = '0';
    }
    if (num >= 50UL) {
      if (num >= 80UL) {
	if (num >= 90UL) {
	  num -= 90UL;
	  *buf++ = '9';
	}
	else {
	  num -= 80UL;
	  *buf++ = '8';
	}
      }
      else if (num >= 60UL) {
	if (num >= 70UL) {
	  num -= 70UL;
	  *buf++ = '7';
	}
	else {
	  num -= 60UL;
	  *buf++ = '6';
	}
      }
      else {
	num -= 50UL;
	*buf++ = '5';
      }
    }
    else if (num >= 20UL) {
      if (num >= 40UL) {
	num -= 40UL;
	*buf++ = '4';
      }
      else if (num >= 30UL) {
	num -= 30UL;
	*buf++ = '3';
      }
      else {
	num -= 20UL;
	*buf++ = '2';
      }
    }
    else if (num >= 10UL) {
      num -= 10UL;
      *buf++ = '1';
    }
    else *buf++ = '0';
  }
  if (num >= 5) {
    if (num >= 8) {
      if (num >= 9) {
	*buf++ = '9';
      }
      else {
	*buf++ = '8';
      }
    }
    else if (num >= 6) {
      if (num >= 7) {
	*buf++ = '7';
      }
      else {
	*buf++ = '6';
      }
    }
    else {
      *buf++ = '5';
    }
  }
  else if (num >= 2) {
    if (num >= 4) {
      *buf++ = '4';
    }
    else if (num >= 3) {
      *buf++ = '3';
    }
    else {
      *buf++ = '2';
    }
  }
  else if (num >= 1) {
    *buf++ = '1';
  }
  else *buf++ = '0';
  if (post_str) {
    while (*post_str) {
      *buf++ = *post_str++;
    }
  }
  bp->next_free_byte = buf;
#endif /* USE_SPRINTF_FOR_NUMS */
  }
  else complain_invalid_buffer();
}

/* The following function copies a string to the specified string
 * buffer, optionally enclosing it in quotes, and escaping embedded
 * quotes along the way.
 */

void add_quoted_str_to_string(const char* str,
			      a_boolean add_quotes,
			      string_buffer_ref bp) {
    const char* p = str;
    const char* quote;
    const char* q;
    size_t remaining_len;
    if (add_quotes) {
	add_1_char_to_string('\"', bp);
    }
    for (quote = strchr(p, '\"'); quote; quote = strchr(p, '\"')) {
	add_to_string_with_len(p, quote - p, bp);
	for (q = quote - 1; q > str && *q == '\\'; q--)
		{ }
	if ((quote - q) % 2) {
	    add_2_chars_to_string("\\\"", bp);
	}
	else add_1_char_to_string('\"', bp);
	p = quote + 1;
    }
    remaining_len = strlen(p);
    add_to_string_with_len(p, remaining_len, bp);
    p += remaining_len;
    q = p - 1;
    if (q >= str && *q == '\\') {
	while (q > str && *q == '\\') {
	    q--;
	}
	if ((p - q + 1) % 2) {
	    add_1_char_to_string('\\', bp);
	}
    }
    if (add_quotes) {
	add_1_char_to_string('\"', bp);
    }
}

/* The following function adds a symbol id to the string, using the
 * syntax required for the IF.
 */

void add_symid_to_string(unsigned long id, string_buffer_ref bp) {
#ifdef USE_SPRINTF_FOR_NUMS
    char buff[32];
    size_t len;
#ifdef __BSD__
    sprintf(buff, " [%lu] ", id);
    len = strlen(buff);
#else
    len = sprintf(buff, " [%lu] ", id);
#endif
    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
    add_num_to_string(id, " [", "] ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
}

/* The following function adds a position (line number, i.e., the
 * sequence number of the line in the cpp output, and column number)
 * to the string.
 */

static a_boolean add_mapped_file_pos_to_string(a_source_position_ptr pos,
					       string_buffer_ptr bp) {
    char buff[32];
    size_t len;
    a_boolean ret = FALSE;
    if (pos->seq) {
	unsigned long seq = pos->seq;
#ifdef XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS
	if (pos->mapped_column == 0 && pos->column != 0) {
	    /* This is a situation in which a zero-length expansion
	     * of a macro has placed the mapped position before the
	     * first character of the line.  Because the IF convention
	     * is that a column number of 0 indicates the end of the
	     * line, the line number needs to be decremented.
	     */
	    seq--;
	}
#ifdef USE_SPRINTF_FOR_NUMS
#ifdef __BSD__
	sprintf(buff, " %lu/%u ", seq, pos->mapped_column);
	len = strlen(buff);
#else
	len = sprintf(buff, " %lu/%u ", seq, pos->mapped_column);
#endif
	add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
	add_num_to_string(seq, " ", "/", bp);
	add_num_to_string(pos->mapped_column, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
#else /* XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS */
	add_num_to_string(seq, " ", "/", bp);
	add_num_to_string(pos->column, NULL, " ", bp);
#endif /* XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS */
       ret = TRUE;
    }
    else {
	add_1_char_to_string(' ', bp);
    }
    return ret;
}

/* The following function adds a position and length to the string. */

static a_boolean add_mapped_file_pos_and_len_to_string(a_source_position_ptr pos,
						       string_buffer_ref bp) {
    char buff[32];
    size_t len;
    a_boolean ret = FALSE;
    if (pos->seq) {
	unsigned long seq = pos->seq;
#ifdef XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS
	if (pos->mapped_column == 0 && pos->column != 0) {
	    /* This is a situation in which a zero-length expansion
	     * of a macro has placed the mapped position before the
	     * first character of the line.  Because the IF convention
	     * is that a column number of 0 indicates the end of the
	     * line, the line number needs to be decremented.
	     */
	    seq--;
	}
#ifdef USE_SPRINTF_FOR_NUM
#ifdef __BSD__
	sprintf(buff, " %lu/%u %d ", seq, pos->mapped_column,
		      pos->len);
	len = strlen(buff);
#else
	len = sprintf(buff, " %lu/%u %d ", seq, pos->mapped_column,
		      pos->len);
#endif
	add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUM) */
	add_num_to_string(seq, " ", "/", bp);
	add_num_to_string(pos->mapped_column, NULL, " ", bp);
	add_num_to_string(pos->len, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUM */
#else /* XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS */
	add_num_to_string(seq, " ", "/", bp);
	add_num_to_string(pos->column, NULL, " ", bp);
	add_num_to_string(pos->len, NULL, " ", bp);
#endif /* XXX_PRE_FULLY_RESOLVED_MACRO_POSITIONS */
        ret = TRUE;
    }
    else {
	add_1_char_to_string(' ', bp);
    }
    return ret;
}

static a_boolean is_precise_pos(a_source_position_ptr pos) {
    return (pos->precise_column != 0);
}

/* The following function adds the original source file position to
 * string. conv_seq_to_physical_file_and_line function is used to calculate
 * the line number in the original file.
 */

static a_boolean add_orig_file_pos_to_string(a_source_position_ptr pos,
					     string_buffer_ref bp,
					     string_buffer_ptr ckbuf,
                                             a_boolean add_dummy_column) {
    char buff[32];
    size_t len;
    a_boolean ret_value = FALSE;
    a_boolean valid_column = (pos) ? is_precise_pos(pos) : FALSE;
    if (pos->seq && bp && (valid_column || add_dummy_column)) {
        char *absolute_path = NULL;
	a_line_number   line_number;
        a_column_number column;
	a_boolean       at_end_of_source;
        a_source_file_ptr src_file;
        if (valid_column) {
            column = pos->precise_column;
	} else {
            /* If precise_column is not correct and add_dummy_column
               is true set column to a dummy value of 1. */
            column = 1;
	}
	conv_seq_to_physical_file_and_line(pos->seq, &src_file,
				           &line_number, &at_end_of_source);
        if (src_file && ckbuf != NULL && src_file->full_name) {
	    absolute_path = get_absolute_path(src_file->full_name, &trial_buf);
            convert_to_lower_case(absolute_path);
	}

	/* Check if the line number is being written to the correct file 
	 * by comparing the filename stored in intr_info_array and the 
         * file name returned by conv_seq_to_physical_file_and_line. Do not 
         * write position information if the file names do not match.
         * Note: full_name returned by conv_seq_to_physical_file_and_line may
         * be NULL, for example if there is "#line" statement, in such cases
         * write position information anyway.
	 */
        if (absolute_path == NULL ||
            strcmp(absolute_path, (get_intr_info_from_buffer(ckbuf))->filename) == 0) {
#ifdef USE_SPRINTF_FOR_NUMS
#ifdef __BSD__
	    sprintf(buff, " %lu/%u ", line_number, column);
	    len = strlen(buff);
#else
	    len = sprintf(buff, " %lu/%u ", line_number, column);
#endif
	    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
	    add_num_to_string((unsigned long)line_number, " ", "/", bp);
	    add_num_to_string(column, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
            ret_value = TRUE;
        }
        else {
            add_1_char_to_string(' ', bp);
#if DEBUG
	    if (db_line) {
	        complain_str(error_csev, "Line number does not belong to file $1", 
			     (get_intr_info_from_buffer(bp))->filename);
            }
#endif /* DEBUG */
        }
    }  
    else {
	add_1_char_to_string(' ', bp);
    }
    return ret_value;
}

/* The following function adds line number (in the source file) and 
 * length to the string. 
 */

static a_boolean add_orig_file_pos_and_len_to_string(a_source_position_ptr pos,
						     string_buffer_ref bp,
						     string_buffer_ptr ckbuf,
                                                     a_boolean add_dummy_column) {
    char buff[32];
    size_t len;
    a_boolean ret_value = FALSE;
    a_boolean valid_column = (pos) ? is_precise_pos(pos) : FALSE;
    if (pos->seq && bp && (valid_column || add_dummy_column)) {
        char *absolute_path = NULL;
        a_line_number   line_number;
        a_column_number column;
        short pos_len;
        a_boolean       at_end_of_source;
        a_source_file_ptr src_file;
        if (valid_column) {
            column = pos->precise_column;
            pos_len = pos->len;
	} else {
            /* If precise_column is not correct and add_dummy_column
               is true set column to a dummy value of 1. */
            column = 1;
            pos_len = 1;
	}
	conv_seq_to_physical_file_and_line(pos->seq, &src_file,
				           &line_number, &at_end_of_source);

        if (src_file && ckbuf != NULL && src_file->full_name) {
	    absolute_path = get_absolute_path(src_file->full_name, &trial_buf);
            convert_to_lower_case(absolute_path);
	}

	/* Check if the line number is being written to the correct file 
	 * by comparing the filename stored in intr_info_array and the 
         * file name returned by conv_seq_to_physical_file_and_line. Do not
         * write position information if the file names do not match.
         * Note: full_name returned by conv_seq_to_physical_file_and_line may
         * be NULL,for example if there is "#line" statement, in such cases
         * write position information anyway.
	 */
        if (absolute_path == NULL ||
            strcmp(absolute_path, (get_intr_info_from_buffer(ckbuf))->filename) == 0) {
#ifdef USE_SPRINTF_FOR_NUM
#ifdef __BSD__
	    sprintf(buff, " %lu/%u %d ", line_number, column,
		    pos_len);
	    len = strlen(buff);
#else
	    len = sprintf(buff, " %lu/%u %d ", line_number, column,
	                  pos_len);
#endif
	    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUM) */
	    add_num_to_string((unsigned long)line_number, " ", "/", bp);
	    add_num_to_string(column, NULL, " ", bp);
	    add_num_to_string(pos_len, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUM */
            ret_value = TRUE;
        }
        else {
            add_1_char_to_string(' ', bp);
#if DEBUG
	    if (db_line) {
	        complain_str(error_csev, "Line number does not belong to file $1", 
			     (get_intr_info_from_buffer(bp))->filename);
            }
#endif /* DEBUG */
        }
    }
    else {
	add_1_char_to_string(' ', bp);
    }
    return ret_value;
}

/* The following function calls either add_orig_file_pos_to_string or
 * add_mapped_file_pos_to_string to add position (line number) with
 * respect to the original file or the mapped (cpp output) file. 
 * If we're generating multiple IFs and the string buffer is for
 * one of the IF files, then the text representation of position will be
 * generated only if pos is within the corresponding source file.
 *
 * The return value indicates whether a position was written.
 *
 * In any case, at least one space character is written to the buffer,
 * and the first and last characters written are spaces.
 */

a_boolean add_pos_to_string(a_source_position_ptr pos,
			    string_buffer_ref bp) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_to_string(pos, bp,
					     bp->output_filename != NULL ? bp : NULL, 
                                             FALSE /*add_dummy_column*/) :
                                          add_mapped_file_pos_to_string(pos, bp);
}

/* Like add_pos_to_string, but optionally specifies a buffer into
 * which the IF is going.  If we're generating multiple IFs and ckbuf
 * is not null, then the text representation of position will be
 * generated only if pos is within the source file associated with ckbuf.
 */

a_boolean add_pos_to_string_checking(a_source_position_ptr pos,
				     string_buffer_ref bp,
				     string_buffer_ptr ckbuf) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_to_string(pos, bp, ckbuf, FALSE /*add_dummy_column*/) :
                                          add_mapped_file_pos_to_string(pos, bp);
}

/* Like add_pos_to_string, but marks add_dummy_column flag TRUE so that 
 * line number and a dummy column number are added if the column value 
 * cannot be calculated precisely.
 */

a_boolean add_sym_def_pos_to_string(a_source_position_ptr pos,
			            string_buffer_ref bp) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_to_string(pos, bp,
					     bp->output_filename != NULL ? bp : NULL, 
                                             TRUE /*add_dummy_column*/) :
                                          add_mapped_file_pos_to_string(pos, bp);
}

/* Similar to add_pos_to_string, adds position and length to the 
 * string.
 */

a_boolean add_pos_and_len_to_string(a_source_position_ptr pos,
				    string_buffer_ref bp) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_and_len_to_string(pos, bp,
					     bp->output_filename != NULL ? bp : NULL, 
                                             FALSE /*add_dummy_column*/) :
                                          add_mapped_file_pos_and_len_to_string(pos, bp);
}

/* Similar to add_pos_to_string_checking, adds position and length to the 
 * string.
 */

a_boolean add_pos_and_len_to_string_checking(a_source_position_ptr pos,
					     string_buffer_ref bp,
					     string_buffer_ptr ckbuf) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_and_len_to_string(pos, bp, ckbuf, FALSE /*add_dummy_column*/) :
                                          add_mapped_file_pos_and_len_to_string(pos, bp);
}

/* Similar to add_sym_def_pos_to_string, adds position and length to the 
 * string.
 */

a_boolean add_sym_def_pos_and_len_to_string(a_source_position_ptr pos,
				            string_buffer_ref bp) {
    return (generate_multiple_IF_files) ? add_orig_file_pos_and_len_to_string(pos, bp,
					     bp->output_filename != NULL ? bp : NULL, 
                                             TRUE /*add_dummy_column*/) :
                                          add_mapped_file_pos_and_len_to_string(pos, bp);
}



/* The following function adds either a start and end position to
 * the string or a start position and len if the end position is
 * invalid (at the time of this writing there are some bugs in the
 * EDG position reporting that cause ending positions to be omitted).
 */

static a_boolean add_range_to_string(a_source_range* range,
				     string_buffer_ref bp) {
    a_boolean ret = FALSE;
    if (range->end.seq) {
	if ((ret = add_pos_to_string(&range->start, bp))) {
	    ret = add_pos_to_string(&range->end, bp);
        }
    }
    else ret = add_pos_and_len_to_string(&range->start, bp);
    return ret;
}

/* The following function adds a file-relative position (as opposed
 * to the normal add_pos_to_string, which uses the mapped line and
 * column position).  NOTE: it is legitimate for pos->seq to be
 * start_seq - 1.  This case should be handled by unsigned no-overflow
 * wrapping.
 */

a_boolean add_file_rel_pos_to_string(a_source_position_ptr pos,
				     a_seq_number start_seq,
				     unsigned long start_lineno,
				     string_buffer_ref bp) {
    char buff[32];
    size_t len;
    a_boolean ret = FALSE;
    if (pos->seq) {
#ifdef USE_SPRINTF_FOR_NUMS
#ifdef __BSD__
	sprintf(buff, " %lu/%u ", pos->seq - start_seq + start_lineno,
		pos->column);
	len = strlen(buff);
#else
	len = sprintf(buff, " %lu/%u ", pos->seq - start_seq +
		      start_lineno, pos->column);
#endif
	add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
	add_num_to_string(pos->seq - start_seq + start_lineno, " ",
			  "/", bp);
	add_num_to_string(pos->column, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
        ret = TRUE;
    }
    return ret;
}

/* The following function adds a file-relative position (as opposed
 * to the normal add_pos_to_string, which uses the mapped line and
 * column position), along with its length.
 */

a_boolean add_file_rel_pos_and_len_to_string(a_source_position_ptr pos,
					     a_seq_number start_seq,
					     unsigned long start_lineno,
					     string_buffer_ref bp) {
    char buff[32];
    size_t len;
    a_boolean ret = FALSE;
    if (pos->seq) {
#ifdef USE_SPRINTF_FOR_NUMS
#ifdef __BSD__
	sprintf(buff, " %lu/%u %d ", pos->seq - start_seq + start_lineno,
	    pos->column, pos->len);
	len = strlen(buff);
#else
	len = sprintf(buff, " %lu/%u %d ", pos->seq - start_seq +
		      start_lineno, pos->column, pos->len);
#endif
	add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
	add_num_to_string(pos->seq - start_seq + start_lineno, " ",
		       "/", bp);
	add_num_to_string(pos->column, NULL, " ", bp);
	add_num_to_string(pos->len, NULL, " ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
        ret = TRUE;
    }
    return ret;
}

/* The following function terminates the current string as
 * appropriate, moves the current string pointer following the
 * termination character, resets the current length to zero, and
 * returns a pointer to the just-terminated string.
 */

const char* terminate_string(string_buffer_ref bp) {
    char* old_str;
    char term = bp->string_terminator;
    if (term != '\r') {
	add_1_char_to_string(term, bp);
    }
    else {
	add_2_chars_to_string("\r\n", bp);
    }
    old_str = bp->cur_str;
    bp->cur_str = bp->next_free_byte;
#if DEBUG
    if (db_sync && bp == &output_buf) {
	handle_output_overflow(&output_buf);	/* flush */
    }
#endif
    return old_str;
}

/* The following function adds "SMT language" to string.
 *
 */
void add_SMT_language_to_string(string_buffer_ref output_buffer)
{
   if (output_buffer != NULL) {
       if (il_header.source_language == sl_Cplusplus) {
	   add_to_string("SMT language \"c++\"", output_buffer);
       }
       else add_to_string("SMT language \"c\"", output_buffer);
       terminate_string(output_buffer);
   }
}

/* @@@ */

/* The following variables implement a stack used for numbering blocks
 * in a function.  Each "{" increments num_blocks, and the stack
 * allows going back to the containing block when a "}" is seen.
 * (Actually, it is IL scopes that are tracked; blocks with no
 * declarations in them are ignored.)  The "push" operation is handled
 * by notify_entering_scope and the "pop" is done by the iek_scope
 * case in process_entry.
 */

static int num_blocks;
static int block_depth;
static int *block_stack;
#define INIT_BLOCK_STACK_SIZE 500
static int block_stack_size;

int *block_stack_top(void) {
    if (block_stack && block_depth) {
        return block_stack + (block_depth - 1);
    }
    else {
	return NULL;
    }
}

/* The following function pushes a new block onto the stack. */

static void push_block(void) {
    if (!block_stack) {
	block_stack_size = INIT_BLOCK_STACK_SIZE;
	block_stack = (int*) malloc(block_stack_size * sizeof(int));
	block_stack_meter += INIT_BLOCK_STACK_SIZE;
    }
    else if (block_depth >= block_stack_size) {
	block_stack_size *= 2;
	block_stack = (int*) realloc(block_stack, block_stack_size);
	block_stack_reallocs++;
	block_stack_meter += block_stack_size;
    }
    block_stack[block_depth++] = ++num_blocks;
}

/* The following function pops a block from the stack. */

void pop_block(void) {
    if (block_stack && block_depth) {
	block_depth--;
    }
    else complain(error_csev, "Too many blocks popped from stack.");
}

/* The following variable gives the name of the current function when
 * inside a function scope, to allow for decoration of the names of
 * local entities with the function name.  It is set (using the
 * extensible string facilities above) on entry into a function scope;
 * it is zeroed when the function scope is encountered by
 * the IL walk (i.e., when the function scope is exited).
 */

const char* fcn_name;

/* The following variable is the numeric identifier of the current
 * function, if any (0 otherwise); it is used to create the relations
 * resulting from references encountered in the IL walk for this
 * function.
 */

unsigned long fcn_id;

/* The following variables are used to link parameter declarations and
 * the outer block of a function definition to the appropriate places
 * in the ast.
 */

static ast_node_info_ptr* fcn_parm_link;
ast_node_info_ptr fcn_declarator;

/* The following variable gives the routine associated with the
 * fcn_name and fcn_id above.
 */

a_routine_ptr curr_fcn;

/* The following variable is the scope associated with the current
 * function.
 */

a_scope_ptr curr_fcn_scope;

/* The following variable is set by notify_entering_scope and unset
 * by the scope IL entry processing in process_entry to allow the
 * suppression of location information when processing a template
 * function instance.
 */

a_boolean in_template_function;

/* The following function is called to process items that were
 * declared inside a function but were "exported" to the global
 * scope because of EDG's memory management requirements.  It
 * calls itself to process nested types inside local classes.
 */

a_source_sequence_entry_ptr orphaned_list_parent_seq_list;

static void process_local_scope(a_scope_ptr scope) {
    a_type_ptr type;
    a_constant_ptr constant;
    a_variable_ptr var;
    a_source_sequence_entry_ptr saved_parent_list =
	    orphaned_list_parent_seq_list;
    if (scope->source_sequence_list) {
	orphaned_list_parent_seq_list = scope->source_sequence_list;
    }
    for (type = scope->types; type; type = type->next) {
	process_entry((char*) type, iek_type);
	if (type->kind == tk_integer &&
	    type->variant.integer.enum_type) {
	    a_constant_ptr enumerator;
	    for (enumerator = type->variant.integer.enum_info.
		 constant_list; enumerator; enumerator =
		 enumerator->next) {
		process_entry((char*) enumerator, iek_constant);
	    }
	}
	else if (type->kind == tk_class || type->kind == tk_struct ||
		 type->kind == tk_union) {
	    if (type->variant.class_struct_union.extra_info &&
		type->variant.class_struct_union.extra_info->assoc_scope) {
		process_local_scope(type->variant.class_struct_union.
				    extra_info->assoc_scope);
	    }
	}
    }
    for (constant = scope->constants; constant;
	 constant = constant->next) {
	process_entry((char*) constant, iek_constant);
    }
    for (var = scope->variables; var; var = var->next) {
	process_entry((char*) var, iek_variable);
    }
    orphaned_list_parent_seq_list = saved_parent_list;
}

/* The following function is called whenever a scope node is
 * encountered during the IL walk (at the beginning, before walking
 * down to enclosed declarations, statements, etc.).  It is
 * responsible for maintaining the scope_no and fcn_name variables
 * above.
 */

void notify_entering_scope(a_scope_ptr ptr) {
    /* do nothing if not during SET IF dump */
    if (doing_IF_dump) {
	a_template_ptr t;
	if (ptr->kind == sck_function) {
	    if (ptr->variant.routine.ptr->is_trivial_default_constructor) {
		/* ignore */
		return;
	    }
	    curr_fcn = ptr->variant.routine.ptr;
	    curr_fcn_scope = ptr;
	    if (curr_fcn) {
	        add_fcn_name_to_string(curr_fcn, &string_buf);
		fcn_name = terminate_string(&string_buf);
		if (!il_entry_prefix_of(curr_fcn).SET_symid) {
		    /* Wasn't processed during traversal of global scope;
		     * probably there was a source error that prevented
		     * its entry there.  We need the symid and AST, though;
		     * process it now.
		     */
		    walk_entry_and_subtree_for_IF((char*) curr_fcn, iek_routine);
		}
		fcn_id = SET_symid_of(curr_fcn, iek_routine);
		ast_notify_entering_scope(ptr);
		in_template_function = (curr_fcn->is_template_function &&
					!curr_fcn->is_specialized);
	    }
	    if (block_depth) {
		complain(error_csev, "Block stack not empty on entering function scope.");
	    }
	    block_depth = 0;
	    num_blocks = 0;
	}
	else {
            int *bstp;
            push_block();
	    bstp = block_stack_top();
	    if (bstp != NULL) {
		ptr->if_id = *bstp;
	    }
	}
	if (ptr->kind == sck_function || ptr->kind == sck_block) {
	    /* Local types and constants were not processed when they were
	     * encountered in the file scope because we can't decorate the
	     * names appropriately.  Now that we're inside the scope, we need
	     * to go back and process all the deferred types and constants.
	     */
	    process_local_scope(ptr);
	}
    }
}

/* The following function is called whenever an orphaned list header
 * is encountered.  It sets the orphaned_list_parent_seq_list to the
 * source sequence list of the associated function so that individual
 * src seq sublists can find the place in the actual source sequence
 * list where the sublist should be plugged in.  (This is used in
 * constructing "<unnamed # xxx>" names for local tagless types.)
 */

void notify_entering_orphaned_list(a_scope_orphaned_list_header_ptr olhp) {
    a_routine_ptr fcn = olhp->assoc_routine;
    if (fcn && fcn->assoc_scope != NULL_region_number) {
	a_scope_ptr scope_ptr =
		il_header.region_scope_entry[fcn->assoc_scope];
	if (scope_ptr) {
	    orphaned_list_parent_seq_list = scope_ptr->source_sequence_list;
	}
    }
}

/* The following function is called whenever a source sequence sublist
 * is entered, before the first source sequence entry that is part of
 * the sublist is processed.  It sets the in_src_seq_sublist variable
 * to TRUE to cause the members of the sublist to be ignored so they
 * can be processed properly when processing the function scope source
 * sequence list to which they logically belong.  It is turned off
 * when the source sequence sublist entry is encountered during the
 * walk (after all its members have been traversed, due to the
 * bottom-up nature of the walk).
 */

a_boolean in_src_seq_sublist;

void notify_entering_src_seq_sublist(a_src_seq_sublist_ptr sublist) {
    if (interesting_node(sublist)) {
	in_src_seq_sublist = TRUE;
    }
}

#define NUM_BITS (sizeof(unsigned char)*8)

/* The following function sets the bit of outbuf_id in SET_outbuf_bit_arr.
 */

a_boolean set_output_buffer_bit(unsigned char **outbuf_bit_arr_ptr, 
			        string_buffer_ref outbuf)
{
  unsigned int byte_number, bit_number;
  int sz = 0;
  a_boolean initialized = FALSE;
  unsigned int outbuf_id = outbuf->id;

  /* Initialize bit array */
  if (!*outbuf_bit_arr_ptr) {
    sz = ceil((double)intr_info_count/NUM_BITS);
    *outbuf_bit_arr_ptr = (unsigned char*)malloc(sz * sizeof(unsigned char));
    memzero(*outbuf_bit_arr_ptr, sz * sizeof(unsigned char));
    initialized = TRUE;
  }
  
  byte_number = floor((double)outbuf_id/NUM_BITS);
  bit_number = outbuf_id%NUM_BITS;
  (*outbuf_bit_arr_ptr)[byte_number] |= (1 << bit_number);

  return initialized;
}

/* The following function is used to see if the bit in SET_outbuf_bit_arr
 * is set for the given outbuf_id. If the bit is set it implies that the
 * SYM line has already been written to this buffer.
 */

a_boolean is_output_buffer_bit_set(unsigned char **outbuf_bit_arr_ptr, 
				   string_buffer_ref outbuf)
{
  unsigned int outbuf_id = outbuf->id;
  a_boolean retval = FALSE;
  unsigned int byte_number, bit_number;

  if (outbuf_bit_arr_ptr && *outbuf_bit_arr_ptr) {
    byte_number = floor((double)outbuf_id/NUM_BITS);
    bit_number = outbuf_id%NUM_BITS;
    retval = ((*outbuf_bit_arr_ptr)[byte_number] & (1 << bit_number)) ? TRUE : FALSE;
  }
  return retval;
}

void set_outbuf_bit_of_clique(void *ilp, string_buffer_ref outbuf)
{ 
  an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
  a_boolean initialized = set_output_buffer_bit(&ep->SET_outbuf_bit_arr, outbuf);

  /* If SET_outbuf_bit_arr is initialized propagate it through out all the
     members of the clique. */
  if (initialized) {
    an_il_entry_prefix_ptr cep;
    void *cilp;
    unsigned char *outbufp = ep->SET_outbuf_bit_arr;
    for (cilp = ilp;; cilp = cep->SET_clique) {
      cep = &il_entry_prefix_of(cilp);
      /* free existing SET_outbuf_bit_arr */
      if (cep->SET_outbuf_bit_arr && cep->SET_outbuf_bit_arr != outbufp) {
        free(cep->SET_outbuf_bit_arr);
      }
      cep->SET_outbuf_bit_arr = outbufp;
      if (cep->SET_clique == ilp) {
        break;
      }
    }
  }
}

/* @@@ */

/* The following function is called to adjust the output file/buffer of a 
 *  variable. The variable might have a non-definition declaration occuring
 * in the file it points to. This is changed to point to the definition file.
 */

a_boolean adjust_variable_output_buffer(a_variable_ptr variable_ptr)
{
    a_boolean changed = FALSE;
    if (variable_ptr->source_corresp.decl_position.seq) {
        intr_info_ptr def_intr_info = get_intr_info_from_source_seq(variable_ptr->source_corresp.decl_position.seq);
        if (il_entry_prefix_of(variable_ptr).full_SET_dump != def_intr_info) {
	   il_entry_prefix_of(variable_ptr).full_SET_dump = def_intr_info;
	   changed = TRUE;
	}
    }
    else if (fcn_name && curr_fcn) {
        /* adjust output_buffer associated with local variables */
        if (il_entry_prefix_of(variable_ptr).full_SET_dump != il_entry_prefix_of(curr_fcn).full_SET_dump) {
	   il_entry_prefix_of(variable_ptr).full_SET_dump = il_entry_prefix_of(curr_fcn).full_SET_dump;
	   changed = TRUE;
        }
    }
    return changed;
}

/* The following function is called to adjust the output file/buffer, 
 * pointing to a non-definition declaration file instead of the definition 
 * file, of an il node for a routine.
 */

a_boolean adjust_routine_output_buffer(a_routine_ptr routine_ptr)
{
    a_boolean changed = FALSE;
    if (routine_ptr->assoc_scope != NULL_region_number && 
        !routine_ptr->compiler_generated) {
	a_scope_ptr scope = il_header.region_scope_entry[routine_ptr->assoc_scope];
        if (scope) {
	    intr_info_ptr def_intr_info = get_intr_info_from_il_node(scope);
            if (il_entry_prefix_of(routine_ptr).full_SET_dump != def_intr_info) {
	       il_entry_prefix_of(routine_ptr).full_SET_dump = def_intr_info;
	       changed = TRUE;
	   }
        }
    }
    return changed;
}

/* The following function is called to adjust the output file/buffer,
 * pointing to a non-definition declaration file instead of the
 * definition file, of an il node for a class/struct/union.
 */

a_boolean adjust_class_struct_union_output_buffer(a_type_ptr 
						  type_ptr)
{
    a_boolean changed = FALSE;
    if (type_ptr->size != 0 && type_ptr->source_corresp.decl_position.seq) {
        intr_info_ptr def_intr_info = get_intr_info_from_source_seq(type_ptr->source_corresp.decl_position.seq);
	if (il_entry_prefix_of(type_ptr).full_SET_dump != def_intr_info){
	    il_entry_prefix_of(type_ptr).full_SET_dump = def_intr_info;   
	    changed = TRUE;
	}
    }
    return changed;
}

/* The following variable is set to control whether IL entities created
 * during the processing of a given file will be fully dumped or only
 * in summary fashion and only if referenced from an "interesting" file.
 * It is used to set the "full_SET_dump" bit in the il_entry_prefix of
 * each node created while that file is current.
 */

intr_info_ptr SET_file_is_interesting = NULL;

intr_info_ptr is_file_listed_explicitly(const char*, a_boolean);
intr_info_ptr is_file_interesting(const char* , a_boolean);

/* The following function will eventually check the list of project
 * files to determine whether a file is interesting (part of a project)
 * or not (foreign file).  Currently it just checks the
 * exclude_headers_from_IF flag and returns the appropriate value
 * based on whether the file is a header or not.
 */

/* A file can be interesting in 3 ways:
 * 1. If "multiple_IF_files" option is specficied all files encountered 
 *    during parsing are considered to be interesting unless 
 *    there is already an IF file existing. A separate IF file is
 *    generated for each source and header file.
 * 2. If "intr_file" or "intr_files_list" command line options 
 *    specify the interesting file names explicilty. The resulting IF 
 *    file has info for the files specified only.
 * 3. If none of the above mentioned options are present, all files are 
 *    marked interesting. The resulting IF file has info for all the 
 *    files. 
 */

intr_info_ptr SET_is_file_interesting(const char* filename, a_boolean is_header) 
{
  intr_info_ptr retval = NULL;
  if (filename) {
    if (generate_multiple_IF_files) {
      retval = is_file_interesting(filename, is_header);
    }
    else {
      retval = is_file_listed_explicitly(filename, is_header);
    }
  }
  else {
    /* If filename is empty (example: for a "#line") return the 
       value of the current file. */
    retval = SET_file_is_interesting;
  }
    
  return retval;
}

void* is_in_interesting_files_table(const char *);
/* This function decides if a file is interesting or not. For a source 
 * file it is always true(interesting). For an include file the way it 
 * decides is by checking the existence of the IF file. If an IF file 
 * already exists it is marked as "not interesting" otherwise it is 
 * interesting.
 */ 

intr_info_ptr is_file_interesting(const char* filename, a_boolean is_header)
{
  a_boolean flag = TRUE;
  char* absolute_path;
  char if_filename[MAX_FILENAME_SIZE];
  intr_info_ptr intrp = NULL;

  /* Initailize the table */
  if (!interesting_file_table) {
    init_file_table(&interesting_file_table);  
  }

  if (filename) {
    absolute_path = get_absolute_path(filename, &trial_buf);
    convert_to_lower_case(absolute_path);

    intrp = (intr_info_ptr)is_in_interesting_files_table(absolute_path);
    if (!intrp && extract_IF_file_for(absolute_path, if_filename, is_header)) {        	
	/* create new intr_info structure and add it to the table */
	intrp = create_new_intr_info(absolute_path, if_filename, is_header);
	add_to_file_table(interesting_file_table, intrp);

        /* If this is the primary source file assign the newly created 
           intr_info to primary_source_intr_info */
	if (!primary_source_intr_info && !is_header) {
	  primary_source_intr_info = intrp;
	}

        /* Add IF file to the generated IF files file */
        add_to_generated_IF_files_file(if_filename);
    }
  }
  return intrp;
}

/* This function is used to read the filenames from both the command line
 * and the list of interesting files, and insert them into the file table
 * that will be consulted by the function "SET_is_interesting_file()" to
 * determine whether or not to dump IF for that file.
 */

static int set_up_interesting_files_table(void)
{
  int   retval = -1;
  char  buffer[MAX_FILENAME_SIZE];
  FILE *interesting_files_file;
  char *intr_file;

  if ( interesting_file_table )
    destroy_file_table(&interesting_file_table);

  init_file_table(&interesting_file_table);
  retval = 0;

  if ( interesting_file_name )
    {
      add_to_file_table(interesting_file_table, interesting_file_name);
      retval++;
    }
  
  if ( interesting_files_file_name )
    {
      interesting_files_file = fopen(interesting_files_file_name, "r");
      if ( interesting_files_file )
	{
	  while ( fgets(buffer, MAX_FILENAME_SIZE, interesting_files_file) == buffer )
	    if ( *buffer )
	      {
		/* remove the newline at the end of the file */
		buffer[strlen(buffer)-1] = '\0';

		/* add to the table of interesting files */
		intr_file = strdup(buffer);
		add_to_file_table(interesting_file_table, intr_file);
		retval++;
	      }
	  
	  if ( ! feof(interesting_files_file) )
	    complain_str(catastrophe_csev, "Read error on list of interesting files $1", interesting_files_file_name);	    

	  if ( fclose(interesting_files_file) != 0 )
	    complain_str(error_csev, "Cannot close list of interesting files $1", interesting_files_file_name);	    
	}
      else
	complain_str(catastrophe_csev, "Cannot open list of interesting files $1", interesting_files_file_name);
    }

  return retval;
}

intr_info_ptr is_file_listed_explicitly(const char *filename, a_boolean is_header)
{
  a_boolean val = TRUE;
  static intr_info one_intr_info = {&output_buf, NULL, NULL, NULL, NULL};
  static a_boolean first = TRUE;

  if (first) {
    add_new_intr_info_to_arr(&one_intr_info);
    first = FALSE;
  }

  if ( filename )
    {
      if ( ! interesting_file_table && ( interesting_file_name || interesting_files_file_name ) )
	{
	  /* Here we want to set up the table, as it has not been created yet. */
	  set_up_interesting_files_table();
	}
      
      if ( interesting_file_table )
	{
	  /* Here we want to check to see if filename is in the interesting file table */
	  val = (is_in_interesting_files_table(filename)) ? TRUE : FALSE;
	}
      else
	{
	  /* if no specific files are specified as interesting, all are interesting */
	  val = TRUE;
	}
    }

  return (val) ? &one_intr_info : NULL;
}

/*
 *
 *
 *
 *
 *
 *
 */

void* is_in_interesting_files_table(const char *filename)
{
  int        i;
  void*  retval = NULL;

  if ( interesting_file_table && filename )
    retval = in_file_table(interesting_file_table, filename);

  return retval;
}

const char* get_filename_from_hash_bucket(void *intrinfo)
{
  const char *ret_ptr = NULL;
  if (intrinfo) {
    if (generate_multiple_IF_files) {
      ret_ptr = ((intr_info_ptr)intrinfo)->filename;
    }
    else ret_ptr = (char *)intrinfo;
  }
  return ret_ptr;  
}


/* The following macros are used to get some file/directory properties  
 * 
 */

#ifdef _WIN32
#define is_slash(f) (((f) == '/') || ((f) == '\\'))
#define has_drive_letter(f)  (isalpha((f)[0]) && ((f)[1] == ':'))
#define is_absolute_path(f)  (has_drive_letter(f) ||\
			     (is_slash((f)[0]) && is_slash((f)[1])))
#define is_missing_drive_letter_only(f) (is_slash((f)[0]))
#else
#define is_absolute_path(f) (*(f) == '/')
#define is_missing_drive_letter_only(f) (0)
#define has_drive_letter(f)  (0)
#endif

/* The following function returns its argument, converted into an
 * absolute path if necessary.  If the argument is already an
 * absolute path (per the preceding macro), the result is just the
 * argument; otherwise, the function constructs an absolute path
 * (in the trial_buf) using the relative path base passed in as a
 * command line argument.
 */

extern int remove_all_dots_and_double_slashes(const char* name,
					      char* canonical_name);

static char canonical_name[MAX_FILENAME_SIZE];

char* get_absolute_path(const char* filename, string_buffer_ref bp) {
    if (filename == NULL) {
        return NULL;
    }
    if (!is_absolute_path(filename)) {
      if(is_missing_drive_letter_only(filename)) {
	if( has_drive_letter(relative_path_base) ){
	  add_1_char_to_string(relative_path_base[0], &trial_buf);
	  add_1_char_to_string(relative_path_base[1], &trial_buf);
	  add_to_string(filename, &trial_buf);
	  filename = terminate_string(&trial_buf);
	}
      } else {
	add_to_string(relative_path_base, &trial_buf);
	add_1_char_to_string('/', &trial_buf);
	add_to_string(filename, &trial_buf);
	filename = terminate_string(&trial_buf);
      }
    }
    if (remove_all_dots_and_double_slashes(filename, canonical_name) == 0) {
	add_to_string(canonical_name, bp);
    }
    else add_to_string(filename, bp);
    return (char *)terminate_string(bp);
}

const char *absolute_path_of(const char* name)
{
    return get_absolute_path(name, &string_buf);
}

void reset_meters_after_preprocessing(void)
{
    pp_string_buffer_meter = string_buffer_meter;
    string_buffer_meter = 0;
}

/* The following function is called in response to the command line
 * option --SET_memory_metering; it shows the memory usage of all the
 * data structures managed in this file.
 */

void dump_memory_meters(void) {
    fprintf(stderr, "string_buffer_reallocs =\t%10lu\n", (unsigned long)string_buffer_reallocs);
    fprintf(stderr, "string_buffer_meter =\t\t%10lu\n", (unsigned long)string_buffer_meter);
    fprintf(stderr, "pp_string_buffer_meter =\t%10lu\n", (unsigned long)pp_string_buffer_meter);
    fprintf(stderr, "block_stack_reallocs =\t\t%10lu\n", (unsigned long)block_stack_reallocs);
    fprintf(stderr, "block_stack_meter =\t\t%10lu\n", (unsigned long)block_stack_meter);
    dump_ast_memory_meters();
    dump_symbol_memory_meters();
    dump_pp_memory_meters();
}

/* The following function is called to initialize any data structures
 * required during the parse before dump_SET_IF is called (currently
 * just the string buffers).
 */

void init_SET_data(void) {
    init_string_buffer(&trial_buf);
    init_string_buffer(&string_buf);
    if (!generate_multiple_IF_files) {
      init_string_buffer(&output_buf);
    }
}
