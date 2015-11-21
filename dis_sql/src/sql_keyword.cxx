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
 * S Q L _ K E Y W O R D . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Dec. 1997 Mark B. Kaminsky
 *
 * Routines for determining if a token is a keyword or an identifier.
 * This is done by checking a table of keywords and the current
 * context.  The current context is set by the scanner and parser
 * by calling various routines in this module such as start_of_statement
 * and current command.
 */

#if defined(linux2) || defined(hp10)
#include <stdlib.h>
#endif

#include <search.h>		// C Library of Knuth search functions
#ifndef ISO_CPP_HEADERS
#include <string.h>		// C Library of string functions
#include <ctype.h>		// C Library of character type functions
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include "sql_all.h"		// EXTERN_C
#include "sql_keyword.h"	// keyword table structs
#include "sql_symbol.h"		// database vendor
#include "sql_scanner.h"       	// esql_mode

#include "sql.yacc.h"		// YACC generated list of token values

// In sql.y -- check parser if keyword is valid in present state
EXTERN_C int yykeywordcheck (int tokenvalue);

// simple double linked-list node to hold keymode value
// (current C++ compiler doesn't have standard container classes.
// Feel free to flush this when it does)
typedef class sql_keymode * sql_keymode_ptr;

class sql_keymode {
 public:
   int mode;
   sql_keymode_ptr next;
   sql_keymode_ptr prev;

   sql_keymode (int init_mode, sql_keymode_ptr prev_ptr = 0);
   ~sql_keymode ();
};

sql_keymode::sql_keymode (int init_mode,
			  sql_keymode_ptr prev_ptr )
	: mode (init_mode), prev (prev_ptr), next (0)
{
   // link to previous node
   if (prev) {
      if (prev->next) {
	 next = prev->next;
      }
      prev->next = this;
   }
}

sql_keymode::~sql_keymode ()
{
   // remove current node by linking previous node to next node
   if (prev) {
      prev->next = next;
   }
   if (next) {
      next->prev = prev;
   }
}

// class to keep stack of keyword modes
//
// Mode operations are: get, push, and pop.
// The base mode can be set.
// The whole stack can also be cleared.
//
// At the start of each sql statement the mode starts
// at the base mode.  While a complex sql statement is being
// parsed, new modes may be pushed (to become the current
// keyword mode), and popped.  The stack is only for a
// single sql statement and is cleared before each new sql
// statement.
//
// For example in PL/SQL:
//	IF cursor_name%FOUND THEN	-- ALL mode (base)
//		SELECT			-- push SQL mode
//			... ;		-- eos: clear to base mode
//
class sql_keyword_state {
 private:
   int base_mode;
   sql_keymode_ptr head;
   sql_keymode_ptr tail;

 public:
   sql_keyword_state (int mode = SQLKEY_ALL);
   ~sql_keyword_state ();
   void clear (void);
   void push (int mode);
   int pop (void);
   int get_mode (void);
   void set_base_mode (int mode);
};

sql_keyword_state::sql_keyword_state (int mode)
	: base_mode (mode), head (0), tail (0)
{
}

sql_keyword_state::~sql_keyword_state ()
{
   clear ();
}

// clear the key mode stack, leave basemode unchanged
void sql_keyword_state::clear (void)
{
  sql_keymode_ptr nexthead; 
   for ( ; head; head = nexthead) {
      nexthead = head->next;
      delete head;
   }
   tail = 0;
}

void sql_keyword_state::push (int mode)
{
   sql_keymode_ptr p = new sql_keymode (mode, tail);
   tail = p;
   if ( ! head ) {
      head = tail;
   }
}

int sql_keyword_state::pop (void)
{
   int mode;
   sql_keymode_ptr p;

   // Trying to pop an empty list is normal because the scanner
   // will clear the list at the end-of-statement semicolon
   // before the parser reduces and calls pop.
   //
   // For example:
   //	INSERT INTO tab1 (x)		-- parser (INSERT): push SQLKEY_SQL
   //		SELECT x FROM tab2	-- parser (SELECT): push SQLKEY_SQL
   //		;			-- scanner: start_of_statement 
   //					--   which clears list
   //					-- parser: pop at end of select_query
   if (tail) {
      p = tail->prev;
      delete tail;
      tail = p;
      if ( ! tail ) {
	 head = 0;
      }
   }
   mode = get_mode ();
   return (mode);
}

int sql_keyword_state::get_mode (void)
{
   int mode;
   if (tail) {
      mode = tail->mode;
   } else {
      mode = base_mode;
   }
   return (mode);
}   

// change to base mode
void sql_keyword_state::set_base_mode (int mode)
{
   base_mode = mode;
}

// keyword table in sql_keytable.C
extern SQL_KEY sql_keyword_table [];
extern "C" const size_t sql_keyword_table_size;

// Compare two null-terminated strings ignoring case
//
// Returns same as strcmp:
// 0   = strings the same
// > 0 = first string is greater than second
// < 0 = first string is less than second
//
// Note: Needed to write my own because the the library routine
// on NT used tolower which meant that keywords with underscores "_"
// were in a different order.
//
EXTERN_C int compcaseless(const char* s1, const char* s2) {
   int ret_val;
   /* compare characters through and including zero terminator */
   for (ret_val = 0 ; ret_val == 0; ++s1, ++s2) {
      ret_val = toupper(*s1) - toupper(*s2);
      if ((*s1 == 0) || (*s2 == 0)) {
	 break;
      }
   }
   return ret_val;
}

// function to compare keys -- passed to bsearch
// must be in "int func(const void*,const void*)" form.
// 
static int keyword_compare (const void* key1, const void* key2) {
   return (compcaseless (((const SQL_KEY*) key1)->key_text,
      ((const SQL_KEY*) key2)->key_text));
}

// class sql_keyword --
//
// Accepts context info and performs keyword lookup to determine if
// a token is a keyword or identifier.
//
class sql_keyword {
private:
   sql_keyword_state keyword_mode;
   int start_of_statement_flag;
   int current_command;
   int last_keyword;
public:
   sql_keyword (void);
   ~sql_keyword ();
   int lookup (const char* text);
   void start_of_statement (int flag);
   void set_keyword_mode (int mode);
   int get_keyword_mode (void);
   void push_keyword_mode (int mode);
   int pop_keyword_mode (void);
   void set_command (int command);
   void add_command (int command);
};

sql_keyword::sql_keyword (void) :
	keyword_mode (SQLKEY_ALL),
	start_of_statement_flag (1),
	current_command (SQLKEYC_NONE),
	last_keyword (0)
{
}

sql_keyword::~sql_keyword ()
{
}

// find the token value associated with a keyword string
// or return 0 if string is not a keyword.
//
// Uses binary search on sorted keyword table.  Compares
// strings using a case-insensitive compare.
//
int sql_keyword::lookup (const char* text) {

   int keyword_token = 0;		// return token value
   SQL_KEY findkey;			// bsearch key
 
   // only key_text is used in compare
   findkey.key_text    = text;
   findkey.key_token   = 0;
   findkey.key_type    = 0;
   findkey.key_command = 0;
   findkey.key_vendor  = 0;

   // call bsearch library function to search ordered table
   // in sql_keytable.C.  See man page on bsearch for more info.
   SQL_KEY* kp = (SQL_KEY*) bsearch (
#ifdef sun4
	   /* cast to sun4 arg type*/ (char*)
#endif
				     &findkey,
#ifdef sun4
	   /* cast to sun4 arg type*/ (char*)
#endif
				     sql_keyword_table,
				     sql_keyword_table_size,
				     sizeof(SQL_KEY),
				     keyword_compare);

   // keyword found -- now check if it is valid in the current
   // context, if not, clear keyword_token return value (token
   // will be treated as an identifier).
   if (kp) {
      // check vendor (e.g. ORACLE) and mode (e.g. SQL, ESQL, PLSQL)
      // bit masks
      if ((kp->key_vendor & dbvendor) &&
	  (kp->key_type & keyword_mode.get_mode())) {
	 // set to token value in table
	 keyword_token = kp->key_token;

	 // check if special processing needed for this keyword
	 if ( ! (kp->key_type & SQLKEY_SPECIAL)) {
	    // Normal keyword handling

	    // handle case of keyword only valid at start of statement
	    //
	    // statements can also start at the begining of the declare
	    // section (e.g. after keywords AS, DECLARE, IS)
	    // or after an ESQL prefix qualifier (AT, FOR)
	    if (kp->key_type & SQLKEY_START) {
	       if ( ! start_of_statement_flag ) {
		  switch (last_keyword) {
		   case Y_AS:
		   case Y_IS:
		   case Y_DECLARE:
		     break;

		   case Y_AT:
		   case Y_FOR:
		     if ( ! esql_mode ) {
			keyword_token = 0;
		     }
		     break;

		   default:
		     keyword_token = 0;
		     break;
		  }
	       }
	    }

	    // handle keywords valid only within certain SQL commands
	    if (keyword_token && (kp->key_type & SQLKEY_COMMAND)) {
	       if ((kp->key_command & current_command) == 0) {
		  keyword_token = 0;
	       }
	    }

	 } else {
	    // special handling
	    switch (kp->key_token) {
	     case Y_ENABLE:
	       // valid only in esql or as part of CREATE_TABLE command
	       if ( (! (SQLKEY_ESQL & keyword_mode.get_mode()) ) &&
		    (! (SQLKEYC_CT & current_command) )) {
		  keyword_token = 0;
	       }
	       break;

	     case Y_EXECUTE:
	       // valid only in esql or at the start of a command
	       if ( (! (SQLKEY_ESQL & keyword_mode.get_mode()) ) &&
		    (! start_of_statement_flag )) {
		  keyword_token = 0;
	       }
	       break;

	     case Y_CONTROLFILE:
	     case Y_PROFILE:
	     case Y_PUBLIC:
	     case Y_ROLE:
	     case Y_SNAPSHOT:
	     case Y_SYNONYM:
	       // valid only after Y_CREATE keyword
	       if (last_keyword != Y_CREATE) {
		  keyword_token = 0;
	       }
	       break;

	     case Y_PRECISION:
	       // valid only after Y_DOUBLE keyword
	       if (last_keyword != Y_DOUBLE) {
		  keyword_token = 0;
	       }
	       break;


	     default:
	       break;
	    }
	 } // endif special

	 // check for keyword valid only in certain commands
	 if (keyword_token && esql_mode &&
	     (current_command == SQLKEYC_VARCHAR)) {
	    if ((kp->key_command & SQLKEYC_VARCHAR) == 0) {
	       keyword_token = 0;
	    }
	 }
      }
   }

#if 1
   // check current yyparser state to see if keyword is valid at this
   // point in the grammar
   if (keyword_token) {
      if (!yykeywordcheck (keyword_token)) {
	 keyword_token = 0;
      }
   }
#endif

   // store last keyword if needed for special processing of next keyword
   last_keyword = keyword_token;

   return (keyword_token);
}

// Set/clear start of statement flag
//
// if set -- reset context info
void sql_keyword::start_of_statement (int flag)
{
   start_of_statement_flag = flag;
   if (flag) {
      keyword_mode.clear ();
      current_command = SQLKEYC_NONE;
      last_keyword = 0;
   }
}

// Sets the base mode, not current mode
// only called once via wrapper below by sql_process in sql.l
//
void sql_keyword::set_keyword_mode (int mode)
{
   keyword_mode.set_base_mode (mode);
}

// get current mode
int sql_keyword::get_keyword_mode (void)
{
   return (keyword_mode.get_mode ());
}

// push new mode for current statement
void sql_keyword::push_keyword_mode (int mode)
{
   keyword_mode.push (mode);
}

// pop mode -- ok to pop empty stack
int sql_keyword::pop_keyword_mode (void)
{
   return (keyword_mode.pop ());
}

// set the current sql command (e.g create table, commit, etc)
//
void sql_keyword::set_command (int command)
{
   current_command = command;
}

// add sql command (Or bit mask)
// TBD: not currently used
void sql_keyword::add_command (int command)
{
   current_command |= command;
}

//////////////////////////////
// instantiate above class  //
//////////////////////////////
static sql_keyword k;

//
// C function wrappers
//

EXTERN_C int sql_keyword_lookup (const char* text)
{
   return (k.lookup (text));
}

EXTERN_C void sql_keyword_start_of_statement (int flag)
{
   k.start_of_statement (flag);
}

EXTERN_C void sql_set_keyword_mode (int mode)
{
   k.set_keyword_mode (mode);
}

EXTERN_C int sql_get_keyword_mode (void)
{
   return (k.get_keyword_mode ());
}

EXTERN_C void sql_push_keyword_mode (int mode)
{
   k.push_keyword_mode (mode);
}

EXTERN_C void sql_pop_keyword_mode (void)
{
   k.pop_keyword_mode ();
}

EXTERN_C void sql_keyword_command (int command)
{
   k.set_command (command);
}
