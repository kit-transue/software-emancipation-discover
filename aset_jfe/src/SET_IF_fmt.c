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
#include <stddef.h>
typedef int a_boolean;
#define TRUE 1
#define FALSE 0
typedef void an_il_to_str_output_control_block;
#include "SET_strings.h"
#include "SET_IF_fmt.h"

/* The following variables implement a stack used for numbering blocks
 * in a function.  Each "{" increments num_blocks, and the stack
 * allows going back to the containing block when a "}" is seen.
 */

static int num_blocks;
static int block_depth;
static int* block_stack;
#define INIT_BLOCK_STACK_SIZE 500
static int block_stack_size;

/* Prepare to push a number. */
static void prepare_block_stack () {
    if (!block_stack) {
	block_stack_size = INIT_BLOCK_STACK_SIZE;
	block_stack = (int*) malloc(block_stack_size * sizeof(int));
    }
    else if (block_depth >= block_stack_size) {
	block_stack_size *= 2;
	block_stack = (int*) realloc(block_stack, block_stack_size * sizeof(int));
    }
}

/* The following function pushes a new block onto the stack. */

void SET_push_block() {
    prepare_block_stack();
    block_stack[block_depth++] = ++num_blocks;
}

/* Push a block numbered 0 onto the stack. Subsequent nested blocks will
 * be counted starting with 1.
 */
void SET_push_main_block() {
    prepare_block_stack();
    block_stack[block_depth++] = num_blocks; /* saves the count */
    prepare_block_stack();
    block_stack[block_depth++] = (num_blocks = 0);
}

/* The following function pops a block from the stack. */

void SET_pop_block() {
    if (block_stack && block_depth) {
	if (block_stack[--block_depth] == 0 && block_depth) {
    	    /* Pop the saved count of the containing block. */
    	    num_blocks = block_stack[--block_depth];
	}
    }
}

int SET_get_block_num() {
    if (block_stack && block_depth) {
	return block_stack[block_depth - 1];
    }
    return 0;
}

/* The following variable gives the name of the current function when
 * inside a function scope, to allow for decoration of the names of
 * local entities with the function name.
 */

static const char* fcn_name;

/* The following function sets the function name */

void SET_set_fcn_name(const char* name) {
    fcn_name = name;
}

/* The following function returns the function name */

const char* SET_get_fcn_name() {
    return fcn_name;
}

/* The following function decorates the names of entities local to a
 * function with the function name and block number, if any; global
 * entities are left unchanged.  It then adds the closing quote.
 */

void decorate_name_and_finish(a_boolean add_block_qual,
			      a_boolean add_closing_quote) {
    if (fcn_name) {
	add_3_chars_to_string(" @ ", &output_buf);
	add_to_string(fcn_name, &output_buf);
	if (add_block_qual) {
	    add_num_to_string(SET_get_block_num(), " {",
			      "}", &output_buf);
	}
    }
    if (add_closing_quote) {
	add_1_char_to_string('\"', &output_buf);
    }
}

/* The following function creates an unterminated REL line (not
 * including any attributes on that line) in the output string_buffer.
 */

void format_rel(unsigned long first_symid,
		       const char* rel_name,
		       unsigned long second_symid) {
    add_3_chars_to_string("REL", &output_buf);
    add_symid_to_string(first_symid, &output_buf);
    add_1_char_to_string('\"', &output_buf);
    add_to_string(rel_name, &output_buf);
    add_1_char_to_string('\"', &output_buf);
    add_symid_to_string(second_symid, &output_buf);
}

/* The following function formats and terminates a REL line (without
 * attributes) in the output string_buffer.
 */

void write_rel(unsigned long first_symid,
		      const char* rel_name,
		      unsigned long second_symid) {
    format_rel(first_symid, rel_name, second_symid);
    terminate_string(&output_buf);
}

/* The following function formats an ATR line for the specified
 * symbol and attribute.
 */

void format_atr(unsigned long symid, const char* attrib) {
    add_3_chars_to_string("ATR", &output_buf);
    add_symid_to_string(symid, &output_buf);
    add_to_string(attrib, &output_buf);
}

/* The following function writes an ATR line to the IF file. */

void write_atr(unsigned long symid, const char* attrib) {
    format_atr(symid, attrib);
    terminate_string(&output_buf);
}

/* The following function is called to add an attribute to the specified
 * string_buffer.  The atr_added parameter is used to keep track of
 * whether a comma is needed before adding the string.
 */

void add_attrib(const char* attrib, a_boolean* atr_added,
		       string_buffer_ptr bp) {
    if (*atr_added) {
	add_2_chars_to_string(", ", bp);
    }
    add_to_string(attrib, bp);
    *atr_added = TRUE;
}

