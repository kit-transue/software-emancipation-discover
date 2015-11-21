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
// vartable_chain.h
// present a hierarchy of variabltables as a single interface
// 3.feb.98 kit transue

#if !defined(_VARTABLE_CHAIN_H_)
#define _VARTABLE_CHAIN_H_

#include "variabletable.h"
#include "env_table.h"
#include <list>
using namespace std;

class vartable_chain : public variabletable
{
public:
	// construct/copy/destroy
	vartable_chain();
	virtual ~vartable_chain();

	// variabletable modify
	virtual bool insert(string const &key, macro_node const &value, bool force);

	// variabletable access
	virtual void dump() const;
	virtual void report_usage() const;
	virtual macro_node const &lookup(string const & key) const;

	// vartable_chain extensions:
	void push_table(variabletable *);

private:
	// list of tables to search in order
	typedef list<variabletable *> list_type;
	list_type searchpath;
};

#endif // !defined(_VARTABLE_CHAIN_H_)
