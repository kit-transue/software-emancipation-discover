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
#ifndef SET_IF_h
#define SET_IF_h

#include "jil_def.h"
#include "fe_types.h"

/* This file contains declarations for the routines added by Software
 * Emancipation Technology for management and dumping of information
 * to the External Language Support Intermediate Format file.
 */

/* The following function dumps all the information collected during
 * this parse to the IF file specified on the command line, if the
 * specified file can be opened.
 */

extern void dump_SET_IF();

/* These functions track the command line file being analyzed. */
extern void set_defining_file_name(const char *fname);
extern void clear_defining_file_name();
extern const char *get_defining_file_name();

/* The following declarations deal with the saved constants.
 */

typedef a_constant_ptr *a_constant_ptr_ptr;

/* The following declarations deal with the saved keywords.
 */

typedef struct a_saved_keyword *a_saved_keyword_ptr;

/* This struct saves one keyword for the SMT. */

typedef struct a_saved_keyword {
  /* This struct represents one keyword. */
  a_source_locator source_location;
  a_token_kind keyword; /* spelling information not preserved here */
} a_saved_keyword;

/* The following declarations have to do with the saved cross
 * reference information; in SET mode, the information is saved in
 * memory rather than just dumped into a file.  an_xref_entry is the
 * data structure used to represent a symbol reference, and
 * get_xref_entry is the function used to traverse the saved entries
 * at the end of the compilation.
 */

/* typedef struct an_xref_entry* an_xref_entry_ptr;
 */

/* This struct saves the xref information for one reference.  Not
 * coincidentally, these are the same as the parameters of
 * write_xref_entry.
 */

/* typedef struct an_xref_entry {
 *   a_symbol_reference_kind srk_flags;
 *   a_symbol_ptr sym_ptr;
 *   a_source_position source_position;
 * } an_xref_entry;
 */

/* The following functions form an iterator for the xref entries; the */
/* list is exhausted when next_xref_entry returns NULL. */

/* void init_xref_entry_iterator();
 * an_xref_entry_ptr next_xref_entry();
 */

/* The following declarations deal with the saved error messages; in
 * SET mode, the messages are saved as well as displayed in the
 * terminal window.
 */

typedef struct a_saved_message* a_saved_message_ptr;

/* This struct saves one error message. */

typedef struct a_saved_message {
  /* This struct represents one message. */
  a_source_locator pos;
  const char* text;
} a_saved_message;

/* For use with SET_strings.h. */

typedef void an_il_to_str_output_control_block;

/* Find compilation_unit (cuk_stub_file kind) for "filename" in the
 * list of compilation_units.
 */
extern a_compilation_unit_ptr stub_compilation_unit_of(const char *filename);

/* Whether the compilation unit was specified directly or indirectly
 * on the command line
 */
extern a_boolean is_being_emitted(a_compilation_unit_ptr cup);

typedef struct a_cu_info *a_cu_info_ptr;
typedef struct a_cu_info {
    a_compilation_unit_ptr cup;
    unsigned long symid;
    a_cu_info_ptr next;
} a_cu_info;

extern a_cu_info_ptr create_a_cu_info_for(char *);
#endif /* SET_IF_h */
