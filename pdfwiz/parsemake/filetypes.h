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
// filetypes.h
// 4.Dec.97 Kit Transue
// Support for deciding file type

#if !defined(_FILETYPES_H_)
#define _FILETYPES_H_

#pragma warning(disable:4786)
#include <string>
#include <map>

#include "filename.h"

using namespace std;


class filetype
{
public:
	string extension;
	string description;
	enum ekind {	CSOURCE,
		CPPSOURCE,
		HEADER,
		MAKEFILE,
		EXECUTABLE,
		OBJECT,
		USER_TYPE,
		UNIMPORTANT,
		UNKNOWN
	};
	bool include_in_pdf() const;
	static filetype const * const Unknown;
	ekind kind;
	filetype(string const &ext, string const &desc, ekind k);
	filetype();
};

	

class filetype_context
{
	// note: I suggest creating instances of this class, so if in
	// different contexts an extension may mean different things,
	// we can support this.
public:
	filetype_context();
	char const * const ext_delims;
	filetype const &what_type(string const &filename);
	string extension(string const &filename);
	void add_type(filetype const &x);
private:
	//struct ftinit {char * ext, char * desc, filetype::ekind kind};
	typedef map<string, filetype, ntfname_str_less<char> > tabletype;
	tabletype ext_table;
};

extern filetype_context global_filetypes;
#endif // !defined(_FILETYPES_H_)
