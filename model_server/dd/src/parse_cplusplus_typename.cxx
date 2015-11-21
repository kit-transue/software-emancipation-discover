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
// parse_cplusplus_typename
//------------------------------------------
// synopsis:
// Figure out where to insert a variable name into a C++ typename to make
// an object declaration.
//
// description:
// Does a partial parse of a C++/C typename, determining where the name
// would go in the abstract declarator.  The syntax of an abstract
// declarator is given on, e.g., p. 130 of Ellis & Stroustrup:
//
//	type-name:
//		type-specifier-list abstract-declarator-opt
//
//	type-specifier-list:
//		type-specifier type-specifier-list-opt
//
//	abstract-declarator:
//		ptr-operator abstract-declarator-opt
//		abstract-declarator-opt ( argument-declaration-list )
//				cv-qualifier-list-opt
//		abstract-declarator-opt [ constant-expression-opt ]
//		( abstract-declarator )
//
// The algorithm used in the parse is to scan off all identifiers at the
// beginning of the type name (the type-specifier-list; in order to
// simplify the analysis, "foo {...}", "foo<...>", and "foo::bar" are
// lexed as single identifiers).  The end of the innermost declarator is
// then just before the terminating NUL or the first-encountered argument
// list, array bound specification, or right parenthesis.  An argument
// list is distinguished from a parenthesized abstract declarator by
// being either an empty list "()" or containing an identifier before the
// first left parenthesis, left bracket, or right parenthesis is reached.
//------------------------------------------
// $Log: parse_cplusplus_typename.cxx  $
// Revision 1.4 2000/07/10 23:01:42EDT ktrans 
// mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.7  1993/09/27  18:36:46  kws
Bug track: Port
Port compatability

Revision 1.2.1.6  1993/09/17  22:34:17  pero
Bug track: 4748
in parse_cplusplus_typename() the delete was called before retrieve on
link list (switch statement); purify error

Revision 1.2.1.5  1993/09/01  02:07:03  aharlap
syntax error

Revision 1.2.1.4  1993/09/01  01:31:23  aharlap
bug # 4641

Revision 1.2.1.3  1992/11/21  21:10:26  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:12:17  builder
fixed rcs header

Revision 1.2.1.1  92/10/07  21:29:08  smit
*** empty log message ***

Revision 1.2  92/10/07  21:29:07  smit
*** empty log message ***

Revision 1.1  92/10/07  18:24:57  smit
Initial revision

//Revision 1.1  92/03/05  08:00:46  wmm
//Initial revision
//
//
//------------------------------------------
// Restrictions:
// Unpredictable (but assumed harmless) results for incorrectly-formed
// typenames.  Also, no attempt is made to handle pathological cases
// like "foo< i < 5 >".
//------------------------------------------
*/

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
namespace std {};
using namespace std;
#include <cstddef>
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include "linked_list.h"

enum typename_token_type {
   ID,
   PTR_OP,
   LPAREN,
   RPAREN,
   LBRACKET,
   OTHER,
   EOD
};

class typename_token: public slink {
public:
   typename_token(const char*);
   ~typename_token();
   static typename_token* first();
   static typename_token* last();
   typename_token* next() const;
   const char* start() const;
   size_t length() const;
   typename_token_type type() const;
private:
   static slink* head;
   static slink* tail;
   const char* first_char;
   size_t len;
   typename_token_type tp;
};

slink* typename_token::head;
slink* typename_token::tail;

//------------------------------------------
// const char* scan_id_or_ptr_to_member(const char*)
//------------------------------------------

// If the string beginning at "p" is a pointer to member (X::*), returns
// a pointer to the character after the *; otherwise, returns a pointer to
// the first nonblank after the identifier

static const char* scan_id_or_ptr_to_member(const char* p) {

identifier_scan:
   while (isalnum(*p) || isspace(*p) || *p == '_')
      p++;

   if (*p == '{') {
      int level = 1;
      while (level) {
	 if (*++p == '{')
	    level++;
	 else if (*p == '}')
	    level--;
      }
      p++;
      goto identifier_scan;
   }

   if (*p == '<') {
      int level = 1;
      while (level) {
	 if (*++p == '<')
	    level++;
	 else if (*p == '>')
	    level--;
      }
      p++;
      goto identifier_scan;
   }

   if (*p == ':' && p[1] == ':') {
      p += 2;
      while (isspace(*p))
	 p++;
      if (*p == '*')
	 return p + 1;
      goto identifier_scan;
   }

   return p;
}

//------------------------------------------
// typename_token::typename_token(const char*)
//------------------------------------------

typename_token::typename_token(const char* start) {
   link_after(tail, head, tail);

   while (isspace(*start))
      start++;
   first_char = start;

   if (isalnum(*start)) {
      const char* next = scan_id_or_ptr_to_member(start);
      if (next[-1] == '*') {
	 tp = PTR_OP;
	 while (isspace(*next))
	    next++;
      }
      else tp = ID;
      len = next - start;
   }
   else if (*start == 0) {
      tp = EOD;
      len = 0;
   }
   else {
      len = 1;
      if (*start == '*' || *start == '&')
	 tp = PTR_OP;
      else if (*start == '(') 
	 tp = LPAREN;
      else if (*start == ')')
	 tp = RPAREN;
      else if (*start == '[')
	 tp = LBRACKET;
      else tp = OTHER;
   }
}

//------------------------------------------
// typename_token::~typename_token()
//------------------------------------------

typename_token::~typename_token() {
   unlink_from(NULL, head, tail);
   delete head;
}

//------------------------------------------
// typename_token::first()
//------------------------------------------

typename_token* typename_token::first() {
   return (typename_token*) head;
}

//------------------------------------------
// typename_token::last()
//------------------------------------------

typename_token* typename_token::last() {
   return (typename_token*) tail;
}

//------------------------------------------
// typename_token::next() const
//------------------------------------------

typename_token* typename_token::next() const {
   typename_token* This = (typename_token*) this;
   return (typename_token*) This->slink::next();
}

//------------------------------------------
// typename_token::start() const
//------------------------------------------

const char* typename_token::start() const {
   return first_char;
}

//------------------------------------------
// typename_token::length() const
//------------------------------------------

size_t typename_token::length() const {
   return len;
}

//------------------------------------------
// typename_token::type() const
//------------------------------------------

typename_token_type typename_token::type() const {
   return tp;
}

//------------------------------------------
// is_arg_list(const typename_token* tok)
//------------------------------------------

static int is_arg_list(const typename_token* tok) {
   if (tok->next()->type() == RPAREN)
      return 1;

   for (const typename_token* p = tok->next(); p; p = p->next()) {
      if (p->type() == ID || p->type() == EOD)
         return 1;
      if (p->type() == LPAREN || p->type() == RPAREN || p->type() ==
            LBRACKET)
         return 0;
   }

   return 1;
}

//------------------------------------------
// parse_cplusplus_typename(const char*)
//------------------------------------------

// Returns offset in input string at which a variable name should be
// inserted to turn a typename into an object declaration

int parse_cplusplus_typename(const char* type_name) {

   int empty_string = 1;

   const char *p;
   for (p = type_name; *p; ++p)
       if (*p != ' ' && *p != '\t') {
	   empty_string = 0;
	   break;
       }

   if (empty_string)
       return 0;

   for (p = type_name; *p; p = typename_token::last()->start() +
         typename_token::last()->length())
      new typename_token(p);

   const typename_token *tok;
   for (tok = typename_token::first(); tok &&
         tok->type() == ID; tok = tok->next()) ;

   while (tok) {
      switch (tok->type()) {
      case EOD:
      case LBRACKET:
      case RPAREN:
	 {
	     int ret_val = tok->start() - type_name;
	     delete typename_token::first();
	     return ret_val;
         }

      case LPAREN:
         if (is_arg_list(tok)) {
	    int ret_val = tok->start() - type_name;
            delete typename_token::first();
            return ret_val;
         }

         /* FALLTHROUGH */

      default:
         tok = tok->next();
      }
   }

   delete typename_token::first();
   return strlen(type_name);
}
