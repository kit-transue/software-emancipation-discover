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
// prefsfile.h
// utility for writing to preferences file
// 11.dec.97 kit transue

#if !defined(_PREFSFILE_H_)
#define _PREFSFILE_H_

#include <string>
#include <map>
#include <vector>


using namespace std;

class prefsfile
{
// follows map semantics
public:
	// construct/copy/destroy
	prefsfile();
	prefsfile(string const &fname);

	// read
	string const *at(string const &) const;

	// modify
	void insert_comment(string const &comment);
	void insert(string const &key, string const &value);
	void write();
	void write(ostream * out);
	void set_name(string const& newname);

	void read(string const &filename);
private:
	typedef vector<string> linevec_type;  // constant insert at end
	typedef map<string, int, less<string> > index_type;  // pref->position mapping
	linevec_type linevec;
	linevec_type valuevec;
	index_type index;
	string filename;
	bool dirty;
	static char const PREFIX[]; // = "*psetPrefs."
	static char const KEYDELIMS[]; // = ":"
};

#endif // !defined(_PREFSFILE_H_)
