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
#include "minixml.h"
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif

using namespace std;

namespace MiniXML {

// ===========================================================================
// ===========================================================================
// Utility Functions
// ===========================================================================
// The length of a string of XMLCh's
//
int XMLs_len(const XMLCh *str) {
    if (str == NULL) {
	return -1;
    }
    else {
	const XMLCh *p = str;
	while (*p != 0) {
	    p += 1;
	}
	return p - str;
    }
}

// ===========================================================================
// Replicate an XMLCh string in independent dynamic storage.
// Caller must eventually free the storage.
//
XMLCh *replicate_XMLs(const XMLCh *str) {
    if (str == NULL) {
	return NULL;
    }
    int len = XMLs_len(str);
    XMLCh *result = new XMLCh[len + 1];
    if (result != NULL) {
	for (int i = 0; i <= len; i += 1) {
    	    result[i] = str[i];
	}
    }
    return result;
}

// ===========================================================================
// Utility to help glue the SAX API to ASCII
// Replicate a char string as an XMLCh string in dynamic storage.
// Caller must eventually free the storage.
//
XMLCh *charsToXMLs(const char *str) {
    if (str == NULL) {
	return NULL;
    }
    int len = strlen(str);
    XMLCh *result = new XMLCh[len + 1];
    if (result != NULL) {
	for (int i = 0; i <= len; i += 1) {
    	    result[i] = (unsigned char)str[i];
	}
    }
    return result;
}

// ===========================================================================
// Utility to help glue the SAX API to ASCII
// Replicate an XMLCh string as a char string in dynamic storage.
// Caller must eventually free the storage.
//
char *XMLsToChars(const XMLCh *str) {
    if (str == NULL) {
	return NULL;
    }
    int len = XMLs_len(str);
    char *result = new char[len + 1];
    if (result != NULL) {
	for (int i = 0; i <= len; i += 1) {
    	    result[i] = (char)str[i];
	}
    }
    return result;
}

// ===========================================================================
//
string XMLsToString(const XMLCh *in) {
#ifdef ISO_CPP_HEADERS
    ostringstream result;
#else
    ostrstream result;
#endif
    if (in != NULL) {
        const XMLCh *p = in;
	while (*p != 0) {
	    result << (char)(*p);
	    p += 1;
	}
    }
#ifndef ISO_CPP_HEADERS
    result << ends;
#endif
    return result.str();
}
} // namespace MiniXML
