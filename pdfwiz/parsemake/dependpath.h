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
// dependpath.h
// 17.Dec.97 Kit Transue
// dependency path parser

#if !defined(_DEPENDPATH_H)
#define _DEPENDPATH_H

#pragma warning(disable:4786) // STL makes for long names that get truncated

#include <string>
#include <set>
#include <iostream>

#include "sort_objects.h"
#include "pathmgr.h"

using namespace std;


class dependpath {
public:
	// public types/class globals:
	typedef set<string> stringset;

	// access
	static stringset * split_into_paths(char const *, directory_vector const &);
		// input: string should already be expanded
		// output: must delete the set returned from this call.
	static string mySearchPath(string const &pathlist, string const &filename, directory_vector const &cwd);

private:
	// historically called by the lexer, but now private
	static void insert(stringset * theset, string const &dependfile, string const &dependpath, directory_vector const &);
	static void insert(stringset * theset, string const &dependfile, directory_vector const &);
};


#endif // _DEPENDPATH_H
