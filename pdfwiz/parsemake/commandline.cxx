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
#pragma warning(disable:4786)
#include <cassert>
#include <strstream>
#include "commandline.h"

#include "filename.h"
#include "option_extractor.h"



// construct/copy/destroy

commandline::commandline() : noBuiltinInference(false),
	envVarOverride(false), batchInferDisable(false), fname("Makefile")
{
}

commandline::commandline(string const &arguments) : noBuiltinInference(false),
	envVarOverride(false), batchInferDisable(false), fname("Makefile")
{
	istrstream s(arguments.c_str());

	// extract commandline:
	if (!s.eof())
		s >> ntfname_extractor(argv0);

	string option;
	string filename;
	while (!s.eof()) {
		s >> option_extractor(option);
		if (!option.empty()) {	// option specified:
			switch (option[0]) {
				case 'R':
					noBuiltinInference = true;
					break;
				case 'E':
					envVarOverride = true;
					break;
				case 'Y':
					batchInferDisable = true;
					break;
				case 'F':
					s >> ntfname_extractor(filename);
					add_makefile(filename);
					break;
				default:
					// ignore other arguments
					break;
			}
		}
		else {	// target or define
			string param;
			s >> ntfname_extractor(param);
			if (param.find_first_of('=') != string::npos)
				add_define(param);
			else if (! param.empty() )
				add_target(param);
		}
	}
}




// modify
void
commandline::add_define(const string &s)
{
	string::size_type pos = s.find('=');
	defines.push_back(pair<string,macro_node>(s.substr(0,pos), macro_node(s.substr(pos + 1, string::npos), -2)));
}

void
commandline::add_target(const string &s)
{
	targets.insert(s);
}

void
commandline::add_makefile(const string &s)
{
	fname = s;
}


// accessor functions

string const &
commandline::argvzero() const
{
	return argv0;
}

set<string> const &
commandline::target() const
{
	 return targets;
}

bool
commandline::optionNoBuiltinInference() const
{
	return noBuiltinInference;
}

bool
commandline::optionEnvVarOverride() const
{
	return envVarOverride;
}

bool
commandline::optionBatchInferDisable() const
{
	return batchInferDisable;
}

string const &
commandline::makefilename() const
{
	assert(!fname.empty());	 // initialized by constructor to Makefile.
	return fname;
}


#if defined(UNIT_TEST)


int main()
{
	string input;
	getline(cin, input);

	commandline c(input);
	cout << "makefile is: " << c.makefilename() << endl;
	cout << "target is: " << c.target() << endl;

	return 0;
}
#endif  // UNIT_TEST
