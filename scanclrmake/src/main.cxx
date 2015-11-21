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
#include <fstream>
#include <iostream>
#else
#include <fstream.h>
#include <iostream.h>
#endif

#include "msg.h"
#include "crparser.h"

namespace std {};
using namespace std;

// ===========================================================================
// Read the named ClearCase config record file,
// identifying source files and their command line options.
//
static bool process_file(const char *file_name,
			 const char *compiler_list,
			 const char *output_file,
			 bool verbose,
			 bool allow_any_extension,
			 bool allow_any_flags) {

    ifstream f(file_name, ios::in);
#ifdef ISO_CPP_HEADERS
    bool ok = f.is_open();
#else
    bool ok = f.rdbuf()->is_open();
#endif
    if (!ok) {
	msg("Could not open config record file $1.", catastrophe_sev)
		<< file_name << eoarg << eom;
    }
    ofstream *ofstrm = NULL;
#ifdef hp10
    /*
     * Ok, the aCC compiler doesn't like our standard code, so I
     * introduced this hack to get the ofstream assigned to the ostream
     * Either way, os is assigned some kind of output stream below.
     */
    ostream &ostrm = static_cast<ostream &>(*ofstrm);
    ostream_withassign &os = static_cast<ostream_withassign &>(ostrm);
#endif
    if (output_file != NULL) {
	ofstrm = new ofstream(output_file, ios::out);
#ifdef ISO_CPP_HEADERS
        if (ofstrm == NULL || !ofstrm->is_open()) {
#else
        if (ofstrm == NULL || !ofstrm->rdbuf()->is_open()) {
#endif
	    ok = false;
	    ofstrm = NULL;
	    msg("Could not open output file $1.", catastrophe_sev)
		<< output_file << eoarg << eom;
	}
    }

#ifndef hp10
    ostream &os = (ofstrm != NULL)
		    ? static_cast<ostream &>(*ofstrm)
		    : cout;
#else
    if (NULL == ofstrm)
	os = cout;
    else
	os = *ofstrm;
#endif

    if (ok) {
	ok = CRParser::parseCR(f, file_name, compiler_list, os, verbose,
			       allow_any_extension, allow_any_flags);
    }
    return ok;
}

// ===========================================================================
bool check_args(int argc, char *argv[], const char *&filename,
					const char *&compiler_list,
					const char *&output_file,
					bool &verbose,
					bool &allow_any_extension,
					bool &allow_any_flags) {
    int fncount = 0;
    const char *mg = NULL;
    const char *lp = NULL;
    bool bad = false;
    compiler_list = NULL;
    output_file = NULL;
    verbose = false;
    allow_any_extension = false;
    allow_any_flags = false;
    for (int i = 1; i < argc; i += 1) {
	const char *nextarg = NULL;
	if (i + 1 < argc) {
	    nextarg = argv[i + 1];
	    if (nextarg != NULL && nextarg[0] == '-') {
		nextarg = NULL;
	    }
	}
	if (argv[i] == NULL) {
	    bad = true;
	}
	else if (strcmp(argv[i], "-v") == 0
	    || strcmp(argv[i], "-V") == 0
	    || strcmp(argv[i], "--verbose") == 0) {
	    verbose = true;
	}
	else if (strcmp(argv[i], "--any_extension") == 0) {
	    allow_any_extension = true;
	}
	else if (strcmp(argv[i], "--any_flags") == 0) {
	    allow_any_flags = true;
	}
	else if (strcmp(argv[i], "-o") == 0
	    || strcmp(argv[i], "-O") == 0
	    || strcmp(argv[i], "--output") == 0) {
	    if (nextarg != NULL) {
		output_file = nextarg;
		i += 1;
	    }
	    else {
		bad = true;
	    }
	}
	else if (strcmp(argv[i], "--message_group") == 0) {
	    if (nextarg != NULL && mg == NULL) {
		mg = nextarg;
		if (lp != NULL) {
		    init_lp_service(lp, mg);
		}
		i += 1;
	    }
	    else {
		bad = true;
	    }
	}
	else if (strcmp(argv[i], "--lpservice") == 0) {
	    if (nextarg != NULL && lp == NULL) {
		lp = nextarg;
		if (mg != NULL) {
		    init_lp_service(lp, mg);
		}
		i += 1;
	    }
	    else {
		bad = true;
	    }
	}
	else if (strcmp(argv[i], "--compilers") == 0) {
	    if (nextarg != NULL) {
		compiler_list = nextarg;
		i += 1;
	    }
	    else {
		bad = true;
	    }
	}
	else if (argv[i][0] == '-') {
	    // unrecognized option
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
    return fncount == 1 && !bad;
}

// ===========================================================================
int main (int argc, char *argv[]) {
    const char *filename;
    const char *compiler_list = NULL;
    const char *output_file = NULL;
    bool verbose = false;
    bool allow_any_extension;
    bool allow_any_flags;
    bool ok = check_args(argc, argv, filename, compiler_list,
			  	     output_file, verbose,
				     allow_any_extension, allow_any_flags);
    if (ok) {
	ok = process_file(filename, compiler_list, output_file, verbose,
                          allow_any_extension, allow_any_flags);
    }
    else {
	cerr << "usage: scanclrmake <catcr output file> --compilers <comp>:<par>[/..]"
                " [--verbose] [--any_extension] [--any_flags]"
                " [--lpservice <name> --message_group <id>]" << endl;
    }
    return ok ? 0 : 1;
}
