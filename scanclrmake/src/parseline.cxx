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
#include "parseline.h"
#include "RegExp.h"

namespace std {}
using namespace std;

// ===========================================================================
ParseLine::ParseLine(const char *line)
  : line(line)
{
}

// ===========================================================================
// Return whether the line has the form of a section heading, i.e.
// heading name followed by ':'.
// If so, set 'str' to the name string.
//
bool ParseLine::getHeadName(string &str)
{
    Regexp re(":( |\t)*$");
    bool matched = re.Match(line.c_str(), line.length(), 0) >= 0;
    if (matched) {
    	str = string(line.c_str(), re.BeginningOfMatch(0));
    }
    return matched;
}

// ===========================================================================
// Return whether the line has the form of a target title, 
// i.e. "Target xxxxx ..."
// If so, set 'str' to the target name.
//
bool ParseLine::getTargetName(string &str)
{
    bool matched = false;
    Regexp re("^Target ");
    if (re.Match(line.c_str(), line.length(), 0) >= 0) {
	// Have "Target ". Look for "built".
        Regexp re2("( |\t)+built");
	int len = re2.Match(line.c_str(), line.length(), re.EndOfMatch(0));
	if (len >= 0) {
	    matched = true;
	    str = string(line.c_str() + re2.BeginningOfMatch(0), len);
	}
    }
    return matched;
}

// ===========================================================================
// Return whether the line has the form of a working directory line.
// For example, "Initial working directory was /d00d".
//
bool ParseLine::getDirectory(string &str)
{
    bool matched = false;
    Regexp re("directory was( |\t)+");
    if (re.Match(line.c_str(), line.length(), 0) >= 0) {
	// Have directory intro. The remainder of the line is the directory.
        Regexp re2("( |\t)+built");
	matched = true;
	str = string(line.c_str() + re.EndOfMatch(0));
    }
    return matched;
}

// ===========================================================================
bool ParseLine::isRule()
{
    Regexp re("^-----*$");
    return re.Match(line.c_str(), line.length(), 0) >= 0;
}

// ===========================================================================
string ParseLine::getLine()
{
    return line;
}
