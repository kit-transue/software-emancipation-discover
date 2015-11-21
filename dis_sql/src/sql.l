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

%{
 
/*
 * S Q L . L
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the flex specifications for the scanner to process
 * SQL (Structured Query Language) programs in the following forms:
 *     Oracle and Informix SQL scripts.
 *     Oracle's PL/SQL (Procedural Language / Structured Query
 *        Language) up to version 7.3.  As well as the Informix
 *        equivalent.  These programs are refered to as
 *        "Stored Procedures".
 *     Embedded SQL (ESQL) C/C++ programs.  These are C or C++ programs
 *        (the "host") with SQL statements with are transformed by the Oracle
 *        or Informix preprocessor before being passed to the C/C++ compiler.
 *
 * The purpose of this scanner (along with the parser and its associated
 * routinues) is to produce an ELS (External Language Support)
 * IFF (Internal Format File).
 *
 * This scanner is invoked by the parser (in sql.y) to read in
 * the PL/SQL program source file and break it up into lexical
 * tokens (keywords, identifiers, and literals).  One token is
 * returned with each call by the parser.  See sql_scanner.h for
 * the token return structure.  A list of tokens, indexed
 * by token number is maintained which contains token text and offset
 * data.
 *
 * A keyword lookup table is used to distinguish between keywords
 * and identifiers.  Keywords, in addition to being returned to
 * the parser, are also output to the IFF.  A symbol table, maintained
 * by the parser, is checked for each identifier to see if it is
 * already known.  This information is returned to the parser, in
 * addition to token value and number.  It is up to the parser to
 * either use this symbol information or to override it, as in the
 * case of a redefinition or scope change.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "sql_symbol.h"		/* symbol struct (SQL_SYM) and table */
#include "sql_ast.h"		/* Abstract Syntax Tree */
#include "sql_scanner.h"	/* yylval token struct */
#include "sql.yacc.h"		/* yacc generated token defines */
#include "sql_iff.h"		/* IFF output */
#include "sql_sym_enum.h"	/* symbol kind enums */
#include "sql_keyword.h"	/* keyword lookup */
#include "sql_host.h"		/* host file output */

#ifdef _WIN32
/* On NT the Thinkage YAY (Yet Another Yacc) parser is used */
#define THINKAGE_YAY	1
#endif

#if SQL_DEBUG	!= 0
extern int sql_debug_level;				/* in sql_driver.C */
#endif

EXTERN_C int need_end_of_line_token;			/* in sql.y */
EXTERN_C int scanner_generic_mode;			/* in sql.y */
EXTERN_C int sql_allow_error;				/* in sql.y */
EXTERN_C int sql_inside_trigger;			/* in sql.y */
EXTERN_C int sql_identifier_next;			/* in sql.y */
EXTERN_C SQL_SYM * sql_symbol_main_file;		/* in sql_driver.C */
EXTERN_C int sql_string_max_lines;			/* in sql_driver.C */

/* routines below */
static token* add_token (SQL_SYM * sym);
static token* add_token_from(const char* txt, SQL_SYM * sym);
static int postprocess (int);
static int process_token (int);
static int process_start_token (int, int);
static int process_end_token (int, int);
static int process_symbol_token (int, SQL_SYM *);
static int process_string (int);
static int process_identifier (void);
static int process_identifier_only (void);
static int process_host_variable (void);
static int process_pct_token (void);
static int discard_comment(int copyit);
static void handle_newline (int copyit);
static void increment_lineno (char*);
static size_t get_start_file_offset (void);
static size_t get_end_file_offset (void);
static int sql_get_icolumn (void);
static void reset_start_of_statement (void);
static int sql_scanner_init_state (int state);
static int sql_scanner_state (int state);
static int sql_scanner_state_wrap (void);
static int esql_del_include (void);
static int esql_pop_include (void);
static int pop_include (void * old_buff, FILE * old_yyin);
static void * push_include (FILE * new_yyin);

/* flex routine to get next input character */
static int input (void);

/* current line/column number
 *
 * This is used for error reporting and IFF mapping entries
 *
 * yylineno increases for the whole sequence of files
 * ilineno is reset for every included file
 *
 * icolumn_last is last column of previous last
 */
unsigned int yylineno = 1;
unsigned int ilineno = 1;
unsigned int icolumn_last = 1;

/* Count of errors --
 *
 * When an error occurs the parser keeps going after skipping to
 * a possible recovery place (such as the next semicolon or closing
 * parenthesis).  This count ensures that the parsers exit status
 * reflects the fact that an error occurred.
 */
int sql_errcount = 0;

/* Include file level.  Zero is main file. */
int esql_include_level = 0;

/* level of nesting in C/C++ host language (really brace count)
 * a level of zero means file (global) level.
 */
int scope_nesting_level = 0;

/* Multi-line string --
 * line/column where multi-line string started
 *
 * In SQL, strings can span multiple lines.  These serve to
 * track the starting point for IFF mapping and test for
 * a multi-line string that is really just a missing quote
 * syntax error (by exceeding some [settable] number of lines). 
 */
static int start_string_line = 0;
static int start_string_column = 0;

/* Starting scanner state --
 * Either scan esql (ignoring host code) or sql
 *
 * There are a number of scanner states, but the two starting states are:
 *    SCAN for esql which ignores (copies out) C/C++ host code
 *       until a sql statement is found.
 *    SQL for sql scripts or PL/SQL programs, where the scanner looks for
 *       sql tokens immediately.
 * After the end of each statement the scanner returns to its start state.
 */
static int sql_scanner_start_state;

/* at the start of SQL command - used for keyword processing */
static int start_of_statement = 1;

/* last character of line was SQL continue line indicator (hyphen) */
static int got_line_continue = 0;

/* Processed last end-of-file.
 * Useful for files that don't end in a newline.
 */
static int got_eof = 0;

/* current scanner state */
static int sql_scanner_current_state = INITIAL;

/* count for nested brackets */
/* static int varchar_bracket_count = 0; */

/* flag if inside a varchar cast (e.g. foo = (VARCHAR*) malloc (x)) */
static int varchar_cast = 0;

/*
 * Flag to resolve conflict between:
 * 	expression BETWEEN expression AND expression
 *	expression AND expression
 *
 * if set indicates that BETWEEN token encountered
 * and treat next AND as belonging to the between
 * clause and not a logical AND.
 */
static int between_flag = 0;

/* Nesting depth of brackets when scanning host variable array
 * subscript.
 */

static int bracket_nesting = 0;

/* starting value of flag so that when decremented in postprocess
 * it is set after first return to the parser and cleared for
 * subsequent returns
 *
 * TBD: this is error prone (if postprocess is called more than once),
 * it would be better to clear these kinds of token by token flags
 * in some sort of preprocess before each scanner call.
 */
#define SET_PROCESS_FLAG	2

#if THINKAGE_YAY == 1
/* message passed to yyerror when called by Thinkage YAY
 * at the place where it allows scanner return value (yychar)
 * to be changed and reprocessed.
 */
#define PARSER_SYNTAX_MESSAGE "Syntax error"

/* set if last return to parser was a keyword */
static int sql_keyword_flag = 0;
#endif /* THINKAGE_YAY == 1 */

/*
 * Track the file offset of the current token (yytext).
 *
 * Programming the Lex way -- with macros --
 * real programming for real men.
 *
 * YY_INPUT is the flex input macro which is called in
 * yy_get_next_buffer() to read in more characters.
 * This macro differs from the standard macro in two
 * respects:
 * 1) it assumes that the yy_current_buffer->yy_is_interactive
 *    flag is false (getc until newline vs. fread)
 * 2) it tracks the current offset within the input file.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * next_offset -- will be the file offset for the next read.
 * number_to_move -- a local in yy_get_next_buffer() which is
 * the count of bytes left in the buffer from the last read.
 *
 * newline_offset -- updated for every newline.
 */

static size_t current_offset = 0;
static size_t next_offset = 0;
static size_t newline_offset = 0;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
   if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
	  && ferror( yyin ) ) \
       YY_FATAL_ERROR( "input in flex scanner failed" ); \
   else { \
      current_offset = next_offset - number_to_move; \
      next_offset += result; \
   }

%}

/*************** Flex Definitions ********************/

	/**** C variable -- NOT including pointer, array ref, etc ****/
CVAR		([a-zA-Z_][a-zA-Z_0-9]*)

	/**** numeric exponent ****/
EXP		([Ee][\+\-]?[0-9]+)

        /**** Any non-whitespace except
	 **** comment and string starters
	 **** and some punctuation:
	 **** dollar sign (INFORMIX prefix),
	 **** braces (tracked for global scope),
	 **** paren and star (could be part of cast (VARCHAR*))
	 ****/
CHR		[^ \n\t\v\f\r\032/\'\"\{\}\$\(\)\*]

	/**** string character (i.e. the stuff inside quotes)
	 **** anything but quote, newline, or backslash
	 **** but accept backslash followed by non-newline
	 ****/
STR		([^\"\n\\]|(\\[^\n]))

	/**** whitespace (w/o newline) ****/
WS		[ \t\v\f\r\032]

	/**** REMark (SQL comment line) ****/
rem		[Rr][Ee][Mm]([Aa][Rr][Kk])?

	/**** Trigger table name
	 **** In a SQL Update trigger
	 **** OLD can be used as shorthand for the record before update
	 **** and NEW after update.  Often with prefixed with a colon.
	 ****/
OLDNEW		(([Oo][Ll][Dd])|([Nn][Ee][Ww]))

        /**** ESQL directives ****/

endexec		[Ee][Nn][Dd]\-[Ee][Xx][Ee][Cc]
exec		[Ee][Xx][Ee][Cc]
iaf		[Ii][Aa][Ff]
oracle		[Oo][Rr][Aa][Cc][Ll][Ee]
sql		[Ss][Qq][Ll]
sqlcursor	[Ss][Qq][Ll]_[Cc][Uu][Rr][Ss][Oo][Rr]
tools		[Tt][Oo][Oo][Ll][Ss]
varchar		[Vv][Aa][Rr][Cc][Hh][Aa][Rr]

        /**** Informix ESQL directives ****/
fixchar		[Ff][Ii][Xx][Cc][Hh][Aa][Rr]
parameter	[Pp][Aa][Rr][Aa][Mm][Ee][Tt][Ee][Rr]
string		[Ss][Tt][Rr][Ii][Nn][Gg]

	/**** common C/C++ declaration tokens (for Informix) *****/

DECL	(char|double|float|extern|int|long|static|struct|unsigned)


/*************** Scanner States *****************/

/*
 * These states are all exclusive (i.e. using lex %x directive) so that
 * only one is active at a time.
 *
 * SCAN		In C/C++ portion of ESQL file.  Just copy file until
 *		SQL statement is found then switch to SQL state to
 *		scan that.  SQL statements start with "EXEC SQL"
 *		(also $ in Informix).  Also handled are ESQL
 *		preprocessor data definition directives such as VARCHAR.
 *		This state also knows about C/C++ comments and strings
 *		so that SQL statements inside them are ignored.
 *
 * SQL		Main state for scanning SQL statements.
 *
 * SQLSTRING	Scan a complex string (SQL strings use single quotes)
 *		inside SQL state.  String is either multi-line
 *		or has embedded quotes.  Normal strings are handled
 *		within a single rule in the SQL state.
 *
 * SCANSTRING	Scan a complex host string (double quotes) inside the
 *		SCAN state.
 *
 * IGNDOT	IGNORE DOT.  Normally Oracle delimits data elements
 *		with a single dot (e.g. tablename.columnname), however
 *		when one of the elements is an ampersand variable
 *		it needs two dots instead of one (e.g. &tabname..colname).
 *		This state throws out the extra dot.  This avoids doubling
 *		dozens of grammar rules and dealing with conflicts with
 *		the SQL range operator (which is dot dot).
 *
 * IDENT	Process identifier.  Used to avoid keyword checking
 *		in dot delimited data elements.  There are a few cases
 *		where elements can be keywords (e.g. tabname.ROWID)
 *		but these are handled (more or less) in the parser.
 *
 * TRIG		Handle the trigger table name (see OLDNEW comment above).
 *
 * HOST		Handle a host identifer.  In ANSI SQL (including Oracle)
 *		these are prefixed with a colon (e.g. :foo).  However,
 *		Informix also allows a dollar sign prefix (e.g. $foo).
 *		This state is active after the prefix has been removed. 
 *
 * HOST_ARRAY	Handle a subscript in a host array reference, which can
 *		include just about anything and stretch over multiple
 *		lines.
 */

%x SCAN SQL SQLSTRING SCANSTRING IGNDOT IDENT TRIG HOST HOST_ARRAY

/*************** Start of Flex Rules *****************/

/*
 * Note on some less (no pun intended) known Lex routines:
 *
 * 1) yymore() is the Lex routine which does the next match
 * but yytext will include both the current text and the matched text.
 * It can be called repeatly to build up a large token such as a
 * multi-line string.
 *
 * 2) yyless(n) changes yytext by putting back all but the first n characters
 * of yytext.  Count from front and put back from there to end of token
 * So:
 *
 * yyless(0) puts back all of yytext (used to reprocess in a new state).
 * yyless(1) puts back the all but the first character (such as the
 * colon of a host variable).
 * yyless(yyleng-1) puts back only the last character (so it can be
 * processed by itself).
 *
 * 3) In "pattern1/pattern2" the foward slash is the Lex lookahead
 * operator.  Only pattern1 becomes yytext.
 *
 * The above are used to avoid most additional non-lex scanning
 * of yytext and so that the process_xxx routines can just use
 * the value in yytext.
 *
 */

%%

	/**** Embedded SQL ****/
<SCAN>{exec}{WS}+{sql}	{
			   sql_scanner_state (SQL);
			   return (process_start_token (Y_EXEC_SQL, 1));
			}

<SCAN>{exec}{WS}+{oracle}	{
			   sql_scanner_state (SQL);
			   return (process_start_token (Y_EXEC_ORACLE, 1));
			}

<SCAN>{exec}{WS}+{iaf}	{
			   sql_scanner_state (SQL);
			   return (process_start_token (Y_EXEC_IAF, 1));
			}

<SCAN>{exec}{WS}+{tools}	{
			   sql_scanner_state (SQL);
			   return (process_start_token (Y_EXEC_TOOLS, 1));
			}

<SCAN>{varchar}		{
			   /* creates token in yylval and outputs keyword */ 
			   int token_type = process_start_token (Y_VARCHAR, 1);
			   /* if varchar_cast flag is set then we got
			    * here from rule below.
			    */
			   if (varchar_cast) {
			      expand_varchar_ptr_cast (yylval);
			      varchar_cast = 0;
			   } else {
			      /* TBD: should really switch to VARCHAR state
			       * to avoid SQL grammar, since we are still
			       * in C grammar.  New state would get VARCHAR
			       * identifiers, array bounds (which can be
			       * expressions), and initializers.
			       */

			      sql_scanner_state (SQL);
			      return (token_type);
			   }
			}

<SCAN>\({WS}*{varchar}{WS}*\*+{WS}*\)		{
			   /* VARCHAR pointer cast -
			    * change (VARCHAR*) to (void*)
			    *
			    * Output first paren and process VARCHAR
			    * above after setting flag, rest is processed
			    * as normal copied out text.
			    */
			   do_output_chr (*yytext);
			   yyless (1);	/* puts back all yytext but 1st char */
			   varchar_cast = 1;
			   /* no return */
			}


<SCAN>{sqlcursor}	{
			   /* don't change state -
			    * just pass to grammar this one token for
			    * transformation.
			    */
			   return (process_start_token (Y_SQL_CURSOR, 1));
			}

	/**** char constants containing [double] quote.
	 **** caught before the quote is interpreted as the start of a string
	 ****/
<SCAN>\'\\\"\'		{
			   /* character constant '\"' */
			   do_output (yytext);
			}

<SCAN>\'\"\'		{
			   /* character constant '"' */
			   do_output (yytext);
			}
		
	/**** strings - no scanning for keywords inside them ****/	   
<SCAN>\"{STR}*\"	{
			   do_output (yytext);
			}


<SCAN>\"{STR}*\\?$	{
			   /* start of string that spans multiple lines */

			   /* The backslash before the newline is optional
			    * (although I am not sure that is valid C/C++)
			    */
			   sql_scanner_state (SCANSTRING);
			   start_string_line = yylineno;
			   start_string_column = sql_get_icolumn ();
			   yymore ();
       			}

<SCANSTRING>{STR}+\\?$	{
			   yymore ();
			}

<SCANSTRING>\n		{
			   /* will be copied later */
			   handle_newline (0);

			   if (yylineno >
			       (start_string_line + sql_string_max_lines)) {
			      sql_scanner_state (SCAN);
	      yyerror ("scan maximum string length reached: unterminated string?");
		           } else {
			      yymore ();
	                   }
			}

<SCANSTRING>{STR}*\"	{
			   /* found end of string */
			   sql_scanner_state (SCAN);
			   do_output (yytext);
			}

	/**** comments ****/
<SCAN>\/\/.*$		{  do_output (yytext); }

<SCAN>\/\*		{
			   do_output (yytext);
			   discard_comment (1);
			}

	/**** Informix (starts with $) ****/
<SCAN>\${WS}*{DECL}	{
			    /* host variable declaration */
			    /* handles common decls, parser handles the rest */
			    /* remove $ and handle rest as host code */
			    if (dbvendor == DBVENDOR_INFORMIX) {
			       do_output_chr (' ');
			       yyless (1);
			    } else {
			       do_output (yytext);
			    }
			}
<SCAN>\${WS}*{parameter}	{
			    /* Informix parameter directive */
			    if (dbvendor == DBVENDOR_INFORMIX) {
			       /* ignore */
			       int i;
			       for (i = 0; i < yyleng; ++i) {
				  do_output_chr (' ');
			       }
			    } else {
			       do_output (yytext);
			    }
			 }

<SCAN>\${WS}*[\{\}]	{
			    /* Informix block */
			    /* remove $ and handle brace as host code */
			    if (dbvendor == DBVENDOR_INFORMIX) {
			       do_output_chr (' ');
			       yyless (1);
			    } else {
			       do_output (yytext);
			    }
			}
<SCAN>\${WS}*{varchar}	{
			   if (dbvendor == DBVENDOR_INFORMIX) {
			      sql_scanner_state (SQL);
			      return (process_start_token (Y_VARCHAR, 1));
			   } else {
			       do_output (yytext);
			   }
			}
<SCAN>\${WS}*{string}	{
			   /* handle like VARCHAR */
			   if (dbvendor == DBVENDOR_INFORMIX) {
			      sql_scanner_state (SQL);
			      return (process_start_token (Y_VARCHAR, 1));
			   } else {
			       do_output (yytext);
			   }
			}
<SCAN>\${WS}*{fixchar}	{
			   /* handle like VARCHAR */
			   if (dbvendor == DBVENDOR_INFORMIX) {
			      sql_scanner_state (SQL);
			      return (process_start_token (Y_VARCHAR, 1));
			   } else {
			       do_output (yytext);
			   }
			}
<SCAN>\${WS}*{CHR}+	{
			   if (dbvendor == DBVENDOR_INFORMIX) {
			      /* this is either the start of a sql command
			       * (e.g. $select) or the start of a data
			       * declaration with a user typedef 
			       * (e.g. $mydata x;).
			       * The parser checks.
			       */
			      yyless (1);
			      sql_scanner_state (SQL);
			      return (process_start_token (Y_EXEC_SQL, 1));
			   } else {
			       do_output (yytext);
			   }
			}
<SCAN>\$		{
			   if (dbvendor == DBVENDOR_INFORMIX) {
			      sql_scanner_state (SQL);
			      return (process_start_token (Y_EXEC_SQL, 1));
			   } else {
			       do_output (yytext);
			   }
			}

	/**** generic C/C++ tokens ****/
<SCAN>{CHR}+		{
			   do_output (yytext);
			}

<SCAN>{WS}+		{
			   do_output (yytext);
			}

<SCAN>\n		{
			   handle_newline (1);
			}

<SCAN>\{		{
			   scope_nesting_level++;
			   do_output (yytext);
			}

<SCAN>\}		{
			   scope_nesting_level--;
			   do_output (yytext);
			}

<SCAN>.			{
			   do_output (yytext);
			}
	/**** Main SQL scanning state ****/

	/**** multicharacter operators and punctuation ****/
<SQL>\:\=		{ return (process_token (OP_ASSIGN)); }
<SQL>\=\>		{ return (process_token (OP_ASSOCIATE)); }
<SQL>\|\|		{ return (process_token (OP_CONCAT)); }
<SQL>\*\*		{ return (process_token (OP_EXPONENT)); }
<SQL>\>\=		{ return (process_token (OP_GE)); }
<SQL>\>\>		{ return (process_token (OP_LABEL_END)); }
<SQL>\<\<		{ return (process_token (OP_LABEL_START)); }
<SQL>\<\=		{ return (process_token (OP_LE)); }
<SQL>\!\=		{ return (process_token (OP_NE)); }
<SQL>\<\>		{ return (process_token (OP_NE)); }
<SQL>\~\=		{ return (process_token (OP_NE)); }
<SQL>\^\=		{ return (process_token (OP_NE)); }
<SQL>\.\.		{ return (process_token (OP_RANGE)); }
<SQL>\@\@		{ return (process_token (OP_START)); }
<SQL>\(\+\)          { /* ignore. TBD: return (process_token (OP_OUTERJOIN)); */ }
<SQL>\-\>		{ return (process_token (OP_ARROW)); }
<SQL>\+\+		{ return (process_token (OP_INCR)); }
<SQL>\-\-		{ return (process_token (OP_DECR)); }
<SQL>\:\:		{ return (process_token (OP_QUAL)); }

	/**** REMARK single line comment in standalone SQL ****/
<SQL>^{rem}[^A-Za-z_\$#0-9\.\n].*$	{ /* ignore */ }
<SQL>^{rem}$		{ /* ignore */ }

	/**** Cedel hack
	 **** Some code begins with $PROD=
	 **** not clear what to do with these
	 ****/
<SQL>^{WS}*\$PROD{WS}*\=	{ /* ignore */ }

	/**** Percent directives: %TYPE, %NOTFOUND, etc ****/
<SQL>\%[A-Za-z_]+	{ return (process_pct_token ()); }

	/**** END-EXEC esql token (end of PL/SQL stmts) ****/
<SQL>{endexec}		{ return (process_token (Y_END_EXEC)); }

	/**** Identifiers ****/
	/*
	 * including keywords and previously identified symbols
	 */
<SQL>[A-Za-z_\$#][A-Za-z_\$#0-9]*	{
			/* first check for Informix host variable.
			 * note that $ is valid in SQL variable names
			 * (as is #)
			 */
			if (esql_mode &&
			    (dbvendor == DBVENDOR_INFORMIX) &&
			    (*yytext == '$')) {
			   yyless (1);
			   sql_scanner_state (HOST);
			   return (process_token (HOST_PREFIX));
			} else {
			   return (process_identifier ());
			}
		     }

	/**** Dot separated element list -- treat as indentifiers */

<SQL>{OLDNEW}\.[A-Za-z_\$#][A-Za-z_\$#0-9]*	{
			/* push back all of yytext and process in new state */
			yyless (0);
			/* OLD and NEW have special meaning in trigger
			 * otherwise they are just identifiers
			 */
			if (sql_inside_trigger) {
			   sql_scanner_state (TRIG);
			} else {
			   sql_scanner_state (IDENT);
			}
		     }

<SQL>[A-Za-z_\$#][A-Za-z_\$#0-9]*\.[A-Za-z_\$#][A-Za-z_\$#0-9]*	{
			/* dot separated list */
			if (esql_mode &&
			    (dbvendor == DBVENDOR_INFORMIX) &&
			    (*yytext == '$')) {
			   yyless (1);
			   sql_scanner_state (HOST);
			   return (process_token (HOST_PREFIX));
			} else {
			   /* reprocess in IDENT state - no keywords */
			   yyless (0);
			   sql_scanner_state (IDENT);
			}
		     }

	/**** identifier only -- 
	 **** splits up "id.id.id" into "id." "id." "id"
	 ****/

<IDENT>[A-Za-z_\$#][A-Za-z_\$#0-9]*\.	{
			/* dot as last charater means that this is not
			 * last identifier
			 */

			/* push back dot */
			yyless (yyleng - 1);
			return (process_identifier_only ());
		     }
<IDENT>\.				{
			return (process_token ((int)*yytext));
		     }

<IDENT>[A-Za-z_\$#][A-Za-z_\$#0-9]*	{
			/* last identifier in dot separated list */
			sql_scanner_state (SQL);
			return (process_identifier_only ());
		     }

<IDENT>.	     {
			/* shouldn't get here */
			yyless (0);
			sql_scanner_state (SQL);
		     }

	/**** Host variable names ****/
	/*
	 * Starts with a colon and can contain dot.
	 * Also covers indicator variables.
	 *
	 * Host variables are variables in the host language, such as
	 * a C or PASCAL variable, that are used in the PL/SQL program.
	 * E.g. SELECT emp_name INTO :host_buffer FROM exployee
	 * WHERE emp_id = 4;
	 *
	 * The initial colon is kept to avoid name conflicts with
	 * PL/SQL variables when searching the symbol table.  The colon
	 * is removed for reporting so that it will match the host
	 * declaration.
	 *
	 * Indicator variables are associated with a host variables,
	 * their purpose is to flag that the host variable is
	 * NULL or NOT NULL.
	 * E.g. SELECT emp_name INTO :host_buffer:isnull_ind
	 * FROM exployee WHERE emp_id = 4;
	 */
<SQL>\:[ \t]*{OLDNEW}(\.{CVAR})?	     {
			/* check for special trigger table keywords
			 * of :NEW and :OLD.  If so remove initial
			 * colon and rescan (NEW and OLD will be
			 * keywords and the rest (if any) will
			 * be normal identifiers.
			 */
			if (sql_inside_trigger) {
			   if (yyleng > 4) {
			      yyless (1);
			      sql_scanner_state (TRIG);
			   } else {
			      yyless (1);
			      return (process_identifier ());
			   }
			} else {
			   yyless (1);
			   sql_scanner_state (HOST);
			   return (process_token (HOST_PREFIX));
			}
		     }

<TRIG>{OLDNEW}	     {
			sql_scanner_state (IDENT);
			/* checks for keyword first */
			return (process_identifier ());
		     }
<TRIG>[ \t]+	     { /* ignore */ }
<TRIG>.		     {
			/* shouldn't get here */
			yyless (0);
			sql_scanner_state (SQL);
		     }
			
<SQL>\:[ \t]*{CVAR}  {
			/* remove colon and process as host variable */
			yyless (1);
			sql_scanner_state (HOST);
			return (process_token (HOST_PREFIX));
		     }

<SQL>\$		     {
			/* standalone $ assume host variable follows */
			if (esql_mode && (dbvendor == DBVENDOR_INFORMIX)) {
			   sql_scanner_state (HOST);
			   return (process_token (HOST_PREFIX));
			} else {
			   /* not Informix ESQL -- therefore valid single
			    * character identifier since the dollar sign
			    * is a valid identifier name.
			    */
			   return (process_identifier ());
			}
		     }


<HOST>[ \t]+	     { /* ignore */ }
<HOST>{CVAR}	     {
			sql_scanner_state (SQL);
			return (process_host_variable ());
		     }
<HOST>.		     {
			/* something other than variable */
			yyless (0);
			sql_scanner_state (SQL);
		     }

<HOST_ARRAY>\[		{ bracket_nesting++;
			  return process_token(GENERIC_TOKEN);
			}

<HOST_ARRAY>\]		{ if (--bracket_nesting) {
			    return process_token(GENERIC_TOKEN);
			  }
			  else {
			    sql_scanner_state(SQL);
			    return process_token(']');
			  }
			}

<HOST_ARRAY>{WS}+	{ /* ignore */ }

<HOST_ARRAY>\n		{ handle_newline(0); }

<HOST_ARRAY>.		{ return process_token(GENERIC_TOKEN); }

	/**** treat numeric :1 like &1, etc ****/
<SQL>\:[0-9]+		 { return (process_identifier ()); }


	/**** prefixed variables *****/
	/*
	 * Starts with one or two ampersands followed by either
	 * a number (&1) or a variable name (&&DATA_TABLESPACE).
	 * Used for variable substitution within SQL scripts.
	 */
<SQL>\&\&?[A-Za-z_\$#0-9]+	{ return (process_identifier ()); }

<SQL>\&\&?[A-Za-z_\$#0-9]+\.\.	{
			/* push the two dots back */
			yyless (yyleng-2);
			/* handle double dot as single when it follows
			 * an ampersand variable.  Which is what Oracle
			 * seems to do.
			 */
			sql_scanner_state (IGNDOT);
			return (process_identifier ());
		     }

<IGNDOT>\.	    {
			/* handle double dot as single when it follows
			 * an ampersand variable.  Which is what Oracle
			 * seems to do.  Ignore first dot.
			 */
			sql_scanner_state (SQL);
		     }

<IGNDOT>.	     {
			/* shouldn't get here */
			yyless (0);
			sql_scanner_state (SQL);
		     }

	/**** numbers ****/
	/*
	 * Need to handle initial plus or minus as unary expression
	 * in grammar to avoid "x+1" being scanned as:
	 *	identifier "x", numeric_literal "+1"
	 * instead of the correct:
	 *	identifier "x", operator "+", numeric_literal "1"
	 *
	 * Also need to avoid the range "1..10" being scanned as:
	 *	numeric_literal "1.", numeric_literal ".10"
	 * instead of the correct:
	 *	numeric_literal "1", operator "..", numeric_literal "10"
	 */
<SQL>[0-9]+\./[^\.0-9]	{ return (process_token (NUMERIC_LITERAL)); }
<SQL>[0-9]+		{ return (process_token (NUMERIC_LITERAL)); }
<SQL>[0-9]+\.[0-9]+	{ return (process_token (NUMERIC_LITERAL)); }
<SQL>\.[0-9]+		{ return (process_token (NUMERIC_LITERAL)); }
<SQL>[0-9]+\.?[0-9]*{EXP}	{ return (process_token (NUMERIC_LITERAL)); }
<SQL>[0-9]*\.?[0-9]+{EXP}	{ return (process_token (NUMERIC_LITERAL)); }



	/**** comments ****/
<SQL>\-\-.*$		{ /* ignore SQL single line comment */ }

<SQL>\/\/.*$		{ /* ignore C++ style comments too */ }

<SQL>\/\*		{ discard_comment (0); }


	/**** string and character literals ****/
	/*
	 * Handle embedded quotes.
	 * These are not backslash quote, but quote quote.
	 * So a string consisting of a single quote would be: ''''
	 * This requires lookahead.
	 *
	 * Strings with double-quotes are used in Oracle
	 * to enclose identifiers (especially alias names)
	 * that may contain non-standard characters, such as
	 * spaces.
	 */
<SQL>\'[^\'\n]*\'/[^\']	{
		   /* Complete string w/o embedded quotes.
		    *
		    * Note: the foward slash is the Lex lookahead
		    * operator.  In this case it ensures that the
		    * next character is not a quote.
		    */
		   return (process_string (0));
		}

<SQL>\"[^\"\n]*\"	{
		   /* Complete double-quoted string.
		    *
		    * Quoted identifier.
		    * process_string() looks at first char and knows.
		    */
		   return (process_string (0));
		}


<SQL>\'[^\'\n]*$	{
		   /* got to end-of-line before reaching terminating quote
		    * start multi-line/inside string state
		    */
                   start_string_line = yylineno;
		   start_string_column = sql_get_icolumn ();
		   sql_scanner_state (SQLSTRING);
		   yymore ();
		}

<SQL>\'[^\'\n]*\'\'	{
		   /* Found an embedded quote.
		    *
		    * Change to the inside-a-string state
		    * and get more of the string.
		    *
		    * Note: yymore() is the Lex routine which
		    * does the next match but yytext will include
		    * both the current text and the matched text.
		    */
                   start_string_line = yylineno;
		   start_string_column = sql_get_icolumn ();
		   sql_scanner_state (SQLSTRING);
		   yymore ();
		}

<SQLSTRING>[^\'\n]*\'/[^\']	{
		   /* Found the end of the string.
		    *
		    * Change state back for normal processing
		    * and return whole string (which is in yytext).
		    *
		    * Note that the patterns for this lexical state
		    * do not include an inital quote, since that was
		    * matched before entering this state.
		    */
		   sql_scanner_state (SQL);
		   return (process_string (1));
		}

<SQLSTRING>\n	{
		   handle_newline (1);

		   if (yylineno > (start_string_line + sql_string_max_lines)) {
		      sql_scanner_state (SQL);
		      yyerror ("maximum string length reached: unterminated string?");
                   } else {
		      yymore ();
                   }
		}

<SQLSTRING>[^\'\n]*\'\'	{
		   /* Some more of the string with another embedded quote.
		    * Add it to yytext and keep going.
		    */
		   yymore ();
		}

<SQLSTRING>[^\'\n]*$	{
		   /* got to end-of-line before reaching terminating quote */
                   yymore ();
		   }

	/**** varchar brackets ****/
<SQL>\[[^\[\n]*\]	{
		    /* TBD: this might be better handled as part of a
		     * separate state
		     */
		    if (esql_mode) {
		       if (!inside_host_variable_reference) {
		         return (process_token (VARCHAR_ARRAY));
		       }
		       else {
			 /* Enter HOST_ARRAY mode and rescan */
			 yyless(1);
			 sql_scanner_state(HOST_ARRAY);
			 bracket_nesting = 1;
			 return process_token('[');
		       }
		    } else {
		       /* handle as single characters */
		       REJECT;
		    }
		   }

	/**** newline ****/
<SQL>\n		{
		   handle_newline (1);

		   /* many SQL script commands can be terminated by
		    * the end of line (parser sets flag if that is the
		    * case) instead of or in addition to a end-of-stmt
		    * character.  If we are in one of those commands
		    * return an eol.
		    */
		   if (need_end_of_line_token) {
		      if (got_line_continue) {
			 got_line_continue = 0;
			 /* ignore - no return */
		      } else {
			 need_end_of_line_token = 0;
			 return (process_end_token (END_OF_LINE, 0));
		      }
		   }
		}

	/**** SQL line continuation ****/
<SQL>\-$	{
		   /* In order to continue commands that are terminated
		    * by the end-of-line the newline must be preceeded
		    * by a hyphen (acts similar to backslash newline
		    * in C macros).  Sets flag used in rule above.
		    */
		   if (need_end_of_line_token) {
		      got_line_continue = 1;
		      /* ignore - no return */
		   } else {
		      /* process normally as single character */
		      return (process_token ((int)*yytext));
		   }
		}

<SQL>\\$	{
		   /* Also accept C macro style line continuation */
		   if (need_end_of_line_token) {
		      got_line_continue = 1;
		      /* ignore - no return */
		   }
		}

	/**** stray backslash (TBD: allow quoted characters?) ****/
<SQL>\\		{ /* ignore */ }


	/**** whitespace ****/
<SQL>{WS}+	{ /* ignore */ }

	/**** semicolon ****/
<SQL>;		{
		   /* SQL end-of-statement character */
		   if (need_end_of_line_token) {
		      return (process_token ((int)*yytext));
		   } else {
		      return (process_end_token ((int)*yytext, 0));
		   }
		}

	/**** "Run command" Slash ****/
<SQL>^\/{WS}*$	{
		   if (esql_mode) {
		      /* run slash only valid in standalone SQL */
		      return (process_token ((int)*yytext));
		   } else {
		      return (process_end_token (OP_SLASH, 0));
		   }
		}

	/**** a '[' that is not part of a VARCHAR_ARRAY ****/
<SQL>\[		{ if (inside_host_variable_reference) {
		    sql_scanner_state(HOST_ARRAY);
		    bracket_nesting = 1;
		  }
		  return process_token('[');
		}

	/**** any single operator or punctuation character ****/
<SQL>.		{ return (process_token ((int)*yytext)); }

	/**** At end-of-file (called after yywrap returns done) */
<<EOF>>		{
                   if (got_eof) {
		      /* needed to avoid infinite scanner loop */
		      yyterminate ();
		   } else {
		      got_eof = 1;
		      need_end_of_line_token = 0;

		      switch (sql_scanner_current_state) {
		       case SQL:
		       case SCAN:
			 /* good */
			 break;

		       case SQLSTRING:
		       case SCANSTRING:
			 iff_error ("Unexpected end-of-file inside string");
			 break;
			 
		       default:
			 iff_error ("Unexpected end-of-file");
			 break;
		      }

		      /* Pass EOF to parser.
		       * Needed for empty files and files that
		       * end without a newline.
		       * Ignored otherwise.
		       */
		      return (process_start_token (END_OF_FILE, 0));
		   }
		}

%%

/*
 * postprocess - perform any cleanup before returning token to parser
 */
static int postprocess (int token_type)
{
   sql_identifier_next = 0;
#if THINKAGE_YAY == 1
   /* Set with 2 so becomes 1 (i.e. set) during current parser processing
    * and cleared to 0 on the next return to the parser.
    *
    * TBD: add a preprocess() to clear flag and avoid this hack.
    */
   if (sql_keyword_flag > 0) {
      --sql_keyword_flag;
   }
#endif
   return (token_type);
}

/*
 * process_token - Add non-identifier token to token list and return value.
 */
static int process_token (int token_type)
{
   add_token (0);
   reset_start_of_statement ();
   if (scanner_generic_mode) {
      token_type = GENERIC_TOKEN;
   }
   postprocess (token_type);
   return (token_type);
}

/*
 * process_symbol_token -
 * Add identifier token to token list and return value.
 */
static int process_symbol_token (int token_type, SQL_SYM * sym)
{
   add_token (sym);
   reset_start_of_statement ();
   postprocess (token_type);
   return (token_type);
}

/*
 * process_keyword - Add to token list, add keyword entry in IFF, return value.
 *
 * sym is passed because some keywords also function as identifiers
 * (e.g. ROWID, SYSDATE, SELECT)
 */
static int process_keyword (int token_type, SQL_SYM * sym)
{
   token *t = add_token (sym);

   /* output keyword in ELS file */
   iff_keyword (t);

   reset_start_of_statement ();
   if (scanner_generic_mode) {
      token_type = GENERIC_KEYWORD;
   }
#if THINKAGE_YAY == 1
   else {
      sql_keyword_flag = SET_PROCESS_FLAG;
   }
#endif
   postprocess (token_type);
   return (token_type);
}

/*
 * process_start_token - handle start of statement tokens
 *
 * Add non-identifier token to token list and return value.
 */
static int process_start_token (int token_type, int keyword_flag)
{
   token *t = add_token (0);

   /* output keyword in ELS file */
   if (keyword_flag) {
      iff_keyword (t);
   }
   sql_keyword_start_of_statement (1);
   start_of_statement = 1;
   scanner_generic_mode = 0;
   postprocess (token_type);
   return (token_type);
}

/*
 * process_end_token - handle end of statement tokens
 *
 * Add non-identifier token to token list and return value.
 */
static int process_end_token (int token_type, int keyword_flag)
{
   /* first do start logic for next statement */
   token_type = process_start_token (token_type, keyword_flag);

   /* handle any scanner state change */
   if (esql_mode) {
      sql_scanner_state (sql_scanner_start_state);
   }

   /* postprocess already called in process_start_token() above */
   return (token_type);
}

/*
 * reset_start_of_statement - Not at start of a statement
 */
static void reset_start_of_statement (void)
{
   if (start_of_statement) {
      start_of_statement = 0;
      sql_keyword_start_of_statement (0);
   }
}

#if 0
void tmp_debug (void)
{
}
#endif

/*
 * process_string - Add string-type token to token list and return value.
 *
 */
static int process_string (int multiline)
{
   int token_type = STRING_LITERAL;
   SQL_SYM * sym = 0;

   /* strings in double-quotes are identifiers (in Oracle) which may contain
    * spaces and other normally non-allowed characters
    */
   if (*yytext == '"') {
      /* check for existing symbol
       * This removes quotes before checking,
       * but doesn't convert to uppercase as per Oracle spec.
       */
      if (dbvendor != DBVENDOR_INFORMIX) {
	 sym = sql_symbol_lookup (yytext);
	 token_type = SYM_IDENTIFIER;
      }
   }

   if (sym) {
      token_type = process_symbol_token (token_type, sym);
   } else {
      token_type = process_token (token_type);
   }

   /* for multi-line strings override position information
    * and add a blank token for the end position
    */
   if (multiline) {
      unsigned int token_no;
      token * tend;
      /* string token is in yylval */
      token * t = get_token (yylval.start_token);
      if (t && (t->lineno != start_string_line)) {
	 t->lineno = start_string_line;
	 t->column = start_string_column;

	 /* blank token with end position */
	 tend = new_token (strdup(""), &token_no);
	 if (tend) {
	    yylval.end_token = token_no;
	    tend->lineno = yylineno;
	    tend->column = get_end_file_offset () - newline_offset + 1;
	 }
      }
   }

   /* no need to call postprocess here since it's called above */
   return (token_type);
}

/*
 * process_identifier
 *
 * Check if identifier is a keyword, if so process it.
 * Otherwise, check if identifier is already in the symbol table,
 * and if so return that info also.
 */
static int process_identifier (void)
{
   int token_type;
   SQL_SYM * sym;

   /* check if keyword */
   if ( ( ! sql_identifier_next ) &&
       (token_type = sql_keyword_lookup (yytext))) {

#if SQL_DEBUG != 0
      if (sql_debug_level >= 3) {
	 fprintf (stderr, "<Key: %s: %d>\n", yytext, token_type);
      }
#endif

      /*
       * Resolve conflict betwixt logical-AND and BETWEEN-AND.
       * See grammar (sql.y) for more information.
       *
       * Note this won't work if the first BETWEEN expression
       * has a logical AND in it.  However, I don't believe that
       * is legal since the BETWEEN expressions have to be values
       * (e.g. numeric or dates).
       */
      if (token_type == Y_BETWEEN) {
	 /* found BETWEEN - set flag to handle next AND special */
	 between_flag = 1;
      } else if (between_flag && (token_type == Y_AND)) {
	 /* handle AND as part of last BETWEEN */
	 token_type = OP_BETWEEN_AND;
	 /* clear flag to resume normal logical-AND processing */
	 between_flag = 0;
      }
      
      /* lookup keyword in symbol table in case it is also used as an
       * identifier
       */
      sym = sql_symbol_lookup (yytext);
      token_type = process_keyword (token_type, sym);

   } else {
      token_type = process_identifier_only ();
   }

   /* no need to call postprocess here since it's called above */
   return (token_type);
}

/*
 * process_identifier_only
 *
 * Don't Check if identifier is a keyword.
 * Check if identifier is already in the symbol table,
 * and if so return that info also.
 */
static int process_identifier_only (void)
{
   int token_type;
   SQL_SYM * sym;

   /* check for existing symbol */
   if (sym = sql_symbol_lookup (yytext)) {

      /* Due to scope - return generic identifier
       * even though token is known.  This is 
       * because a name can be redefined in a more inner
       * scope.  E.g. within a loop or local function.
       *
       *      x CHAR(5);      -- x is a char variable
       *      FOR x in 1..10
       *         LOOP
       *              -- in here x is an integer
       *         END LOOP
       *
       * So the grammar rule for any identifier needs to
       * be generic.
       */

      token_type = process_symbol_token (SYM_IDENTIFIER, sym);

   } else {
      /*
       * New identifier.
       *
       * Return with no symbol information.
       * Parser will set symbol information.
       */
      token_type = process_token (SYM_IDENTIFIER);
   }
   postprocess (token_type);
   return (token_type);
}


/*
 * process_host_variable
 *
 * Check if encountered before, if so return with symbol information,
 * otherwise just return it.
 */
static int process_host_variable (void)
{
   SQL_SYM * sym = 0;
   int token_type;
   char * s;

   if (scanner_generic_mode) {
      token_type = GENERIC_TOKEN;
      postprocess (token_type);
   } else {
      /* create host variable name (add 1 for colon and 1 for nul) */
      s = malloc (yyleng + 2);
      if (s) {
	 *s = ':';
	 strcpy (s+1, yytext);
	 sym = sql_symbol_global_lookup (s);
	 /* above allocates its own storage so free this one */
	 free (s);
      }
      
      /* set replace flag to indicate that this symbol's AST should be
       * replaced with the host AST, since only the host parser
       * knows about its variables (scope, type, etc).
       *
       * If not in esql then this is a host variable from some
       * database application (e.g. Oracle Forms) and will be
       * treated like an externref.
       */
      if (esql_mode && sym) {
	 sym->sym_replace = 1;
      }
      token_type = process_symbol_token (SYM_HOST_VARIABLE, sym);
   }

   /* no need to call postprocess here since it's called above */
   return (token_type);
}

/*
 * process_pct_token
 *
 * Handle percent directives (e.g. %FOUND) as keywords since they
 * cannot be identifiers (% is an illegal identifier character).
 * 
 */
static int process_pct_token (void)
{
   int token_type;

   /* check table */
   if (token_type = sql_keyword_lookup (yytext)) {
      process_keyword (token_type, 0);
   } else {
      /* error -- percent directive not found */
      yyerror ("unknown percent directive");

      token_type = process_token (*yytext);
   }

   /* no need to call postprocess here since it's called above */
   return (token_type);
}


/*
 * handle_newline - update line counter and set offset
 */
static void handle_newline (int copyit)
{
   size_t position = get_end_file_offset ();

   /* save last column position of this line */
   icolumn_last = position - newline_offset;
   if (icolumn_last < 1) {
      icolumn_last = 1;
   }

   /* update global and current file (such as an include file) line counters */
   ++yylineno;
   ++ilineno;

   /*
    * Point to first character of new line (hence the + 1),
    * so that the column can be calculated for
    * subsequent text on that line.
    */
   newline_offset = position + 1;

   if (esql_mode && copyit && !inside_host_variable_reference) {
      do_output_newline ();
   }
}

/*
 * increment_lineno - update line counter for embedded newlines in text
 */
static void increment_lineno (char* text)
{
   char* s;
   for (s = text; s=strchr(s,'\n'); ++s) {
      handle_newline (0);
   }
}


/*
 * yywrap
 *
 * Note: yywrap is the name for the standard programmer-supplied yacc routine
 * called after encountering input end-of-file.  Returning 1 means
 * done, 0 means a new input source has been setup, keep processing.
 */
int yywrap (void)
{
   int done = 1;

   if ( ! got_eof ) {
      if (esql_mode) {
	 /* check include file stack -- if any */
	 if (esql_include_level) {
	    /* more input */
	    done = 0;
	    esql_pop_include ();
	 } else {
	    esql_del_include ();
	 }
      }

      if (done) {
	 /* wrapup any scanner state info */
	 sql_scanner_state_wrap ();
      }
   }

   /* 1 means finished - no more input */
   return (done);
}

/*
 * format_line_source
 *
 * Set passed buffer with current line number and column.
 * This is used for IFF entries (especially errors).
 */
void format_line_source (char * buffer)
{
   sprintf (buffer, "%d/%d", ilineno, sql_get_icolumn ());
}

#if THINKAGE_YAY == 1
/*
 * sql_check_keyword_error -  check for keyword parser error and retry
 *
 * Thinkage YAY (Yet Another Yacc) allows for retry of grammar with another
 * token value (yychar) after call to yyerror().  This is useful since
 * keywords can be used as identifer names.
 *
 * This routine checks if last return to parser was a keyword
 * and that yyerror was called by parser and not some other routine.
 * If so, it changes the token value to an identifer.
 *
 * Returns:	1 - token changed (yyerror should not print message)
 *		0 - no change
 *
 * Uses:
 *	yychar - token value in parser returned by yylex
 *	msg - message passed to yyerror() ("Syntax error" if parser call)
 *	sql_keyword_flag - set by scanner if last token was keyword
 *	scanner_generic_mode - set by scanner if in generic mode
 */
static sql_check_keyword_error (const char* msg)
{
   EXTERN_C int yychar;
   int retval = 0;	/* don't retry with identifier */
   if ((!scanner_generic_mode) &&
       sql_keyword_flag &&
       (!strcmp (msg, PARSER_SYNTAX_MESSAGE))) {
      /* retry with identifier */
      retval = 1;
      sql_keyword_flag = 0;
      yychar = SYM_IDENTIFIER;
   }
   return (retval);
}
#endif /* THINKAGE_YAY == 1 */

/*
 * yyerror
 *
 * Uses the message passed along with the current token and calls
 * the IFF error routine to create an error entry.
 *
 * Note: yyerror is the name for the standard programmer-supplied yacc
 * error routine.  It is called whenever an error is encountered, such
 * as a parse (syntax) error, i.e. the next token does not fit in
 * the grammar.  It is also called above.
 */
int yyerror (const char* msg)
{
   char error_buffer [1024];

   if ( ! sql_allow_error ) {
#if THINKAGE_YAY == 1
      if (!sql_check_keyword_error (msg)) 
#endif
      {
	 /* call iff_error with message and token text
	  * which will print error on stderr and in IFF
	  * 
	  * yytext can get big in the case of multi-line strings
	  * avoid overflowing error_buffer.
	  */
         sprintf (error_buffer, "%s at %.800s", msg, yytext);
	 iff_error (error_buffer);
      }
   }

   return (0);
}

/* switch to SQL state (initial scanner state for sql and pl/sql) */
int sql_scanner_sql_mode (void)
{
   sql_scanner_start_state = SQL;
   sql_scanner_state (sql_scanner_start_state);
   return (0);
}

/* switch to SCAN state (initial scanner state for esql) */
int sql_scanner_scan_mode (void)
{
   sql_scanner_start_state = SCAN;
   sql_scanner_state (sql_scanner_start_state);
   return (0);
}


/* place where first state is specified and any initial processing
 * based on state occurs.
 */
static int sql_scanner_init_state (int state)
{
   if (esql_mode) {
      if (state == SCAN) {
	 /* initialize mapping */
	 iff_map_initial ();
      }
   }
   sql_scanner_state (state);
   return (0);
}

/* sql_scanner_state --
 *
 * Wrapper around lex BEGIN command to change to a new scanner state
 * sets sql_scanner_current_state to new state.
 *
 * Any state transistion processing can go here.
 */
static int sql_scanner_state (int state)
{
   sql_scanner_current_state = state;
   BEGIN state;
   return (0);
}

/* finial state postprocessing */
static int sql_scanner_state_wrap (void)
{
   return (0);
}

/*
 * sql_process - Process input (SQL program).
 *
 * Initialize variables and start new file
 */
int sql_process (FILE * fp_in)
{
   int ret;

#if 0
extern int yydebug;
yydebug = 1;
#endif 

   /*
    * yy_init is the flex initialization flag.
    * If set it indicates to flex to do its initialiation.
    * It starts at 1, so the cleanup below only happens
    * after the first file.
    *
    * Note: when normally called, only one file is passed so this
    * block is only called for testing, etc.
    */
   if ( ! yy_init ) {
      /*
       * Tell flex to do its initialiation.
       */
      yy_init = 1;

      /* line number (one-relative) */
      yylineno = 1;
      ilineno = 1;

      /* file offset */
      current_offset = 0;
      next_offset = 0;
      newline_offset = 0;

      /* Include file level.  Zero is main file. */
      esql_include_level = 0;

      /* ast root */
      /* TBD: free this */
      ast_root = 0;

      /* newline terminated directives */
      need_end_of_line_token = 0;

      /* clear generic token mode */
      scanner_generic_mode = 0;

      /* handle errors normally */
      sql_allow_error = 0;

      /* not processing an sql trigger */
      sql_inside_trigger = 0;

      /* error count */
      sql_errcount = 0;

      /* between clause flag (see note on top) */
      between_flag = 0;

#if THINKAGE_YAY == 1
      sql_keyword_flag = 0;
#endif

      /* cleanup previous token list */
      free_token_list ();

      /* at beginning of a statement */
      sql_keyword_start_of_statement (1);
      start_of_statement = 1;

      /* clear command continuation status */
      got_line_continue = 0;

      /* clear eof status */
      got_eof = 0;

      /* clear nesting level of C host program */
      scope_nesting_level = 0;

      /* clear varchar cast flag */
      varchar_cast = 0;

      /* reinitialize parse buffer */
      yyrestart (fp_in);
   }

   /* setup scanner and keyword state based on ESQL mode */
   if (esql_mode) {
      sql_scanner_start_state = SCAN;
      sql_set_keyword_mode (SQLKEY_ALL_ESQL);
   } else {
      sql_scanner_start_state = SQL;
      sql_set_keyword_mode (SQLKEY_ALL);
   }
   sql_scanner_init_state (sql_scanner_start_state);

   /* flex input -- zero means stdin */
   yyin = fp_in;

   /* routine to calculate token position */
   get_icolumn = &sql_get_icolumn;

   /*
    * This is where it all happens.
    *
    * Call the parser: reads and parses whole file until EOF.
    */
   ret = yyparse ();
 
#if SQL_DEBUG != 0
   if (ret) {
      if (sql_debug_level >= 1) {
	 fprintf (stderr, "yyparse failed, returned: %d\n", ret);
	 fprintf (stderr, "Errors found: %d\n", sql_errcount);
      }
   } else if (sql_errcount) {
      if (sql_debug_level >= 1) {
	 fprintf (stderr, "Errors found: %d\n", sql_errcount);
      }
      ret = 1;
   }
#else
   if ((!ret) && sql_errcount) {
      ret = 1;
   }
#endif
 
   return (ret);
}

/*
 * Calculate file offset of yytext.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * yy_ch_buf -- the flex input buffer.
 * yytext_ptr -- current position in buffer (same as yytext).
 */
static size_t get_start_file_offset (void)
{
   size_t offset;
   if (yytext && yy_current_buffer) {
      offset = current_offset + (yytext - yy_current_buffer->yy_ch_buf);
   }
   else offset = 0;
   return (offset);
}

/*
 * Calculate file offset of end of yytext.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * yy_ch_buf -- the flex input buffer.
 * yy_c_buf_p -- next position in buffer (one after end of yytext).
 */
static size_t get_end_file_offset (void)
{
   size_t offset = current_offset +
	 (yy_c_buf_p - yy_current_buffer->yy_ch_buf) - 1;
   return (offset);
}

/*
 * Calculate column on the current line of yytext.
 *
 * newline_offset -- the file offset of the first character of the
 * current line (one past the newline char).  Add one to make column
 * a one-relative number.
 */
static int sql_get_icolumn (void)
{
   size_t column = get_start_file_offset () - newline_offset + 1;
   return (column);
}

/* Function pointer to above routine */
int (*get_icolumn)(void) = &sql_get_icolumn;

/*
 * struct for pushing and popping include files
 *
 * contains the state info needed to switch back to reading from
 * previous file.
 */
typedef struct include_spec {
   struct include_spec * prev;		/* linked-list */
   void * yybuffer;			/* scanner text buffer */
   FILE * fp;				/* input file handle */
   const char * filename;		/* filename */
   SQL_SYM * sym_file;			/* symbol table entry for filename */
   unsigned int lineno;			/* save line number */
   unsigned int ilineno_start;		/* input file line number */
   unsigned int olineno_start;		/* output file line number */
   size_t current_offset;		/* save position in yybuffer */
   size_t next_offset;			/* save position in yybuffer */
   size_t newline_offset;		/* save position in yybuffer */
} include_spec;

/* the include file stack */
static include_spec * include_stack = 0;

/* initialize include stack with main file */
static int esql_init_include (void)
{
   include_spec * is = malloc (sizeof (include_spec));
   int ret_val = -1;

   if (is) {
      ret_val = 0;
      /* first entry */
      include_stack = is;

      /* link */
      is->prev = 0;

      /* lex input text buffer */
      is->yybuffer = YY_CURRENT_BUFFER;

      /* yyin is lex input file i/o handle */
      is->fp = yyin;

      /* main filename */
      if (sql_infilename) {
	 is->filename = strdup (sql_infilename);
      } else {
	 is->filename = strdup ("");
      }

      /* symbol table entry of main file */
      is->sym_file = sql_symbol_main_file;

      /* current input line number */ 
      is->lineno = ilineno;

      /* starting input and output files line numbers
       * these are 1 since this is the first file
       */
      is->ilineno_start = 1;
      is->olineno_start = 1;

      /* current scanner pointers into lex buffer */
      is->current_offset = current_offset;
      is->next_offset = next_offset;
      is->newline_offset = newline_offset;
   }
   return (ret_val);
}

/* delete main file include stack entry
 * and finish mapping entry
 */
static int esql_del_include (void)
{
   unsigned int last_lineno;

   /* check if last line was terminated with a newline */
   if (sql_get_icolumn () > 1) {
      /* no -- use current line number */
      last_lineno = ilineno;
   } else {
      /* yes -- line number has been incremented past end */
      last_lineno = ilineno - 1;
   }

   /* check for stack
    * if found map last portion of main file and free head
    * else (no includes) map full file
    */
   if (include_stack) {
      iff_mapping_lines (include_stack->filename,
			 include_stack->ilineno_start,
			 last_lineno,
			 include_stack->olineno_start);

      free ((void*) (include_stack->filename));
      free (include_stack);
      include_stack = 0;
   } else {
      iff_mapping_lines (sql_infilename, 1, last_lineno, 1);
   }
   esql_include_level = 0;
   return (0);
}

/* esql_push_include
 *
 * Open include file and save state info
 */ 
int esql_push_include (const char * fname)
{
   include_spec * is = malloc (sizeof (include_spec));
   FILE * new_yyin;
   void * new_buff = 0;
   SQL_SYM * sym = 0;
   const char * filename = strdup (fname);
   int ret_val = 0;

   if ( ! is ) {
      /* out of memory */
      ret_val = -1;
   } else if ( ! include_stack ) {
      /* first time -- initialize stack with main file */
      ret_val = esql_init_include ();
   }

   /* open new include file */
   if ((ret_val == 0) && (new_yyin = fopen (filename, "r")) == 0) {
      ret_val = -1;
      iff_error ("Can't open include file");
   }

   /* output mapping entry for previous file */
   if (ret_val == 0) {
      if (include_stack) {
	 /* newline for line with include has already been scanned
	  * (and counted) so subtract one from input line number.
	  */
	 iff_mapping_lines (include_stack->filename,
			    include_stack->ilineno_start,
			    ilineno - 1,
			    include_stack->olineno_start);
      }
   }

   /* output entries for filename and set include relation for symbol */
   if (ret_val == 0) {
      /* generate symbol table entry for this filename */
      sym = iff_file (filename);
      iff_source_filename (filename);
      if (sym && include_stack) {
	 iff_include_relation (include_stack->sym_file, sym);
      }
   }

   /*
    * Do the lower level lex switch
    * set scanner input to use new file and allocate new buffer
    */
   if ((ret_val == 0) && (new_buff = push_include (new_yyin)) == 0) {
      ret_val = -1;
   }

   /* save current scanner state on include stack */
   if (ret_val == 0) {
      /* include file depth */
      esql_include_level++;

      /* link list */
      is->prev = include_stack;

      /* scanner and file state info */
      is->yybuffer = new_buff;
      is->fp = new_yyin;
      is->filename = filename;
      is->sym_file = sym;
      is->lineno = ilineno;
      is->ilineno_start = 1;
      is->olineno_start = sync_linenos(1);

      is->current_offset = current_offset;
      is->next_offset = next_offset;
      is->newline_offset = newline_offset;

      /* new head */
      include_stack = is;

      /* input line number of current file (one-relative) */
      ilineno = 1;
   }
   return (ret_val);
}

/* esql_pop_include
 *
 * Routine called when EOF is reached on an include file.
 * Close current include file and restore previous file as input
 * from info stored on include stack.  Free old entry.
 */
static int esql_pop_include (void)
{
   int ret_val = 0;
   include_spec * is;

   if (include_stack) {

      if (esql_include_level > 0) {

	 /* decrement depth */
	 esql_include_level--;

	 /* pop stack */
	 is = include_stack;
	 include_stack = is->prev;

	 /* output mapping entry for popped file */
	 iff_mapping_lines (is->filename,
			    is->ilineno_start,
			    ilineno - 1,
			    is->olineno_start);
 
	 /* restore previous file */
	 if (include_stack) {
	    /* switch to prev lex input buffer and file handle
	     * also frees old buffer
	     */
	    ret_val = pop_include (include_stack->yybuffer, include_stack->fp);

	    /* iff entry for restored filename */
	    iff_file_reference (include_stack->filename);

	    /* error reporting filename */
	    iff_source_filename (include_stack->filename);

	    /* save current line positions for mapping later */
	    include_stack->ilineno_start = is->lineno;
	    include_stack->olineno_start = sync_linenos(is->lineno);
	 }

	 /* close old file */
	 fclose (is->fp);

	 /* restore current input line number */
	 ilineno  = is->lineno;

	 /* restore offsets into lex buffer */
	 current_offset  = is->current_offset;
	 next_offset  = is->next_offset;
	 newline_offset  = is->newline_offset;

	 /* free popped include stack entry */
	 free ((void*) (is->filename));
	 free (is);
      }
   }
   return (ret_val);
}    

/* call lex routines to switch to a new input file
 * returns newly allocated lex input buffer
 */
static void * push_include (FILE * new_yyin)
{
   void * new_buff = 0;
   if (new_yyin) {
      /* flex routine to allocate new text input buffer
       * YY_BUF_SIZE is flex macro
       */
      new_buff = (void*) yy_create_buffer (new_yyin, YY_BUF_SIZE);
      if (new_buff) {
	 /* flex routine to change input buffer */
	 yy_switch_to_buffer (new_buff);
	 /* yyin is flex global with input file i/o descriptor */
	 yyin = new_yyin;
      }
   }
   return (new_buff);
}

/* call lex routines to switch to previous input file
 * passed buffer and input file handle to switch to,
 * frees current buffer
 */
static int pop_include (void * old_buff, FILE * old_yyin)
{
   int ret_val = 0;
   /* flex current buffer macro */
   void * prev_buff = YY_CURRENT_BUFFER;
   if (old_buff) {
      /* flex routine to restore old buffer */
      yy_switch_to_buffer (old_buff);
      /* yyin is flex global with input file i/o descriptor */
      yyin = old_yyin;
   }
   /* flex routine to free buffer */
   yy_delete_buffer (prev_buff);
   return (ret_val);
}
	 

/************************************************************************/

/*
 * Most of the following is taken from esql.l by wmm,
 * BUT with modifications, so the bugs are mine.
 *				- Mark B. Kaminsky
 */


/* Tokens are kept for the entire duration of the run,
* for simplicity.  The following structure contains a pointer to a
* dynamically-allocated array of 8000 tokens.  Instances of this
* struct are kept in a singly-linked (LIFO) list, rooted in
* "curr_token_list."  Looking up a token by number involves scanning
* for the token_list instance and indexing, offset by the first token
* number contained in the list.
*/

typedef struct token_list {
 struct token_list* prev;
 unsigned int first_token;
 unsigned int next_token;
 token* tokens;
} token_list;

#define TOKENS_PER_LIST 8000

token_list* curr_token_list = 0;

/* The following function creates a new token_list instance and links
* it into the list.  If this is the first token_list instance, its
* first_token will be 0; otherwise, it will be the next_token from the
* previous instance.
*/

static void new_token_list() {
 token_list* p = (token_list*) malloc(sizeof(token_list));

 /* MBK 1997/11/01 - Added !p check */
 if (!p) {
   perror ("Allocating token list");
   iff_error ("Allocating token list");
   YY_FATAL_ERROR ("Out of memory");
 }
 p->prev = curr_token_list;
 p->first_token = (curr_token_list) ? curr_token_list-> next_token : 0;
 p->next_token = p->first_token;
 p->tokens = (token*) malloc(TOKENS_PER_LIST * sizeof(token));

 /* MBK 1997/11/01 - Added !p->tokens check */
 if (!p->tokens) {
   perror ("Allocating token buffer");
   iff_error ("Allocating token buffer");
   YY_FATAL_ERROR ("Out of memory");
 }
 curr_token_list = p;

 /* MBK 1997/11/01 - Added this token zero stuff */
 /* special token zero is used to flag an empty grammar rule */
 if (p->first_token == 0) {
   /* tests curr_token_list - so call after setting curr_token_list */
   add_token_from ("", 0);
 }
}

/* The following two functions create tokens and add them to the list.
* The text to be added is always in memory that is explicitly
* malloced, so it can/must be freed when the token list is emptied
* upon initialization.
*
* MBK 1997/11/01 - Added symbol and syntax tree pointers.
* Added column and offset.
*/

static token* add_token (SQL_SYM * sym) {
 return (add_token_from(strdup(yytext), sym));
}


static token* add_token_from(const char* txt, SQL_SYM * sym) {
 unsigned int token_no;
 token *t = new_token (txt, &token_no);

 yylval.start_token = yylval.end_token = token_no;
 yylval.sym = sym;
 yylval.ast = 0;
 return (t);
}

token* new_token (const char* txt, unsigned int * ptoken_no)
{
 unsigned int offset;
 token *t;
 if (!curr_token_list || curr_token_list->next_token >=
	 curr_token_list->first_token + TOKENS_PER_LIST) {
     new_token_list();
 }
 *ptoken_no = curr_token_list->next_token++;
 offset = (*ptoken_no) - curr_token_list->first_token;
 t = &(curr_token_list->tokens[offset]);
 t->text = txt;
 t->olineno = yylineno;
 t->lineno = ilineno;
 t->file_offset = get_start_file_offset ();
 t->column = sql_get_icolumn ();

 return (t);
}

/* The following function returns a pointer to the token identified by
* the specified token number.
*/

token* get_token(unsigned int token_no) {
 token_list* p;

 /* Find the appropriate token_list instance */

 for (p = curr_token_list; p && p->first_token > token_no; p = p->prev)
     { }

 if (p) {
     return &p->tokens[token_no - p->first_token];
 }
 return 0;
}

/* Clean up the token list from the preceding run.  It
* frees all but the first (i.e., token 0) token_list instance, and
* initializes that instance to be empty.
*/

void free_token_list() {
 token_list* p = curr_token_list;
 while (p) {
     p = p->prev;
     if (p) {
	 /* MBK TBD: need to free token text */

	 free(curr_token_list->tokens);
	 free(curr_token_list);
	 curr_token_list = p;
     }
     else {
	 /* MBK TBD: need to free token text */

	 /* token zero is reserved */
	 curr_token_list->next_token = 1;
     }
 }
}

/* discard_comment ---
*
* The following function reads until the end of a comment; if it
* encounters EOF along the way, it passes that back to the caller.
*
* MBK 1998/02/10 - Added comment_input(), copyit and handle_newline().
* MBK 1997/11/01 - Added error msg about EOF.
*/

static int comment_input (int copyit)
{
   int ch;
   if ((ch = input()) != EOF) {
      if (ch == '\n') {
	 /* We always copy out newlines in comments, to keep the
	  * output line count in sync.  -- wmm, 990308.
	  */
	 handle_newline (1);
      } else if (copyit) {
	 do_output_chr (ch);
      }
   }
   return (ch);
}

static int discard_comment(int copyit)
{
int ch;
for (;;) {
   while ((ch = comment_input(copyit)) != '*' && ch != EOF) {
   }
   if (ch == '*') {
      while ((ch = comment_input(copyit)) == '*') {
      }
      if (ch == '/') {
	 break;
      }
      if (ch == EOF) {
	 iff_error ("EOF before end of comment");
	 break;
      }
   }
}
return ch;
}
