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
// pathmgr.cxx
// NT path handling functions
// 3.Dec.97 Kit Transue

#pragma warning(disable:4786)

#include <cassert>
#include <string>
#include <vector>
#include <direct.h>

#include "pathmgr.h"
#include "filename.h"


// construct/copy/destroy

directory_vector::directory_vector()
{
	// initialize CWD for all drives based on current default
	char buffer[_MAX_PATH];
	for (int ndrive = 0; ndrive < 26; ++ndrive) {
		// start by creating a name for this thing:
		string &drive_name = lettered_drive_name[ndrive];
		drive_name = 'A' + ndrive;
		drive_name += ':';
		// test for ndrive skips floppy drives, avoiding time and funny noise:
		if(ndrive > 1 && _getdcwd(ndrive + 1, buffer, sizeof(buffer) / sizeof(buffer[0]))) {
			// _getdcwd will not append a trailing slash/backslash
			int n = strlen(buffer);
			assert (n > 0);
			if (buffer[n - 1] != '\\') {
				buffer[n] = '\\';
				buffer[n + 1] = 0;
			}
			dir_vec[drive_name] = buffer;
		}
		else {
			dir_vec[drive_name] = drive_name;
			dir_vec[drive_name] += '\\';
		}
	}
	defaultdrive = lettered_drive_name[_getdrive() - 1];
}


directory_vector::~directory_vector()
{
}


// compare

bool
directory_vector::operator< (directory_vector const &x) const
{
	bool ret = false;
	if (defaultdrive != x.defaultdrive) {
		ret = defaultdrive < x.defaultdrive;
	}
	else {
		dir_vec_type::const_iterator lhs = dir_vec.find(defaultdrive);
		dir_vec_type::const_iterator rhs = x.dir_vec.find(defaultdrive);
		assert(lhs != dir_vec.end());
		assert(rhs != x.dir_vec.end());
		ret = lhs->second < rhs->second;
	}
	return ret;
}

// modify

void
directory_vector::set_directory(string const &drive, string const &path)
{
	// change working directory on a drive without changing current drive
	dir_vec[drive] = path;
}


void
directory_vector::set_drive(string const &drive)
{
	defaultdrive = drive;
}


void
directory_vector::cd (string const &path)
{
	// start by cannonizing this path relative to where we are now:
	string abspath = relative_to_absolute(path);
	// satisfy the set_directory gods:
	if (abspath[abspath.length() - 1] != '\\')
		abspath += '\\';

	// now go about making the change real:
	set_drive(filename::drivename(abspath));
	set_directory(filename::drivename(abspath), abspath);
}


// access

string const &
directory_vector::directory(string const &drive) const
{
	static string empty_string;
	string const *ret = &empty_string;
	dir_vec_type::const_iterator i = dir_vec.find(drive);
	if (i != dir_vec.end()) {
		ret = &(i->second);
	}
	return *ret;
}

string const &
directory_vector::directory() const
{
	return directory(defaultdrive);
}

string
directory_vector::relative_to_absolute(string const &path) const
{
	// very similar to _fullpath(), but doesn't depend on state of the
	// system--only the state as captured in the directory_vector.

	// there are a couple of cases to consider:
	// no drive is specified, but path is otherwise absolute (/foo)
	// no drive is specified and path is relative (foo)
	// a drive is specified, but path is relative to drive's cwd (e:foo)
	// path is uniquely specified but is not cannonical

	string ret(path);
	// make sure non-UNC names are absolute:
	if (!filename::is_UNC_name(ret)) {
		string drive_component = filename::drivename(ret);
		
		// if the default drive is missing:
		if (drive_component.empty()) {
			if (ret[0] != '\\') {
				// need both drive and working directory:
				dir_vec_type::const_iterator d = dir_vec.find(defaultdrive);
				assert(d != dir_vec.end());
				ret.insert(0, d->second);
			}
			else {
				// only add the drive
				ret.insert(0, defaultdrive);
			}
		}
		else {
			// add the drive's working directory if the root is relative
			if (!filename::is_absolute_path(ret)) {
				assert(ret[1] == ':');  //required by replace, below
				// since drive is specified, must be old-style w/o path
				string const &cwd = directory(drive_component);
				// use replace since table includes drive letter
				ret.replace(0, 2, cwd);
			}
		}
	}

	// now that nothing is left to chance, we cannonize with _fullpath:
	char buffer[_MAX_PATH];
	if (_fullpath(buffer, ret.c_str(), sizeof(buffer) / sizeof(buffer[0])))
		ret = buffer;
#if 0 // 990824.  UNC support has introduced case-insensitivity to drivenames
	// and just to make sure the drivename is uppercase:
	ret[0] = toupper(ret[0]);
#endif
	return ret;
}

string const &
directory_vector::drive() const
{
	return defaultdrive;
}

