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
#include <stdio.h>
#include "machdep.h"
#ifdef _WIN32
#include <io.h>
#include "registry_if.h"
#endif
#ifdef _WIN32
#pragma warning (disable:4786)
#endif
#ifdef ISO_CPP_HEADERS
#include <sstream>
#include <fstream>
#include <iostream>
#else
#include <strstream.h>
#include <fstream.h>
#include <iostream.h>
#endif
#include <string>
#include <list>
#include <ctype.h>

namespace std {}
using namespace std;

static char search_sep =
#ifdef _WIN32
		       ';'
#else
		       ':'
#endif
			  ;

static char path_sep =
#ifdef _WIN32
		       '\\'
#else
		       '/'
#endif
			  ;

// Returns true if dir exists.
static bool dir_exists(const char *dir)
{
    bool exists = false;
    if (dir != NULL) {
#ifdef _WIN32
	exists = _access(dir, 00) != -1; // had to include <io.h>
#else
	exists = access(dir, F_OK) != -1;
#endif
    }
    return exists;
}

// Extend the given directory path with the given file name.
// Insert path_sep as necessary.
string pappend(string dir, string fname) {
    if (dir.length() > 0 && dir[dir.length() - 1] != path_sep
                         && dir[dir.length() - 1] != '/') {
	dir += path_sep;
    }
    return dir += fname;
}

// Remove trailing separator from dir if possible.
string unsep(string dir) {
    string::size_type len = dir.length();
    string::size_type last = len - 1;
    if (len == 0 || !(dir[last] == path_sep || dir[last] == '/')) {
	// dir is OK.
    }
    else if (len == 1) {
	// dir is OK, 1-char path_sep.
    }
#ifdef _WIN32
    else if (len == 3 && dir[1] == ':') {
	// dir is OK, "C:\"
    }
#endif
    else {
	dir.erase(last, 1);
    }
    return dir;
}

// In the following functions 0 indicates success, otherwise failure.

// Determine PSETHOME (installation dir for DIScover).
// Finds model_server executable in path and deduces install dir from there.
// This "easy" way of determining install dir can be overridden by explicitly
// setting the PSETHOME env variable.

int find_psethome(string &dir) {

		string serv_file =
#ifdef _WIN32
					"model_server.exe"
#else
					"model_server"
#endif
							;
    char *psethome = OSapi_getenv("PSETHOME");
    if(psethome) {
		if(dir_exists(psethome)) {
			string serv_path = pappend(pappend(psethome, "bin"), serv_file);
			if (OSapi_access(serv_path.c_str(), F_OK) != 0)
				psethome = NULL;
  		} else
			psethome = NULL;
    }
#ifdef _WIN32
    if (psethome == NULL) {
	string home;
	string key75 = "HKEY_LOCAL_MACHINE\\SOFTWARE\\THECOMPANY\\DIScover\\7.5\\PSETHOME";
	// Look up key in registry.
	if (registry::exists(key75)) {
	    home = registry::at(key75);
	}
	if (home.length() > 0) {
	    psethome = new char[home.length() + 1];
	    strcpy(psethome, home.c_str());
		string szpsethome = "PSETHOME=";
		szpsethome += psethome;
		OSapi_putenv(szpsethome.c_str());
	}
    }
#endif
    if (psethome == NULL) {
	char *search_path = OSapi_getenv("PATH");
	if (search_path != NULL) {
	    string search_str = search_path;
    	    size_t search_path_len = search_str.length();
    	    size_t i;
	    list<string> paths;
	    // Get a string for each directory in the path.
	    i = 0;
	    for (;;) {
		// i is the next place to look for a (possibly empty) string
#ifdef ISO_CPP_HEADERS
		ostringstream pstrm;
#else
		ostrstream pstrm;
#endif
		while (i < search_path_len && search_str[i] != search_sep) {
		    pstrm << search_str[i];
		    i += 1;
		}
#ifndef ISO_CPP_HEADERS
		pstrm << ends;
#endif
		string path = pstrm.str();
		// An empty string is equivalent to ".".
		if (path.length() == 0) {
		    path = ".";
		}

		// We'll only consider paths which end in /bin.
		size_t len = path.length();
		string path3;
		size_t bin_end = (len > 0 && (path[len - 1] == path_sep || path[len - 1] == '/')) ? len - 1 : len;
		if (bin_end >= 4 && (path[bin_end - 4] == path_sep || path[bin_end - 4] == '/')) {
		    path3 = path.substr(bin_end - 3, 3);
		}
		if (path3 == "bin" || path3 == "BIN") {
		    paths.push_back(path.substr(0, bin_end - 3));
		}

		if (i == search_path_len) {
		    break;
		}
		i += 1;		// Skip past the ':'.
	    }
	    list<string>::iterator iter;
	    for (iter = paths.begin(); iter != paths.end(); ++iter) {
		string serv_path = pappend(pappend(*iter, "bin"), serv_file);
		if (OSapi_access(serv_path.c_str(), F_OK) == 0) {
		    // exists, accessible
		    dir = unsep(*iter);
		    static string szpsethome = "PSETHOME=";
		    szpsethome += dir;
		    OSapi_putenv(szpsethome.c_str());
		    return 0;
		}
	    }
	}
    }
    if (psethome == NULL) {
#ifdef _WIN32
	cerr << "DIScover is not installed in %PATH%." << endl;
#else
	cerr << "DIScover is not installed in $PATH." << endl;
#endif
	return 1;
    }
    if (!dir_exists(psethome)) {
#ifdef _WIN32
	cerr << "%PSETHOME% is not a directory." << endl;
#else
	cerr << "$PSETHOME is not a directory." << endl;
#endif
	return 1;
    }
    dir = psethome;
    return 0;
}

// Value returned has leading and trailing whitespace removed.
string trimmed(const string &s) {
    int len = s.length();
    int leading_white = 0;
    int trailing_white = 0;
    for (int i = 0; i < len; i += 1) {
	if (isspace(s[i])) {
	    if (leading_white == i) {
		leading_white += 1;
	    }
	}
	else {
	    trailing_white = i + 1;
	}
    }
    // Leading_white is the number of leading whitespace bytes.
    // Trailing_white is the offset where trailing whitespace begins.
    // If the string is all whitespace, then the leading and trailing overlap.
    if (trailing_white < leading_white) {
	return "";
    }
    else {
	return s.substr(leading_white, trailing_white - leading_white);
    }
}

// Open the given file and search for the definition of the name "var".

int lookup_config(string fname, string var, string &val) {
    ifstream f;
    f.open(fname.c_str());
    if (!f.good()) {
	return 1;
    }
    bool in_key = 1;
    string key;
    string value;
    for (;;) {
	char ch;
	f.get(ch);
	if (!(f.eof() || f.good())) {
	    break;
	}
	if (f.eof() || ch == '\x0d' || ch == '\x0a') {
	    if (in_key) {
		// Reached end of line with no value.
	    }
	    else {
		// Reached end of line with value.
		if (var == trimmed(key)) {
		    val = trimmed(value);
		    f.close();
		    return 0;
		}
	    }
	    key = "";
        }
	if (f.eof()) {
	    break;
	}
	if (in_key && ch != '=') {
	    key += ch;
	}
	else if (in_key) {
	    in_key = false;
	    value = "";
	}
	else {
	    value += ch;
	}
    }
    f.close();
    return 1;
}

// Determine DISIM (repository dir for DIScover).
// Normally this is stored in $HOME/.codeintegrity (%USERPROFILE%\.codeintegrity on Windows).
// It can be overridden using the environment variable DISIM.

int find_disim(string &dir) {
    char *disim = OSapi_getenv("DISIM");

    if (disim == NULL) {
	const char *home_var =
#ifdef _WIN32
			 "USERPROFILE"
#else
			 "HOME"
#endif
			 ;
	char *home = OSapi_getenv(home_var);
	const char *dot_codeintegrity = ".codeintegrity";
	if (home != NULL) {
	    string dot_codeintegrity_path = pappend(home, dot_codeintegrity);
	    string found_disim;
	    if (lookup_config(dot_codeintegrity_path, "DISIM", found_disim) == 0) {
		if (dir_exists(found_disim.c_str())) {
		    dir = found_disim;
		    return 0;
		}
	    }
	}
	cerr << "No default DIScover repository is known for the current user ("
#ifdef _WIN32
	     << "%" << home_var << "%"
#else
	     << "$" << home_var
#endif
	     << "/" << dot_codeintegrity << ")." << endl;
	return 1;
    }
    else if (!dir_exists(disim)) {
#ifdef _WIN32
	cerr << "%DISIM% is not a directory." << endl;
#else
	cerr << "$DISIM is not a directory." << endl;
#endif
	return 1;
    }
    else {
        dir = disim;
        return 0;
    }
}


