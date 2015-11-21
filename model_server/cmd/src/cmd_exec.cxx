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
// file cmd_exec.C
// 
//
// History:  10/02/91    M.Furman   Initial coding.

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#include <ctype.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#include <cctype>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

#include "cmd.h"
#include "genError.h"
#include "genString.h"
#include "systemMessages.h"
#include "messages.h"
#include "msg.h"

// Fills genString txt with text of a specified file. Returns 
// -1 if a file does not exist;
//  0 if file is not an ASCII file
//  1 if succeded
//
static int get_file_text (const char *fname, genString& txt)
{
    Initialize(get_file_text);
    int count = 0;
    char *buf = 0;
    char ch;

    ifstream temp_in( fname, ios::in );
    if ( !temp_in )		// open failed ?
	return -1;

    while (temp_in.get(ch))	// character count and type check  
    {
	count++ ;
	if(! isascii(ch))
	    return 0;
    }

    // force a new line at the end

    buf = new char[++count + 1];
    temp_in.close();		// beginning of file
    count = 0;

    ifstream second_temp_in( fname, ios::in );
    while (second_temp_in.get(ch)) 
    {
        buf[count] = ch;
        count++ ;
    }
    second_temp_in.close();
    buf[count++] = '\n';              
    buf[count] = '\0';		// Instead of EOF for LEX.
      
    txt = buf; 
    delete buf;
    return 1;
}

// Adds uniqe file name before single block
//
//
static void check_func_or_block (genString& txt)
{
    Initialize(check_func_or_block);
    static int b_counter = 0;

    char* body = txt;
    body = strchr(body, '{'); // find first left brace and ignore everything before
    if (body){
	char buf[40];
	char* body_end = strrchr(body, '}'); // find last right brace and ignore
	if (body_end)                       // everyting after
		  body_end[1] = '\0';
	if (b_counter)
	    sprintf (buf, "%s%d%s","___cmd_journal", b_counter," () ");
	else
	    sprintf (buf, "%s%s", "___cmd_journal", " () ");		
	b_counter++;
	genString n_txt(buf);
        n_txt += body;
	IF (!body_end)                       // everyting after
	    n_txt += "}";
        txt.put_value ((const char *)n_txt);
    }
}

extern "C" void execute_regression(char *s);

void cmd_execute(char *fname)
{
    Initialize(cmd_execute);

    if (fname == 0)
	return;

    genString txt;
    int res = get_file_text (fname, txt);
    if (res == -1)
    {
	msg("ERROR: File $1 does not exist.") << fname << eom;
	return;
    }
    else if (res == 0)
    {
	msg("ERROR: File \'$1\' is not ASCII.") << fname << eom;
	return;
    }

    /* should be renamed "parse_regression_text: */
    execute_regression ((char *)txt);
}


