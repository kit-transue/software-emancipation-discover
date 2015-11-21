/*
 * S Q L _ S C A N N E R . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This include file declares routines and the structures for tokens
 * and yylval for the interface between the flex scanner and the
 * SQL parser.
 *
 * 99-05-27: Updated to include the entry for the Sybase parser.
 *	Most things declared in here are specific to the (Oracle
 *	and Informix) SQL parser; the only routines and data
 *	shared between the parsers are related to the token list,
 *	the sql_type struct, and line number and offset management.
 */

#ifndef SQL_SCANNER_H
#define SQL_SCANNER_H

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstddef>
#endif /* ISO_CPP_HEADERS */

#include "sql_all.h"		/* EXTERN_C */
#include "sql_symbol.h"		/* symbol struct (SQL_SYM) */
#include "sql_ast.h"		/* Abstract Syntax Tree (ast_tree) */

/*
 * Structure for storing a token
 * in the token list (indexed by token number)
 *
 * Line and column numbers are one-relative.
 */
typedef struct token {
   const char* text;		/* text of token */
   unsigned int lineno;		/* input file line number */
   unsigned int olineno;	/* output file line number */
   unsigned int column;		/* column number (one-relative) */
   size_t file_offset;		/* offset within input file */
} token;

/*
 * Structure for yylval (returned by Scanner to Parser
 * and then used by the Parser as the value of its
 * rules).
 *
 * Contains token range which starts out as the same value
 * (token number) but will be expanded in the
 * Parser to identify the starting and ending tokens
 * of grammar constructs.
 *
 * Symbol pointer is set by the scanner if this token is an identifier
 * which is found in the symbol table.  Otherwise it is set by the
 * parser.
 *
 * The Abstract Syntax Tree is set by the parser as it constructs
 * the tree from the grammar rules.
 */
typedef struct sql_type {
   unsigned int start_token;
   unsigned int end_token;
   SQL_SYM * sym;
   ast_tree * ast;
} sqltype;

/* define for flex/bison indicating to use this as yylval */
#undef YYSTYPE
#define YYSTYPE sqltype

/* input SQL source file position info
 *
 * current line number:
 * yylineno increases for the whole sequence of files
 * ilineno is reset for every included file
 *
 * icolumn_last is last column of previous last
 */
EXTERN_C unsigned int yy_sql_lineno;
EXTERN_C unsigned int ilineno;
EXTERN_C unsigned int icolumn_last;

/* Get the input column number of the current token */
EXTERN_C int  (*get_icolumn) (void);

/* Include file level.  Zero is main file. */
EXTERN_C int esql_include_level;

/*
 * Start a parsing run with a new file.
 * Ensures that initialization is done.
 *
 * Called by the driver after the input and output
 * files are opened.
 */
EXTERN_C int sql_process (FILE * fp_in);
EXTERN_C int sybase_process(FILE* inp);

/* The following function returns a pointer to a token, given a token
 * number.
 */
EXTERN_C token* get_token(unsigned int token_no);
 
/* Reinitialize the token list after a preprocessing run: */
EXTERN_C void free_token_list();

/* Create a new token: */
EXTERN_C token* new_token (const char* txt, unsigned int * ptoken_no);

/* Standard yacc/lex error routine name.
 * Called for parse errors.
 */
EXTERN_C int yyerror (const char* msg);
EXTERN_C int sql_errcount;

/* change the starting scanner state - the state it switches to
 * after each command
 */
EXTERN_C int sql_scanner_sql_mode (void);
EXTERN_C int sql_scanner_scan_mode (void);

/* Embedded SQL flag - true if parsing an esql file */
EXTERN_C int esql_mode;

/* Embedded SQL host variable reference -- true if currently parsing
 * a host variable reference (used to handle "[ ... ]" differently
 * from the default.
 */

EXTERN_C int inside_host_variable_reference;

/* flag: parsing specification files -- not generating output */
EXTERN_C int symbol_specification_phase;                /* in sql_driver.C */

/* open an include file */
EXTERN_C int esql_push_include (const char * fname);	/* in sql.l */

#endif /* SQL_SCANNER_H */
