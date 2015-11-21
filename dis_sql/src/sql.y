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
 * S Q L . Y
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the yacc (really bison) grammar rules and action code
 * to process SQL (Structured Query Language) programs in the following forms:
 *     Oracle and Informix SQL scripts.
 *     Oracle's PL/SQL (Procedural Language / Structured Query
 *        Language) up to version 7.3.  As well as the Informix
 *        equivalent.  These programs are refered to as
 *        "Stored Procedures".
 *     Embedded SQL (ESQL) C/C++ programs.  These are C or C++ programs
 *        (the "host") with SQL statements with are transformed by the Oracle
 *        or Informix preprocessor before being passed to the C/C++ compiler.
 *
 * The purpose of this parser (along with its associated routinues) is
 * to produce an ELS (External Language Support) IFF (Internal Format File).
 *
 * This parser uses the flex scanner (in sql.l) to read in the
 * input program(s) and transform it into lexical tokens 
 * which are either keywords, literals, or identifiers.  The
 * structure of the productions below is in sql_scanner.h.
 * During parsing, symbols (identifiers) are tracked and an internal
 * Abstract Syntax Tree (AST) is built.  This tree also
 * contains infomation about symbol relationships.  The ELS output file
 * contains keyword entries produced by the scanner as well as symbol
 * and syntax entries generated here.
 *
 * Before a keyword is returned by the scanner it undergoes a number
 * of tests based on the current context.  See note before the token
 * list below before adding a token or changing any grammar.
 *
 * This parser is called by a driver which also calls routinues
 * to open input/output files and generate the IF file after parsing.
 *
 * One weakness of this parser (which is not the case inside
 * Oracle) is that it has no information from the database.  It
 * therefore, does not know the symbol types of external references
 * (in "SELECT x FROM y", x could be either a column or a function
 * that takes no arguments).  In addition,
 * because of this, a symbol's datatype is often unknowable. A lot of
 * what goes on below has to do with trying to figure out what a
 * symbol is.  Local symbols do not have this problem.
 *
 * In general, this parser is meant to be fed working programs.
 * When it encounters a syntax error, it generates an error message
 * entry, and then continues with the next statement (after the next
 * semi-colon or, in a few cases, some other token(s) like close-parenthesis).
 *  In addition this parser will accept much that is incorrect,
 * because of limited semantic checking (e.g. passing a date to a
 * numeric function).
 */

#ifndef _WIN32
#include <alloca.h>		/* needed for bison */
#endif
#include <stdio.h>

#include "sql_symbol.h" 	/* symbols (SQL_SYM) */
#include "sql_scanner.h"	/* yylval and tokens */
#include "sql_iff.h"		/* output routines */
#include "sql_ast.h"		/* syntax tree (ast_tree) */
#include "sql_resolve.h"	/* resolve symbols */
#include "sql_keyword.h"	/* keyword handling */
#include "sql_host.h"		/* output to host (C/C++) file */

#if SQL_DEBUG	!= 0
extern int sql_debug_level;
/* Allow debugger to find static routines */
#define STATIC
#else
#define STATIC static
#endif /* ! SQL_DEBUG */

/* esql: output varchar declarations */
/* #define DO_VARCHAR_DECL */

/* Root of ast (Abstract Syntax Tree)
 * built of nodes for each grammar rule.
 */
ast_tree * ast_root = 0;

/*
 * Variables for parser-->scanner communication.
 *
 * These change what the scanner might return.
 * Care should be taken when setting/clearing these
 * that scanner lookahead is not needed (e.g. rules
 * with only single tokens, or parallel structures).
 *
 * need_end_of_line_token -	Return newline, normally ignored as whitespace.
 * scanner_generic_mode -	Return only "generic" tokens.  Used for
 *				ignoring misc SQL commands, but still process
 *				some low-level tokens like strings.
 * sql_allow_error -		Allows (does not report) a parse error.  Used
 *				to ignore SQL commands.
 * sql_inside_trigger -		Processing trigger.  Handle :new and :old.
 * sql_identifier_next -       	Treat next keyword as an identifier
 */
int need_end_of_line_token = 0;
int scanner_generic_mode = 0;
int sql_allow_error = 0;
int sql_inside_trigger = 0;
int sql_identifier_next = 0;
int inside_host_variable_reference = 0;

extern FILE *symbol_specification_file;	/* in sql_driver.C */

/* grammar "global" variables */
static SQL_SYM * record_sym = 0;        /* record symbol for field defs */
static const char * package_name = 0;	/* package name for members */
const char * last_table_name = 0;	/* table name for columns */
static const char * trigger_table_name = 0;	/* table name for trigger */
static char txtbuf [1024];		/* temp text buffer */
static int parameter_count = 0;		/* current argument number */
static int parameter_attribute = 0;	/* mask for input/output arg */
static int in_specification = 0;        /* in spec: no params defined */
static int for_loop_cursor = 0;         /* cursor loop (not range) */
static int table_count = 0;		/* count of tables in a list */
static ast_tree * table_list = 0;	/* ast list of tables */
static ast_tree * saved_ast_tree = 0;	/* saved ast  */
static sqltype prefix_token;		/* esql cmd start */	
static sqltype saved_token;		/* saved token */	
static in_select_items = 0;		/* processing select item clause */
static int datatype_mask = 0;		/* data attribute mask */
static int datatype_size = 0;		/* data size value */
static int datatype_scale = 0;		/* data scale value*/
static int alternate_check = 0;		/* check for different command */
static int alternate_command = 0;	/* different command flag */

/* The following variable is set to true whenever a "{" is output by
 * one of the actions below to flag the need for a matching "}".
 * Applies only to esql.
 */

static int need_postfix = 0;

/*
 * static routines below
 *
 * Most of these are simple C front-ends for symbol, iff, and ast
 * C++ routines.  See description where they are defined.
 */
SQL_SYM * symbol_set (sqltype t, sym_kind symbol_type);
SQL_SYM * symbol_add (const char * text, sym_kind symbol_type);
SQL_SYM * symbol_extern (sqltype t, sym_kind symbol_type);
STATIC SQL_SYM * symbol_field (SQL_SYM * rec_sym, const char * text);
sym_kind symbol_get (sqltype t);
int symbol_check (sqltype t, sym_kind symbol_type);
int symbol_is_known (sqltype t);
void set_result (sqltype *result,
		 sqltype left,
		 sqltype right,
		 SQL_SYM * sym,
		 ast_tree * ast);
STATIC void error_result (sqltype *result, sqltype left, sqltype right);
STATIC ast_tree * error_node (sqltype left, sqltype right);
void set_empty (sqltype *result);
int is_empty (sqltype t);
ast_tree * symbol_call_stmt (sqltype *tp,
			     sqltype left,
			     sqltype call_right,
			     sqltype stmt_right);
ast_tree * sql_call_stmt (const char * funcname,
			  sqltype left,
			  sqltype call_right,
			  sqltype stmt_right);
ast_tree * symbol_call_expr (sqltype *tp, sqltype left, sqltype right);
ast_tree * sql_call_expr (const char * funcname,
			  sqltype left,
			  sqltype right);
const char* token_text (sqltype t);
SQL_SYM * symbol_type (sqltype t);
SQL_SYM * symbol_type_text (const char * text);
ast_tree * new_tree (ast_node_kind ank,
		     SQL_SYM *sym,
		     sqltype tstart,
		     sqltype tend);
ast_loc token_loc_start (sqltype t);
ast_loc token_loc_end (sqltype t);
ast_tree * empty_tree (ast_node_kind ank);
ast_tree * symbol_token (sqltype t);
ast_tree * symbol_tree (SQL_SYM * sym, sqltype t);
ast_tree * symbol_tree_range (SQL_SYM *sym,
			      sqltype tstart,
			      sqltype tend);
ast_tree * symbol_tree_ast (SQL_SYM *sym,
			    sqltype tstart,
			    sqltype tend);
STATIC ast_tree * symbol_tree_type (SQL_SYM *sym,
   sqltype tstart,
   sqltype tend,
   int refmask);
ast_tree * symbol_reference (sqltype t);
ast_tree * symbol_reference_tree (SQL_SYM * sym, sqltype t);
ast_tree * reference_tree (ast_node_kind ank,
			   SQL_SYM *sym,
			   sqltype tstart,
			   sqltype tend);
ast_tree * reference_type_tree (ast_node_kind ank,
				SQL_SYM *sym,
				sqltype tstart,
				sqltype tend,
				int refmask);
ast_tree * literal_tree (sqltype t);
ast_tree * string_literal_tree (sqltype t);
void ast_relation (ast_tree * at, relation_kind rk);
void ast_relation_attr (ast_tree * at, int reltype);
void ast_relation_arg (ast_tree * at, int arg);
void ast_relation_size (ast_tree * at, int size);
void ast_relation_scale (ast_tree * at, int scale);
void ast_defined (ast_tree * at);
void ast_decldefined (ast_tree * at);
STATIC void ast_replacement (ast_tree * at);
void ast_keyword (ast_tree * at);
STATIC void symbol_attribute (SQL_SYM * sym, int attrtype);
STATIC void sql_keywords_only (void);
STATIC void sql_no_keywords (void);
STATIC void sql_clear_no_keywords (void);
STATIC void sql_error_clear (void);
int sql_specfile_output (SQL_SYM * sym, sqltype left, sqltype right);
STATIC int esql_prefix_check (void);
STATIC ast_tree * varchar_list_decl (sqltype varchar_decl,
				     sqltype varchar_type);
STATIC int varchar_set_end (ast_tree * at_head, sqltype end_token);
STATIC int symbol_keyword (sqltype t);
STATIC int symbol_keyword_check (sqltype t, const char *keyword);
STATIC int symbol_keyword_list (sqltype t, const char **spp);
int sql_tableref (sqltype *tp);
char* token_range_text(unsigned start_tok, unsigned end_tok);

/* keyword lists
 *
 * A number of keywords are returned by the scanner as identifiers.
 * Most of these are keywords that are part of dot separated
 * data names (e.g. tablename.ROWID).  Others are special cases
 * such as transaction type.
 * These lists are used to test whether an identifier is really
 * a keyword in these cases.
 */
static const char * keylist_plsql_table [] = {
   "COUNT", "DELETE", "EXISTS", "FIRST", "LAST", "NEXT", "PRIOR", 0
};

static const char * keylist_pseudo_column [] = {
   "ROWID", "USER", 0
};

static const char * keylist_sequence_value [] = {
   "CURRVAL", "NEXTVAL", 0
};

static const char * keylist_transaction [] = {
   "COMMITTED", "ISOLATION", "LEVEL", "ONLY", "READ", "SEGMENT",
   "SERIALIZABLE", "TRANSACTION", "USE", "WRITE", 0
};

#if YYBISON == 1
/* entities for yykeywordcheck() -- see comments there */
#define YYCHECK_MAX	256		/* size of check keyword stack */

static int yystate_save = 0;		/* yyparse state info */
static int yyn_save = 0;		/* yyparse state info */
static short *yyssp_save = 0;		/* yyparse state info */
static int yyerrstatus_save = 0;	/* yyparse state info */
static short yycheck_stack [YYCHECK_MAX];	/* check keyword stack */
static int yycheck_stack_len = 0;	/* check keyword stack current size */

STATIC int yycharcheck (int yychar, int yystate, int yyn);

/* macro to catch internal yyparse values just before call to scanner */
#define YYLEX	(yyn_save=yyn,\
		 yystate_save=yystate,\
		 yyssp_save=yyssp,\
		 yyerrstatus_save=yyerrstatus,\
		 yylex())
#endif /* YYBISON == 1 */

%}

/************************ Token List ****************************/

/****
 ****
 ****   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE
 ****
 **** Just because a keyword is used in the grammar does NOT mean that the
 **** scanner will return it or will return it in the current context.  The
 **** keyword processing routines (in sql_keyword.C) determine if a name is a
 **** keyword or an identifier.  Keyword processing uses keyword states (SQL
 **** vs PL/SQL), vendor (Oracle vs Informix), current command (CREATE
 **** TABLE, TYPEdef, etc) set here in the parser, as well as other special
 **** processing, along with end-of-statement conditions set by the scanner
 **** (some keywords are only valid at the beginning of a statement).  The
 **** full list of keywords are in a table in sql_keytable.C.  ANY TOKEN
 **** ADDED BELOW NEEDS TO BE ALSO ADDED TO THE TABLE.
 **** 
 ****
 **** A summary of keyword processing is as follows:
 ****	Scanner initiated:
 ****		OP_BETWEEN_AND: Special handling in scanner of the AND
 ****			 that follows a BETWEEN.
 ****		THINKAGE parser: Changing a keyword token after a parser error.
 ****			See yyerror ().
 ****		start/end of statement: sets/clears flags. Certain keywords
 ****			only valid at start of stmt.
 ****	Parser initiated:
 ****		statement mode: SQL, PL/SQL, ESQL
 ****		sql command: SQLKEYC_xxx keywords active only in certain stmt.
 ****		sql_generic_mode: for ignoring SQL commands.
 ****		sql_identifier_next: when it is known that the next token
 ****			must be an identifier.
 ****		sql_no_keywords: allow no keywords, used when scanning a
 ****			pathname.
 ****		keyword_id: grammar rules which contain keywords to be treated
 ****			as identifiers.
 ****		keyword: grammar rule in which an identifier is to be treated 
 ****			as keywords.
 ****		keylist_xxx: keywords that are passed back as identifiers. 
 ****		BISON parser: yykeywordcheck(): checking internal bison tables
 ****			to see if keyword is valid given current grammar state.
 ****	Other:
 ****		vendor check: (in sql_keyword.C) Oracle vs Informix.
 ****		special processing: (in sql_keyword.C) certain keywords only
 ****			valid in special cases or when it immediately follows
 ****			another given keyword (e.g. DOUBLE PRECISION).
 ****
 ****  
 **** Why keyword processing?  Oracle allows keywords as identifiers
 **** in many cases.  Reserved keywords are only reserved in some
 **** contexts.  For example RECORD is a reserved keyword in PL/SQL
 **** but not in standalone SQL, therefore a column name can be
 **** named RECORD (even when referred to in PL/SQL code).  In
 **** practice, however, it is even worse, as Oracle allows
 **** reserved keywords as identifiers (a record called RECORD) even
 **** though Oracle documentation says that that is illegal.
 ****
 **** Warning: <WHINING ALERT ON>
 **** The complexity of the above keyword processing shows the desirability
 **** of having a large test base of real-world customer code before
 **** starting out.  When I started I had only text book examples which
 **** did not break the rules. Only simple keyword processing such as
 **** statement mode was in the original design. Dispite repeated requests,
 **** test code (from the first customer) did not arrive until during 
 **** system test time.  In addition, the initial design called for just
 **** support of PL/SQL and not standalone SQL.
 **** Thus the above reflects the feature creep and bug fixes
 **** as more code arrived from various customers.  Keep this in mind
 **** before supporting another vendor (such as Sybase).
 **** <WHINING ALERT OFF>
 ****
 **** TBD: Simplify the above (left as an exercise for the reader :-).
 ****
 ****
 **** Token keyword names match the actual text as follows:
 **** Y_text for "text", Y_PCT_text for "%text". 
 **** 
 **** "Y_" stands for yacc, "OP_" for operator, "SYM_" for
 **** symbol.  These prefixes also avoid conflict with C/C++
 **** libraries: The SQL keyword NULL would conflict with
 **** the stdio NULL, but Y_NULL does not.
 ****
 ****   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE   NOTE
 ****
 **** Note that the ast node names (e.g. ank_list) are
 **** extracted by a script which generates include and source
 **** files with enumerated values.
 ****/

/* keywords */
%token Y_ACCEPT
%token Y_AFTER
%token Y_ALL
%token Y_ALLOCATE
%token Y_ALTER
%token Y_ANALYSE
%token Y_ANALYZE
%token Y_AND
%token Y_ANY
%token Y_ARRAYLEN
%token Y_AS
%token Y_ASC
%token Y_AT
%token Y_AUDIT
%token Y_AUTHORIZATION
%token Y_AVG
%token Y_BEFORE
%token Y_BEGIN
%token Y_BETWEEN
%token Y_BINARY_INTEGER
%token Y_BIND
%token Y_BODY
%token Y_BOOLEAN
%token Y_BREAK
%token Y_BTITLE
%token Y_BY
%token Y_BYTE
%token Y_CACHE
%token Y_CASCADE
%token Y_CHAR
%token Y_CHARACTER
%token Y_CHARF
%token Y_CHARZ
%token Y_CHECK
%token Y_CLEAR
%token Y_CLOSE
%token Y_CLUSTER
%token Y_COLUMN
%token Y_COMMENT
%token Y_COMMIT
%token Y_COMPUTE
%token Y_CONST
%token Y_CONSTRAINT
%token Y_CONNECT
%token Y_CONSTANT
%token Y_CONTEXT
%token Y_CONTROLFILE
%token Y_COPY
%token Y_COUNT
%token Y_CREATE
%token Y_CURRENT
%token Y_CURRVAL
%token Y_CURSOR
%token Y_DATABASE
%token Y_DATE
%token Y_DATETIME
%token Y_DAY
%token Y_DEC
%token Y_DBA
%token Y_DECIMAL
%token Y_DECLARE
%token Y_DEFAULT
%token Y_DEFINE
%token Y_DEGREE
%token Y_DELETE
%token Y_DELETING
%token Y_DESC
%token Y_DESCRIBE
%token Y_DESCRIPTOR
%token Y_DISABLE
%token Y_DISPLAY
%token Y_DISTINCT
%token Y_DOUBLE
%token Y_DROP
%token Y_ELSE
%token Y_ELSIF
%token Y_ENABLE
%token Y_END
%token Y_ENDIF
%token Y_END_EXEC
%token Y_ESCAPE
%token Y_EXCEPTION
%token Y_EXCEPTIONS
%token Y_EXCLUSIVE
%token Y_EXEC
%token Y_EXEC_IAF
%token Y_EXEC_ORACLE
%token Y_EXEC_SQL
%token Y_EXEC_TOOLS
%token Y_EXECUTE
%token Y_EXISTS
%token Y_EXIT
%token Y_EXPLAIN
%token Y_FALSE
%token Y_FETCH
%token Y_FIRST
%token Y_FLOAT
%token Y_FOR
%token Y_FORCE
%token Y_FOREIGN
%token Y_FRACTION
%token Y_FREE
%token Y_FREELIST
%token Y_FREELISTS
%token Y_FROM
%token Y_FUNCTION
%token Y_GET
%token Y_GOTO
%token Y_GRANT
%token Y_GROUP
%token Y_GROUPS
%token Y_HAVING
%token Y_HOST
%token Y_HOUR
%token Y_IDENTIFIED
%token Y_IF
%token Y_IFDEF
%token Y_IFNDEF
%token Y_IMMEDIATE
%token Y_IN
%token Y_INCLUDE
%token Y_INDEX
%token Y_INDICATOR
%token Y_INITIAL
%token Y_INITRANS
%token Y_INSERT
%token Y_INSERTING
%token Y_INSTANCES
%token Y_INT
%token Y_INTEGER
%token Y_INTERSECT
%token Y_INTERVAL
%token Y_INTO
%token Y_IS
%token Y_KEY
%token Y_LAST
%token Y_LIKE
%token Y_LIST
%token Y_LOCK
%token Y_LONG
%token Y_LOOP
%token Y_MATCHES
%token Y_MAX
%token Y_MAXEXTENTS
%token Y_MAXTRANS
%token Y_MESSAGE
%token Y_MIN
%token Y_MINEXTENTS
%token Y_MINUS
%token Y_MINUTE
%token Y_MLSLABEL
%token Y_MODE
%token Y_MONEY
%token Y_MONTH
%token Y_NATURAL
%token Y_NCHAR
%token Y_NEW
%token Y_NEXT
%token Y_NEXTVAL
%token Y_NOAUDIT
%token Y_NOCACHE
%token Y_NOPARALLEL
%token Y_NOT
%token Y_NOWAIT
%token Y_NULL
%token Y_NUMBER
%token Y_NUMERIC
%token Y_NVARCHAR
%token Y_OF
%token Y_OLD
%token Y_ON
%token Y_OPEN
%token Y_OPTION
%token Y_OR
%token Y_ORDER
%token Y_OPTIMAL
%token Y_OTHERS
%token Y_OUT
%token Y_PACKAGE
%token Y_PARALLEL
%token Y_PAUSE
%token Y_PCT_FOUND
%token Y_PCT_ISOPEN
%token Y_PCT_NOTFOUND
%token Y_PCT_ROWCOUNT
%token Y_PCT_ROWTYPE
%token Y_PCT_TYPE
%token Y_PCTFREE
%token Y_PCTINCREASE
%token Y_PCTUSED
%token Y_POSITIVE
%token Y_PRAGMA
%token Y_PRECISION
%token Y_PREPARE
%token Y_PRIMARY
%token Y_PRINT
%token Y_PRIOR
%token Y_PROCEDURE
%token Y_PROFILE
%token Y_PROMPT
%token Y_PUBLIC
%token Y_PUT
%token Y_QUIT
%token Y_RAISE
%token Y_RANGE
%token Y_RAW
%token Y_REAL
%token Y_RECORD
%token Y_REF
%token Y_REFERENCE
%token Y_REFERENCES
%token Y_REFERENCING
%token Y_RELEASE
%token Y_RENAME
%token Y_REPLACE
%token Y_RESTRICT_REFERENCES
%token Y_RETURN
%token Y_RETURNING
%token Y_REVERSE
%token Y_REVOKE
%token Y_ROLE
%token Y_ROLLBACK
%token Y_ROWID
%token Y_RNDS
%token Y_RNPS
%token Y_RUN
%token Y_RUNFORM
%token Y_SAVEPOINT
%token Y_SCHEMA
%token Y_SCROLL
%token Y_SECOND
%token Y_SECTION
%token Y_SELECT
%token Y_SEQUENCE
%token Y_SERIAL
%token Y_SET
%token Y_SHOW
%token Y_SMALLFLOAT
%token Y_SMALLINT
%token Y_SNAPSHOT
%token Y_SOME
%token Y_SPOOL
%token Y_SQL
%token Y_SQLCA
%token Y_SQLCODE
%token Y_SQL_CURSOR
%token Y_SQLERRM
%token Y_START
%token Y_STATEMENT
%token Y_STDDEV
%token Y_STORAGE
%token Y_STRING
%token Y_SUBTYPE
%token Y_SUM
%token Y_SYNONYM
%token Y_SYS_INCLUDE
%token Y_TABLE
%token Y_TABLESPACE
%token Y_TEXT
%token Y_THEN
%token Y_THREADS
%token Y_TIMING
%token Y_TO
%token Y_TRIGGER
%token Y_TRUE
%token Y_TRUNCATE
%token Y_TTITLE
%token Y_TYPE
%token Y_UNDEFINE
%token Y_UNION
%token Y_UNIQUE
%token Y_UNSIGNED
%token Y_UPDATE
%token Y_UPDATING
%token Y_USE
%token Y_USER
%token Y_USING
%token Y_VALIDATE
%token Y_VALUES
%token Y_VAR
%token Y_VARCHAR
%token Y_VARCHAR2
%token Y_VARIABLE
%token Y_VARIABLES
%token Y_VARIANCE
%token Y_VARNUM
%token Y_VARRAW
%token Y_VARYING
%token Y_VIEW
%token Y_VOLATILE
%token Y_WHEN
%token Y_WHENEVER
%token Y_WHERE
%token Y_WHILE
%token Y_WITH
%token Y_WNDS
%token Y_WNPS
%token Y_WORK
%token Y_YEAR

/* multicharacter operators and punctuation */
%token OP_ASSIGN		/* assignment ":=" */
%token OP_ASSOCIATE		/* associate argument "=>" */
%token OP_CONCAT		/* concatenate strings "||" */
%token OP_EXPONENT		/* power "**" */
%token OP_GE			/* greater than or equal to ">=" */
%token OP_LABEL_END		/* label right delimiter ">>" */
%token OP_LABEL_START		/* label left delimiter "<<" */
%token OP_LE			/* less than or equal to "<=" */
%token OP_NE			/* not equal to "!=", "<>", "~=", or "^=" */
%token OP_OUTERJOIN		/* TBD: outer join "(+)" */
%token OP_RANGE			/* number range ".." */
%token OP_START			/* start command "@@" */ 
%token OP_ARROW			/* pointer "->" */
%token OP_INCR			/* increment "++" */
%token OP_DECR			/* decrement "--" */
%token OP_QUAL			/* qualification "::" */

/* "Run last command" in standalone SQL is a slash by itself on a line */
%token OP_SLASH

/*
 * special token to resolve conflict with logical-AND
 * (see scanner)
 */
%token OP_BETWEEN_AND

/* literals */
%token STRING_LITERAL
%token NUMERIC_LITERAL

/* symbol names */
%token SYM_IDENTIFIER

/* colon prefixed host variable name */
%token HOST_PREFIX
%token SYM_HOST_VARIABLE

/* token for any keyword when ignoring non-data sql statements */
%token GENERIC_KEYWORD
%token GENERIC_TOKEN

/* token only returned by scanner when parser detects a statement
 * that is terminated by a newline
 */
%token END_OF_LINE

/* token returned when end of file reached */
%token END_OF_FILE

/* varchar bracketed array subscript "[...]" */
%token VARCHAR_ARRAY

/* special tokens for vendor specific grammar */
%token INFORMIX_PROCEDURE

/*
 * Precedence for operators:
 * low to high
 *
 * Note that unlike SQL, PL/SQL has a BOOLEAN type, which means
 * that operators that were once found only in WHERE clauses
 * can now be found in any expressions.
 *
 * TBD: is exponent left or right?
 *
 * TBD: not clear where BETWEEN, IN, and LIKE rank with respect to
 * each other (I believe it only matters for strange boolean
 * constructs).
 */
%left Y_UNION
%left Y_INTERSECT
%left Y_MINUS
%left OP_ASSOCIATE
%left OP_ASSIGN
%left Y_OR
%left Y_AND
%left Y_NOT
%left OP_BETWEEN_AND
%left Y_BETWEEN
%left Y_IN
%left Y_LIKE Y_MATCHES
%left Y_ESCAPE
%left '=' OP_NE OP_GE OP_LE '<' '>'
%left OP_CONCAT
%left '+' '-'
%left '*' '/'
%left OP_EXPONENT
%left ']'
%left VARCHAR_ARRAY
%nonassoc UMINUS

/*********************** Start Of Grammar ***********************/
%%

/*
 * First rule: sequence of one or more program elements
 *
 * Chain each subtree to the root.  Each grammar rule
 * forms a new node to the tree with its child nodes attached
 * below it (ast_child_tree ()).  In the case of simple rules
 * which contain only one non-terminal and no "useful" terminals,
 * e.g. '(' expression ')', the value of the single child rule is
 * just passed up, in this case $$.ast = $2.ast.  Note that the
 * default yacc rule is $$ = $1 (see sql_program_object).
 * 
 */
program_file:
	program
		{
		   /* create the top level tree node (root) */
		   if (esql_mode) {
		      /* for esql, each top level child tree (nodes directly
		       * under ast_replace) is the replacement for an
		       * ast_block in the host ast tree.  This block is
		       * output by this parser to replace any sql stmt
		       * (see esql_prefix_check and postfix_reference).
		       * The replacement is done by the ifext program
		       * in a special ast merge pass.
		       */
		      ast_root = new_tree (ank_ast_replace, 0, $1, $1);
		   } else {
		      ast_root = new_tree (ank_ast_root, 0, $1, $1);
		   }
		   ast_child_tree (ast_root, $1.ast);
		}
	| program_file program
		{
		   ast_child_tree (ast_root, $2.ast);
		}
	;

/*
 * Basic SQL program statements
 *
 */
program:
	program_element
	| esql_element
	| OP_SLASH
		{
		   /*
		    * A single slash is SQL*PLUS "Execute" command
		    * (i.e. "run what I just typed in).
		    * Ignore
		    */
		   set_empty (&$$);
		}
	| END_OF_FILE
		{
		   /* An empty program.
		    * This includes an esql program with only host code.
		    */
		}
	;

/*
 * A procedure/function/package/trigger may start with
 * CREATE OR REPLACE if meant to be run within SQL*PLUS
 * (Oracle's interactive SQL command interpreter).
 */
program_element:
	plsql_program_object
	| create_replace_object
        | ddl_stmt
        | dml_stmt
	| sql_directive
	| plsql_mainblock
	| error sql_eos
		{
		   /* Catch parse errors not caught at the statement level
		    * by skiping to the next semi-colon.
		    *
		    * Note: "error" is a yacc built-in element. 
		    */
		   if (sql_allow_error) {
		      sql_allow_error = 0;
		      set_empty (&$$);
		   } else {
		      error_result (&$$, $2, $2);
		   }
		   sql_error_clear ();
		}
	| error END_OF_LINE
		{
		   /* eol is returned only for sql commands that are
		    * single line commands (those that do not require
		    * a terminating semi-colon).
		    */
		   if (sql_allow_error) {
		      sql_allow_error = 0;
		      set_empty (&$$);
		   } else {
		      error_result (&$$, $2, $2);
		   }
		   sql_error_clear ();
		}
	;

/* end-of-statement */
sql_eos:
	';'
	| OP_SLASH
	;

/* end-of-line -
 * END_OF_LINE is only returned if flagged as needed.
 * END_OF_FILE is returned once at the end of the final file
 * (NOT after each included file).  EOF is needed for files
 * where the last line does not have a newline.
 */
sql_eol:
	END_OF_LINE
	| END_OF_FILE
	;

/*
 * These are the basic PL/SQL stand-alone objects.
 *
 * List of single non-terminals (uses yacc default rule of $$ = $1).
 */
plsql_program_object:
	procedure_body
	| function_body
	| procedure_specification
	| function_specification
	| package_body
	| package_specification
	| trigger_body
	;

create_replace_object:
	Y_CREATE or_replace_opt plsql_program_object
		{
		   $$.ast = $3.ast;
		}
	| Y_CREATE or_replace_opt force_opt Y_VIEW
		{
		   sql_keywords_only ();
		   sql_push_scope (0, 0);
		}
	view_definition
		{
		   $$.ast = $6.ast;
		}
	;

or_replace_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_OR Y_REPLACE
	;

/* Y_FORCE or Y_NOFORCE */
force_opt:
	/* empty */	{ set_empty (&$$); }
	| keyword
	;


/* top level rule for esql stmts
 *
 * Starts with command prefix (e.g. EXEC SQL)
 * some commands allow an AT clause for remote database
 * and FOR clause for a repeat factor/limit of the command.
 * Commands that are executable statements are replaced with
 * braces which serve as placeholders in the host program.
 * Non-executable sql stmts (e.g. declarations) are not replaced by braces
 * because that would cause an error in C if it was in the middle of
 * a C declaration section.
 *
 * Example:
 *	void foo (void) {
 *	   int i;
 *	   EXEC SQL BEGIN DECLARE SECTION;
 *	   int x;
 *	   EXEC SQL END DECLARE SECTION;
 *	   int a;
 *	   if (b)
 *		EXEC SQL SELECT id INTO :x FROM tab;
 * Becomes:
 *	void foo (void) {
 *	   int i;
 *	   
 *	   int x;
 *	   
 *	   int a;
 *	   if (b)
 *		{                        x;        }
 *
 */
esql_element:
	esql_command_prefix
	at_clause_opt for_clause_opt esql_command
		{
		   $$.ast = $4.ast;
		   ast_extend_start ($$.ast, token_loc_start ($1));
		   ast_replacement ($$.ast);

		   if (need_postfix) {
		      postfix_reference($4);
		      need_postfix = 0;
		   }
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	| esql_command_prefix 
	at_clause_opt for_clause_opt error ';'
		{
		   /* Catch parse errors not caught at the statement level
		    * by skiping to the next semi-colon.
		    *
		    * Note: "error" is a yacc built-in element. 
		    */
		   if (sql_allow_error) {
		      sql_allow_error = 0;
		      set_empty (&$$);
		   } else {
		      error_result (&$$, $1, $5);
		   }
		   sql_error_clear ();

		   if (need_postfix) {
		      postfix_reference($5);
		      need_postfix = 0;
		   }
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	| Y_EXEC_ORACLE
		{
		   sql_allow_error = 1;
		}
	oracle_command
		{
		   set_result (&$$, $1, $3, 0, $3.ast);
		}
	| Y_VARCHAR
		{
		   sql_keyword_command (SQLKEYC_VARCHAR);
#ifndef DO_VARCHAR_DECL
		   iff_map_token_start_fixed ($1);
#endif /* ! DO_VARCHAR_DECL */
		   /* save type token for symbol reference */
		   saved_token = $1;
		}
	varchar_directive
		{
		   set_result (&$$, $1, $3, 0, $3.ast);
		}
	| Y_SQL_CURSOR
		{
		   expand_sqlcursor ($1);
		}
	| esql_command_prefix SYM_IDENTIFIER
		{
		   /* check for Informix host variable declaration */
		   /* if so -- don't process, just output */
		   if (esql_mode &&
		       (dbvendor == DBVENDOR_INFORMIX) &&
		       (strcmp (token_text ($1), "$") == 0)) {

		      /* output host code */
		      copy_out ($2, 0);

		      /* reset scanner to esql mode */
		      sql_scanner_scan_mode ();
		   } else {
		      iff_error_token (get_token ($2.start_token),
			       "unexpected syntax");
		   }
		   set_empty (&$$);
		}
	| Y_EXEC_TOOLS
		{
		   sql_keyword_command (SQLKEYC_IAF);
		   prefix_token = $1;
		}
	esql_tools_command
		{
		   $$.ast = $3.ast;
		   ast_extend_start ($$.ast, token_loc_start ($1));
		   ast_replacement ($$.ast);

		   if (need_postfix) {
		      postfix_reference($3);
		      need_postfix = 0;
		   }
		   set_result (&$$, $1, $3, 0, $3.ast);
		}
	;

esql_command_prefix:
	Y_EXEC_SQL
		{
		   /* store starting position of command
		    * so that a brace can be output at that place
		    * in the host output file in case this is an
		    * executable statement.
		    */
		   prefix_token = $1;
		}
	| Y_EXEC_IAF
		{
		   sql_keyword_command (SQLKEYC_IAF);
		   prefix_token = $1;
		}
	;

/* list of all esql commands */
esql_command:
	esql_allocate
	| esql_alter
	| esql_arraylen
	| esql_begin_work
	| esql_close
	| esql_commit
	| esql_connect
	| esql_context
	| esql_create
	| esql_database
	| esql_declare
	| esql_delete
	| esql_describe
	| esql_drop
	| esql_enable
	| esql_execute
	| esql_fetch
	| esql_get
	| esql_include
	| esql_insert
	| esql_lock
	| esql_open
	| esql_prepare
	| esql_put
	| esql_rollback
	| esql_savepoint
	| esql_section_begin
	| esql_section_end
	| esql_select
	| esql_set_transaction
	| esql_truncate
	| esql_type
	| esql_update
	| esql_var
	| esql_whenever
	;

at_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| at_clause
	;

at_clause:
	Y_AT
		{
		}
	host_identifier_name
		{
		   $$.ast = $3.ast;
		}
	;

for_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| for_clause
	;

for_clause:
	Y_FOR
		{
		   esql_prefix_check ();
		}
	host_identifier
	;

host_identifier:
	host_variable_symbol
	| SYM_IDENTIFIER
	;

host_identifier_reference:
	host_variable_symbol
	| host_reference_list
	;

/* C variable including structure references */
host_reference_list:
	host_reference
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| host_reference_list '.' host_reference
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	| host_reference_list '-' '>' host_reference
		{
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $4, 0, $1.ast);
		}
	;

host_reference:
	SYM_IDENTIFIER
	;

host_identifier_name:
	host_variable_symbol
	| SYM_IDENTIFIER
	| STRING_LITERAL
	;

/* handle Oracle preprocessor conditionals (happens before C/C++
 * preprocessor).
 */
oracle_command:
	Y_OPTION '(' oracle_option_spec ')' ';'
	| Y_DEFINE '=' SYM_IDENTIFIER ';'
		{
		   define_id ($3);
		}
	| Y_IFDEF SYM_IDENTIFIER ';'
		{
		   do_ifdef($2);
		}
	| Y_IFNDEF SYM_IDENTIFIER ';'
		{
		   do_ifndef($2);
		}
	| Y_ELSE ';'
		{
		   do_else($1);
		}
	| Y_ENDIF ';'
		{
		   do_endif($1);
		}
	;

oracle_option_spec:
	Y_INCLUDE '='
		{
		   sql_no_keywords ();
		}
	pathname
		{
		   add_include_path ($4);
		   sql_clear_no_keywords ();
		}
	| Y_SYS_INCLUDE '='
		{
		   sql_no_keywords ();
		}
	 pathname
		{
		   add_sysinclude_path ($4);
		   sql_clear_no_keywords ();
		}
	| Y_DEFINE '=' SYM_IDENTIFIER
		{
		   define_id ($3);
		}
	;

pathname:
	pathname_element
	| pathname pathname_element
		{
		   set_result (&$$, $1, $2, 0, 0);
		}
	;

/* sql_no_keywords() is turned on here */
/* TBD: need to get single token to insure no spaces */
pathname_element:
	SYM_IDENTIFIER
	| NUMERIC_LITERAL
	| '.'
	| '/'
	;

esql_allocate:
	Y_ALLOCATE
		{
		   esql_prefix_check ();
		   sql_identifier_next = 1;
		}
	descriptor with_max_clause_opt ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $3, $5);
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

descriptor:
	cursor_handle
	| STRING_LITERAL
		{
		   $$.ast = string_literal_tree ($1);
		}
	;

/* INFORMIX option */
with_max_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_WITH Y_MAX NUMERIC_LITERAL
	| Y_WITH Y_MAX host_identifier
	;

esql_alter:
	Y_ALTER
		{
		   scanner_generic_mode = 1;
		}
	generic_token_list_opt ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;


esql_arraylen:
	Y_ARRAYLEN
		{
		   esql_prefix_check ();
		}
	host_identifier '(' NUMERIC_LITERAL ')' ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $6, $7);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, literal_tree ($5));
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

esql_begin_work:
	begin_work_statement
	;

esql_close:
	close_statement
	;

esql_commit:
	commit_statement
	;

esql_connect:
	connect_keyword	user_and_password using_opt ';'
		{
		   sqltype tlast;

		   if ( ! is_empty ($3) ) {
		     tlast = $3;
		   } else {
		     tlast = $2;
		   }

		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $4);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	| connect_keyword Y_TO connect_database
	as_connect_opt user_clause_opt with_concurrent_opt ';'
		{
		   /* INFORMIX */
		   sqltype tlast;

		   if ( ! is_empty ($6) ) {
		     tlast = $6;
		   } else if ( ! is_empty ($5) ){
		     tlast = $5;
		   } else if ( ! is_empty ($4) ){
		     tlast = $4;
		   } else {
		     tlast = $3;
		   }

		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $7);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $6.ast);
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

connect_keyword:
	Y_CONNECT
		{
		   sql_keyword_command (SQLKEYC_CONNECT);
		   esql_prefix_check ();
		}
	;

user_and_password:
	host_variable Y_IDENTIFIED Y_BY host_variable
		{
		   $$.ast = new_tree (ank_list, 0, $1, $4);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| host_variable
	;

/* USING db_string */
using_opt:
	/* empty */	{ set_empty (&$$); }
	| at_clause
	| at_clause Y_USING host_identifier_name
	| Y_USING host_identifier_name
	;

connect_database:
	Y_DEFAULT
	| host_identifier_name
	;

as_connect_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_AS host_identifier_name
		{
		   set_result (&$$, $1, $2, $2.sym, $2.ast);
		}
	;

user_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_USER host_identifier_name Y_USING host_identifier_name
	;

with_concurrent_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_WITH SYM_IDENTIFIER SYM_IDENTIFIER
		{
		   symbol_keyword_check ($1, "CONCURRENT");
		   symbol_keyword_check ($1, "TRANSACTION");
		}
	;

esql_context:
	context_keyword Y_ALLOCATE host_identifier ';'
		{
		   set_result (&$$, $1, $4, 0, $3.ast);
		}
	| context_keyword Y_FREE host_identifier ';'
		{
		   set_result (&$$, $1, $4, 0, $3.ast);
		}
	| context_keyword Y_USE host_identifier ';'
		{
		   set_result (&$$, $1, $4, 0, $3.ast);
		}
	;

context_keyword:
	Y_CONTEXT
		{
		   sql_keyword_command (SQLKEYC_CONTEXT);
		   esql_prefix_check ();
		}
	;

esql_create:
	Y_CREATE
		{
		   /* set scanner to sql mode */
		   sql_scanner_sql_mode ();
		   esql_prefix_check ();
		}
	or_replace_opt plsql_program_object
	Y_END_EXEC ';'
		{
		   /* reset scanner to esql mode */
		   sql_scanner_scan_mode ();
		   set_result (&$$, $1, $6, 0, $4.ast);
		}
	| table_definition
	| misc_create
	;

esql_database:
	database_statement
	;

esql_declare:
	esql_declare_cursor
	| declare_keyword host_identifier Y_DATABASE ';'
		{
		   set_result (&$$, $1, $2, 0, $3.ast);
		}
	| declare_keyword host_identifier Y_STATEMENT ';'
		{
		   set_result (&$$, $1, $2, 0, $3.ast);
		}
	| declare_keyword host_identifier Y_TABLE
		{
		   sql_push_scope (0, 0);
		   /* set table_name for current scope */
		   sql_scope_set_table_name (token_text ($2));
		   $2.sym = symbol_extern ($1, sk_table);
		   $2.ast = symbol_token ($2);
		}
	 '(' esql_column_list ')' ';'
		{
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $8);
		   ast_child_tree ($$.ast, $2.ast);

		   at_block = new_tree (ank_ast_block, 0, $5, $7);
		   ast_child_tree (at_block, $6.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_relation ($6.ast, rk_context);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);

		   sql_pop_scope ();
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

esql_declare_cursor:
	declare_keyword host_identifier cursor_keywords Y_FOR 
		{
		   char * text = 0;
		   $2.sym = symbol_set ($2, sk_cursor);
		   if ($2.sym) {
		      text = $2.sym->sym_text;
		   }
		   sql_push_scope (text, 0);
		}
	 esql_declare_cursor_object ';'
		{
		   ast_tree * at_decl;
		   at_decl = new_tree (ank_cursor_decl, $2.sym, $1, $3);
		   ast_child_tree (at_decl, symbol_token ($2));

		   $$.ast = new_tree (ank_cursor_def, $2.sym, $1, $7);

		   /* TBD: need to create virtual record for cursor */
		   /* empty (implies unknown) return type */
		   ast_child_tree ($$.ast, empty_tree (ank_ast_declspec));

		   ast_child_tree ($$.ast, at_decl);
		   ast_child_tree ($$.ast, $6.ast);

		   ast_defined ($$.ast);
		   ast_decldefined (at_decl);
		   sql_pop_scope ();
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

cursor_keywords:
	Y_CURSOR
	| Y_SCROLL Y_CURSOR
		{
		   set_result (&$$, $1, $2, 0, 0);
		}
	;

esql_declare_cursor_object:
	select_compound
	| host_identifier
	;

declare_keyword:
	Y_DECLARE
		{
		   sql_identifier_next = 1;
		   esql_prefix_check ();
		}
	;

esql_column_list:
	esql_column
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| esql_column_list ',' esql_column
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

esql_column:
	column_name column_type esql_column_constraint_opt
		{
		   ast_tree * at_field;
		   sqltype tlast;

		   if ( ! is_empty ($3) ) {
		     tlast = $3;
		   } else {
		     tlast = $2;
		   }

		   $$.ast = new_tree (ank_list_decl, $1.sym, $1, tlast);
		   ast_child_tree ($$.ast, $2.ast);

		   at_field = reference_type_tree (ank_field_decl, $1.sym,
						  $1, $1, REFMASK_AST);
		   ast_child_tree (at_field, $1.ast);
		   ast_decldefined (at_field);
		   ast_child_tree ($$.ast, at_field);

		   ast_defined ($$.ast);
		}
	;

esql_column_constraint_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_DEFAULT expression
		{
		   $$.ast = $2.ast;
		}
	| Y_DEFAULT expression Y_NULL
		{
		   $$.ast = $2.ast;
		}
	| Y_DEFAULT expression Y_NOT Y_NULL
		{
		   $$.ast = $2.ast;
		}
	| Y_NOT Y_NULL
	| Y_NOT Y_NULL Y_WITH Y_DEFAULT
	;

esql_delete:
	delete_statement
	;

esql_describe:
	describe_keyword Y_SELECT Y_LIST Y_FOR host_identifier 
	Y_INTO host_identifier ';'
		{
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	| describe_keyword Y_BIND Y_VARIABLES Y_FOR host_identifier
	Y_INTO host_identifier ';'
		{
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

describe_keyword:
	Y_DESCRIBE
		{
		   sql_keyword_command (SQLKEYC_DESCRIBE);
		}
	;

esql_drop:
	Y_DROP
		{
		   sql_allow_error = 1;
		   esql_prefix_check ();
		}
	Y_TABLE other_table_reference ';'
		{
		   sql_allow_error = 0;
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

esql_enable:
	Y_ENABLE
		{
		   sql_keyword_command (SQLKEYC_ENABLE);
		   esql_prefix_check ();
		}
	Y_THREADS ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $3, $4);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

esql_execute:
	execute_keyword plsql_mainblock Y_END_EXEC ';'
		{
		   sql_scanner_scan_mode ();
		   set_result (&$$, $1, $4, 0, $2.ast);
		}
	| execute_keyword host_identifier using_items_opt ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	| execute_keyword Y_IMMEDIATE host_identifier_name ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

execute_keyword:
	Y_EXECUTE
		{
		   esql_prefix_check ();
		}
	;
   
using_items_opt:
	/* empty */	{ set_empty (&$$); }
	| using_variable_list
	| using_descriptor
	;

using_variable_list:
	Y_USING host_variable_symbol_list
		{
		   $$.ast = $2.ast;
		}
	;

using_descriptor_opt:
	/* empty */	{ set_empty (&$$); }
	| using_descriptor
	;
using_descriptor:
	Y_USING Y_DESCRIPTOR host_identifier
		{
		   $$.ast = $3.ast;
		}
	;

esql_fetch:
	fetch_keyword cursor_handle Y_INTO esql_into_target_list ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	| fetch_keyword cursor_handle using_descriptor_opt ';'
		{
		   sqltype tlast;
		   if (!is_empty($3)) {
		      tlast = $3;
		   } else {
		      tlast = $2;
		   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $4);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

fetch_keyword:
	Y_FETCH
		{
		   esql_prefix_check ();
		   sql_identifier_next = 1;
		}
	;

esql_into_target_list:
	esql_into_target
		{ $$.ast = new_tree(ank_list, 0, $1, $1);
		  ast_child_tree($$.ast, $1.ast);
	        }
	| esql_into_target_list ',' esql_into_target
		{ ast_child_tree($1.ast, $3.ast);
		  set_result(&$$, $1, $3, NULL, $1.ast);
	        }
	;

esql_into_target:
	esql_into_var
	| esql_into_var host_variable
		{ $$.ast = sql_call_expr("INDICATOR", $2, $2);
		  ast_child_tree($$.ast, $1.ast);
		  ast_child_tree($$.ast, $2.ast);
	        }
	| esql_into_var Y_INDICATOR host_variable
		{ $$.ast = symbol_call_expr(&$2, $1, $3);
		  ast_child_tree($$.ast, $1.ast);
		  ast_child_tree($$.ast, $3.ast);
	        }
	;

esql_into_var:
	single_identifier
	| host_variable
	;
   
/* TBD: could not find any Oracle doc on this command (or PUT)
 * so this is just from a source example.
 */
esql_get:
	Y_GET
		{
		   esql_prefix_check ();
		}
	host_variable_symbol Y_INTO host_variable_symbol ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $5, $6);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

/* esql include statement 
 *
 * Opens file and switches scanner input to that file.  Includes
 * can be nested.  When eof is reached on the included file input
 * is switched back to this file.
 *
 * Note that the newline at the end of the include statement line
 * is processed in the scanner before the file is included.  This
 * is important for the correct iff mapping lines entries.
 */
esql_include:
	Y_INCLUDE
		{
		   sql_allow_error = 1;
		   sql_no_keywords ();
		   need_end_of_line_token = 1;
		}
	pathname ';' sql_eol
		{
		   sql_clear_no_keywords ();
		   sql_allow_error = 0;
		   include_file($3);
		   set_result (&$$, $1, $4, 0, 0);
		}
	;

esql_insert:
	insert_statement
	;

esql_lock:
	lock_table_statement
	;

esql_open:
	open_keyword cursor_handle using_items_opt ';'
		{
		   sqltype tlast;
		   if (!is_empty($3)) {
		      tlast = $3;
		   } else {
		      tlast = $2;
		   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $4);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

open_keyword:
	Y_OPEN
		{
		   esql_prefix_check ();
		   sql_identifier_next = 1;
		}
	;

esql_prepare:
	Y_PREPARE
		{
		   esql_prefix_check ();
		}
	host_identifier Y_FROM host_identifier_name ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $5, $6);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

esql_put:
	Y_PUT
		{
		   esql_prefix_check ();
		}
	host_variable_symbol values_clause ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;


esql_rollback:
	rollback_statement
	;

esql_savepoint:
	savepoint_statement
	;

esql_section_begin:
	Y_BEGIN Y_DECLARE Y_SECTION ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

esql_section_end:
	Y_END Y_DECLARE Y_SECTION ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

esql_select:
	select_statement
	;

esql_set_transaction:
	set_transaction_statement
	;

/* Handle Oracle EXEC TOOLS statements.
 * These are separated from other ESQL statements to handle SET accurately.
 */
esql_tools_command:
	esql_tools_get
	| esql_tools_get_context
	| esql_tools_message
	| esql_tools_set
	| esql_tools_set_context
	;

esql_tools_get:
	Y_GET
		{
		   esql_prefix_check ();
		}
	identifier_list Y_INTO host_variable_symbol_list ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $5, $6);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

esql_tools_get_context:
	Y_GET
		{
		   esql_prefix_check ();
		}
	Y_CONTEXT context_name Y_INTO host_variable ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $6, $7);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $6.ast);
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

esql_tools_message:
    	Y_MESSAGE
		{
		   esql_prefix_check ();
		}
	literal_or_host_item literal_or_host_item_opt ';'
		{
    	    	   sqltype tlast;
    	    	   if ( ! is_empty ($4) ) {
    	    	     tlast = $4;
    	    	   }
    	    	   else {
    	    	     tlast = $3;
    	    	   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $5);
    	    	   ast_child_tree ($1.ast, $3.ast);
    	    	   ast_child_tree ($1.ast, $4.ast);
    	    	   set_result (&$$, $1,	$5, 0, $$.ast);
		}

esql_tools_set:
	Y_SET
		{
		   esql_prefix_check ();
		}
	identifier_list values_clause ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

esql_tools_set_context:
	Y_SET
		{
		   esql_prefix_check ();
		}
	Y_CONTEXT host_variable Y_IDENTIFIED Y_BY context_name ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $7, $8);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $7.ast);
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

context_name:
	SYM_IDENTIFIER
		{
		   /* sk_context */
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_set ($1, sk_context);
		   }
		   $$.ast = symbol_token ($1);
		}
	| host_variable
	;

esql_type:
	Y_TYPE
		{
		   sql_keyword_command (SQLKEYC_TYPE);
		   sql_identifier_next = 1;
		}
	identifier Y_IS data_type reference_opt ';'
		{
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

reference_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_REFERENCE
	;

esql_truncate:
	Y_TRUNCATE
		{
		   scanner_generic_mode = 1;
		}
	generic_token_list_opt ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

esql_update:
	update_statement
	;

esql_var:
	Y_VAR host_identifier_reference Y_IS data_type ';'
		{
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

esql_whenever:
	Y_WHENEVER
		{
		   scanner_generic_mode = 1;
		}
	generic_token_list_opt ';'
		{
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

/* VARCHAR directive
 * defines host string variables.
 * Changed to a C structure definition.
 */
varchar_directive:
	varchar_declarator_list ';'
		{
		   token *tok = get_token ($2.start_token);

#ifdef DO_VARCHAR_DECL
		   /* add semicolon position to last VARCHAR */
		   varchar_set_end (saved_ast_tree, $2);
#endif /* DO_VARCHAR_DECL */

		   /* map semicolon */
		   if (tok) {
		      iff_map_copy_start_loc (tok->lineno, tok->column,
					      olineno, ocolumn);
		      iff_map_copy_end_loc (tok->lineno, tok->column,
					    olineno, ocolumn);
		   }

#ifndef DO_VARCHAR_DECL
		   iff_map_token_end_fixed ($2);
#endif /* ! DO_VARCHAR_DECL */
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	| varchar_ptr_operator
		{
		   /* output structure to host file */
		   expand_varchar_ptr ($1);

		   /* reset scanner to esql mode */
		   sql_scanner_scan_mode ();

#ifndef DO_VARCHAR_DECL
		   iff_map_token_end_fixed ($1);
#endif /* ! DO_VARCHAR_DECL */
		}
	;

varchar_declarator_list:
	varchar_declarator
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
#ifdef DO_VARCHAR_DECL
		   /* save tree to add comma or semicolon later */
		   saved_ast_tree = varchar_list_decl ($1, saved_token);
		   ast_child_tree ($$.ast, saved_ast_tree);
#endif /* DO_VARCHAR_DECL */
		}
	| varchar_declarator_list ',' varchar_declarator
		{
#ifdef DO_VARCHAR_DECL
		   /* add comma position to last VARCHAR */
		   varchar_set_end (saved_ast_tree, $2);
		   saved_ast_tree = varchar_list_decl ($3, saved_token);
		   ast_child_tree ($1.ast, saved_ast_tree);
#else /* DO_VARCHAR_DECL */
		   ast_child_tree ($1.ast, $3.ast);
#endif /* DO_VARCHAR_DECL */
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

/* expand VARCHAR definition into C structure.
 * Cannot be a typedef because the string size must be part
 * of that structure member.
 */
varchar_declarator:
	varchar_identifier
		{
		   /* output structure to host file */
		   expand_varchar_default ($1);

		   set_result (&$$, $1, $1, $1.sym, $1.ast);
		}
	| varchar_identifier array_bound
		{
		   /* output structure to host file */
		   expand_varchar ($1, $2);

		   set_result (&$$, $1, $2, $1.sym, $1.ast);
		}
	| varchar_identifier array_bound '=' expression
		{
		   /* output structure to host file */
		   expand_varchar_init ($1, $2, $4);

#ifdef DO_VARCHAR_DECL
		   $$.ast = new_tree (ank_list, 0, $1, $4);
		   ast_child_tree ($$.ast, $1.ast);
		   /* initializer */
		   ast_child_tree ($$.ast, $4.ast);
#endif /* DO_VARCHAR_DECL */

		   set_result (&$$, $1, $4, $1.sym, $$.ast);
		}
	| varchar_identifier array_bound '=' '{' expression ',' expression '}'
		{
		   ast_tree * at;
		   /* set initializer range - to be copied out as is */
		   sqltype t;
		   set_result (&t, $3, $8, 0, 0);

		   /* output structure to host file */
		   expand_varchar_init2 ($1, $2, t);

#ifdef DO_VARCHAR_DECL
		   $$.ast = new_tree (ank_list, 0, $1, $8);
		   ast_child_tree ($$.ast, $1.ast);

		   /* initializer */
		   at = new_tree (ank_ast_block, 0, $4, $8);
		   ast_child_tree ($$.ast, at);
		   ast_child_tree (at, $5.ast);
		   ast_child_tree (at, $7.ast);
#endif /* DO_VARCHAR_DECL */

		   set_result (&$$, $1, $8, $1.sym, $$.ast);
		}
	| varchar_identifier array_bound '=' '{' error '}'
		{
		   /* set initializer range - to be copied out as is */
		   sqltype t;
		   set_result (&t, $3, $6, 0, 0);

		   /* output structure to host file */
		   expand_varchar_init2 ($1, $2, t);

		   set_result (&$$, $1, $6, $1.sym, $1.ast);
		}
	| varchar_identifier array_bound array_bound
		{
		   /* output structure to host file */
		   expand_varchar_array ($1, $2, $3);

		   set_result (&$$, $1, $3, $1.sym, $1.ast);
		}
	;

varchar_identifier:
	SYM_IDENTIFIER
		{
#ifdef DO_VARCHAR_DECL
		   $$.sym = symbol_extern ($1, sk_variable);
		   if ($$.sym) {
		      $$.sym->sym_replace = 1;
		      $$.ast = symbol_tree ($$.sym, $1);
		   }
#endif /* DO_VARCHAR_DECL */
		}
	;

array_bound:
	VARCHAR_ARRAY
	| '[' expression ']'
		{
		   set_result (&$$, $1, $3, 0, 0);
		}
	| '[' expression VARCHAR_ARRAY ']'
		{
		   set_result (&$$, $1, $4, 0, 0);
		}
	;

varchar_ptr_operator:
	'*'
	| '&'
	| Y_CONST
	| Y_VOLATILE
	;

/*
 * These are the basic SQL DDL (Data Definition Language) Statements.
 *
 * SCHEMA can prefix the others.
 */
ddl_stmt:
        schema_prefix
        | table_definition
        | connect_command
	| misc_definition
        ;

/*
 * Basic SQL DML (Data Manipulation Language) Statements.
 */
dml_stmt:
	close_statement
	| commit_statement
	| database_statement
	| delete_statement
	| fetch_statement
	| insert_statement
	| open_statement
	| rollback_statement
	| select_statement
	| update_statement
	| explain_prefix delete_statement
		{
		   $$.ast = $2.ast;
		}
	| explain_prefix insert_statement
		{
		   $$.ast = $2.ast;
		}
	| explain_prefix select_statement
		{
		   $$.ast = $2.ast;
		}
	| explain_prefix update_statement
		{
		   $$.ast = $2.ast;
		}
	;

/*
 * SQL standalone script directives
 */
sql_directive:
	sql_directive_start
		{
		   need_end_of_line_token = 1;
		   scanner_generic_mode = 1;
		}
	generic_token_list_opt semicolon_opt sql_eol
		{
		   set_empty (&$$);
		}
	| execute_clause
		{
		   need_end_of_line_token = 1;
		}
	identifier_clause semicolon_opt sql_eol
		{
		   sqltype tlast;

		   if ( ! is_empty ($4) ) {
		     tlast = $4;
		   } else {
		     tlast = $3;
		   }

		   /* function/procedure call */
		   $$.ast = new_tree (ank_expr_stmt, 0, $1, tlast);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_relation ($$.ast, rk_call);
		}
	;

execute_clause:
	Y_EXECUTE
	| Y_EXEC
	;

semicolon_opt:
	/* empty */	{ set_empty (&$$); }
	| ';'
	;

/*
 * Ignored commands
 * These have to be single tokens so that scanner lookahead
 * does not happen before "scanner_generic_mode" is set.
 *
 * Note: The ORACLE SQL DEFINE here is a not the same as the Oracle
 * ESQL preprocessor DEFINE.  This DEFINE is a simple macro-like
 * variable value (DEFINE variable_name = value) which should be substituted
 * when a &variable_name is encountered.  The preprocessor DEFINE
 * is a simple token which is used as an argument to an ORACLE IFDEF
 * line.
 *
 * TBD: This DEFINE is sometimes used for more that a simple value,
 * but for a whole command string or fragment.
 *
 * For Example:
 *
 * DEFINE exception_insert =  "INSERT INTO mis_samples -
 * (sample_id,sample_date_time,char1,char2,number1,number2,date1)VALUES -
 * ('DIARY_ERR',this.time,this.pin,err_code,err_number,sql_code_num,SYSDATE); -
 * err_number := err_number + 1"
 *
 * Whenever an &exception_insert occurs in the code, this string should be
 * substituted.  The DEFINE value can also be a constant, not just a string.
 *
 * One way to implement this is to store these strings/values in a table and
 * when a &variable_name (or &&variable_name) is encounted in the
 * scanner, to switch the scanner input buffer to the string (just like
 * pushing an include file (but without the file)).  The appropriate
 * IFF mapping entries will also have to be generated.
 *
 * I don't know if a &variable can be nested inside another DEFINE, and
 * if so, if it substituted at DEFINE time or "runtime".  Also, in Oracle
 * SQL scripts, the DEFINE may be in another file which is unavailable at
 * parse time, so the &variable could be treated as an externref or an
 * error in these cases.
 */
sql_directive_start:
	'@'
	| OP_START
	| Y_ACCEPT
	| Y_ANALYSE
	| Y_ANALYZE
	| Y_AUDIT
	| Y_BREAK
	| Y_BTITLE
	| Y_CLEAR
	| Y_COLUMN
	| Y_COMMENT
	| Y_COMPUTE
	| Y_COPY
	| Y_DEFINE	/* see note above */
	| Y_EXIT
	| Y_HOST
	| Y_NOAUDIT
	| Y_PAUSE
	| Y_PRINT
	| Y_PROMPT
	| Y_QUIT
	| Y_RENAME
	| Y_RUN
	| Y_RUNFORM
	| Y_SET
	| Y_SHOW
	| Y_SPOOL
	| Y_START
	| Y_TIMING
	| Y_TRUNCATE
	| Y_TTITLE
	| Y_UNDEFINE
	| Y_VALIDATE
	| Y_VARIABLE
	| Y_WHENEVER
	;

/**************** Schema Prefix ******************/

schema_prefix:
        Y_CREATE Y_SCHEMA Y_AUTHORIZATION user_name
                {
		   set_result (&$$, $1, $4, 0, $4.ast);
	        }
        ;

user_name:
	SYM_IDENTIFIER
                {
		   /* user name is schema name */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }
		   $$.ast = symbol_token ($1);
	        }
        ;
	
/**************** Table Definition ******************/

create_table_clause:
	Y_CREATE Y_TABLE
		{
		   sql_keywords_only ();
		   sql_keyword_command (SQLKEYC_CT);
		   esql_prefix_check ();
		   sql_push_scope (0, 0);
		}
	;

table_definition:
	create_table_clause
	table_reference
	'(' table_element_list ')' constraint_clause_opt as_query_opt sql_eos
                {
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $8);
		   ast_child_tree ($$.ast, $2.ast);

		   at_block = new_tree (ank_ast_block, 0, $3, $5);
		   ast_child_tree (at_block, $4.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_relation ($4.ast, rk_context);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   sql_specfile_output ($2.sym, $1, $8);

		   sql_pop_scope ();
		   set_result (&$$, $1, $8, 0, $$.ast);
	        }
	| create_table_clause
	table_reference
	'(' table_element_list ')' error sql_eos
                {
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $7);
		   ast_child_tree ($$.ast, $2.ast);

		   at_block = new_tree (ank_ast_block, 0, $3, $5);
		   ast_child_tree (at_block, $4.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_relation ($4.ast, rk_context);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);

		   sql_pop_scope ();
		   set_result (&$$, $1, $7, 0, $$.ast);
	        }
	| create_table_clause
	table_reference
        as_query sql_eos
                {
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $4);
		   ast_child_tree ($$.ast, $2.ast);

		   at_block = new_tree (ank_ast_block, 0, $3, $3);
		   ast_child_tree (at_block, $3.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);

		   sql_pop_scope ();
		   set_result (&$$, $1, $4, 0, $$.ast);
	        }
        ;

table_element_list:
	table_element
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| table_element_list ',' table_element
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	| table_element_list ',' error
		{
		   ast_child_tree ($1.ast, error_node ($2, $3));
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	|  error ',' table_element
		{
		   $$.ast = new_tree (ank_list, 0, $2, $3);
		   ast_child_tree ($$.ast, error_node ($2, $2));
		   ast_child_tree ($$.ast, $3.ast);
		}
	;

table_element:
	column_definition
	| constraint_clause
	;

column_definition:
	column_name column_type constraint_clause_opt
		{
		   ast_tree * at_field;
		   sqltype tlast;

		   if ( ! is_empty ($3) ) {
		     tlast = $3;
		   } else {
		     tlast = $2;
		   }

		   $$.ast = new_tree (ank_list_decl, $1.sym, $1, tlast);
		   ast_child_tree ($$.ast, $2.ast);

		   at_field = reference_type_tree (ank_field_decl, $1.sym,
						  $1, $1, REFMASK_AST);
		   ast_child_tree (at_field, $1.ast);
		   ast_decldefined (at_field);
		   ast_child_tree ($$.ast, at_field);

		   ast_defined ($$.ast);
		}
	;

column_type:
	scalar_type
		{
		   ast_relation ($$.ast, rk_type);
		}
	;

constraint_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| constraint_clause
	;

constraint_clause:
	constraint_item_list
	;

constraint_item_list:
	constraint_item
		{
		   $$.ast = new_tree (ank_list,
			 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| constraint_item_list constraint_item
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

/* removed NUMERIC_LITERAL from PCTFREE, PCTUSED, INITRANS, MAXTRANS,
 * DEGREE, and INSTANCES so that their values can be a &variable
 * (identifier)
 */
constraint_item:
	Y_CONSTRAINT
	| SYM_IDENTIFIER
	| Y_NULL
	| Y_NOT Y_NULL
	| Y_UNIQUE
	| Y_UNIQUE '(' column_list ')'
	| Y_PRIMARY Y_KEY
	| Y_PRIMARY Y_KEY '(' column_list ')'
	| Y_FOREIGN Y_KEY '(' column_list ')'
	| Y_DEFAULT literal_item
		{
		   $$.ast = $2.ast;
		}
	| Y_DEFAULT Y_NULL
	| Y_DEFAULT Y_USER
		{
		   $$.ast = symbol_call_expr (&$2, $2, $2);
		}
	| Y_DEFAULT SYM_IDENTIFIER
		{
		   /* includes UID and SYSDATE */
		   $$.ast = symbol_call_expr (&$2, $2, $2);
		}
	| Y_CHECK '(' search_condition ')'
	| Y_REFERENCES other_table_reference
	| Y_REFERENCES other_table_reference '(' column_list ')'
	| Y_USING Y_INDEX
	| Y_PCTFREE
	| Y_PCTUSED
	| Y_INITRANS
	| Y_MAXTRANS
	| Y_TABLESPACE SYM_IDENTIFIER
	| storage_clause
	| Y_ON Y_DELETE Y_CASCADE
	| Y_CASCADE
	| Y_EXCEPTIONS Y_INTO other_table_reference
	| Y_DISABLE	
	| Y_ENABLE
	| Y_CACHE
	| Y_NOCACHE
	| Y_CLUSTER SYM_IDENTIFIER '(' column_list ')'
	| parallel_clause
	| Y_DEGREE
	| Y_INSTANCES
	| Y_IS
	| NUMERIC_LITERAL
	;

storage_clause:
	Y_STORAGE '(' storage_item_list ')'
	;

storage_item_list:
	storage_item
	| storage_item_list storage_item
	;

storage_item:
	Y_INITIAL
	| Y_NEXT
	| Y_PCTINCREASE
	| Y_MINEXTENTS
	| Y_MAXEXTENTS
	| Y_OPTIMAL
	| Y_OPTIMAL Y_NULL
	| Y_FREELISTS
	| Y_FREELIST Y_GROUPS
	| NUMERIC_LITERAL
	| SYM_IDENTIFIER
	| '='
	;

parallel_clause:
	Y_PARALLEL '(' parallel_item_list ')'
	| Y_PARALLEL
	| Y_NOPARALLEL
	;

parallel_item_list:
	parallel_item
	| parallel_item_list parallel_item
	;

parallel_item:
	Y_DEGREE
	| Y_INSTANCES
	| Y_DEFAULT
	| NUMERIC_LITERAL
	| SYM_IDENTIFIER
	;

as_query_opt:
	/* empty */	{ set_empty (&$$); }
	| as_query
	;

as_query:
	Y_AS select_query
		{
		   set_result (&$$, $1, $2, 0, $2.ast);
		}
	;

/**************** View Definition ******************/

/* does not include the CREATE ... VIEW part */
view_definition:
	table_reference column_list_opt
	as_is select_query with_check_opt sql_eos
		{
		   ast_tree * at_block;
		   sqltype tblock;
		   
		   if (is_empty ($2)) {
		      tblock = $3;
		   } else {
		      tblock = $2;
		   }

		   $$.ast = new_tree (ank_type_decl, $1.sym, $1, $6);
		   ast_child_tree ($$.ast, $4.ast);

		   at_block = new_tree (ank_ast_block, 0, tblock, $4);
		   ast_child_tree (at_block, $2.ast);
		   ast_child_tree (at_block, $4.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

with_check_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_WITH Y_CHECK Y_OPTION
	;	


/**************** Connect  ******************/

connect_command:
        Y_CONNECT
		{
		   need_end_of_line_token = 1;
		}
	user_name password_opt dblink_name_opt semicolon_opt sql_eol
        ;

password_opt:
	/* empty */	{ set_empty (&$$); }
	| '/' SYM_IDENTIFIER
	;	


/**************** Misc definitions ******************/

misc_definition:
	Y_ALTER
		{
		   sql_allow_error = 1;
		}
	| Y_DROP
		{
		   sql_allow_error = 1;
		}
	| Y_GRANT
		{
		   sql_allow_error = 1;
		}
	| Y_REVOKE
		{
		   sql_allow_error = 1;
		}
	| misc_create
	;

misc_create:
	Y_CREATE Y_CONTROLFILE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_CLUSTER
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_DATABASE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_INDEX
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_PROFILE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_PUBLIC
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_ROLE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_ROLLBACK
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_SEQUENCE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_SNAPSHOT
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_SYNONYM
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_TABLESPACE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_UNIQUE
		{
		   sql_allow_error = 1;
		}
	| Y_CREATE Y_USER
		{
		   sql_allow_error = 1;
		}
	;


/**************** Explain Plan ******************/

/* keyword is "PLAN" */
explain_prefix:
	Y_EXPLAIN keyword
	set_stmt_id_opt into_table_opt
	Y_FOR
	;

/* keyword is "statement_id" */
set_stmt_id_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_SET keyword	'=' STRING_LITERAL
	;

into_table_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_INTO other_table_reference
	;

/**************** PL/SQL Block ******************/

plsql_mainblock: 
	label_declaration_opt plsql_block
			{
			   set_result (&$$, $1, $2, 0, $2.ast);
			}
        ;

plsql_block: 
	block_begin
	statement_list
	exception_clause_opt
	Y_END label_opt ';'
			{
			   $$.ast = new_tree (ank_ast_block,
			      0, $1, $6);

			   /* block_begin */
			   ast_child_tree ($$.ast, $1.ast);

			   /* statement_list */
			   ast_child_tree ($$.ast, $2.ast);

			   /* exception_clause_opt */
			   ast_child_tree ($$.ast, $3.ast);

			   /* label_opt */
			   ast_child_tree ($$.ast, symbol_reference ($5));

			   sql_pop_scope ();
			   set_result (&$$, $1, $6, 0, $$.ast);
			}
	;

block_begin:
	Y_BEGIN
		{
		   sql_push_scope (0, 0);
		   if (esql_mode) {
		      sql_scanner_sql_mode ();
		   }
		}
	| Y_DECLARE
		{
		   sql_push_scope (0, 0);
		   if (esql_mode) {
		      sql_scanner_sql_mode ();
		   }
		}
          object_declaration_list_opt
          subprogram_declaration_list_opt
          Y_BEGIN
		{
		   $$.ast = new_tree (ank_list, 0, $1, $5);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

/**************** Labels ******************/

label_declaration_opt:
	/* empty */	{ set_empty (&$$); }
	| label_declaration
	;

label_declaration: 
	OP_LABEL_START label_name OP_LABEL_END
		{
		   $$.ast = new_tree (ank_label_decl, $2.sym, $1, $3);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   set_result (&$$, $1, $3, $2.sym, $$.ast);
		}
	;

/*
 * label is first encountered either in a label_declaration
 * or in a goto statement.
 *
 * TBD: a label can be used to identify scope: label.variable
 */
label_name:
	SYM_IDENTIFIER	
			{
			   /* check if unset or redefine */
			   if (symbol_get ($1) != sk_label) {
			      $1.sym = symbol_set ($1, sk_label);
			   }
			   $$.sym = $1.sym;
			   $$.ast = symbol_token ($1);
			}
	;

label_opt:
	/* empty */	{ set_empty (&$$); }
	| SYM_IDENTIFIER
		{
		   if ($1.sym) {
		     $$.ast = symbol_token ($1);
		   }
		}
	;


/**************** Declarations ******************/


object_declaration_list:
	object_declaration
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| object_declaration_list object_declaration
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

/* basic data objects */
object_declaration:
	constant_declaration
	| cursor_declaration
	| exception_declaration
	| exception_init_pragma
	| record_type_definition
	| ref_type_definition
	| subtype_definition
	| table_type_definition
	| variable_declaration
	| error_in_statement
	;

constant_declaration:
	identifier Y_CONSTANT
		{
		   $1.sym = symbol_set ($1, sk_variable);
		}
	constant_type default_value ';'
		{
		   ast_tree * at_decl;

		   $$.ast = new_tree (ank_list_decl, $1.sym, $1, $6);

		   /* put ast_declspec first */
		   ast_child_tree ($$.ast, $4.ast);

		   at_decl = reference_type_tree (ank_var_decl, $1.sym,
						  $1, $6, REFMASK_AST);
		   ast_child_tree (at_decl, symbol_token ($1));
		   ast_child_tree (at_decl, $5.ast);
		   ast_decldefined (at_decl);
		   ast_child_tree ($$.ast, at_decl);

		   ast_defined ($$.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

constant_type:
	data_type
		{
		   ast_relation_attr ($$.ast, RELMASK_CONST);
		   ast_relation ($$.ast, rk_type);
		}
	;

default_value_opt:
	/* empty */	{ set_empty (&$$); }
	| default_value
	;

default_value:
	not_null_opt default_assign expression
		{
		   sqltype tfirst;

		   if ( ! is_empty ($1) ) {
		      tfirst = $1;
		   } else {
		      tfirst = $2;
		   }

		   /* handle not null as attribute */
		   set_result (&$$, tfirst, $3, 0, $3.ast);
		}
	;

not_null_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_NOT Y_NULL
		{
		   /* TBD: set notnull attribute */
		   set_result (&$$, $1, $2, 0, 0);
		}
	;

default_assign:
	OP_ASSIGN
	| Y_DEFAULT
	;


variable_declaration:
	first_identifier
		{
		   $1.sym = symbol_set ($1, sk_variable);
		}
	variable_type default_value_opt ';'
		{
		   ast_tree * at_decl;
		   sqltype tdecl = $4;

		   if (is_empty (tdecl)) {
		      tdecl = $1;
		   }

		   $$.ast = new_tree (ank_list_decl, $1.sym, $1, $5);
		   
		   /* put ast_declspec first */
		   ast_child_tree ($$.ast, $3.ast);

		   at_decl = reference_type_tree (ank_var_decl, $1.sym,
						  $1, tdecl,
						  REFMASK_AST);
		   ast_child_tree (at_decl, symbol_token ($1));
		   ast_child_tree (at_decl, $4.ast);
		   ast_decldefined (at_decl);
		   ast_child_tree ($$.ast, at_decl);

		   ast_defined ($$.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

variable_type:
	data_type
		{
		   ast_relation ($$.ast, rk_type);
		}
	;

data_type:
	scalar_type
	| SYM_IDENTIFIER '.' scalar_type
		{
		   /* TBD: what to do with id (package)? */
		   set_result (&$$, $3, $3, $3.sym, $3.ast);
		}
	| SYM_IDENTIFIER
		{
		   if (! symbol_is_known ($1) ) {
		      /* TBD: not clear if this is allowed - to have
		       * an undeclared table_type.  Should really
		       * be "package_name.table_type_name".
		       */
		      $1.sym = symbol_extern ($1, sk_table_type);
		   } else {
		      /* could be record type, etc */
		      if (symbol_get ($1) == sk_ref_type) {
			 symbol_attribute ($1.sym, ATTRMASK_CURSOR);
		      }
		   }
		   $$.ast = reference_tree (ank_ast_declspec, $1.sym, $1, $1);

		   set_result (&$$, $1, $1, $1.sym, $$.ast);
		}
	| SYM_IDENTIFIER Y_PCT_ROWTYPE
		{
		   /* sk_table or sk_cursor */
		   if ( ! symbol_is_known ($1) ) {
		     /* new sk_table reference */
		     $1.sym = symbol_extern ($1, sk_table);
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $1.sym, $1, $2,
						 REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));

		   ast_relation_attr ($$.ast, RELMASK_PCTROWTYPE);
		   set_result (&$$, $1, $2, $1.sym, $$.ast);
		}
	| SYM_IDENTIFIER Y_PCT_TYPE
		{
		   /* sk_variable (cursor) */
		   /* sk_record */
		   /* sk_variable */
		   /* local sk_field */
		   if ( ! $1.sym ) {
		     $1.sym = symbol_extern ($1, sk_externref);
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $1.sym, $1, $2,
						 REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));

		   ast_relation_attr ($$.ast, RELMASK_PCTTYPE);
		   set_result (&$$, $1, $2, $1.sym, $$.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER data_type_args_opt
		{
		   /* sk_package . <type> */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s",
		      token_text ($1), token_text ($3));

		   $$.sym = sql_symbol_lookup (txtbuf);
		   if (! symbol_is_known ($$)) {
		      $$.sym = symbol_type_text (txtbuf);
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $$.sym, $1, $3,
						 REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $3));

		   set_result (&$$, $1, $3, $$.sym, $$.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   /* sk_schema . sk_package . <type> */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }

		   if ( ! symbol_is_known ($3) ) {
		      $1.sym = symbol_extern ($3, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s",
		      token_text ($3), token_text ($5));

		   $$.sym = sql_symbol_lookup (txtbuf);
		   if (! symbol_is_known ($$)) {
		      $$.sym = symbol_type_text (txtbuf);
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $$.sym, $1, $5,
						 REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference ($3));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $5));

		   set_result (&$$, $1, $3, $$.sym, $$.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER Y_PCT_TYPE
		{
		   /* sk_record . sk_field */
		   /* sk_table . sk_column */
		   if ( ! $1.sym ) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   switch (symbol_get ($1)) {
		      case sk_table:
			 $$.sym = sql_symbol_extern (txtbuf, sk_column);
			 break;
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
			 /* get/set field of record */
			 $$.sym = symbol_field ($1.sym, txtbuf);
			 break;
		      case sk_externref:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    iff_error_token (get_token ($3.start_token),
			       "unknown data type symbol");
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $$.sym, $1, $4, REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $3));

		   ast_relation_attr ($$.ast, RELMASK_PCTTYPE);
		   set_result (&$$, $1, $4, $$.sym, $$.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' SYM_IDENTIFIER Y_PCT_TYPE
		{
		   /* sk_record . sk_field */
		   /* sk_table . sk_column */
		   if ( ! $1.sym ) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   if ( ! $3.sym ) {
		      $3.sym = symbol_extern ($3, sk_externref);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($3), token_text ($5));

		   switch (symbol_get ($3)) {
		      case sk_table:
			 $$.sym = sql_symbol_extern (txtbuf, sk_column);
			 break;
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
			 /* get/set field of record */
			 $$.sym = symbol_field ($3.sym, txtbuf);
			 break;
		      case sk_externref:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    iff_error_token (get_token ($5.start_token),
			       "unknown data type symbol");
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $$.sym, $1, $6, REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference ($3));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $5));

		   ast_relation_attr ($$.ast, RELMASK_PCTTYPE);
		   set_result (&$$, $1, $6, $$.sym, $$.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER Y_PCT_ROWTYPE
		{
		   if ( ! symbol_is_known ($1) ) {
		     /* new sk_schema reference */
		     $1.sym = symbol_extern ($1, sk_schema);
		   }

		   if ( ! symbol_is_known ($3) ) {
		     /* new sk_table reference */
		     $$.sym = symbol_extern ($3, sk_table);
		   } else {
		     /* sk_table */
		     $$.sym = $3.sym;
		   }

		   $$.ast = reference_type_tree (ank_ast_declspec,
						 $$.sym, $1, $4, REFMASK_AST);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $3));

		   ast_relation_attr ($$.ast, RELMASK_PCTTYPE);
		   set_result (&$$, $1, $4, $$.sym, $$.ast);
		}
	;

data_type_args_opt:
	/* empty */	{ set_empty (&$$); }
	| data_type_args
	;

data_type_args:
	'(' NUMERIC_LITERAL ')'
	;

subprogram_declaration_list_opt:
	/* empty */	{ set_empty (&$$); }
	| subprogram_declaration_list
	;

subprogram_declaration_list:
	subprogram_declaration
		{
		   $$.ast = new_tree (ank_list,
			 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| subprogram_declaration_list subprogram_declaration
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

subprogram_declaration:
	function_body
	| procedure_body
	;

/*
 * Built-In Datatypes
 *
 * Not all of these are native Oracle SQL datatypes.
 * Some are PL/SQL only (BOOLEAN), others are for for compatibility
 * with ANSI SQL, SQL/DS and DB2 (DOUBLE_PRECISION, NUMERIC,
 * REAL, SMALLINT, VARCHAR)
 */


scalar_type:
	scalar_base_type
	| scalar_base_type scalar_size
		{
		   /* check bit mask set in scalar_size */
		   if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size ($1.ast, datatype_size);
		   }
		   if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale ($1.ast, datatype_scale);
		   }
		      
		   ast_extend_end ($1.ast, token_loc_end ($2));
		   set_result (&$$, $1, $2, $1.sym, $1.ast);
		}
	| scalar_informix
	;

scalar_size:
	'(' NUMERIC_LITERAL ')'
		{
		   datatype_mask = RELMASK_SIZE;
		   datatype_size = atoi (token_text ($2));
		   set_result (&$$, $1, $3, 0, 0);
		}
	| '(' SYM_IDENTIFIER ')'
		{
		   datatype_mask = 0;
		   set_result (&$$, $1, $3, 0, 0);
		}
	| '(' '*' ')'
		{
		   /* valid only for Y_NUMBER */
		   datatype_mask = 0;
		   set_result (&$$, $1, $3, 0, 0);
		}
	| '(' NUMERIC_LITERAL ',' NUMERIC_LITERAL ')'
		{
		   datatype_mask = RELMASK_SIZE | RELMASK_SCALE;
		   datatype_size = atoi (token_text ($2));
		   datatype_scale = atoi (token_text ($4));
		   set_result (&$$, $1, $5, 0, 0);
		}
	| '(' NUMERIC_LITERAL ',' SYM_IDENTIFIER ')'
		{
		   datatype_mask = RELMASK_SIZE;
		   datatype_size = atoi (token_text ($2));
		   set_result (&$$, $1, $5, 0, 0);
		}
	| '(' SYM_IDENTIFIER ',' NUMERIC_LITERAL ')'
		{
		   datatype_mask = RELMASK_SCALE;
		   datatype_scale = atoi (token_text ($4));
		   set_result (&$$, $1, $5, 0, 0);
		}
	| '(' SYM_IDENTIFIER  ',' SYM_IDENTIFIER ')'
		{
		   datatype_mask = 0;
		   set_result (&$$, $1, $5, 0, 0);
		}
	| '(' '*' ',' NUMERIC_LITERAL ')'
		{
		   /* valid only for Y_NUMBER */
		   datatype_mask = RELMASK_SCALE;
		   datatype_scale = atoi (token_text ($4));
		   set_result (&$$, $1, $5, 0, 0);
		}
	| '(' '*'  ',' SYM_IDENTIFIER ')'
		{
		   /* valid only for Y_NUMBER */
		   datatype_mask = 0;
		   set_result (&$$, $1, $5, 0, 0);
		}
	;

scalar_base_type:
	scalar_base_single
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	| Y_CHARACTER
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	| Y_CHARACTER Y_VARYING
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_DOUBLE Y_PRECISION
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_LONG
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	| Y_LONG Y_RAW
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_LONG Y_VARCHAR
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_LONG Y_VARRAW
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_RAW
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	| Y_RAW Y_MLSLABEL
		{
		   sprintf (txtbuf, "%s %s",
		      token_text ($1), token_text ($2));
		   $$.sym = symbol_type_text (txtbuf);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $2);
		   ast_keyword ($$.ast);
		}
	| Y_ROWID
		{
		   /* TBD: ROWID is both a builtin function and datatype
		    * keep the first occurance to avoid a conflict
		    * until overloading is implemented
		    */
		   if ( ! symbol_is_known ($1) ) {
		      $$.sym = symbol_type ($1);
		   } else {
		      $$.sym = $1.sym;
		   }
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	;

scalar_base_single:
	Y_BINARY_INTEGER
	| Y_BOOLEAN
	| Y_CHAR
	| Y_CHARF
	| Y_CHARZ
	| Y_DATE
	| Y_DEC
	| Y_DECIMAL
	| Y_DISPLAY
	| Y_FLOAT
	| Y_INT
	| Y_INTEGER
	| Y_MLSLABEL
	| Y_MONEY
	| Y_NATURAL
	| Y_NCHAR
	| Y_NUMBER
	| Y_NUMERIC
	| Y_NVARCHAR
	| Y_POSITIVE
	| Y_REAL
	| Y_SERIAL
	| Y_SMALLFLOAT
	| Y_SMALLINT
	| Y_STRING
	| Y_UNSIGNED
	| Y_VARCHAR
	| Y_VARCHAR2
	| Y_VARNUM
	| Y_VARRAW
	;

scalar_informix:
	byte_datatype byte_datatype_location_opt
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	| datetime_datatype
		{
		   /* turn on datetime keywords: YEAR, MONTH, etc. */
		   sql_keyword_command (SQLKEYC_DATETIME);
		}
	datetime_qualifier
		{
		   $$.sym = symbol_type ($1);
		   $$.ast = reference_tree (ank_ast_declspec, $$.sym, $1, $1);
		   ast_keyword ($$.ast);
		}
	;

byte_datatype:
	Y_BYTE
	| Y_TEXT
	;

byte_datatype_location_opt:
	/* empty */	{ set_empty (&$$); }
	| byte_datatype_location
	;

/* IN TABLE or blobspace */
byte_datatype_location:
	Y_IN Y_TABLE
	| Y_IN SYM_IDENTIFIER
	;

datetime_datatype:
	Y_DATETIME
	| Y_INTERVAL
	;

datetime_qualifier:
	datetime_part datetime_precision_opt
	Y_TO datetime_part datetime_precision_opt
	;

datetime_part:
	Y_YEAR
	| Y_MONTH
	| Y_DAY
	| Y_HOUR
	| Y_MINUTE
	| Y_SECOND
	| Y_FRACTION
	;

datetime_precision_opt:
	/* empty */	{ set_empty (&$$); }
	| datetime_precision
	;

datetime_precision:
	'(' NUMERIC_LITERAL ')'
	;


/**************** Exceptions ******************/

exception_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| exception_clause
	;

exception_clause:
	Y_EXCEPTION exception_handler_list
		{
		   $$.ast = new_tree (ank_ast_block, 0, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	| Y_EXCEPTION error
		{
		   $$.ast = new_tree (ank_ast_block, 0, $1, $2);
		   ast_child_tree ($$.ast, error_node ($1, $1));
		}
	;


exception_handler_list:
	exception_handler
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| exception_handler_list exception_handler
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

exception_init_pragma: 
	Y_PRAGMA SYM_IDENTIFIER
	'(' exception_name ',' error_number ')' ';'
		{
		   $$.ast = symbol_call_stmt (&$2, $1, $7, $8);

		   /* exception_name */
		   ast_child_tree ($2.ast, $4.ast);

		   /* error_number */
		   ast_child_tree ($2.ast, $6.ast);
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

exception_declaration: 
	SYM_IDENTIFIER Y_EXCEPTION ';'
		{
		   $$.sym = symbol_set ($1, sk_exception);

		   $$.ast = new_tree (ank_exception_decl,
			$$.sym, $1, $3);
		   ast_child_tree ($$.ast, symbol_tree ($$.sym, $1));
		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   set_result (&$$, $1, $3, $$.sym, $$.ast);
		}
	;

exception_handler:
	Y_WHEN exception_name_clause Y_THEN statement_list
		{
		   $$.ast = new_tree (ank_exception_handler, 0, $1, $4);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| Y_WHEN error Y_THEN statement_list
		{
		   $$.ast = new_tree (ank_exception_handler, 0, $1, $4);
		   ast_child_tree ($$.ast, error_node ($1, $3));
		   ast_child_tree ($$.ast, $4.ast);
		}
	;

exception_name_clause:
	exception_name_list
	| Y_OTHERS
		{
		   $$.sym = symbol_extern ($1, sk_exception);
		   $$.ast = symbol_tree ($$.sym, $1);
		}
	;

exception_name_list:
	exception_name
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| exception_name_list Y_OR exception_name
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

exception_name:
	SYM_IDENTIFIER
		{
		   if ($1.sym) {
		      if ( ! symbol_check ($1, sk_exception)) {
			 iff_error_token (get_token ($1.start_token),
			       "expecting exception name");
		      }
		      $$.sym = $1.sym;
		   } else {
		      $$.sym = symbol_extern ($1, sk_exception);
		   }

		   $$.ast = symbol_tree ($$.sym, $1);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s",
		      token_text ($1), token_text ($3));
		   $$.sym = sql_symbol_extern (txtbuf, sk_exception);

		   $$.ast = symbol_tree_ast ($$.sym, $1, $3);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $3));
		}
	;

error_number:
	NUMERIC_LITERAL
		{
		   $$.ast = literal_tree ($1);
		}
	| '-' NUMERIC_LITERAL
		{
		   $$.ast = new_tree (ank_negate_expr, 0, $1, $2);
		   ast_child_tree ($$.ast, literal_tree ($2));
		}
	| '+' NUMERIC_LITERAL
		{
		   $$.ast = literal_tree ($2);
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	;

/**************** Statements ******************/


statement_list:
	plsql_stmt
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| statement_list plsql_stmt
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

/* list of PL/SQL statements */
plsql_stmt:
	identifier_statement
	| begin_work_statement
	| close_statement
	| commit_statement
	| delete_statement
	| exit_statement
	| fetch_statement
	| goto_statement
	| if_statement
	| insert_statement
	| label_declaration
	| lock_table_statement
	| loop_statement
	| null_statement
	| open_for_statement
	| open_statement
	| plsql_table_delete_statement
	| raise_statement
	| return_statement
	| rollback_statement
	| savepoint_statement
	| select_statement
	| set_transaction_statement
	| update_statement
	| plsql_block
	| error_in_statement
	;

/*
 * Catch parse errors by skiping to the next semi-colon
 * which is the end-of-statement character.
 */
error_in_statement:
	error ';'
		{
		   error_result (&$$, $2, $2);
		}
	;

/* 
 * An assignment (x := y) or function/procedure call.
 *
 * Note that PL/SQL functions/procedures that take no arguments
 * do not use parenthesis.
 */
identifier_statement:
	identifier_clause OP_ASSIGN expression ';'
		{
		   /* assignment */
		   $$.ast = new_tree (ank_assign_stmt, 0, $1, $4);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	| identifier_clause ';'
		{
		   /* function/procedure call */
		   $$.ast = new_tree (ank_expr_stmt, 0, $1, $2);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_relation ($$.ast, rk_call);
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	;


identifier_clause:
	symbol_name
	;

identifier_list:
	symbol_name
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| identifier_list ',' symbol_name
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

/**************** Begin Work ******************/

begin_work_statement:
	Y_BEGIN Y_WORK without_replication_opt ';'
		{
		   $$.ast = sql_call_stmt ("BEGIN WORK", $1, $2, $3);
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
	;

/* INFORMIX IDS option */
without_replication_opt:
	/* empty */	{ set_empty (&$$); }
	| SYM_IDENTIFIER SYM_IDENTIFIER
		{
		   symbol_keyword_check ($1, "WITHOUT");
		   symbol_keyword_check ($2, "REPLICATION");
		}
	;

/**************** Close ******************/

close_statement:
	Y_CLOSE
		{
		   esql_prefix_check ();
		   sql_identifier_next = 1;
		   /* check for CLOSE DATABASE command in cursor_handle */
		   alternate_command = 0;
		   if (dbvendor == DBVENDOR_INFORMIX) {
		      alternate_check = 1;
		   }
		}
	cursor_handle cursor_parameter_name_list_opt ';'
		{
		   alternate_check = 0;
		   if (alternate_command) {
		      sprintf (txtbuf, "%s %s",
			       token_text ($1), token_text ($3));
		      $$.ast = sql_call_stmt (txtbuf, $1, $3, $5);
		   } else {
		      $$.ast = symbol_call_stmt (&$1, $1, $3, $5);
		      ast_child_tree ($1.ast, $3.ast);
		      ast_child_tree ($1.ast, $4.ast);
		   }
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

cursor_handle:
	SYM_IDENTIFIER
		{
		   /* sk_cursor */
		   /* sk_variable (cursor) */
		   if ( ! symbol_is_known ($1)) {
		      /* check if really CLOSE DATABASE command */
		      if (alternate_check && 
			  symbol_keyword_check ($1, "DATABASE")) {
			 alternate_command = 1;
		      } else {
			 $1.sym = symbol_set ($1, sk_cursor);
		      }
		   }
		   if ( ! alternate_command ) {
		      $$.ast = symbol_token ($1);
		   }

		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   $$.sym = sql_symbol_lookup (txtbuf);

		   if ( ! symbol_is_known ($$) ) {
		      $$.sym = sql_symbol_extern (txtbuf, sk_cursor);
		   }

		   $$.ast = symbol_tree_ast ($$.sym, $1, $3);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast,
				   symbol_reference_tree ($$.sym, $3));
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.'  SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }

		   if ( ! symbol_is_known ($3) ) {
		      $1.sym = symbol_extern ($3, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($3), token_text ($5));

		   $$.sym = sql_symbol_lookup (txtbuf);

		   if ( ! symbol_is_known ($$) ) {
		      $$.sym = sql_symbol_extern (txtbuf, sk_cursor);
		   }

		   $$.ast = symbol_tree_ast ($$.sym, $1, $5);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference ($3));
		   ast_child_tree ($$.ast,
				   symbol_reference_tree ($$.sym, $5));
		}
	| host_cursor_variable
	;

/**************** Commit ******************/

commit_statement:
	Y_COMMIT
		{
		   sql_keyword_command (SQLKEYC_COMMIT);
		   esql_prefix_check ();
		}
	work_opt commit_option_list_opt sql_eos
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $1, $5);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

work_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_WORK
	;

commit_option_list_opt:
	/* empty */	{ set_empty (&$$); }
	| commit_option_list
	;

commit_option_list:
	commit_option
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| commit_option_list commit_option
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

commit_option:
	Y_COMMENT STRING_LITERAL
		{
		   $$.ast = string_literal_tree ($2);
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	| Y_RELEASE
	| Y_FORCE STRING_LITERAL
		{
		   $$.ast = string_literal_tree ($2);
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	| Y_FORCE STRING_LITERAL ',' NUMERIC_LITERAL
		{
		   $$.ast = string_literal_tree ($2);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

/**************** Cursors ******************/

cursor_declaration:
	cursor_decl Y_IS select_query ';'
		{
		   $$.ast = new_tree (ank_cursor_def, $1.sym, $1, $4);

		   /* TBD: need to create virtual record for cursor */
		   /* empty (implies unknown) return type */
		   ast_child_tree ($$.ast, empty_tree (ank_ast_declspec));

		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_defined ($$.ast);
		   ast_decldefined ($1.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

cursor_decl:
	Y_CURSOR SYM_IDENTIFIER
		{
		   $2.sym = symbol_set ($2, sk_cursor);
		   parameter_count = 0;
		   sql_push_scope ($2.sym->sym_text, 0);
		}
	parameters_opt
		{
		   sqltype tlast;

		   if ( ! is_empty ($4) ) {
		     tlast = $4;
		   } else {
		     tlast = $2;
		   }

		   $$.ast = new_tree (ank_cursor_decl, $2.sym, $1, tlast);
		   ast_child_tree ($$.ast, symbol_token ($2));
		   ast_child_tree ($$.ast, $4.ast);

		   set_result (&$$, $1, tlast, $2.sym, $$.ast);
		}
	;

cursor_specification:
	cursor_decl Y_RETURN cursor_return_type ';'
		{
		   $$.ast = new_tree (ank_list_decl, $1.sym, $1, $4);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $1.ast);
		   /* cursor_decl pushes scope */
		   sql_pop_scope ();
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;
	
cursor_return_type:
	cursor_type
		{
		   ast_relation_attr ($$.ast, RELMASK_RETURN);
		}
	;

cursor_type:
	data_type
		{
		   ast_relation ($$.ast, rk_type);
		}
	;

cursor_body:
	cursor_decl Y_RETURN cursor_return_type Y_IS
        select_query ';'
		{
		   $$.ast = new_tree (ank_cursor_def, $1.sym, $1, $6);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		   ast_defined ($$.ast);
		   ast_decldefined ($1.ast);
		   /* cursor_decl pushes scope */
		   sql_pop_scope ();
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

ref_type_definition: 
	type_clause SYM_IDENTIFIER Y_IS Y_REF Y_CURSOR
		{
		   $2.sym = symbol_set ($2, sk_ref_type);
		}
	Y_RETURN cursor_type ';'
		{
		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $9);
		   ast_child_tree ($$.ast, symbol_token ($2));
		   ast_child_tree ($$.ast, $8.ast);
		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   set_result (&$$, $1, $9, 0, $$.ast);
		}
	;

type_clause:
	Y_TYPE
		{
		   sql_keyword_command (SQLKEYC_TYPE);
		}
	;

/**************** Database (Informix) ******************/

database_statement:
	database_keyword database_name exclusive_opt sql_eos
		{
		   sqltype tlast = $3;
		   if (is_empty($3)) {
		      tlast = $2;
		   }

		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $4);
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

database_keyword:
	Y_DATABASE 
		{
		   esql_prefix_check ();
		   sql_keyword_command (SQLKEYC_DATABASE);
		}
	;

database_name:
	SYM_IDENTIFIER dblink_name_opt
		{
		   sqltype tlast = $2;
		   if (is_empty($2)) {
		      tlast = $1;
		   }

		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }
		   $$.sym = $1.sym;
		   $$.ast = symbol_token ($1);
		   set_result (&$$, $1, tlast, $$.sym, $$.ast);
		}
	;


exclusive_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_EXCLUSIVE
	;

/**************** Delete ******************/

delete_statement: 
	Y_DELETE
		{
		   sql_keywords_only ();
		   esql_prefix_check ();
		   sql_push_scope (0, 0);
   		}
	from_opt tableref_entry where_clause_opt sql_eos
		{
		   sqltype tlast = $5;
		   if (is_empty($5)) {
		      tlast = $4;
		   }

		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $6);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

from_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_FROM
	;

tableref_entry:
	tableref_or_subquery alias_name_opt 
		{
		   if ( ! is_empty ($2) ) {
		      ast_child_tree ($1.ast, $2.ast);
		      set_result (&$$, $1, $2, 0, $1.ast);

		      /* set alias symbol to ptr to real table */
		      if ($2.sym && $1.sym) {
			 $2.sym->alias_sym = $1.sym;
		      }
		   } else {
		      set_result (&$$, $1, $1, 0, $1.ast);
		   }
		}
	;

tableref_or_subquery:
	table_reference
	| '(' subquery ')'
		{
		  set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

table_reference:
	compound_table_reference
		{
		   /* set table name (used to resolve column names) */
		   sql_scope_set_table_name (last_table_name);
		   set_result (&$$, $1, $1, $1.sym, $1.ast);
		}
	;

/* Same as table_reference but does not overwrite the table_name global.
 * Used for foreign table name references in column constraints where
 * overwriting the table_name would cause the following column definitions
 * to be associated with the foreign table instead of the primary table.
 *
 * Also used for table references in statements where no column references
 * need to be resolved (e.g. DROP TABLE).
 */
other_table_reference:
	compound_table_reference
	;

compound_table_reference:
	SYM_IDENTIFIER dblink_name_opt
		{
		   sql_tableref (&$1);
		   if ( ! is_empty ($2) ) {
		      ast_child_tree ($1.ast, $2.ast);
		      set_result (&$$, $1, $2, $1.sym, $1.ast);
		   } else {
		      set_result (&$$, $1, $1, $1.sym, $1.ast);
		   }
		}
	| SYM_IDENTIFIER '.' tableref dblink_name_opt
		{
		   sqltype tlast;

		   if ( ! is_empty ($4) ) {
		      tlast = $4;
		   } else {
		      tlast = $3;
		   }

		   /* same as user */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }

		   $$.ast = $3.ast;
		   ast_child_tree ($$.ast, symbol_token ($1));
		   ast_child_tree ($$.ast, $4.ast);
		   set_result (&$$, $1, tlast, $3.sym, $$.ast);
		}
	| SYM_IDENTIFIER dblink_name_opt HOST_PREFIX SYM_HOST_VARIABLE
		{
		   /* In Informix fully specified table names
		    * use the colon.  So these are not really host
		    * variables.
		    *	dbname@dbservername:tablename
		    */
		   if (dbvendor == DBVENDOR_INFORMIX) {
		      sql_tableref (&$4);
		      if ( ! symbol_is_known ($1) ) {
			 $1.sym = symbol_extern ($1, sk_schema);
		      }
		      $$.ast = $4.ast;
		      ast_child_tree ($$.ast, symbol_token ($1));
		      ast_child_tree ($$.ast, $2.ast);
		      set_result (&$$, $1, $4, $4.sym, $$.ast);

		   } else {
		      iff_error_token (get_token ($1.start_token),
				       "Unexpected table name syntax");
		   }
		}
	| SYM_IDENTIFIER dblink_name_opt HOST_PREFIX SYM_HOST_VARIABLE
	  '.' tableref
		{
		   /* In Informix fully specified table names
		    * use the colon.  So these are not really host
		    * variables.
		    *	dbname@dbservername:owner.tablename
		    */
		   if (dbvendor == DBVENDOR_INFORMIX) {
		      $$.ast = $6.ast;
		      if ( ! symbol_is_known ($1) ) {
			 $1.sym = symbol_extern ($1, sk_schema);
		      }
		      $4.sym = symbol_extern ($4, sk_schema);
		      ast_child_tree ($$.ast, symbol_token ($1));
		      ast_child_tree ($$.ast, $2.ast);
		      ast_child_tree ($$.ast, symbol_token ($4));
		      set_result (&$$, $1, $6, $6.sym, $$.ast);

		   } else {
		      iff_error_token (get_token ($1.start_token),
				       "Unexpected table name syntax");
		   }
		}
	;

/* table_name or view_name */
tableref:
	SYM_IDENTIFIER
		{
		   sql_tableref (&$1);
		   set_result (&$$, $1, $1, $1.sym, $1.ast);
		}
	;

dblink_name_opt:
	/* empty */	{ set_empty (&$$); }
	| dblink_name
        ;

dblink_name:
	'@' SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($2) ) {
		      $2.sym = symbol_extern ($2, sk_dblink);
		   }
		   $$.ast = symbol_tree ($$.sym, $2);
		   set_result (&$$, $1, $2, $2.sym, $$.ast);
		}
	;

alias_name:
	SYM_IDENTIFIER
		{
		   $$.sym = symbol_set ($1, sk_alias);
		   $$.ast = symbol_tree ($$.sym, $1);
		}
	;

alias_name_opt:
	/* empty */	{ set_empty (&$$); }
	| alias_name
	;

where_clause_opt:
	/* empty */	{ set_empty (&$$); }
	| where_clause
	;

where_clause:
	Y_WHERE search_condition
		{
		   $$.ast = new_tree (ank_where, 0, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	| Y_WHERE Y_CURRENT Y_OF SYM_IDENTIFIER
		{
		   /* sk_cursor */
		   symbol_check ($4, sk_cursor);
		   $$.ast = new_tree (ank_where, 0, $1, $4);
		   ast_child_tree ($$.ast, symbol_token ($4));
		}
	;

/**************** Exit ******************/

exit_statement: 
	Y_EXIT label_opt ';'
		{
		   $$.ast = new_tree (ank_exit_stmt, 0, $1, $3);
		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
	| Y_EXIT label_opt Y_WHEN boolean_expression ';'
		{
		   $$.ast = new_tree (ank_exit_stmt, 0, $1, $5);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

when_expression_list_opt:
	/* empty */	{ set_empty (&$$); }
	| when_expression_list
	;

when_expression_list:
	when_expression
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| when_expression_list when_expression
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;


when_expression:
	Y_WHEN boolean_expression
		{
		   set_result (&$$, $1, $2, 0, $2.ast);
		}
	;

/**************** Expressions ******************/

/*
 * Since in many cases the datatype of a symbol is unknown,
 * just use generic expression.
 */
boolean_expression:
	search_condition
	;

search_condition:
	expression
	;

value_expression:
	expression
	;

/*
 * Expressions can be boolean as well as numeric, date, or
 * character (string).  Grammar conflicts are resolved by
 * the precedence rules section listed up top.
 */
expression:
	expression Y_OR expression
		{
		   $$.ast = new_tree (ank_truth_or_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression Y_AND expression
		{
		   $$.ast = new_tree (ank_truth_and_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| Y_NOT expression
		{
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	| '(' expression ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	| expression '=' expression
		{
		   /* this is equality not assignment */
		   $$.ast = new_tree (ank_eq_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression OP_NE expression
		{
		   $$.ast = new_tree (ank_ne_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression OP_GE expression
		{
		   $$.ast = new_tree (ank_ge_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression OP_LE expression
		{
		   $$.ast = new_tree (ank_le_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '<' expression
		{
		   $$.ast = new_tree (ank_lt_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '>' expression
		{
		   $$.ast = new_tree (ank_gt_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '=' '(' subquery ')'
		{
		   $$.ast = new_tree (ank_eq_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression OP_NE '(' subquery ')'
		{
		   $$.ast = new_tree (ank_ne_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression OP_GE '(' subquery ')'
		{
		   $$.ast = new_tree (ank_ge_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression OP_LE '(' subquery ')'
		{
		   $$.ast = new_tree (ank_le_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression '<' '(' subquery ')'
		{
		   $$.ast = new_tree (ank_lt_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression '>' '(' subquery ')'
		{
		   $$.ast = new_tree (ank_gt_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression '=' any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_eq_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression_multilist '=' '(' subquery_or_expr_list ')'
		{
		   $$.ast = new_tree (ank_eq_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression OP_NE any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_ne_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression_multilist OP_NE '(' subquery_or_expr_list ')'
		{
		   $$.ast = new_tree (ank_ne_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression OP_GE any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_ge_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression OP_LE any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_le_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression '<' any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_lt_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression '>' any_all_some '(' subquery ')'
		{
		   $$.ast = new_tree (ank_gt_expr, 0, $1, $6);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression Y_NOT Y_BETWEEN expression OP_BETWEEN_AND expression
		{
		   ast_tree * at;
		   /*
		    * Scanner sends OP_BETWEEN_AND instead of Y_AND
		    * when it encounters an AND after a BETWEEN
		    */
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $6);
		   at = symbol_call_expr (&$3, $1, $6);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $4.ast);
		   ast_child_tree (at, $6.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_BETWEEN expression OP_BETWEEN_AND expression
		{
		   $$.ast = symbol_call_expr (&$2, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| expression Y_NOT Y_LIKE expression
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $4);
		   at = symbol_call_expr (&$3, $1, $4);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $4.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_LIKE expression
		{
		   $$.ast = symbol_call_expr (&$2, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression Y_NOT Y_LIKE expression Y_ESCAPE literal_or_host_item
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $6);
		   at = symbol_call_expr (&$3, $1, $6);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $4.ast);
		   ast_child_tree(at, $6.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_LIKE expression Y_ESCAPE literal_or_host_item
		{
		   $$.ast = symbol_call_expr (&$2, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree($$.ast, $5.ast);
		}
	| expression Y_NOT Y_MATCHES expression
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $4);
		   at = symbol_call_expr (&$3, $1, $4);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $4.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_MATCHES expression
		{
		   $$.ast = symbol_call_expr (&$2, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| value_item Y_IS Y_NOT Y_NULL
		{
		   $$.ast = new_tree (ank_ne_expr, 0, $1, $4);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, literal_tree ($4));
		}
	| value_item Y_IS Y_NULL
		{
		   $$.ast = new_tree (ank_eq_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, literal_tree ($3));
		}
	| expression Y_NOT Y_IN '(' subquery ')'
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $6);
		   at =  new_tree (ank_in_expr, 0, $1, $6);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $5.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression_multilist Y_NOT Y_IN '(' subquery_or_expr_list ')'
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $6);
		   at =  new_tree (ank_in_expr, 0, $1, $6);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $5.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_IN '(' subquery ')'
		{
		   $$.ast = new_tree (ank_in_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression_multilist Y_IN '(' subquery_or_expr_list ')'
		{
		   $$.ast = new_tree (ank_in_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| expression Y_NOT Y_IN '(' expression_list ')'
		{
		   ast_tree * at;
		   $$.ast = new_tree (ank_truth_not_expr, 0, $1, $6);
		   at =  new_tree (ank_in_expr, 0, $1, $6);

		   ast_child_tree (at, $1.ast);
		   ast_child_tree (at, $5.ast);

		   ast_child_tree ($$.ast, at);
		}
	| expression Y_IN '(' expression_list ')'
		{
		   $$.ast = new_tree (ank_in_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| Y_EXISTS '(' subquery ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $4);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '+' expression
		{
		   $$.ast = new_tree (ank_plus_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '-' expression
		{
		   $$.ast = new_tree (ank_minus_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '*' expression
		{
		   $$.ast = new_tree (ank_mult_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression '/' expression
		{
		   $$.ast = new_tree (ank_div_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| '+' expression %prec UMINUS
		{
		   /* see sql.l comment about uniary plus on a numeric const */
		   set_result (&$$, $1, $2, 0, $2.ast);
		}
	| '-' expression %prec UMINUS
		{
		   $$.ast = new_tree (ank_negate_expr, 0, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	| Y_PRIOR expression %prec UMINUS
		{
		   set_result (&$$, $1, $2, 0, $2.ast);
		}
	| expression OP_CONCAT expression
		{
		   $$.ast = symbol_call_expr (&$2, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| expression OP_EXPONENT expression
		{
		   $$.ast = new_tree (ank_expon_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| SYM_IDENTIFIER OP_ASSOCIATE expression
		{
		   set_result (&$$, $1, $3, 0, $3.ast);
		}
	| expression VARCHAR_ARRAY
		{
		   $$.ast = sql_call_expr ("[]", $1, $2);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| atom
	| value_item
	| boolean_item
	;

any_all_some:
	Y_ANY
	| Y_ALL
	| Y_SOME
	;

/* TBD: is this still used?  See scanner handling of dot separated tokens. */
plsql_table_exists:
	SYM_IDENTIFIER '.' Y_EXISTS '(' expression ')'
		{
		   $$.ast = symbol_call_expr (&$3, $1, $6);

		   /* sk_plsql_table */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_plsql_table);
		   }

		   ast_child_tree ($$.ast, symbol_token ($1));
		   ast_child_tree ($$.ast, $5.ast);
		}
	;

cursor_bool_clause:
	cursor_handle cursor_bool_test
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| Y_SQL cursor_bool_test
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		   ast_child_tree ($$.ast, symbol_call_expr (&$1, $1, $1));
		}
	;

cursor_bool_test:
	Y_PCT_FOUND
	| Y_PCT_ISOPEN
	| Y_PCT_NOTFOUND
	;

value_item:
	numeric_item
	| character_item
	| symbol_name
	;

numeric_item:
	cursor_rowcount_clause
	| SYM_IDENTIFIER plsql_table_numeric_attribute
		{
		   /* sk_plsql_table */
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_plsql_table);
		   }

		   $$.ast = $2.ast;
		   ast_child_tree ($1.ast, symbol_token ($1));
		}
	| sqlcode_function
	| group_function
	| SYM_IDENTIFIER '.' sequence_value
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_sequence); 
		   } else if ( ! symbol_check ($1, sk_sequence)) {
		      iff_error_token (get_token ($1.start_token),
			 "expecting sequence");
		   }

		   $$.ast = $3.ast;
		   ast_child_tree ($$.ast, symbol_token ($1));
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' sequence_value
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_schema); 
		   }

		   if ( ! symbol_is_known ($2) ) {
		      $1.sym = symbol_extern ($2, sk_sequence); 
		   } else if ( ! symbol_check ($2, sk_sequence)) {
		      iff_error_token (get_token ($2.start_token),
			 "expecting sequence");
		   }

		   $$.ast = $5.ast;
		   ast_child_tree ($$.ast, symbol_token ($3));
		   ast_child_tree ($$.ast, symbol_reference ($1));
		}
	;


cursor_rowcount_clause:
	cursor_handle Y_PCT_ROWCOUNT
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| Y_SQL Y_PCT_ROWCOUNT
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		   ast_child_tree ($$.ast, symbol_call_expr (&$1, $1, $1));
		}
	;

/* TBD: is this still used?  See scanner handling of dot separated tokens. */
plsql_table_numeric_attribute:
	'.' Y_COUNT
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		}
	| '.' Y_FIRST
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		}
	| '.' Y_LAST
		{
		   $$.ast = symbol_call_expr (&$2, $1, $2);
		}
	| '.' Y_NEXT '(' expression ')'
		{
		   $$.ast = symbol_call_expr (&$2, $1, $5);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| '.' Y_PRIOR '(' expression ')'
		{
		   $$.ast = symbol_call_expr (&$2, $1, $5);
		   ast_child_tree ($$.ast, $4.ast);
		}
	;

group_function:
	group_function_item '(' distinct_or_all_opt expression ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $5);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| Y_COUNT '(' distinct_or_all_opt expression ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $5);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| Y_COUNT '(' distinct_or_all_opt '*' ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $5);
		   /* TBD: what about star? ast_child_tree ($$.ast, $4.ast); */
		}
	;

/* TBD: should this still be used?  Just use builtin function table. */
group_function_item:
	Y_AVG
	| Y_MAX
	| Y_MIN
	| Y_STDDEV
	| Y_SUM
	| Y_VARIANCE
	;

character_item:
	sqlerrm_function
	| replace_function
	;

/* TBD: is this still used? */
/* The REPLACE function uses the keyword "REPLACE"
 * also used in "CREATE OR REPLACE PROCEDURE..."
 * so a separate rule is needed.
 */
replace_function:
	Y_REPLACE '(' value_expression ',' value_expression ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $6);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $5.ast);
		}
	| Y_REPLACE '(' value_expression ',' value_expression ','
	   value_expression ')'
		{
		   $$.ast = symbol_call_expr (&$1, $1, $8);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $5.ast);
		   ast_child_tree ($$.ast, $7.ast);
		}
	;

boolean_item:
	boolean_literal
	| trigger_boolean
	| cursor_bool_clause
	| plsql_table_exists
	;


/* symbols include:
 *
 *	function_call
 *	variable_name
 *
 *	alias_name
 */
symbol_name:
	single_identifier
		{
		   /* sk_variable */
		   /* sk_variable (cursor) */
		   /* sk_function */
		   /* sk_procedure */
		   /* sk_column */

		   /* Check for tablename */
		   if (sql_scope_table_name ()) {
		      SQL_SYM * sym;
		      sprintf (txtbuf, "%s.%s", 
			       sql_scope_table_name (), token_text ($1));

		      if (sym = sql_symbol_lookup (txtbuf)) {
			 if (sym->sym_token_type == sk_externref) {
			    /* now we know it is a column */
			    sym->sym_token_type = sk_column;
			 }
			 $1.sym = sym;
		      }
		   }

		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   switch (symbol_get ($1)) {
		    case sk_function:
		    case sk_procedure:
		      $$.ast = new_tree (ank_call_expr, $$.sym, $1, $1);
		      ast_child_tree ($$.ast, symbol_token ($1));
		      break;

		    default:
		      $$.ast = symbol_token ($1);
		      break;
		   }
		}
	| host_variable_symbol
	| SYM_IDENTIFIER '.' symbol_identifier
		{
		   int suffix_function = 0;

		   if ( ! $1.sym) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   switch (symbol_get ($1)) {
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
		         if (symbol_keyword_list ($3,
						  keylist_plsql_table)) {
			    suffix_function = 1;
			    $$.sym = $1.sym;
			 } else {
			    /* get/set field of record */
			    $$.sym = symbol_field ($1.sym, txtbuf);
			 }
			 break;
		      case sk_externref:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    if (symbol_keyword_list ($3,
						     keylist_sequence_value)) {
			       suffix_function = 1;
			       $1.sym = symbol_extern ($1, sk_sequence);
			       $$.sym = $1.sym;
			    } else {
			       $$.sym = sql_symbol_extern (txtbuf,
							   sk_externref);
			    }
			 }
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_global_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      case sk_table:
			 if ( ! in_select_items ) {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_column);
			 } else {
			    /* will be resolved later */
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      case sk_alias:
			 /* 
			  * track table name
			  */
			 if ($1.sym->alias_sym) {
			    sprintf (txtbuf, "%s.%s", 
				     $1.sym->alias_sym->sym_text,
				     token_text ($3));
			    $$.sym = sql_symbol_extern (txtbuf, sk_column);
			 } else {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_procedure:
		      case sk_function:
			 /* local scope */
			 if ($3.sym) {
			    $$.sym = $3.sym;
			 } else {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_plsql_table:
			 $$.sym = $1.sym;
			 if (symbol_keyword_list ($3, keylist_plsql_table)) {
			    suffix_function = 1;
			 }
			 break;
		      case sk_sequence:
			 $$.sym = $1.sym;
			 if (symbol_keyword_list ($3, keylist_sequence_value)) {
			    suffix_function = 1;
			 }
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    iff_error_text (txtbuf,
			       "unknown symbol name");
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		   }

		   switch (symbol_get ($$)) {
		    case sk_function:
		    case sk_procedure:
		      $$.ast = new_tree (ank_call_expr, $$.sym, $1, $3);
		      ast_child_tree ($$.ast,
				      symbol_tree_ast ($$.sym, $1, $3));
		      ast_child_tree ($$.ast, symbol_reference ($1));
		      ast_child_tree ($$.ast,
				      symbol_reference_tree ($$.sym, $3));
		      break;

		    default:
		      if (suffix_function) {
			 $$.ast = symbol_call_expr (&$3, $1, $3);
			 ast_child_tree ($$.ast, symbol_token ($1));
		      } else {
			 $$.ast = symbol_tree_ast ($$.sym, $1, $3);
			 ast_child_tree ($$.ast, symbol_reference ($1));
			 ast_child_tree ($$.ast,
					 symbol_reference_tree ($$.sym, $3));
		      }
		      break;
		   }
		}
	| trigger_table '.' symbol_identifier
		{
		   /* Check for tablename */
		   if (trigger_table_name) {
		      sprintf (txtbuf, "%s.%s", 
			       trigger_table_name, token_text ($3));
		      
		      $$.sym = sql_symbol_extern (txtbuf, sk_column);
		   }

		   if ( ! $$.sym) {
		      $$.sym = symbol_extern ($3, sk_externref);
		   }

		   $$.ast = symbol_tree_ast ($$.sym, $1, $3);
		   ast_child_tree ($$.ast,
				      symbol_reference_tree ($$.sym, $3));

		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   int suffix_function = 0;

		   if ( ! $1.sym) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   sprintf (txtbuf, "%s.%s.%s", 
			      token_text ($1), token_text ($3),
			      token_text ($5));

		   switch (symbol_get ($1)) {
		      case sk_schema:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    if ( ! symbol_is_known ($3) ) {
			       if (symbol_keyword_list ($5,
						     keylist_sequence_value)) {
				  suffix_function = 1;
				  $3.sym = symbol_extern ($3, sk_sequence);
				  $$.sym = $3.sym;
			       } else {
				  $3.sym = symbol_extern ($3, sk_table);
			       }
			    }
			    if (symbol_get ($3) == sk_table) {
			       $$.sym = sql_symbol_extern (txtbuf, sk_column);
			    }
			 }
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
						    sk_externref);
			 }
			 /* note: overwriting txtbuf */
			 if ( ! $3.sym ) {
			    sprintf (txtbuf, "%s.%s", 
				     token_text ($1), token_text ($3));
			    $3.sym = sql_symbol_extern (txtbuf,
						    sk_externref);
			 }
			 break;
		      default:
		         if (symbol_get ($3) == sk_table) {
			    /* TBD: add schema.table */
			    $1.sym = symbol_extern ($1, sk_schema);
		            $$.sym = sql_symbol_extern (txtbuf, sk_column);
			 } else if (symbol_keyword_list ($5,
						   keylist_sequence_value)) {
			    suffix_function = 1;
			    $1.sym = symbol_extern ($1, sk_schema);
			    $3.sym = symbol_extern ($3, sk_sequence);
			    $$.sym = $3.sym;
		         } else {
			    if ( ! $3.sym ) {
			       symbol_extern ($3, sk_externref);
			    }
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
		         }
			 break;
		   }

		   if (suffix_function) {
		      $$.ast = symbol_call_expr (&$5, $1, $5);
		      ast_child_tree ($$.ast, symbol_token ($3));
		      ast_child_tree ($$.ast, symbol_reference ($1));
		   } else {
		      $$.ast = symbol_tree_ast ($$.sym, $1, $5);
		      ast_child_tree ($$.ast, symbol_reference ($1));
		      ast_child_tree ($$.ast, symbol_reference ($3));
		      ast_child_tree ($$.ast,
				      symbol_reference_tree ($$.sym, $5));
		   }
		}
	| SYM_IDENTIFIER '(' expression_list ')'
		{
		   ast_tree * at;
		   if ( ! $1.sym) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   at = symbol_token ($1);
		   ast_resolve_function (at, $3.ast);

		   $$.ast = new_tree (ank_call_expr, 0, $1, $4);
		   ast_child_tree ($$.ast, at);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| SYM_IDENTIFIER dblink_name '(' expression_list ')'
		{
		   ast_tree * at;
		   if ( ! $1.sym) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   at = symbol_token ($1);
		   ast_resolve_function (at, $4.ast);

		   $$.ast = new_tree (ank_call_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, at);
		   ast_child_tree(at, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '(' expression_list ')'
		{
		   ast_tree * at;
		   /* sk_package . sk_function */
		   /* sk_package . sk_procedure */
		   /* sk_package . sk_plsql_table */

		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   switch (symbol_get ($1)) {
		      case sk_externref:
			 /* now we know it is a package */
		         $1.sym = symbol_extern ($1, sk_package);
			 $$.sym = sql_symbol_extern (txtbuf,
			    sk_externref);
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_global_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      case sk_procedure:
		      case sk_function:
		      case sk_label:
			 /* local scope */
			 /* TBD: make sure we grab the right one */
			 if ($3.sym) {
			    $$.sym = $3.sym;
			 } else {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
			 /* get/set field of record */
			 $$.sym = symbol_field ($1.sym, txtbuf);
			 break;
		      case sk_plsql_table:
			 $$.sym = $1.sym;
			 symbol_keyword_list ($3, keylist_plsql_table);
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    /***
			    iff_error_token (get_token ($3.start_token),
		   "unknown symbol -- expecting package function/procedure");
		             ***/
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		   }

		   switch (symbol_get ($$)) {
		    case sk_plsql_table:
		      $$.ast = symbol_call_expr (&$3, $1, $6);
		      ast_child_tree ($$.ast, symbol_token ($1));
		      ast_child_tree ($$.ast, $5.ast);
		      break;

		    default:
		      at = symbol_tree_ast ($$.sym, $1, $3);
		      ast_resolve_function (at, $5.ast);

		      $$.ast = new_tree (ank_call_expr, at->sym, $1, $6);
		      ast_child_tree ($$.ast, at);
		      ast_child_tree ($$.ast, symbol_reference ($1));
		      ast_child_tree ($$.ast,
				      symbol_reference_tree (at->sym, $3));
		      ast_child_tree ($$.ast, $5.ast);
		      break;
		   }
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER dblink_name '(' expression_list ')'
		{
		   ast_tree * at;
		   /* sk_package . sk_function */
		   /* sk_package . sk_procedure */
		   /* sk_package . sk_plsql_table */

		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   switch (symbol_get ($1)) {
		      case sk_externref:
			 /* now we know it is a package */
		         $1.sym = symbol_extern ($1, sk_package);
			 $$.sym = sql_symbol_extern (txtbuf,
			    sk_externref);
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_global_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      case sk_procedure:
		      case sk_function:
		      case sk_label:
			 /* local scope */
			 /* TBD: make sure we grab the right one */
			 if ($3.sym) {
			    $$.sym = $3.sym;
			 } else {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
			 /* get/set field of record */
			 $$.sym = symbol_field ($1.sym, txtbuf);
			 break;
		      case sk_plsql_table:
			 $$.sym = $1.sym;
			 symbol_keyword_list ($3, keylist_plsql_table);
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    /***
			    iff_error_token (get_token ($3.start_token),
		   "unknown symbol -- expecting package function/procedure");
		             ***/
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		   }

		   switch (symbol_get ($$)) {
		    case sk_plsql_table:
		      $$.ast = symbol_call_expr (&$3, $1, $7);
		      at = symbol_token($1);
		      ast_child_tree ($$.ast, at);
		      ast_child_tree(at, $4.ast);
		      ast_child_tree ($$.ast, $5.ast);
		      break;

		    default:
		      at = symbol_tree_ast ($$.sym, $1, $3);
		      ast_resolve_function (at, $6.ast);

		      $$.ast = new_tree (ank_call_expr, at->sym, $1, $7);
		      ast_child_tree ($$.ast, at);
		      ast_child_tree(at, $4.ast);
		      ast_child_tree ($$.ast, symbol_reference ($1));
		      ast_child_tree ($$.ast,
				      symbol_reference_tree (at->sym, $3));
		      ast_child_tree ($$.ast, $6.ast);
		      break;
		   }
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' SYM_IDENTIFIER
	 '(' expression_list ')'
		{
		   ast_tree * at;
		   /* sk_schema . sk_package . sk_function */
		   /* sk_schema . sk_package . sk_procedure */
		   /* sk_schema . sk_package . sk_plsql_table */

		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_schema);
		   }

		   if ( ! symbol_is_known ($3)) {
		      $3.sym = symbol_extern ($3, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($3), token_text ($5));

		   switch (symbol_get ($3)) {
		      case sk_externref:
			 /* now we know it is a package */
		         $3.sym = symbol_extern ($3, sk_package);
			 $$.sym = sql_symbol_extern (txtbuf,
			    sk_externref);
			 break;
		      case sk_package:
			 $$.sym = sql_symbol_global_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		      case sk_procedure:
		      case sk_function:
		      case sk_label:
			 /* local scope */
			 /* TBD: make sure we grab the right one */
			 if ($5.sym) {
			    $$.sym = $5.sym;
			 } else {
			    $$.sym = sql_symbol_extern (txtbuf,
							sk_externref);
			 }
			 break;
		      case sk_record:
		      case sk_variable:	 /* really a record variable */
			 /* get/set field of record */
			 $$.sym = symbol_field ($3.sym, txtbuf);
			 break;
		      default:
			 $$.sym = sql_symbol_lookup (txtbuf);
			 if ( ! $$.sym ) {
			    /***
			    iff_error_token (get_token ($5.start_token),
		   "unknown symbol -- expecting package function/procedure");
		             ***/
			    $$.sym = sql_symbol_extern (txtbuf,
			       sk_externref);
			 }
			 break;
		   }

		   at = symbol_tree_ast ($$.sym, $1, $5);
		   ast_resolve_function (at, $7.ast);

		   $$.ast = new_tree (ank_call_expr, at->sym, $1, $8);
		   ast_child_tree ($$.ast, at);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference ($3));
		   ast_child_tree ($$.ast,
				   symbol_reference_tree (at->sym, $5));
		   ast_child_tree ($$.ast, $7.ast);
		}
	| SYM_IDENTIFIER '(' expression_list ')' '.' SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_plsql_table);
		   }

		   sprintf (txtbuf, "%s.%s",
		      token_text ($1), token_text ($6));

		   $$.sym = sql_symbol_lookup (txtbuf);
		   if ( ! symbol_is_known ($$) ) {
		      $$.sym = sql_symbol_add (txtbuf, sk_field);
		   }

		   /* TBD: change this */
		   $$.ast = new_tree (ank_call_expr, $$.sym, $1, $6);
		   ast_child_tree ($$.ast, symbol_token ($1));
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, symbol_tree ($$.sym, $6));
		}
	| SYM_IDENTIFIER '(' expression_list ')' '(' expression_list ')'
	   '.' SYM_IDENTIFIER
		{
		   /* function that returns a plsql_table
		    * second expression specifies the record
		    * and the last identifier is the field
		    */
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_function);
		   }

		   /* TBD: not clear what the reference should be
		    * since the plsql table is not known until
		    * runtime.
		    */
		   if ( ! symbol_is_known ($9) ) {
		      $9.sym = symbol_set ($9, sk_field);
		   }

		   $$.ast = new_tree (ank_call_expr, $9.sym, $1, $9);
		   ast_child_tree ($$.ast, symbol_token ($1));
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $6.ast);
		   ast_child_tree ($$.ast, symbol_token ($9));
		}
	| SYM_IDENTIFIER '(' expression_list ')' '(' expression_list ')'
		{
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_function);
		   }

		   $$.ast = new_tree (ank_call_expr, $1.sym, $1, $7);
		   ast_child_tree ($$.ast, symbol_token ($1));
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, $6.ast);
		}
	| pseudo_column
		{
		   /* TBD: ROWID is both a builtin function and datatype
		    * keep the first occurance to avoid a conflict
		    * until overloading is implemented
		    */
		   if ( (! symbol_is_known ($1)) ||
		       (symbol_get($1) == sk_function)) {
		      /* treat SYSDATE, ROWID, etc as function calls */
		      $$.ast = symbol_call_expr (&$1, $1, $1);
		   } else {
		      $$.ast = symbol_token ($1);
		   }
		}
	| SYM_IDENTIFIER '.' pseudo_column
		{
		   /* TBD: ROWID is both a builtin function and datatype
		    * keep the first occurance to avoid a conflict
		    * until overloading is implemented
		    */
		   if ( (! symbol_is_known ($3)) ||
		       (symbol_get($3) == sk_function)) {
		      /* treat SYSDATE, ROWID, etc as function calls */
		      $$.ast = symbol_call_expr (&$3, $1, $3);
		   } else {
		      $$.ast = symbol_token ($3);
		   }

		   /* add table/alias reference */
		   if ((! $1.sym) || in_select_items) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }
		   ast_child_tree ($$.ast, symbol_reference ($1));
		}
	;

/* Pseudo columns that are keywords.
 * Others (SYSDATE, LEVEL, ROWNUM, UID) are identifiers.
 */
pseudo_column:
	Y_ROWID
	| Y_USER
	;

trigger_table:
	Y_NEW
	| Y_OLD
	;

/* symbol hacks to allow some keywords as identifiers */
first_identifier:
        SYM_IDENTIFIER
	| Y_FIRST
	| Y_NEXT
	;	
        
single_identifier:
	symbol_identifier
	| Y_FIRST
	| Y_NEXT
	;

symbol_identifier:
        SYM_IDENTIFIER
        | keyword_id
        ;

identifier:
        SYM_IDENTIFIER
        | keyword_id
	| pseudo_column
        ;

any_identifier:
	identifier
	| Y_FIRST
	| Y_NEXT
	;

/* keyword hacks to allow use of certain keywords as identifiers */
keyword_id:
        Y_TYPE
	| Y_COMMENT
        | Y_RANGE
	| Y_SEQUENCE
	| scalar_base_single
        ;

/* A keyword that is returned by the scanner as an identifer.
 *
 * This is because the keyword is either not in the keyword table
 * or that keyword is turned off in the current keyword state.
 * See keyword processing in sql_keyword.C and the table in
 * sql_keytable.C.
 *
 * Since keywords can be used as identifiers it is important
 * to keep the number of keywords limited.  Therefore if a
 * keyword can only be in a given place in the syntax and is
 * not needed for the grammar, it is better to use an identifier
 * token for it.  In general this can be done only when the
 * keyword is part of some phrase (but not the first word) or
 * a single optional word in a given place in the command.
 *
 * For example:
 * Part of a trigger definition can be the phase "FOR EACH ROW".
 * Since the word "FOR" can only be followed by "EACH ROW" inside
 * a trigger definition, this clause can be coded "FOR keyword keyword"
 * and "EACH" and "ROW" can be left out of the keyword table
 * unless they are needed for some other command.
 * 
 * TBD: check to make sure that the keyword is indeed the correct
 * text.
 */
keyword:
	SYM_IDENTIFIER
		{
		   symbol_keyword ($1);
		}
	;

atom:
	literal_item
	;


/*  Either host_variable or host_cursor_variable_name */
host_variable_symbol:
	host_variable
	| host_variable host_variable
		{
		   $$.ast = sql_call_expr ("INDICATOR", $2, $2);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $2.ast);
		}
	| host_variable Y_INDICATOR host_variable
		{
		   $$.ast = symbol_call_expr (&$2, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	;

/* HOST_PREFIX is either a colon or (Informix only) a dollar sign.
 * Convert to colon (ANSI SQL) for symbol check.
 * host_prefix_expr can be a complex C/C++ reference with pointers, etc.
 * For esql the symbol is used as a place holder as the actual reference
 * is replaced by the host AST reference.
 */
host_variable:
	HOST_PREFIX host_postfix_expr
		{
		    char* ref_name = token_range_text($1.start_token,
						      $2.end_token);
		    $2.sym = sql_symbol_lookup(ref_name);
		    if (!symbol_is_known($2)) {
			$2.sym = sql_symbol_extern(ref_name, sk_variable);
			symbol_attribute($2.sym, ATTRMASK_HOST);
		    }
		    $$.sym = $2.sym;
		    $$.ast = symbol_tree($$.sym, $2);
		    free(ref_name);

		   if (esql_mode) {
		      /* only output to host file if in block */
		      if (need_postfix) {
			 reference_var ($2);
		      }
		      if ($$.sym) {
			 $$.sym->sym_replace = 1;
		      }
		   } else {
		      ast_extend_start ($$.ast, token_loc_start ($1));
		   }
		   inside_host_variable_reference = 0;
		}
	;

host_postfix_expr:
	host_id_expr
	| host_postfix_expr '[' host_generic_tokens ']'
		{
		    set_result(&$$, $1, $4, $$.sym, $$.ast);
		}
	| host_postfix_expr '.' host_id_expr
		{
		    set_result(&$$, $1, $3, $$.sym, $$.ast);
		}
	| host_postfix_expr OP_ARROW host_id_expr
		{
		    set_result(&$$, $1, $3, $$.sym, $$.ast);
		}
	| host_postfix_expr OP_INCR
		{
		    set_result(&$$, $1, $2, $$.sym, $$.ast);
		}
	| host_postfix_expr OP_DECR
		{
		    set_result(&$$, $1, $2, $$.sym, $$.ast);
		}
	;

host_id_expr:
	SYM_HOST_VARIABLE
		{
		    inside_host_variable_reference = 1;
		}
	| SYM_IDENTIFIER
		{
		    /* This production comes into play from the
		     * preceding nonterminal -- the intent is to allow
		     * "a.b::c" or "a->b::c".  It should not be used
		     * following HOST_PREFIX.
		     */
		}
	| host_id_expr OP_QUAL SYM_IDENTIFIER
		{
		    set_result(&$$, $1, $3, $$.sym, $$.ast);
		}
	;

host_generic_tokens:
	GENERIC_TOKEN
	| host_generic_tokens GENERIC_TOKEN
		{
		    set_result(&$$, $1, $2, NULL, NULL);
		}
	;

host_variable_symbol_list:
	host_variable_symbol
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| host_variable_symbol_list ',' host_variable_symbol
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

/*  host_cursor_variable_name has no indicator_name option */
host_cursor_variable:
	host_variable
	;

literal_item:
	NUMERIC_LITERAL
		{
		   $$.ast = literal_tree ($1);
		}
	| STRING_LITERAL
		{
		   $$.ast = string_literal_tree ($1);
		}
	;

literal_or_host_item:
	host_variable
	| literal_item
	;

literal_or_host_item_opt:
	/* empty */	{ set_empty (&$$); }
	| literal_or_host_item
	;

/**************** Fetch ******************/

fetch_statement: 
	fetch_keyword cursor_handle Y_INTO into_destination ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

into_destination:
	expression_list
	;

/**************** Functions ******************/

function_clause:
	SYM_IDENTIFIER
		{
		   $1.sym = symbol_set ($1, sk_function);
		   sql_push_scope ($1.sym->sym_text, $1.sym);
		   parameter_count = 0;
		   set_result (&$$, $1, $1, $1.sym, 0);
		}
        ;

function_specification:
        Y_FUNCTION function_clause
	parameters_opt Y_RETURN return_type ';'
		{
		   ast_tree * at;
		   sqltype tdecl;

		   if (is_empty ($3)) {
		      tdecl = $2;
		   } else {
		      tdecl = $3;
		   }

		   $$.ast = new_tree (ank_list_decl,
		      $2.sym, $1, $6);

		   /* return_type */
		   ast_child_tree ($$.ast, $5.ast);

		   at = new_tree (ank_function_decl, $2.sym, $1, tdecl);
		   ast_child_tree (at, symbol_token ($2));
		   /* parameters_opt */
		   ast_child_tree (at, $3.ast);
		   ast_child_tree ($$.ast, at);

		   sql_pop_scope ();
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;


function_body:
	Y_FUNCTION function_clause
	parameters_opt
	Y_RETURN return_type as_is
	object_declaration_list_opt
	subprogram_declaration_list_opt
	Y_BEGIN
	statement_list
	exception_clause_opt
	Y_END
		{
		   sql_pop_scope ();
		}
        function_name_opt ';'
		{
		   ast_tree * at_decl;
		   ast_tree * at_body;
		   sqltype tdecl;

		   if (is_empty ($3)) {
		      tdecl = $2;
		   } else {
		      tdecl = $3;
		   }

		   $$.ast = new_tree (ank_function_def,
		      $2.sym, $1, $15);

		   /* return_type */
		   ast_child_tree ($$.ast, $5.ast);

		   /*
		    * declaration
		    */
		   at_decl = new_tree (ank_function_decl,
		      $2.sym, $1, tdecl);
		   ast_child_tree (at_decl, symbol_token ($2));
		   /* parameters_opt */
		   ast_child_tree (at_decl, $3.ast);
		   ast_decldefined (at_decl);
		   ast_child_tree ($$.ast, at_decl);

		   /*
		    * body
		    */
		   at_body = new_tree (ank_ast_block,
		      $2.sym, $6, $15);

		   /* object_declaration_list_opt */
		   ast_child_tree (at_body, $7.ast);

		   /* subprogram_declaration_list_opt */
		   ast_child_tree (at_body, $8.ast);

		   /* statement_list */
		   ast_child_tree (at_body, $10.ast);

		   /* exception_clause_opt */
		   ast_child_tree (at_body, $11.ast);

		   ast_child_tree ($$.ast, at_body);

		   /* function_name_opt */
		   ast_child_tree ($$.ast, $14.ast);

		   ast_defined ($$.ast);
		   set_result (&$$, $1, $15, 0, $$.ast);
		}
	;

parameters_opt:
	/* empty */	{ set_empty (&$$); }
	| '(' parameter_declaration_list ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	| '(' error ')'
		{
		   error_result (&$$, $1, $3);
		}
	;

parameter_declaration_list:
	parameter_declaration
		{
		   $$.ast = new_tree (ank_list,
		      0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| parameter_declaration_list ',' parameter_declaration
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	| parameter_declaration_list ',' error
		{
		   ast_child_tree ($1.ast, error_node ($2, $3));
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	|  error ',' parameter_declaration
		{
		   $$.ast = new_tree (ank_list,
		      0, $2, $3);
		   ast_child_tree ($$.ast, error_node ($2, $2));
		   ast_child_tree ($$.ast, $3.ast);
		}
	;

parameter_declaration:
	any_identifier in_out_opt variable_type default_value_opt
		{
		   sqltype tlast;

		   if (!is_empty ($4)) {
		      tlast = $4;
		   } else {
		      tlast = $3;
		   }

		   $$.sym = symbol_set ($1, sk_variable);

		   $$.ast = new_tree (ank_parm_decl, $$.sym, $1, tlast);
		   ast_child_tree ($$.ast, $3.ast);
		   ast_child_tree ($$.ast, symbol_tree ($$.sym, $1));
		   ast_child_tree ($$.ast, $4.ast);

		   ast_relation ($$.ast, rk_arg);
		   ast_relation_arg ($$.ast, ++parameter_count);
		   if ( ! is_empty ($2) ) {
		      ast_relation_attr ($$.ast, parameter_attribute);
		   }

		   if ( ! in_specification ) {
		      ast_defined ($$.ast);
		      ast_decldefined ($$.ast);
		   }
		}
	;

as_is:
        Y_IS
        | Y_AS
        ;

in_out_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_IN
		{
		   parameter_attribute = RELMASK_INPUT;
		}
	| Y_OUT
		{
		   parameter_attribute = RELMASK_OUTPUT;
		}
	| Y_IN Y_OUT
		{
		   parameter_attribute = RELMASK_INPUT | RELMASK_OUTPUT;
		}
	;

return_type:
	data_type
		{
		   ast_relation ($$.ast, rk_type);
		   ast_relation_attr ($$.ast, RELMASK_RETURN);
		}
	;

object_declaration_list_opt:
	/* empty */	{ set_empty (&$$); }
	| object_declaration_list
	;

function_name_opt:
	/* empty */	{ set_empty (&$$); }
	| SYM_IDENTIFIER
		{
		   if ($1.sym) {
		      if (symbol_check ($1, sk_function)) {
			 $$.ast = symbol_reference ($1);
		      }
		   }
		}
	;

/**************** GoTo ******************/

goto_statement:
	Y_GOTO label_name ';'
		{
		   $$.ast = new_tree (ank_goto_stmt, 0, $1, $3);
		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
	;


/**************** If ******************/

if_statement: 
	Y_IF boolean_expression Y_THEN statement_list
	elsif_list_opt else_opt Y_END Y_IF ';'
		{
		   ast_tree * at_then;
		   $$.ast = new_tree (ank_if_stmt, 0, $1, $9);
		   ast_child_tree ($$.ast, $2.ast);

		   at_then = new_tree (ank_ast_block, 0, $4, $4);
		   ast_child_tree (at_then, $4.ast);

		   ast_child_tree ($$.ast, at_then);

		   /* TBD: make this into third branch */
		   ast_child_tree ($$.ast, $5.ast);
		   ast_child_tree ($$.ast, $6.ast);
		   set_result (&$$, $1, $9, 0, $$.ast);
		}
	| Y_IF error Y_THEN statement_list
	elsif_list_opt else_opt Y_END Y_IF ';'
		{
		   ast_tree * at_then;
		   $$.ast = new_tree (ank_if_stmt, 0, $1, $9);
		   ast_child_tree ($$.ast, error_node ($1, $2));

		   at_then = new_tree (ank_ast_block, 0, $4, $4);
		   ast_child_tree (at_then, $4.ast);
		   ast_child_tree ($$.ast, at_then);

		   /* TBD: make this into third branch */
		   ast_child_tree ($$.ast, $5.ast);
		   ast_child_tree ($$.ast, $6.ast);
		   set_result (&$$, $1, $9, 0, $$.ast);
		}
	;

elsif_list_opt:
	/* empty */	{ set_empty (&$$); }
	| elsif_list
	;

elsif_list:
	elsif_clause
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| elsif_list elsif_clause
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

elsif_clause:
	Y_ELSIF boolean_expression Y_THEN statement_list   
		{
		   $$.ast = new_tree (ank_elsif_clause, 0, $1, $4);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	;


else_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_ELSE statement_list
		{
		   $$.ast = new_tree (ank_ast_block, 0, $2, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	;

/**************** Insert ******************/

insert_statement: 
	insert_keyword Y_INTO tableref_or_subquery
	'(' column_list ')' values_clause sql_eos
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $7, $8);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $7.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	| insert_keyword Y_INTO tableref_or_subquery
	values_clause sql_eos
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	| insert_keyword Y_INTO tableref_or_subquery
	'(' column_list ')' select_query sql_eos
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $7, $8);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $7.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	| insert_keyword Y_INTO tableref_or_subquery
	select_query sql_eos
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

insert_keyword:
	Y_INSERT
		{
		   sql_keywords_only ();
		   esql_prefix_check ();
		   sql_push_scope (0, 0);
		}
	;

column_list_opt:
	/* empty */	{ set_empty (&$$); }
	| '(' column_list ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

column_list:
	column_name
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| column_list ',' column_name
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

column_name:
	identifier
		{
		   /* Check for tablename */
		   if (sql_scope_table_name ()) {
		      sprintf (txtbuf, "%s.%s", 
		         sql_scope_table_name (), token_text ($1));

		      $1.sym = sql_symbol_extern (txtbuf, sk_column);

		   } else if ( ! $1.sym ) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   $$.sym = $1.sym;
		   $$.ast = symbol_token ($1);
		}
	;

values_clause:
	Y_VALUES
		{
		   /* Table name only needed for INSERT column list.
		    * Clear it here so that any symbols referenced in
		    * the values clause are not taken as column names.
		    */
		   sql_scope_clear_table_name();
		}
	'(' expression_list ')'
		{
		   set_result (&$$, $1, $5, 0, $4.ast); 
		}
	;

/* expression list with at least two expressions */
expression_multilist:
	'(' expression ',' expression_list ')'
		{
		   $$.ast = new_tree (ank_ast_block, 0, $1, $5);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;


expression_list:
	expression
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| expression_list ',' expression
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

boolean_literal:
	Y_TRUE
		{
		   $$.ast = literal_tree ($1);
		}
	| Y_FALSE
		{
		   $$.ast = literal_tree ($1);
		}
	| Y_NULL
		{
		   $$.ast = literal_tree ($1);
		}
	;


/**************** Lock Table ******************/

lock_table_statement: 
	Y_LOCK Y_TABLE other_table_reference_list
	Y_IN lock_mode_list Y_MODE nowait_opt ';'
		{
		   sqltype tlast = $7;
		   if (is_empty($7)) {
		      tlast = $6;
		   }

		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $8);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $7.ast);
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

other_table_reference_list:
	other_table_reference
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| other_table_reference_list ',' other_table_reference
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

lock_mode_list:
	lock_mode
	| lock_mode_list lock_mode
	;

/* keyword can be UPDATE, SHARE, ROW, EXCLUSIVE */
lock_mode:
	Y_UPDATE
	| keyword
	;

nowait_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_NOWAIT
	;

/**************** Loops ******************/

/* basic, WHILE, and FOR loops */
loop_statement:
        loop_stmt label_opt ';'
		{
		   $$.ast = $1.ast;
		   ast_child_tree ($$.ast, symbol_reference ($2));
		   ast_extend_end ($$.ast, token_loc_end ($3));
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
        ;

loop_stmt:
	loop_body
		{
		   $$.ast = new_tree (ank_loop_stmt, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| Y_WHILE boolean_expression loop_body
		{
		   $$.ast = new_tree (ank_loop_stmt, 0, $1, $3);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	 | Y_FOR SYM_IDENTIFIER Y_IN for_expression
		{
		   sql_push_scope (0, 0);
		   if (for_loop_cursor) {
		      $2.sym = symbol_set ($2, sk_record);
		   } else {
		      $2.sym = symbol_set ($2, sk_variable);
		   }
		}
	   loop_body
		{
		   $2.ast = reference_type_tree (ank_var_decl, $2.sym,
						  $2, $4,
						  REFMASK_AST);
		   ast_child_tree ($2.ast, symbol_token ($2));
		   ast_child_tree ($2.ast, $4.ast);
		   ast_defined ($2.ast);
		   ast_decldefined ($2.ast);

		   if ( ! for_loop_cursor ) {
		      symbol_attribute ($2.sym, ATTRMASK_LOOPIDX);
		   }

		   $$.ast = new_tree (ank_loop_stmt, 0, $1, $6);
				  		   
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $6.ast);

		   sql_pop_scope ();
		}
	;

loop_body:
	Y_LOOP statement_list Y_END Y_LOOP
		{
		   $$.ast = new_tree (ank_ast_block, 0, $1, $4);
		   ast_child_tree ($$.ast, $2.ast);
		}
	;


for_expression:
	lower_bound OP_RANGE upper_bound
		{
		   $$.ast = symbol_call_expr (&$2, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		   for_loop_cursor = 0;
		}
	| Y_REVERSE lower_bound OP_RANGE upper_bound
		{
		   $$.ast = symbol_call_expr (&$3, $1, $4);
		   ast_child_tree ($$.ast, $4.ast);
		   ast_child_tree ($$.ast, $2.ast);
		   for_loop_cursor = 0;
		}
	| SYM_IDENTIFIER cursor_parameter_name_list_opt
		{
		   /* sk_cursor */
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_set ($1, sk_cursor);
		   }

		   $$.ast = new_tree (ank_call_expr, $$.sym, $1, $2);
		   ast_child_tree ($$.ast, symbol_token($1));
		   ast_child_tree ($$.ast, $2.ast);

		   for_loop_cursor = 1;
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER cursor_parameter_name_list_opt
		{
		   if ( ! symbol_is_known ($1)) {
		      $1.sym = symbol_extern ($1, sk_package);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   $$.sym = sql_symbol_global_lookup (txtbuf);
		   if ( ! symbol_is_known ($$) ) {
		      $$.sym = sql_symbol_extern (txtbuf, sk_cursor);
		   }

		   $$.ast = new_tree (ank_call_expr, $$.sym, $1, $4);
		   ast_child_tree ($$.ast, symbol_tree_ast ($$.sym, $1, $3));
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast, symbol_reference_tree ($$.sym, $3));
		   ast_child_tree ($$.ast, $4.ast);

		   for_loop_cursor = 1;
		}
	| '(' select_query ')'
		{
		   for_loop_cursor = 1;
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;


lower_bound:
	value_expression
	;

upper_bound:
	value_expression
	;

cursor_parameter_name_list_opt:
	/* empty */	{ set_empty (&$$); }
	| '(' expression_list ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

/**************** Null ******************/

null_statement: 
	Y_NULL ';'
		{
		   $$.ast = new_tree (ank_expr_stmt, 0, $1, $2);
		   ast_child_tree ($$.ast, literal_tree ($1));
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	;

/**************** Open ******************/

open_statement: 
	open_keyword cursor_handle cursor_parameter_name_list_opt ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $3, $4);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

open_for_statement: 
	open_keyword cursor_handle Y_FOR select_query ';'
		{
		   $$.ast = symbol_call_stmt (&$1, $1, $4, $5);
		   ast_child_tree ($1.ast, $2.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;


/**************** Packages ******************/

package_specification:
	Y_PACKAGE package_clause
		{
		   in_specification = 1;
		   sql_push_scope (package_name, 0);
		}
	as_is
	object_or_spec_declaration_list_opt
	Y_END
		{
		   sql_pop_scope ();
		}
        package_name_opt ';'
		{
		   ast_tree * at_block;
		   sqltype tblock = $5;
		   if (is_empty($5)) {
		      tblock = $4;
		   }

		   $$.ast = new_tree (ank_type_decl, $2.sym, $1, $9);

		   ast_child_tree ($$.ast, symbol_token ($2));

		   at_block = new_tree (ank_ast_block, 0, tblock, tblock);
		   /* object_or_spec_declaration_list_opt */
		   ast_child_tree (at_block, $5.ast);
		   ast_child_tree ($$.ast, at_block);

		   /* Indicate as package members */
		   ast_relation ($5.ast, rk_context);
		   ast_relation_attr ($5.ast, RELMASK_PUBLIC);

		   /* package_name_opt */
		   ast_child_tree ($$.ast, $8.ast);
		   
		   ast_decldefined ($$.ast);
		   ast_defined ($$.ast);
		   package_name = 0;
		   in_specification = 0;
		   sql_specfile_output ($2.sym, $1, $9);
		   set_result (&$$, $1, $9, 0, $$.ast);
		}
	;

package_body:
	Y_PACKAGE Y_BODY package_clause as_is
		{
		   sql_push_scope (package_name, 0);
		}
	package_body_declaration_list_opt
	begin_opt
	Y_END
		{
		   sql_pop_scope ();
		}
        package_name_opt ';'
		{
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_package_body, $3.sym, $1, $11);

		   ast_child_tree ($$.ast, symbol_token ($3));

		   at_block = new_tree (ank_ast_block, $3.sym, $4, $11);

		   /* body_declaration_list_opt */
		   ast_child_tree (at_block, $6.ast);

		   /* Indicate as package members */
		   /* TBD: ast_relation ($6.ast, rk_context); */
		   /* TBD: flag some private.  Contains mix of public
		    * and private, but don't know which since private
		    * members are those in this body but not in the
		    * specification (implicitly public).
		    */

		   /* begin_opt */
		   ast_child_tree (at_block, $7.ast);

		   /* Indicate as package members */
		   /* TBD: ast_relation ($7.ast, rk_context); */

		   ast_child_tree ($$.ast, at_block);

		   /* package_name_opt */
		   ast_child_tree ($$.ast, $10.ast);

		   /* ast_defined ($$.ast); */

		   ast_resolve_package (package_name, at_block);

		   package_name = 0;
		   set_result (&$$, $1, $11, 0, $$.ast);
		}
	;

object_or_spec_declaration:
	object_declaration
	| spec_declaration
	;

object_or_spec_declaration_list_opt:
	/* empty */	{ set_empty (&$$); }
	| object_or_spec_declaration_list
	;

object_or_spec_declaration_list:
	object_or_spec_declaration
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| object_or_spec_declaration_list object_or_spec_declaration
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

package_clause:
	SYM_IDENTIFIER
		{
		   $1.sym = symbol_set ($1, sk_package);
		   package_name = $1.sym->sym_text;
		   set_result (&$$, $1, $1, $1.sym, 0);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   /* TBD: make ast */
		   $3.sym = symbol_set ($3, sk_package);
		   package_name = $3.sym->sym_text;
		   set_result (&$$, $1, $3, $3.sym, 0);
		}
	;

package_name_opt:
	/* empty */	{ set_empty (&$$); }
	| SYM_IDENTIFIER
		{
		   if ($1.sym) {
		      if (symbol_check ($1, sk_package)) {
			 $$.ast = symbol_reference ($1);
		      }
		   }
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   if ($3.sym) {
		      if (symbol_check ($3, sk_package)) {
			 $$.ast = symbol_reference ($3);
		      }
		   }
		}
	;

package_body_declaration_list_opt:
	/* empty */	{ set_empty (&$$); }
	| package_body_declaration_list
	;


package_body_declaration_list:
	package_body_declaration
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| package_body_declaration_list package_body_declaration
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

package_body_declaration:
        spec_declaration
	| object_declaration
	| body_declaration
	;

begin_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_BEGIN statement_list
		{
		   $$.ast = $2.ast;
		}
	;

spec_declaration:
	cursor_specification
	| function_specification
	| procedure_specification
	| pragma_restrict_references
	;

body_declaration:
	cursor_body
	| function_body
	| procedure_body
	;

pragma_restrict_references:
	Y_PRAGMA Y_RESTRICT_REFERENCES
	'(' SYM_IDENTIFIER ',' purity_level_list ')' ';'
		{
		   if ( ! $4.sym ) {
		      $4.sym = symbol_extern ($4, sk_externref);
		   }
		   $$.ast = symbol_call_stmt (&$2, $1, $7, $8);
		   ast_child_tree ($1.ast, symbol_token ($4));
		   ast_child_tree ($1.ast, $6.ast);
		   set_result (&$$, $1, $8, 0, $$.ast);
		}
	;

purity_level_list:
	purity_level
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| purity_level_list ',' purity_level
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

/*
 * purity_level values are just copied as literal text.
 * If needed, these can be added as real ast node kinds.
 */
purity_level:
	Y_WNDS
	| Y_WNPS
	| Y_RNDS
	| Y_RNPS
	;

/**************** PL/SQL Tables ******************/

plsql_table_delete_statement:
	SYM_IDENTIFIER '.' Y_DELETE index_list_opt ';'
		{
		   sqltype tlast = $4;
		   if (is_empty ($4)) {
		      tlast = $3;
		   }

		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_plsql_table);
		   }
		   $$.ast = symbol_call_stmt (&$3, $1, tlast, $5);
		   ast_child_tree ($1.ast, symbol_token ($1));
		   ast_child_tree ($1.ast, $4.ast);
		   set_result (&$$, $1, $5, 0, $$.ast);
		}
	;

index_list_opt:
	/* empty */	{ set_empty (&$$); }
	| '(' expression_list ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

table_type_definition:
	type_clause SYM_IDENTIFIER Y_IS Y_TABLE
		{
		   $2.sym = symbol_set ($2, sk_table_type);
		}
	Y_OF table_data_type
	not_null_opt Y_INDEX Y_BY Y_BINARY_INTEGER ';'
		{
		   $$.ast = new_tree (ank_type_decl,
			$2.sym, $1, $12);
		   ast_child_tree ($$.ast, symbol_token ($2));
		   ast_child_tree ($$.ast, $7.ast);
		   ast_child_tree ($$.ast, $8.ast);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   set_result (&$$, $1, $12, 0, $$.ast);
		}
	;

table_data_type:
	data_type
		{
		   ast_relation ($$.ast, rk_type);
		}
	;

subtype_definition:
	Y_SUBTYPE SYM_IDENTIFIER Y_IS data_type subtype_range_opt ';'
		{
		   $2.sym = symbol_type ($2);
		   $$.sym = $2.sym;

		   $$.ast = new_tree (ank_type_decl,
			$2.sym, $1, $6);
		   ast_child_tree ($$.ast, symbol_token ($2));
		   ast_child_tree ($$.ast, $4.ast);
		   ast_child_tree ($$.ast, $5.ast);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

subtype_range_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_RANGE lower_bound OP_RANGE upper_bound
		{
		   $$.ast = symbol_call_expr (&$1, $1, $4);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	;

/**************** Procedures ******************/

procedure_clause:
	SYM_IDENTIFIER
		{
		   $1.sym = symbol_set ($1, sk_procedure);
		   sql_push_scope ($1.sym->sym_text, $1.sym);
		   parameter_count = 0;
		   set_result (&$$, $1, $1, $1.sym, 0);
		}
	;

procedure_specification:
	Y_PROCEDURE procedure_clause
	parameters_opt ';'
		{
		   ast_tree * at_decl;
		   sqltype tdecl;

		   if (is_empty ($3)) {
		      tdecl = $2;
		   } else {
		      tdecl = $3;
		   }

		   $$.ast = new_tree (ank_list_decl, $2.sym, $1, $4);

		   /* empty (implies void) return type */
		   ast_child_tree ($$.ast, empty_tree (ank_ast_declspec));

		   at_decl = new_tree (ank_function_decl, $2.sym, $1, tdecl);

		   ast_child_tree (at_decl, symbol_token ($2));

		   /* parameter_opt */
		   ast_child_tree (at_decl, $3.ast);

		   ast_child_tree ($$.ast, at_decl);

		   sql_pop_scope ();
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

procedure_body:
	Y_PROCEDURE procedure_clause
	parameters_opt as_is
        object_declaration_list_opt
        subprogram_declaration_list_opt
        Y_BEGIN statement_list
        exception_clause_opt
	Y_END
		{
		   sql_pop_scope ();
		}
        procedure_name_opt ';'
		{
		   ast_tree * at_decl;
		   ast_tree * at_body;
		   sqltype tdecl;

		   if (is_empty ($3)) {
		      tdecl = $2;
		   } else {
		      tdecl = $3;
		   }

		   $$.ast = new_tree (ank_function_def, $2.sym, $1, $13);

		   /* empty (implies void) return type */
		   ast_child_tree ($$.ast, empty_tree (ank_ast_declspec));

		   /*
		    * declaration
		    */
		   at_decl = new_tree (ank_function_decl, $2.sym, $1, tdecl);

		   ast_child_tree (at_decl, symbol_token ($2));

		   /* parameters_opt */
		   ast_child_tree (at_decl, $3.ast);

		   /* flag as declaration definition */
		   ast_decldefined (at_decl);

		   /* add decl to def */
		   ast_child_tree ($$.ast, at_decl);

		   /*
		    * body
		    */
		   at_body = new_tree (ank_ast_block, $2.sym, $4, $13);

		   /* object_declaration_list_opt */
		   ast_child_tree (at_body, $5.ast);

		   /* subprogram_declaration_list_opt */
		   ast_child_tree (at_body, $6.ast);

		   /* statement_list */
		   ast_child_tree (at_body, $8.ast);

		   /* exception_clause_opt */
		   ast_child_tree (at_body, $9.ast);

		   /* add body to def */
		   ast_child_tree ($$.ast, at_body);

		   /* procedure_name_opt */
		   ast_child_tree ($$.ast, $12.ast);

		   ast_defined ($$.ast);
		   set_result (&$$, $1, $13, 0, $$.ast);
		}
	;

procedure_name_opt:
	/* empty */	{ set_empty (&$$); }
	| SYM_IDENTIFIER
		{
		   if ($1.sym) {
		      if (symbol_check ($1, sk_procedure)) {
			 $$.ast = symbol_reference ($1);
		      }
		   }
		}
	;

/**************** Raise ******************/

raise_statement:
	Y_RAISE exception_name_opt ';'
		{
		   sqltype tlast = $2;
		   if (is_empty ($2)) {
		      tlast = $1;
		   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $3);
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
	;

exception_name_opt:
	/* empty */	{ set_empty (&$$); }
	| exception_name
	;

/**************** Records and Fields ******************/

record_type_definition:
	type_clause SYM_IDENTIFIER Y_IS Y_RECORD
		{
		   /* no longer need TYPE keywords (REF, REFERENCE, etc)
		    * clearing keyword command will allow them as field names
		    */
		   sql_keyword_command (SQLKEYC_NONE);

		   $2.sym = symbol_set ($2, sk_record_type);
		   record_sym = $2.sym;
		   sql_push_scope (token_text ($2), 0);
		}
	'(' field_declaration_list ')' ';'
		{
		   ast_tree * at_block;

		   $$.ast = new_tree (ank_type_decl,
			$2.sym, $1, $9);
		   ast_child_tree ($$.ast, symbol_token ($2));

		   at_block = new_tree (ank_ast_block, 0, $6, $8);
		   ast_child_tree (at_block, $7.ast);
		   ast_child_tree ($$.ast, at_block);

		   ast_defined ($$.ast);
		   ast_decldefined ($$.ast);
		   record_sym = 0;
		   sql_pop_scope ();
		   set_result (&$$, $1, $9, 0, $$.ast);
		}
	;

field_declaration_list:
	field_declaration
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| field_declaration_list ',' field_declaration
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

field_declaration:
	identifier field_type default_value_opt
		{
		   sqltype tlast;

		   if (!is_empty ($3)) {
		      tlast = $3;
		   } else {
		      tlast = $2;
		   }

		   if (record_sym) {
		      if (record_sym->full_sym) {
			 /* add PACKAGE.RECORD.FIELD to global scope */
			 sprintf (txtbuf, "%s.%s", 
				  record_sym->full_sym->sym_text,
				  token_text ($1));
			 $$.sym = sql_symbol_extern (txtbuf, sk_field);

			 /*
			  * add RECORD.FIELD to previous scope (record scope)
			  */
			 sprintf (txtbuf, "%s.%s", 
				  record_sym->sym_text,
				  token_text ($1));

			 sql_symbol_prev_local ($$.sym, txtbuf);
		      } else {
			 /*
			  * add RECORD.FIELD to previous scope (record scope)
			  */
			 sprintf (txtbuf, "%s.%s", 
				  record_sym->sym_text,
				  token_text ($1));

			 $$.sym = sql_symbol_add_prev (txtbuf, sk_field);
		      }
		      
		      
		      /* Add local copy with field name only.
		       * This is only usable within this field
		       * declaration list to specify another
		       * field's type.
		       * 
		       * For example:
		       *    TYPE rec IS RECORD (f1 CHAR, f2 f1%TYPE);
		       *
		       * The datatype of f2 is the same as f1.
		       * Outside this statement, a field name cannot
		       * appear without a record qualifier (e.g. rec.f1).
		       */
		      sql_symbol_local ($$.sym, token_text ($1));
		      
		      $$.ast = new_tree (ank_field_decl, $$.sym, $1, tlast);
		      ast_child_tree ($$.ast, $2.ast);
		      ast_child_tree ($$.ast, symbol_tree ($$.sym, $1));
		      ast_child_tree ($$.ast, $3.ast);
		      
		      ast_relation ($$.ast, rk_context);
		      ast_defined ($$.ast);
		      ast_decldefined ($$.ast);
		   } else {
		      iff_error ("missing record name");
		   }
		}
	;

field_type:
	data_type
		{
		   ast_relation ($$.ast, rk_type);
		}
	;

/**************** Return ******************/

return_statement: 
	Y_RETURN expression_opt ';'
		{
		   $$.ast = new_tree (ank_return_stmt, 0, $1, $3);
		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);
		}
	;

expression_opt:
	/* empty */	{ set_empty (&$$); }
	| expression
	;

/**************** Rollback and Savepoint ******************/

rollback_statement: 
	Y_ROLLBACK
		{
		   sql_keyword_command (SQLKEYC_COMMIT);
		   esql_prefix_check ();
		}
	work_opt to_savepoint_opt commit_option_list_opt sql_eos
		{
		   sqltype tlast;
		   if (!is_empty ($5)) {
		      tlast = $5;
		   } else if (!is_empty ($4)) {
		      tlast = $4;
		   } else if (!is_empty ($3)) {
		      tlast = $3;
		   } else {
		      tlast = $1;
		   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $6);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   set_result (&$$, $1, $6, 0, $$.ast);
		}
	;

to_savepoint_opt:
	/* empty */	{ set_empty (&$$); }
	| to_savepoint
	;

to_savepoint:
	Y_TO savepoint_opt SYM_IDENTIFIER 
		{
		   if ((! $3.sym) || (symbol_get ($3) != sk_savepoint)) {
		      $3.sym = symbol_extern ($3, sk_savepoint);
		   }
		      
		   $$.ast = symbol_token ($3);
		}
	;

savepoint_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_SAVEPOINT
	;

savepoint_statement: 
	Y_SAVEPOINT
		{
		   esql_prefix_check ();
		}
	SYM_IDENTIFIER ';'
		{
		   $2.sym = symbol_extern ($3, sk_savepoint);

		   $$.ast = symbol_call_stmt (&$1, $1, $3, $4);
		   ast_child_tree ($1.ast, $3.ast);

		   ast_defined ($$.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);
		}
	;

/**************** Select ******************/

select_statement:
	select_compound sql_eos
		{
		   $$.ast = new_tree (ank_expr_stmt, 0, $1, $2);
		   ast_child_tree ($$.ast, $1.ast);
		   set_result (&$$, $1, $2, 0, $$.ast);
		}
	;

select_query:
	select_compound
	;

select_compound:
	select_expression
	select_order_update_list_opt
	nowait_opt
		{
		   $$.ast = $1.ast;
		   if (!is_empty ($2)) {
		      ast_child_tree ($$.ast, $2.ast);
		      ast_extend_end ($$.ast, token_loc_end ($2));
		      
		      /* resolve column names in ORDER BY and UPDATE clauses */
		      if (table_count > 1) {
			 ast_resolve_columns (table_list, $2.ast);
		      }
		   }
		   if (!is_empty ($3)) {
		      ast_child_tree ($$.ast, $3.ast);
		      ast_extend_end ($$.ast, token_loc_end ($3));
		   }

		   sql_pop_scope ();
		   sql_pop_keyword_mode ();
		}
	;
	
select_expression:
	select_simple
	| '(' select_expression ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	| select_expression Y_UNION all_opt select_expression
		{
		   $$.ast = $1.ast;
		   $2.ast = symbol_call_expr (&$2, $2, $4);
		   ast_child_tree ($2.ast, $4.ast);

		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $4, 0, $$.ast);

		   sql_pop_scope ();
		   sql_pop_keyword_mode ();
		}
	| select_expression Y_INTERSECT select_expression
		{
		   $$.ast = $1.ast;
		   $2.ast = symbol_call_expr (&$2, $2, $3);
		   ast_child_tree ($2.ast, $3.ast);

		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);

		   sql_pop_scope ();
		   sql_pop_keyword_mode ();
		}
	| select_expression Y_MINUS select_expression
		{
		   $$.ast = $1.ast;
		   $2.ast = symbol_call_expr (&$2, $2, $3);
		   ast_child_tree ($2.ast, $3.ast);

		   ast_child_tree ($$.ast, $2.ast);
		   set_result (&$$, $1, $3, 0, $$.ast);

		   sql_pop_scope ();
		   sql_pop_keyword_mode ();
		}
	;

select_simple:
	select_keyword distinct_or_all_opt select_item_clause
	into_opt
	from_list
	where_clause_opt 
	select_condition_list_opt
		{
		   sqltype tlast;
		   if (!is_empty ($7)) {
		      tlast = $7;
		   } else if (!is_empty ($6)) {
		      tlast = $6;
		   } else {
		      tlast = $5;
		   }
		   $$.ast = symbol_call_expr (&$1, $1, tlast);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $4.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $6.ast);
		   ast_child_tree ($1.ast, $7.ast);

		   /* Resolve column names in select list.
		    * These (select items) come before the table names
		    * and thus can only be resolved after the table names are
		    * known in the FROM list.
		    */
		   ast_resolve_columns ($5.ast, $3.ast);
		   
		   /* Column names in the where and condition clauses
		    * only need to be resolved if there is more than
		    * one table name because they occur after the FROM
		    * list which sets the table_name string.
		    */
		   if (table_count > 1) {
		      if (!is_empty ($6)) {
			 ast_resolve_columns ($5.ast, $6.ast);
		      }
		      if (!is_empty ($7)) {
			 ast_resolve_columns ($5.ast, $7.ast);
		      }
		   }
		}
	;
	
select_keyword:
	Y_SELECT
		{
		   sql_keywords_only ();
		   esql_prefix_check ();
		   sql_push_scope (0, 0);
		   in_select_items = 1;
		}
	;

into_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_INTO into_destination
		{
		   set_result (&$$, $1, $2, 0, $2.ast);
		}
	;

from_list:
	Y_FROM tableref_entry_list
		{
		   $$.ast = new_tree (ank_ast_block, 0, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);

		   /* Reset column_alias symbols as active
		    * so that they will now be available in
		    * any following select clauses (such as the
		    * where clause).   See comment at column_alias
		    * for more details.
		    */
		   sym_inactive_clear ();
		   
		   in_select_items = 0;

		   /* set list of tables for resolving column names */
		   table_list = $2.ast;
		}
	;

distinct_or_all_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_DISTINCT
	| Y_UNIQUE	/* same as distinct */
	| Y_ALL
	; 

all_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_ALL
	; 

select_item_clause:
	select_item_list
	;

select_item_list:
	select_item
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| select_item_list ',' select_item
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

select_item:
	expression as_column_alias_opt
		{
		   if ( ! is_empty ($2) ) {
		      $$.ast = new_tree (ank_field_decl, $2.sym, $1, $2);
		      ast_child_tree ($$.ast, $2.ast);
		      ast_child_tree ($$.ast, $1.ast);

		      ast_defined ($$.ast);
		      ast_decldefined ($$.ast);
		      set_result (&$$, $1, $2, $2.sym, $$.ast);
		   } else {
		      set_result (&$$, $1, $1, 0, $1.ast);
		   }
		}
	| select_element
	;

select_element:
	'*'
	| SYM_IDENTIFIER '.' '*'
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_table);
		   } else {
		      switch (symbol_get ($1)) {
			 case sk_externref:
			    /* now we know it is a table */
			    $1.sym = symbol_extern ($1, sk_table);
			    break;
			 case sk_table:
			 case sk_alias:
			    break;
			 default:
			    iff_error_token (get_token ($1.start_token),
			       "expecting table");
			    break;
		      }
		   }

		   $$.sym = $1.sym;

		   $$.ast = symbol_token ($1);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER '.' '*'
		{
		   $1.sym = symbol_extern ($1, sk_schema); /* USER */
		   $3.sym = symbol_extern ($3, sk_table);

		   $$.ast = symbol_token ($3);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		}
	;

/* TBD: Is this still used?  See scanner handling of dot separated tokens. */
sequence_value:
	Y_CURRVAL
		{
		   $$.ast = symbol_call_expr (&$1, $1, $1);
		}
	| Y_NEXTVAL
		{
		   $$.ast = symbol_call_expr (&$1, $1, $1);
		}
	;

as_column_alias_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_AS column_alias
		{
		   set_result (&$$, $1, $2, $2.sym, $2.ast);
		}
	|  column_alias
	;

column_alias:
	SYM_IDENTIFIER
		{
		   /* column_alias (for a select item) */
		   $$.sym = symbol_set ($1, sk_column_alias);
		   $$.ast = symbol_tree ($$.sym, $1);

		   /*
		    * The select item list is where column_aliases are
		    * defined.  However, they cannot be used until after
		    * the select item list.  Flag as inactive so that
		    * they are not available within the select item list.
		    * After the select item list (in the grammar above)
		    * is defined the inactive column_aliases are made
		    * active.
		    *
		    * For example:
		    *
		    * SELECT UPPER(first_name) AS first_name,
		    *        (first_name || ' ' || last_name) AS full_name
		    * FROM employee WHERE first_name = 'JOE';
		    *
		    * The "first_name" which is used in the concatenation
		    * with "last_name" (to make the column_alias "full_name")
		    * is the column "employee.first_name" (as is the one
		    * passed to UPPER()).  However the "first_name" in the
		    * WHERE clause is the column_alias and not the column.
		    */
		   sym_inactive ($$.sym);
		}
	;

tableref_entry_list:
	tableref_entry
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);

		   /* Keep count of tables in the list
		    * Used for resolving column names.
		    */
		   table_count = 1;
		}
	| tableref_entry_list ',' tableref_entry
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		   ++table_count;
		}
	;

select_condition_list_opt:
	/* empty */	{ set_empty (&$$); }
	| select_condition_list
	;

select_condition_list:
	select_condition
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| select_condition_list select_condition
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

select_condition:
	Y_CONNECT Y_BY boolean_expression
		{
		   $$.ast = symbol_call_expr (&$1, $1, $3);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| Y_START Y_WITH boolean_expression
		{
		   $$.ast = symbol_call_expr (&$1, $1, $3);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| select_group
	;

select_group:
	Y_GROUP Y_BY expression_list
		{
		   $$.ast = symbol_call_expr (&$1, $1, $3);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| Y_HAVING boolean_expression
		{
		   $$.ast = symbol_call_expr (&$1, $1, $2);
		   ast_child_tree ($$.ast, $2.ast);
		}
	;

select_order_update_list_opt:
	/* empty */	{ set_empty (&$$); }
	| select_order_update_list
	;

select_order_update_list:
	select_order_update
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| select_order_update_list select_order_update
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

select_order_update:
	select_order
	| select_for_update
	;

select_order:
	Y_ORDER Y_BY expression_asc_list
		{
		   $$.ast = symbol_call_expr (&$1, $1, $3);
		   ast_child_tree ($$.ast, $3.ast);
		}
	;

expression_asc_list:
	expression_asc
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| expression_asc_list ',' expression_asc
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

expression_asc:
	expression asc_opt
		{
		   if (is_empty ($2)) {
		      $$.ast = $1.ast;
		   } else {
		      $$.ast = symbol_call_expr (&$2, $1, $2);
		      ast_child_tree ($$.ast, $1.ast);
		   }
		}
	;

asc_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_ASC
	| Y_DESC
	;

select_for_update:
	Y_FOR Y_UPDATE of_opt tableref_column_list_opt
	;

of_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_OF
	;

tableref_column_list_opt:
	/* empty */	{ set_empty (&$$); }
	| tableref_column_list
	;

tableref_column_list:
	tableref_column
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| tableref_column_list ',' tableref_column
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

/* TBD: add x.x.x ? */
tableref_column:
	SYM_IDENTIFIER
		{
		   /* Check for tablename */
		   if (sql_scope_table_name ()) {
		      sprintf (txtbuf, "%s.%s", 
		         sql_scope_table_name (), token_text ($1));

		      $1.sym = sql_symbol_extern (txtbuf, sk_column);

		   } else if ( ! $1.sym ) {
		      $1.sym = symbol_extern ($1, sk_externref);
		   }

		   $$.sym = $1.sym;
		   $$.ast = symbol_token ($1);
		}
	| SYM_IDENTIFIER '.' SYM_IDENTIFIER
		{
		   if ( ! symbol_is_known ($1) ) {
		      $1.sym = symbol_extern ($1, sk_table);
		   }

		   sprintf (txtbuf, "%s.%s", 
		      token_text ($1), token_text ($3));

		   switch (symbol_get ($1)) {
		    case sk_table:
		      $$.sym = sql_symbol_extern (txtbuf, sk_column);
		      break;
		    case sk_alias:
		      /* 
		       * track table name
		       */
		      if ($1.sym->alias_sym) {
			 sprintf (txtbuf, "%s.%s", 
				  $1.sym->alias_sym->sym_text,
				  token_text ($3));
			 $$.sym = sql_symbol_extern (txtbuf, sk_column);
		      } else {
			 $$.sym = sql_symbol_extern (txtbuf,
						     sk_externref);
		      }
		      break;
		    default:
		      /* overloaded name */
		      $1.sym = symbol_extern ($1, sk_table);
		      $$.sym = sql_symbol_extern (txtbuf, sk_column);
		   }

		   $$.ast = symbol_tree_ast ($$.sym, $1, $3);
		   ast_child_tree ($$.ast, symbol_reference ($1));
		   ast_child_tree ($$.ast,
				      symbol_reference_tree ($$.sym, $3));
		}
	;

subquery:
	select_query
	;

subquery_or_expr_list:
	select_query
	| expression_list
		{
		   /* enclose list in block */
		   $$.ast = new_tree (ank_ast_block, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	;

/**************** Transaction ******************/

set_transaction_statement: 
	Y_SET transaction_item_list ';'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

transaction_item_list:
	transaction_item
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| transaction_item_list transaction_item
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

/* transaction grammar:
 * Y_SET Y_TRANSACTION transaction_type ';'
 *
 * transaction_type:
 * Y_READ Y_ONLY
 * | Y_READ Y_WRITE
 * | Y_ISOLATION Y_LEVEL transaction_level
 * | Y_USE Y_ROLLBACK Y_SEGMENT SYM_IDENTIFIER
 * ;
 * transaction_level:
 * Y_SERIALIZABLE
 * | Y_READ Y_COMMITTED
 *
 * Currently only Y_ROLLBACK is a keyword
 */
transaction_item:
	Y_ROLLBACK
	| SYM_IDENTIFIER
		{
		   /* output keyword entry */
		   symbol_keyword_list ($1, keylist_transaction);
		}		    
	;


/**************** Triggers ******************/

trigger_body:
	Y_TRIGGER SYM_IDENTIFIER
		{
		   $2.sym = symbol_set ($2, sk_trigger);
		   sql_push_scope ($2.sym->sym_text, $2.sym);
		   sql_inside_trigger = 1;
		}
	trigger_time trigger_event_list Y_ON table_reference
		{
		   trigger_table_name = sql_scope_table_name ();
		}
	trigger_referencing_opt trigger_row_opt when_expression_list_opt
	plsql_block
		{
		   $$.ast = new_tree (ank_ast_block, $2.sym, $1, $12);

		   ast_child_tree ($$.ast, symbol_token ($2));

		   /* trigger_time */
		   ast_child_tree ($$.ast, $4.ast);

		   /* trigger_event_list */
		   ast_child_tree ($$.ast, $5.ast);

		   /* table */
		   ast_child_tree ($$.ast, $7.ast);

		   /* trigger_referencing_opt */
		   ast_child_tree ($$.ast, $9.ast);

		   /* trigger_row_opt */
		   ast_child_tree ($$.ast, $10.ast);

		   /* when_expression_list_opt */
		   ast_child_tree ($$.ast, $11.ast);

		   /* plsql_block */
		   /* plsql_block includes terminating semi-colon */
		   ast_child_tree ($$.ast, $12.ast);

		   ast_defined ($$.ast);
		   sql_pop_scope ();
		   trigger_table_name = 0;
		   sql_inside_trigger = 0;
		}
	;

trigger_time:
	Y_BEFORE
	| Y_AFTER
	;

trigger_event_list:
	trigger_event
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| trigger_event_list Y_OR trigger_event
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

trigger_event:
	Y_DELETE
	| Y_INSERT
	| Y_UPDATE trigger_column_list_opt
		{
		   if ( ! is_empty ($2) ) {
		      set_result (&$$, $1, $2, 0, 0);
		   }
		}
	;

trigger_column_list_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_OF column_list
		{
		   $$.ast = $2.ast;
		}
	;

trigger_referencing_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_REFERENCING old_new_as
	| Y_REFERENCING old_new_as old_new_as
	;

old_new_as:
	Y_OLD as_opt alias_name
	| Y_OLD as_opt Y_OLD
	| Y_NEW as_opt alias_name
	| Y_NEW as_opt Y_NEW
	;

as_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_AS
	;

/* FOR EACH ROW */
trigger_row_opt:
	/* empty */	{ set_empty (&$$); }
	| Y_FOR keyword keyword
		{
		   set_result (&$$, $1, $3, 0, 0);
		}
	;

trigger_boolean:
	Y_INSERTING
	| Y_DELETING
	| Y_UPDATING
	;

/**************** SQL Values ******************/

sqlcode_function:
	Y_SQLCODE
		{
		   $$.ast = symbol_call_expr (&$1, $1, $1);
		}
	;


sqlerrm_function:
	Y_SQLERRM error_number_opt
		{
		   sqltype tlast;
		   if (!is_empty ($2)) {
		      tlast = $2;
		   } else {
		      tlast = $1;
		   }

		   $$.ast = symbol_call_expr (&$1, $1, tlast);
		   ast_child_tree ($$.ast, $2.ast);
		}
	;

error_number_opt:
	/* empty */	{ set_empty (&$$); }
	| '(' expression ')'
		{
		   set_result (&$$, $1, $3, 0, $2.ast);
		}
	;

/**************** Update ******************/

update_statement: 
	Y_UPDATE
		{
		   sql_keywords_only ();
		   esql_prefix_check ();
		   sql_push_scope (0, 0);
		}
	tableref_entry Y_SET update_set_list where_clause_opt sql_eos 
		{
		   sqltype tlast = $6;
		   if (is_empty ($6)) {
		      tlast = $5;
		   }
		   $$.ast = symbol_call_stmt (&$1, $1, tlast, $7);
		   ast_child_tree ($1.ast, $3.ast);
		   ast_child_tree ($1.ast, $5.ast);
		   ast_child_tree ($1.ast, $6.ast);
		   sql_pop_scope ();
		   set_result (&$$, $1, $7, 0, $$.ast);
		}
	;

update_set_list:
	update_set
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| update_set_list ',' update_set
		{
		   ast_child_tree ($1.ast, $3.ast);
		   set_result (&$$, $1, $3, 0, $1.ast);
		}
	;

update_set:
	column_set
	| column_list_set
	;

column_set:
	tableref_column '=' expression
		{
		   $$.ast = new_tree (ank_assign_expr, 0, $1, $3);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $3.ast);
		}
	| tableref_column '=' '(' subquery ')'
		{
		   $$.ast = new_tree (ank_assign_expr, 0, $1, $5);
		   ast_child_tree ($$.ast, $1.ast);
		   ast_child_tree ($$.ast, $4.ast);
		}
	;

column_list_set:
	'(' tableref_column_list ')' '=' '(' subquery_or_expr_list ')'
		{
		   $$.ast = new_tree (ank_assign_expr, 0, $1, $7);
		   ast_child_tree ($$.ast, $2.ast);
		   ast_child_tree ($$.ast, $6.ast);
		}
	;

/* generic tokens are returned by the scanner when it is in
 * generic mode.  Used to ignore statements that are not of
 * interest.  EOS ends generic mode.
 */
generic_token_list_opt:
	/* empty */	{ set_empty (&$$); }
	| generic_token_list
	;

generic_token_list:
	generic_token
		{
		   $$.ast = new_tree (ank_list, 0, $1, $1);
		   ast_child_tree ($$.ast, $1.ast);
		}
	| generic_token_list generic_token
		{
		   ast_child_tree ($1.ast, $2.ast);
		   set_result (&$$, $1, $2, 0, $1.ast);
		}
	;

generic_token:
	GENERIC_KEYWORD
	| GENERIC_TOKEN
	| SYM_IDENTIFIER
	| host_variable
	;



%%

/*
 * symbol_set - add symbol to the current scope
 */
SQL_SYM * symbol_set (sqltype t, sym_kind symbol_type)
{
   SQL_SYM * sym = 0;
   token *tok = get_token (t.start_token);

   if (tok) {
      sym = symbol_add (tok->text, symbol_type);
   }

   return (sym);
}


/*
 * symbol_add
 *
 * Add a symbol to the local scope and include any procedure/function
 * information.  Also, in the case of package members add a global
 * scope entry with the fully specified name.
 */
SQL_SYM * symbol_add (const char * text, sym_kind sk)
{
   SQL_SYM * sym;
   SQL_SYM * global_sym;
   SQL_SYM * proc_sym;
   char buffer [1024];

   /* if inside a procedure/function add that info to symbol entry */
   if ((proc_sym = sql_scope_proc_symbol ()) != 0) {

      /* add symbol to local scope */
      sym = sql_symbol_add (text, sk);

      sym->proc_sym = proc_sym;
      sym->proc_block = sql_scope_proc_block ();

   } else if (package_name && (sql_scope_level () == 1)) {
      /* top level of a package */

      sprintf (buffer, "%s.%s", package_name, text);

      /* add global symbol with full name */
      global_sym = sql_symbol_extern (buffer, sk);

      /* add symbol to local scope */
      sym = sql_symbol_local (global_sym, text);
   } else {
      /* add symbol to local scope */
      sym = sql_symbol_add (text, sk);
   }

   return (sym);
}

/*
 * symbol_extern - add symbol to the global scope if not already there
 */
SQL_SYM * symbol_extern (sqltype t, sym_kind symbol_type)
{
   SQL_SYM * psym = 0;
   token *tok = get_token (t.start_token);

   if (tok) {
#if SQL_DEBUG != 0
      if (sql_debug_level >= 2) {
	 fprintf (stderr, "<SymExt: %s: %d>\n", tok->text, symbol_type);
      }
#endif

      psym = sql_symbol_extern (tok->text, symbol_type);
   }

   return (psym);
}

/*
 * symbol_field - add symbol to the record symbol's scope if not already there
 */
STATIC SQL_SYM * symbol_field (SQL_SYM * rec_sym, const char * text)
{
   SQL_SYM * sym = 0;

   sym = sql_symbol_lookup (text);
   if ( (! sym ) || (sym->sym_token_type == sk_externref) ) {
      /* add field name to same scope as record */
      sym = sql_symbol_add_to_symbol_scope (rec_sym, text, sk_field);
      if ( ! sym ) {
	 sym = sql_symbol_add (text, sk_field);
      }
   }
   return (sym);
}
	
/*
 * get the symbol kind (e.g. variable, table) of an identifier
 */
sym_kind symbol_get (sqltype t)
{
   if (t.sym) {
      return (t.sym->sym_token_type);
   }
   return (0);
}

/* check if symbol type matches
 *
 * displays error if no match.
 *
 * returns
 *	1 for match (good)
 * 	0 no match (error)
 */
int symbol_check (sqltype t, sym_kind symbol_type)
{
   char errbuf [256];
   token *tok = get_token (t.start_token);

   if (t.sym) {
      if (! is_symbol_kind_match (t.sym->sym_token_type, symbol_type) ) {
	    sprintf (errbuf, "got %s, expecting %s symbol",
	       sym_kind_lookup (t.sym->sym_token_type),
	       sym_kind_lookup (symbol_type));
	    iff_error_token (tok, errbuf);
	    return (0);
      }
   } else {
	    sprintf (errbuf, "expecting %s symbol",
	       sym_kind_lookup (symbol_type));
	    iff_error_token (tok, errbuf);
	    return (0);
   }
   return (1);
}

/* check if a symbol is known
 *
 */
int symbol_is_known (sqltype t)
{
   int retval = 0;
   if (t.sym && (t.sym->sym_token_type != sk_externref)) {
      retval = 1;
   }
   return (retval);
}

/*
 * set_result - set the rule result to be passed up when the
 * grammar rule is reduced.
 *
 * result		-  pointer to $$.
 * left and right	- token endpoints used to figure out the
 *			  line/column range of the rule.
 * sym			- pointer to rule symbol
 * ast			- syntax tree for rule
 */
void set_result (sqltype *result,
		 sqltype left,
		 sqltype right,
		 SQL_SYM * sym,
		 ast_tree * ast)
{
   result->start_token = left.start_token;
   result->end_token = right.end_token;
   result->sym = sym;
   result->ast = ast;
}

/* generate an ast error node for code region with syntax error */
STATIC void error_result (sqltype *result,
   sqltype left,
   sqltype right)
{
   set_result (result, left, right, 0, error_node (left, right));
}

STATIC ast_tree * error_node (sqltype left, sqltype right)
{
   return (new_tree (ank_error_mark, 0, left, right));
}

/*
 * set_empty - Used to flag an optional rule as empty
 * so it can be tested with is_empty().
 *
 * Token numbers start with 1, so 0 indicates no token.
 */
void set_empty (sqltype *result)
{
   result->start_token = 0;
   result->end_token = 0;
   result->sym = 0;
   result->ast = 0;
}

/*
 * is_empty - Used to check if an optional rule is not present.
 */
int is_empty (sqltype t)
{
   if ((t.start_token == 0) &&
	 (t.end_token == 0) &&
	 (t.sym == 0) &&
	 (t.ast == 0)) {
      return (1);
   }
   return (0);
}

/* Generate a call statement from a token and arguments.
 * A call statement is a call expression (function or procedure call)
 * by itself (i.e. not part of some other expression).
 * Example: "foo(x);"
 */
ast_tree * symbol_call_stmt (sqltype *tp,
			     sqltype left,
			     sqltype call_right,
			     sqltype stmt_right)
{
   ast_tree * at = new_tree (ank_expr_stmt, 0, left, stmt_right);
   ast_child_tree (at, symbol_call_expr (tp, left, call_right));
   return (at);
}

/* generate a call statement from a function name (text) and arguments */
ast_tree * sql_call_stmt (const char * funcname,
			  sqltype left,
			  sqltype call_right,
			  sqltype stmt_right)
{
   ast_tree * at = new_tree (ank_expr_stmt, 0, left, stmt_right);
   ast_child_tree (at, sql_call_expr (funcname, left, call_right));
   return (at);
}

/* Generate a call expression from a token and arguments.
 * A call expression is a function or procedure call that is part
 * of another expression.  Example: "foo(x)" in "a = foo(x);".
 */
ast_tree * symbol_call_expr (sqltype *tp, sqltype left, sqltype right)
{
   if (!tp) {
      return (0);
   }
   tp->sym = symbol_extern (*tp, sk_function);

   tp->ast = new_tree (ank_call_expr, tp->sym, left, right);
   ast_child_tree (tp->ast, symbol_token (*tp));
   /* TBD: ast_relation (tp->ast, rk_call); */

   return (tp->ast);
}

/* generate a call expression from a function name (text) and arguments */
ast_tree * sql_call_expr (const char * funcname,
			  sqltype left,
			  sqltype right)
{
   ast_tree * at = 0;
   SQL_SYM * sym = 0;
   sym = sql_symbol_extern (funcname, sk_function);

   at = new_tree (ank_call_expr, sym, left, right);
   ast_child_tree (at, symbol_tree_range (sym, left, right));
   /* TBD: ast_relation (at, rk_call); */

   return (at);
}



/*
 * symbol_type - add a datatype to the global scope
 */
SQL_SYM * symbol_type (sqltype t)
{
   SQL_SYM * sym = symbol_extern (t, sk_type);

   return (sym);
}

/*
 * symbol_type_text - add a datatype to the global scope
 */
SQL_SYM * symbol_type_text (const char * text)
{
   SQL_SYM * sym = sql_symbol_extern (text, sk_type);

   return (sym);
}

/*
 * token_text - returns the text value of a token
 *
 * Only works for single tokens.
 */
const char* token_text (sqltype t)
{
   token * tok = get_token (t.start_token);

   if (tok) {
      return (tok->text);
   }
   return ("");
}

/*
 * new_tree - creates a new ast tree node and initializes it
 * with the values passed.
 */
ast_tree * new_tree (ast_node_kind ank,
   SQL_SYM *sym,
   sqltype tstart,
   sqltype tend)
{
   ast_loc start;
   ast_loc end;
   token * tok;
   token * endtok;
   int toklen;

   start.lineno = 0;
   start.column = 0;
   end.lineno = 0;
   end.column = 0;

   /* Calculate the starting and ending line and column numbers.
    * Leaves start zero to indicate unknown (line number is one-relative).
    * Leaves end zero to indicate same as start.
    */
   if (tstart.start_token) {
      tok = get_token (tstart.start_token);
      if (tok) {
	 start.lineno = tok->lineno;
	 start.column = tok->column;
	 toklen = strlen (tok->text);

	 if (tend.end_token) {
	    endtok = get_token (tend.end_token);
	    if (endtok) {
	       end.lineno = endtok->lineno;
	       /* Calcuate last character of last token.
		* The minus one is because tok->column already
		* indicates the first character of the token.
		*/
	       end.column = endtok->column + strlen (endtok->text) - 1;
	    }
	 } else {
	    end.lineno = start.lineno;
	    end.column = start.column + toklen - 1;
	 }
      }
   }

   return (ast_new_tree (ank, sym, start, end, 0));
}

ast_loc token_loc_start (sqltype t)
{
   ast_loc start;
   token * tok;

   start.lineno = 0;
   start.column = 0;

   if (t.start_token) {
      tok = get_token (t.start_token);
      if (tok) {
	 start.lineno = tok->lineno;
	 start.column = tok->column;
      }
   }
   return (start);
}

ast_loc token_loc_end (sqltype t)
{
   ast_loc end;
   token * tok = 0;

   end.lineno = 0;
   end.column = 0;

   if (t.end_token) {
      tok = get_token (t.end_token);
   } else if (t.start_token) {
      tok = get_token (t.start_token);
   }

   if (tok) {
      end.lineno = tok->lineno;
      end.column = tok->column + strlen (tok->text) - 1;
   }

   return (end);
}

/*
 * empty_tree - create a place holder tree node with no location info.
 */
ast_tree * empty_tree (ast_node_kind ank)
{
   ast_loc start;
   ast_loc end;

   start.lineno = 0;
   start.column = 0;
   end.lineno = 0;
   end.column = 0;

   return (ast_new_tree (ank, 0, start, end, 0));
}

/*
 * symbol_token -
 * symbol_tree -
 * symbol_tree_range -
 * symbol_tree_ast -
 * symbol_tree_type -
 *
 * creates a new tree node for this symbol instance
 * reference mask indicates if symbol is to appear in the AST, SMT, or both.
 */

ast_tree * symbol_token (sqltype t)
{
   return (symbol_tree_range (t.sym, t, t));
}

ast_tree * symbol_tree (SQL_SYM * sym, sqltype t)
{
   return (symbol_tree_range (sym, t, t));
}

ast_tree * symbol_tree_range (SQL_SYM *sym,
   sqltype tstart,
   sqltype tend)
{
   return (symbol_tree_type (sym, tstart, tend, REFMASK_SMT | REFMASK_AST));
}

ast_tree * symbol_tree_ast (SQL_SYM *sym,
			    sqltype tstart,
			    sqltype tend)
{
   return (symbol_tree_type (sym, tstart, tend, REFMASK_AST));
}

STATIC ast_tree * symbol_tree_type (SQL_SYM *sym,
   sqltype tstart,
   sqltype tend,
   int refmask)
{
   if ( ! sym ) {
      return (0);
   }

   return (reference_type_tree (ank_symbol, sym, tstart, tend, refmask));
}

ast_tree * reference_tree (ast_node_kind ank,
			   SQL_SYM *sym,
			   sqltype tstart,
			   sqltype tend)
{
   return (reference_type_tree (ank, sym, tstart, tend,
				REFMASK_SMT | REFMASK_AST));
}

ast_tree * symbol_reference (sqltype t)
{
   return (reference_type_tree (ank_none, t.sym, t, t, REFMASK_SMT));
}

ast_tree * symbol_reference_tree (SQL_SYM * sym, sqltype t)
{
   return (reference_type_tree (ank_none, sym, t, t, REFMASK_SMT));
}


ast_tree * reference_type_tree (ast_node_kind ank,
				SQL_SYM *sym,
				sqltype tstart,
				sqltype tend,
				int refmask)
{
   ast_tree * at = 0;

   if ( ! sym ) {
      return (0);
   }

   /*
    * Check for built-in global symbols not yet flaged as referenced
    *
    * Built-in symbols (standard Oracle functions, packages,
    * error exceptions, etc) are in the symbol table without
    * a symbol number.  A symbol number is assigned only when
    * one of these is found (referenced) in the program.
    */
   if ( ! sym->sym_number ) {
      /* assigns a symbol number */
      sql_symbol_number_assign (sym);
   }

   at = new_tree (ank, sym, tstart, tend);
   if (at) {
      at->info.reference = refmask;
   }
   return (at);
}


/* 
 * literal_tree - create an ast tree node with literal text.
 *
 * Note that this includes numeric literals, which are kept as text
 * and other constants such as NULL, TRUE and FALSE.
 */
ast_tree * literal_tree (sqltype t)
{
   ast_tree * at;
   ast_loc start;
   ast_loc end;
   token * tok;
   SQL_SYM * sym;

   start.lineno = 0;
   start.column = 0;
   end.lineno = 0;
   end.column = 0;

   tok = get_token (t.start_token);

   if (tok) {
      sym = symbol_extern (t, sk_constant);
      at = symbol_tree (sym, t);
      if (at) {
	 ast_child_tree (at,
			 ast_new_tree (ank_none, 0, start, end, tok->text));
	 return (at);
      }
   }
   return (0);
}

/* 
 * string_literal_tree - create an ast tree node with literal text.
 */
ast_tree * string_literal_tree (sqltype t)
{
   ast_tree * at = 0;
   ast_loc start;
   ast_loc end;
   token * tok;
   token * tokend;
   SQL_SYM * sym;

   start.lineno = 0;
   start.column = 0;
   end.lineno = 0;
   end.column = 0;

   tok = get_token (t.start_token);

   if (tok) {
      sym = symbol_extern (t, sk_string);
      at = symbol_tree (sym, t);
      if (at) {
	 /* check for multiline strings:
	  * these have a special end point token which is blank
	  * but contains the last postion (the final quote) of the string.
	  */
	 if (t.end_token && (t.start_token != t.end_token)) {
	    tokend = get_token (t.end_token);
	    if (tokend) {
	       at->end.lineno = tokend->lineno;
	       at->end.column = tokend->column;
	    }
	 }

	 /* make a child tree with the string constant value */
	 ast_child_tree (at,
			 ast_new_tree (ank_none, 0, start, end, tok->text));
      }
   }
   return (at);
}

/*
 * ast_relation - Set the symbol relation for an ast tree node.
 *
 * This relates the symbol in the node's parent to the child symbol at this
 * or a lower node.  After parsing, when the tree is walked, this
 * relationship will be output as a relation entry.
 *
 * Note that relationships and their attributes need not be set
 * in the child symbol tree node, but can be any non-symbol node between
 * the parent symbol node and the child symbol node.  However, a
 * relation is between two (and only two) symbols, so only
 * non-symbol nodes can be in-between.  Also relations between a
 * parent and child cannot be set in the parent's node, because
 * that is the place for relations between the parent and its
 * parent.
 */
void ast_relation (ast_tree * at, relation_kind rk)
{
   if (!at) {
      return;
   }

   at->info.relation = 1;
   at->info.rk = rk;
}

/*
 * ast_relation_attr - Add a relationship attribute to the current node.
 */
void ast_relation_attr (ast_tree * at, int reltype)
{
   if (!at) {
      return;
   }

   at->info.relation_attr_mask |= reltype;
}

/*
 * ast_relation_arg - Add the Argument relation attribute and set its value.
 *
 * E.g. y is the second argument to foo in "foo (x, y)".
 */
void ast_relation_arg (ast_tree * at, int arg)
{
   if (!at) {
      return;
   }

   at->info.relation_attr_mask |= RELMASK_ARG;
   at->info.arg = arg;
}

/*
 * ast_relation_size - Add the Size relation attribute and set its value.
 *
 * E.g. size is 100 in "x VARCHAR2 (100)".
 */
void ast_relation_size (ast_tree * at, int size)
{
   if (!at) {
      return;
   }

   at->info.relation_attr_mask |= RELMASK_SIZE;
   at->info.size = size;
}

/*
 * ast_relation_scale - Add the Scale relation attribute and set its value.
 *
 * E.g. scale is 2 in "x DECIMAL (10,2)"
 */
void ast_relation_scale (ast_tree * at, int scale)
{
   if (!at) {
      return;
   }

   at->info.relation_attr_mask |= RELMASK_SCALE;
   at->info.scale = scale;
}

/*
 * ast_defined - Flag that the tree node contains the information
 * for a symbol definition.
 */
void ast_defined (ast_tree * at)
{
   if (at && at->sym) {
      at->info.defined = 1;
   }
}

/*
 * ast_decldefined - Flag that the tree node contains the information
 * for a symbol declaration's definition.
 */
void ast_decldefined (ast_tree * at)
{
   if (at && at->sym) {
      at->info.decldef = 1;
   }
}

/*
 * ast_replacement - Flag that the tree node should be marked in the IF
 * as a replacement for a host language node (esql only).
 */
STATIC void ast_replacement (ast_tree * at)
{
   if (at) {
      at->info.replace = 1;
   }
}

/*
 * ast_keyword - Flag that the tree node represents a keyword
 * so that a SMT entry is not generated for it, since keyword SMT
 * entries are handled by scanner.
 */
void ast_keyword (ast_tree * at)
{
   if (at) {
      at->info.keyword = 1;
   }
}

/*
 * symbol_attribute - Add a symbol attribute.
 *
 * E.g. that a variable is a host variable.
 */
STATIC void symbol_attribute (SQL_SYM * sym, int attrtype)
{
   if (!sym) {
      return;
   }

   sym->attr_mask |= attrtype;
}

/*
 * End-of-statement routines
 */
STATIC void eos_sql (void)
{
}

STATIC void eos_plsql (void)
{
}

STATIC void eos_esql (void)
{
}

/* allow only SQL keywords (not PL/SQL)
 * in effect until popped or EOS
 */
STATIC void sql_keywords_only (void)
{
   sql_push_keyword_mode (SQLKEY_SQL);
}

/* no keywords - just identifiers */
STATIC void sql_no_keywords (void)
{
   sql_push_keyword_mode (SQLKEY_NONE);
}

/* allow keywords again */
STATIC void sql_clear_no_keywords (void)
{
   if (sql_get_keyword_mode () == SQLKEY_NONE) {
      sql_pop_keyword_mode ();
   }
}

/* clear flags that might have been in effect when error detected */
STATIC void sql_error_clear (void)
{
   in_specification = 0;
   sql_inside_trigger = 0;
   sql_clear_no_keywords ();
}

/* sql_specfile_output ---
 *
 * Output a new data definition to the symbols specification file
 * listed on the command line.
 *
 * Copies the definition token text from the leftmost to rightmost
 * token.  Preserves line and column spacing, but each tab is converted
 * to a single space.
 */
int sql_specfile_output (SQL_SYM * sym, sqltype left, sqltype right)
{
   int ret_val = 0;
   unsigned int i;
   token* p;
   int current_lineno = 0;
   int current_column = 0;
   int first_token = 1;
   int ret_io;
   int len;

   /* check if already in specification file */
   if (sym && (sym->sym_specfile == 0)) {
      /* check if specification file is open */
      if (symbol_specification_file) {
	 /* copy text of source code region */
	 for (i = left.start_token; i <= right.end_token; i++) {
	    p = get_token(i);
	    if (p) {
	       /* first text -- sync line number and column */
	       if (first_token) {
		  first_token = 0;
		  current_lineno = p->lineno;
		  current_column = 1;
	       } else if (p->lineno > current_lineno) {
		  /* check for newline */
		  current_lineno = p->lineno;
		  current_column = 1;
		  if (fputc ('\n', symbol_specification_file) == EOF) {
		     perror ("");
		  }
	       }
	       /* seek to correct column */
	       for ( ; p->column > current_column; ++current_column) {
		  if (fputc (' ', symbol_specification_file) == EOF) {
		     perror ("");
		  }
	       }
	       /* output token text */
	       len = strlen (p->text);
	       if ((ret_io = fwrite(p->text, (size_t) 1, (size_t) len,
				    symbol_specification_file)) != len) {
		  perror ("Error writing to symbols file");
	       }
	       current_column += len;
	    }
	 }
	 /* if any output - print newline and set symbol flag */
	 if ( ! first_token ) {
	    if ((ret_io = fprintf(symbol_specification_file, "\n\n")) < 0) {
	       perror ("");
	    }
	    fflush (symbol_specification_file);
	    sym->sym_specfile = 1;
	 }
      } else {
	 /* set flag for read-only (-include) specification files also.
	  * That way we won't copy specs that are already in a -include file
	  * that is before a -symbols file on the command line.
	  */
	 sym->sym_specfile = 1;
      } /* symbol_specification_file */
   }

   return (ret_val);
}

/* esql_prefix_check --
 *
 * Check and output the host file block prefix if needed.
 * Called for each executable sql statement that can appear
 * in an esql program at the point in the grammar where the
 * statement type is known.
 */
STATIC int esql_prefix_check (void)
{
   /* only esql and prefix not yet output */
   if (esql_mode && (need_postfix == 0)) {
      /* if scope_nesting_level is 0 (e.g. host program global scope)
       * check suppress flag.
       */
      if (scope_nesting_level || ( ! suppress_global_references ) ) {
	 /* output prefix.  Open brace for local scope,
	  * Dummy function for global/file scope.
	  */
	 prefix_reference(prefix_token);
	 /* set flag indicating need for postfix (closing brace) */
	 need_postfix = 1;
      }
   }
   return (0);
}

/* handle lists of VARCHAR variables
 * TBD: not active
 */
STATIC ast_tree * varchar_list_decl (sqltype varchar_decl,
				     sqltype varchar_type)
{
   ast_tree * at = 0;
   ast_tree * at_spec;
   SQL_SYM * sym;
		   
   if (varchar_decl.ast) {
      at = new_tree (ank_list_decl, 0, saved_token, varchar_decl);

      sym = symbol_type (varchar_type);
      at_spec = reference_tree (ank_ast_declspec, sym,
				varchar_type, varchar_type);
      ast_keyword (at_spec);

      ast_child_tree (at, at_spec);
      ast_child_tree (at, varchar_decl.ast);
   }
   return (at);
}

/* handle next/last VARCHAR variable in a list.
 * TBD: not active
 */
STATIC int varchar_set_end (ast_tree * at_head, sqltype end_token)
{
   ast_tree * at;

   at = ast_find_node_kind (at_head, ank_list_decl);
	 
   if (at) {
      ast_extend_end (at, token_loc_end (end_token));
#if 0
      at = ast_find_node_kind (at, ank_ast_declspec);
      if (at) {
	 ast_extend_end (at, token_loc_end (end_token));
      }
#endif
   }
   return (0);
}

/* symbol is really a keyword - output IFF keyword entry */
STATIC int symbol_keyword (sqltype t)
{
   token *tok = get_token (t.start_token);

   if (tok) {
      iff_keyword (tok);
   }
   return (0);
}

/* check if symbol is really a keyword - output IFF keyword entry */
STATIC int symbol_keyword_check (sqltype t, const char *keyword)
{
   int found = 0;
   token * tok = get_token (t.start_token);
   if (tok && tok->text) {
      const char * id = tok->text;
      if (id) {
	 if (compcaseless (id, keyword) == 0) {
	    found = 1;
	    iff_keyword (tok);
	 }
      }
   }
   return (found);
}

/* check keyword list if symbol is a keyword
 * yes - output IFF keyword entry and return 1
 * otherwise return 0
 */
STATIC int symbol_keyword_list (sqltype t, const char **spp)
{
   int found = 0;
   token * tok = get_token (t.start_token);
   if (tok && tok->text) {
      const char * id = tok->text;
      if (id) {
	 for ( ; *spp && !found; spp++) {
	    if (compcaseless (id, *spp) == 0) {
	       found = 1;
	       iff_keyword (tok);
	    }
	 }
      }
   }
   return (found);
}

/* sql_tableref - sql table name reference
 *
 * set token as table symbol/ast.
 * store name in last_table_name, which may used to resolve
 * column names that follow.
 */
int sql_tableref (sqltype *tp)
{
   if (tp) {
      tp->sym = symbol_extern (*tp, sk_table);
      tp->ast = symbol_token (*tp);

      /* save this table name */
      last_table_name = token_text (*tp);
   }
   return (0);
}

#if YYBISON == 1
/*
 * yykeywordcheck - check if keyword is valid given current yyparse state
 *
 * Argument:   	tokenvalue - yylex "return value" of keyword
 *
 * Returns:	1 - keyword valid.
 *		0 - keyword not valid at this point in the grammar
 *
 * This routine only applies to Bison.  A different method is used
 * for Thinkage.  See yyerror() in the scanner.
 *
 * This routine is called by the keyword processing routines, in
 * sql_keyword.C (which is called by the scanner, in sql.l).
 * A zero return will cause the scanner to treat the keyword as an identifer.
 *
 * Example:
 *	"CREATE TABLE table ..."
 *	The third word is the keyword "table", but since in the grammar
 *	state after CREATE TABLE that is not valid (it would cause a parse
 *	error), this routine will return invalid and yylex() will
 *	return SYM_IDENTIFIER to the parser (instead of Y_TABLE)
 *	and "table" will be the table name.
 *
 * This routine is needed to handle these few cases where the user
 * is using reserved keywords as identifers.  Even though the vendor
 * (e.g. Oracle, Informix) documentation states that this is not allowed,
 * they accept many of these constructs. Therefore, so must we.
 * Most code would parse fine without this check and it can be turned
 * off to increase speed, etc. if needed.
 * 
 * Method:
 *
 * Does not do check if scanner/parser is in generic mode or error
 * recovery.  Otherwise clears local state stack and calls
 * yycharcheck() with the relevent state variables to simulate
 * the return of the passed keyword from yylex to the parser to
 * see if the keyword is valid.
 *
 *
 * Notes:
 *
 * This code is a complex hack and involves a lot of "cargo cult programming"
 * in that I've blindly copied code from yyparse() (i.e. bison.simple)
 * to simulate the grammar shifts and reductions without fully understanding
 * all the underlining logic.  I've commented based upon what I think I
 * known, but take the comments with a grain of salt.
 * 
 * Note also that this is the reverse of the normal Lex/Yacc flow.  Where
 * the scanner returns tokens to the parser without any knowledge of the
 * parser internals.  Here the scanner is testing what the parser is
 * going to do with its return value before it returns it.
 *
 * There are other ways to deal with the reserved keyword as identifier
 * problem.  Some of which are also employed in the grammar above and
 * in the scanner keyword routines.  See comments for
 * sql_keyword_command() in sql_keyword.C and sql_keytable.C and the use
 * sql_no_keywords() above, etc.  However, these cannot handle all the
 * cases.  Alternate ways might include: exhaustively listing all "invalid"
 * keywords for each grammar rule and treating them as identifiers;
 * and abandoning Lex and Yacc altogether. If you know of a better
 * way of doing this please let me know.  Mark B. Kaminsky kaminsky@acm.org
 *
 * The below method still won't handle all cases, such as when a keyword
 * looks valid, but further lookahead reveals problems.  In addition, this
 * method will mask some real parse errors and cause others to be found
 * later in the code.
 *
 * Dependencies: (for both yykeywordcheck and yycharcheck)
 *
 * These routines are highly dependent on the bison parser skeleton
 * /usr/lib/local/bison.simple.
 *
 * They must be in the bison/yacc module (sql.y) since they use
 * the static tables and #defines found there.
 *
 * In the first section of this module (above inside the "%{" and "%}")
 * YYLEX must be defined to first save the yyparse state local variables
 * listed below and then call yylex() (_save is appended to these for the
 * check versions).
 *	yystate - the current state.
 *	yyn - the next state to shift or rule to reduce
 *	yyssp - the state stack pointer
 *	yyerrstatus - error recovery status
 *
 */
EXTERN_C int yykeywordcheck (int tokenvalue)
{
   int retval = 1;
   if ( (!scanner_generic_mode) &&
       (!sql_allow_error) &&
       (!yyerrstatus_save)) {
      /* clear our local state stack used in yycharcheck */
      yycheck_stack_len = 0;
      retval = yycharcheck (tokenvalue, yystate_save, yyn_save);
   }
   return (retval);
}

/*
 * yycharcheck - recursive routine to check if token is valid in the given
 * yyparse() state.
 *
 * See yykeywordcheck() comments above for overview and details.
 *
 * Most of this is copied from yyparse() (in bison.simple) starting
 * from when yylex() returns its value in yychar.
 *
 * Since this routine cannot change the real state stack it uses
 * its own small stack for pushing the new reduce state when needed.
 * For popping the state stack it uses the saved copy (yyssp_save)
 * after its own is empty.  Note that we should never change the
 * real stack (*yyssp or *yyssp_save) contents!
 *
 *	yycheck_stack_len - current size of local stack
 *	yycheck_stack - local stack
 *	YYCHECK_MAX - max size of local stack
 *	yychar - value of token returned by scanner
 *
 */
STATIC int yycharcheck (int yychar, int yystate, int yyn)
{
   int retval = 1;		/* 1 means good keyword */

   /* At this point in yyparse
    * yylex has been called and the return is in yychar.
    * yystate and yyn have been set for the current state.
    */

   /* get yyparse table lookup value of token */ 
   int yychar1 = YYTRANSLATE (yychar);

   /* test of YYFLAG only needed when yycharcheck is called recursively.
    * YYFLAG is special large negative value.  If value is YYFLAG
    * then do default action (test after yyparse yybackup: label)
    */
   if (yyn != YYFLAG) {
      /* calc new table index for state */
      yyn += yychar1;
   }

   /* check if yyn is a valid table index (YYLAST is table size) */
   if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1) {
      /* no actions for this token in this state - check for default action
       * yydefact is the default action table. (yyparse yydefault: label)
       */
      yyn = yydefact[yystate];
      if (yyn == 0) {
	 /* no default action - keyword is invalid */
	 retval = 0;
      } else {
	 /* Default action is present (always a reduce).
	  * Simulate a reduce and try token again
	  */
	 short stacktop;	/* value at top of state stack */

	 /* number states to pop for this reduce */
	 int yylen = yyr2[yyn];

	 /* first pop off our local stack, then real stack
	  * (using local copy of real stack pointer)
	  */
	 if (yylen > 0) {
	    if (yycheck_stack_len >= yylen) {
	       yycheck_stack_len -= yylen;
	       yylen = 0;
	    } else {
	       yylen -= yycheck_stack_len;
	       yycheck_stack_len = 0;
	    }
	    yyssp_save -= yylen;
	 }

	 /* value at the top of current stack is used to calc next state */
	 stacktop = (yycheck_stack_len) ?
	       yycheck_stack[yycheck_stack_len - 1] :
	       *yyssp_save;

	 /* new value of yyn reduce needed for state calc */
	 yyn = yyr1[yyn];

	 /* new value of state after reduce */
	 yystate = yypgoto[yyn - YYNTBASE] + stacktop;
	 if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == stacktop)
	       yystate = yytable[yystate];
	 else
	       yystate = yydefgoto[yyn - YYNTBASE];

	 /* new value of yyn after reduce
	  * (in yyparse done at yybackup: label
	  * the YYFLAG test that happens there is moved above)
	  */
	 yyn = yypact[yystate];

	 /* push new reduce state onto local stack
	  * and check keyword again in new state
	  * (this happens at the top of yyparse after newstate: label)
	  */
	 if (yycheck_stack_len < YYCHECK_MAX) {
	    yycheck_stack[yycheck_stack_len++] = yystate;
	    /* recursive call */
	    retval = yycharcheck (yychar, yystate, yyn);
	 } else {
	    yyerror ("yycheck stack overflow");
	 }
      }
   } else {
      /* keyword is in table */
      yyn = yytable[yyn];
      /* check if next state/rule is valid
       * positive is state to shift
       * negative (except YYFLAG) is rule to reduce
       */
      if (yyn == 0 || yyn == YYFLAG) {
	 /* invalid keyword */
	 retval = 0;
      }
   }
   return (retval);
}

#else /* YYBISON != 1 */

/* stub for non-bison parser */
EXTERN_C int yykeywordcheck (int tokenvalue)
{
   return (1);
}
#endif /* YYBISON != 1 */
