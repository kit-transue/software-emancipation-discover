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
// macro_expander.cxx
// wrapper for lexer to expand macros in strings
// oct.97 kit transue

#include <cassert>

#include "pipebuf.h"
#include "macro_node.h"
#include "variabletable.h"
#include "message.h"

#include "macro_expander.h"

// global variables

static macro_expander GlobalMacroExpander;
macro_expander * global_macroexpander = &GlobalMacroExpander;


// macroexpander implementation
// construct/copy/destroy
macro_expander::macro_expander()
{
	inpipe = new iostream(new pipebuf);
	outpipe = new iostream(new pipebuf);
}

macro_expander::~macro_expander()
{
	streambuf * pipebufptr = inpipe->rdbuf();
	delete inpipe;
	delete pipebufptr;
	pipebufptr = outpipe->rdbuf();
	delete outpipe;
	delete pipebufptr;
}

void
macro_expander::expand_macros_using_streams(variabletable *vt)
{
	int const MAX_LEVEL = 20;
	int status;
	int level = 0;
	while( (status = do_expand(vt)) == 2 && (++level < MAX_LEVEL)) {
		inpipe->clear();
		swap(inpipe, outpipe);
	}
	if (level >= MAX_LEVEL)
		msgid("Too many levels in macro expansion") << eom;
}

void
macro_expander::expand_one_macro_in_string(string &value, string const &name, variabletable *vt)
{
	string::size_type pos = 0;
	for (pos = value.find('$', pos); pos != string::npos; pos = value.find('$', pos)) {
		if (value.size() > pos) {
			string::size_type pattern_start = pos + 1;
			string::size_type pattern_end = pos + 1;
			string::size_type subs_end = pattern_end;
			if (value[pattern_start] == '(') {
				++pattern_start;
				subs_end = value.find(')', pattern_start);
				pattern_end = subs_end - 1;
			}
			else if (value[pattern_start] == '*' && value[pattern_start + 1] == '*') {
				++pattern_end;
				++subs_end;
			}
			if (name == value.substr(pattern_start, pattern_end - pattern_start + 1)) {
				value.replace(pos, subs_end - pos + 1,
					(vt->lookup(name)).value);
			}
			++pos;
		}
	}
}

string
macro_expander::expand_macros_in_string(string const &s, variabletable *vt)
{
	string ret;
	// make sure all pending stuff has been properly consumed
	assert(inpipe->peek() == EOF);
	assert(outpipe->peek() == EOF);

	inpipe->clear();
	outpipe->clear();
	*inpipe << s;
	expand_macros_using_streams(vt);
	getline(*outpipe, ret, (char) 0);
	return ret;
}

int
macro_expander::do_expand(variabletable *vt)
{
	// replacement for lexer.  returns 2 if macro expanded; 0 on eof.
	bool expansionmade = false;
	char c;

	while (inpipe->get(c)) {  // get returns stream reference, convert to bool
		if (c == '$') {
			expansionmade = true;
			string macro;
			// macro referenced, and macros don't nest!
			if (inpipe->peek() == '(') {
				inpipe->ignore();	// consume open paren
				getline(*inpipe, macro, ')'); 	// get macro text
				if (inpipe->peek() == ')')
					inpipe->ignore();
			}
			else {	// just a single character:
				inpipe->get(c);
				macro += c;
				// well, usually: may be $**:
				if (c == '*' && inpipe->peek() == '*') {
					macro += '*';
					inpipe->ignore();
				}
			}

			// macro established. Now expand:
			macro_node const &expanded = vt->lookup(macro);
			if (&expanded != macro_node::null_macro)
				*outpipe << expanded.value;
		}
		else
			*outpipe << c;
	}
	return (expansionmade) ? 2 : 0;
}
