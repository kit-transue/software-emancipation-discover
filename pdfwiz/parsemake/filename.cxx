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
// filename.cxx
// 15.Dec.97 Kit Transue

#include <stdlib.h> // for splitpath

#include <string>
#include <iostream>
#include <cassert>
#include <sstream>

using namespace std;
#include "filename.h"


// major overlap with pathmgr.cxx
// RECONCILE THIS!


char const filename::SEPARATOR = '\\';


// construct/copy/destroy
filename::filename(string const &name) : data(name)
{
	// make sure directories aren't built with trailing slash:
	string::size_type len = data.length();
	if (len > 1 && (data[len - 1] == '/' || data[len - 1] == '\\') && !is_root(data)) {
		data.erase(len - 1);
	}
}


string const &
filename::name() const
{
	return data;
}

string
filename::dirname() const
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath(data.c_str(), drive, dir, NULL, NULL);

	string ret(drive);
	ret += dir;
	return ret;
}

string
filename::basename() const
{
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(data.c_str(), NULL, NULL, fname, ext);

	string ret(fname);
	ret += ext;
	return ret;
}

string
filename::drivename(string const &data)
{
	string ret;
	if (is_UNC_name(data)) {
		// need to get both machine and drive:
		size_t pos = data.find_first_of('\\', 2);
		if (pos != string::npos) {
			pos = data.find_first_of('\\', pos + 1);
		}
		// pos != npos -> valid UNC name (with both components)
		if (pos != string::npos) {
			ret = data.substr(0, pos);
		}
	}
	else if (data.length() > 1 && data[1] == ':') {
		ret = data.substr(0, 2);
	}
	else {
		// data does not contain a valid drivename;
		assert(ret.empty());
	}
	return ret;
}


string
filename::common_root(string const &name1, string const &name2)
{
	string::const_iterator i = name1.begin();
	string::const_iterator j = name2.begin();
	int last_component = 0;

	for (int n = 1;
			i != name1.end()
			&& j != name2.end()
			&& *i == *j;
		++n)
	{
		if (*i == filename::SEPARATOR)
			last_component = n;
		++i;
		++j;
	}
	return name1.substr(0, last_component);
}



bool
filename::is_absolute_path(string const &path)
{
	bool ret = false;
	// UNC paths are always absolute:
	if (is_UNC_name(path)) {
		ret = true;
	}
	else {
		if (path.length() > 2
		    && path[1] == ':'
		    && (path[2] == '\\'
		    || path[2] == '/'))
			ret = true;
	}
	return ret;
}

bool
filename::is_root(string const &path)
{
	bool ret = false;
	// if it's not an absolute path, don't bother with the work!
	if (is_absolute_path(path)) {
		assert(path.length() > 2);	// unless something's changed about absolute paths!
		string::size_type i = path.find_first_of("/\\", 2);
		if (i == string::npos || i == path.length() - 1)
			ret = true;
	}
	return ret;
}

bool
filename::is_UNC_name(string const &path)
{
	bool ret = false;
	if (path.length() > 2 && path[0] == '\\' && path[1] == '\\')
		ret = true;
	return ret;
}


bool
filename::is_command(string const &line, string const &commandlist)
{
	// commandlist should already be macro-expanded

	stringstream ss(line);
	// find basename for line
	string argOne;
	ss >> ntfname_extractor(argOne);

	string command = filename(argOne).basename();

	string component;
	ss.str(commandlist);
	ss.clear();
	while (ss.good()) {
		ss >> ws >> ntfname_extractor(component);
		if (command == component)
			return true;
	}
	return false;
}
