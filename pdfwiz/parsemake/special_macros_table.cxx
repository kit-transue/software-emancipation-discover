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
// special_macros_table.cxx
// provide symbol-table access for special make variables from current target

// 4.jun.1998 kit transue

#pragma warning(disable:4786)

#include <cassert>
#include <stack>
#include "filename.h"

#include "special_macros_table.h"

// construct/copy/destroy
special_macros_table::special_macros_table() : mtp(0), dependp(0)
{
}

special_macros_table::~special_macros_table()
{
}

// modify
bool 
special_macros_table::insert(string const &key, macro_node const &value, bool force)
{
	assert(0);
	// this is a meaningless operation, and should be caught by one of the
	// writable tables before seen by this.
	return false;
}

void
special_macros_table::set_target(make_target *target)
{
	mtp = target;
}

void
special_macros_table::set_depend(string const *depend)
{
	dependp = depend;
}

void
special_macros_table::clear_depend()
{
	dependp = 0;
}

// access
void
special_macros_table::dump() const
{
}

void
special_macros_table::report_usage() const
{
}

macro_node const &
special_macros_table::lookup(string const &key) const
{
	macro_node const * ret = macro_node::null_macro;	// not found
	// don't know how we're going to fulfill this reference reqm'nt, but...

	// decide what we're looking at:
	bool expanded = false;
	string value;
	if (key == "@" || key == "$@") {
		// @: current target full name
		// $@: current target full name (dependent in dependency)
		assert(mtp);
		expanded = true;
		value = mtp->name();
	}
	else if (key == "*") {
		// *: path and base w/o extension
		assert(mtp);
		expanded = true;
		value = mtp->name();
		string::size_type pos = value.rfind('.');
		if (pos != string::npos) {
			value.erase(pos, string::npos);
		}
	}
	else if (key == "**" || key == "?") {
		// **: all dependents of current target
		// ?: all dependents newer than target
		assert(mtp);
		expanded = true;
		// need to make a decision here: if makefile->current_depend is
		// set, then we're in a !action, and we use this instead of the
		// whole list:

		if (dependp) {
			// don't know if this wrapping is necessary...
			value += " \"";
			value += *dependp;
			value += '"';
		}
		else {  // list all together
			make_target::stringpset const &depends = mtp->dependents();
			make_target::stringpset::const_iterator i = depends.begin();
			while (i != depends.end()) {
				value += " \"";
				value += *i;
				value += '"';
				++i;
			}
		}
	}
	else if (key == "<") {
		// <: depenent newer than target (in inference rule)
	}

	// strip off trailing modifier (case sensitive with nmake, insensitive with omake)
		// D: Drive plus directory
		// B: Base name
		// F: Base name plus extension
		// R: Drive plus directory plus base name

	// create a new instantiation
	static stack<macro_node> instantiations;
	if (expanded) {
		static stack<macro_node> instantiations;
		instantiations.push(macro_node(value, 2));
		ret = &instantiations.top();
	}

	return *ret;
}
