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
#include "util.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(irix6) || defined(sun5)
#include <sys/systeminfo.h>
#endif
#if defined(hp10)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <winsock2.h>
#endif
namespace std {}
using namespace std;

namespace scanclrmake {

// ===========================================================================
string &psethome()
{
    static string *str = NULL;
    if (str == NULL) {
	// We could just look up PSETHOME here, but check CODE_ROVER_HOME
	// because that's what the code has done in the past (paranoia).
	const char *envval = getenv("CODE_ROVER_HOME");
	if (envval == NULL) {
	    envval = getenv("PSETHOME");
	}
	str = new string(envval != NULL ? envval : "");
    }
    return *str;
}

// ==========================================================================
void gethostname (char *ainet, int size) {
#if defined(hp10) || defined(_WIN32)
    ::gethostname(ainet, size);
#else
    ::sysinfo(SI_HOSTNAME, ainet, size);
#endif
}

// ==========================================================================
bool executableExists(const string &path)
{
#ifdef _WIN32
    struct _stat st;
    int ret = _stat(path.c_str(), &st);
#else
    struct stat st;
    int ret = stat(path.c_str(), &st);
#endif
    if (ret != 0) {
        return false;
    }
#ifdef _WIN32
    return st.st_mode & _S_IEXEC;
#else
    return st.st_mode & S_IEXEC;
#endif
}

const char *platformName()
{
    return
#ifdef _WIN32
	"WIN32"
#elif defined(sun5)
	"SunOS"
#elif defined(hp10)
	"HP-UX"
#elif defined(irix6)
	"IRIX"
#else
	"unknown"
#endif
	         ;
}

// ==========================================================================
// Trim whitespace from the beginning and end of s.
//
void trim(string &s)
{
    string::size_type sz = s.size();
    string::size_type first_nonspace = string::npos;
    string::size_type after_last_nonspace = 0;
    for (int i = 0; i < sz; i += 1) {
	if (!isspace(s.at(i))) {
	    after_last_nonspace = i + 1;
	    if (i < first_nonspace) {
		first_nonspace = i;
	    }
	}
    }
    s.erase(after_last_nonspace, sz - after_last_nonspace);
    if (first_nonspace != string::npos) {
	s.erase(0, first_nonspace);
    }
}

// ===========================================================================
// Append the name, along with a preceding / or \ path separator.
//
void appendNameInDir(string &path, const string &name)
{
#ifdef _WIN32
    path.append("\\");
#else
    path.append("/");
#endif
    path.append(name);
}

}
