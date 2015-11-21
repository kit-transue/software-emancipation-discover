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

/* macro for memory allocation debugging */
#define DO_STRUCT_REPORT			\
  {						\
    ReportAstbodyStructs();			\
    ReportAstnodeStructs();			\
    ReportAttributeStructs();			\
    ReportLocationStructs();			\
    ReportNodeinfoStructs();			\
    ReportPositionStructs();			\
    ReportTransformationStructs();		\
    ReportMaplineStructs();			\
  }

#include <iff_parser.h>

#include <string.h>
#include <stdio.h>
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

#include <discover-interface.h>

/* interface to the C++ file caching object */
#include <ifFileCache.h>

/* extern reference to the global line number var for the IF */
extern int iff_parser_LineNumber;


/* some file static variables used during parsing */
char  *current_smt_filename = 0;
char  *current_smt_relpath  = 0;
char  *current_anon_target  = 0;
char  *current_smt_language = 0;

/* used to keep symbols from multiple languages from clashing */
unsigned int current_sym_offset = 0;
unsigned int highest_sym_used   = 0;

%}

%union {
  
  /* data types defined in the interface, as well as lists of those data types */
 
  unsigned int           uint;
  uintList              *uintlist_ptr;

  astnodeStruct         *astnode_ptr;
  astnodeList           *astnodelist_ptr;

  attributeStruct       *attribute_ptr;
  attributeList         *attributelist_ptr;

  locationStruct        *location_ptr;
  locationList          *locationlist_ptr;

  maplineStruct         *mapline_ptr;
  maplineList           *maplinelist_ptr;

  transformationStruct  *transformation_ptr;
  transformationList    *transformationlist_ptr;

  /* structs not in the interface, but necessary for parsing */
  astbodyStruct         *astbody_ptr;
  nodeinfoStruct        *nodeinfo_ptr;
  positionStruct        *position_ptr;

  /* bit field to keep track of various flags */
  int                   flags;

  /* char pointer to keep track of strings */
  char                  *char_ptr;

  /* type to assign to when the result is ignored */
  void                  *dummy;
}

%token <char_ptr> QSTRING
%token <char_ptr> STRING
%token <uint> AST
%token <uint> ATR
%token <uint> CG
%token <uint> DEF
%token <uint> REP
%token <uint> COPY
%token <uint> ERR
%token <uint> TOK_FILE
%token <uint> FIXED
%token <uint> IMPLICIT
%token <uint> KEYWORD
%token <uint> LANGUAGE
%token <uint> LINES
%token <uint> MAP
%token <uint> NUM
%token <uint> REL
%token <uint> SMT
%token <uint> SYM

%type <astbody_ptr>        astbody
%type <astnode_ptr>        tree
%type <astnodelist_ptr>    treelist
%type <attribute_ptr>      name_part
%type <attributelist_ptr>  attr_list
%type <attributelist_ptr>  opt_attr_list
%type <char_ptr>           astfile
%type <char_ptr>           node
%type <char_ptr>           optkey
%type <char_ptr>           optoutfile
%type <char_ptr>	   string_or_file
%type <dummy>              ast_statement
%type <dummy>              atr_statement
%type <dummy>              err_statement
%type <dummy>              if_file
%type <dummy>              map_statement
%type <dummy>              rel_statement
%type <dummy>              smt_statement
%type <dummy>              statement
%type <dummy> 		   statement_list
%type <dummy>              sym_statement
%type <flags>              opt_implicit
%type <flags>              opt_sym_flag_list
%type <flags>              sym_flag_list
%type <flags>              sym_flag_token
%type <flags>              opt_ast_flag_list
%type <flags>              ast_flag_list
%type <flags>              ast_flag_token
%type <location_ptr>       bytemap
%type <location_ptr>       linecolmap
%type <location_ptr>       optsource
%type <location_ptr>       source
%type <location_ptr>       realastloc
%type <location_ptr>       source_loc
%type <location_ptr>       target_loc
%type <locationlist_ptr>   astloc
%type <locationlist_ptr>   astloclist
%type <locationlist_ptr>   source_list
%type <mapline_ptr>        mapline
%type <maplinelist_ptr>    maplinelist
%type <nodeinfo_ptr>       astnode
%type <position_ptr>       linecolend
%type <position_ptr>       linecolstart
%type <transformation_ptr> source_target
%type <transformationlist_ptr> map_list
%type <uint>               bytelen
%type <uint>               bytestart
%type <uint>               ident
%type <uint>               lineno
%type <uintlist_ptr>       ident_list

%start if_file

%%

/********************************************************************************
*
* an IF file is a list of IF statements 
*
*********************************************************************************/

if_file:                statement_list                  {
								$$ = $1;
#if defined(REPORT_SUMMARY) && defined(COUNT_STRUCTS)
                                                                DO_STRUCT_REPORT;
#endif		
 							}
			;

statement_list:		statement			{
								$$ = $1;
							}
			| statement_list statement     	{
								$$ = $1;
							}
			;
/********************************************************************************
*
* statements come in the followng types:
*
*********************************************************************************/

statement:              sym_statement                   {
                                                                $$ = $1;
                                                        }
                        | atr_statement                 {
                                                                $$ = $1;
                                                        }
                        | smt_statement                 {
                                                                $$ = $1;
                                                        }
                        | rel_statement                 {
                                                                $$ = $1;
                                                        }
                        | err_statement                 {
                                                                $$ = $1;
                                                        }
                        | map_statement                 {
                                                                $$ = $1;
                                                        }
                        | ast_statement                 {
                                                                $$ = $1;
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for a symbol statement
*
***********************************************************************************/

sym_statement:          SYM ident string_or_file opt_sym_flag_list QSTRING optsource {
                                                                if ( InsertSymbol($2, $4, $3, $5, $6) != 0 )
								  {
								    if ($3) free ($3);
								    if ($5) free($5);
								    if ($6) DestroyLocationStruct($6);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = 0;
                                                        }
                        ;

ident:                  '[' NUM ']'                     {
                                                                $$ = $2;
								if ( $$ > highest_sym_used )
								  highest_sym_used = $$;
                                                        }
                        ;

opt_sym_flag_list:	/* NULL */			{
								$$ = 0;
							}
			| ':' sym_flag_list		{
								$$ = $2;
							}
			;

sym_flag_list:		sym_flag_token			{
								$$ = $1;
							}
			| sym_flag_token ',' sym_flag_list {
								$$ = $1 | $3;
							}
			;

sym_flag_token:		CG				{
								$$ = IFP_COMPILER_GENERATED;
							}
			| REP				{
								$$ = IFP_REPLACEABLE;
							}
			;

string_or_file:		STRING				{
								$$ = $1;
							}
			| TOK_FILE			{
								$$ = strdup("file");
							}
			;

optsource:              source                          {
                                                                $$ = $1;
                                                        }
                        | /* NULL */                    {
                                                                $$ = 0;
                                                        }
                        ;

source:                 bytestart                       {
                                                                $$ = CreateLocationStruct();
                                                                if ( $$ )
                                                                  {
                                                                     $$->start_line = 0;
                                                                     $$->start_column = $1;
                                                                     $$->end_line = 0;
                                                                     $$->end_column = 0;
                                                                     $$->filename = current_smt_filename;
                                                                     $$->relpath  = current_smt_relpath;
                                                                  }
                                                        }
                        | bytemap                       {
                                                                $$ = $1;
                                                        }
                        | linecolstart                  {
                                                                $$ = CreateLocationStruct();
                                                                if ( $$ )
                                                                  {
                                                                     $$->start_line = $1->val1;
                                                                     $$->start_column = $1->val2;
                                                                     $$->end_line = 0;
                                                                     $$->end_column = 0;
                                                                     $$->filename = current_smt_filename;
                                                                     $$->relpath  = current_smt_relpath;
								     DestroyPositionStruct($1);
                                                                  }
                                                        }
                        | linecolmap                    {
                                                                $$ = $1;
                                                        }
                        ;

bytemap:                bytestart bytelen               {
								$$ = CreateLocationStruct();
								if ( $$ )
								  {
								     $$->start_line = 0;
								     $$->start_column = $1;
								     $$->end_line = 0;
								     $$->end_column = $2;
								     $$->filename = current_smt_filename;
								     $$->relpath  = current_smt_relpath;
								  }
                                                        }
                        ;

bytestart:              NUM                             {
                                                                $$ = $1;
                                                        }
                        ;

bytelen:                NUM                             {
                                                                $$ = $1;
                                                        }
                        ;

linecolmap:             linecolstart bytelen            {
								$$ = CreateLocationStruct();
								if ( $$ )
								  {
								     $$->start_line = $1->val1;
								     $$->start_column = $1->val2;
								     $$->end_line = 0;
								     $$->end_column = $2;
								     $$->filename = current_smt_filename;
								     $$->relpath  = current_smt_relpath;
								     DestroyPositionStruct($1);
								  }
                                                        }
                        | linecolstart linecolend       {
								$$ = CreateLocationStruct();
								if ( $$ )
								  {
								     $$->start_line = $1->val1;
								     $$->start_column = $1->val2;
								     $$->end_line = $2->val1;
								     $$->end_column = $2->val2;
								     $$->filename = current_smt_filename;
								     $$->relpath  = current_smt_relpath;
								     DestroyPositionStruct($1);
								     DestroyPositionStruct($2);
								  }
                                                        }
                        ;

linecolstart:           NUM '/' NUM                     {
                                                                $$ = CreatePositionStruct();
                                                                if ( $$ )
                                                                  {
                                                                     $$->val1 = $1;
                                                                     $$->val2 = $3;
								  }
                                                        }
                        ;

linecolend:             NUM '/' NUM                     {
                                                                $$ = CreatePositionStruct();
                                                                if ( $$ )
                                                                  {
                                                                     $$->val1 = $1;
                                                                     $$->val2 = $3;
								  }
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for an attribute statement
*
***********************************************************************************/
                        
atr_statement:          ATR ident attr_list             {
                                                                int i, length;
                                                                if ( ModifyAttributes($2, $3) != 0 )
								  {
								    length = LengthOfAttributeList($3);
								    for ( i = 0; i < length; i++ )
								      {
									attributeStruct *as = GetElementNoAttributeList($3,i);
									if (as->name) free(as->name);
									if (as->value) free(as->value);
									DestroyAttributeStruct(as);
								      }
								    DestroyAttributeList($3);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = 0;
                                                        }
                        ;

attr_list:              name_part                       {
                                                                $$ = CreateAttributeList();
                                                                if ( $$ )
                                                                  {
                                                                    if ( InsertLastIntoAttributeList($$, $1) < 0 )
                                                                      {
                                                                        NonFatalError("Could not add to attribute list");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | attr_list ',' name_part       {
                                                                if ( InsertLastIntoAttributeList($1, $3) < 0 )
                                                                  {
                                                                    NonFatalError("Could not add to attribute list");
                                                                  }
                                                                $$ = $1;
                                                        }
                        ;

name_part:              STRING                          {
                                                                $$ = CreateAttributeStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->name = $1;
								    $$->value = 0;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError ( "Out of memory" );
                                                                    YYABORT;
                                                                  }
                                                        }
                        | STRING '(' STRING ')'         {
                                                                $$ = CreateAttributeStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->name = $1;
                                                                    $$->value = $3;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError ( "Out of memory" );
                                                                    YYABORT;
                                                                  }
                                                        }
                        | STRING '(' QSTRING ')'        {
                                                                $$ = CreateAttributeStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->name = $1;
                                                                    $$->value = $3;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError ( "Out of memory" );
                                                                    YYABORT;
                                                                  }
                                                        }
                         | STRING '(' NUM ')'           {
                                                                $$ = CreateAttributeStruct();
                                                                if ( $$ )
                                                                  {
								    char buffer[128];
								    sprintf ( buffer, "%d", $3 );
                                                                    $$->name = $1;
								    $$->value = strdup(buffer);
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError ( "Out of memory" );
                                                                    YYABORT;
                                                                  }
                                                        }
			| STRING '(' ident ')'		{
								$$ = CreateAttributeStruct();
                                                                if ( $$ )
                                                                  {
								    char buffer[128];
								    sprintf ( buffer, "%d", $3 );
                                                                    $$->name = $1;
								    $$->value = strdup(buffer);
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError ( "Out of memory" );
                                                                    YYABORT;
                                                                  }
							}
                        ;


/**********************************************************************************
*
* Grammar for an smt statement
*
***********************************************************************************/

smt_statement:          SMT ident opt_implicit source_list {
                                                                int i, length;
                                                                if ( AddReference($3 & IFP_IMPLICIT, $2, $4) != 0 )
								  {
								    length = LengthOfLocationList($4);
								    for ( i = 0; i < length; i++ )
								      DestroyLocationStruct(GetElementNoLocationList($4,i));
								    DestroyLocationList($4);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = NULL;
                                                        }
                        | SMT TOK_FILE QSTRING          {
                                                                if ( $3 )
                                                                  {
                                                                    if ( (($3)[0]) != '\0' )
								      {
									ifFileCache_insertFile($3, &current_smt_relpath,
											           &current_smt_filename);
								      }
								    else
								      {
									current_smt_filename = current_anon_target;
									current_smt_relpath  = current_anon_target;
								      }
								    if ($3) free($3);
								    ReportSmtFile(current_smt_filename);
                                                                  }
                                                                $$ = 0;
                                                        }
                        | SMT KEYWORD optkey source_list {
                                                                int i, length;
                                                                if ( AddKeywordReference($3, $4) != 0 )
								  {
								    length = LengthOfLocationList($4);
								    for ( i = 0; i < length; i++ )
								      DestroyLocationStruct(GetElementNoLocationList($4,i));
								    DestroyLocationList($4);
								    if ($3) free($3);
								  }
                                                                $$ = 0;
                                                        }
			| SMT LANGUAGE QSTRING		{
			                                        /* when transitioning from 1 language to the next  */
			                                        /* we need to keep track of the number of the last */
                                                                /* symbol used so that the symbol numbers of the   */
			                                        /* next language (which most likely re-start at 1) */
			                                        /* don't confilct with symbol numbers from the     */
                                                                /* previous language.                              */
                                                                if (current_smt_language) 
								  free(current_smt_language);
								current_smt_language = $3;
                                                                $$ = 0;
							}
                        ;

optkey:                 QSTRING                         {
                                                                $$ = $1;
                                                        }
                        | /* NULL */                    {
                                                                $$ = 0;
                                                        }
                        ;

opt_implicit:           IMPLICIT                        {
                                                                $$ = IFP_IMPLICIT;
                                                        }
                        | /* NULL */                    {
                                                                $$ = 0;
                                                        }
                        ;

source_list:            source                          {
                                                                $$ = CreateLocationList();
                                                                if ( $$ )
                                                                  {
                                                                    if ( InsertLastIntoLocationList($$, $1) < 0 )
                                                                      {
                                                                        NonFatalError("Could not add to location list");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | source_list ',' source        {
                                                                if ( InsertLastIntoLocationList($1, $3) < 0 )
                                                                  {
                                                                    NonFatalError("Could not add to location list");
                                                                  }
                                                                $$ = $1;
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for a relation statement
*
***********************************************************************************/

rel_statement:          REL ident_list QSTRING ident_list opt_attr_list {
                                                                int i, length;
								if ( ReportRelation($2, $3, $4, $5) != 0 )
								  {
								    if ($3) free($3); 
								    DestroyUintList($2); 
								    DestroyUintList($4); 
								    if ( $5 ) 
								      { 
									length = LengthOfAttributeList($5); 
									for ( i = 0; i < length; i++ ) 
									  { 
									    attributeStruct *as = GetElementNoAttributeList($5,i); 
									    if (as) 
									      { 
										if (as->name) free(as->name); 
										if (as->value) free(as->value); 
										DestroyAttributeStruct(as); 
									      } 
									  } 
									DestroyAttributeList($5); 
								      } 
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = NULL;
							      }

                        ;

ident_list:             ident                           {
                                                                $$ = CreateUintList();
                                                                if ( $$ )
                                                                  {
                                                                    if ( InsertLastIntoUintList($$, $1) < 0 )
                                                                      {
                                                                        NonFatalError("Could not add to identifier list");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | ident_list ',' ident          {
                                                                if ( InsertLastIntoUintList($1, $3) < 0 )
                                                                  {
                                                                    NonFatalError("Could not add to identifier list");
                                                                  }
                                                                $$ = $1;
                                                        }
                        ;


opt_attr_list:          attr_list                       {
                                                                $$ = $1;
                                                        }
                        | /* NULL */                    {
                                                                $$ = NULL;
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for a statement that reports parser errors and warnings
*
***********************************************************************************/

err_statement:          ERR source QSTRING              {
                                                                if( ReportError($2, $3) != 0 )
								  {
								    DestroyLocationStruct($2);
								    if ($3) free($3);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = 0;
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for a mapping statement
*
***********************************************************************************/

map_statement:          MAP COPY QSTRING QSTRING '{' map_list '}' {
                                                                int i, length;
								char *abs1 = 0;
								char *abs2 = 0;
								char *rel1 = 0;
								char *rel2 = 0;
								ifFileCache_insertFile($3, &rel1, &abs1);
								ifFileCache_insertFile($4, &rel2, &abs2);
								if ($3) free($3);
								if ($4) free($4);
                                                                if ( ReportTransformation(1, abs1, rel1, abs2, rel2, $6) != 0 )
								  {
								    length = LengthOfTransformationList($6);
								    for ( i = 0; i < length; i++ )
								      {
									transformationStruct *t = GetElementNoTransformationList($6,i);
									if ( t )
									  {
									    DestroyLocationStruct(t->source);
									    DestroyLocationStruct(t->target);
									    DestroyTransformationStruct(t);
									  }
								      }
								    DestroyTransformationList($6);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }
                        | MAP COPY QSTRING '{' map_list '}' {
                                                                int i, length;
								char *abs = 0;
								char *rel = 0;
								ifFileCache_insertFile($3, &rel, &abs );
								if ($3) free($3);
                                                                if ( ReportTransformation(1, abs, rel,
											  current_anon_target, current_anon_target, 
											  $5) != 0 )
								  {
								    length = LengthOfTransformationList($5);
								    for ( i = 0; i < length; i++ )
								      {
									transformationStruct *t = GetElementNoTransformationList($5,i);
									if ( t )
									  {
									    DestroyLocationStruct(t->source);
									    DestroyLocationStruct(t->target);
									    DestroyTransformationStruct(t);
									  }
								      }
								    DestroyTransformationList($5);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }
                        | MAP FIXED QSTRING QSTRING '{' map_list '}' {
                                                                int i, length;
								char *abs1 = 0;
								char *abs2 = 0;
								char *rel1 = 0;
								char *rel2 = 0;
								ifFileCache_insertFile($3, &rel1, &abs1);
								ifFileCache_insertFile($4, &rel2, &abs2);
								if ($3) free($3);
								if ($4) free($4);
                                                                if ( ReportTransformation(0, abs1, rel1, abs2, rel2, $6) != 0 )
								  {
								    length = LengthOfTransformationList($6);
								    for ( i = 0; i < length; i++ )
								      {
									transformationStruct *t = GetElementNoTransformationList($6,i);
									if ( t )
									  {
									    DestroyLocationStruct(t->source);
									    DestroyLocationStruct(t->target);
									    DestroyTransformationStruct(t);
									  }
								      }
								    DestroyTransformationList($6);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }
                        | MAP FIXED QSTRING '{' map_list '}' {
                                                                int i, length;
								char *abs = 0;
								char *rel = 0;
								ifFileCache_insertFile($3, &rel, &abs);
								if ($3) free($3);
                                                                if ( ReportTransformation(0, abs, rel,
											  current_anon_target, current_anon_target,
											  $5) != 0 )
								  {
								    length = LengthOfTransformationList($5);
								    for ( i = 0; i < length; i++ )
								      {
									transformationStruct *t = GetElementNoTransformationList($5,i);
									if ( t )
									  {
									    DestroyLocationStruct(t->source);
									    DestroyLocationStruct(t->target);
									    DestroyTransformationStruct(t);
									  }
								      }
								    DestroyTransformationList($5);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }
                        | MAP LINES QSTRING '{' maplinelist '}' {
								int i, length;
								char *rel = 0;
								char *abs = 0;
								ifFileCache_insertFile($3, &rel, &abs);
								if ($3) free($3);
								if ( ReportMaplines(abs, rel,
										    current_anon_target, current_anon_target,
										    $5) != 0 )
								  {
								    length = LengthOfMaplineList($5);
								    for ( i = 0; i < length; i++ )
								      {
									DestroyMaplineStruct(GetElementNoMaplineList($5,i));
								      }
								    DestroyMaplineList($5);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }       

                        | MAP LINES QSTRING QSTRING '{' maplinelist '}' {
								int i, length;
								char *abs1 = 0;
								char *abs2 = 0;
								char *rel1 = 0;
								char *rel2 = 0;
								ifFileCache_insertFile($3, &rel1, &abs1);
								ifFileCache_insertFile($4, &rel2, &abs2);
								if ($3) free($3);
								if ($4) free($4);
								if ( ReportMaplines(abs1, rel1, abs2, rel2, $6) != 0 )
								  {
								    length = LengthOfMaplineList($6);
								    for ( i = 0; i < length; i++ )
								      {
									DestroyMaplineStruct(GetElementNoMaplineList($6,i));
								      }
								    DestroyMaplineList($6);
								  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                        }       
                        ;

map_list:               source_target                   {
                                                                $$ = CreateTransformationList();
                                                                if ( $$ )
                                                                  {
                                                                    if ( InsertLastIntoTransformationList($$, $1) < 0 )
                                                                      {
                                                                        NonFatalError("Could not add to map list");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                                    
                                                        }
                        | map_list ';' source_target    {
                                                                if ( InsertLastIntoTransformationList($1, $3) < 0 )
                                                                  {
                                                                    NonFatalError("Cound not add to map list");
                                                                  }
                                                                $$ = $1;
                                                        }
                        ;

source_target:          source_loc ',' target_loc       {
                                                                $$ = CreateTransformationStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->source = $1;
                                                                    $$->target = $3;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        ;

source_loc:             linecolmap                      {
								$$ = $1;
                                                        }
                        | bytemap                       {
								$$ = $1;
                                                        }
                        ;

target_loc:             source                          {
								$$ = $1;
                                                        }
                        ;

optoutfile:             QSTRING                         {
								$$ = $1;
                                                        }
                        | /* NULL */                    {
								$$ = 0;
                                                        }
                        ;

maplinelist:            mapline                         {
								$$ = CreateMaplineList();
								if ( $$ )
								  {
								     InsertLastIntoMaplineList($$, $1);
								  }
                                                        }
			| maplinelist ';' mapline       {
								InsertLastIntoMaplineList($1, $3);
								$$ = $1;
							}
			;


mapline:                lineno lineno ',' lineno        {
								$$ = CreateMaplineStruct();
								if ( $$ )
								  {
								     $$->sourceFile   = 0;
								     $$->sourceStart  = $1;
								     $$->sourceEnd    = $2;
								     $$->targetFile   = 0;
								     $$->targetStart  = $4;
								  }
                                                        }
                        ;

lineno:                 NUM                             {
								$$ = $1;
                                                        }
                        ;

/**********************************************************************************
*
* Grammar for an abstract syntax tree
*
***********************************************************************************/

ast_statement:          AST astfile '{' treelist '}'        {
                                                                if ( !($2) )
								  {
                                                                    if ( ReportAST(current_smt_filename, current_smt_relpath, $4) != 0 )
								      DeallocateRecursiveAST($4);
								  }
								else if ( ($2)[0] == 0 )
								  {
								    if ( ReportAST(current_anon_target, current_anon_target, $4) != 0 )
								      DeallocateRecursiveAST($4);
								  }
								else
                                                                  {
								    char *rel = 0;
								    char *abs = 0;
								    ifFileCache_insertFile($2, &rel, &abs);
								    if ($2) free($2);
                                                                    if ( ReportAST(abs, rel, $4) != 0 )
								      DeallocateRecursiveAST($4);
                                                                  }
#if defined(COUNT_STRUCTS) && defined(REPORT_DETAILED)                                                            
                                                                DO_STRUCT_REPORT;
#endif
                                                                $$ = 0;
                                                        }
                        | AST astfile '{' '}'           {
			                                        if ( $2 )
								  free($2);
								$$ = 0;
			                                }
			;

astfile:                optoutfile                      {
                                                                $$ = $1;
                                                        }
                        ;

tree:                   astnode astloc '{' astbody '}'  {
                                                                $$ = CreateAstnodeStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->node = $1->kind;
                                                                    $$->identifier = $1->identifier;
                                                                    $$->generated = $1->generated;
                                                                    $$->definition = $1->definition;
								    $$->replaceable = $1->replaceable;
                                                                    $$->location = $2;
                                                                    if ( $4->type == ASTBODYTYPE_STRING )
                                                                      {
                                                                        $$->children = NULL;
                                                                        $$->value = $4->string;
                                                                      }
                                                                    else if ( $4->type == ASTBODYTYPE_TREELIST )
                                                                      {
                                                                        $$->children = $4->treelist;
                                                                        $$->value = NULL;
                                                                      }
                                                                    else 
                                                                      {
                                                                        NonFatalError("Undefined AST body type encountered");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                                DestroyNodeinfoStruct($1);
                                                                DestroyAstbodyStruct($4);
                                                        }
                        ;

astnode:                node opt_ast_flag_list        {
                                                                $$ = CreateNodeinfoStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->type = NODEINFO_PLAIN;
                                                                    $$->kind = $1;
                                                                    $$->identifier = 0;
								    $$->generated  = 0;
								    $$->definition = 0;
								    $$->replaceable = 0;
								    if ( $2 & IFP_COMPILER_GENERATED )
								      $$->generated = 1;
								    if ( $2 & IFP_DEFINITION )
								      $$->definition = 1;
								    if ( $2 & IFP_REPLACEABLE )
								      $$->replaceable = 1;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | node opt_ast_flag_list ident  {
                                                                $$ = CreateNodeinfoStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->type = NODEINFO_WITH_IDENT;
                                                                    $$->kind = $1;
                                                                    $$->identifier = $3;
								    $$->generated  = 0;
								    $$->definition = 0;
								    $$->replaceable = 0;
								    if ( $2 & IFP_COMPILER_GENERATED )
								      $$->generated = 1;
								    if ( $2 & IFP_DEFINITION )
								      $$->definition = 1;
								    if ( $2 & IFP_REPLACEABLE )
								      $$->replaceable = 1;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
							}
                        ;

astloc:			astloclist			{
								$$ = $1;
							}
			| /* NULL */			{
								$$ = 0;
							}
			;

astloclist:		realastloc			{
							    $$ = CreateLocationList();
							    if ( $$ )
							      {
								if ( InsertLastIntoLocationList($$, $1) < 0 )
								  {
								    NonFatalError("Could not add to location list");
								  }
							      }
							    else
							      {
								FatalError("Out of memory");
								YYABORT;
							      }
							}
			| astloclist ',' realastloc	{
							    if ( $1 )
							      {
								if ( InsertLastIntoLocationList($1, $3) < 0 )
								  {
								    NonFatalError("Could not add to location list");
								  }
							      }
							    else
							      {
								FatalError("Out of memory");
								YYABORT;
							      }
							    $$ = $1;
							}
			;

realastloc:		astfile source  		{
                                                            if ( $1 ) 
							      {
								if ( ($1)[0] == 0 )
								  {
								    $2->filename = current_anon_target;
								  }
								else
								  {
								    char *rel = 0;
								    char *abs = 0;
								    ifFileCache_insertFile($1, &rel, &abs);
								    $2->filename = abs;
								    $2->relpath  = rel;
								  }
								free($1);
							      }
							    $$ = $2;
                                                        }
                        ;


astbody:                QSTRING                         {
                                                                $$ = CreateAstbodyStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->type = ASTBODYTYPE_STRING;
                                                                    $$->string = $1;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | treelist                      {
                                                                $$ = CreateAstbodyStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->type = ASTBODYTYPE_TREELIST;
                                                                    $$->treelist = $1;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | /* NULL */                    {
                                                                $$ = CreateAstbodyStruct();
                                                                if ( $$ )
                                                                  {
                                                                    $$->type = ASTBODYTYPE_STRING;
                                                                    $$->string = 0;
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
							}
                        ;

treelist:               tree                            {
                                                                $$ = CreateAstnodeList();
                                                                if ( $$ )
                                                                  {
                                                                    if ( InsertLastIntoAstnodeList($$, $1) < 0 )
                                                                      {
                                                                        NonFatalError("Could not add to tree list");
                                                                      }
                                                                  }
                                                                else
                                                                  {
                                                                    FatalError("Out of memory");
                                                                    YYABORT;
                                                                  }
                                                        }
                        | treelist tree                 {
                                                                if ( $2 )
								  {
								    if ( InsertLastIntoAstnodeList($1, $2) < 0 )
								      {
									NonFatalError("Could not add to tree list");
								      }
								  }
								else
								  {
								    FatalError("Out of memory");
								    YYABORT;
								  }
                                                                $$ = $1;
                                                        }
                        ;   

node:                   STRING                          {
                                                                $$ = $1;
                                                        }
                        ;

opt_ast_flag_list:	/* NULL */			{
								$$ = 0;
							}
			| ':' ast_flag_list		{
								$$ = $2;
							}
			;

ast_flag_list:		ast_flag_token			{
								$$ = $1;
							}
			| ast_flag_token ',' ast_flag_list {
								$$ = $1 | $3;
							}
			;

ast_flag_token:		CG				{
								$$ = IFP_COMPILER_GENERATED;
							}
			| DEF				{
								$$ = IFP_DEFINITION;
							}
			| REP				{
								$$ = IFP_REPLACEABLE;
							}
			;

%%

static char *getNewAnonTarg(void)
{
  char buffer[256];
  char *newval = 0;
  char *rel    = 0;
  static unsigned int current = 0;

  sprintf(buffer, "::Anonymous-Target-%u::", current++);

  ifFileCache_insertFile(buffer, &rel, &newval);

  return newval;
}

/* resets the parser to initial state so that a single program can parse */
/* multiple files                                                        */

int iff_parser_reset(void)
{
  int retval = -1;
  
  /* reset line number (global, see iff_parser.l for declaration) */
  iff_parser_LineNumber = 1;
  
  /* Prime the current anonymous target before each parse            */
  if ( (current_anon_target = getNewAnonTarg()) != 0 )
    {
      /* before we start parsing, we prime the current_smt_filename with */
      /* the imiginary target file, because if no specific file is       */
      /* specifed with a location, and there is no SMT file directive    */
      /* yet, then we assume that all locations are relative to this     */
      /* imigainary file.  E.g. in the case of C/C++ the .i file never   */
      /* gets produced, yet all location not specifically specified      */
      /* otherwise are reletive to the .i file.                          */
      if ( current_smt_filename = current_smt_relpath = current_anon_target )
	{
	  retval = 1;
	} 
    }

  if (current_smt_language) {
    free(current_smt_language);
    current_smt_language = 0;
  }

  return retval;
}

int iff_parse(/*const*/ char *if_filename)
{
  int retval = -1;

  if ( if_filename )
    {
      /* redirect iff_dotlyin (the global FILE * that flex reads from) */
      extern FILE *iff_dotlyin;

      if ((iff_dotlyin = fopen(if_filename, "r")) != 0 )
	{
	  /* perform pre-parse callback */
	  if ( iff_parser_preparse_CB() >= 0 )
	    {
	      /* reset the parser data structures */
	      if ( iff_parser_reset() >= 0 )
		{
		  if ( iff_dotlyparse() == 0 ) /* zero is success for yyparse() */
		    {
		      /* perform post-parse callback */
		      if ( iff_parser_postparse_CB() >= 0 )
			retval = 1;
		    }
		}
	    }
	  fclose(iff_dotlyin);
	}
    }

  return retval;
}

int FatalError(char const *msg)
{
  yyerror(msg);
  return 0;
}

int NonFatalError(char const *msg)
{
  yyerror(msg);
  return 0;
}
  
void DeallocateRecursiveAST(astnodeList *l)
{
  int i;

  if ( l )
    {
      int length = LengthOfAstnodeList(l);
      for ( i = 0; i < length; i++ )
	{
	  astnodeStruct *a = GetElementNoAstnodeList(l,i);
	  if ( a )
	    {
	      if ( a->node )
		free(a->node);
	      
	      if ( a->location )
		{
		  int j;
		  int length2 = LengthOfLocationList(a->location);
		    for ( j = 0; j < length2; j++ )
		      DestroyLocationStruct(GetElementNoLocationList(a->location,j));
		  DestroyLocationList(a->location);
		}
	      
	      if ( a->children )
		DeallocateRecursiveAST(a->children);

	      if ( a->value )
		free(a->value);

	      DestroyAstnodeStruct(a);
	    }
	}
      DestroyAstnodeList(l);
    }
}

              
  



