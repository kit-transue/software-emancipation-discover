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
/* --C */

/* ++ADS */
#include <cstring.h>
/* --ADS */

/* ++SEV */
#include <descParser.h>
/* --SEV */

static reportItem *item = 0;

%}

%union {
  int      integer;
  cstring *pcstring;
  char     ch;
};

%token <ch>      PLAINCHAR
%token <integer> CODE_MODE
%token <integer> INPUTS
%token <integer> INVENTORY
%token <integer> LINE_END
%token <integer> METRIC_TABLE
%token <integer> OVERALL_QUALITY
%token <integer> PARA_END
%token <integer> QUALITY_GRAPH
%token <integer> QUALITY_TABLE
%token <integer> QUALITY_RESULT
%token <integer> TEXT_MODE
%token <integer> EMPTY_DESCRIPTION

%type <integer>  description
%type <integer>  element_list
%type <integer>  element
%type <integer>  tag
%type <pcstring> text

%%

description:	element_list			{
							$$ = 0;
						}
		;

element_list:	element				{
							$$ = 0;
							$$ = 0;
						}
		| element_list element 		{
							$$ = 0;
						}
		;

element:	tag				{
							$$ = 0;
						}
		| text	        		{
							ri_on_string(item, get_char_ptr_cstring($1));
							destroy_cstring($1);
							$$ = 0;
						}
		;


tag:		TEXT_MODE			{
  							ri_on_tag(item, TEXT_MODE);
							$$ = 0;
						}
                | CODE_MODE			{
							ri_on_tag(item, CODE_MODE);
							$$ = 0;
						}
		| INPUTS			{
							ri_on_tag(item, INPUTS);
							$$ = 0;
						}
		| INVENTORY			{
							ri_on_tag(item, INVENTORY);
							$$ = 0;
						}
		| METRIC_TABLE			{
							ri_on_tag(item, METRIC_TABLE);
							$$ = 0;
						}
		| QUALITY_GRAPH			{
							ri_on_tag(item, QUALITY_GRAPH);
							$$ = 0;
						}
		| QUALITY_TABLE			{
							ri_on_tag(item, QUALITY_TABLE);
							$$ = 0;
						}
		| QUALITY_RESULT		{
							ri_on_tag(item, QUALITY_RESULT);
							$$ = 0;
						}
		| LINE_END			{
							ri_on_tag(item, LINE_END);
							$$ = 0;
						}
		| PARA_END			{
							ri_on_tag(item, PARA_END);
							$$ = 0;
						}
		| OVERALL_QUALITY               {
							ri_on_tag(item, OVERALL_QUALITY);
							$$ = 0;
						}
		;

text:		PLAINCHAR			{
							$$ = create_cstring();
							$$ = add_char_cstring($$, $1);
						}
		| text PLAINCHAR 		{
							$$ = add_char_cstring($1, $2);
						}
		;


%%

/* can't include the headerfile, as it is not C-safe (is C++) */
extern char *pdtempname();
extern int   pdrmfile();
extern int   pdstrgsub();

extern FILE *yyin;

int parseDescription(reportItem *ri, char *desc) 
{
  int retval = -1;

  char *tempfilename = 0;
  char *newdesc      = 0;
  char *temp         = 0;

  yyin = 0;

  if ( ri )
    {
      if ( desc )
	{
	  item = ri;
	  if ( tempfilename = pdtempname(0) )
	    {
	      if ( yyin = fopen(tempfilename, "w+") )
		{
		  /* replace % with %% to get past fprintf */
		  pdstrgsub(desc, "%", "\02", &temp);
		  if ( temp )
		    {
		      pdstrgsub(temp, "\02", "%%", &newdesc);
		      free(temp);
		      if ( newdesc )
			{
			  /* get the data to yyin */
			  fprintf(yyin, newdesc);
			  fflush(yyin);
			  rewind(yyin);
		      
			  /* parse it */
			  if ( yyparse() == 0 )
			    retval = 1;
		      
			  /* clean up */
			  fclose(yyin);
			  free(newdesc);
			}
		    }
		}
	    }
	  item = 0;
	}
      else
	{
	  /* description is empty, let each object */
	  /* decide how to handle that situation   */
	  ri_on_tag(ri, EMPTY_DESCRIPTION);
	  retval = 1;
	}
    }
  
  if ( yyin )
    pdrmfile(tempfilename);
  
  if ( tempfilename )
    free(tempfilename);
  
  return retval;
}
