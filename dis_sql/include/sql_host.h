/*
 * S Q L _ H O S T . H
 *
 * Copyright 1998  Software Emancipation Technology, Inc.
 *
 * Created Feb. 1998 Mark B. Kaminsky
 *
 * Routines for creating a host output file (either C or C++) from
 * an esql (Embedded SQL) source file.  Contains defines for both
 * sql_host.C and esql.C.
 */

#ifndef SQL_HOST_H
#define SQL_HOST_H

#ifndef ISO_CPP_HEADERS
#include <stdio.h>		
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "sql_all.h"		/* EXTERN_C */

EXTERN_C const char * hostfilename;

EXTERN_C int open_hostfile (const char * filename);

EXTERN_C int close_hostfile (void);

/* in esql.C */
EXTERN_C unsigned int olineno;
EXTERN_C unsigned int ocolumn;
EXTERN_C unsigned int ocolumn_last;

EXTERN_C void do_output(const char* txt);

EXTERN_C void do_output_chr(char c);

EXTERN_C void do_output_newline(void);

EXTERN_C void copy_out(sqltype val, int map_flag);

EXTERN_C void define_id(sqltype id);

EXTERN_C void do_ifdef (sqltype id);

EXTERN_C void do_ifndef (sqltype id);

EXTERN_C void do_else (sqltype id);

EXTERN_C void do_endif (sqltype id);

EXTERN_C void expand_varchar(sqltype id, sqltype size);

EXTERN_C void expand_varchar_init(sqltype id, sqltype size, sqltype init);

EXTERN_C void expand_varchar_init2(sqltype id, sqltype size, sqltype init);

EXTERN_C void expand_varchar_array(sqltype id, sqltype size, sqltype bound);

EXTERN_C void expand_varchar_ptr(sqltype ptr_op);

EXTERN_C void expand_varchar_default(sqltype id);

EXTERN_C void expand_varchar_ptr_cast(sqltype tok);

EXTERN_C void expand_sqlcursor (sqltype sqlcursor);

EXTERN_C int esql_command_line_option (const char* arg);

EXTERN_C int esql_initialize (void);

EXTERN_C void include_file(sqltype filename);

EXTERN_C void reference_var(sqltype ref);

EXTERN_C unsigned int sync_linenos(unsigned int ilineno);

/* The following functions add text surrounding a host variable
 * reference (which needs to be different at file scope from what is
 * used at block scope).
 */

EXTERN_C void prefix_reference(sqltype val);
EXTERN_C void postfix_reference(sqltype val);

EXTERN_C void iff_map_token_start_fixed (sqltype val);
EXTERN_C void iff_map_token_end_fixed (sqltype val);

/* The following variable is used to distinguish between file scope
 * (value == 0) and block scope (value > 0); it simpling counts the
 * number of open "{"s.
 */

EXTERN_C int scope_nesting_level;

/* If the following variable is true, any host variable references that
 * occur in file scope (i.e., with scope_nesting_level == 0) will be
 * suppressed.
 */

EXTERN_C int suppress_global_references;

/* Structure for Esql IFF Mapping records.
 *
 * Maps between text of input source file and host output.
 */
typedef struct iffmap_entry iffmap;

struct iffmap_entry {
   int src_lineno_start;
   int src_column_start;
   int src_lineno_end;
   int src_column_end;
   int dest_lineno_start;
   int dest_column_start;
   int dest_lineno_end;
   int dest_column_end;
   iffmap * next;
};

/* Structure for mapping whole lines */

typedef struct iffmap_lines iffmaplines;

struct iffmap_lines {
   int src_lineno_start;
   int src_lineno_end;
   int dest_lineno_start;
   iffmaplines * next;
};

EXTERN_C int iff_map_clear (void);
EXTERN_C int iff_map_set_src_filename (const char *name);
EXTERN_C const char * iff_map_get_src_filename (void);
EXTERN_C int iff_map_initial (void);

EXTERN_C int iff_map_copy_start_loc (int il, int ic, int ol, int oc);
EXTERN_C int iff_map_copy_end_loc (int il, int ic, int ol, int oc);
EXTERN_C int iff_map_fixed_start_loc (int il, int ic, int ol, int oc);
EXTERN_C int iff_map_fixed_end_loc (int il, int ic, int ol, int oc);

EXTERN_C iffmap * iff_map_get_copy_head (void);
EXTERN_C iffmap * iff_map_get_fixed_head (void);

EXTERN_C int iff_map_newline_check (void);

#endif /* SQL_HOST_H */
