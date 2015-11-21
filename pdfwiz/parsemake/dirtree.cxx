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
// dirtree.cxx
// 31.may.1998 kit transue

#include "dirtree.h"
#include "filename.h"
#include <cassert>
#include <iostream>
#include <io.h> // for findfirst()


// construct/copy/delete:

dirnode::dirnode(string const &name) : thisname(name)
{
	// file, if not empty/toplevel, is absolute:
	assert(thisname.empty() ? 1 :
		thisname.at(thisname.length() - 1) == filename::SEPARATOR);
	subdirs = new maptype;
}

dirnode::~dirnode()
{
	// the following causes FMR: hopefully compiler bug?
	//delete subdirs;
}


// access:

string const &
dirnode::name() const
{
	return thisname;
}


set<string>
dirnode::missing_files(string const &extension) const
{
	// not the most efficient, but what the heck!
	set<string> ret;

	// construct wildcard to use while looking:
	string wildcard(name());
	wildcard += "*";
	wildcard += extension;

	// check each file in turn, and see if it's here:
	struct _finddata_t found_file;
	long findhandle;
	findhandle = _findfirst(wildcard.c_str(), &found_file);
	bool found(findhandle != -1);
	while (found) {
		// see if this is a member:
		if (files.find(found_file.name) == files.end())
			ret.insert(found_file.name);
		found = !_findnext(findhandle, &found_file);
	}
	if (findhandle != -1)
		_findclose(findhandle);
	return ret;
} 


// modify:

#if 0	// may not be needed....
void
dirnode::addDir(string const & dirname)
{
	string::size_type start = thisname.length();
	string::size_type pos = dirname.find(filename::SEPARATOR, start);
	string key(dirname, start, pos - start);

	maptype::iterator i = subdirs->find(key);
	if (i == subdirs->end()) {
		i = subdirs->insert(maptype::value_type(key,
			dirnode(thisname + key))).first;
	}	
	i->second.addDir(dirname);
}
#endif


void
dirnode::addFile(string const & filename)
{
	// make sure this filename lives under this directory:
	// can't do: assert(filename.find(name()) == 0);
	// because it's not case-insenstive:
	assert(!ntfname_traits<char>::compare(name().c_str(), filename.c_str(), name().length()));

	// see if there's a subdirectory here we should be using...
	string::size_type start = thisname.length();
	string::size_type pos = filename.find(filename::SEPARATOR, start);
	if (pos != string::npos) {
		// pass on to our child directory:
		string key(filename, start, pos - start + 1);
		maptype::iterator i = subdirs->find(key);
		if (i == subdirs->end()) {
			i = subdirs->insert(maptype::value_type(key,
				dirnode(thisname + key))).first;
		}	
		i->second.addFile(filename);
	}
	else {
		// remove path part
		files.insert(string(filename, start, string::npos));
	}
}

