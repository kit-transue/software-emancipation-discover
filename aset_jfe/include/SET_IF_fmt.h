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
#ifndef SET_IF_fmt_h
#define SET_IF_fmt_h

/* The following functions format various kinds of lines and parts of
 * lines in the IF file, using the output_buf declared in SET_strings.h.
 */

/* The following functions are used in formatting references to add
 * function and block qualification, e.g., "i @ f() {2}".
 */

int SET_get_block_num(void);
void SET_push_block(void);
void SET_push_main_block(void);
void SET_pop_block(void);
void SET_set_fcn_name(const char* name);
const char* SET_get_fcn_name(void);

/* The following function is called after beginning a SYM line (up
 * through the name, less function/block qualification, e.g.,
 *
 *		SYM [23] "xyz
 *
 * ) and adds the function/block qualification and closing quote.
 * It does NOT output the line.
 */

void decorate_name_and_finish(a_boolean add_block_qual,
			      a_boolean add_closing_quote);

/* The following functions create the left substring of a line or
 * output an entire line to the output_buf.
 */

void format_rel(unsigned long first_symid,
		const char* rel_name,
		unsigned long second_symid);
void write_rel(unsigned long first_symid,
	       const char* rel_name,
	       unsigned long second_symid);
void format_atr(unsigned long symid, const char* attrib);
void write_atr(unsigned long symid, const char* attrib);

/* The following function adds an attribute to a line already being
 * constructed (e.g., a REL line) and sets a boolean variable passed
 * in to indicate that an attribute was added.
 */

void add_attrib(const char* attrib, a_boolean* atr_added,
		string_buffer_ptr bp);
#endif
