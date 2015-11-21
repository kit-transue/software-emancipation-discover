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
/* The primary entry which is called from the
 * EDG front end to create the IF file.
 */

#ifndef SET_dump_if_h
#define SET_dump_if_h
#include <stdio.h>
#include "SET_names.h"

extern FILE* IF_file;

/* The following flag is TRUE during the period of the IF dump and
 * FALSE otherwise.
 */

extern a_boolean doing_IF_dump;

/* The following function dumps all the information collected during
 * this parse to the IF file specified on the command line, if the
 * specified file can be opened.
 */

extern void dump_SET_IF(void);

/* Used during the tree walk to output IF. */
extern void (*walk_entry_and_subtree_for_IF)(char*, an_il_entry_kind);

/* Simply add the specified number of spaces to the output buffer. */
extern void indent_to_depth(int depth, string_buffer_ref output_buffer);

/* functions defined by il_walk.c */
extern void walk_entry_and_subtree(char*, an_il_entry_kind);
extern void SET_walk_entry_and_subtree(char*, an_il_entry_kind);
#endif /* SET_dump_if_h */
