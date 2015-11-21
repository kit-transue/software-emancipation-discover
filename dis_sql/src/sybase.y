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
/* This parser is part of the "plsql" (dis_sql) executable.  It is
 * used instead of sql.y when the "-database Sybase" flag is given.
 * It is completely separate from the sql.y/sql.l pair that handle
 * Oracle and Informix SQL because it would have been too difficult
 * to integrate this processing with that -- the grammar is quite
 * different (no semicolons in Sybase SQL, the syntax of some
 * commands is different, etc.), and a number of constructs are
 * handled in sql.y by simply turning on an "ignore errors" flag
 * that resyncs on the next semicolon, which obviously woulnd't
 * work for a grammar without semicolons! :-)
 *
 * This file does, however, use a number of utility functions from
 * the sql.y file rather than duplicating them here.  The only
 * support functions defined here are those that are dependent
 * on local variables that are not shared across the application.
 */
%{
#ifndef _WIN32
#include <alloca.h>
#else
#include <malloc.h>
#endif
#include "sql_symbol.h"
#include "sql_scanner.h"
#include "sql_iff.h"
#include "sql_ast.h"
#include "sql_resolve.h"

/* Number of parameters seen so far (used to create "order(N)"
 * attributes in the IF).
 */
static int parameter_count;

/* Flags to indicate which parts of an optional datatype modifier
 * (size and scale in a precision_opt) were specified; set in
 * precision_spec and queried in the various datatypes that might
 * have a precision to create the full datatype information for
 * the IF.  (The values are saved in datatype_size and
 * datatype_scale, if present.)
 */
static int datatype_mask;
static int datatype_size;
static int datatype_scale;

/* Various support routines in sql.y and elsewhere: */

void set_empty(sqltype* result);
ast_tree* new_tree(ast_node_kind ank, SQL_SYM* sym, sqltype first,
		   sqltype last);
int symbol_is_known (sqltype t);
SQL_SYM * symbol_set (sqltype t, sym_kind symbol_type);
ast_tree * symbol_token (sqltype t);
ast_tree * symbol_call_stmt (sqltype *tp, sqltype left,
			     sqltype call_right, sqltype stmt_right);
void set_result (sqltype *result, sqltype left, sqltype right,
		 SQL_SYM * sym, ast_tree * ast);
int is_empty (sqltype t);
char* token_range_text(unsigned first_tok, unsigned last_tok);
ast_tree * symbol_tree_range (SQL_SYM *sym,
			      sqltype tstart,
			      sqltype tend);
ast_tree * empty_tree (ast_node_kind ank);
void ast_defined (ast_tree * at);
void ast_decldefined (ast_tree * at);
void ast_relation_arg (ast_tree * at, int arg);
void ast_relation_attr (ast_tree * at, int reltype);
ast_tree * symbol_tree (SQL_SYM * sym, sqltype t);
void ast_relation (ast_tree * at, relation_kind rk);
SQL_SYM * symbol_extern (sqltype t, sym_kind symbol_type);
SQL_SYM * symbol_add (const char * text, sym_kind symbol_type);
int sql_specfile_output (SQL_SYM * sym, sqltype left, sqltype right);
int sql_tableref (sqltype *tp);
ast_tree * reference_type_tree (ast_node_kind ank,
				SQL_SYM *sym,
				sqltype tstart,
				sqltype tend,
				int refmask);
SQL_SYM * symbol_type (sqltype t);
ast_tree * reference_tree (ast_node_kind ank,
			   SQL_SYM *sym,
			   sqltype tstart,
			   sqltype tend);
SQL_SYM * symbol_type_text (const char * text);
void ast_relation_size (ast_tree * at, int size);
void ast_relation_scale (ast_tree * at, int scale);
ast_loc token_loc_end (sqltype t);
void ast_keyword (ast_tree * at);
const char* token_text (sqltype t);
ast_loc token_loc_start (sqltype t);
int symbol_check (sqltype t, sym_kind symbol_type);
sym_kind symbol_get (sqltype t);
ast_tree * symbol_call_expr (sqltype *tp, sqltype left, sqltype right);
ast_tree * symbol_tree_ast (SQL_SYM *sym,
			    sqltype tstart,
			    sqltype tend);
ast_tree * symbol_reference_tree (SQL_SYM * sym, sqltype t);
ast_tree * symbol_reference (sqltype t);
ast_tree * literal_tree (sqltype t);
ast_tree * sql_call_stmt (const char * funcname,
			  sqltype left,
			  sqltype call_right,
			  sqltype stmt_right);
ast_tree * sql_call_expr (const char * funcname,
			  sqltype left,
			  sqltype right);
ast_tree * string_literal_tree (sqltype t);
void map_eq_to_assign(ast_tree*);

extern const char* last_table_name;

static char txtbuf [1024];		/* temp text buffer */

/* The number of tables seen in a SELECT command.  If there's
 * only one, the column references in clauses after the FROM
 * clause will be resolved automatically to that table;
 * otherwise, the list of tables will be used to resolve
 * column references explicitly.
 */
static int table_count;

/* A flag indicating if we are in the specification section of
 * a SELECT command.  If so, unknown column references are
 * turned into externrefs to be resolved after we've seen the
 * FROM clause; otherwise, they're registered as columns
 * directly in the symbol table.
 */
static int in_select_items;
static ast_tree* table_list;

/* A flag allowing us to distinguish procedure names from table
 * names in GRANT commands; if the privilege is EXECUTE, a name
 * like x.y is a procedure name, while for all the other
 * privileges, x.y is a table name.
 */
static int grant_execute;

/* Sybase SQL does not require that the body of a PROCEDURE be
 * a single statement (simple or compound); instead, it runs
 * until the end of the "batch" (i.e., EOF or the next GO
 * command).  Since there's no syntactic element (BEGIN-END
 * block) under which to collect the statements as a normal
 * part of the syntax processing, we set the following
 * variables and then attach the following top-level
 * statements as children of that node instead of the AST
 * root.
 */
static ast_tree* proc_body;
static ast_tree* pending_proc_body;

/* Support for the token_is_allowed and map_to_allowed_token
 * hack at the end of this file.  That processing requires
 * access to the bison state stack, so we use the following
 * macro definition to make a copy of the current top-of-stack
 * pointer each time the lexer is called (which is exactly the
 * time we need it).  This relies on a slightly-hacked version
 * of bison.simple that only defines YYLEX if it is not
 * already defined.
 */
static short* cur_state_stack;
#define YYLEX (cur_state_stack = yyssp, yylex())
%}
%token string
%token number
%token identifier
%token label_name

%token OP_GE
%token OP_LE
%token OP_NE
%token OP_LEFT_JOIN
%token OP_RIGHT_JOIN
%token Y_ADD
%token Y_ALL
%token Y_ALLOW_DUP_ROW
%token Y_ALTER
%token Y_AND
%token Y_AND_AFTER_BETWEEN
%token Y_ANSINULL
%token Y_ANSI_PERMISSIONS
%token Y_ANY
%token Y_ARITHABORT
%token Y_ARITHIGNORE
%token Y_ARITH_OVERFLOW
%token Y_AS
%token Y_ASC
%token Y_AT
%token Y_AUTHORIZATION
%token Y_AVG
%token Y_BEGIN
%token Y_BETWEEN
%token Y_BINARY
%token Y_BIT
%token Y_BLOCKSIZE
%token Y_BREAK
%token Y_BROWSE
%token Y_BY
%token Y_BYTES
%token Y_CAPACITY
%token Y_CASCADE
%token Y_CASE
%token Y_CHAINED
%token Y_CHAR
%token Y_CHARACTERS
%token Y_CHAR_CONVERT
%token Y_CHECK
%token Y_CHECKALLOC
%token Y_CHECKCATALOG
%token Y_CHECKDB
%token Y_CHECKPOINT
%token Y_CHECKTABLE
%token Y_CLIENT
%token Y_CLOSE
%token Y_CLUSTERED
%token Y_CNTRLTYPE
%token Y_COMMIT
%token Y_COMMITTED
%token Y_COMPUTE
%token Y_CONSTRAINT
%token Y_CONTIGUOUS
%token Y_CONTINUE
%token Y_CONVERT
%token Y_COUNT
%token Y_CREATE
%token Y_CROSS
%token Y_CURRENT
%token Y_CURSOR
%token Y_DATABASE
%token Y_DATEADD
%token Y_DATEDIFF
%token Y_DATEFIRST
%token Y_DATEFORMAT
%token Y_DATENAME
%token Y_DATEPART
%token Y_DATETIME
%token Y_DAY
%token Y_DAYOFYEAR
%token Y_DBCC
%token Y_DBREPAIR
%token Y_DEALLOCATE
%token Y_DECIMAL
%token Y_DECLARE
%token Y_DEFAULT
%token Y_DELAY
%token Y_DELETE
%token Y_DENSITY
%token Y_DESC
%token Y_DISK
%token Y_DISMOUNT
%token Y_DISTINCT
%token Y_DOUBLE
%token Y_DROP
%token Y_DROPDB
%token Y_DUMP
%token Y_DUMPVOLUME
%token Y_ELSE
%token Y_END
%token Y_ENDTRAN
%token Y_ERROR
%token Y_ERRORDATA
%token Y_ERROREXIT
%token Y_ESCAPE
%token Y_EXECUTE
%token Y_EXISTS
%token Y_FAST
%token Y_FETCH
%token Y_FILE
%token Y_FILLFACTOR
%token Y_FIPSFLAGGER
%token Y_FIX
%token Y_FIX_TEXT
%token Y_FLOAT
%token Y_FLUSHMESSAGE
%token Y_FOR
%token Y_FOREIGN
%token Y_FROM
%token Y_FULL
%token Y_GO
%token Y_GOTO
%token Y_GRANT
%token Y_GROUP
%token Y_HAVING
%token Y_HEADERONLY
%token Y_HOLDLOCK
%token Y_HOUR
%token Y_IDENTITY
%token Y_IDENTITY_INSERT
%token Y_IF
%token Y_IGNORE_DUP_KEY
%token Y_IGNORE_DUP_ROW
%token Y_IMAGE
%token Y_IN
%token Y_INDEX
%token Y_INDEXALLOC
%token Y_INIT
%token Y_INNER
%token Y_INSERT
%token Y_INTEGER
%token Y_INTO
%token Y_IO
%token Y_IS
%token Y_ISOLATION
%token Y_JOIN
%token Y_KEY
%token Y_KILL
%token Y_LANGUAGE
%token Y_LEFT
%token Y_LEVEL
%token Y_LIKE
%token Y_LISTONLY
%token Y_LOAD
%token Y_LOG
%token Y_LRU
%token Y_MAX
%token Y_MAX_ROWS_PER_PAGE
%token Y_MILLISECOND
%token Y_MIN
%token Y_MINUTE
%token Y_MIRROR
%token Y_MIRROREXIT
%token Y_MODE
%token Y_MONEY
%token Y_MONTH
%token Y_MRU
%token Y_NAME
%token Y_NATURAL
%token Y_NCHAR
%token Y_NOCOUNT
%token Y_NODISMOUNT
%token Y_NOEXEC
%token Y_NOFIX
%token Y_NOHOLDLOCK
%token Y_NOINIT
%token Y_NONCLUSTERED
%token Y_NOSERIAL
%token Y_NOT
%token Y_NOTIFY
%token Y_NOUNLOAD
%token Y_NOWAIT
%token Y_NO_ERROR
%token Y_NO_LOG
%token Y_NO_TRUNCATE
%token Y_NULL
%token Y_NUMERIC
%token Y_NUMERIC_TRUNCATION
%token Y_NVARCHAR
%token Y_OF
%token Y_OFF
%token Y_OFFSETS
%token Y_ON
%token Y_ONLINE
%token Y_ONLY
%token Y_OPEN
%token Y_OPERATOR_CONSOLE
%token Y_OPTIMIZED
%token Y_OPTION
%token Y_OR
%token Y_ORDER
%token Y_OUTER
%token Y_OUTPUT
%token Y_OVERRIDE
%token Y_PARAM
%token Y_PARSEONLY
%token Y_PARTITION
%token Y_PATINDEX
%token Y_PHYSNAME
%token Y_PRECISION
%token Y_PREFETCH
%token Y_PREPARE
%token Y_PRIMARY
%token Y_PRINT
%token Y_PRIVILEGES
%token Y_PROCEDURE
%token Y_PROCESSEXIT
%token Y_PROCID
%token Y_PUBLIC
%token Y_QUARTER
%token Y_QUOTED_IDENTIFIER
%token Y_RAISERROR
%token Y_READ
%token Y_READTEXT
%token Y_REAL
%token Y_RECOMPILE
%token Y_RECONFIGURE
%token Y_REFERENCES
%token Y_REFIT
%token Y_REINDEX
%token Y_REINIT
%token Y_REMIRROR
%token Y_REMOVE
%token Y_REPLACE
%token Y_RETAIN
%token Y_RETAINDAYS
%token Y_RETURN
%token Y_REVOKE
%token Y_RIGHT
%token Y_ROLE
%token Y_ROLLBACK
%token Y_ROWCOUNT
%token Y_ROWS
%token Y_RULE
%token Y_SAVE
%token Y_SCHEMA
%token Y_SECOND
%token Y_SECONDARY
%token Y_SELECT
%token Y_SELF_RECURSION
%token Y_SERIAL
%token Y_SERIALIZABLE
%token Y_SET
%token Y_SETUSER
%token Y_SHARED
%token Y_SHOWPLAN
%token Y_SHUTDOWN
%token Y_SIZE
%token Y_SKIP_NCINDEX
%token Y_SMALLDATETIME
%token Y_SMALLINT
%token Y_SMALLMONEY
%token Y_SORTED_DATA
%token Y_STATEMENT
%token Y_STATISTICS
%token Y_STRING_RTRUNCATION
%token Y_STRIPE
%token Y_SUBQUERYCACHE
%token Y_SUM
%token Y_SYSNAME
%token Y_TABLE
%token Y_TABLEALLOC
%token Y_TEXT
%token Y_TEXTSIZE
%token Y_THEN
%token Y_TIME
%token Y_TIMESTAMP
%token Y_TINYINT
%token Y_TO
%token Y_TRANSACTION
%token Y_TRIGGER
%token Y_TRUNCATE
%token Y_TRUNCATE_ONLY
%token Y_UNCOMMITTED
%token Y_UNION
%token Y_UNIQUE
%token Y_UNLOAD
%token Y_UNMIRROR
%token Y_UNPARTITION
%token Y_UPDATE
%token Y_USE
%token Y_USER
%token Y_USING
%token Y_VALUES
%token Y_VARBINARY
%token Y_VARCHAR
%token Y_VDEVNO
%token Y_VIEW
%token Y_VSTART
%token Y_WAIT
%token Y_WAITFOR
%token Y_WEEK
%token Y_WEEKDAY
%token Y_WHEN
%token Y_WHERE
%token Y_WHILE
%token Y_WITH
%token Y_WORK
%token Y_WRITES
%token Y_WRITETEXT
%token Y_YEAR

/* The grammar below has a number of SR conflicts.  In addition to
 * the normal expression operator precedence issues, there are
 * things like UPDATE, which can be part of an expression or a
 * statement, causing problems with "RETURN UPDATE" (start of a
 * new statement or part of the return expression), as well as the
 * usual ELSE hack.
 */
%nonassoc LOWER_THAN_BETWEEN
%left Y_OR
%left Y_AND Y_AND_AFTER_BETWEEN
%right Y_NOT
%left Y_EXISTS
%left '=' '>' '<' OP_LE OP_GE OP_NE Y_IN Y_BETWEEN Y_LIKE Y_IS OP_LEFT_JOIN OP_RIGHT_JOIN
%left Y_ANY Y_ALL
%left '+' '-' '&' '|' '^'
%left '*' '/' '%'
%left UMINUS UPLUS '~'
%right Y_UPDATE
%nonassoc HIGHER_THAN_UPDATE
%nonassoc LOWER_THAN_ELSE
%nonassoc Y_ELSE

%%
file:

	  /* empty */
	{
	    set_empty(&$$);
	    ast_root = new_tree(ank_ast_root, NULL, $$, $$);
	}

	| file_component_seq
	{
	    /* If we were in a PROCEDURE body when we got the
	     * EOF, we need to clean up.
	     */
	    if (proc_body) {
		proc_body = NULL;
		sql_pop_scope();
	    }
	}
	;


file_component_seq:

	  statement
	  {
	      ast_root = new_tree(ank_ast_root, NULL, $1, $1);
	      ast_child_tree(ast_root, $1.ast);

	      /* We set pending_proc_body and not proc_body when we
	       * see a PROCEDURE statement; that way the proc itself
	       * will be linked at the top level (as a child of
	       * ast_root), and then proc_body is set so the
	       * contents of the proc will be linked below it.
	       */

	      if (pending_proc_body) {
		  proc_body = pending_proc_body;
		  pending_proc_body = NULL;
	      }
	  }

	| file_component_seq statement
	  {
	      if (proc_body) {
		  /* This statement is part of a procedure */
		  ast_child_tree(proc_body, $2.ast);
	      }
	      else {
		  ast_child_tree(ast_root, $2.ast);
		  if (pending_proc_body) {
		      /* This statement is a procedure; set proc_body
		       * so subsequent statements will be recognized
		       * as belonging to it.
		       */
		      proc_body = pending_proc_body;
		      pending_proc_body = NULL;
		  }
	      }
	  }
	;


statement:

	  alter_database

	| alter_table

	| block

	| begin_transaction

	| break

	| checkpoint

	| close_cursor

	| commit

	| continue

	| create_database

	| create_default

	| create_index

	| create_procedure

	| create_rule

	| create_schema

	| create_table

	| create_trigger

	| create_view

	| dbcc

	| deallocate_cursor

	| declare_variable

	| declare_cursor

	| delete

	| disk_init

	| disk_mirror

	| disk_refit

	| disk_reinit

	| disk_remirror

	| disk_unmirror

	| drop_database

	| drop_default

	| drop_index

	| drop_procedure

	| drop_rule

	| drop_table

	| drop_trigger

	| drop_view

	| dump_database

	| dump_transaction

	| execute

	| fetch

	| go

	| goto

	| grant

	| if

	| insert

	| kill

	| label

	| load_database

	| load_transaction

	| online

	| open

	| prepare

	| print

	| raiserror

	| readtext

	| reconfigure

	| return

	| revoke

	| rollback_trans_or_work

	| rollback_trigger

	| save

	| select

	| set

	| setuser

	| shutdown

	| truncate

	| update_columns

	| update_statistics

	| use

	| waitfor

	| while

	| writetext

	| ':'
	;

/* Many/most of the commands below don't deal with things Discover
 * knows/cares about.  In those cases, we don't build a subtree
 * with AST nodes and symbols; we just do a set_result so that
 * the source mapping is correct and leave it at that.
 *
 * Put another way, symbol references and such are only reported
 * into the IF if they occur in the AST, so nodes are (only)
 * needed for things that need to appear in the IF.
 *
 * Instead of using SQL-specific AST node names, many of the
 * options and statements below are represented by either
 * call_stmt or call_expr nodes, referencing nonexistent
 * functions named according to the option or statement.
 * This (odd) practice reduces the number of changes required to
 * the model-build code in aset.
 */

alter_database:

	  Y_ALTER Y_DATABASE identifier on_device_list_opt 
		log_device_opt with_override_opt for_load_opt
	{
	    sqltype last_tok;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    if (is_empty($5)) {
			if (is_empty($4)) {
			    last_tok = $3;
			}
			else last_tok = $4;
		    }
		    else last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    $$.ast = sql_call_stmt("ALTER DATABASE", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


on_device_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ON on_device_list
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


on_device_list:

	  on_device

	| on_device_list ',' on_device
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


on_device:

	  default_or_device size_opt
	{
	    if (is_empty($2)) {
		set_result(&$$, $1, $1, NULL, NULL);
	    }
	    else set_result(&$$, $1, $2, NULL, NULL);
	}
	;


default_or_device:

	  Y_DEFAULT

	| identifier
	;


size_opt:

	  /* empty */	{ set_empty(&$$); }

	| '=' number
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


log_device_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_LOG Y_ON on_device_list
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


with_override_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_OVERRIDE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


for_load_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_FOR Y_LOAD
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


alter_table:

	  Y_ALTER Y_TABLE
	{
	    sql_push_scope(NULL, NULL);
	}
	table_spec alter_table_variant
	{
	    ast_tree* call_expr;
	    $$.ast = sql_call_stmt("ALTER TABLE", $1, $5, $5);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $4.ast);
	    ast_child_tree(call_expr, $5.ast);
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}
	;


table_spec:

	  identifier '.' owner_opt '.' identifier
	  {
	      if (!symbol_is_known($1)) {
		  $1.sym = symbol_extern($1, sk_database);
	      }
	      if (grant_execute) {
		  $5.sym = symbol_extern($5, sk_procedure);
		  $5.ast = symbol_token($5);
	      }
	      else {
		  sql_tableref(&$5);
		  sql_scope_set_table_name(last_table_name);
	      }
	      ast_extend_start($5.ast, token_loc_start($1));
	      set_result(&$$, $1, $5, $5.sym, $5.ast);
	  }

	| identifier '.' identifier
	  {
	      if (!symbol_is_known($1)) {
		  $1.sym = symbol_extern($1, sk_database);
	      }
	      if (grant_execute) {
		  $3.sym = symbol_extern($3, sk_procedure);
		  $3.ast = symbol_token($3);
	      }
	      else {
		  sql_tableref(&$3);
		  sql_scope_set_table_name(last_table_name);
	      }
	      ast_extend_start($3.ast, token_loc_start($1));
	      set_result(&$$, $1, $3, $3.sym, $3.ast);
	  }

	| identifier
	  {
	      if (grant_execute) {
		  $1.sym = symbol_extern($1, sk_procedure);
		  $1.ast = symbol_token($1);
	      }
	      else {
		  sql_tableref(&$1);
		  sql_scope_set_table_name(last_table_name);
	      }
	      set_result(&$$, $1, $1, $1.sym, $1.ast);
	  }
	;


owner_opt:

	  /* empty */	{ set_empty(&$$); }

	| identifier
	;


alter_table_variant:

	  Y_ADD add_column_list
	{
	    $$.ast = symbol_call_expr(&$1, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| add_constraint

	| Y_DROP Y_CONSTRAINT identifier
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_constraint);
	    }
	    $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($$.ast, symbol_token($3));
	}

	| replace_column

	| Y_PARTITION number
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_UNPARTITION
	;


add_column_list:

	  add_column
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| add_column_list ',' add_column
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}


add_column:

	  identifier datatype Y_DEFAULT expression
		add_column_option_seq_opt
	  {
	      ast_tree* field;
	      sqltype last_tok;
	      if (is_empty($5)) {
		  last_tok = $4;
	      }
	      else last_tok = $5;
	      if (sql_scope_table_name()) {
		  sprintf(txtbuf, "%s.%s", sql_scope_table_name(),
			  token_text($1));
		  $1.sym = sql_symbol_extern(txtbuf, sk_column);
	      }
	      else if (!$1.sym) {
		  $1.sym = symbol_extern($1, sk_column);
	      }
	      $1.ast = symbol_token($1);
	      ast_relation($2.ast, rk_type);
	      $$.ast = new_tree(ank_list_decl, $1.sym, $1, last_tok);
	      ast_child_tree($$.ast, $2.ast);
	      field = reference_type_tree(ank_field_decl, $1.sym,
					  $1, $1, REFMASK_AST);
	      ast_child_tree(field, $1.ast);
	      ast_decldefined(field);
	      ast_child_tree($$.ast, field);
	      ast_defined($$.ast);
	  }

	| identifier datatype add_column_option_seq_opt
	  {
	      ast_tree* field;
	      sqltype last_tok;
	      if (is_empty($3)) {
		  last_tok = $2;
	      }
	      else last_tok = $3;
	      if (sql_scope_table_name()) {
		  sprintf(txtbuf, "%s.%s", sql_scope_table_name(),
			  token_text($1));
		  $1.sym = sql_symbol_extern(txtbuf, sk_column);
	      }
	      else if (!$1.sym) {
		  $1.sym = symbol_extern($1, sk_column);
	      }
	      $1.ast = symbol_token($1);
	      ast_relation($2.ast, rk_type);
	      $$.ast = new_tree(ank_list_decl, $1.sym, $1, last_tok);
	      ast_child_tree($$.ast, $2.ast);
	      field = reference_type_tree(ank_field_decl, $1.sym,
					  $1, $1, REFMASK_AST);
	      ast_child_tree(field, $1.ast);
	      ast_decldefined(field);
	      ast_child_tree($$.ast, field);
	      ast_defined($$.ast);
	  }
	;


default_expr:

	  Y_DEFAULT expression
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


add_column_option_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| add_column_option_seq
	;


add_column_option_seq:

	  add_column_option
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| add_column_option_seq add_column_option
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


add_column_option:

	  identity_null

	| add_column_constraint
	;


identity_null:

	  Y_IDENTITY

	| Y_NULL

	| Y_NOT Y_NULL
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


add_column_constraint:

	  constraint_name_opt key_references_check
	{
	    if (is_empty($1)) {
		set_result(&$$, $2, $2, NULL, $2.ast);
	    }
	    else {
		$$.ast = new_tree(ank_list, NULL, $1, $2);
		ast_child_tree($$.ast, $1.ast);
		ast_child_tree($$.ast, $2.ast);
		set_result(&$$, $1, $2, NULL, $$.ast);
	    }
	}
	;


constraint_name_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_CONSTRAINT identifier
	{
	    if (!symbol_is_known($2)) {
		$2.sym = symbol_extern($2, sk_constraint);
	    }
	    set_result(&$$, $1, $2, NULL, symbol_token($2));
	}
	;


key_references_check:

	  key_spec

	| references_spec

	| check_spec
	;


key_spec:

	  unique_or_primary clustered_spec_opt page_spec_opt
		on_segment_opt
	{
	    sqltype last_tok;
	    if (is_empty($4)) {
		if (is_empty($3)) {
		    if (is_empty($2)) {
			last_tok = $1;
		    }
		    else last_tok = $2;
		}
		else last_tok = $3;
	    }
	    else last_tok = $4;
	    set_result(&$$, $1, last_tok, NULL, NULL);
	}
	;


unique_or_primary:

	  Y_UNIQUE

	| Y_PRIMARY Y_KEY
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


clustered_spec_opt:

	  /* empty */	{ set_empty(&$$); }

	| clustered_spec
	;


clustered_spec:

	  Y_CLUSTERED

	| Y_NONCLUSTERED
	;


page_spec_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH page_spec
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


page_spec:

	  fillfactor_or_max_rows '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


fillfactor_or_max_rows:

	  Y_FILLFACTOR

	| Y_MAX_ROWS_PER_PAGE
	;


on_segment_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ON identifier
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


references_spec:

	  Y_REFERENCES table_spec paren_column_list_opt
	{
	    sqltype last_tok;
	    if (is_empty($3)) {
		last_tok = $2;
	    }
	    else last_tok = $3;
	    $$.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


check_spec:

	  Y_CHECK '(' expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


add_constraint:

	  Y_ADD constraint_name_opt constraint_foreign_check
	{
	    $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


constraint_foreign_check:

	  add_constraint_spec

	| foreign_key_spec

	| check_spec
	;


add_constraint_spec:

	  unique_or_primary clustered_spec_opt
		paren_column_list page_spec_opt 
		on_segment_opt
	{
	    sqltype last_tok;
	    if (is_empty($5)) {
		if (is_empty($4)) {
		    last_tok = $3;
		}
		else last_tok = $4;
	    }
	    else last_tok = $5;
	    set_result(&$$, $1, last_tok, NULL, $3.ast);
	}
	;


column_name_list:

	  column_ref
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| column_name_list ',' column_ref
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


foreign_key_spec:

	  Y_FOREIGN Y_KEY paren_column_list Y_REFERENCES
	{
	    sql_push_scope(NULL, NULL);
	}
	table_spec paren_column_list_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		last_tok = $6;
	    }
	    else last_tok = $7;
	    $$.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($$.ast, $3.ast);
	    ast_child_tree($$.ast, $6.ast);
	    ast_child_tree($$.ast, $7.ast);
	    sql_pop_scope();
	}
	;


replace_column:

	  Y_REPLACE identifier default_expr_opt
	{
	    sqltype last_tok;
	    if (is_empty($3)) {
		last_tok = $2;
	    }
	    else last_tok = $3;
	    if (sql_scope_table_name()) {
		sprintf(txtbuf, "%s.%s", sql_scope_table_name(),
			token_text($2));
		$2.sym = sql_symbol_extern(txtbuf, sk_column);
	    }
	    else if (!$2.sym) {
		$2.sym = symbol_extern($2, sk_column);
	    }
	    $2.ast = symbol_token($2);
	    $$.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


default_expr_opt:

	  /* empty */	{ set_empty(&$$); }

	| default_expr
	;


statement_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| statement_seq
	| prn_statement_seq
	;


statement_seq:

	  statement
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| statement_seq statement
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


prn_statement_seq:   /* sturner, 001214:
                      * statement seq beginning with parenthesized select,
                      * added to handle customer stored procs. */

	  '(' select ')'
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	}

	| prn_statement_seq statement
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


block:

	  Y_BEGIN statement_seq_opt Y_END
	  {
	      $$.ast = new_tree(ank_ast_block, NULL, $1, $3);
	      ast_child_tree($$.ast, $2.ast);
	  }
	;


begin_transaction:

	  Y_BEGIN Y_TRANSACTION transaction_name_opt
	{
	    sqltype last_tok;
	    if (is_empty($3)) {
		last_tok = $2;
	    }
	    else {
		last_tok = $3;
		if (!symbol_is_known($3)) {
		    $3.sym = symbol_set($3, sk_transaction);
		}
		$3.ast = symbol_token($3);
	    }
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


transaction_name_opt:

	  /* empty */	{ set_empty(&$$); }

	| identifier
	;


break:

	  Y_BREAK
	{
	    $$.ast = new_tree(ank_exit_stmt, NULL, $1, $1);
	    ast_keyword($$.ast);
	}
	;


checkpoint:

	  Y_CHECKPOINT
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $1, $1);
	}
	;


close_cursor:

	  Y_CLOSE identifier
	  {
	      if (!symbol_is_known($2)) {
		  $2.sym = symbol_extern($2, sk_cursor);
	      }
	      $2.ast = symbol_token($2);
	      $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	      ast_child_tree($1.ast->head->ast, $2.ast);
	      set_result(&$$, $1, $2, NULL, $$.ast);
	  }
	;


commit:

	  Y_COMMIT trans_or_work_opt identifier_opt
	  {
	      sqltype last_tok;
	      if (is_empty($3)) {
		  if (is_empty($2)) {
		      last_tok = $1;
		  }
		  else last_tok = $2;
	      }
	      else {
		  last_tok = $3;
		  if (!symbol_is_known($3)) {
		      $3.sym = symbol_set($3, sk_transaction);
		  }
		  $3.ast = symbol_token($3);
	      }
	      $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	      ast_child_tree($1.ast, $3.ast);
	      set_result(&$$, $1, last_tok, NULL, $$.ast);
	  }
	;


trans_or_work_opt:

	  /* empty */	{ set_empty(&$$); }

	| trans_or_work
	;


trans_or_work:

	  Y_TRANSACTION

	| Y_WORK
	;



continue:

	  Y_CONTINUE
	;


create_database:

	  Y_CREATE Y_DATABASE identifier on_device_list_opt
		log_device_opt with_override_opt for_load_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    if (is_empty($5)) {
			if (is_empty($4)) {
			    last_tok = $3;
			}
			else last_tok = $4;
		    }
		    else last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $$.ast = sql_call_stmt("CREATE DATABASE", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, symbol_token($3));
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


create_default:

	  Y_CREATE Y_DEFAULT owner_opt_id Y_AS expression
	{
	    $$.ast = sql_call_stmt("CREATE DEFAULT", $1, $5, $5);
	    ast_child_tree($$.ast, $5.ast);
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}
	;


owner_opt_id:

	  identifier

	| identifier '.' identifier
	  {
	      set_result(&$$, $1, $3, NULL, NULL);
	  }
	;


create_index:

	  Y_CREATE unique_opt clustered_spec_opt Y_INDEX
	{
	    sql_push_scope(NULL, NULL);
	}
	identifier Y_ON table_spec paren_column_list create_index_with_opt
		on_segment_opt
	{
	    sqltype last_tok;
	    ast_tree* call_expr;
	    if (is_empty($11)) {
		if (is_empty($10)) {
		    last_tok = $9;
		}
		else last_tok = $10;
	    }
	    else last_tok = $11;
	    $$.ast = sql_call_stmt("CREATE INDEX", $1, last_tok, last_tok);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $8.ast);
	    ast_child_tree(call_expr, $9.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


unique_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_UNIQUE
	;


create_index_with_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH create_index_with_list
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


create_index_with_list:

	  create_index_with_spec

	| create_index_with_list ',' create_index_with_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


create_index_with_spec:

	  page_spec

	| Y_IGNORE_DUP_KEY

	| Y_SORTED_DATA

	| dup_row_spec
	;


dup_row_spec:

	  Y_IGNORE_DUP_ROW

	| Y_ALLOW_DUP_ROW
	;


create_procedure:

	  Y_CREATE Y_PROCEDURE owner_opt_id proc_subid_opt
	  {
	      char* proc_name = token_range_text($3.end_token,
						 ((is_empty($4)) ?
						  $3.end_token :
						  $4.end_token));
	      $3.sym = sql_symbol_extern(proc_name, sk_procedure);
	      sql_push_scope(proc_name, $3.sym);
	      parameter_count = 0;
	      free(proc_name);
	  }
	  parameter_list_opt recompile_opt Y_AS statement
	  {
	      ast_tree* at_decl;
	      ast_tree* at_body;
	      ast_tree* declspec;
	      sqltype end_of_declarator;
	      sqltype empty_declspec;
	      if (is_empty($6)) {
		  if (is_empty($4)) {
		      end_of_declarator = $3;
		  }
		  else end_of_declarator = $4;
	      }
	      else end_of_declarator = $6;
	      $$.ast = new_tree(ank_function_def, $3.sym, $1, $9);
	      set_empty(&empty_declspec);
	      empty_declspec.sym = symbol_type_text("INTEGER");
	      empty_declspec.ast = reference_tree(ank_ast_declspec,
						  empty_declspec.sym,
						  empty_declspec,
						  empty_declspec);
	      ast_keyword(empty_declspec.ast);
	      ast_relation(empty_declspec.ast, rk_type);
	      ast_child_tree($$.ast, empty_declspec.ast);
	      at_decl = new_tree(ank_function_decl, $3.sym, $1,
				 end_of_declarator);
	      if (is_empty($4)) {
		  ast_child_tree(at_decl, symbol_token($3));
	      }
	      else ast_child_tree(at_decl,
				  symbol_tree_range($3.sym, $3, $4));
	      ast_child_tree(at_decl, $6.ast);
	      ast_decldefined(at_decl);
	      ast_child_tree($$.ast, at_decl);
	      ast_child_tree($$.ast, $9.ast);
	      ast_defined($$.ast);
	      set_result(&$$, $1, $9, NULL, $$.ast);
	      pending_proc_body = $$.ast;
	  }
	;


proc_subid_opt:

	  /* empty */	{ set_empty(&$$); }

	| ';' number
	  {
	      set_result(&$$, $1, $2, NULL, NULL);
	  }
	;


parameter_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| '(' parameter_list ')'
	  {
	      set_result(&$$, $1, $3, NULL, $2.ast);
	  }

	| parameter_list
	;


parameter_list:

	  parameter_decl
	  {
	      $$.ast = new_tree(ank_list, NULL, $1, $1);
	      ast_child_tree($$.ast, $1.ast);
	  }

	| parameter_list ',' parameter_decl
	  {
	      ast_child_tree($1.ast, $3.ast);
	      set_result(&$$, $1, $3, NULL, $1.ast);
	  }
	;


parameter_decl:

	  '@' identifier datatype default_arg_opt
		output_opt
	  {
	      sqltype end_of_declarator;
	      char* txt = token_range_text($1.start_token, $2.end_token);
	      if (is_empty($5)) {
		  if (is_empty($4)) {
		      end_of_declarator = $3;
		  }
		  else end_of_declarator = $4;
	      }
	      else end_of_declarator = $5;
	      ast_relation($3.ast, rk_type);
	      $$.sym = symbol_add(txt, sk_variable);
	      $$.ast = new_tree(ank_parm_decl, $$.sym, $1,
				end_of_declarator);
	      ast_child_tree($$.ast, $3.ast);
	      ast_child_tree($$.ast, symbol_tree_range($$.sym, $1, $2));
	      ast_child_tree($$.ast, $4.ast);
	      ast_relation($$.ast, rk_arg);
	      ast_relation_arg($$.ast, ++parameter_count);
	      if (!is_empty($5)) {
		  ast_relation_attr($$.ast, RELMASK_OUTPUT);
	      }
	      ast_defined($$.ast);
	      ast_decldefined($$.ast);
	      free(txt);
	  }
	;


default_arg_opt:

	  /* empty */	{ set_empty(&$$); }

	| '=' expression
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


output_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_OUTPUT
	;


recompile_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_RECOMPILE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


create_rule:

	  Y_CREATE Y_RULE owner_opt_id Y_AS expression
	{
	    $$.ast = sql_call_stmt("CREATE RULE", $1, $5, $5);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}
	;


create_schema:

	  Y_CREATE Y_SCHEMA Y_AUTHORIZATION identifier
		/* substatements handled by statement_seq action */
	{
	    $$.ast = sql_call_stmt("CREATE SCHEMA", $1, $4, $4);
	    set_result(&$$, $1, $4, NULL, $$.ast);
	}
	;


create_table:

	  Y_CREATE Y_TABLE 
	  {
	      sql_push_scope(NULL, NULL);
	  }
	table_spec paren_col_constr_list page_spec_opt on_segment_opt
	  {
	      ast_tree* block;
	      sqltype last_tok;
	      if (is_empty($7)) {
		  if (is_empty($6)) {
		      last_tok = $5;
		  }
		  else last_tok = $6;
	      }
	      else last_tok = $7;
	      $$.ast = new_tree(ank_type_decl, $4.sym, $1, last_tok);
	      ast_child_tree($$.ast, $4.ast);
	      block = new_tree(ank_ast_block, NULL, $5, $5);
	      ast_child_tree(block, $5.ast);
	      ast_child_tree($$.ast, block);
	      ast_relation($5.ast, rk_context);
	      ast_defined($$.ast);
	      ast_decldefined($$.ast);
	      sql_specfile_output($4.sym, $1, last_tok);
	      sql_pop_scope();
	      set_result(&$$, $1, last_tok, NULL, $$.ast);
	  }
	;


paren_col_constr_list:

	  '(' column_or_constraint_list ')'
	  {
	      set_result(&$$, $1, $3, NULL, $2.ast);
	  }

	| '(' column_or_constraint_list ',' ')'
	  {
	      set_result(&$$, $1, $4, NULL, $2.ast);
	  }
	;


column_or_constraint_list:

	  column_or_constraint
	  {
	      $$.ast = new_tree(ank_list, NULL, $1, $1);
	      ast_child_tree($$.ast, $1.ast);
	  }

	| column_or_constraint_list ',' column_or_constraint
	  {
	      ast_child_tree($1.ast, $3.ast);
	      set_result(&$$, $1, $3, NULL, $1.ast);
	  }
	;


column_or_constraint:

	  add_column

	| constraint_name_opt constraint_foreign_check
	{
	    if (is_empty($1)) {
		set_result(&$$, $2, $2, NULL, $2.ast);
	    }
	    else {
		$$.ast = new_tree(ank_list, NULL, $1, $2);
		ast_child_tree($$.ast, $1.ast);
		ast_child_tree($$.ast, $2.ast);
		set_result(&$$, $1, $2, NULL, $$.ast);
	    }
	}
	;


create_trigger:

	  Y_CREATE Y_TRIGGER owner_opt_id Y_ON
		owner_opt_id for_ins_upd_del Y_AS
		statement
	{
	    $$.ast = sql_call_stmt("CREATE TRIGGER", $1, $8, $8);
	    ast_child_tree($$.ast->head->ast, $8.ast);
	}
	;


for_ins_upd_del:

	  Y_FOR ins_upd_del_list
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


ins_upd_del_list:

	  ins_upd_del

	| ins_upd_del_list ',' ins_upd_del
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


ins_upd_del:

	  Y_INSERT

	| Y_UPDATE

	| Y_DELETE
	;


view_spec:

	  identifier '.' identifier
	  {
	      if (grant_execute) {
		  $3.sym = symbol_extern($3, sk_procedure);
		  $3.ast = symbol_token($3);
	      }
	      else {
		  sql_tableref(&$3);
		  sql_scope_set_table_name(last_table_name);
	      }
	      ast_extend_start($3.ast, token_loc_start($1));
	      set_result(&$$, $1, $3, $3.sym, $3.ast);
	  }

	| identifier
	  {
	      if (grant_execute) {
		  $1.sym = symbol_extern($1, sk_procedure);
		  $1.ast = symbol_token($1);
	      }
	      else {
		  sql_tableref(&$1);
		  sql_scope_set_table_name(last_table_name);
	      }
	      set_result(&$$, $1, $1, $1.sym, $1.ast);
	  }
	;


create_view:

	  Y_CREATE Y_VIEW
	{
	    sql_push_scope(NULL, NULL);
	}
	  view_spec
		paren_column_list_opt Y_AS select with_check_opt
	{
	    ast_tree * at_block;
	    sqltype tblock;
	    sqltype last_tok;

	    if (is_empty ($5)) {
	       tblock = $6;
	    } else {
	       tblock = $5;
	    }
	    if (is_empty($8)) {
		last_tok = $7;
	    }
	    else last_tok = $8;
	    $$.ast = new_tree (ank_type_decl, $4.sym, $1, last_tok);
	    ast_child_tree ($$.ast, $4.ast);

	    at_block = new_tree (ank_ast_block, 0, tblock, $7);
	    ast_child_tree (at_block, $5.ast);
	    ast_child_tree (at_block, $7.ast);
	    ast_child_tree ($$.ast, at_block);

	    ast_defined ($$.ast);
	    ast_decldefined ($$.ast);
	    sql_pop_scope ();
	    set_result (&$$, $4, last_tok, 0, $$.ast);
	}
	;


with_check_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_CHECK Y_OPTION
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


dbcc:

	  Y_DBCC dbcc_action
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    ast_child_tree($1.ast, $2.ast);
	}
	;


dbcc_action:

	  checktable

	| checkdb

	| checkalloc

	| tablealloc

	| indexalloc

	| checkcatalog

	| dbrepair

	| reindex

	| fix_text
	;


checktable:

	  Y_CHECKTABLE '(' table_name_or_id skip_ncindex_opt ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $5);
	    ast_child_tree($$.ast, $3.ast);
	}
	;

table_name_or_id:

	  table_spec

	| number
	;


skip_ncindex_opt:

	  /* empty */	{ set_empty(&$$); }

	| ',' Y_SKIP_NCINDEX
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


checkdb:

	  Y_CHECKDB checkdb_arg_opt
	{
	    $$.ast = symbol_call_expr(&$1, $1, $2);
	    if (!is_empty($2)) {
		ast_child_tree($$.ast, $2.ast);
	    }
	}
	;


checkdb_arg_opt:

	  /* empty */	{ set_empty(&$$); }

	| '(' identifier skip_ncindex_opt ')'
	{
	    if (!symbol_is_known($2)) {
		$2.sym = symbol_extern($2, sk_database);
	    }
	    $$.ast = symbol_token($2);
	    set_result(&$$, $1, $4, NULL, $$.ast);
	}
	;


checkalloc:

	  Y_CHECKALLOC db_fix_opt
	{
	    sqltype last_tok;
	    if (is_empty($2)) {
		last_tok = $1;
	    }
	    else last_tok = $2;
	    $$.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($$.ast, $2.ast);
	}
	;


db_fix_opt:

	  /* empty */	{ set_empty(&$$); }

	| '(' identifier fix_nofix_opt ')'
	{
	    if (!symbol_is_known($2)) {
		$2.sym = symbol_extern($2, sk_database);
	    }
	    $$.ast = symbol_token($2);
	    set_result(&$$, $1, $4, NULL, $$.ast);
	}
	;


fix_nofix_opt:

	  /* empty */	{ set_empty(&$$); }

	| ',' Y_FIX
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| ',' Y_NOFIX
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


tablealloc:

	  Y_TABLEALLOC '(' table_name_or_id tablealloc_options ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $5);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


tablealloc_options:

	  fix_nofix_opt

	| ',' Y_FULL fix_nofix_opt
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| ',' Y_OPTIMIZED fix_nofix_opt
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| ',' Y_FAST fix_nofix_opt
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| ',' Y_NULL fix_nofix_opt
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


indexalloc:

	  Y_INDEXALLOC '(' table_name_or_id ',' number
		tablealloc_options ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $7);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


checkcatalog:

	  Y_CHECKCATALOG
	{
	    $$.ast = symbol_call_expr(&$1, $1, $1);
	}

	| Y_CHECKCATALOG '(' identifier ')'
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, symbol_token($3));
	}
	;


dbrepair:

	  Y_DBREPAIR '(' identifier ',' Y_DROPDB ')'
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast, symbol_token($3));
	}
	;


reindex:

	  Y_REINDEX '(' table_name_or_id ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


fix_text:

	  Y_FIX_TEXT '(' table_name_or_id ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


deallocate_cursor:

	  Y_DEALLOCATE Y_CURSOR identifier
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_cursor);
	    }
	    $3.ast = symbol_token($2);
	    $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	    ast_child_tree($1.ast, symbol_token($3));
	}
	;


declare_variable:

	  Y_DECLARE variable_list
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


variable_list:

	  '@' identifier datatype
	{
	    char* txt = token_range_text($1.start_token, $2.end_token);
	    ast_tree* decl;
	    ast_tree* list_decl;
	    ast_relation($3.ast, rk_type);
	    $2.sym = symbol_add(txt, sk_variable);
	    list_decl = new_tree(ank_list_decl, $2.sym, $1, $3);
	    decl = reference_type_tree(ank_var_decl, $2.sym, $1, $2,
				       REFMASK_AST);
	    ast_child_tree(list_decl, $3.ast);
	    ast_child_tree(decl, symbol_tree_range($2.sym, $1, $2));
	    ast_defined(decl);
	    ast_decldefined(decl);
	    ast_child_tree(list_decl, decl);
	    $$.ast = new_tree(ank_list, NULL, $1, $3);
	    ast_child_tree($$.ast, list_decl);
	    free(txt);
	}

	| variable_list ',' '@' identifier datatype
	{
	    char* txt = token_range_text($3.start_token, $4.end_token);
	    ast_tree* decl;
	    ast_tree* list_decl;
	    ast_relation($5.ast, rk_type);
	    $4.sym = symbol_add(txt, sk_variable);
	    list_decl = new_tree(ank_list_decl, $4.sym, $3, $5);
	    decl = reference_type_tree(ank_var_decl, $4.sym, $3, $4,
				       REFMASK_AST);
	    ast_child_tree(list_decl, $5.ast);
	    ast_child_tree(decl, symbol_tree_range($4.sym, $3, $4));
	    ast_defined(decl);
	    ast_decldefined(decl);
	    ast_child_tree(list_decl, decl);
	    ast_child_tree($1.ast, list_decl);
	    set_result(&$$, $1, $5, NULL, $1.ast);
	    free(txt);
	}
	;


declare_cursor:

	  Y_DECLARE identifier Y_CURSOR
	{
	    char* name = NULL;
	    $2.sym = symbol_set($2, sk_cursor);
	    if ($2.sym) {
		name = $2.sym->sym_text;
	    }
	    sql_push_scope(name, NULL);
	}
	Y_FOR select
	{
	    ast_tree* decl;
	    decl = new_tree(ank_cursor_decl, $2.sym, $1, $3);
	    ast_child_tree(decl, symbol_token($2));
	    $$.ast = new_tree(ank_cursor_def, $2.sym, $1, $6);
	    ast_child_tree($$.ast, empty_tree(ank_ast_declspec));
	    ast_child_tree($$.ast, decl);
	    ast_child_tree($$.ast, $6.ast);
	    ast_defined($$.ast);
	    ast_decldefined(decl);
	    sql_pop_scope();
	    set_result(&$$, $1, $6, NULL, $$.ast);
	}
	;


read_only_or_update:

	  Y_READ Y_ONLY
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_UPDATE

	| Y_UPDATE Y_OF column_name_list
	{
	    set_result(&$$, $1, $3, NULL, $3.ast);
	}
	;


delete:

	  Y_DELETE
	{
	    sql_push_scope(NULL, NULL);
	}
	from_opt table_spec delete_from_list_opt where_opt
	{
	    sqltype last_tok;
	    if (is_empty($6)) {
		if (is_empty($5)) {
		    last_tok = $4;
		}
		else last_tok = $5;
	    }
	    else last_tok = $6;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $5.ast);
	    ast_child_tree($1.ast, $6.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


from_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_FROM
	;


delete_from_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_FROM delete_from_list
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


delete_from_list:

	  delete_from
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| delete_from_list ',' delete_from
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


delete_from:

	  table_spec identifier_opt index_prefetch_queuing_opt
		join_seq_opt
	{
	    sqltype last_tok;
	    if (is_empty($4)) {
		if (is_empty($3)) {
		    if (is_empty($2)) {
			last_tok = $1;
		    }
		    else last_tok = $2;
		}
		else last_tok = $3;
		set_result(&$$, $1, last_tok, NULL, $1.ast);
	    }
	    else {
		last_tok = $4;
		$$.ast = new_tree(ank_list, NULL, $1, $4);
		ast_child_tree($$.ast, $1.ast);
		ast_child_tree($$.ast, $4.ast);
	    }
	}
	;


index_prefetch_queuing_opt:

	  /* empty */	{ set_empty(&$$); }

	| '(' Y_INDEX identifier prefetch_opt queuing_opt ')'
	{
	    set_result(&$$, $1, $6, NULL, NULL);
	}
	;


prefetch_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_PREFETCH number
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


queuing_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_LRU

	| Y_MRU
	;


where_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WHERE expression
	{
	    $$.ast = new_tree(ank_where, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| Y_WHERE Y_CURRENT Y_OF identifier
	{
	    if (!symbol_is_known($4)) {
		$4.sym = symbol_extern($4, sk_cursor);
	    }
	    $$.ast - new_tree(ank_where, NULL, $1, $4);
	    ast_child_tree($$.ast, symbol_token($4));
	}
	;


disk_init:

	  Y_DISK Y_INIT disk_init_arg_list
	{
	    $$.ast = sql_call_stmt("DISK INIT", $1, $3, $3);
	}
	;


disk_init_arg_list:

	  disk_init_arg

	| disk_init_arg_list ',' disk_init_arg
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


disk_init_arg:

	  Y_NAME '=' string 
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_PHYSNAME '=' string
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_VDEVNO '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_SIZE '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_VSTART '=' number ',' Y_CNTRLTYPE '=' number
	{
	    set_result(&$$, $1, $7, NULL, NULL);
	}

	| Y_CONTIGUOUS
	;


disk_mirror:

	  Y_DISK Y_MIRROR Y_NAME '=' string ',' Y_MIRROR '=' string
		mirror_options_list_opt
	{
	    $$.ast = sql_call_stmt("DISK MIRROR", $1, $10, $10);
	}
	;


mirror_options_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| mirror_options_list
	;


mirror_options_list:

	  mirror_option

	| mirror_options_list mirror_option
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


mirror_option:

	  ',' Y_WRITES '=' Y_SERIAL
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| ',' Y_WRITES '=' Y_NOSERIAL
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| ',' Y_CONTIGUOUS
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


disk_refit:

	  Y_DISK Y_REFIT
	{
	    $$.ast = sql_call_stmt("DISK REFIT", $1, $2, $2);
	}
	;


disk_reinit:

	  Y_DISK Y_REINIT disk_init_arg_list
	{
	    $$.ast = sql_call_stmt("DISK REINIT", $1, $3, $3);
	}
	;


disk_remirror:

	  Y_DISK Y_REMIRROR Y_NAME '=' string
	{
	    $$.ast = sql_call_stmt("DISK REMIRROR", $1, $5, $5);
	}
	;


disk_unmirror:

	  Y_DISK Y_UNMIRROR Y_NAME '=' string unmirror_options_seq_opt
	{
	    $$.ast = sql_call_stmt("DISK UNMIRROR", $1, $6, $6);
	}
	;


unmirror_options_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| unmirror_options_seq
	;


unmirror_options_seq:

	  unmirror_option

	| unmirror_options_seq unmirror_option
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


unmirror_option:

	  ',' Y_SIZE '=' Y_PRIMARY
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| ',' Y_SIZE '=' Y_SECONDARY
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| ',' Y_MODE '=' Y_RETAIN
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| ',' Y_MODE '=' Y_REMOVE
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}
	;


drop_database:

	  Y_DROP Y_DATABASE database_name_list
	{
	    $$.ast = sql_call_stmt("DROP DATABASE", $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


database_name_list:

	  identifier
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    if (!symbol_is_known($1)) {
		$1.sym = symbol_extern($1, sk_database);
	    }
	    ast_child_tree($$.ast, symbol_token($1));
	}

	| database_name_list ',' identifier
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    ast_child_tree($1.ast, symbol_token($3));
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


drop_default:

	  Y_DROP Y_DEFAULT default_name_list
	{
	    $$.ast = sql_call_stmt("DROP DEFAULT", $1, $3, $3);
	}
	;


default_name_list:

	  owner_opt_id

	| default_name_list ',' owner_opt_id
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


drop_index:

	  Y_DROP Y_INDEX index_name_list
	{
	    $$.ast = sql_call_stmt("DROP INDEX", $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


index_name_list:

	  identifier '.' identifier
	{
	    sprintf(txtbuf, "%s.%s", token_text($1),
		    token_text($3));
	    $3.sym = sql_symbol_extern(txtbuf, sk_column);
	    $3.ast = symbol_token($3);
	    ast_extend_start($3.ast, token_loc_start($1));
	    $$.ast = new_tree(ank_list, NULL, $1, $3);
	    ast_child_tree($1.ast, $3.ast);
	}

	| index_name_list ',' identifier '.' identifier
	{
	    sprintf(txtbuf, "%s.%s", token_text($3),
		    token_text($5));
	    $5.sym = sql_symbol_extern(txtbuf, sk_column);
	    $5.ast = symbol_token($5);
	    ast_extend_start($5.ast, token_loc_start($3));
	    ast_child_tree($1.ast, $5.ast);
	    set_result(&$$, $1, $5, NULL, $1.ast);
	}
	;


drop_procedure:

	  Y_DROP Y_PROCEDURE procedure_name_list
	{
	    $$.ast = sql_call_stmt("DROP PROCEDURE", $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


procedure_name_list:

	  owner_opt_id
	{
	    char* proc_name = token_range_text($1.end_token,
					       $1.end_token);
	    $1.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $1.ast = symbol_token($1);
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	    free(proc_name);
	}

	| procedure_name_list ',' owner_opt_id
	{
	    char* proc_name = token_range_text($3.end_token,
					       $3.end_token);
	    $3.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $3.ast = symbol_token($3);
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


drop_rule:

	  Y_DROP Y_RULE rule_name_list
	{
	    $$.ast = sql_call_stmt("DROP RULE", $1, $3, $3);
	}
	;


rule_name_list:

	  owner_opt_id

	| rule_name_list ',' owner_opt_id
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


drop_table:

	  Y_DROP Y_TABLE table_spec_list
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $$.ast);
	}
	;


table_spec_list:

	  table_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| table_spec_list ',' table_spec
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


drop_trigger:

	  Y_DROP Y_TRIGGER trigger_name_list
	{
	    $$.ast = sql_call_stmt("DROP TRIGGER", $1, $3, $3);
	}
	;


trigger_name_list:

	  owner_opt_id

	| trigger_name_list ',' owner_opt_id
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


drop_view:

	  Y_DROP Y_VIEW view_name_list
	{
	    $$.ast = sql_call_stmt("DROP VIEW", $1, $3, $3);
	}
	;


view_name_list:

	  owner_opt_id

	| view_name_list ',' owner_opt_id
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


dump_database:

	  Y_DUMP Y_DATABASE identifier Y_TO stripe_dev_spec
		stripe_on_seq_opt dump_with_list_opt
	{
	    sqltype last_tok;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    $$.ast = sql_call_stmt("DUMP DATABASE", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


stripe_dev_spec:

	  string at_spec_opt file_spec_opt
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


at_spec_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_AT identifier
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


file_spec_opt:

	  /* empty */	{ set_empty(&$$); }

	| file_spec_list
	;


file_spec_list:

	  file_spec

	| file_spec_list ',' file_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


file_spec:

	  Y_DENSITY '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_BLOCKSIZE '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_CAPACITY '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_DUMPVOLUME '=' string 
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_FILE '=' string
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


stripe_on_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| stripe_on_seq
	;


stripe_on_seq:

	  stripe_on_spec

	| stripe_on_seq stripe_on_spec
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


stripe_on_spec:

	  Y_STRIPE Y_ON stripe_dev_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


dump_with_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH dump_with_list
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


dump_with_list:

	  dump_with_spec

	| dump_with_list ',' dump_with_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


dump_with_spec:

	  file_spec

	| Y_DISMOUNT

	| Y_NODISMOUNT

	| Y_NOUNLOAD

	| Y_UNLOAD

	| Y_RETAINDAYS '=' number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_NOINIT

	| Y_INIT

	| Y_NO_TRUNCATE

	| Y_TRUNCATE_ONLY

	| Y_NO_LOG

	| Y_LISTONLY

	| Y_LISTONLY '=' Y_FULL
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_HEADERONLY

	| Y_NOTIFY '=' Y_CLIENT
	{
	    set_result(&$$, $1, $3, NULL,  NULL);
	}

	| Y_NOTIFY '=' Y_OPERATOR_CONSOLE
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


dump_transaction:

	  Y_DUMP Y_TRANSACTION identifier Y_TO stripe_dev_spec
		stripe_on_seq_opt dump_with_list_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    $$.ast = sql_call_stmt("DUMP TRANSACTION", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


execute:

	  Y_EXECUTE retval_opt exec_proc_name_spec %prec HIGHER_THAN_UPDATE
	{
	    ast_tree* call_expr;
	    $$.ast = new_tree(ank_expr_stmt, NULL, $1, $3);
	    call_expr = new_tree(ank_call_expr, $3.sym, $1, $3);
	    ast_child_tree(call_expr, $3.ast);
	    if (is_empty($2)) {
		ast_child_tree($$.ast, call_expr);
	    }
	    else {
		ast_tree* assign_expr = new_tree(ank_assign_expr, NULL,
						 $1, $3);
		ast_child_tree(assign_expr, $2.ast);
		ast_child_tree(assign_expr, call_expr);
		ast_child_tree($$.ast, assign_expr);
	    }
	}

	| Y_EXECUTE retval_opt exec_proc_name_spec arg_list
	{
	    ast_tree* call_expr;
	    $$.ast = new_tree(ank_expr_stmt, NULL, $1, $4);
	    call_expr = new_tree(ank_call_expr, $3.sym, $1, $4);
	    ast_child_tree(call_expr, $3.ast);
	    ast_child_tree(call_expr, $4.ast);
	    if (is_empty($2)) {
		ast_child_tree($$.ast, call_expr);
	    }
	    else {
		ast_tree* assign_expr = new_tree(ank_assign_expr, NULL,
						$1, $4);
		ast_child_tree(assign_expr, $2.ast);
		ast_child_tree(assign_expr, call_expr);
		ast_child_tree($$.ast, assign_expr);
	    }
	}

	| Y_EXECUTE retval_opt exec_proc_name_spec
		arg_list_opt Y_WITH Y_RECOMPILE
	{
	    ast_tree* call_expr;
	    $$.ast = new_tree(ank_expr_stmt, NULL, $1, $6);
	    call_expr = new_tree(ank_call_expr, $3.sym, $1, $6);
	    ast_child_tree(call_expr, $3.ast);
	    ast_child_tree(call_expr, $4.ast);
	    if (is_empty($2)) {
		ast_child_tree($$.ast, call_expr);
	    }
	    else {
		ast_tree* assign_expr = new_tree(ank_assign_expr, NULL,
						 $1, $6);
		ast_child_tree(assign_expr, $2.ast);
		ast_child_tree(assign_expr, call_expr);
		ast_child_tree($$.ast, assign_expr);
	    }
	}

	| Y_EXECUTE '(' expression ')'
	{
	    set_result(&$$, $1, $4, NULL, $3.ast);
	}
	;


retval_opt:

	  /* empty */	{ set_empty(&$$); }

	| '@' identifier '='
	{
	    char* txt = token_range_text($1.start_token, $2.end_token);
	    $2.sym = sql_symbol_lookup(txt);
	    $2.ast = symbol_tree_range($2.sym, $1, $2);
	    set_result(&$$, $1, $3, $2.sym, $2.ast);
	    free(txt);
	}
	;


exec_proc_name_spec:

	  identifier '.' identifier '.' identifier '.' identifier
		proc_subid_opt
	{
	    char* proc_name = token_range_text($7.end_token,
					       ((is_empty($8)) ?
						$7.end_token :
						$8.end_token));
	    $7.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $7.ast = symbol_token($7);
	    if (is_empty($8)) {
		set_result(&$$, $1, $7, $7.sym, $7.ast);
	    }
	    else set_result(&$$, $1, $8, $7.sym, $7.ast);
	    free(proc_name);
	}

	| identifier '.' identifier '.' '.' identifier proc_subid_opt
	{
	    char* proc_name = token_range_text($6.end_token,
					       ((is_empty($7)) ?
						$6.end_token :
						$7.end_token));
	    $6.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $6.ast = symbol_token($6);
	    if (is_empty($7)) {
		set_result(&$$, $1, $6, $6.sym, $6.ast);
	    }
	    else set_result(&$$, $1, $7, $6.sym, $6.ast);
	    free(proc_name);
	}

	| identifier '.' identifier '.' identifier proc_subid_opt
	{
	    char* proc_name = token_range_text($5.end_token,
					       ((is_empty($6)) ?
						$5.end_token :
						$6.end_token));
	    $5.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $5.ast = symbol_token($5);
	    if (is_empty($6)) {
		set_result(&$$, $1, $5, $5.sym, $5.ast);
	    }
	    else set_result(&$$, $1, $6, $5.sym, $5.ast);
	    free(proc_name);
	}

	| identifier '.' '.' identifier proc_subid_opt
	{
	    char* proc_name = token_range_text($4.end_token,
					       ((is_empty($5)) ?
						$4.end_token :
						$5.end_token));
	    $4.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $4.ast = symbol_token($4);
	    if (is_empty($5)) {
		set_result(&$$, $1, $4, $4.sym, $4.ast);
	    }
	    else set_result(&$$, $1, $4, $4.sym, $4.ast);
	    free(proc_name);
	}

	| identifier '.' identifier proc_subid_opt
	{
	    char* proc_name = token_range_text($3.end_token,
					       ((is_empty($4)) ?
						$3.end_token :
						$4.end_token));
	    $3.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $3.ast = symbol_token($3);
	    if (is_empty($4)) {
		set_result(&$$, $1, $3, $3.sym, $3.ast);
	    }
	    else set_result(&$$, $1, $4, $3.sym, $3.ast);
	    free(proc_name);
	}

	| identifier proc_subid_opt
	{
	    char* proc_name = token_range_text($1.end_token,
					       ((is_empty($2)) ?
						$1.end_token :
						$2.end_token));
	    $1.sym = sql_symbol_extern(proc_name, sk_procedure);
	    $1.ast = symbol_token($1);
	    if (is_empty($2)) {
		set_result(&$$, $1, $1, $1.sym, $1.ast);
	    }
	    else set_result(&$$, $1, $2, $1.sym, $1.ast);
	    free(proc_name);
	}
	;


arg_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| arg_list
	;


arg_list:

	  arg_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| arg_list ',' arg_spec
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


arg_spec:

	  expression output_opt
	{
	    if (is_empty($2)) {
		set_result(&$$, $1, $1, NULL, $1.ast);
	    }
	    else set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


fetch:

	  Y_FETCH identifier fetch_into_opt
	{
	    sqltype last_tok;
	    if (is_empty($3)) {
		last_tok = $2;
	    }
	    else last_tok = $3;
	    if (!symbol_is_known($1)) {
		$2.sym = symbol_set($2, sk_cursor);
	    }
	    $2.ast = symbol_token($2);
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($1.ast, $2.ast);
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


fetch_into_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_INTO variable_name_list
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


variable_name_list:

	  '@' identifier
	{
	    char* txt = token_range_text($1.start_token, $2.end_token);
	    $2.sym = sql_symbol_lookup(txt);
	    $2.ast = symbol_tree_range($2.sym, $1, $2);
	    $$.ast = new_tree(ank_list, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	    free(txt);
	}

	| variable_name_list ',' '@' identifier
	{
	    char* txt = token_range_text($3.start_token, $4.end_token);
	    $4.sym = sql_symbol_lookup(txt);
	    $4.ast = symbol_tree_range($4.sym, $3, $4);
	    ast_child_tree($1.ast, $4.ast);
	    set_result(&$$, $1, $4, NULL, $1.ast);
	    free(txt);
	}
	;


go:

	  Y_GO
	{
	    if (proc_body) {
		proc_body = NULL;
		sql_pop_scope();
	    }
	    pending_proc_body = NULL;
	    $$.ast = symbol_call_stmt(&$1, $1, $1, $1);
	}
	;


goto:

	  Y_GOTO identifier
	{
	    if (symbol_get($2) != sk_label) {
		$2.sym = symbol_set($2, sk_label);
	    }
	    $2.ast = symbol_token($2);
	    $$.ast = new_tree(ank_goto_stmt, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $$.ast);
	}
	;


grant:

	  Y_GRANT privilege_spec grant_on_opt grantee_spec with_grant_opt
	{
	    sqltype last_tok;
	    if (is_empty($5)) {
		last_tok = $4;
	    }
	    else last_tok = $5;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	    grant_execute = 0;
	}
	;


privilege_spec:

	  Y_ALL

	| Y_ALL Y_PRIVILEGES
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| permission_list paren_column_list_opt
	{
	    if (is_empty($2)) {
		set_result(&$$, $1, $1, NULL, NULL);
	    }
	    else set_result(&$$, $1, $2, NULL, $2.ast);
	}

	| grant_command_list
	;


grant_on_opt:

	  /* empty */	{ set_empty(&$$); }

	| grant_on_spec
	;


grant_on_spec:

	  Y_ON table_spec paren_column_list_opt
	{
	    if (is_empty($3)) {
		set_result(&$$, $1, $2, NULL, $2.ast);
	    }
	    else {
		$$.ast = new_tree(ank_list, NULL, $1, $3);
		ast_child_tree($$.ast, $2.ast);
		ast_child_tree($$.ast, $3.ast);
	    }
	}
	;


grantee_spec:

	  to_or_from grantee
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


to_or_from:

	  Y_TO

	| Y_FROM
	;


grantee:

	  Y_PUBLIC

	| name_list
	;


name_list:

	  identifier

	| name_list ',' identifier
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


with_grant_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_GRANT Y_OPTION
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


permission_list:

	  permission

	| permission_list ',' permission
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


permission:

	  Y_SELECT

	| Y_INSERT

	| Y_DELETE

	| Y_UPDATE

	| Y_EXECUTE
	{
	    grant_execute = 1;
	}

	| Y_REFERENCES
	;


grant_command_list:

	  grant_command

	| grant_command_list ',' grant_command
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


grant_command:

	  Y_ALTER Y_DATABASE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_DATABASE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_DEFAULT
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_PROCEDURE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_RULE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_TABLE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_CREATE Y_VIEW
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


paren_column_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| paren_column_list
	;


paren_column_list:

	  '(' column_name_list ')'
	{
	    set_result(&$$, $1, $3, NULL, $2.ast);
	}
	;


if:

	  Y_IF expression statement %prec LOWER_THAN_ELSE
	{
	    $$.ast = new_tree(ank_if_stmt, NULL, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $$.ast);
	}

	| Y_IF expression statement Y_ELSE statement
	{
	    ast_tree* else_clause;
	    $$.ast = new_tree(ank_if_stmt, NULL, $1, $5);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	    else_clause = new_tree(ank_ast_block, NULL, $4, $5);
	    ast_child_tree(else_clause, $5.ast);
	    ast_child_tree($$.ast, else_clause);
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}
	;


insert:

	  insert_head into_opt table_spec paren_column_list values_list
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $5, $5);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $5.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}

	| insert_head into_opt table_spec values_list
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $4, $4);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $4, NULL, $$.ast);
	}

	| insert_head into_opt table_spec paren_column_list select
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $5, $5);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $5.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $5, NULL, $$.ast);
	}

	| insert_head into_opt table_spec select
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $4, $4);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $4, NULL, $$.ast);
	}

	| insert_head into_opt table_spec paren_column_list '(' select ')'
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $7, $7);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $6.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $7, NULL, $$.ast);
	}

	| insert_head into_opt table_spec '(' select ')'
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $6, $6);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $5.ast);
	    sql_pop_scope();
	    set_result(&$$, $1, $6, NULL, $$.ast);
	}
	;


insert_head:

	  Y_INSERT
	{
	    sql_push_scope(NULL, NULL);
	}
	;


into_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_INTO
	;


values_list:

	  Y_VALUES
	{
	    sql_scope_clear_table_name();
	}
	'(' expression_list_opt ')'
	{
	    set_result(&$$, $1, $5, NULL, $4.ast);
	}
	;


expression_list:

	  expression
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| expression_list ',' expression
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


kill:

	  Y_KILL number
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	}
	;


label:

	  label_name ':'
	{
	    if (symbol_get($1) != sk_label) {
		$1.sym = symbol_set($1, sk_label);
	    }
	    $$.ast = new_tree(ank_label_decl, $1.sym, $1, $2);
	    ast_child_tree($$.ast, symbol_token($1));
	    ast_defined($$.ast);
	    ast_decldefined($$.ast);
	    set_result(&$$, $1, $2, $1.sym, $$.ast);
	}
	;


load_database:

	  Y_LOAD Y_DATABASE identifier Y_FROM stripe_dev_spec
		stripe_on_seq_opt dump_with_list_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    $$.ast = sql_call_stmt("LOAD DATABASE", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


load_transaction:

	  Y_LOAD Y_TRANSACTION identifier Y_FROM stripe_dev_spec
		 stripe_on_seq_opt dump_with_list_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    $$.ast = sql_call_stmt("LOAD TRANSACTION", $1, last_tok,
				   last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


online:

	  Y_ONLINE Y_DATABASE identifier
	{
	    if (!symbol_is_known($3)) {
		$3.sym = symbol_extern($3, sk_database);
	    }
	    $3.ast = symbol_token($3);
	    $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


open:

	  Y_OPEN identifier
	  {
	      if (!symbol_is_known($2)) {
		  $2.sym = symbol_extern($2, sk_cursor);
	      }
	      $2.ast = symbol_token($2);
	      $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	      ast_child_tree($1.ast->head->ast, $2.ast);
	      set_result(&$$, $1, $2, NULL, $$.ast);
	  }
	;


prepare:

	  Y_PREPARE Y_TRANSACTION
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	}
	;


print:

	  Y_PRINT expression_list
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	}
	;


const_or_variable:

	  constant

	| variable
	;


variable:

	  '@' identifier
	{
	    char* txt = token_range_text($1.start_token, $2.end_token);
	    $2.sym = sql_symbol_lookup(txt);
	    $2.ast = symbol_tree_range($2.sym, $1, $2);
	    set_result(&$$, $1, $2, NULL, $2.ast);
	    free(txt);
	}

	| '@' '@' identifier
	{
	    char* txt = token_range_text($1.start_token, $3.end_token);
	    $3.sym = sql_symbol_extern(txt, sk_variable);
	    $3.ast = symbol_tree_range($3.sym, $1, $3);
	    set_result(&$$, $1, $3, NULL, $3.ast);
	    free(txt);
	}
	;


raiserror:

	  Y_RAISERROR const_or_variable expression_list with_errordata_opt
	{
	    ast_tree* call_expr;
	    sqltype last_tok;
	    if (is_empty($4)) {
		last_tok = $3;
	    }
	    else last_tok = $4;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $2.ast);
	    ast_child_tree(call_expr, $3.ast);
	    ast_child_tree(call_expr, $4.ast);
	}

	| Y_RAISERROR const_or_variable ',' expression_list with_errordata_opt
	{
	    ast_tree* call_expr;
	    sqltype last_tok;
	    if (is_empty($5)) {
		last_tok = $4;
	    }
	    else last_tok = $5;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $2.ast);
	    ast_child_tree(call_expr, $4.ast);
	    ast_child_tree(call_expr, $5.ast);
	}

	| Y_RAISERROR const_or_variable %prec HIGHER_THAN_UPDATE
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	}

	| Y_RAISERROR const_or_variable Y_WITH Y_ERRORDATA select_list
	{
	    ast_tree* call_expr;
	    $$.ast = symbol_call_stmt(&$1, $1, $5, $5);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $2.ast);
	    ast_child_tree(call_expr, $5.ast);
	}
	;


with_errordata_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_ERRORDATA select_list
	{
	    set_result(&$$, $1, $3, NULL, $3.ast);
	}
	;


readtext:

	  Y_READTEXT column_spec variable number number holdlock_opt
		bytes_chars_opt isolation_opt
	{
	    sqltype last_tok;
	    ast_tree* call_expr;
	    if (is_empty($8)) {
		if (is_empty($7)) {
		    if (is_empty($6)) {
			last_tok = $5;
		    }
		    else last_tok = $6;
		}
		else last_tok = $7;
	    }
	    else last_tok = $8;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    call_expr = $$.ast->head->ast;
	    ast_child_tree(call_expr, $2.ast);
	    ast_child_tree(call_expr, $3.ast);
	}
	;


column_spec:

	  identifier '.' owner_opt '.' identifier '.' identifier
	{
	    sprintf(txtbuf, "%s.%s", token_text($5), token_text($7));
	    $7.sym = sql_symbol_extern(txtbuf, sk_column);
	    $7.ast = symbol_token($7);
	    set_result(&$$, $1, $7, NULL, $7.ast);
	}

	| identifier '.' identifier
	{
	    sprintf(txtbuf, "%s.%s", token_text($1), token_text($3));
	    $3.sym = sql_symbol_extern(txtbuf, sk_column);
	    $3.ast = symbol_token($3);
	    set_result(&$$, $1, $3, NULL, $3.ast);
	}
	;


holdlock_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_HOLDLOCK
	;


bytes_chars_opt:

	  /* empty */	{ set_empty(&$$); }

	| using_bytes_chars
	;


using_bytes_chars:

	  Y_USING Y_BYTES
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_USING Y_CHARACTERS
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


isolation_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_AT Y_ISOLATION read_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


read_spec:

	  Y_READ Y_UNCOMMITTED
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_READ Y_COMMITTED
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_SERIALIZABLE

	| number
	;


reconfigure:

	  Y_RECONFIGURE
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $1, $1);
	}
	;


return:

	  Y_RETURN %prec HIGHER_THAN_UPDATE
	{
	    $$.ast = new_tree(ank_return_stmt, NULL, $1, $1);
	}

	| Y_RETURN expression
	{
	    $$.ast = new_tree(ank_return_stmt, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}
	;


revoke:

	  Y_REVOKE grant_for_opt privilege_spec grant_on_opt
		grantee_spec cascade_opt
	{
	    sqltype last_tok;
	    if (is_empty($6)) {
		last_tok = $5;
	    }
	    else last_tok = $6;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	    ast_child_tree($$.ast->head->ast, $4.ast);
	}
	;


grant_for_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_GRANT Y_OPTION Y_FOR
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


cascade_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_CASCADE
	;


rollback_trans_or_work:

	  Y_ROLLBACK trans_or_work identifier_opt
	{
	    sprintf(txtbuf, "ROLLBACK %s", token_text($2));
	    if (is_empty($3)) {
		$$.ast = sql_call_stmt(txtbuf, $1, $2, $2);
	    }
	    else $$.ast = sql_call_stmt(txtbuf, $1, $3, $3);
	}
	;


identifier_opt:

	  /* empty */	{ set_empty(&$$); }

	| identifier
	;


rollback_trigger:

	  Y_ROLLBACK Y_TRIGGER raiserror_opt
	{
	    if (is_empty($3)) {
		$$.ast = sql_call_stmt("ROLLBACK TRIGGER",
				       $1, $2, $2);
	    }
	    else {
		$$.ast = sql_call_stmt("ROLLBACK TRIGGER",
				       $1, $3, $3);
		ast_child_tree($$.ast->head->ast, $3.ast);
	    }
	}
	;


raiserror_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH  raiserror
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


save:

	  Y_SAVE Y_TRANSACTION identifier
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	}
	;


select:

	  select_kwd all_distinct_opt select_list select_into_opt
		select_from_opt where_opt group_by_opt having_opt
		union_seq_opt order_by_opt compute_seq_opt
		for_at_seq_opt
	{
	    sqltype last_tok;
	    ast_tree* call_expr;
	    if (is_empty($12)) {
		if (is_empty($11)) {
		    if (is_empty($10)) {
			if (is_empty($9)) {
			    if (is_empty($8)) {
				if (is_empty($7)) {
				    if (is_empty($6)) {
					if (is_empty($5)) {
					    if (is_empty($4)) {
						last_tok = $3;
					    }
					    else last_tok = $4;
					}
					else last_tok = $5;
				    }
				    else last_tok = $6;
				}
				else last_tok = $7;
			    }
			    else last_tok = $8;
			}
			else last_tok = $9;
		    }
		    else last_tok = $10;
		}
		else last_tok = $11;
	    }
	    else last_tok = $12;
	    map_eq_to_assign($3.ast);
	    $$.ast = new_tree(ank_expr_stmt, NULL, $1, last_tok);
	    $1.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $5.ast);
	    ast_child_tree($1.ast, $6.ast);
	    ast_child_tree($1.ast, $7.ast);
	    ast_child_tree($1.ast, $8.ast);
	    ast_child_tree($1.ast, $9.ast);
	    ast_child_tree($1.ast, $10.ast);
	    ast_child_tree($1.ast, $11.ast);
	    ast_child_tree($1.ast, $12.ast);
	    ast_child_tree($$.ast, $1.ast);
	    ast_resolve_columns($5.ast, $3.ast);
	    ast_resolve_columns($5.ast, $5.ast);
	    if (table_count > 1) {
		if (!is_empty($6)) {
		    ast_resolve_columns($5.ast, $6.ast);
		}
		if (!is_empty($7)) {
		    ast_resolve_columns($5.ast, $7.ast);
		}
		if (!is_empty($8)) {
		    ast_resolve_columns($5.ast, $8.ast);
		}
		if (!is_empty($10)) {
		    ast_resolve_columns($5.ast, $10.ast);
		}
		if (!is_empty($11)) {
		    ast_resolve_columns($5.ast, $11.ast);
		}
	    }
	    sql_pop_scope();
	}
	;


unionable_select:

	  select_kwd all_distinct_opt select_list select_into_opt
		select_from_opt where_opt group_by_opt having_opt
	{
	    sqltype last_tok;
	    ast_tree* call_expr;
	    if (is_empty($8)) {
		if (is_empty($7)) {
		    if (is_empty($6)) {
			if (is_empty($5)) {
			    if (is_empty($4)) {
				last_tok = $3;
			    }
			    else last_tok = $4;
			}
			else last_tok = $5;
		    }
		    else last_tok = $6;
		}
		else last_tok = $7;
	    }
	    else last_tok = $8;
	    map_eq_to_assign($3.ast);
	    $$.ast = symbol_call_expr(&$1, $1, last_tok);
	    ast_child_tree($$.ast, $3.ast);
	    ast_child_tree($$.ast, $4.ast);
	    ast_child_tree($$.ast, $5.ast);
	    ast_child_tree($$.ast, $6.ast);
	    ast_child_tree($$.ast, $7.ast);
	    ast_child_tree($$.ast, $8.ast);
	    ast_resolve_columns($5.ast, $3.ast);
	    ast_resolve_columns($5.ast, $5.ast);
	    if (table_count > 1) {
		if (!is_empty($6)) {
		    ast_resolve_columns($5.ast, $6.ast);
		}
		if (!is_empty($7)) {
		    ast_resolve_columns($5.ast, $7.ast);
		}
		if (!is_empty($8)) {
		    ast_resolve_columns($5.ast, $8.ast);
		}
	    }
	    sql_pop_scope();
	}

	| '(' unionable_select union_seq_opt ')'
	{
	    ast_child_tree($2.ast, $3.ast);
	    set_result(&$$, $1, $4, NULL, $2.ast);
	}
	;


select_kwd:

	  Y_SELECT
	{
	    sql_push_scope(NULL, NULL);
	    in_select_items = 1;
	}


all_distinct_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ALL

	| Y_DISTINCT
	;


select_list:

	  select_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| select_list ',' select_spec
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


select_spec:

	  '*'

	| expression as_opt identifier_or_string
	{
	    $3.sym = symbol_set($3, sk_column_alias);
	    $3.ast = symbol_tree($3.sym, $3);
	    sym_inactive($3.sym);
	    $$.ast = new_tree(ank_field_decl, $3.sym, $1, $3);
	    ast_child_tree($$.ast, $3.ast);
	    ast_child_tree($$.ast, $1.ast);
	    ast_defined($$.ast);
	    ast_decldefined($$.ast);
	    set_result(&$$, $1, $3, $3.sym, $$.ast);
	}

	| expression
	;


as_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_AS
	;


identifier_or_string:

	  identifier

	| string
	{
	    $$.ast = string_literal_tree($1);
	}
	;


select_into_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_INTO table_spec
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


select_from_opt:

	  /* empty */
	{
	    set_empty(&$$);
	    sym_inactive_clear();
	    in_select_items = 0;
	}

	| Y_FROM select_from_list
	{
	    $$.ast = new_tree(ank_ast_block, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	    sym_inactive_clear();
	    in_select_items = 0;
	    table_list = $2.ast;
	}
	;


select_from_list:

	  select_from_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	    table_count = 1;
	}

	| select_from_list ',' select_from_spec
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	    table_count++;
	}
	;


select_from_spec:

	  table_spec identifier_opt index_prefetch_queuing_opt
		hold_nohold_opt shared_opt join_seq_opt
	{
	    sqltype last_tok;
	    if (is_empty($6)) {
		if (is_empty($5)) {
		    if (is_empty($4)) {
			if (is_empty($3)) {
			    if (is_empty($2)) {
				last_tok = $1;
			    }
			    else last_tok = $2;
			}
			else last_tok = $3;
		    }
		    else last_tok = $4;
		}
		else last_tok = $5;
	    }
	    else last_tok = $6;
	    if (!is_empty($2)) {
		$2.sym = symbol_set($2, sk_alias);
	        $2.ast = symbol_tree($2.sym, $2);
		if ($2.sym && $1.sym) {
		    $2.sym->alias_sym = $1.sym;
		}
	    }
	    if (!is_empty($2) || !is_empty($6)) {
		$$.ast = new_tree(ank_list, NULL, $1, last_tok);
		ast_child_tree($$.ast, $1.ast);
		ast_child_tree($$.ast, $2.ast);
		ast_child_tree($$.ast, $6.ast);
		if (!is_empty($6)) {
		    ast_resolve_columns($1.ast, $6.ast);
		    ast_resolve_columns($6.ast, $6.ast);
		}
	    }
	    else set_result(&$$, $1, last_tok, NULL, $1.ast);
	}
	;


join_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| join_seq
	;


join_seq:

	  join_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| join_seq join_spec
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


join_spec:
	  join_operator table_spec identifier_opt Y_ON expression
	{
	    ast_child_tree($1.ast, $2.ast);
	    if (!is_empty($3)) {
		$3.sym = symbol_set($3, sk_alias);
		$3.ast = symbol_tree($3.sym, $3);
		if ($2.sym && $3.sym) {
		    $3.sym->alias_sym = $2.sym;
		}
	    }
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $5.ast);
	    set_result(&$$, $1, $5, NULL, $1.ast);
	}

	| join_operator table_spec identifier_opt Y_USING
		paren_column_list
	{
	    ast_child_tree($1.ast, $2.ast);
	    if (!is_empty($3)) {
		$3.sym = symbol_set($3, sk_alias);
		$3.ast = symbol_tree($3.sym, $3);
		if ($2.sym && $3.sym) {
		    $3.sym->alias_sym = $2.sym;
		}
	    }
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $5.ast);
	    set_result(&$$, $1, $5, NULL, $1.ast);
	}

	| join_operator table_spec identifier_opt join_seq
		Y_ON expression
	{
	    ast_child_tree($1.ast, $2.ast);
	    if (!is_empty($3)) {
		$3.sym = symbol_set($3, sk_alias);
		$3.ast = symbol_tree($3.sym, $3);
		if ($2.sym && $3.sym) {
		    $3.sym->alias_sym = $2.sym;
		}
	    }
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $6.ast);
	    set_result(&$$, $1, $6, NULL, $1.ast);
	}

	| join_operator table_spec identifier_opt join_seq
		Y_USING paren_column_list
	{
	    ast_child_tree($1.ast, $2.ast);
	    if (!is_empty($3)) {
		$3.sym = symbol_set($3, sk_alias);
		$3.ast = symbol_tree($3.sym, $3);
		if ($2.sym && $3.sym) {
		    $3.sym->alias_sym = $2.sym;
		}
	    }
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $4.ast);
	    ast_child_tree($1.ast, $6.ast);
	    set_result(&$$, $1, $6, NULL, $1.ast);
	}
	;


hold_nohold_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_HOLDLOCK

	| Y_NOHOLDLOCK
	;


shared_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_SHARED
	;


join_operator:

	  Y_CROSS Y_JOIN
	{
	    $$.ast = new_tree(ank_cross_join_expr, NULL, $1, $2);
	}

	| join_type Y_JOIN
	{
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}

	| Y_NATURAL join_type Y_JOIN
	{
	    set_result(&$$, $1, $3, NULL, $2.ast);
	}
	;


join_type:

	  Y_INNER
	{
	    $$.ast = new_tree(ank_inner_join_expr, NULL, $1, $1);
	}

	| Y_LEFT outer_opt
	{
	    if (is_empty($1)) {
		$$.ast = new_tree(ank_left_join_expr, NULL, $1, $1);
	    }
	    else $$.ast = new_tree(ank_left_join_expr, NULL, $1, $2);
	}

	| Y_RIGHT outer_opt
	{
	    if (is_empty($1)) {
		$$.ast = new_tree(ank_right_join_expr, NULL, $1, $1);
	    }
	    else $$.ast = new_tree(ank_right_join_expr, NULL, $1, $2);
	}

	| Y_FULL outer_opt
	{
	    if (is_empty($1)) {
		$$.ast = new_tree(ank_full_join_expr, NULL, $1, $1);
	    }
	    else $$.ast = new_tree(ank_full_join_expr, NULL, $1, $2);
	}
	; 


outer_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_OUTER
	;


group_by_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_GROUP Y_BY all_opt expression_list
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


all_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ALL
	;


having_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_HAVING expression
	{
	    $$.ast = symbol_call_expr(&$1, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}
	;


union_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| union_seq
	;


union_seq:

	  union_clause
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| union_seq union_clause
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


union_clause:

	  Y_UNION unionable_select
	{
	    $$.ast = symbol_call_expr(&$1, $1, $2);
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $$.ast);
	}

	| Y_UNION Y_ALL unionable_select
	{
	    $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $$.ast);
	}
	;


order_by_opt:

	  /* empty */	{ set_empty(&$$); }

	| order_by_clause
	;


order_by_clause:

	  Y_ORDER Y_BY order_by_spec_list
	{
	    $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


order_by_spec_list:

	  order_by_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| order_by_spec_list ',' order_by_spec
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


order_by_spec:

	  expression asc_desc_opt
	{
	    if (is_empty($2)) {
		$$.ast = $1.ast;
	    }
	    else {
		$$.ast = symbol_call_expr(&$2, $1, $2);
		ast_child_tree($$.ast, $1.ast);
	    }
	}
	;


asc_desc_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ASC

	| Y_DESC
	;


compute_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| compute_seq;


compute_seq:

	  compute_spec
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| compute_seq compute_spec
	{
	    ast_child_tree($1.ast, $2.ast);
	    set_result(&$$, $1, $2, NULL, $1.ast);
	}
	;


compute_spec:

	  Y_COMPUTE agg_expr_list compute_by_opt
	{
	    if (is_empty($3)) {
		$$.ast = symbol_call_expr(&$1, $1, $2);
	    }
	    else $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


agg_expr_list:

	  agg_expr
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| agg_expr_list ',' agg_expr
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


agg_expr:

	  Y_SUM '(' all_distinct_opt expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $5);
	    ast_child_tree($$.ast, $4.ast);
	}

	| Y_AVG '(' all_distinct_opt expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $5);
	    ast_child_tree($$.ast, $4.ast);
	}

	| Y_MIN '(' expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}

	| Y_MAX '(' expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}

	| Y_COUNT '(' all_distinct_opt expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $5);
	    ast_child_tree($$.ast, $4.ast);
	}

	| Y_COUNT '(' '*' ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	}
	;


compute_by_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_BY column_name_list
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


for_at_seq_opt:

	  /* empty */	{ set_empty(&$$); }

	| for_at_seq
	;


for_at_seq:

	   for_at_spec

	| for_at_seq for_at_spec
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


for_at_spec:

	  Y_FOR read_only_or_update
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_FOR Y_BROWSE
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_AT Y_ISOLATION read_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


set:

	  Y_SET set_variant
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	}
	;


set_variant:

	  set_misc_on_off

	| set_misc_list_on_off

	| set_date_lang_list

	| set_rowcount_textsize_list

	| char_convert

	| cursor_rows

	| table_count

	| transaction_isolation
	;


set_misc_on_off:

	  Y_ANSINULL on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_ANSI_PERMISSIONS on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_ARITHABORT ofl_trunc on_off
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_ARITHIGNORE ofl_opt on_off
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_FIPSFLAGGER on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_FLUSHMESSAGE on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_IDENTITY_INSERT table_spec on_off
	{
	    $$.ast = symbol_call_expr(&$1, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	}

	| Y_OFFSETS set_offset_spec_list on_off
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_PREFETCH on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_QUOTED_IDENTIFIER on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_ROLE string on_off
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_STATISTICS set_stat_spec_list on_off
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_STRING_RTRUNCATION on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


on_off:

	  Y_ON

	| Y_OFF
	;


ofl_trunc:

	  Y_ARITH_OVERFLOW

	| Y_NUMERIC_TRUNCATION
	;


ofl_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ARITH_OVERFLOW
	;


set_offset_spec_list:

	  set_offset_spec

	| set_offset_spec_list ',' set_offset_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


set_offset_spec:

	  Y_SELECT

	| Y_FROM

	| Y_ORDER

	| Y_COMPUTE

	| Y_TABLE

	| Y_PROCEDURE

	| Y_STATEMENT

	| Y_PARAM

	| Y_EXECUTE
	;


set_stat_spec_list:

	  set_stat_spec

	| set_stat_spec_list ',' set_stat_spec
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


set_stat_spec:

	  Y_IO

	| Y_SUBQUERYCACHE

	| Y_TIME
	;


set_misc_list_on_off:

	  set_misc_list on_off
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


set_misc_list:

	  set_misc_spec

	| set_misc_list ',' set_misc_spec
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


set_misc_spec:

	  Y_CHAINED

	| Y_CLOSE Y_ON Y_ENDTRAN
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}

	| Y_NOCOUNT

	| Y_NOEXEC

	| Y_PARSEONLY

	| Y_PROCID

	| Y_SELF_RECURSION

	| Y_SHOWPLAN
	;


set_date_lang_list:

	  set_date_lang

	| set_date_lang_list ',' set_date_lang
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


set_date_lang:

	  Y_DATEFIRST number
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_DATEFORMAT identifier
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_LANGUAGE identifier
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


set_rowcount_textsize_list:

	  set_rowcount_textsize

	| set_rowcount_textsize_list ',' set_rowcount_textsize
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


set_rowcount_textsize:

	  Y_ROWCOUNT number
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_ROWCOUNT variable
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_TEXTSIZE number
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_TEXTSIZE variable
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


char_convert:

	  Y_CHAR_CONVERT off_on_charset
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


off_on_charset:

	  Y_OFF

	| Y_ON with_err_noerr_opt
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| charset with_err_noerr_opt
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


with_err_noerr_opt:

	  Y_WITH Y_ERROR
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_WITH Y_NO_ERROR
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


charset:

	  number

	| identifier
	;


cursor_rows:

	  Y_CURSOR Y_ROWS number Y_FOR identifier
	{
	    if (!symbol_is_known($5)) {
		$5.sym = symbol_extern($5, sk_cursor);
	    }
	    $5.ast = symbol_token($5);
	    $$.ast = sql_call_expr("CURSOR ROWS", $1, $5);
	    ast_child_tree($$.ast, $5.ast);
	}
	;


table_count:

	  Y_TABLE Y_COUNT number
	{
	    set_result(&$$, $1, $3, NULL, NULL);
	}
	;


transaction_isolation:

	  Y_TRANSACTION Y_ISOLATION Y_LEVEL number
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}
	;


setuser:

	  Y_SETUSER
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $1, $1);
	}

	| Y_SETUSER string
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	}

	;


shutdown:

	  Y_SHUTDOWN identifier wait_opt
	{
	    if (is_empty($3)) {
		$$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    }
	    else $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	}

	| Y_SHUTDOWN wait_opt
	{
	    if (is_empty($2)) {
		$$.ast = symbol_call_stmt(&$1, $1, $1, $1);
	    }
	    else $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	}
	;


wait_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_WAIT
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_WITH Y_NOWAIT
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


truncate:

	  Y_TRUNCATE Y_TABLE table_spec
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


update_columns:

	  Y_UPDATE
	{
	    sql_push_scope(NULL, NULL);
	    in_select_items = 1;
	}
	table_spec Y_SET column_asgn_list select_from_opt where_opt
	{
	    sqltype last_tok;
	    if (is_empty($7)) {
		if (is_empty($6)) {
		    last_tok = $5;
		}
		else last_tok = $6;
	    }
	    else last_tok = $7;
	    $$.ast = symbol_call_stmt(&$1, $1, last_tok, last_tok);
	    ast_child_tree($1.ast, $3.ast);
	    ast_child_tree($1.ast, $5.ast);
	    ast_child_tree($1.ast, $6.ast);
	    ast_child_tree($1.ast, $7.ast);
	    ast_resolve_columns($6.ast, $5.ast);
	    ast_resolve_columns($6.ast, $6.ast);
	    if (table_count > 1) {
		if (!is_empty($7)) {
		    ast_resolve_columns($6.ast, $7.ast);
		}
	    }
	    sql_pop_scope();
	    set_result(&$$, $1, last_tok, NULL, $$.ast);
	}
	;


column_asgn_list:

	  column_asgn
	{
	    $$.ast = new_tree(ank_list, NULL, $1, $1);
	    ast_child_tree($$.ast, $1.ast);
	}

	| column_asgn_list ',' column_asgn
	{
	    ast_child_tree($1.ast, $3.ast);
	    set_result(&$$, $1, $3, NULL, $1.ast);
	}
	;


column_asgn:

	  column_ref '=' expression
	{
	    $$.ast = new_tree(ank_assign_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


update_statistics:

	  Y_UPDATE Y_STATISTICS identifier index_name_opt
	{
	    sql_tableref(&$3);
	    sql_scope_set_table_name(last_table_name);
	    if (!is_empty($4)) {
		$$.ast = sql_call_stmt("UPDATE STATISTICS", $1, $4, $4);
	    }
	    else $$.ast = sql_call_stmt("UPDATE STATISTICS", $1, $3, $3);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	}
	;


index_name_opt:

	  /* empty */	{ set_empty(&$$); }

	| identifier
	;


use:

	  Y_USE identifier
	{
	    if (!symbol_is_known($2)) {
		$2.sym = symbol_extern($1, sk_database);
	    }
	    $2.ast = symbol_token($2);
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	}
	;


waitfor:

	  Y_WAITFOR waitfor_spec
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $2, $2);
	}
	;


waitfor_spec:

	  Y_DELAY string
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_TIME string
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}

	| Y_ERROREXIT

	| Y_PROCESSEXIT

	| Y_MIRROREXIT	
	;


while:

	  Y_WHILE expression statement
	{
	    $$.ast = new_tree(ank_while_stmt, NULL, $1, $3);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $3.ast);
	}
	;


writetext:

	  Y_WRITETEXT column_spec expression with_log_opt
		const_or_variable
	{
	    $$.ast = symbol_call_stmt(&$1, $1, $5, $5);
	    ast_child_tree($$.ast->head->ast, $2.ast);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}
	;


with_log_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_WITH Y_LOG
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


sys_fcn:

	  agg_expr

	| identifier '(' expression_list_opt ')'
	{
	    if (!symbol_is_known($1)) {
		$1.sym = symbol_extern($1, sk_function);
	    }
	    $1.ast = symbol_token($1);
	    $$.ast = new_tree(ank_call_expr, $1.sym, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| Y_CONVERT '(' datatype ',' expression_list ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}

	| Y_DATEADD '(' datepart ',' expression_list ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}

	| Y_DATEDIFF '(' datepart ',' expression_list ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}

	| Y_DATENAME '(' datepart ',' expression_list ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}

	| Y_DATEPART '(' datepart ',' expression ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $6);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}

	| Y_PATINDEX '(' expression ',' expression comma_bytes_chars_opt ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $7);
	    ast_child_tree($$.ast->head->ast, $3.ast);
	    ast_child_tree($$.ast->head->ast, $5.ast);
	}
	;


expression_list_opt:

	  /* empty */	{ set_empty(&$$); }

	| expression_list
	;


datepart:

	  Y_YEAR

	| Y_QUARTER

	| Y_MONTH

	| Y_WEEK

	| Y_DAY

	| Y_DAYOFYEAR

	| Y_WEEKDAY

	| Y_HOUR

	| Y_MINUTE

	| Y_SECOND

	| Y_MILLISECOND
	;


comma_bytes_chars_opt:

	  /* empty */	{ set_empty(&$$); }

	| ',' using_bytes_chars
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


primary:

	  constant

	| variable

	| column_ref

	| sys_fcn

	| Y_USER

	| Y_NULL

	| Y_UPDATE '(' column_ref ')'
	{
	    $$.ast = symbol_call_expr(&$1, $1, $4);
	    ast_child_tree($$.ast, $3.ast);
	}

	| Y_IDENTITY '(' number ')'
	{
	    set_result(&$$, $1, $4, NULL, NULL);
	}

	| '(' expression ')'
	{
	    set_result(&$$, $1, $3, NULL, $2.ast);
	}

	| '(' select ')'
	{
	    set_result(&$$, $1, $3, NULL, $2.ast);
	}
	;


column_ref:

	  identifier
	{
	    if (sql_scope_table_name()) {
		sprintf(txtbuf, "%s.%s", sql_scope_table_name(),
			token_text($1));
		$1.sym = sql_symbol_extern(txtbuf, sk_column);
	    }
	    else if (!$1.sym) {
		if (in_select_items) {
		    $1.sym = symbol_extern($1, sk_externref);
		}
		else $1.sym = symbol_extern($1, sk_column);
	    }
	    $$.sym = $1.sym;
	    $$.ast = symbol_token($1);
	}

	| identifier '.' identifier
	{
	    if (!symbol_is_known($1)) {
		if (in_select_items) {
		    $1.sym = symbol_extern($1, sk_externref);
		}
		else $1.sym = symbol_extern($1, sk_table);
	    }
	    sprintf(txtbuf, "%s.%s", token_text($1), token_text($3));
	    switch(symbol_get($1)) {
	    case sk_table:
		$$.sym = sql_symbol_extern(txtbuf, sk_column);
		break;
	    case sk_alias:
		if ($1.sym->alias_sym) {
		    sprintf(txtbuf, "%s.%s", $1.sym->alias_sym->sym_text,
			    token_text($3));
		    $$.sym = sql_symbol_extern(txtbuf, sk_column);
		}
		else $$.sym = sql_symbol_extern(txtbuf, sk_externref);
		break;
	    default:
		$$.sym = sql_symbol_extern(txtbuf, sk_externref);
	    }
	    $$.ast = symbol_tree_ast($$.sym, $1, $3);
	    ast_child_tree($$.ast, symbol_reference($1));
	    ast_child_tree($$.ast, symbol_reference_tree($$.sym, $3));
	}

	| identifier '.' owner_opt '.' identifier '.' identifier
	{
	    if (!symbol_is_known($1)) {
		$1.sym = symbol_extern($1, sk_database);
	    }
	    if (!symbol_is_known($5)) {
		if (in_select_items) {
		    $5.sym = symbol_extern($5, sk_externref);
		}
		else $5.sym = symbol_extern($5, sk_table);
	    }
	    sprintf(txtbuf, "%s.%s", token_text($5), token_text($7));
	    switch(symbol_get($5)) {
	    case sk_table:
		$$.sym = sql_symbol_extern(txtbuf, sk_column);
		break;
	    case sk_alias:
		if ($5.sym->alias_sym) {
		    sprintf(txtbuf, "%s.%s", $5.sym->alias_sym->sym_text,
			    token_text($7));
		    $$.sym = sql_symbol_extern(txtbuf, sk_column);
		}
		else $$.sym = sql_symbol_extern(txtbuf, sk_externref);
		break;
	    default:
		$$.sym = sql_symbol_extern(txtbuf, sk_column);
	    }
	    $$.ast = symbol_tree_ast($$.sym, $5, $7);
	    ast_child_tree($$.ast, symbol_reference($5));
	    ast_child_tree($$.ast, symbol_reference_tree($$.sym, $7));
	    ast_extend_start($$.ast, token_loc_start($1));
	}
	;


expression:

	  primary

	| expression '=' expression
	{
	    $$.ast = new_tree(ank_eq_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '>' expression
	{
	    $$.ast = new_tree(ank_gt_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '<' expression
	{
	    $$.ast = new_tree(ank_lt_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression OP_GE expression
	{
	    $$.ast = new_tree(ank_ge_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression OP_LE expression
	{
	    $$.ast = new_tree(ank_le_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression OP_NE expression
	{
	    $$.ast = new_tree(ank_ne_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression OP_LEFT_JOIN expression
	{
	    $$.ast = new_tree(ank_ljoin_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression OP_RIGHT_JOIN expression
	{
	    $$.ast = new_tree(ank_rjoin_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '=' Y_ANY expression
	{
	    $$.ast - new_tree(ank_eq_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression '>' Y_ANY expression
	{
	    $$.ast - new_tree(ank_gt_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression '<' Y_ANY expression
	{
	    $$.ast - new_tree(ank_lt_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_GE Y_ANY expression
	{
	    $$.ast - new_tree(ank_ge_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_LE Y_ANY expression
	{
	    $$.ast - new_tree(ank_le_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_NE Y_ANY expression
	{
	    $$.ast - new_tree(ank_ne_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression '=' Y_ALL expression
	{
	    $$.ast - new_tree(ank_eq_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression '>' Y_ALL expression
	{
	    $$.ast - new_tree(ank_gt_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression '<' Y_ALL expression
	{
	    $$.ast - new_tree(ank_lt_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_GE Y_ALL expression
	{
	    $$.ast - new_tree(ank_ge_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_LE Y_ALL expression
	{
	    $$.ast - new_tree(ank_le_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression OP_NE Y_ALL expression
	{
	    $$.ast - new_tree(ank_ne_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression Y_IN '(' subquery_or_expr_list ')'
	{
	    $$.ast = new_tree(ank_in_expr, NULL, $1, $5);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression Y_NOT Y_IN '(' subquery_or_expr_list ')'
	{
	    ast_tree* in_expr;
	    in_expr = new_tree(ank_in_expr, NULL, $1, $6);
	    ast_child_tree(in_expr, $1.ast);
	    ast_child_tree(in_expr, $5.ast);
	    $$.ast = new_tree(ank_truth_not_expr, NULL, $1, $6);
	    ast_child_tree($$.ast, in_expr);
	}

	| Y_EXISTS expression
	{
	    $$.ast = symbol_call_expr(&$1, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| expression Y_BETWEEN expression Y_AND_AFTER_BETWEEN expression
	{
	    $$.ast = symbol_call_expr(&$2, $1, $5);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	    ast_child_tree($$.ast, $5.ast);
	}

	| expression Y_NOT Y_BETWEEN expression Y_AND_AFTER_BETWEEN expression
	{
	    ast_tree* between_expr;
	    between_expr = symbol_call_expr(&$3, $1, $6);
	    ast_child_tree(between_expr, $1.ast);
	    ast_child_tree(between_expr, $4.ast);
	    ast_child_tree(between_expr, $6.ast);
	    $$.ast = new_tree(ank_truth_not_expr, NULL, $1, $6);
	    ast_child_tree($$.ast, between_expr);
	}

	| expression Y_LIKE primary escape_opt
	{
	    sqltype last_tok;
	    if (is_empty($4)) {
		last_tok = $3;
	    }
	    else last_tok = $4;
	    $$.ast = symbol_call_expr(&$2, $1, last_tok);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| expression Y_NOT Y_LIKE primary escape_opt
	{
	    sqltype last_tok;
	    ast_tree* like_expr;
	    if (is_empty($5)) {
		last_tok = $4;
	    }
	    else last_tok = $5;
	    like_expr = symbol_call_expr(&$3, $1, last_tok);
	    ast_child_tree(like_expr, $1.ast);
	    ast_child_tree(like_expr, $4.ast);
	    ast_child_tree(like_expr, $5.ast);
	    $$.ast = new_tree(ank_truth_not_expr, NULL, $1, $5);
	    ast_child_tree($$.ast, like_expr);
	}

	| expression Y_IS Y_NULL
	{
	    $$.ast = new_tree(ank_eq_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, literal_tree($3));
	}

	| expression Y_IS Y_NOT Y_NULL
	{
	    $$.ast = new_tree(ank_ne_expr, NULL, $1, $4);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, literal_tree($4));
	}

	| Y_NOT expression
	{
	    $$.ast = new_tree(ank_truth_not_expr, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| expression Y_AND expression %prec LOWER_THAN_BETWEEN
	{
	    $$.ast = new_tree(ank_truth_and_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression Y_OR expression
	{
	    $$.ast = new_tree(ank_truth_or_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| '-' expression %prec UMINUS
	{
	    $$.ast = new_tree(ank_negate_expr, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| '+' expression %prec UPLUS
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}

	| '~' expression
	{
	    $$.ast = new_tree(ank_bit_not_expr, NULL, $1, $2);
	    ast_child_tree($$.ast, $2.ast);
	}

	| expression '*' expression
	{
	    $$.ast = new_tree(ank_mult_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '/' expression
	{
	    $$.ast = new_tree(ank_div_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '%' expression
	{
	    $$.ast = new_tree(ank_trunc_mod_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '+' expression
	{
	    $$.ast = new_tree(ank_plus_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '-' expression
	{
	    $$.ast = new_tree(ank_minus_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '&' expression
	{
	    $$.ast = new_tree(ank_bit_and_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '|' expression
	{
	    $$.ast = new_tree(ank_bit_ior_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| expression '^' expression
	{
	    $$.ast = new_tree(ank_bit_xor_expr, NULL, $1, $3);
	    ast_child_tree($$.ast, $1.ast);
	    ast_child_tree($$.ast, $3.ast);
	}

	| case_expr
	;


case_expr:

	  Y_CASE when_seq else_opt Y_END
	{
	    ast_child_tree($2.ast, $3.ast);
	    set_result(&$$, $1, $4, NULL, $2.ast);
	}
	;


when_seq:

	  Y_WHEN expression Y_THEN expression
	{
	    $$.ast = new_tree(ank_if_stmt, NULL, $1, $4);
	    ast_child_tree($$.ast, $2.ast);
	    ast_child_tree($$.ast, $4.ast);
	}

	| when_seq Y_WHEN expression Y_THEN expression
	{
	    ast_tree* elsif = new_tree(ank_elsif_clause, NULL,
				       $2, $5);
	    ast_child_tree(elsif, $3.ast);
	    ast_child_tree(elsif, $5.ast);
	    ast_child_tree($1.ast, elsif);
	    set_result(&$$, $1, $5, NULL, $1.ast);
	}
	;


else_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ELSE expression
	{
	    set_result(&$$, $1, $2, NULL, $2.ast);
	}
	;


subquery_or_expr_list:

	  select

	| expression_list
	;


escape_opt:

	  /* empty */	{ set_empty(&$$); }

	| Y_ESCAPE string
	{
	    set_result(&$$, $1, $2, NULL, NULL);
	}
	;


constant:

	  string
	{
	    $$.ast = string_literal_tree($1);
	}

	| number
	{
	    $$.ast = literal_tree($1);
	}
	;


datatype:

	  Y_TINYINT
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_SMALLINT
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_INTEGER
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_NUMERIC precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_DECIMAL precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_FLOAT precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_DOUBLE Y_PRECISION
	  {
	      sprintf(txtbuf, "%s %s", token_text($1), token_text($2));
	      $$.sym = symbol_type_text(txtbuf);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $2);
	      ast_keyword($$.ast);
	  }

	| Y_REAL precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_MONEY
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_SMALLMONEY
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_TIMESTAMP
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_DATETIME
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_SMALLDATETIME
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_CHAR precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_NCHAR precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_VARCHAR precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_NVARCHAR precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_BINARY precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_VARBINARY precision_opt
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	      if (!is_empty($2)) {
		  if (datatype_mask & RELMASK_SIZE) {
		      ast_relation_size($$.ast, datatype_size);
		  }
		  if (datatype_mask & RELMASK_SCALE) {
		      ast_relation_scale($$.ast, datatype_scale);
		  }
		  ast_extend_end($$.ast, token_loc_end($2));
		  set_result(&$$, $1, $2, $$.sym, $$.ast);
	      }
	  }

	| Y_IMAGE
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_BIT
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_SYSNAME
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| Y_TEXT
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	      ast_keyword($$.ast);
	  }

	| identifier
	  {
	      $$.sym = symbol_type($1);
	      $$.ast = reference_tree(ank_ast_declspec, $$.sym, $1, $1);
	  }
	;


precision_opt:

	  /* empty */	{ set_empty(&$$); }

	| precision_spec
	;


precision_spec:
/* Records both the existence and value of the precision spec in
 * global values for interrogation by the major data type spec.
 */

	  '(' number ')'
	  {
	      datatype_mask = RELMASK_SIZE;
	      datatype_size = atoi(token_text($2));
	      set_result(&$$, $1, $3, NULL, NULL);
	  }

	| '(' number ',' number ')'
	  {
	      datatype_mask = RELMASK_SIZE | RELMASK_SCALE;
	      datatype_size = atoi(token_text($2));
	      datatype_scale = atoi(token_text($4));
	      set_result(&$$, $1, $5, NULL, NULL);
	  }
	;
%%
/* token_is_allowed returns 1 if the specified token is accepted in the
 * current state (i.e., if there is a shift or reduce action specifically
 * for that token).  Since default actions don't tell us anything about
 * whether a given token is acceptable or not, we have to simulate the
 * normal state machine for defaults.
 */

int token_is_allowed(tok)
     int tok;
{
    short* top_of_state_stack = cur_state_stack;
    int state = *top_of_state_stack;
    int mapped_token = YYTRANSLATE(tok);
    int table_base;
    int table_idx;
    static short* local_state_stack = NULL;
    static int local_state_stack_size = 0;
    int local_state_stack_depth = 0;
    int rule;
    int states_to_pop;
    int nonterminal;

    for (;;) {
	/* If the table base is YYFLAG, the state has only a default
	 * action, regardless of the token.
	 */

	table_base = yypact[state];
	if (table_base != YYFLAG) {

	    /* If the token has no valid entry in yytable, the default
	     * action is taken.
	     */

	    table_idx = table_base + mapped_token;
	    if (table_idx >= 0 &&
		table_idx <= YYLAST &&
		yycheck[table_idx] == mapped_token) {

		/* A yytable value of 0 or YYFLAG indicates an error,
		 * so the token is not allowed; otherwise, there is
		 * a shift or reduce for the token and the token is
		 * accepted.
		 */

		if (yytable[table_idx] == 0 ||
		    yytable[table_idx] == YYFLAG) {
		    return 0;	/* not allowed */
		}

		return 1;	/* allowed */
	    }
	}

	/* The default action (a reduction, by definition) was selected.
	 * If the rule being reduced is 0, it is an error and the token is
	 * not allowed.
	 */

	rule = yydefact[state];
	if (rule == 0) {
	    return 0;	/* not allowed */
	}

	/* Pop the requisite number of states from the stack. */

	states_to_pop = yyr2[rule];
	while (states_to_pop--) {
	    if (local_state_stack_depth > 0) {
		local_state_stack_depth--;
		if (local_state_stack_depth == 0) {
		    state = *top_of_state_stack;
		}
		else state = local_state_stack[local_state_stack_depth - 1];
	    }
	    else state = *--top_of_state_stack;
	}

	/* Based on the nonterminal just reduced, determine the new
	 * state.  If the nonterminal/current state combination has
	 * a valid entry in yytable, it gives the new state; otherwise,
	 * the nonterminal has a default state associated with it.
	 */

	nonterminal = yyr1[rule];
	table_idx = yypgoto[nonterminal - YYNTBASE] + state;
	if (table_idx >= 0 &&
	    table_idx <= YYLAST &&
	    yycheck[table_idx] == state) {
	    state = yytable[table_idx];
	}
	else state = yydefgoto[nonterminal - YYNTBASE];

	/* Push the new state on the (local) stack and try again to
	 * find an action for the token.
	 */

	if (!local_state_stack) {
	    local_state_stack_size = YYINITDEPTH;
	    local_state_stack = (short*) malloc(local_state_stack_size *
						sizeof(short));
	}
	else if (local_state_stack_depth >= local_state_stack_size - 1) {
	    local_state_stack_size *= 2;
	    local_state_stack = (short*) realloc(local_state_stack,
						 local_state_stack_size *
						 sizeof(short));
	}
	local_state_stack[local_state_stack_depth++] = state;
    }
}

/* map_to_allowed_token returns its second argument if the first
 * argument is a disallowed token and the second is allowed; otherwise,
 * it returns its first argument.
 */

int map_to_allowed_token(from_tok, to_tok)
     int from_tok;
     int to_tok;
{
    if (!token_is_allowed(from_tok) &&
	token_is_allowed(to_tok)) {
#if YYDEBUG
	if (yydebug) {
	    fprintf(stderr, "mapped from %s to %s: ",
		    yytname[YYTRANSLATE(from_tok)],
		    yytname[YYTRANSLATE(to_tok)]);
	}
#endif
	return to_tok;
    }
    return from_tok;
}

/* token_range_text returns a pointer to a string resulting from the
 * concatenation of the text of all tokens in the indicated range.
 * The storage for the string is in malloced memory, and the caller
 * is responsible for freeing it.
 */
char* token_range_text(unsigned first_tok, unsigned last_tok) {
    size_t len = 1;
    unsigned i;
    char* txt;
    char* p;
    for (i = first_tok; i <= last_tok; i++) {
	token* tok = get_token(i);
	len += strlen(tok->text);
    }
    txt = (char *)malloc(len);
    p = txt;
    for (i = first_tok; i <= last_tok; i++) {
	token* tok = get_token(i);
	size_t tok_len = strlen(tok->text);
	strncpy(p, tok->text, tok_len);
	p += tok_len;
    }
    *p = 0;
    return txt;
}
