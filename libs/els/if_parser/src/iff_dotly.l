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

#include <stdlib.h>
#include <string.h>

#include <astbodyStruct.h>
#include <astnodeStruct.h>
#include <attributeStruct.h>
#include <locationStruct.h>
#include <maplineStruct.h>
#include <nodeinfoStruct.h>
#include <positionStruct.h>
#include <transformationStruct.h>

#include <astnodeList.h>
#include <attributeList.h>
#include <locationList.h>
#include <maplineList.h>
#include <stringList.h>
#include <transformationList.h>
#include <uintList.h>

#include "iff_dotly_parser.h"
#define MAX_QUOTE_SIZE 65536
static int  qstring_index;
static char quoted_string[MAX_QUOTE_SIZE];

/* This is declared global so that iff_parse in the YACC file can reset it when necessary */
int iff_parser_LineNumber;

#ifdef YY_BUF_SIZE
#undef YY_BUF_SIZE
#endif
#define YY_BUF_SIZE 262144
%}

lbrack          "["
rbrack          "]"
lbrace          "{"
rbrace          "}"
lparen          "("
rparen          ")"
slash           "/"
comma           ","
quote           ["]
bsquote         \\["]
bsbs            \\\\
nl              "\n"

whitespace      [ \t\r]+
digits          [0-9]+
string          [^ \t\n\r{}()/,\[\]";:]+
comment         (^"#".*)

%s NQ Q

%%

%{
    BEGIN NQ;
%}

<Q>{quote}     {
                 quoted_string[qstring_index] = 0;
		 iff_dotlylval.char_ptr = strdup(quoted_string);
                 BEGIN NQ;
		 return QSTRING;
               }

<Q>{bsquote}   {          
                 if ( qstring_index + 1 >= MAX_QUOTE_SIZE )
		   {
		     iff_dotlyerror("Buffer overflow");
		     iff_dotlyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = '\\';
		     quoted_string[qstring_index++] = '"';
		   }
               }

<Q>{bsbs}      {
                 if ( qstring_index + 1 >= MAX_QUOTE_SIZE )
		   {
		     iff_dotlyerror("Buffer overflow");
		     iff_dotlyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = '\\';
		     quoted_string[qstring_index++] = '\\';
		   }
               }

<Q>.           {
                 if ( qstring_index >= MAX_QUOTE_SIZE )
		   {
		     iff_dotlyerror("Buffer overflow");
		     iff_dotlyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = yytext[0];
		   }
               }

<Q>{nl}        {
                  iff_dotlyerror("Unterminated quoted string");
		  quoted_string[qstring_index] = 0;
		  iff_dotlylval.char_ptr = strdup(quoted_string);
                  BEGIN NQ;
		  return QSTRING;
               }

<NQ>{quote} {
          qstring_index = 0;
          BEGIN Q;
	}

<NQ>{lbrack} {
          return yytext[0];
	}

<NQ>{rbrack} {
          return yytext[0];
	}

<NQ>{lbrace} {
          return yytext[0];
	}

<NQ>{rbrace} {
          return yytext[0];
	}

<NQ>{lparen} {
          return yytext[0];
	}

<NQ>{rparen} {
          return yytext[0];
	}

<NQ>{slash} {
          return yytext[0];
	}

<NQ>{comma} {
          return yytext[0];
	}

<NQ>AST     {  
          iff_dotlylval.uint = AST;      
	  return AST; 
	}

<NQ>ATR     { 
          iff_dotlylval.uint = ATR;      
	  return ATR; 
	}

<NQ>copy    { 
          iff_dotlylval.uint = COPY;     
	  return COPY; 
	}


<NQ>cg  {
          iff_dotlylval.uint = CG;
          return CG;
        }

<NQ>def {
          iff_dotlylval.uint = DEF;
          return DEF;
        }

<NQ>rep {
          iff_dotlylval.uint = REP;
          return REP;
        }

<NQ>ERR	{ 
	  iff_dotlylval.uint = ERR;      
	  return ERR; 
	}

<NQ>file	{ 
	  iff_dotlylval.uint = TOK_FILE;     
	  return TOK_FILE; 
	}

<NQ>fixed   { 
	  iff_dotlylval.uint = FIXED;    
	  return FIXED; 
	}

<NQ>implicit { 
	  iff_dotlylval.uint = IMPLICIT; 
	  return IMPLICIT; 
	}

<NQ>keyword	{ 
	  iff_dotlylval.uint = KEYWORD;  
	  return KEYWORD; 
	}

<NQ>language	{ 
	  iff_dotlylval.uint = LANGUAGE;  
	  return LANGUAGE; 
	}

<NQ>lines {
          iff_dotlylval.uint = LINES;
          return LINES;
        }

<NQ>MAP	{ 
 	  iff_dotlylval.uint = MAP;      
	  return MAP; 
	}

<NQ>REL	{ 
	  iff_dotlylval.uint = REL;      
	  return REL; 
	}

<NQ>SMT	{ 
	  iff_dotlylval.uint = SMT;      
	  return SMT; 
	}

<NQ>SYM	{ 
	  iff_dotlylval.uint = SYM;      
	  return SYM; 
	}

<NQ>^"#".* { 
	  ; 
	}

<NQ>{whitespace} { 
	  ; 
	}

<NQ>{digits} { 
	  iff_dotlylval.uint = atoi(yytext); 
	  return NUM; 
 	}

<NQ>{string} { 
	  iff_dotlylval.char_ptr = strdup(yytext);
	  return STRING; 
	}

<NQ>{nl}	{ 
	  iff_parser_LineNumber++; 
	}


<NQ>. 	{ 
	  return yytext[0]; 
	}
%%

int yywrap()
{
  return 1;
}

int iff_dotlyerror(msg)
     char *msg;
{
  return fprintf ( stderr, "IFF parsing error: Line %d: %s at '%s'\n", iff_parser_LineNumber, msg, yytext );
}

/********************************************************************************
 *  This function will re-quote a string that the lexer unquoted               
 ********************************************************************************/

int iff_quote(in, out)
   char *in; 
   char **out;
{
  int retval   = -1;
  int i        = 0;
  int length   = 0;
  int out_indx = 0;
 
  if ( in )
    {
      (*out) = (char *)malloc(2*strlen(in)+3);
      if ( (*out) )
	{
	  out_indx = 0;
	  (*out)[out_indx++] = '"';
	  length = strlen(in);
	  for ( i = 0; i < length; i++ )
	    {
	      switch ( in[i] )
		{
		case '"':
		  (*out)[out_indx++] = '\\';
		  (*out)[out_indx++] = '"';
		  break;
		case '\\':
		  (*out)[out_indx++] = '\\';
		  (*out)[out_indx++] = '\\';
		  break;
		default:
		  (*out)[out_indx++] = in[i];
		  break;
		}
	    }
	  (*out)[out_indx++] = '"';
	  (*out)[out_indx++] = '\0';
	  retval = strlen((*out));
	}
    }
  
  return retval;
}

