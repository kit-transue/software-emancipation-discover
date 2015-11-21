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
// cached_vartable.h
// 5.feb.98 kit transue
// common implementation of table using map

#if !defined(_CACHED_VARTABLE_H_)
#define _CACHED_VARTABLE_H_

#pragma warning(disable:4786) // STL makes for long names that get truncated

#include <string>
#include <map>
#include <set>
#include <iostream>

#include "variabletable.h"

using namespace std;


class cached_vartable : public variabletable {
public:
	// construct/copy/destroy
	cached_vartable(bool is_writable = true);
	virtual ~cached_vartable();

		// modify
	virtual bool insert(string const &key, macro_node const &value, bool force);

	// access
	virtual void dump() const;
	virtual void report_usage() const;
	virtual macro_node const &lookup(string const & key) const;

private:
	typedef map<string, macro_node, less<string> > tabletype;
	tabletype table;
	typedef set<string, less<string> > stringset;

	// keep track of macros actually used
	mutable stringset refs;

	// flag to indicate if this should accept non-forced writes:
	bool is_writable;
};


#endif // !defined(_CACHED_VARTABLE_H_)
