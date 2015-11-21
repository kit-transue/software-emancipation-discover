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
// commandline.h
// 5.feb.98 kit transue
// parse commandline options to makefile

#if !defined(_COMMANDLINE_H_)
#define _COMMANDLINE_H_

#pragma warning(disable:4786)

#include <string>
#include <set>
#include <deque>
#include "macro_node.h"
using namespace std;


class commandline {
public:
	// construct/copy/destroy
	commandline();
	commandline(string const &);  // includes argv[0] (executable name)

	// access
	bool optionNoBuiltinInference() const;	// /R
	bool optionEnvVarOverride() const;	// /E
	bool optionBatchInferDisable() const;	// /Y
	string const & makefilename() const;
	set<string> const & target() const;
	string const & argvzero() const;  // command

	//macro definitions:
	deque<pair<string, macro_node> > defines;
private:
	// modify
	void add_define(string const &);
	void add_target(string const &);
	void add_makefile(string const &);
	bool noBuiltinInference;
	bool envVarOverride;
	bool batchInferDisable;

	// should be collection, since multiple names can be specd on one line:
	string fname; 
	string argv0;
	set<string> targets;
};

#endif // !defined(_COMMANDLINE_H_)
