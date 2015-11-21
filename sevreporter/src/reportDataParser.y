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
#include <memory.h>
#include <stdio.h>
/* --C */

/* ++SEV */
/* reportDataParser.h will contain the appropriate typedefs for the classes for C */
#include <reportDataParser.h>  
/* --SEV */

static rootFolder *root_folder    = 0;
static inventory  *inventory_list = 0;
static inputs     *inputs_list    = 0;

struct _finfo { 
  char *weight;
  char *quality; 
  char *desc;
} *finfo;

%}

%union {
  char           *pchar;
  inputs         *pinputs;
  inventory      *pinven;
  metric         *pmetric;
  metricFolder   *pmfolder;
  query          *pquery;
  queryFolder    *pqfolder;
  reportItemList *prnlist;
  struct _finfo  *pfinfo;	  
  void           *dummy;
};



%type <dummy>    inputs
%type <dummy>    inventory
%type <dummy>    reportfile
%type <dummy>    rootfolder
%type <pfinfo>   folderinfo
%type <pinputs>  inputsdata
%type <pinven>   inventorydata
%type <pmetric>  metric
%type <pmetric>  metricdata
%type <pmfolder> metricfolder
%type <pqfolder> queryfolder
%type <prnlist>  queryfolderlist
%type <pquery>   normal_query
%type <pquery>   qnormdata
%type <pquery>   qthreshdata
%type <pquery>   thresh_query
%type <prnlist>  mfcontents
%type <prnlist>  qfcontents

%type <pchar> name
%type <pchar> quality
%type <pchar> thresh
%type <pchar> weight
%type <pchar> units
%type <pchar> count
%type <pchar> totalunits
%type <pchar> desctag
%type <pchar> high
%type <pchar> low
%type <pchar> avg
%type <pchar> stddev
%type <pchar> string

%token         KWD_BEGIN
%token         KWD_END
%token         KWD_FOLDER
%token         KWD_INPUTS
%token         KWD_INVENTORY
%token         KWD_METRIC
%token         KWD_QUERY_NORM
%token         KWD_QUERY_THRESH
%token         KWD_ROOT
%token <pchar> QSTRING
%token <pchar> STRING

%start reportfile

%%

reportfile:	inputs inventory rootfolder	{
							$$ = 0;
						}
		;

inputs:		KWD_BEGIN KWD_INPUTS inputsdata	KWD_END {
							inputs_list = $3;
							$$ = 0;
						}
		;


inventory:	KWD_BEGIN KWD_INVENTORY inventorydata KWD_END {
							inventory_list = $3;
							$$ = 0;
						}
		;

rootfolder:	KWD_BEGIN KWD_FOLDER KWD_ROOT folderinfo queryfolderlist metricfolder KWD_END {
							if ( $4 && $5 )
							  {
							    root_folder = create_rootFolder($4->weight, $4->quality, $4->desc);
							    if ( root_folder )
							      {
								insertQueryFolders_rootFolder(root_folder, $5);
								insertMetricFolder_rootFolder(root_folder, $6);
							      }
							  }
							if ( $4 )
							  free ( $4 );
							$$ = 0;
						}
		;

queryfolder: 	KWD_BEGIN KWD_FOLDER name folderinfo qfcontents KWD_END {
							if ( $3 && $4 && $5 )
							  {
							    $$ = create_queryFolder($3, $4->weight, $4->quality, $4->desc);
							    if ( $$ )
							      {
								insertItems_queryFolder($$, $5);
							      }
							  }
							else
							  {
							    $$ = 0;
							  }
							if ( $4 )
							  free ( $4 );
						}
		;

metricfolder:	KWD_BEGIN KWD_FOLDER name folderinfo mfcontents KWD_END {
							if ( $3 && $4 && $5 )
							  {
							    if ( ($3)[0] == '.' )
							      {
								/* .some-name-here is a hidden folder, i.e. no name */
								$$ = create_metricFolder(0, $4->desc);
							      }
							    else
							      {
								$$ = create_metricFolder($3, $4->desc);
							      }
							    if ( $$ )
							      {
								insertItems_metricFolder($$, $5);
							      }
							  }
							else
							  {
							    $$ = 0;
							  }
							if ( $4 )
							  {
							    if ( $4->weight )
							      free ( $4->weight );
							    if ( $4->quality )
							      free ( $4->quality );
							    free ( $4 );
							  }
						}
		;


folderinfo:	weight quality desctag		{
  							$$ = (struct _finfo *) malloc(sizeof(struct _finfo));
							if ( $$ )
							  {
							    $$->weight  = $1;
							    $$->quality = $2;
							    $$->desc    = $3;
							  }
						}
		;

queryfolderlist:queryfolder			{
							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| queryfolderlist queryfolder	{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		;

qfcontents:	normal_query			{
  							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| thresh_query			{
  							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| queryfolder			{
  							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| qfcontents normal_query	{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		| qfcontents thresh_query	{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		| qfcontents queryfolder	{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		;

mfcontents:	metric				{
  							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| metricfolder			{
  							$$ = create_reportItemList();
							if ( $$ )
							  insertItem_reportItemList($$, $1);
						}
		| mfcontents metric		{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		| mfcontents metricfolder	{
							if ( $1 )
							  insertItem_reportItemList($1, $2);
							$$ = $1;
						}
		;
				

normal_query:	KWD_BEGIN KWD_QUERY_NORM qnormdata KWD_END {
							$$ = $3;
						}
		;

thresh_query: 	KWD_BEGIN KWD_QUERY_THRESH qthreshdata KWD_END {
							$$ = $3;
						}
		;

metric:		KWD_BEGIN KWD_METRIC metricdata	KWD_END {
							$$ = $3;
						}
		;

qnormdata:	name weight count totalunits units quality desctag {
							$$ = create_query($1, $7, $6, $2, $3, $4, $5);
						}
		;

qthreshdata: 	name weight count totalunits units quality desctag thresh {
							char *temp = merge_name_and_thresh($1, $8);
							free($1);
							free($8);
							$$ = create_query(temp, $7, $6, $2, $3, $4, $5);
						}
		;

metricdata:	name high low avg stddev desctag {
							$$ = create_metric($1, $6, $2, $3, $4, $5);
						}
		;

inputsdata:	string				{
							$$ = create_inputs();
							if ( $$ )
							  insert_inputs($$, $1);
						}
		| inputsdata string		{
							if ( $1 )
							  insert_inputs($1, $2);
							$$ = $1;
						}
		;

inventorydata:	string string			{
							$$ = create_inventory();
							if ( $$ )
							  insert_inventory($$, $1, $2);
						}
		| inventorydata string string	{
							if ( $1 )
							  insert_inventory($1, $2, $3);
							$$ = $1;
						}
		;

name:		string				{
							$$ = $1;
						}
		;

quality:	string				{
							$$ = $1;
						}
		;

weight:		string				{
							$$ = $1;
						}
		;

thresh:		string				{
							$$ = $1;
						}
		;

units:		string				{
							$$ = $1;
						}
		;

count:		string				{
							$$ = $1;
						}
		;

totalunits:	string				{
							$$ = $1;
						}
		;

desctag:	string				{
							$$ = $1;
						}
		;

high:		string				{
							$$ = $1;
						}
		;

low:		string				{
							$$ = $1;
						}
		;

avg:		string				{
							$$ = $1;
						}
		;

stddev:		string				{
							$$ = $1;
						}
		;

string:		QSTRING				{
							$$ = $1;
						}
		| STRING			{
							$$ = $1;
						}
		;

%%


int reportDataParse(char *filename, rootFolder **rf, inventory **inv, inputs **inp)
{
  int retval = -1;

  /* redirect yyin (the global FILE * that lex reads from) */
  extern FILE *yyin;

  /* make sure that the input is valid */
  if ( rf && inv && inp )
    {
      /* redirect input from file or stdin */
      if ( filename )
	yyin = fopen(filename, "r");
      else
	yyin = stdin;
      
      if ( yyin )
	{
	  /* reset file static variables */
	  root_folder    = 0;
	  inventory_list = 0;
	  inputs_list    = 0;
	  
	  if ( yyparse() == 0 ) /* zero is success for yyparse() */
	    {
	      /* set the return values */
	      *rf  = root_folder;
	      *inv = inventory_list;
	      *inp = inputs_list;
	      retval = 1;
	    }
	  else
	    {
	      /* clear the return values */
	      *rf  = 0;
	      *inv = 0;
	      *inp = 0;
	    }
	  
	  if ( yyin != stdin )
	    fclose(yyin);
	}
    }
  
  return retval;
}
