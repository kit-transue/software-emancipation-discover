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

#ifdef _WIN32
#pragma warning (disable:4786)
#endif

#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#include <fstream>
#else
#include <iostream.h>
#include <strstream.h>
#include <fstream.h>
#endif
#include <string>
#include <vector>

#include "clconvert.h"
#include "msg.h"
#include "startprocess.h"
#include "configpar.h"

namespace std {};
using namespace std;
using namespace MBDriver;

// ===========================================================================
// Translate a command line from the native compiler's version to the 
// aset_CCcc version.  Returns either the translated command line or a
// diagnostic.
//
static bool massage_line(string linestr, CommandLineInterpreter &interp, string &result) {
    list<string> line;
    list<string> result_list;
    determine_command_arguments(linestr, line);
    CommandLineConverter converter(interp, line, result_list);
    string diagnostic;
    bool success = converter.convert(diagnostic);

    if (success) {
	// Convert the list of strings into a single string.
        // On Windows we have the special requirement that
        // %PSETHOME%\include\dis_nt.h be converted with enclosing quotes.
        // This is because the downstream expansion of environment vars which
	// takes place in dismb_lib::expand_env_vars is a little lame.
	result = prepare_command_arguments(result_list);
    }
    else {
	result = diagnostic;
    }
    return success;
}

// ===========================================================================
// Read lines from standard input (cin), and for each line, translate it from
// the native compiler's version to the aset_CCcc version, based on the
// given configuration file.  The resulting lines go to standard output.
//
static bool process_by_file(const char *configuration,
                            vector<string> &envargs,
			    bool verbose,
			    msg_sev unrec_arg) {
    if (verbose) {
	msg("Translating command lines based on $1, with $2 arguments.",
	    normal_sev)
	    << configuration << eoarg
	    << envargs.size() << eoarg << eom;
    }

    // Construct an interpreter object based on the configuration file.
    CommandLineInterpreter interp;
    string *diagnostic = NULL;
    if (!interp.configure(configuration, envargs, diagnostic, unrec_arg)) {
	if (diagnostic != NULL) {
	    msg("Parser flags configuration error: $1", catastrophe_sev)
		<< *diagnostic << eoarg << eom;
	}
	if (verbose) {
	    msg("Parser flags configuration failed.", normal_sev) << eom;
	}
	return false;
    }

    // Convert the lines.
    istream &f = cin;
    bool ok = true;
    int count = 0;
    if (ok) {
	for (;;) {
#ifdef ISO_CPP_HEADERS
	    stringstream line;
#else
	    ostrstream line;
#endif
	    f.get(*line.rdbuf(), '\n');
#ifdef ISO_CPP_HEADERS
	    if (f.eof() && line.str().length() == 0) {  
#else
	    if (f.eof() && line.pcount() == 0) {  
#endif
		if (verbose) {
		    msg("Completed $1 input lines.", normal_sev)
			<< count << eoarg << eom;
		}
		break;
	    }
	    if (f.bad() || f.eof()) {
		if (verbose) {
		    msg("Input error after $1 successful input lines.", normal_sev)
			<< count << eoarg << eom;
		}
		ok = false;
		break;
	    }
	    f.clear();
#ifdef ISO_CPP_HEADERS
	    istream::int_type nl = f.get(); // get the newline
#else
	    int nl = f.get(); // get the newline
#endif
#ifdef ISO_CPP_HEADERS
	    if (f.eof() && line.str().length() == 0) {  
#else
	    if (f.eof() && line.pcount() == 0) {  
#endif
		if (verbose) {
		    msg("Eof after getting newline.", normal_sev) << eom;
		}
		if (verbose) {
		    msg("Completed $1 input lines.", normal_sev)
			<< count << eoarg << eom;
		}
	        break;
	    }
	    if (f.bad() || f.eof()) {
		if (verbose) {
		    msg("Input error getting newline after $1 successful input lines.", normal_sev)
			<< count << eoarg << eom;
		}
		ok = false;
		break;
	    }
	    f.clear();
	    count += 1;
	    if (ok) {
		string result;
#ifndef ISO_CPP_HEADERS
	        line << ends;
#endif
		if (verbose) {
		    msg("Translating input line: $1", normal_sev)
			<< line.str() << eoarg << eom;
		}
		ok = massage_line(line.str(), interp, result);
		if (ok) {
		    cout << result << endl;
		    if (verbose) {
			msg("Translated input line to: $1", normal_sev)
			    << result << eoarg << eom;
		    }
		}
		else {
		    msg("Parser flags error using configuration $2: $1", catastrophe_sev)
			<< result << eoarg << configuration << eoarg << eom;
		}
	    }
	    else if (verbose) {
		msg("Not translating input line $1 due to error.", normal_sev)
		    << count << eoarg << eom;
	    }
	}
    }
    return ok;
}

// ===========================================================================
bool check_args(int argc, char *argv[], const char *&filename,
                                        vector<string> &envargs,
					bool &verbose,
					msg_sev &unrec_arg) {
    int fncount = 0;
    const char *mg = NULL;
    const char *lp = NULL;
    const char *ur = NULL;
    bool bad = false;
    for (int i = 1; i < argc; i += 1) {
	if (argv[i] == NULL) {
	    bad = true;
	}
	else if (strcmp(argv[i], "--message_group") == 0
	         && i+1 < argc && argv[i+1] != NULL && mg == NULL) {
	    mg = argv[i+1];
	    if (lp != NULL) {
		init_lp_service(lp, mg);
	    }
	    i += 1;
	}
	else if (strcmp(argv[i], "--lpservice") == 0
	         && i+1 < argc && argv[i+1] != NULL && lp == NULL) {
	    lp = argv[i+1];
	    if (mg != NULL) {
		init_lp_service(lp, mg);
	    }
	    i += 1;
	}
	else if (strcmp(argv[i], "--verbose") == 0) {
	    verbose = true;
	}
	else if (strcmp(argv[i], "--unrecognized") == 0
	         && i+1 < argc && argv[i+1] != NULL && ur == NULL) {
	    ur = argv[i+1];
	    msg_sev sev;
	    bool ok = string_to_severity(ur, sev);
	    if (ok) {
	        unrec_arg = sev;
	    }
	    else {
		msg("Invalid severity in --unrecognized $1.\n"
		    "Valid severities are: "
		    "normal, warning, error, catastrophe, and no_message.",
		    catastrophe_sev) << ur << eoarg << eom;
	    }
	    i += 1;
	}
	else {
	    if (argv[i] == NULL) {
		bad = true;
	    }
	    else if (fncount == 0) {
		filename = argv[i];
		fncount += 1;
	    }
	    else {
	        envargs.push_back(string(argv[i]));
	    }
	}
    }
    return !bad && fncount == 1;
}

// ===========================================================================
int main (int argc, char *argv[]) {
    const char *filename;
    vector<string> envargs;
    bool verbose = false;
    msg_sev unrec_sev = msg_sev(unspecified_sev);
    bool ok = check_args(argc, argv, filename, envargs, verbose, unrec_sev);
    if (ok) {
	ok = process_by_file(filename, envargs, verbose, unrec_sev);
    }
    else {
	cerr << "usage: parsercmd <config file> [arg1 ...] [--lpservice <name> --message_group <id>]" << endl;
    }
    return ok ? 0 : 1;
}
