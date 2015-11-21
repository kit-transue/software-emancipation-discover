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
// file_table.cxx
// provide symbol-table access and tracking for variables defined from a file
// containing lines of KEY=VALUE pairs, much like the output from "set".
// Typical use is for providing built-in macro definitions

// 5.feb.1998 kit transue

#pragma warning(disable:4786)
#include <fstream>

#include "message.h"
#include "file_table.h"

// construct/copy/destroy

file_table::file_table(string const & filename)
{
	ifstream input(filename.c_str(), ios_base::in);
	if (!input.good()) {
		msgid("Error opening file") << filename << eom;
	}
	string line;
	while (getline(input, line)) {
		// strip whitespace...
		string::size_type first = 0;
		for (; isspace(line[first]); ++first);
		// ...and check for comment
		bool comment = (line[first] == '#');

		string::size_type pos = line.find_first_of('=');
		if (!comment && pos != string::npos) {
			string key = line.substr(0,pos);
			string value = line.substr(pos + 1);
			insert(key, macro_node(value, -1), true);
		}
	}
	input.close();
}


file_table::~file_table()
{
}
