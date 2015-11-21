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
 * S Q L _ S Y M B O L . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the symbol table routines.  Symbols are stored
 * in nested scope tables indexed by symbol name.  There is one global scope
 * table.  In addition, local symbol tables are created (pushed) when
 * a new symbol scope (program block) is entered by the parser.  These
 * local tables are removed (popped) when the parser leaves that scope
 * (end of a program block).  When the scanner finds an identifier the
 * symbol tables are searched from the bottom scope up.  The parser
 * creates newly defined symbols in the current (bottom) scope.  Symbols
 * referenced but not found are assumed to be external and are created
 * in the global symbol table.
 *
 * While local tables come and go, all symbols, whether in or out of
 * scope are kept in a master list so that they can be reported on
 * and referred to later.
 *
 * Symbols are also pointed to by the Abstract Syntax Tree (AST) built
 * by the parser.  Each symbol has an unique symbol number used for
 * creating ELS (External Language Support) IFF (Internal Format File)
 * entries after parsing (see sql_iff.C).  Also placed in the global
 * symbol table are built-in Oracle symbols (functions, procedures,
 * packages, exceptions, etc), these symbols have a symbol number of
 * zero until found in the program.   Symbols are only accessed by
 * name, pointer, or sequentually, not by number, which is only used
 * for IFF entries.
 */


//
// TBD: This was written in 1st ed. C++.  When ISO-C++ comes along,
// the implementation of this can be changed to use a map
// as well as making use of the other standard library classes.
//

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef ISO_CPP_HEADERS
#include <string.h>		// C Library of string functions
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include "sql.yacc.h"		// YACC token values
#include "sql_symbol.h"		// symbol structure and bitmasks
#include "sql_iff.h"		// IFF output routines
#include "sql_sym_enum.h"	// symbol kinds enum

// Arrays of built-in SQL symbols in sql_builtin.C
//
extern SQL_TAB sql_ansi_builtin_array [];
EXTERN_C const size_t sql_ansi_builtin_size;
extern SQL_TAB sql_oracle_builtin_array [];
EXTERN_C const size_t sql_oracle_builtin_size;
extern SQL_TAB sql_informix_builtin_array [];
EXTERN_C const size_t sql_informix_builtin_size;

// Convert passed text to uppercase, place result in passed buffer
//
static int string_upper (char *buf, const char * src, int buflen)
{
   int i;
   for (i = 0; i < (buflen - 1) && *src; ++i) {
      *(buf++) = toupper (*(src++));
   }
   *buf = '\0';
   return (i);
}

// string_to_key --
//
// Converts string to a symbol table key value.  Value also used for IFF
// entries.  Key value is used to match identifiers.  Converts text
// to uppercase except for quoted strings and names of host variables.
// 
// Values in double quotes are Oracle identifiers.  These quotes are 
// removed so that identifiers without quotes match.  Many Oracle
// utilities add the quotes when generating SQL even if they are
// not required. E.g. CREATE TABLE "foo" ("x" INTEGER);
// By removing double quotes above the following symbols will be
// resolved correctly: SELECT x FROM foo;
//
// Since double quotes are added around symbol names in the IFF,
// having symbol names with embedded spaces is not a problem.
//
static int string_to_key (char *buf, const char * src, int buflen)
{
   int i;
   int inquote = 0;
   int sc;

   // Just copy SQL string (starts with single quote)
   // Don't change host variable either (starts with colon)
   if (*src == '\'' | *src == ':') {
      strncpy (buf, src, buflen);
      *(buf + (buflen - 1)) = '\0';
   } else {
      // Convert to uppercase, unless quoted (w/ double quotes)
      // then copy w/o quotes.
      // Note: No embedded double quotes allowed in SQL?
      //
      // Note that double quotes in SQL are used to delimit identifiers
      // that may contain spaces, etc.
      for (i = 0; i < (buflen - 1) && *src; ++i) {
	 sc = *(src++);
	 if (inquote) {
	    if (sc == '"' && dbvendor != DBVENDOR_SYBASE) {
	       inquote = 0;
	    } else {
	       *(buf++) = sc;
	    }
	 } else if (sc == '"' && dbvendor != DBVENDOR_SYBASE) {
	    inquote = 1;
	 } else {
	    *(buf++) = toupper (sc);
	 }
      }
      *buf = '\0';
   }
   return (0);
}

// Check if a symbol represents a procedural element
//
// In Oracle a procedure is a function that returns nothing.
//
EXTERN_C int is_proc_symbol_kind (sym_kind sk)
{
   return (sk == sk_procedure || sk == sk_function);
}

// Check if a symbol is a data type
EXTERN_C int is_type_symbol_kind (sym_kind sk)
{
   return (sk == sk_record_type || sk == sk_ref_type ||
	   sk == sk_table_type || sk == sk_type || sk == sk_plsql_table);
}

// Check if symbols are of the same kind
// regard procedure and function as the same kind
EXTERN_C int is_symbol_kind_match (sym_kind sk1, sym_kind sk2)
{
   int retval = 0;
   if ((sk1 == sk2) ||
       (is_proc_symbol_kind (sk1) && is_proc_symbol_kind (sk2))) {
      retval = 1;
   }
   return (retval);
}

//
// Symbol allocation block size.
//
static const int symbol_block = 1000;

// master symbol list class
//
// allocates blocks of symbols and allows for single-threaded
// iteration.
class sql_symbol_list {
private:
   SQL_LIST * head;
   SQL_LIST * tail;

   // for looping through list
   SQL_LIST * current_list;
   unsigned int current_item;

   // singleton class -- don't implement copy or assignment
   sql_symbol_list (const sql_symbol_list& rhs);
   sql_symbol_list& operator= (const sql_symbol_list& rhs);

public:
   sql_symbol_list ();
   ~sql_symbol_list ();
   SQL_SYM * new_symbol (const char*, sym_kind, int);
   int clear (void);
   int init_list (void);
   SQL_SYM * next_sym (void);
};

sql_symbol_list::sql_symbol_list () : head (0), tail (0) {}

sql_symbol_list::~sql_symbol_list ()
{
   // delete master symbol table
   clear ();
}

// Creates a new symbol
//
// Allocates a new block of symbols if needed.
//
SQL_SYM * sql_symbol_list::new_symbol (
	const char* text,
	sym_kind token_type,
	int symbol_number)
{
   SQL_LIST * p;
   SQL_SYM * sym = 0;
   int len;

   // Allocate new block of symbols if needed
   if ( (! tail) || 
       (tail->current_no >= symbol_block) ) {
      p = new SQL_LIST;
      if (p) {
	 p->syms = new SQL_SYM [symbol_block];
	 p->current_no = 0;
	 p->next = 0;
      
	 // link to current block list
	 if (! head ) {
	    p->prev = 0;
	    head = tail = p;
	 } else {
	    p->prev = tail;
	    tail->next = p;
	    tail = p;
	 }
      }
   } else {
      p = tail;
   }

   // p is ptr to current symbol block
   if (p) {
      // get new symbol from block
      sym = &(p->syms [p->current_no++]);

      // allocate and copy text
      len = strlen (text);
      sym->sym_text = new char [len+1];
      strcpy (sym->sym_text, text);

      // allocate hash/output key
      sym->sym_key = new char [len+1];

      // convert text to uppercase for key
      //
      // but leave alone for strings and filenames.
      if ((token_type == sk_string) ||
	  (token_type == sk_file) ||
	  (*text == '\'')) {
	 strcpy (sym->sym_key, text);
      } else {
	 string_to_key (sym->sym_key, text, len+1);
      }

      // Initialize fields
      sym->full_sym = 0;
      sym->alias_sym = 0;
      sym->proc_sym = 0;
      sym->overload_sym = 0;
      sym->ref_count = 0;
      sym->proc_block = 0;
      sym->sym_token_type = token_type;
      sym->sym_number = symbol_number;
      sym->attr_mask = ATTRMASK_NONE;
      sym->sym_defined = 0;
      sym->sym_active = 1;
      sym->sym_specfile = 0;
      sym->sym_replace = 0;
   }

   return (sym);
}

// delete all symbols and symbol blocks
int sql_symbol_list::clear (void)
{
   SQL_LIST * p;
   SQL_LIST * pnext;
   SQL_SYM * sym;
   int i;

   for (p = head; p; p = pnext) {
      pnext = p->next;

      for (i = 0; i < p->current_no; ++i) {
	 sym = &(p->syms [i]);
	 if (sym->sym_text) {
	    delete [] sym->sym_text;
	 }
	 if (sym->sym_key) {
	    delete [] sym->sym_key;
	 }
      }
      delete [] p->syms;
      delete p;
   }
   head = tail = 0;
   return (0);
}

// Routines to loop through full symbol list.
//
// There is no iterator passed back so only one
// loop can be active at a time.  If you need more, change to
// pass back an iterator with current_list and current_item
// to be used in next_sym().
//
int sql_symbol_list::init_list (void)
{
   current_list = head;		// first block
   current_item = 0;		// index (first symbol in block)
   return (0);
}

// Return next symbol or 0 if no more
//
// Checks for end of block and switches to next block
//
SQL_SYM * sql_symbol_list::next_sym (void)
{
   SQL_SYM * sym = 0;
   if (current_list) {
      // check for end of block and switch to next
      if (current_item >= current_list->current_no) {
	 current_item = 0;
	 current_list = current_list->next;
      }
      if (current_list) {
	 // set symbol ptr and increment index
	 sym = &(current_list->syms [current_item++]);
      }
   }
   return (sym);
}

/////////////////////////////
// instantiate above class //
/////////////////////////////
static sql_symbol_list symlist;

// SQL wrapper to Discover hash routines in sym_hash.C
class symhash;

extern "C" symhash * symhash_create ();
extern "C" SQL_SYM * symhash_lookup (const char * name, symhash * h);
extern "C" void symhash_add (SQL_SYM * sym, symhash * h);

//
// sql_symbol class
//
// Class to handle a hash table of symbols, including functions
// to add and lookup symbols.
//
class sql_symbol {
private:
   symhash *sql_hashtable;
   char txtbuf [1024];		// buffer for key lookup

   // private, not implemented
   sql_symbol (const sql_symbol& rhs);
   sql_symbol& operator= (const sql_symbol& rhs);

public:
   sql_symbol (void);
   ~sql_symbol ();
   int clear (void);
   SQL_SYM* lookup (const char* text);
   SQL_SYM* lookup_by_type (const char* text, sym_kind token_type);
   SQL_SYM * redefine_or_overload (SQL_SYM *sym,
	 const char* text,
	 sym_kind token_type,
	 int symbol_number);

   SQL_SYM* add (const char* text, sym_kind token_type, int symbol_number);
};

// Create symbol hash table
sql_symbol::sql_symbol (void) {
   sql_hashtable = symhash_create ();
}

sql_symbol::~sql_symbol () {
   clear ();
}

int sql_symbol::clear (void) {
   // No delete - need to keep symbol table around

   // TBD: but should delete hash table?
   // should add symhash_delete() to sym_hash.C
   // and the following code here:
   // symhash_delete (sql_hashtable);
   // sql_hashtable = 0;
   return (0);
}

// sql_symbol::lookup --
//
// find the symbol entry associated with a symbol string
// or return 0 if string is not a symbol.
//
// Uses hash lookup on symbol table.
//
SQL_SYM* sql_symbol::lookup (const char* text) {

   string_to_key (txtbuf, text, 1024);
   SQL_SYM * sym = symhash_lookup (txtbuf, sql_hashtable);
   return (sym);
}

// sql_symbol::lookup_by_type --
//
// lookup by name and type (must match both)
//
// finds symbol in hash table by name and then loops through
// overload linked-list until type matches or not found.
// Returns 0 if not found
//
SQL_SYM* sql_symbol::lookup_by_type (const char* text, sym_kind token_type)
{
   SQL_SYM * sym = lookup (text);
   while (sym && sym->sym_token_type != token_type) {
      sym = sym->overload_sym;
   }
   return (sym);
}
   
// sql_symbol::redefine_or_overload --
//
// Called whenever a symbol is added and there is already a
// symbol with the same name in the same scope.  Checks if
// the symbol is the same or should be redefined or overloaded.
//
// Arguments:
//	sym --			existing symbol
//	text --			symbol name
//	token_type --		new symbol kind (e.g. function, table, etc.)
//	symbol_number --	number to assign to new symbol
//
// Returns pointer to existing symbol if the same or new symbol
// in the case of overload.
//
SQL_SYM * sql_symbol::redefine_or_overload (SQL_SYM *sym,
		    const char* text,
		    sym_kind token_type,
		    int symbol_number)
{
   // return value -- default to existing symbol
   SQL_SYM * sym_ret = sym;

   if (sym) {
      // Check for built-in symbol (or symbol from specification file)
      // These symbols have a zero symbol number.  If type matches
      // this is a reference to that symbol.  Assign number.
      if ( (sym->sym_number == 0) &&
	  is_symbol_kind_match (sym->sym_token_type, token_type)) {
	 sym->sym_number = symbol_number;

      // Check for symbol type mismatch, but ignore if new symbol is externref
      //
      } else if ((! is_symbol_kind_match (sym->sym_token_type, token_type)) &&
		 (token_type != sk_externref)) {

	 if (sym->sym_token_type == sk_externref) {
	    // existing symbol type unknown (externref).
	    // Now we know type, set existing symbol.
	    sym->sym_token_type = token_type;

	 } else {
	    // check for overloading of symbol name (different type)
	    if (is_type_symbol_kind (token_type) &&
		       is_type_symbol_kind (sym->sym_token_type)) {
	       // both are typedefs
	       // redefine to new type
	       // (TBD: not clear what to do here)
	       sym->sym_token_type = token_type;

	       // TBD: no need to do this?
	       sym->sym_number = symbol_number;
	    } else {
	       // Symbols are of different type
	       // check for existing overload symbol
	       // by looping through linked-list
	       while (sym_ret = sym_ret->overload_sym) {
		  if (is_symbol_kind_match(sym_ret->sym_token_type,
					   token_type)) {
		     break;
		  }
	       }

	       // if not found
	       // create a new overloaded symbol
	       if ( ! sym_ret) {
		  sym_ret = symlist.new_symbol (text,
						token_type,
						symbol_number);

		  // TBD: put in Out-of-memory throw, etc. in new_symbol
		  // instead of these lame null pointer tests
		  //
		  if (sym_ret) {
		     // link to head of existing symbol's overload list
		     // (sym pointer already in hash table and used in parser
		     //  productions (ast and sym) so cannot make new
		     //  symbol the primary symbol)
		     //
		     sym_ret->overload_sym = sym->overload_sym;
		     sym->overload_sym = sym_ret;
		  }
	       }
	       // sym_ret now points to correct overloaded symbol
	    }
	 }
      } else {
	 // types match or new symbol is externref
	 // ok -- no change (return existing symbol)
      }
   }
   return (sym_ret);
}

// sql_symbol::add --
//
// Add a new symbol.  Returns pointer to symbol.
//
// Checks if symbol already exists.  If not creates new symbol
// and adds it to the hash table.
//
SQL_SYM* sql_symbol::add (const char* text,
			  sym_kind token_type,
			  int symbol_number) {

   SQL_SYM* sym = lookup (text);
   if (sym) {
      sym = redefine_or_overload (sym, text, token_type, symbol_number);
   } else {
      sym = symlist.new_symbol (text, token_type, symbol_number);
      symhash_add (sym, sql_hashtable);
   }
   return (sym);
}

//
// sql_scope_entry class
//
// Class that handles the symbols and other information
// for a single symbol scope (e.g. global, package, function, block, 
// statement, etc.).
//
// Contains pointer to previous (outer) and pointer to next (inner)
// scopes on the scope stack.
//
typedef class sql_scope_entry SQL_SCOPE;

class sql_scope_entry {

private:
   // not implemented
   sql_scope_entry (const sql_scope_entry& rhs);
   sql_scope_entry& operator= (const sql_scope_entry& rhs);

public:
   sql_symbol symbols;			// symbols in this scope
   char * name;				// scope name: package, func, etc.
   const char * table_name;		// current table name
   unsigned int level;			// scope depth
   SQL_SYM * proc_sym;			// if inside function
   unsigned int proc_block;		// count of scopes inside function
   struct sql_scope_entry * next;	// next (inner) scope
   struct sql_scope_entry * prev;	// prev (outer) scope

   sql_scope_entry (const char * scope_name = 0);
   ~sql_scope_entry ();
};

// Constructor
//
// Name is name of function, package, label, cursor.  It can be used
// to resolve scope.
//
// TBD: Use scope name to resolve symbols.
//
sql_scope_entry::sql_scope_entry (const char * scope_name /* = 0 */) : 
	table_name (0), 
	level (0), 
	proc_sym (0),
	proc_block (0),
	next (0),
	prev (0)
{
   if (scope_name) {
      int len = strlen (scope_name) + 1;
      name = new char [len];
      strcpy (name, scope_name);
   } else {
      name = 0;
   }
}

sql_scope_entry::~sql_scope_entry ()
{
   if (name) {
      delete [] name;
   }
}

//
// sql_scope class
//
// Class that tracks all symbol scopes.
//
// Includes global scope and local scope stack.  Keeps track of current scope
// depth (level) and scope block count for current function.  Responsible
// for incrementing symbol number.
//
// Has routines to push and pop scopes and to add symbols and information
// to and lookup symbols from global, local, and previous scopes.
//
class sql_scope {

private:
   // current symbol number
   // starts at 1 and increases for the life of the run
   // symbol number 0 is for built-in symbols not yet found
   int symbol_number;

   // global symbols
   SQL_SCOPE global_scope;

   // current scope level (0 == global)
   unsigned int scope_level;

   // current block id for current procedure or function.
   // Unlike level, this keeps incrementing (inside same function)
   // to distinguish symbols that occur in different blocks
   // Example:
   //					-- level 0 (global)
   //	FUNCTION foo ...
   //	BEGIN				-- block 0, level 1
   //		x INTEGER;
   //		BEGIN			-- block 1, level 2
   //			x INTEGER;
   //		END;
   //		BEGIN			-- block 2, level 2
   //			x INTEGER;
   //
   unsigned int proc_block_id;

   // scope levels list 
   SQL_SCOPE* scope_head;
   SQL_SCOPE* scope_tail;

   // has initialization occurred with built-in database vendor symbols
   int initialized;

   // add symbol to passed scope
   SQL_SYM * scope_add (SQL_SCOPE * pscope,
		      const char * text,
		      sym_kind token_type,
		      int sym_number = 0);

   // lookup scope of a symbol
   SQL_SCOPE* lookup_symbol_scope (SQL_SYM * find_sym);

   // load built-in symbols from static table
   int load_builtin (SQL_TAB *table_ptr, size_t table_size);

   // not implemented
   sql_scope (const sql_scope& rhs);
   sql_scope& operator= (const sql_scope& rhs);

public:
   sql_scope ();
   ~sql_scope ();
   int reset (void);
   int clear (void);
   int push (const char * name = 0, SQL_SYM * sym = 0);
   int pop (void);
   unsigned int level (void);
   SQL_SYM* global_lookup (const char* text);
   SQL_SYM* lookup (const char* text);
   int assign_symbol_number (SQL_SYM * sym);
   SQL_SYM* global_add (const char* text, sym_kind token_type);
   SQL_SYM* add (const char* text, sym_kind token_type);
   SQL_SYM* add_local (SQL_SYM * full_sym, const char* text);
   SQL_SYM* add_symbol_scope (SQL_SYM * scope_sym,
				      const char* text,
				      sym_kind token_type);
   SQL_SYM* add_previous (const char* text, sym_kind token_type);
   SQL_SYM* add_previous_local (SQL_SYM * full_sym, const char* text);
   SQL_SYM * procedure_symbol (void);
   unsigned int procedure_block (void);
   int set_table_name (const char * tabname);
   const char * get_table_name (void);
   int initialize (int dbtype = DBVENDOR_ORACLE);
};


sql_scope::sql_scope (void) :
        symbol_number (1),
	scope_level (0),
	proc_block_id (0),
	scope_head (0),
	scope_tail (0),
	initialized (0)
{
   // for global level: built-in tables, functions and packages

   load_builtin (&(sql_ansi_builtin_array [0]), sql_ansi_builtin_size);
}

sql_scope::~sql_scope () {
   clear ();
}

// sql_scope::scope_add --
//
// Add symbol to passed scope, or global scope if no scope passed.
//
// If symbol number is zero, assigns next symbol_number.
//
// Returns pointer to symbol.
//
SQL_SYM * sql_scope::scope_add (SQL_SCOPE * pscope,
		     const char * text,
		     sym_kind token_type,
		     int sym_number /* = 0 */) {
   SQL_SYM * sym;
   if (pscope) {
      // local scope
      if (sym_number) {
	 // use existing symbol number
	 // (might be another name for an existing symbol)
	 sym = pscope->symbols.add (text, token_type, sym_number);
      } else {
	 // assign next symbol_number
	 sym = pscope->symbols.add (text, token_type, symbol_number++);
      }
   } else {
      // global scope
      if (sym_number) {
	 // use existing symbol number
	 sym = global_scope.symbols.add (text, token_type, sym_number);
      } else {
	 // global_add assigns next symbol_number
	 sym = global_add (text, token_type);
      }
   }

   return (sym);
}

// sql_scope::initialize --
//
// Adds database vendor built-in symbols (tables, functions, packages, etc)
// to global scope.  This cannot occur at construction because the 
// database vendor is not known until the command line is processed.
//
int sql_scope::initialize (int dbtype) {
   int ret = 0;
   if (initialized) {
      ret = -1;
      iff_error ("Database vendor already specified");
   } else {
      if (dbtype == DBVENDOR_ORACLE) {
	 load_builtin (&(sql_oracle_builtin_array [0]),
		       sql_oracle_builtin_size);
      } else if (dbtype == DBVENDOR_INFORMIX) {
	 load_builtin (&(sql_informix_builtin_array [0]),
		       sql_informix_builtin_size);
      } else if (dbtype == DBVENDOR_SYBASE) {
         /* nothing for the moment */
      } else {
	 ret = -1;
	 iff_error ("Unknown database vendor");
      }

      if (ret == 0) {
	 initialized = 1;
      }
   }
   return (ret);
}


// sql_scope::load_builtin --
//
// Add symbols from passed table of symbols to global scope.
//
int sql_scope::load_builtin (SQL_TAB * tp, size_t tsize) {
   int i;
   // loop through static table
   for (i=0; i < tsize; ++i, ++tp) {
      global_scope.symbols.add (tp->sym_text, tp->sym_token_type, 0);
   }
   return (0);
}

// delete local symbols and restart at symbol number 1
int sql_scope::reset (void) {
   int ret = clear ();
   symbol_number = 1;
   return (ret);
}

// delete all local symbols
// all globals remain
int sql_scope::clear (void) {
   SQL_SCOPE* p;
   while (scope_head) {
      p = scope_head->next;
      delete scope_head;
      scope_head = p;
   }
   scope_tail = 0;

   scope_level = 0;
   proc_block_id = 0;
   return (0);
}

// sql_scope::push --
//
// Push a new scope.
//
// Arguments:
//	name --		scope name, if any
//	sym --		function symbol, only if start of new function.
//
// Increments depth (scope_level).  If inside a function, increments
// procedure block count (becomes scope's proc_block) and copies function
// symbol.
//
int sql_scope::push (const char * name /* = 0 */, SQL_SYM * sym /* = 0 */)
{
   // Create a new scope entry
   SQL_SCOPE* new_scope = new SQL_SCOPE (name);

   // increment depth
   new_scope->level = ++scope_level;

   new_scope->next = 0;		// TBD: done in SQL_SCOPE constructor

   // add to double linked-list of local scopes
   if ( ! scope_head ) {
      // top level local
      new_scope->prev = 0;	// TBD: done in SQL_SCOPE constructor
      scope_head = new_scope;
      scope_tail = new_scope;
   } else {
      new_scope->prev = scope_tail;
      scope_tail->next = new_scope;
      scope_tail = new_scope;
   }

   // set current procedure
   new_scope->proc_sym = sym;

   if (sym) {
      // new proc - reset block id
      proc_block_id = 0;
   } else {
      // check for new sub-block of same procedure:
      // increment block_id and copy procedure symbol
      if (new_scope->prev) {
	 if (new_scope->prev->proc_sym) {
	    new_scope->proc_sym = new_scope->prev->proc_sym;
	    new_scope->proc_block = ++proc_block_id;
	 }
      }
   }
   return (0);
}

// sql_scope::pop --
//
// Pop a local scope
//
// Decrements depth (scope_level) and deletes scope.  Resets procedure
// block count if new function.  Note that in SQL functions can be
// nested.
//
// Prints error if attempt is made to pop empty stack.
//
int sql_scope::pop (void) {
   int ret = 0;
   // non-empty list
   if (scope_head) {

      // decrement scope depth counter
      --scope_level;

      // only one entry on list
      if (scope_head == scope_tail) {
	 delete scope_tail;
	 scope_head = 0;
	 scope_tail = 0;

	 // now in global scope reset procedure block count
	 proc_block_id = 0;

      } else {
	 // remove tail
	 SQL_SCOPE * p = scope_tail->prev;
	 SQL_SYM * sym = scope_tail->proc_sym;
	 delete scope_tail;
	 scope_tail = p;
	 scope_tail->next = 0;

	 // different procedure - zero procedure block id
	 //
	 // Note: if proc_sym == 0 we are no longer in a function
	 // (could be back in the package),
	 // but if a pop results in a different function, then
	 // we were in a nested function and the procedure block id
	 // needs to be cleared so that the main function scope's symbols
	 // (which come after nested functions) have the correct
	 // block id.
	 //
	 // TBD: Could there be a case where the proc_block_id should
	 // be restored (proc_block_id = scope_tail->proc_block) instead
	 // of cleared after a pop from a nested function?
	 //
	 if ((! scope_tail->proc_sym) ||
	     (sym != scope_tail->proc_sym)) {
	    proc_block_id = 0;
	 }
      }
   } else {
      iff_error ("Attempt to pop empty scope");
      ret = -1;
   }
   return (ret);
}

// get the current scope depth
unsigned int sql_scope::level (void) {
   return (scope_level);
}

// sql_scope::global_lookup --
//
// Lookup a symbol by name in the global scope
//
// Returns symbol pointer or 0
//
SQL_SYM* sql_scope::global_lookup (const char* text) {
   return (global_scope.symbols.lookup (text));
}

// sql_scope::lookup --
//
// Find the symbol entry associated with a symbol string
// or return 0 if string is not a symbol.
//
// Checks from inner-most to outer-most local scope
// then checks global scope.
//
SQL_SYM* sql_scope::lookup (const char* text) {

   SQL_SYM* sym = 0;
   SQL_SCOPE* pscope;

   for (pscope = scope_tail; pscope; pscope = pscope->prev) {
      if ((sym = pscope->symbols.lookup (text)) != 0) {
	 // ignore if symbol is not active -- keep looking
	 if (sym->sym_active) {
	    break;
	 }
      }
   }

   // not local, check global scope
   if ( ! sym ) {
      sym = global_lookup (text);
   }
   return (sym);
}

// sql_scope::lookup_symbol_scope --
//
// Find the scope pointer of a symbol
//
// Checks from inner-most to outer-most local scope.
// Returns zero for global scope.
//
// Needed for adding a symbol to another symbol's scope.
//
SQL_SCOPE * sql_scope::lookup_symbol_scope (SQL_SYM * find_sym) {

   SQL_SYM* sym = 0;
   SQL_SCOPE* pscope = 0;
   if (find_sym) {
      for (pscope = scope_tail; pscope; pscope = pscope->prev) {
	 if ((sym = pscope->symbols.lookup (find_sym->sym_text)) != 0) {
	    if (sym->sym_active && (sym == find_sym)) {
	       break;
	    }
	 }
      }

      // not local
      if (sym != find_sym) {
	 // zero means global scope
	 pscope = 0;
      }
      
   }
   return (pscope);
}

// sql_scope::assign_symbol_number --
//
// Assigns next symbol number to a symbol.
//
// Used for symbols with a symbol number of zero (unreferenced)
// which is the case for symbols specified before
// the main parsing phase (either in the built-in table
// or in a specification file).  It is the caller's responsibility to
// check that the symbol number is zero before calling this
// routine.
//
// This is not the only place where symbol_number is assigned
// or incremented.  See also sql_scope::scope_add() and
// sql_scope::global_add().
//
int sql_scope::assign_symbol_number (SQL_SYM * sym) {
   int ret = 0;
   if (sym) {
      ret = sym->sym_number = symbol_number++;
   }
   return (ret);
}

// sql_scope::global_add --
//
// Add a symbol to the global scope.  Checks if the symbol is already
// in the symbol table, and if so checks if is the same or being redefined
// or overloaded.
//
// Returns the symbol pointer.
//
SQL_SYM* sql_scope::global_add (const char* text, sym_kind token_type) {

   SQL_SYM * sym;

   sym = global_lookup (text);
   if (sym) {
      // existing symbol
      if (sym->sym_number) {
	 // symbol already referenced -- use same symbol number
	 sym = global_scope.symbols.redefine_or_overload (sym, text,
							  token_type,
							  sym->sym_number);
      } else {
	 // symbol unreferenced -- assign new symbol_number
	 sym = global_scope.symbols.redefine_or_overload (sym, text,
							  token_type,
							  symbol_number++);
      }
   } else {
      // new symbol -- assign new symbol_number
      sym = global_scope.symbols.add (text, token_type, symbol_number++);
   }

   return (sym);
}

// sql_scope::add --
//
// Add a symbol to current scope.
// The current scope may be the global scope (scope_tail is zero).
//
SQL_SYM* sql_scope::add (const char* text, sym_kind token_type) {
   return (scope_add (scope_tail, text, token_type));
}

// sql_scope::add_local --
//
// Add symbol to current scope based on another symbol.
//
// Used for the local scope name of a global symbol
// E.g. "func1" for "package1.func1" within "package1".
//
SQL_SYM* sql_scope::add_local (SQL_SYM * full_sym, const char* text) {
   SQL_SYM * sym;
   // Add symbol name using full symbol's type and number
   sym = scope_add (scope_tail,
		    text,
		    full_sym->sym_token_type,
		    full_sym->sym_number);

   // set full_sym to symbol with full name
   if (sym && (sym != full_sym)) {
      sym->full_sym = full_sym;
   }

   return (sym);
}

// sql_scope::add_symbol_scope --
//
// Add symbol to same scope as another symbol.
//
// Used for adding full field names their record's scope.
//
SQL_SYM* sql_scope::add_symbol_scope (SQL_SYM * scope_sym,
				      const char* text,
				      sym_kind token_type) {

   // if scope is 0 then global scope
   SQL_SCOPE * pscope = lookup_symbol_scope (scope_sym);
   return (scope_add (pscope, text, token_type));
}

// sql_scope::add_previous --
//
// Add symbol to previous scope.
//
// Used for members (e.g. fields of a record)
//
SQL_SYM* sql_scope::add_previous (const char* text, sym_kind token_type) {
   SQL_SCOPE * pscope = 0;	// global scope

   // previous (next outer) scope
   if (scope_tail && scope_tail->prev) {
      pscope = scope_tail->prev;
   }
   
   return (scope_add (pscope, text, token_type));
}

// sql_scope::add_previous_local --
//
// Add symbol to previous scope based on another symbol.
//
// Combines add_previous() with add_local() functionality.
//
SQL_SYM* sql_scope::add_previous_local (SQL_SYM * full_sym, const char* text) {
   SQL_SYM * sym;
   SQL_SCOPE * pscope = 0;		// global scope

   // previous (next outer) scope
   if (scope_tail && scope_tail->prev) {
      pscope = scope_tail->prev;
   }

   // add symbol name based on full symbol's type and number
   sym = scope_add (pscope,
		    text,
		    full_sym->sym_token_type,
		    full_sym->sym_number);

   // set full_sym to symbol with full name
   if (sym && (sym != full_sym)) {
      sym->full_sym = full_sym;
   }

   return (sym);
}

// sql_scope::procedure_symbol --
//
// Get current procedure (or function) symbol pointer.
// Returns 0 if not in procedure.
//
SQL_SYM * sql_scope::procedure_symbol (void) {
   SQL_SYM * sym = 0;
   // must be in a local scope (not global) to be in a procedure.
   if (scope_tail) {
      sym = scope_tail->proc_sym;
   }
   return (sym);
}

// sql_scope::procedure_block --
//
// Get current procedure (or function) symbol block id
//
// Returns 0 for global or procedure scope.
//
// Used for distinguishing local symbols with the same name, but
// defined in different blocks.
//
unsigned int sql_scope::procedure_block (void) {
   unsigned int id = 0;		// global scope (not in a procedure)
   // Get current block count
   if (scope_tail) {
      id = scope_tail->proc_block;
   }
   return (id);
}

// sql_scope::set_table_name --
//
// Sets the table name currently active in the current scope.
//
int  sql_scope::set_table_name (const char * tabname)
{
   if (scope_tail) {
      scope_tail->table_name = tabname;
   } else {
      global_scope.table_name = tabname;
   }
   return (0);
}

// sql_scope::get_table_name --
//
// Gets the table name currently active in the current scope.
//
// Used inside statement clauses (such as the column list of an INSERT)
// in which the table has been set earlier.
//
const char *  sql_scope::get_table_name (void)
{
   const char * tabname;
   if (scope_tail) {
      tabname = scope_tail->table_name;
   } else {
      tabname = global_scope.table_name;
   }
   return (tabname);
}

/////////////////////////////
// instantiate scope class //
/////////////////////////////
static sql_scope sym_scope;

//
// C wrappers
//

// C function wrapper for initialize
EXTERN_C int sql_symbols_initialize (int dbtype)
{
   return (sym_scope.initialize (dbtype));
}

// C function wrapper for lookup
EXTERN_C SQL_SYM* sql_symbol_lookup (const char* text)
{
   return (sym_scope.lookup (text));
}

// C function wrapper for global lookup
EXTERN_C SQL_SYM* sql_symbol_global_lookup (const char* text)
{
   return (sym_scope.global_lookup (text));
}

// C function wrapper for add_previous
EXTERN_C SQL_SYM* sql_symbol_add_prev (const char* text, sym_kind token_type)
{
   return (sym_scope.add_previous (text, token_type));
}

// C function wrapper for add
EXTERN_C SQL_SYM* sql_symbol_add (const char* text, sym_kind token_type)
{
   return (sym_scope.add (text, token_type));
}

// C function wrapper for add_local
EXTERN_C SQL_SYM* sql_symbol_local (SQL_SYM * full_sym, const char* text)
{
   return (sym_scope.add_local (full_sym, text));
}

// C function wrapper for add_symbol_scope
EXTERN_C SQL_SYM* sql_symbol_add_to_symbol_scope (SQL_SYM * scope_sym,
				      const char* text,
				      sym_kind token_type)
{
   return (sym_scope.add_symbol_scope (scope_sym, text, token_type));
}

// C function wrapper for add_previous_local
EXTERN_C SQL_SYM* sql_symbol_prev_local (SQL_SYM * full_sym,
					 const char* text)
{
   return (sym_scope.add_previous_local (full_sym, text));
}

// C function wrapper for global_add
EXTERN_C SQL_SYM* sql_symbol_extern (const char* text, sym_kind token_type)
{
   return (sym_scope.global_add (text, token_type));
}

// C function wrapper for starting a new deeper scope for symbols
EXTERN_C void sql_push_scope (const char* name, SQL_SYM * proc_sym)
{
   sym_scope.push (name, proc_sym);
}

// C function wrapper for end previous scope for symbols
EXTERN_C void sql_pop_scope (void)
{
   sym_scope.pop ();
}


// C wrapper for get current scope level (0 == global)
EXTERN_C unsigned int sql_scope_level (void)
{
   return (sym_scope.level ());
}

// C wrapper for get current procedure or function symbol
EXTERN_C SQL_SYM * sql_scope_proc_symbol (void)
{
   return (sym_scope.procedure_symbol ());
}

// C wrapper for get current procedure or function depth
EXTERN_C unsigned int sql_scope_proc_block (void)
{
   return (sym_scope.procedure_block ());
}

// C wrapper for set current table name
EXTERN_C int sql_scope_set_table_name (const char * tabname)
{
   return (sym_scope.set_table_name (tabname));
}

// sql_scope_clear_table_name --
//
// Clear current table name (set to 0) in current scope.
//
EXTERN_C int sql_scope_clear_table_name (void)
{
   return (sym_scope.set_table_name (0));
}

// C wrapper for get current table name
EXTERN_C const char *sql_scope_table_name (void)
{
   return (sym_scope.get_table_name ());
}

// C wrapper for assign next symbol number
//
EXTERN_C void sql_symbol_number_assign (SQL_SYM * sym)
{
   sym_scope.assign_symbol_number (sym);
}

//
// Routines to loop through full symbol table.
// Table has all symbols, including those which have gone out of scope.
//

// sql_symbol_init_list -- 
//
// Reset internal counter to first symbol.
//
EXTERN_C void sql_symbol_init_list (void)
{
   symlist.init_list ();
}

// sql_symbol_next --
//
// Return the next symbol or zero if at end of symbol table.
//
EXTERN_C SQL_SYM * sql_symbol_next (void)
{
   return (symlist.next_sym ());
}

// sql_symbols_reset --
//
// Set all symbol numbers to zero (means unreferenced).
//
// Used after specification phase (processing command line
// -include and -symbols files for symbol names and types).
//
EXTERN_C void sql_symbols_reset (void)
{
   SQL_SYM * sym;

   // set all symbol numbers to zero to indicate unreferenced
   symlist.init_list ();
   while (sym = symlist.next_sym ()) {
      sym->sym_number = 0;
   }

   // clear local scopes and start at first symbol number
   sym_scope.reset ();
}

