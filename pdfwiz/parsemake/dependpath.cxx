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
// dependpath.cxx
// 17.Dec.97 Kit Transue
// dependency path parser

#include <windows.h>
#pragma warning(disable:4786) // STL makes for long variable names
#pragma warning(disable:4503) // STL makes for long variable names

#include "dependpath.h"
#include "pathmgr.h"
#include "pipebuf.h"
#include "variabletable.h"
#include "macro_expander.h"
#include "make_toplevel.h"

#include "message.h"

#include <cassert>
#include <cstring>
#include <io.h>


using namespace std;


string
dependpath::mySearchPath(string const &pathlist, string const &filename, directory_vector const &cwd)
{
	string ret;
	assert(!filename.empty());
	struct _finddata_t fileinfo;

	// tokenize path according to semicolons:
	string::size_type begin;
	string::size_type end;
	bool found(false);
	for (begin = 0; begin != string::npos && !found; begin = end) {
		end = pathlist.find(';', begin);
		ret = pathlist.substr(begin, end - begin);
		if (end != string::npos)	// we're looking at the ;
			++end;
		// add a delimiter if none available:
		if (filename[0] != '/' && filename[0] != '\\' &&
				ret[ret.length() - 1] != '/' &&
				ret[ret.length() - 1] != '\\' ) {
			ret += '/';
		}
		ret += filename;
		string const &abspath = cwd.relative_to_absolute(ret);
		long findhandle = _findfirst(abspath.c_str(), &fileinfo);
		if (findhandle != -1) {
			found = true;
			_findclose(findhandle);
		}
	}

	if (!found) {
		ret = filename;
		msgid("SearchPath didn't find file in path") << filename << eoarg << pathlist << eom;
	}
	return ret;
}


void
dependpath::insert(stringset * theset, string const &dependfile, string const &dependpath, directory_vector const &dv)
{
	if (dependpath.empty())
		insert(theset, dependfile, dv);
	else {	// dependpath specified
		// I think VC5 requires scope resolution here is a bug:
		insert(theset, dependpath::mySearchPath(dependpath, dependfile, dv), dv);
	}
}


dependpath::stringset *
dependpath::split_into_paths(char const * s, directory_vector const &dv)
{
	// parses a number of whitespace-delimited files of the form
	// {path}/filename or filename.  Finds in {path} and expands.

	stringset *theset = new stringset;
	
	string pathpart;
	string dependfile;
	while (*s) {
		if (isspace(*s)) {
			insert(theset, dependfile, pathpart, dv);
			dependfile.erase();
			pathpart.erase();
		}
		else {
			switch (*s) {
				case '"': {
					++s; // consume quote
					char const *end = strchr(s, '"');
					if (end == 0) {
						dependfile.append(s, strlen(s));
						s += strlen(s);
					}
					else {
						dependfile.append(s, end - s);
						s = end;
					}
					break;
				}
				case '{': {
					++s;  // consume brace
					char const *end = strchr(s, '}');
					if (end == 0) {
						msgid("Unmatched brace parsing path") << eom;
						pathpart.append(s, strlen(s));
						s += strlen(s);
					}
					else {
						pathpart.append(s, end - s);
						s = end;
					}
					break;
				}
				default:
					dependfile += *s;
					break;
			}
		}
		++s;
	}
	return theset;
}

void
dependpath::insert(stringset * theset, string const &path, directory_vector const &dv)
{
	if (!path.empty()) {
#if 0	// should maintain relativity
		string const & absvalue = dv.relative_to_absolute(path);
#else
		string const & absvalue = path;
#endif
		// confirm this isn't a duplicate:
		stringset::iterator i = theset->find(absvalue);
		if (i == theset->end())
			theset->insert(absvalue);
	}
}
