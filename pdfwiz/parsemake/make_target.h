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
// make_target.h
// 16.Dec.97 Kit Transue

#if !defined(_MAKE_TARGET_H)
#define _MAKE_TARGET_H

#pragma warning(disable:4786)

#include <string>
#include <set>
#include <vector>
#include <utility>

#include "sort_objects.h"
#include "pathmgr.h"
#include "action.h"

using namespace std;

class make_target
{
public:
	// public types
	typedef set<string, less<string> > stringpset;
	typedef vector<string> stringpvec;

	// construct/copy/destroy
	make_target(string const &targetname, directory_vector const &dir, env_table const& env);

	// comparisons
	bool operator< (make_target const &) const;

	// modifiers
	string const &add_dependent(string const &dependfile);
	action const &add_action(string const &action_text);

	// access
	string const &name() const;
	stringpset const &dependents() const;
	vector<action> const &actions() const;
	action const *compile_action() const;

	// in Microsoft nmake, the working directory and environment is
	// maintained as the makefile is parsed; this allows one make
	// action (if it's executed) to change the working directory for
	// another make action.  I'm isolating us from this foolery, by
	// keeping a working directory for each target:
	directory_vector cwd;
	env_table environment;
private:
	string tname;
	stringpset dependent_set;
	vector<action> action_vec;
	int compile_line_index;
};

#endif // _MAKE_TARGET_H
