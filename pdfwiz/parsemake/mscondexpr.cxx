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
#include "mscondexpr.h"
#include "pipebuf.h"

#undef yyFlexLexer
#define yyFlexLexer yyMSExprFlexLexer
#include <FlexLexer.h>


using namespace std;

msCondExprParserClass msCondExprParser;

msCondExprParserClass::msCondExprParserClass()
{
	thestream = new iostream(new pipebuf);
	msexprLexer = new yyMSExprFlexLexer;
	msexprLexer->switch_streams(thestream, &cerr);
}

msCondExprParserClass::~msCondExprParserClass()
{
	delete msexprLexer;
	streambuf * pipebufptr = thestream->rdbuf();
	delete thestream;
	delete pipebufptr;
}

int 
msCondExprParserClass::yylex()
{
	return msexprLexer->yylex();
}

extern "C" int
yylex()
{
	return msCondExprParser.yylex();
}

extern "C" int
yyerror(char *s)
{
	cerr << s << endl;
	return 0;
}

bool
msCondExprParserClass::evaluate_line(const string &s)
{
	thestream->clear();
	*thestream << s;
	yyparse();
	return (parseexpr_y_retval != 0);
}
