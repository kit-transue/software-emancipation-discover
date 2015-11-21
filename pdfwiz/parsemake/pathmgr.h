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
// pathmgr.h
// 3.Dec.97 Kit Transue

#if !defined(_PATHMGR_H_)
#define _PATHMGR_H_

#include <map>
#include <string>

#include "filename.h"

using namespace std;

class directory_vector
{
public:
	// construct/copy/destroy
	directory_vector();
	virtual ~directory_vector();

	// comparison
	bool operator< (directory_vector const &) const;

	// modify
	void cd (string const &path);	// changes both drive and cwd

	// access
	string relative_to_absolute(string const &path) const;
	string const &directory(string const &drive) const;
	string const &directory() const;	// cwd for default drive
	string const &drive() const;	// returns drive letter w/ colon or UNC base

private:
	// modify
	void set_directory(string const &drive, string const &path);
	void set_drive(string const &drive);

	string defaultdrive;
	string lettered_drive_name[26];

	typedef map<string, string, ntfname_str_less<char> > dir_vec_type;
	dir_vec_type dir_vec;
};

#endif // !defined(_PATHMGR_H_)
