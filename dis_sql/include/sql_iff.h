/*
 * S Q L _ I F F . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This is the include file for the IFF (Internal Format File) routines
 * that output ELS (External Language Support) entries for programs in
 * SQL (Structured Query Language).
 */

#ifndef SQL_IFF_H
#define SQL_IFF_H

#include "sql_all.h"		/* EXTERN_C */
#include "sql_symbol.h"		/* SQL_SYM symbol struct */
#include "sql_scanner.h"	/* sqltype token struct */

/*
 * iff (Intermediate Format File) labels
 */

/* basic entry labels */
#define IFFSTR_AST		"AST"		/* Abstract Syntax Tree */
#define IFFSTR_ATR		"ATR"		/* Attribute */
#define IFFSTR_ERR		"ERR"		/* Error */
#define IFFSTR_MAPCOPY		"MAP copy"	/* copied source */
#define IFFSTR_MAPFIXED		"MAP fixed"	/* changed source */
#define IFFSTR_MAPLINES		"MAP lines"	/* lines copied */
#define IFFSTR_REL		"REL"		/* Relationship */
#define IFFSTR_SMT		"SMT"		/* Symbol reference */
#define IFFSTR_SMTFILE		"SMT file"	/* Filename */
#define IFFSTR_SMTKEYWORD	"SMT keyword"	/* Keyword */
#define IFFSTR_SMTLANG		"SMT language"	/* Language */
#define IFFSTR_SYM		"SYM"		/* Symbol definition */

/* AST node names */
#define IFFSTR_HOSTVAR		"host_variable"	/* any host variable */

/* Supplemental labels */
#define IFFSTR_DEF		"def"		/* symbol definition node */
#define IFFSTR_REP		"rep"		/* symbol replacement */

/* Languages of input source file */
#define IFFSTR_LANG_SQL		"sql"		/* sql */
#define IFFSTR_LANG_ESQL	"esql"		/* embedded sql */

/* Relationships */
#define IFFSTR_REL_ARG		"argument"
#define IFFSTR_REL_CALL		"call"
#define IFFSTR_REL_CONTEXT	"context"
#define IFFSTR_REL_INCLUDE	"include"
#define IFFSTR_REL_TYPE		"type"

/* Attributes (both of relationships and standalone) */
#define IFFSTR_ATTR_ARG		"order"          /* was "argument" */
#define IFFSTR_ATTR_CONST	"const"
#define IFFSTR_ATTR_CURSOR	"cursor"
#define IFFSTR_ATTR_HOST	"host"
#define IFFSTR_ATTR_HOSTIND	"host_indicator"
#define IFFSTR_ATTR_INPUT	"input"
#define IFFSTR_ATTR_LOOPIDX	"loop_index"
#define IFFSTR_ATTR_OUTPUT	"output"
#define IFFSTR_ATTR_PCTROWTYPE	"percent_rowtype"
#define IFFSTR_ATTR_PCTTYPE	"percent_type"
#define IFFSTR_ATTR_PRIVATE	"private"
#define IFFSTR_ATTR_PUBLIC	"public"
#define IFFSTR_ATTR_RETURN	"return"
#define IFFSTR_ATTR_SCALE	"scale"
#define IFFSTR_ATTR_SIZE	"size"

/* current input filename (SQL source file) */
EXTERN_C const char * sql_infilename;

/* Record input filename for error reporting. */
EXTERN_C int iff_source_filename (const char * inname);

/* open output IFF file */
EXTERN_C int iff_openfile (const char * outname, const char * filemode);

/* close output file */
EXTERN_C int iff_closefile (void);

/* error message entry */
EXTERN_C int iff_error (const char * msg);

/* error message entry w/ additional text */
EXTERN_C int iff_error_text (const char * text, const char * msg);

/* error message entry w/ token */
EXTERN_C int iff_error_token (const token *t, const char * msg);

/* file entries */
EXTERN_C SQL_SYM * iff_file (const char* filename);

/* iff_file_reference - Output a filename reference entry. */
EXTERN_C int iff_file_reference (const char* filename);

/* iff_language - Output a language entry. */
EXTERN_C int iff_language (void);

/* keyword entry */
EXTERN_C int iff_keyword (const token * t);

/* relation for include of a file */
EXTERN_C int iff_include_relation (const SQL_SYM * sym_source, const SQL_SYM * sym_target);

/* transverse the ast (Abstract Syntax Tree) and output symbol entries
 * and the ast entry
 */
EXTERN_C int iff_ast (void);

EXTERN_C int iff_mapping_lines (const char * filename, 
			      int start_line, int end_line, int dest_line);

#endif /*  SQL_IFF_H */
