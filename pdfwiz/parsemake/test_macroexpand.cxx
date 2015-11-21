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
/* parser for makefiles */


/* Input from the makefile is processed in three layers:
	a preprocessing layer that expands macros (using lex)
	a standard makefile parser that identifies rules/actions (using lex)
	a parser for microsoft extensions (using a lexer and yacc)

	Lines are processed one at a time.  Data are fed from one lexer
	to the next by using a strstream like a pipe between each.  The
	first two lexers are called sequentially; the second calls
	yacc if necessary to parse the extensions; yacc in turn uses
	the last lexer to get its input.
*/

#pragma warning(disable:4503)

#include <iostream>
#include "pipebuf.h"


#undef yyFlexLexer
#define yyFlexLexer yyRecurseMacroFlexLexer
#include <FlexLexer.h>


int main()
{
	iostream pipe1(new pipebuf);
	
	static yyRecurseMacroFlexLexer macroExpander(&pipe1, &pipe1);
	pipe1 << "test$d" << endl;
	while (macroExpander.yylex() == 2);
	char foo[1000];
	pipe1.getline(foo, sizeof(foo)/sizeof(foo[0]));
	cout << foo;
	return 0;
}

