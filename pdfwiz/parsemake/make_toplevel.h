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
// make_toplevel.h
// 16.Dec.97 Kit Transue

// summary information for makefile


#if !defined(_MAKE_TOPLEVEL_H)
#define _MAKE_TOPLEVEL_H

#include <string>
#include <map>

#include "sort_objects.h"
#include "make_target.h"
#include "dependpath.h"
#include "pathmgr.h"
#include "sort_objects.h"
#include "variabletable.h"
#include "action.h"
#include "filename.h"
#include "special_macros_table.h"
#include "env_table.h"
#include "commandline.h"


#include "pipebuf.h"


using namespace std;

class makefile
{
public:
	// class members
	static makefile * globalmakefile;

	// public types:
	typedef set<string, less<string> > stringset;

	// construct/copy/destroy
	makefile(commandline const &, directory_vector const &dv);
	virtual ~makefile();

	// not quite a constructor, but a big-time initializer
	void read_file();
	void read_file(istream * stream);

	// modifiers
	void add_target(string const &targetname, bool is_suffix = false);
	void add_dependent(string const &dependfile);
	void add_action(string const &action);
	void add_suffixrule(string const &rule);
	void add_includefile(string const &filename);
	// too tightly coupled to dependpath.h, but I'm pressed for time:
	void add_dependents(char const *);

	// accessor and reporting functions
	string const &name() const;
	void dump() const;
	stringset * list_all_depends(string const &target) const;
	stringset * list_target_depends(string const &target) const;
	stringset * list_all_targets() const;
	stringset const * list_default_targets() const;
	set<action> * list_makeactions(string const &target) const;
	string const & flags_db_version() const;
	void write_flags_database(ostream *os, string const *compiler = 0) const;
	pair<string,string> compile_line(string const &sourefile) const;
	string member_as_absolute(string const &target_or_depend) const;

	// public members (bad form here!)
	variabletable * symbols;

	// public data members for tracking parse progress:
	// should encapsulate these better?
	int progress_currentchar;
	int progress_totalchars;
private:
	// same as add_target, but doesn't split multiple targets into components
	make_target *add_atomic_target(string const &filename);
	// multiple targets may be specified on LHS of target/dependency line:
	set<make_target *> current_targets;
	set<string> default_targets;

	// lookup table for context-sensitive dependent/target special variables:
	special_macros_table *specialtable;
	// and for environment;
	env_table *envtable;

	// directory vector when make was invoked:
	directory_vector cwds;

	typedef map<string, make_target *, ntfname_str_less<char> > target_map_type;
	target_map_type target_set;  // target string->target
	string fname;
	set<string *, pless<string *> > envvar_refs;
	typedef map<string const *, make_target const *, pless<string const *> > rindex_type;
	rindex_type rindex;
	string newincludename;
};

#endif // _MAKE_TOPLEVEL_H
