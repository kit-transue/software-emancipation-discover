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
// cached_vartable.cxx
// 5.feb.98 kit transue
// common implementation of table using map

#include "cached_vartable.h"

#include <cassert>

using namespace std;


// construct/copy/destroy

cached_vartable::cached_vartable(bool writable) :
	is_writable(writable)
{
}

cached_vartable::~cached_vartable()
{
}



// modify

bool
cached_vartable::insert(string const & key, macro_node const &value, bool force)
{
	bool ret(false);
	if (is_writable || force) {
		ret = true;
		// strip leading/trailing whitespace
		string::size_type pos = 0;
		while (isspace(key[pos]))
			++pos;
		string::size_type end = key.length();
		while (end > 0 && isspace(key[end - 1]))
			--end;
	
		// and set value
		if (pos > end)
			pos = end;
		string strippedkey(key, pos, end - pos);
	
		// check to see if this key overwrites another key:
		tabletype::iterator i = table.find(strippedkey);
		if (i != table.end())  // must redefine this macro
			table.erase(i);
		table.insert(tabletype::value_type(strippedkey, value));
	}
	return ret;
}



// access
void
cached_vartable::dump() const
{
	tabletype::const_iterator I = table.begin();
	while (I != table.end()) {
		cout << "line " << I->second.line_number << ":" << I->first << " maps to: " << I->second.value << endl;	
		++I;
	}
}


macro_node const &
cached_vartable::lookup(string const &key) const
{
	macro_node const * ret = macro_node::null_macro;
	tabletype::const_iterator i = table.find(key);
	if (i != table.end()) {
		ret = &(i->second);
	}
	return *ret;
}


void
cached_vartable::report_usage() const
{
	stringset::const_iterator I = refs.begin();
	while (I != refs.end()) {
		cout << "variable " << *I << " referenced" << endl;
		++I;
	}
}

