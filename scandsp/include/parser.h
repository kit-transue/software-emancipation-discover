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
#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>

// ===========================================================================
// Class which is extensible to create a backtracking parser.
// Input is a null-terminated array of char in order to backtrack readily.
// Has public member functions for basic parsing actions.
// Any parse action returns a bool to indicate success/failure,
// and in case of failure backtracks the input.
//
// When a derived class creates more sophisticated parser actions,
// it should use the same techniques for backtracking that are exemplified
// in the implementation of the Parser class.  See parser.C.
// ===========================================================================

class Parser {
public:
    Parser(const char *input);

    bool parseChar(char);
    bool parseItem(char &);
    bool peek(char &);
    bool parseSatisfying(char &, bool (*)(char));
    bool parseSatisfying(std::string &, bool (*)(char));
    bool word(std::string &);
    bool whitespace(bool requireSubstantial = true);
    bool quotedString(std::string&);
    bool bracketedString(std::string&);
    bool matchChars(const char *);
    bool matchWord(const char *);
    bool parseDelimiter(std::string &);

    const char *peek();

private:
    class tracker {
    public:
	tracker(Parser &);
	bool fail();
	operator bool();
    private:
	Parser &parser;
	const char *pos;
	bool ok;
    };
    friend tracker;
    const char *curChar;
};

#endif // PARSER_H
