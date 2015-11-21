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

/* ++C */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
/* --C */

/* ++ADS */
#include <parray.h>
#include <cstring.h>
/* --ADS */

static parray *tag_file_list;
static void *create_tag_pair(char *tag, char *text);
%}

%union {
  parray  *pparray;
  cstring *pcstring;
  char     ch;	
};

%type <pcstring> text
%type <pparray>  text_pair
%type <pparray>  text_pairs
%type <pparray>  tag_file

%token <pcstring> TAG
%token <ch>       PLAINCHAR

%%

tag_file:	text_pairs			{
							tag_file_list = $1;
						}
		;

text_pairs:	text_pair			{
							$$ = create_parray(0);
							$$ = insert_element_parray($$, $1);  
						}
		| text_pairs text_pair		{
							$$ = insert_element_parray($1, $2);
						}

		;

text_pair:	TAG text			{
							$$ = create_tag_pair(strdup(get_char_ptr_cstring($1)), 
									     strdup(get_char_ptr_cstring($2)));
							destroy_cstring($1);
							destroy_cstring($2);
						}
		;

text:		PLAINCHAR			{
  							$$ = create_cstring();
							$$ = add_char_cstring($$, $1);
						}
		| text PLAINCHAR		{
							$$ = add_char_cstring($1, $2);
						}
		;

%%

static void *create_tag_pair(char *tag, char *text)
{
  struct _tag {
    char *tag;
    char *text;
  } *retval = 0;

  if ( tag && text )
    {
      retval = (struct _tag *) malloc(sizeof(struct _tag));
      if ( retval )
	{
	  retval->tag  = tag;
	  retval->text = text;
	}
    }

  return retval;
}

int reportTemplateParse(char *file, parray **assocs)
{
  int retval = -1;

  if ( file && assocs )
    {
      FILE *f = fopen(file, "r");
      if ( f )
	{
	  extern FILE *yyin;

	  yyin = f;
	  if ( yyparse() == 0 )
	    {
	      *assocs = tag_file_list;
	      retval = 1;
	    }
	}
      fclose(f);
    }

  return retval;
}
