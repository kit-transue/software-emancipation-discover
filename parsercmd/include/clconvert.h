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
// An object of this class is based on a particular
// compiler on a particular operating system.  It serves
// to translate a command line used with that compiler
// to a command line for use with aset_CCcc.

// Since it parses a command line, it retains a parse state.

#ifndef CLCONVERT_H
#define CLCONVERT_H

#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#else
#include <iostream.h>
#include <strstream.h>
#endif
#include <queue>
#include "clinterpret.h"
#include <string>
namespace std {};
using namespace std;

class CommandLineConverter {
public:
    CommandLineConverter(CommandLineInterpreter &,
	list<string> &i, list<string> &o);
    ~CommandLineConverter();
    bool convert(string &diagnostic);
    bool command_line();
    bool complete_option();
    bool option_token(queue<const OptionInterpreter *> &opt);
    bool unrecognized_option(const string &);
    bool nonwhite(string &);
    bool whitespace();
    bool at_whitespace();
    bool argument(string &);
    bool at_argument();
    bool eof();
private:
    CommandLineInterpreter *interp;
    list<string> &iargs;
    list<string>::iterator iargp;
    istream *istrm;
    list<string> &oargs;
    bool fault; // Flag set if the parser could not backtrack
	        // and istrm state is incorrect.  This should relate to
		// parser or config file problems, not just a
		// command line which uses unexpected syntax.
    string *diagnostic;

    void interpret(const OptionInterpreter *interp, string *arg);
    void makeDiagnostic(const string&);
    bool next_arg();
    bool fetch_arg();
#ifdef ISO_CPP_HEADERS
    void makeDiagnostic(ostringstream&);
#else
    void makeDiagnostic(ostrstream&);
#endif

    // Preallocated data structures for option_token:
    int num_nameintros;
    string *nameintros; // = interp->nameintro();
    bool *maybe_name; // = nameintro != NULL;
    bool *is_name; // = false; // true if nameintro detected
    int *intro_len; // = maybe_name ? nameintro->length() : -1;
    NameSelector **name_selectors;
};

#endif // CLCONVERT_H
