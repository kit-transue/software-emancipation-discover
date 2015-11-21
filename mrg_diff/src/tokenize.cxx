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
#include <mrgFile.h>
#include <cLibraryFunctions.h>
#include <machdep.h>

static mrgFile *curr_file = 0;

enum {
    TOKEN,
    WHITE,
    NEWLINE,
    UNTOK
};

int dis_mrg_char_type ( char c )
{
    int tp = UNTOK;
    if ( c == '_' || c == '$' || isalnum(c) )
	tp = TOKEN;
    else if ( c == '\t' || c == ' ' )
	tp = WHITE;
    else if ( c == '\n' ||  c == '\015' )
	tp = NEWLINE;

    return tp;
}

/*
C [_a-zA-Z0-9\$]
U [^ \t\n\r_a-zA-Z0-9\$]
W [ \t\n\r]
%%
{W}+   { output_white(yytext,yyleng,mrg_diff_offset);     mrg_diff_offset += yyleng; }
{C}+   { output_token(yytext,yyleng,mrg_diff_offset);     mrg_diff_offset += yyleng; }
{U}+   { output_untok(yytext,yyleng,mrg_diff_offset);     mrg_diff_offset += yyleng; }
*/
void dis_mrg_diff_tokenize (FILE *fin)
{
    if (!fin) return;

    int off = 0;
    char c;
    int state = -1;
    
    while ( (c = getc(fin)) != EOF ) {
	int char_type = dis_mrg_char_type (c);
	if ( state == -1 || state != char_type ) {
	    curr_file->append (off);
	    state = char_type;
	}
	off ++;
    }
}

int dis_lex_tokenize (const char *fname, mrgFile *cur)
{
    curr_file = cur;
    FILE *fin;
#ifdef _WIN32
    if(OSapi_getenv("DIS_NLUNIXSTYLE"))
      fin = OSapi_fopen ((char *)fname, "rt");
    else 
#endif
      fin = OSapi_fopen ((char *)fname, "r");
    
    dis_mrg_diff_tokenize (fin);

    OSapi_fclose (fin);
    int res = cur->tokens();
    curr_file = 0;
    return res;
}

