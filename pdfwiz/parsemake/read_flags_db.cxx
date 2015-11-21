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
// read_flags_db.cxx
// 2.mar.98 kit transue

#include "filename.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
using namespace std;

void usage()
{
	cout << "Usage: read_flags_db flagsdatabase filename" << endl;
}

int main(int argc, char *argv[])
{
	int errstatus = 0;
	int format_version = 0;
	if (argc != 3) {
		cout << "read_flags_db expecting 2 arguments; got " << argc - 1 << endl;
		usage();
		return 2;
	}

	ifstream db(argv[1], ios::in);
	if (!db.good()) {
		cerr << "Could not open file " << argv[1] << endl;
		return 3;
	}

	string target(argv[2]);
	target += ':';

	string fn;
	bool found = false;
	while (!db.eof() && !found) {
		db >> ntfname_extractor(fn);
		if (fn == "#v2") {
			format_version = 2;
		}
		if ( !ntfname_str_less<char>()(fn, target)
			&& !ntfname_str_less<char>()(target, fn) ) {
			found = true;
			if (format_version > 1) {
				string working_directory;
				db >> ntfname_extractor(working_directory);
			}
			string makeline;
			db >> ws;
			getline(db, makeline);
			cout << makeline;
		}
		else
			db.ignore(numeric_limits<int>::max(), '\n');
	}
	return (found) ? 0 : 1;
}
