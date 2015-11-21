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
#pragma warning (disable:4786)

#include <fstream>
#include <iostream>

#include "dspparser.h"
#include "dswparser.h"
#include "msg.h"

using namespace std;

// ===========================================================================
// Read the named DSP (MS Developer Studio Project) or DSW (Workspace) file,
// identifying source files and their command line options.
//
static bool process_file(const char *file_name, const char *output, bool verbose) {
    ifstream f(file_name, ios::in);
    bool ok = f.is_open();
    if (ok) {
	ofstream *ofstrm = NULL;
	if (output != NULL) {
	    ofstrm = new ofstream(output, ios::out);
	    if (ofstrm == NULL || !ofstrm->is_open()) {
		ok = false;
		ofstrm = NULL;
		msg("Could not open output file $1.", catastrophe_sev) 
		    << output <<eoarg << eom;
	    }
	}
	ostream &ostrm = (ofstrm != NULL) 
	    ? static_cast<ostream &>(*ofstrm) : cout;

	if (DSWParser::isDSW(file_name)) {
	    ok = DSWParser::parseDSW(f, file_name, ostrm, verbose);
	}
	else {
	    ok = DSPParser::parseDSP(f, file_name, true, ostrm, verbose);
	}
    }
    else {
	msg("Could not open input file $1.", catastrophe_sev)
	    << file_name << eoarg << eom;
    }
    return ok;
}

// ===========================================================================
// Find the -I command line options which are associated with the user's
// installation of Microsoft Visual Studio, and put them in the named file.
//
bool process_query(const char *output) {
    bool ok = true;
    ofstream *ofstrm = NULL;
    if (output != NULL) {
	ofstrm = new ofstream(output, ios::out);
	if (ofstrm == NULL || !ofstrm->is_open()) {
	    ok = false;
	    ofstrm = NULL;
	    msg("Could not open output file $1.", catastrophe_sev) 
		<< output <<eoarg << eom;
	}
    }
    ostream &ostrm = (ofstrm != NULL) 
	? static_cast<ostream &>(*ofstrm) : cout;

    string cmd_options;
    ok = ok && DSPParser::analyzeVCIncludes(cmd_options, true);
    if (ok) {
	ostrm << cmd_options << endl;
    }
    if (ofstrm != NULL && ofstrm->is_open()) {
	ofstrm->close();
    }
    return ok;
}

// ===========================================================================
// Find the installation dir of Microsoft Visual Studio, and put it in the named file.
//
bool process_msvc_query(const char *output) {
    bool ok = true;
    ofstream *ofstrm = NULL;
    if (output != NULL) {
	ofstrm = new ofstream(output, ios::out);
	if (ofstrm == NULL || !ofstrm->is_open()) {
	    ok = false;
	    ofstrm = NULL;
	    msg("Could not open output file $1.", catastrophe_sev) 
		<< output <<eoarg << eom;
	}
    }
    ostream &ostrm = (ofstrm != NULL) 
	? static_cast<ostream &>(*ofstrm) : cout;

    string msvc_dir;
    ok = ok && DSPParser::analyzeVCDir(msvc_dir);
    if (ok) {
	ostrm << msvc_dir << endl;
    }
    if (ofstrm != NULL && ofstrm->is_open()) {
	ofstrm->close();
    }
    return ok;
}

// ===========================================================================
bool check_args(int argc, char *argv[], const char *&filename,
			    const char *&output, bool &verbose,
			    bool &is_query,
			    bool &is_msvc_query) {
    int fncount = 0;
    const char *mg = NULL;
    const char *cumg = NULL;
    const char *lp = NULL;
    bool bad = false;
    bool saw_get = false;
    bool saw_get_msvc = false;
    for (int i = 1; i < argc; i += 1) {
	if (argv[i] == NULL) {
	    bad = true;
	}
	else if (strcmp(argv[i], "-o") == 0
	    || strcmp(argv[i], "-O") == 0
	    || strcmp(argv[i], "--output") == 0) {
	    output = argv[i+1];
	    if (output == NULL)
		bad = true;
	    i += 1;
	}
	else if (strcmp(argv[i], "-v") == 0
	    || strcmp(argv[i], "-V") == 0
	    || strcmp(argv[i], "--verbose") == 0) {
	    verbose = true;
	}
	else if (strcmp(argv[i], "--message_group") == 0
	         && i+1 < argc && argv[i+1] != NULL && mg == NULL) {
	    mg = argv[i+1];
	    if (lp != NULL) {
		init_lp_service(lp, mg);
	    }
	    i += 1;
	}
	else if (strcmp(argv[i], "--compilation_units_message_group") == 0
	         && i+1 < argc && argv[i+1] != NULL && cumg == NULL) {
	    cumg = argv[i+1];
	    compilation_units_message_group_id = cumg;
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
	else if (strcmp(argv[i], "--get_include_path") == 0) {
	    saw_get = true;
	}
	else if (strcmp(argv[i], "--get_msvc_dir") == 0) {
	    saw_get_msvc = true;
	}
	else if (argv[i][0] == '-') {
	    bad = true;
	}
	else {
	    fncount += 1;
	    filename = argv[i];
	    if (filename == NULL) {
		bad = true;
	    }
	}
    }
    is_query = saw_get;
    is_msvc_query = saw_get_msvc;
    return fncount == ((saw_get || saw_get_msvc) ? 0 : 1) && !bad;
}

// ===========================================================================
int main (int argc, char *argv[]) {
    const char *filename, *output = NULL;
    bool verbose;
    bool is_query;
    bool is_msvc_query;
    bool ok = check_args(argc, argv, filename, output, verbose, is_query, is_msvc_query);
    if (ok) {
	if (!is_query && !is_msvc_query) {
	    ok = process_file(filename, output, verbose);
	}
	else if (is_query) {
	    ok = process_query(output);
	}
	else {
	    ok = process_msvc_query(output);
	}
    }
    else {
	cerr << "usage: scandsp <project file> --output <file> [--verbose] [--lpservice <name> --message_group <id>]" << endl;
	cerr << "   or: scandsp --get_include_path --output <file> [--lpservice <name> --message_group <id>]" << endl;
	cerr << "   or: scandsp --get_msvc_dir --output <file> [--lpservice <name> --message_group <id>]" << endl;
    }
    return ok ? 0 : 1;
}
