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
// prefsfile.cxx
// 11.dec.97 kit transue
// read/write preferences file [hierarchy] using map semantics

#pragma warning(disable:4786)

#include <fstream>
#include "message.h"

#include "prefsfile.h"

using namespace std;

char const prefsfile::PREFIX[] = "*psetPrefs.";
char const prefsfile::KEYDELIMS[] = ":";


prefsfile::prefsfile() : dirty(false)
{
}

prefsfile::prefsfile(string const &fname) : filename(fname), dirty(false)
{
	read(filename);
}

void
prefsfile::read(string const &fname)
{
	// open file
	ifstream input(fname.c_str(), ios_base::in);
	if (!input.good()) {
		msgid("Error opening preferences file") << fname << eom;
	} else {
		// read all lines
		int line_number = linevec.size();
		string s;
		while (input.good()) {
			getline(input, s);
			// wrap escaped lines if necessary
			int length;
			while ( (length = s.length()) > 1
				&& s[length - 2] == '\\'
				&& input.good())
			{
				s += '\n';
				string buffer;
				getline(input, buffer);
				s += buffer;
			}
	
	
			string value;
			// add to index if appropriate
			int const PREFIXLEN = sizeof(PREFIX) / sizeof(PREFIX[0]) - 1;
			if (!s.compare(0, PREFIXLEN, PREFIX)) {
				int i = s.find_first_of(KEYDELIMS);
				if (i != string::npos) {
					string key (s, PREFIXLEN, i - PREFIXLEN);
					index.insert(index_type::value_type(key, line_number));
					if (i < s.length())
						value = s.substr(i + 1);
				}
				else {
					// badly formed preference line
				}
			}
	
			// add to storage--preserves original order and blank lines
			linevec.push_back(s);
			valuevec.push_back(value);
			// increment counter
			++line_number;
			//assert(lineno == linevec.end() - linevec.begin());
		}
	
		// close
		input.close();
	}
}


string const *
prefsfile::at(string const &key) const
{
	string const *ret = NULL;

	index_type::const_iterator i = index.find(key);
	if (i != index.end()) {
		ret = &valuevec[ i->second ];
	}
	return ret;
}

void
prefsfile::write()
{
	// may be better to write to a tempfile and rename
	if (dirty) {
		ofstream outstream (filename.c_str(), ios_base::out);
		if (!outstream.good()) {	
			msgid("Error opening prefsfile for write") << filename << eom;
		}
		else {
			write(&outstream);
			if (!outstream.good()) {	
				msgid("Error writing prefsfile") << filename << eom;
			}
			else dirty = false;
			outstream.close();
		}
	}
}

void
prefsfile::write(ostream * out)
{
	linevec_type::iterator i;
	for (i = linevec.begin(); i != linevec.end(); ++i)
		*out << *i << endl;
}

void
prefsfile::insert_comment(string const &comment)
{
	dirty = true;
	linevec.push_back(comment);
	valuevec.push_back("");
}

void
prefsfile::insert(string const &key, string const &value)
{
	dirty = true;
	
	// construct new value line:
	string line(PREFIX);
	line += key;
	line += KEYDELIMS[0];
	line += '\t';
	line += value;

	// see if this key already exists:
	index_type::iterator i = index.find(key);
	if (i != index.end()) {
		linevec[ i->second ] = line;
		valuevec[ i->second ] = value;
	}
	else {
		// must add to the vector;
		linevec.push_back(line);
		valuevec.push_back(value);
		index.insert(index_type::value_type(key, linevec.size()));
	}
}

void
prefsfile::set_name(string const &s)
{
	filename = s;
}
