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
/*
This program makes it possible to do without the PSETHOME and
DISIM environment variables.  It only need be installed in the
user's PATH, and it will find the DIScover installation.
   1. (Win32) Installation of CIE 7.5 in registry.
   2. PSETHOME environment variable, if valid.
   3. PATH environment variable, if it contains a bin directory
      where CIE is installed.
The --check command line flag suppresses standard output.
 */
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

#include <env_lib.h>

// The various functions of this program:
enum {get_nothing, get_psethome, get_disim, get_help};

// Returns 0 in case of success, 1 if failure.
// mode is set to the enum to indicate the desired function.
int args(int argc, char *argv[], int *mode, bool *check_only) {
    int psethome = 0;
    int disim = 0;
    int garbage = 0;
    int help = 0;
    int i;
    *mode = get_nothing;
    *check_only = false;
    for (i = 1; i < argc; i += 1) {
	if (strcmp(argv[i], "--psethome") == 0) {
		psethome = 1;
		*mode = get_psethome;
	}
	else if (strcmp(argv[i], "--disim") == 0) {
		disim = 1;
		*mode = get_disim;
	}
	else if (strcmp(argv[i], "--check") == 0) {
		*check_only = true;
	}
	else if (strcmp(argv[i], "--help") == 0) {
		help = 1;
		*mode = get_help;
	}
	else if (strcmp(argv[i], "-?") == 0) {
		help = 1;
		*mode = get_help;
	}
	else {
		garbage = 1;
	}
    }
    return (psethome + disim + help + garbage <= 1 && !garbage) ? 0 : 1;
}

// Analyze the command line arguments, then act accordingly.
int main(int argc, char *argv[]) {
    int mode;
    bool check_only;
    int return_code = args(argc, argv, &mode, &check_only);
    if (return_code != 0 || mode == get_help) {
	cerr << "Usage: ci_environment [--psethome|--disim] [--check]" << endl;
	return return_code;
    }
    if (mode == get_psethome) {
	string psethome;
	if (find_psethome(psethome) == 0) {
	    if (!check_only) {
		cout << psethome;
		cout.flush();  // needed on HP
	    }
	}
	else {
	    return 1;
	}
    }
    else if (mode == get_disim) {
	string disim;
	if (find_disim(disim) == 0) {
	    if (!check_only) {
		cout << disim;
		cout.flush();  // needed on HP
	    }
	}
	else {
	    return 1;
	}
    }
    return 0;
}
