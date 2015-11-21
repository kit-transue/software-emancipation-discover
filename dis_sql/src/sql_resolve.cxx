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
/*
 * S Q L _ R E S O L V E . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Dec. 1997 Mark B. Kaminsky
 *
 * Routines that resolve symbols that may not have been known
 * when they were first encountered.  These include forward
 * references (columns and package members) and overloaded symbols.
 * These routines only deal with unresolved symbols within one
 * source module, they do nothing to handle externrefs that
 * occur because the definition of a symbol is located in another
 * file (see -include and -symbols command line flags in sql_driver.C
 * for that).
 */

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstdlib>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include "machdep.h"

#include "sql_scanner.h"	// token struct
#include "sql_iff.h"		// IFF string defines
#include "sql.yacc.h"		// bison generated token values
#include "sql_ast.h"		// ast_tree struct
#include "sql_sym_enum.h"	// symbol kinds
#include "sql_resolve.h"	// resolve external symbols

// simple linked-list node to hold a symbol
struct sym_list {
   SQL_SYM * sym;
   struct sym_list * next;
};

// simple class w/ linked-list of symbols
typedef struct sym_list symlist;

class symbol_list {
 public:
   symlist * head;
   symlist * tail;

   symbol_list (void);
   ~symbol_list ();
   void clear (void);
   void add (SQL_SYM * sym);
};

symbol_list::symbol_list (void) : head (0), tail (0)
{
}

symbol_list::~symbol_list ()
{
   clear ();
}

// delete list
void symbol_list::clear (void)
{
   symlist * sl;
   symlist * slnext;

   if (head) {
      for (sl = head; sl; sl = slnext) {
	 slnext = sl->next;
	 delete sl;
      }
      head = 0;
      tail = 0;
   }
}

// add a symbol entry
void symbol_list::add (SQL_SYM * sym)
{
   symlist * sl = new symlist;
   sl->sym = sym;
   sl->next = 0;
   if (tail) {
      tail->next = sl;
      tail = sl;
   } else {
      head = sl;
      tail = sl;
   }
}


// class to resolve symbols
//
// Walks parts of the ASTs (abstract syntax trees) built by the parser
// in order to find unresolved symbols.
//
class ast_resolve {

 private:
   char txtbuf [1024];			// temp buffer to build names
   symbol_list sl_table_list;		// table list for column resolution
   const char *package_name;		// package name for member resolution

   void clear (void);
   void build_table_list (ast_tree * at);
   void resolve_columns (ast_tree * at);
   void resolve_package_members (ast_tree * at);

   SQL_SYM * add_column (const char * table_name,
			 const char * column_name);

   SQL_SYM * find_symbol (const char * parent_name,
			  const char * component_name);

   void set_symbol (ast_tree * at, SQL_SYM * sym);
   const char * get_table_name (SQL_SYM * sym);

   // copy and assignment not implemented
   ast_resolve (const ast_resolve& rhs);
   ast_resolve& operator= (const ast_resolve& rhs);

 public:
   ast_resolve (void);
   ~ast_resolve ();
   void resolve_package (const char * package, ast_tree * at);
   void resolve_table (ast_tree * at_table, ast_tree * at_column);
   void resolve_function (ast_tree * at_func, ast_tree * at_args);
};

ast_resolve::ast_resolve (void) :
	package_name ("")
{
}

ast_resolve::~ast_resolve ()
{
   clear ();
}

void ast_resolve::resolve_package (const char * package, ast_tree * at)
{
   if (package) {
      package_name = package;
      resolve_package_members (at);
   }
}

void ast_resolve::resolve_table (ast_tree * at_table, ast_tree * at_column)
{
   // first build linked-list of table symbols
   build_table_list (at_table);
   
   // if any tables see if any unresolved symbols
   // are column names
   if (sl_table_list.head) {
      resolve_columns (at_column);
   }
}

void ast_resolve::clear (void)
{
   package_name = "";
   sl_table_list.clear ();
}


// ast_resolve::build_table_list ---
//
// Recursively build linked-list of table symbols.
//
// Take AST which points to one or more tables
// and build list to be used to resolve column names.
//
// Also includes table alias symbols since they may be used
// in place of table names when referring to columns.
//
void ast_resolve::build_table_list (ast_tree * at)
{
   ast_treelist * atl;
   
   if (at) {
      if (at->sym) {
	 if ((at->sym->sym_token_type == sk_table) ||
	     (at->sym->sym_token_type == sk_alias)) {
	    sl_table_list.add (at->sym);
	 }
      }
      // recursive call to check child nodes
      for (atl = at->head; atl; atl = atl->next) {
	 build_table_list (atl->ast);
      }
   }
}

// ast_resolve::resolve_columns ---
//
// Recursive function to resolve column names in a AST subtree.
//
// Called by resolve_table() after table list is built in build_table_list().
//
// Walks AST that contains references to symbols that may be columns.
// Concatinates table names with candidate column names to see if
// that forms a known column name.  If there is a match the ast is
// updated.
//
// Example:
//	SELECT a, t.c, tab1.d
//	FROM tab1, tab2 t
//	WHERE e != 3;
//
// A table list of tab1, tab2, and table alias t (of tab2) will be built in 
// resolve_table() from the FROM AST.  Then this routine is passed
// the select list AST.  This will check the symbol table for columns
// tab1.a and tab2.a if found it will resolve the AST with
// that column symbol.  If not found will leave it unchanged as an
// externref.  It will resolve t.c as tab2.c.  If tab1.d is unresolved,
// which would be the case if it was not known to be a table, then
// it will be resolved (tab1 will be known as a table once it is found
// in the FROM list where only tables and their aliases can be listed).
//
// Later this routine will be called again for the WHERE clause AST,
// even though it is not a forward reference, since there is more then
// one table.
//
// Note that even if there was only one table in the FROM clause,
// the identifiers a and e might still not be column names, they
// might be functions that take no arguments (e.g. SELECT sysdate
// FROM dual;)
//
// End of example.
//
// This function is dependant on the way the parser builds its AST
// nodes.  Especially externrefs in the form "x.x".
//
// TBD: Break this up into smaller functional units or objects.
// Also handle schema_name.table_name.column_name
//
void ast_resolve::resolve_columns (ast_tree * at)
{
   ast_treelist * atl;
   symlist * sl;
   SQL_SYM * sym;
   const char * sym_name;
   const char * table_name;
   const char * column_name;
   const char * dot;
   int children_processed = 0;
   
   if (at) {
      // first check current AST node
      if (at->sym) {

	 // only interested in unresolved symbols (i.e. externref)
	 if (at->sym->sym_token_type == sk_externref) {

	    // full symbol name
	    sym_name = at->sym->sym_text;

	    // check if symbol name is of the form table_name.column_name
	    if ((dot = strchr (sym_name, '.')) == 0) {

	       // single name (no dots) -- is column candidate
	       column_name = sym_name;

	       // loop through list of table symbols
	       // and try table_name.column_name
	       //
	       sym = 0;		// symbol not found yet (loop test)

	       for (sl = sl_table_list.head; sl && !sym; sl = sl->next) {

		  // get table name (real table name in case of alias)
		  if (table_name = get_table_name (sl->sym)) {
		     
		     // lookup table_name.column_name in symbol table
		     if (sym = find_symbol (table_name, column_name)) {
			// found it

			// check if also unresolved in symbol table
			if (sym->sym_token_type == sk_externref) {
			   // now we know it is a column,
			   // correct symbol table entry
			   sym->sym_token_type = sk_column;
			}

			// modify AST entry with found symbol
			set_symbol (at, sym);
		     } else if (OSapi_strcasecmp (sl->sym->sym_text, column_name)
				== 0) {
			// column name candidate matches table name so it
			// is really a table or alias name not a column name

			// set sym to stop loop
			sym = sl->sym;

			// modify AST entry with found symbol
			set_symbol (at, sym);
		     }
		     else if (sl_table_list.head == sl_table_list.tail &&
			      dbvendor == DBVENDOR_SYBASE) {
			 sym = add_column(table_name, column_name);
			 set_symbol(at, sym);
		     }
		  }
	       }
	    } else { // dot != 0

	       // found dot in symbol name (dot points to '.')

	       // set column_name (character after '.')
	       column_name = dot + 1;

	       // check for table_name.column_name
	       // (or alias_name.column_name)

	       // make sure there is not a second dot
	       // TBD: handle schema_name.table_name.column_name
	       if (strchr (column_name, '.') == 0) {

		  // symbol name is of form x.x
		  // check for table_name.column_name
		  // (or alias_name.column_name)

		  // length of table_name
		  int len = dot - sym_name;

		  // build table name from symbol name
		  char table_buf [1024];
		  strncpy (table_buf, sym_name, len);
		  table_buf [len] = '\0';
		  
		  sym = 0;		// symbol not found yet (loop test)

		  // loop through table list to see if any match the
		  // table name in the unresolved symbol name
		  for (sl = sl_table_list.head; sl && !sym; sl = sl->next) {
		     
		     // check for table name match
		     if (OSapi_strcasecmp (sl->sym->sym_text, table_buf) == 0) {

			// found match

			// get table name (real table name in case of alias)
			if (table_name = get_table_name (sl->sym)) {

			   // add this table_name.column_name to the 
			   // symbol table if needed.
			   //
			   // symbol might already be in symbol table
			   // either because of table alias name here
			   // or earlier resolution in this call to
			   // resolve_column().
			   if (sym = add_column (table_name, column_name)) {

			      // if different --
			      // modify AST entry with found column symbol
			      if (sym != at->sym) {
				 set_symbol (at, sym);
			      }
 
			      // The AST of a table_name.column_name node
			      // has two AST children.  The first is the
			      // table or alias name, the second is the
			      // column name.  These are unresolved
			      // also and need to be set with their
			      // respective symbols.

			      // resolve table/alias (first child tree node)
			      atl = at->head;
			      if (atl) {
				 // modify AST entry with table symbol
				 set_symbol (atl->ast, sl->sym);

				 // resolve column (second/last child)
				 atl = atl->next;
				 if (atl) {
				    // modify AST entry with column symbol
				    set_symbol (atl->ast, sym);
				 }
			      }

			      // since the child subtrees are set for this
			      // AST node don't reprocess children
			      children_processed = 1;
			   }
			}
		     }
		  }
	       }
	    }
	 }
      } // if (at->sym)

      // recursively check all AST subtrees (child nodes)
      if ( ! children_processed ) {
	 for (atl = at->head; atl; atl = atl->next) {
	    resolve_columns (atl->ast);
	 }
      }
   }
}

// ast_resolve::resolve_package_members ---
//
// Recursive function to resolve package members in a AST subtree.
//
// Called by resolve_package() after package_name has been set.
//
// Walks package body AST and resolves symbols that were forward 
// references to other package members.
//
// Example:
//	PACKAGE BODY pack IS
//		PROCEDURE proc1 IS
//		BEGIN
//			x := proc2;	-- forward reference to pack.proc2
//		END proc1;
//		PROCEDURE proc2 IS
//		...
//	END pack;
//
// When the call to proc2 is parsed it may be unresolved if the package
// specification is not present.  However, once the PROCEDURE proc2
// is parsed, the symbol "pack.proc2" will be added to the symbol table
// and when this routine is called at the end of the parsing of the
// package body, the call will be resolved.
//
void ast_resolve::resolve_package_members (ast_tree * at)
{
   ast_treelist * atl;
   const char * sym_name;
   SQL_SYM * sym;
   const char * dot;
   
   if (at) {
      // first check current AST node
      if (at->sym) {

	 // only interested in unresolved symbols (i.e. externref)
	 if (at->sym->sym_token_type == sk_externref) {

	    // full symbol name
	    sym_name = at->sym->sym_text;

	    // cannot resolve fully specified names here (those with a dot)
	    // because these already have package name.
	    //
	    // TBD: What about record_name.field_name where record_name is
	    // member of package?
	    // Should try package_name.record_name.field_name?

	    if ((dot = strchr (sym_name, '.')) == 0) {

	       // lookup package_name.sym_name in symbol table
	       if (sym = find_symbol (package_name, sym_name)) {
		  // found it

		  // modify AST entry with found symbol
		  set_symbol (at, sym);
	       }
	    }
	 }
      }

      // recursively check all AST subtrees (child nodes)
      for (atl = at->head; atl; atl = atl->next) {
	 resolve_package_members (atl->ast);
      }
   }
}

// ast_resolve::add_column --
//
// Adds a table_name.column_name symbol to the global symbol table
// (if not there already) and returns its symbol pointer
//
SQL_SYM * ast_resolve::add_column (const char * table_name,
				   const char * column_name)
{
   SQL_SYM * sym;

   sprintf (txtbuf, "%s.%s", table_name, column_name);
   sym = sql_symbol_extern (txtbuf, sk_column);
   return (sym);
}

// ast_resolve::find_symbol --
//
// Does a lookup of parent_name.component_name in the global
// symbol table and returns the symbol pointer if found
// (or NULL if not found).
//
// If parent_name is a table then component_name is a column,
// if parent_name is a package then component_name is a package member
// such as a function, procedure, type, or variable.
//
SQL_SYM * ast_resolve::find_symbol (const char * parent_name,
				    const char * component_name)
{
   SQL_SYM * sym;

   sprintf (txtbuf, "%s.%s", parent_name, component_name);
   sym = sql_symbol_global_lookup (txtbuf);

   return (sym);
}

// ast_resolve::set_symbol --
//
// Changes the symbol in an AST node.
//
// Also sets the end point if not already set (TBD: may not be needed
// any more -- I think end point is now always set initially).  Note
// that the end point of the AST entry is calculated with the actual
// source text end, and not the full symbol name (e.g. colname vs
// tabname.colname).
//
// Also checks if this is the first reference for this symbol (see
// comment below).
//
void ast_resolve::set_symbol (ast_tree * at, SQL_SYM * sym)
{ 
   if (sym) {
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
      if (at && at->sym) {
	 // set end point
	 if ( ! at->end.lineno ) {
	    at->end.lineno = at->start.lineno;
	    at->end.column = at->start.column + strlen (at->sym->sym_text) - 1;
	 }

	 // set the symbol
	 at->sym = sym;
      }
   }
}

// ast_resolve::get_table_name --
//
// Get the real table name of a symbol.  Returns symbol name if a table,
// table name if a table alias, or 0 if neither.
//
const char * ast_resolve::get_table_name (SQL_SYM * sym)
{
   const char * table_name = 0;

   if (sym->sym_token_type == sk_table) {
      table_name = sym->sym_text;
   } else if (sym->sym_token_type == sk_alias) {
      // alias_sym is the actual table name of this table alias
      if (sym->alias_sym) {
	 table_name = sym->alias_sym->sym_text;
      }
   }
   return (table_name);
}

// ast_resolve::resolve_function --
//
// Sets the AST symbol pointer to the function symbol pointer.
//
// The same symbol name can refer to more than one symbol type
// (e.g. function, table, etc).  These different symbols are kept
// in the overload_sym linked-list in the symbol entry.
//
// Loops through the overloaded symbols for this symbol name until
// the function/procedure one is found and sets it.  If not found
// leaves it unchanged.
//
// The function arguments AST (at_args) is not used yet.
//
// TBD: Currently all overloaded procedures and functions are kept
// as one symbol.  Need to separate them into different entities.
// This would be the place to resolve an overloaded function
// based on its arguments.  However, this will involve adding the following
// to symbol handling and resolution:
//	Number of arguments passed and default arguments.
//      Symbol table entry data type (e.g. VARCHAR, INTERGER, etc).  Including
//		derived types (e.g. tab.col%TYPE).
//      Overloading data type conversion rules (e.g. a CHAR can be passed
//		to a VARCHAR argument)
//	Data types of expressions (passed as call arguments).
//	Tracking both argument data type and name in the function
//		specification.  Name needs to be kept because of the
//		association operator => which specifies which argument is
//		being passed.  For example:
//		FUNCTION foo (x INTEGER DEFAULT 0, y INTEGER DEFAULT 1)
//			RETURNS INTEGER;
//		foo(y => 5);  -- calls foo(0,5);
//	Handling cases where no function specification is known (same as
//		externref problem, but context can tell us that it is a
//		function).  A way to handle this is by keeping a general
//		function symbol (name only) as the first symbol in the list
//		of overloaded functions that share that name.  That symbol
//		can be used if there is not enough info to resolve the
//		full overloaded name.
//	Also need to handle case of argument data type unknown (externref
//		again) leading to ambigious resolution.
//
// Currently, overloading of functions is allowed only in Oracle packages
// and is not widely used.  They will be more widely used in Oracle 8 and
// should be added as part of that port.
//
void ast_resolve::resolve_function (ast_tree * at_func, ast_tree * at_args)
{
   SQL_SYM * sym = 0;

   // TBD: check arguments for overloaded functions

   // make sure symbol is a function
   if (at_func && at_func->sym) {

      // loop through symbols with same name for function
      for (sym = at_func->sym; sym; sym = sym->overload_sym) {
	 if (is_proc_symbol_kind (sym->sym_token_type)) {
	    break;
	 }
      }

      // change symbol if different
      if (sym && (sym != at_func->sym)) {
	 at_func->sym = sym;
      }
   }
}

//
// C wrappers
//

EXTERN_C void ast_resolve_columns (ast_tree * at_table, ast_tree * at_column)
{
   ast_resolve r;
   r.resolve_table (at_table, at_column);
}

EXTERN_C void ast_resolve_package (const char * package_name, ast_tree * at)
{
   ast_resolve r;
   r.resolve_package (package_name, at);
}

EXTERN_C void ast_resolve_function (ast_tree * at_func, ast_tree * at_args)
{
   ast_resolve r;
   r.resolve_function (at_func, at_args);
}

//
// Inactive symbol list
//
// List of symbol with their sym_active set to 0.  These symbols
// will not be returned during symbol table lookup.
//
// This is used for column alias symbols that are specified in
// a SELECT list, but are not active until the SELECT list has
// been fully specified.  See the column_alias rule in the parser
// (sql.y) for an example.
//
static symbol_list inactive_list;

// Set a symbol as inactive and add it to the list.
//
EXTERN_C void sym_inactive (SQL_SYM * sym)
{
   if (sym) {
      sym->sym_active = 0;
      inactive_list.add (sym);
   }
}

// Make all symbols active and delete inactive list.
//
EXTERN_C void sym_inactive_clear (void)
{
   symlist * sl;
  
   for (sl = inactive_list.head; sl; sl = sl->next) {
      sl->sym->sym_active = 1;
   }

   inactive_list.clear ();
}

/* Convert Sybase "select @i = 10" from an eq_expr to an assign_expr
 * (the parsing was easier just to treat the select spec as an
 * expression, but we need to detect assignments for DFA, TPM, etc.).
 */

EXTERN_C void map_eq_to_assign(ast_tree* at) {
    if (at && at->ank == ank_list) {
	ast_treelist* atl;
	for (atl = at->head; atl; atl = atl->next) {
	    ast_tree* ast = atl->ast;

	    // Make sure this is the case that should be mapped

	    if (ast &&
		ast->ank == ank_eq_expr &&
		ast->head &&
		ast->head->ast &&
		ast->head->ast->ank == ank_symbol &&
		ast->head->ast->sym &&
		ast->head->ast->sym->sym_token_type == sk_variable) {
		ast->ank = ank_assign_expr;
	    }
	}
    }
}
