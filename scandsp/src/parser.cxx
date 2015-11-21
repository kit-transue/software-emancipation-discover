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
// The parser functions all reset to their original positions in case
// of failure.  This is the responsibility of the failing function
// because in case of failure there's no further use of the point
// of failure.
// If the calling function then fails as a consequence, and immedidately resets
// still farther back, so be it.

// A tracker holds a reference to the input stream, and when initialized,
// records that state.  Tracker::fail resets the stream to its original
// position and returns false. Tracker::operator bool() returns false
// if fail has been called. 
// Every parsing function should declare a tracker at its outset, and
// return the tracker (implicitly converted to bool) at the end.
// No stream operations may be performed between the call to fail
// and the return.

#include "parser.h"
#include "ctype.h"
using namespace std;

// ===========================================================================
// ===========================================================================
// The generic parsing.
// ===========================================================================
Parser::tracker::tracker(Parser &par)
: parser(par), ok(true)
{
    pos = parser.curChar;
}

// ===========================================================================
bool Parser::tracker::fail() 
{
    parser.curChar = pos;
    return ok = false;
}

// ===========================================================================
Parser::tracker::operator bool()
{
    return ok;
}

// ===========================================================================
Parser::Parser(const char *input)
  : curChar(input)
{
}

// ===========================================================================
bool Parser::parseChar(char requiredChar)
{
    tracker isOk(*this);
    char ch;
    (parseItem(ch) && ch == requiredChar) || isOk.fail();
    return isOk;
}

// ===========================================================================
bool Parser::parseItem(char &ch)
{
    ch = *curChar;
    bool ok = (ch != '\0');
    if (ok) {
	curChar += 1;
    }
    return ok;
}

// ===========================================================================
bool Parser::parseSatisfying(char &ch, bool (*predicate)(char))
{
    tracker isOk(*this);
    (parseItem(ch) && predicate(ch)) || isOk.fail();
    return isOk;
}

// ===========================================================================
bool Parser::peek(char &ch)
{
    ch = *curChar;
    return (ch != '\0');
}

// ===========================================================================
static bool isOfWord(char ch)
{
    return (isalnum(ch) || ch == '_');
}

// ===========================================================================
// This function is unusual in that it appends to the string argument.
//
bool Parser::parseSatisfying(string &str, bool (*predicate)(char))
{
    for (;;) {
	char ch;
	if (!parseSatisfying(ch, predicate)) {
	    break;
	}
	str.append(1, ch);
    }
    return true;
}

// ===========================================================================
bool Parser::word(std::string &str)
{
    str = "";
    bool ok = parseSatisfying(str, &isOfWord);
    return ok && str.length() > 0;
}

// ===========================================================================
static bool isSpaceOfLine(char ch) {
    return (isspace(ch) && ch != '\r' && ch != '\n');
}

// ===========================================================================
bool Parser::whitespace(bool requireSubstantial)
{
    bool have_char = false;
    for (;;) {
	char ch;
	if (!parseSatisfying(ch, &isSpaceOfLine)) {
	    break;
	}
	have_char = true;
    }
    return have_char || !requireSubstantial;
}

// ===========================================================================
static bool isQuotable(char ch) {
    return (ch != '"' && ch != '\r' && ch != '\n');
}

// ===========================================================================
bool Parser::quotedString(string &str)
{
    tracker isOk(*this);
    parseChar('"') || isOk.fail();
    string enclosed;
    if (isOk) {
	parseSatisfying(enclosed, &isQuotable);
	parseChar('"') || isOk.fail();
    }
    if (isOk) {
        str = "\"";
	str.append(enclosed);
	str.append("\"");
    }
    return isOk;
}

// ===========================================================================
static bool isBracketable(char ch) {
    return (ch != '>' && ch != '\r' && ch != '\n');
}

// ===========================================================================
bool Parser::bracketedString(std::string &str)
{
    tracker isOk(*this);
    parseChar('<') || isOk.fail();
    if (isOk) {
	str = "";
	parseSatisfying(str, &isBracketable);
	parseChar('>') || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
static bool isOfDelimiter(char ch)
{
    return (!isalnum(ch) && !isspace(ch));
}

// ===========================================================================
bool Parser::parseDelimiter(string &str)
{
    tracker isOk(*this);
    char ch;
    if (parseSatisfying(ch, &isOfDelimiter)) {
	str = "";
	str.append(1, ch);
	if (ch == '=') {
	    // Try for ==.
	    tracker after1(*this);
	    if (parseChar('=')) {
		str.append(1, ch);
	    }
	    else {
		after1.fail();
	    }
	}
    }
    else {
	isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool Parser::matchChars(const char *word)
{
    tracker isOk(*this);
    const char *p = word;
    for (;;) {
	if (*p == '\0') {
	    break;
	}
	if (!parseChar(*p)) {
	    isOk.fail();
	    break;
	}
	p += 1;
    }
    return isOk;
}

// ===========================================================================
bool Parser::matchWord(const char *wordChars)
{
    tracker isOk(*this);
    string str;
    (word(str) && str == wordChars) || isOk.fail();
    return isOk;
}

// ===========================================================================
const char *Parser::peek()
{
    return curChar;
}
