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
// make_target.cxx
// 17.Dec.97 Kit Transue

#pragma warning(disable:4786)
#include <string>

#include "make_target.h"
#include "make_toplevel.h"
#include "variabletable.h"
#include "filename.h"
#include "macro_expander.h"
#include "pdfwiz_settings.h"

using namespace std;


// construct/copy/destroy
make_target::make_target(string const &targetname, directory_vector const& wd, env_table const &env) :
	tname(targetname),
	compile_line_index(-1),
	cwd(wd),
	environment(env)
{
}

// comparisons
bool
make_target::operator< (make_target const & mt) const
{
	return tname < mt.tname;
}

// modifiers
string const &
make_target::add_dependent(string const &dependfile)
{
	return *(dependent_set.insert(dependfile).first);
}


action const &
make_target::add_action(string const &action_text)
{
	action_vec.push_back(action(action_text, cwd, environment));
	action const *ret = &action_vec.back();
	// check for compiler name:
	if (compile_line_index == -1) {
		string const &compilers = global_macroexpander->expand_macros_in_string(settings::compiler_list(), makefile::globalmakefile->symbols);
		if (filename::is_command(action_text, compilers)) {
			compile_line_index = action_vec.size() - 1;
		}
	}
	// check for cd:
	if (filename::is_command(action_text, "cd")) {
		string::size_type start = action_text.find_first_of(" \t");
		start = action_text.find_first_not_of(" \t", start);
		if (start != string::npos) {
			cwd.cd(action_text.substr(start));
		}
	}
	// and this is really the place and the way to create make actions
	return *ret;
}


// access
string const &
make_target::name() const
{
	return tname;
}

make_target::stringpset const &
make_target::dependents() const
{
	return dependent_set;
}

vector<action> const &
make_target::actions() const
{
	return action_vec;
}

action const *
make_target::compile_action() const
{
	action const *alp = (compile_line_index != -1) ?
		&action_vec[compile_line_index] : 0;
	return alp;
}

