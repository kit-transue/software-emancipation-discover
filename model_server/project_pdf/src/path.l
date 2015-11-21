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
%p 9000
%{
#include <stdio.h>
#include <ctype.h>

#if 0
#include "y.tab.h"
#else
#include "path_parser.h"
#ifndef _patherr_h
#include "patherr.h"
#endif
#endif

#if 0
#define my_print(a,b)    fprintf(a,b)
#define my_print0(a,b,c) fprintf(a,b,c)
#else
#define my_print(a,b)
#define my_print0(a,b,c)
#endif

#include <path_l.h>

static int path_lineno = 1;

char *path_sys_vars_translate();

void init_path_line_no()
{
    path_lineno = 1;
}

static char last_token_is_right_brace = 0;
static int ret_token;

extern int inside_proj_count; /* defined in path.y */
extern void pdf_cli_start();
extern void pdf_cli_line();
%}

NT_PATH   ([a-zA-Z]:[\\/][^<>{}:= \r\t\n]+)|([a-zA-Z]:[\\/])
UNIX_PATH ([^<>{}:= \r\t\n]+)
PATH      {NT_PATH}|(\({NT_PATH})|{UNIX_PATH}

%s   CLI
%%

<CLI>.*\n                  {pdf_cli_line(yytext);}
<CLI>.*                    {pdf_cli_line(yytext);pdf_cli_line("\n");}

^"# "[0-9]+"\r"?"\n"  { path_lineno = atoi(yytext+2); }

"#dis_tcl#".*     {pdf_cli_start(); BEGIN CLI;}

"#".*         { my_print(stderr," COMMENT\n"); }

"[[".*"]]"    { my_print0(stderr,"LEX SELECTOR: %s\n",yytext);
                last_token_is_right_brace = 0;
                auto_add_string();
                return SELECTOR; }

\"([^\n"]|[\\](a|[^a]))*\" { my_print0(stderr,"\"WILDCARD\": %s\n",yytext);
                last_token_is_right_brace = 0;
                auto_add_string();
                return WILDCARD; }

"<->"         { my_print(stderr,"LEX: <->\n");
                if (last_token_is_right_brace)
                    ret_token = NULL_TWO_WAY_SYMBOL;
                else
                    ret_token = TWO_WAY_SYMBOL;

                last_token_is_right_brace = 0;
                return ret_token; }

"->"          { my_print(stderr,"LEX: ->\n");
                last_token_is_right_brace = 0;
                return RIGHT_WAY_SYMBOL; }

"<-"          { my_print(stderr,"LEX: <-\n");
                last_token_is_right_brace = 0;
                return LEFT_WAY_SYMBOL; }
"<=>"          { my_print(stderr,"LEX: <=>\n");
                return BAD_TOKEN; }

"=>"(([ \t\n\r])*.+) {
                my_print0(stderr,"LEX: %s\n", yytext);
                last_token_is_right_brace = 0;
                auto_add_trimed_string(yytext+2);
                return RIGHT_WAY_REALPATH_SYMBOL; }

"{"           { my_print(stderr,"LEX: { \n");
                last_token_is_right_brace = 1;
                return LEFT_CURLY_SQ; }

"}"           { my_print(stderr,"LEX: } \n");
                last_token_is_right_brace = 0;
                return RIGHT_CURLY_SQ; }

[:=]          { my_print(stderr,"LEX: :=\n");
                last_token_is_right_brace = 0;
                return yytext[0]; }

(\^\"[a-zA-Z]:[\\/][^<>{}:=\"\r\t\n]+\")|(\^\"[^<>{}:=\"\r\t\n]+\") {
                last_token_is_right_brace = 0;
                auto_add_string();
                my_print0(stderr,"^\"WILDCARD\": %s\n",yytext);
                return WILDCARD; }


(\^[a-zA-Z]:[\\/][^<>{}:= \r\t\n]+)|(\^[^<>{}:= \r\t\n]+) {
                last_token_is_right_brace = 0;
                auto_add_string();
                if (inside_proj_count)
                {
                    my_print0(stderr,"^WILDCARD: %s\n",yytext);
                    ret_token = WILDCARD;
                }
                else
                {
                    my_print0(stderr,"TOP_EXCLUDE_PROJ: %s\n",yytext);
                    ret_token = TOP_EXCLUDE_PROJ;
                }
                return ret_token; }

{PATH} { 
		 my_print0(stderr,"WILDCARD: %s\n",yytext);
                last_token_is_right_brace = 0;
                auto_add_string();
                return WILDCARD; }

\n            { path_lineno++; }

[ \r\t\n]       ;

%%


/* type = 1 needs a leading " */
auto_add_string()
{
    auto_add_string_internal(yytext);
}

auto_add_trimed_string(str)
char* str;
{
	char c, *beg, *end;
  beg = str;
  while ( *beg == ' ' || *beg == '\t' || *beg == 0x0a || *beg == 0x0d )
      beg++;

  end = beg + strlen(beg) -1;
  while ( *end == ' ' || *end == '\t' || *end == 0x0a || *end == 0x0d )
      end--;
  c= *(++end);

  *end = 0;
  auto_add_string_internal( beg );
  *end = c;
}

auto_add_string_internal(str)
char* str;
{
    /* We need the original string for the error code generator. */
    /* Do the replacement later. */
    /* yylval.string = path_sys_vars_translate(yytext); */
    int len;
    char *p = str;

    len = strlen(p);
    if (*p == '\"')
    {
        if (len == 1 || *(p+len-1) != '\"')
            create_one_path_parser_error(PATH_PARSE_SYNTAX_ERR, yytext, 0, 0, 0);
    }

    if (len >= 2 && *p == '\"' && *(p+len-1) == '\"')
    {
        *(p+len-1) = '\0';
        p++;
        len -= 2;
    }

    pathlval.string = (char *)malloc(len + 1);
    strcpy(pathlval.string, p);

    return 0;
}


void patherror(s)
char *s;
{
/*  UI will display all error codes. */
/*  fprintf(stderr,"%s in line %d\nThe token is \"%s\"\n",s, path_lineno, yytext); */
    create_one_path_parser_error(PATH_PARSE_SYNTAX_ERR, yytext, 0, 0, 0);
}

int get_path_parse_line_num()
{
    return path_lineno;
}

void yy_path_init_input (f)
    FILE * f;
{
    yyin = f;
}

void pathunput(int c)
{
    unput(c);
}
