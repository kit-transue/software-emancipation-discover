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
#include <string>
#include "svcnames.h"
#include "startprocess.h"
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif

namespace std {};
using namespace std;
using namespace MBDriver;

// ===========================================================================
const char *mbdriverId()
{
    static string *str = NULL;
    if (str == NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream strm;
#else
	ostrstream strm;
#endif
#ifdef _WIN32
	DWORD pid = GetCurrentProcessId();
#else
	MBDriver::pid_t pid = get_pid();
#endif
	strm << (unsigned int)pid;
#ifndef ISO_CPP_HEADERS
	strm << ends;
#endif
	str = new string(strm.str());
    }
    return str->c_str();
}

// ===========================================================================
const char *lpservice()
{
    static string *str = NULL;
    if (str == NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream strm;
#else
	ostrstream strm;
#endif
	strm << "Build:lp";
	strm << mbdriverId() << ends;
#ifndef ISO_CPP_HEADERS
	strm << ends;
#endif
	str = new string(strm.str());
    }
    return str->c_str();
}

// ===========================================================================
const char *passservice()
{
    static string *str = NULL;
    if (str == NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream strm;
#else
	ostrstream strm;
#endif
	strm << "Build:pass";
	strm << mbdriverId() << ends;
#ifndef ISO_CPP_HEADERS
	strm << ends;
#endif
	str = new string(strm.str());
    }
    return str->c_str();
}

static string *gui_service_name = NULL;

// ===========================================================================
const char *guiservice()
{
    if (gui_service_name == NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream strm;
#else
	ostrstream strm;
#endif
	strm << "Build:gui";
	strm << mbdriverId() << ends;
#ifndef ISO_CPP_HEADERS
	strm << ends;
#endif
	gui_service_name = new string(strm.str());
    }
    return gui_service_name->c_str();
}

// ===========================================================================
void set_gui_service_name(const char *name)
{
    if (name != NULL) {
	gui_service_name = new string(name);
    }
}
