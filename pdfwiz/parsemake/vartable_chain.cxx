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
// vartable_chain.cxx
// hierarchy of symbol tables presented with single interface
// 3.feb.97 kit transue

#include <cassert>

#include "vartable_chain.h"



vartable_chain::vartable_chain()
{
}

vartable_chain::~vartable_chain()
{
	list_type::iterator i = searchpath.begin();
	while (i != searchpath.end()) {
		delete *i;
		++i;
	}
}

bool
vartable_chain::insert(string const & key, macro_node const & value, bool force)
{
	assert(!force);		// I think forced insert here would be unusual
	bool ret(false);

	// write to the first component willing to accept this:
	list_type::iterator i = searchpath.begin();
	while (!ret && i != searchpath.end()) {
		ret = (*i)->insert(key, value, force);
		++i;
	}
	return ret;
}

void
vartable_chain::push_table(variabletable *table)
{
	searchpath.push_back(table);
}

void
vartable_chain::dump() const
{
	list_type::const_iterator i = searchpath.begin();
	while (i != searchpath.end()) {
		(*i)->dump();
		++i;
	}
}

void
vartable_chain::report_usage() const
{
}

macro_node const &
vartable_chain::lookup(string const &key) const
{
	macro_node const * ret = macro_node::null_macro;
	list_type::const_iterator i = searchpath.begin();
	while (ret == macro_node::null_macro && i != searchpath.end()) {
		ret = &((*i)->lookup(key));
		++i;
	}
	return *ret;
}



