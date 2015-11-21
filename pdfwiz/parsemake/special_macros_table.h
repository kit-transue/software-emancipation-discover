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
// special_macros_table.h
// provide symbol-table access for special make variables from current target

// 4.jun.1998 kit transue


#if !defined(_SPECIAL_MACRO_TABLE_H_)
#define _SPECIAL_MACRO_TABLE_H_

#include <string>
#include <set>
#include <map>

using namespace std;

#include "variabletable.h"
#include "make_target.h"

class special_macros_table : public variabletable
{
public:
	// construct/copy/destroy
	special_macros_table();
	virtual ~special_macros_table();

	// modify
	bool insert(string const &key, macro_node const &value, bool force);
	// specific to this class:
	void set_depend(string const *depend);
	void clear_depend();
	void set_target(make_target *);


	// access
	void dump() const;
	void report_usage() const;
	macro_node const &lookup(string const &key) const;
private:
	make_target const *mtp;  // current target to use
	string const *dependp;	// current depend, if expanding ! rule
	
};

#endif // !defined(_SPECIAL_MACRO_TABLE_H_)
