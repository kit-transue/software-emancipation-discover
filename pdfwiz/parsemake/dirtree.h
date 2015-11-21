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
// dirtree.h
// 31.may.98 kit transue
// create a hierarchical index to a set of delimited entries

// if this assumes directory structure, then perhaps support other direcotry
// stuff as well


#if !defined(_DIRTREE_H_)
#define _DIRTREE_H_

#pragma warning(disable:4786)

#include <set>
#include <string>
#include <map>
#include "filename.h"

using namespace std;


class dirnode
{
public:
	// construct/copy/destroy
	dirnode(string const &);
	virtual ~dirnode();

	// access
	string const &name() const;
	set<string> missing_files(string const &extension) const;
		// returns files on disk with this extension that aren't in this node

	// modify
	//void addDir(string const &);
	void addFile(string const &);

//private:
	// available types
	typedef map<string, dirnode, ntfname_str_less<char> > maptype;
		// key to map is relative to root, to avoid redundancy
	typedef set<string, ntfname_str_less<char> > files_type;
	maptype *subdirs;
	string thisname;
	files_type files;
};




#endif //!defined(_DIRTREE_H_)
