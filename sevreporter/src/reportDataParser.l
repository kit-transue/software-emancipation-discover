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
#include <parray.h>
#include <reportDataParser.h>
#include <reportDataParser.yacc.h>

#define MAX_QUOTE_SIZE 65536
static int  linenum = 1;
static int  qstring_index = 0;
static char quoted_string[MAX_QUOTE_SIZE];

#ifndef _WIN32
int yyerror(msg)
     char *msg;
{
  return fprintf ( stderr, "Report data parsing error: line %d: %s at '%s'\n", linenum, msg, yytext );
}
#else
void yyerror(msg)
     char *msg;
{
  (void) fprintf ( stderr, "Report data parsing error: line %d: %s at '%s'\n", linenum, msg, yytext );
}
#endif

%}

quote           ["]
quote1          ["]
bsquote         \\["]
bsbs            \\\\
nl              "\n"
whitespace      [ \t\r]+
string          [^ \t\n\r"]+

%s NQ Q

%%

%{
    BEGIN NQ;
%}

<Q>{quote}     {
                 quoted_string[qstring_index] = 0;
		 yylval.pchar = strdup(quoted_string);
                 BEGIN NQ;
		 return QSTRING;
               }

<Q>{bsquote}   {          
                 if ( qstring_index >= MAX_QUOTE_SIZE )
		   {
		     yyerror("Buffer overflow");
		     yyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = '"';
		   }
               }

<Q>{bsbs}      {
                 if ( qstring_index >= MAX_QUOTE_SIZE )
		   {
		     yyerror("Buffer overflow");
		     yyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = '\\';
		   }
               }

<Q>.           {
                 if ( qstring_index >= MAX_QUOTE_SIZE )
		   {
		     yyerror("Buffer overflow");
		     yyerror("Maximum size of quoted string exceeded");
		   }
		 else
		   {
		     quoted_string[qstring_index++] = yytext[0];
		   }
               }

<Q>{nl}        {
                  yyerror("Unterminated quoted string");
		  quoted_string[qstring_index] = 0;
		  yylval.pchar = strdup(quoted_string);
                  BEGIN NQ;
		  linenum++;
		  return QSTRING;
               }

<NQ>{quote} {
          qstring_index = 0;
          BEGIN Q;
	}

<NQ>BEGIN {  
	  return KWD_BEGIN; 
	}

<NQ>END { 
	  return KWD_END; 
	}

<NQ>QUERY_0 { 
	  return KWD_QUERY_NORM; 
	}


<NQ>QUERY_1 {
          return KWD_QUERY_THRESH;
        }

<NQ>QUERY_2 {
          return KWD_METRIC;
        }

<NQ>FOLDER {
          return KWD_FOLDER;
        }

<NQ>INPUTS {
  	  return KWD_INPUTS;
	}

<NQ>INVENTORY {
  	  return KWD_INVENTORY;
	}

<NQ>ROOT {
          return KWD_ROOT;
}

<NQ>{whitespace} { 
	  ; 
	}

<NQ>{string} { 
	  yylval.pchar = strdup(yytext);
	  return STRING; 
	}

<NQ>{nl} { 
           linenum++;
	}


%%

int yywrap()
{
  return 1;
}

