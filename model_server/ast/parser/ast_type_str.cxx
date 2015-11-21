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
#define GCC_2_6
/****#include "general.h"****/
#include "astTree.h"

#ifndef C_PARSER
#ifdef GCC_2_6
#include "cp/parse.h"
#else
#include "cp-parse.h"
#endif /* GCC_2_6 */
#else
#include "c-parse.h"
#endif

#define ____CQT(x) 'x'
#if ____CQT(y) == 'y'
#define ts(x) case x : return "x"
#else
#define ts(x) case x : return #x
#endif
#undef ___CQT


static char  *char_buf = "a";

char * ast_type_str(int code){

if(code == 0) {return "0";} 
if(code == -1) {return "-1";} 

if(code < 32) {
  return "CTRL_CHAR";
} 

if(code < 128) { 
  char_buf[0] = code;
  return char_buf;
}

switch (code){

#ifdef GCC_2_6
#ifdef C_PARSER
#include "c-parse.h.ts"
#else
#include "cp/parse.h.ts"
#endif /* C_PARSER */
#else

ts(IDENTIFIER);
ts(TYPENAME);

#ifndef C_PARSER
ts(SCOPED_TYPENAME);
#endif

ts(SCSPEC);
ts(TYPESPEC);
ts(TYPE_QUAL);
ts(CONSTANT);
ts(STRING);
ts(ELLIPSIS);
ts(SIZEOF);
ts(ENUM);
ts(IF);
ts(ELSE);
ts(WHILE);
ts(DO);
ts(FOR);
ts(SWITCH);
ts(CASE);
ts(DEFAULT);
ts(BREAK);
ts(CONTINUE);
ts(RETURN);
ts(GOTO);
ts(ASM_KEYWORD);
ts(TYPEOF);
ts(ALIGNOF);
#ifdef C_PARSER
ts(ALIGN);
#endif

#ifndef C_PARSER
ts(HEADOF);
ts(CLASSOF);
#endif

ts(ATTRIBUTE);
ts(EXTENSION);
ts(LABEL);
#ifndef C_PARSER
ts(AGGR);
#else
ts(STRUCT);
ts(UNION);
#endif

#ifndef C_PARSER
ts(VISSPEC);
#endif

#ifndef C_PARSER
ts(DELETE);
ts(NEW);
ts(OVERLOAD);
#endif

#ifndef C_PARSER
ts(THIS);
ts(OPERATOR);
ts(LEFT_RIGHT);
#endif

#ifndef C_PARSER
ts(TEMPLATE);
#endif

#ifndef C_PARSER
ts(SCOPE);
#endif

#ifndef C_PARSER
ts(START_DECLARATOR);
#endif

#ifndef C_PARSER
ts(EMPTY);
ts(TYPENAME_COLON);
#endif
ts(ASSIGN);
#ifndef C_PARSER
ts(RANGE);
#endif
ts(OROR);
ts(ANDAND);
#ifndef C_PARSER
ts(MIN_MAX);
#endif
ts(EQCOMPARE);
ts(ARITHCOMPARE);
ts(LSHIFT);
ts(RSHIFT);
ts(UNARY);
ts(PLUSPLUS);
ts(MINUSMINUS);
ts(HYPERUNARY);
#ifndef C_PARSER
ts(PAREN_STAR_PAREN);
#endif

ts(POINTSAT);

#ifndef C_PARSER
ts(POINTSAT_STAR);
ts(DOT_STAR);
#endif

#ifndef C_PARSER
ts(RAISE);
ts(RAISES);
ts(RERAISE);
ts(TRY);
ts(EXCEPT);
ts(CATCH);
#endif

#ifndef C_PARSER
ts(TYPENAME_ELLIPSIS);
#endif

#ifndef C_PARSER
ts(PTYPENAME);
#endif

#ifndef C_PARSER
ts(PRE_PARSED_FUNCTION_DECL);
ts(EXTERN_LANG_STRING);
ts(ALL);
#endif

#ifndef C_PARSER
ts(PRE_PARSED_CLASS_DECL);
ts(TYPENAME_DEFN);
ts(IDENTIFIER_DEFN);
ts(PTYPENAME_DEFN);
ts(END_OF_SAVED_INPUT);
#endif

#endif /* GCC_2_6 */

default: return "UNKNOWN";
}} 





