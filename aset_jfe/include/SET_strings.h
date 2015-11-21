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
#ifndef SET_strings_h
#define SET_strings_h

/* -------------------- STRING MANAGEMENT --------------------
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
 * The ocb entry is used in the EDG C/C++ IF generation to parameterize
 * the routines in the EDG code that output names and constants,
 * causing this output to be placed in the associated string buffer.
 * It is not used in the EDG Java implementation.
 */

#define STRING_BUFFER_SIZE 500000

typedef struct string_buffer* string_buffer_ptr;

typedef struct string_buffer {
    char* buf;		/* pointer to beginning of buffer */
    size_t buf_size;	/* number of bytes that the buffer can hold */
    char* cur_str;	/* pointer to beginning of string under construction */
    char* next_free_byte; /* points to where to add new text in the buffer */
    char* last_byte_of_buf; /* points to end of buffer */
    void (*handle_overflow)(string_buffer_ptr);
    an_il_to_str_output_control_block* ocb;
    char string_terminator;
} string_buffer;

/* The following string buffers are provided automatically by the
 * string package.  "output_buf" is a buffer that is dumped to
 * IF_file when it fills up.  "string_buf" is a buffer that
 * maintains its contents indefinitely; when it fills up, a new
 * region of storage is allocated.  (Pointers to the individual
 * strings in the buffer are returned by terminate_string().)
 * "trial_buf" is a buffer in which to put scratch text; if it
 * is needed, it can be copied out from the pointer returned by
 * terminate_string().
 */

extern string_buffer output_buf;
extern string_buffer trial_buf;
extern string_buffer string_buf;

/* The following functions place text of various kinds into the
 * specified string buffer.  (The "N-char" versions are
 * optimizations of the add_to_string_with_len entry.)
 */

void add_to_string_with_len(const char* text, size_t len,
			    string_buffer_ptr bp);
void add_1_char_to_string(char ch, string_buffer_ptr bp);
void add_2_chars_to_string(const char* str,
			   string_buffer_ptr bp);
void add_3_chars_to_string(const char* str,
			   string_buffer_ptr bp);
void add_to_string(const char* text, string_buffer_ptr bp);
void add_num_to_string(unsigned long num, const char* pre_str,
		       const char* post_str,
		       string_buffer_ptr bp);
void add_quoted_str_to_string(const char* str,
			      a_boolean add_quotes,
			      string_buffer_ptr bp);
void add_symid_to_string(unsigned long id, string_buffer_ptr bp);

/* The following function terminates a string in the manner
 * appropriate to its buffer and returns a pointer to the
 * beginning of the string (i.e., the first character that was
 * added after the last string was terminated).
 */

const char* terminate_string(string_buffer_ptr bp);

/* The following function initializes all three of the provided
 * string buffers, including opening the output file specified by
 * the first argument.  It returns FALSE if the file could not be
 * opened, TRUE otherwise.
 */

a_boolean initialize_string_buffers(const char* output_filename,
				    an_il_to_str_output_control_block* output_ocb,
				    an_il_to_str_output_control_block* string_ocb,
				    an_il_to_str_output_control_block* trial_ocb);

/* The following function flushes the output_buf and closes the
 * associated file.
 */

void close_output_buf();

/* The following function adds a specified number of blanks to the
 * output buffer.
 */

void indent_to_depth(int depth);
#endif
