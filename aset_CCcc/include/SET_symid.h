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
/* assignment of symbol ids for use in the IF */

#ifndef SET_symid_h
#define SET_symid_h

#include "SET_names.h"

/* Counter used to assign symbol ids. */
extern unsigned long next_symid;

/* The unique id used to indicate an error symbol. 
 * Such symbols do not go into the IF. */
extern unsigned long error_symid;

/* Return the symid of an ellipsis parameter. */
extern unsigned long symid_of_ellipsis(string_buffer_ptr output_buffer);

/* Ensure_same_symid assigns the same symid to both entities,
 * complaining if both have already been assigned.
 */
void ensure_same_symid(void* entity1, an_il_entry_kind, void* entity2, an_il_entry_kind);

/* Same as ensure_same_symid, but with a specific message if
 * both have already been assigned. */
extern a_boolean attempt_same_symid(void* entity1, an_il_entry_kind kind1, void* entity2, an_il_entry_kind kind2,
                                    const char *message);

/* Assign an id to the given entry (and others in its clique).
 * May also write a SYM line in some minor cases. */
extern unsigned long assign_symid(void* ilp, an_il_entry_kind);

/* Assign the error symid to the given il entry. */
extern void assign_error_symid(void* ilp);

/* This trio of functions tracks that every symid emitted into
 * the IF is eventually satisfied by a SYM line defining the symid.
 * Not particularly useful in the generate_multiple_IF_files case
 * where the SYM line must precede all references. Call expect_SYM
 * when a symid is assigned, and fulfill_SYM when its SYM line goes out.*/
extern void expect_SYM(void *ep, an_il_entry_kind);
extern void fulfill_SYM(void *ilp);

/* Report if there are unfulfilled symids. */
extern void check_expected_SYMs(string_buffer_ptr output_buffer);

/* Assign an id to the given entry, and ensure that a SYM line
 * is written to the given output buffer. */
extern unsigned long SET_symid_and_write_sym(void* ilp, 
					     an_il_entry_kind il_kind,
					     string_buffer_ptr output_buffer);

/* Return a symbol id for the given entry, and mark the given AST node
 * so that if the AST node is written to an IF file, a SYM line
 * will also be written. */
extern unsigned long SET_symid_and_add_to_ast(void* ilp,
					      an_il_entry_kind il_kind,
					      ast_node_info_ptr astp);

/* The following macro extracts the SET_symid field from the entry
 * prefix of a supplied IL node, if any; if the IL node pointer is
 * NULL, the value of the macro is 0.  If the IL node exists but
 * the symid is 0, a new symid is allocated and assigned to it.
 *
 * In order to facilitate tracking whether SYM lines have been
 * emitted for all symids, call SET_symid_of only for entries which
 * will be mentioned in the IF.
 */

#define SET_symid_of(ilp, kind) ( \
	/* if */ (ilp) ? \
	    /* then if */ ((il_entry_prefix_of(ilp).SET_symid) ? \
		/* then */ (il_entry_prefix_of(ilp).SET_symid) : \
	    /* else */ assign_symid(ilp, kind)) : \
	/* else */ 0)


#endif /* SET_symid_h */
