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
#include "nametracker.h"

namespace std {}
using namespace std;

// See full description of the NameTracker class in nametracker.h.

// ==========================================================================
NameTracker::NameTracker()
: tracking(false), have_name(false)
{
}

// ==========================================================================
void NameTracker::start(char ch)
{
    tracking = true;
    expected_count = (unsigned char)ch;
    theName = "";
    have_name = (expected_count == 0);
}

// ==========================================================================
void NameTracker::track(char ch)
{
    if (tracking) {
	if (expected_count > 0) {
	    expected_count -= 1;
	    theName.append(1, ch);
	    if (expected_count == 0) {
		have_name = true;
	    }
	}
	else {
	    tracking = false;
	}
    }
}

// ==========================================================================
bool NameTracker::atEnd()
{
    return have_name && tracking;
}

// ==========================================================================
bool NameTracker::haveName()
{
    return have_name;
}

// ==========================================================================
string NameTracker::name()
{
    return theName;
}
