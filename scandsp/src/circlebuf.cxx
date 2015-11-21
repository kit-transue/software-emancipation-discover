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
#include "circlebuf.h"
#include <iostream>

namespace std {}
using namespace std;

// ==========================================================================
CircleBuf::CircleBuf(unsigned long sz) : bufsize(sz)
{
    circle = new char[bufsize];
    circle_back = circle + bufsize;
    circle_count = 0;
    track_count = 0;
    circle_p = circle;
}

// ==========================================================================
CircleBuf::~CircleBuf()
{
    delete [] circle;
}

// ==========================================================================
void CircleBuf::track(char ch)
{
    track_count += 1;
    *circle_p = ch;
    circle_p += 1;
    if (circle_p == circle_back) {
	circle_p = circle;
    }
    if (circle_count < bufsize) {
	circle_count += 1;
    }
}

// ==========================================================================
// Return the number of bytes in the history.
// Limited by the size of the buffer, which is set upon construction.
//
unsigned long CircleBuf::count()
{
    return circle_count;
}

// ==========================================================================
// Return the count of all bytes ever tracked.
//
unsigned long CircleBuf::trackCount()
{
    return track_count;
}

// ==========================================================================
// Indexes into the available characters (there are count() of them)
// with 0 yielding the first character in the buffer, -1 yielding the
// last, and all other indices selecting based on modulo count().
//
char CircleBuf::operator [] (long idx)
{
    if (count == 0) {
	return '\0';
    }
    else {
	idx = idx % (long)circle_count;
	if (idx >= 0) {
	    idx -= (long)circle_count;
	}
	int circle_idx = (circle_p - circle) + idx;
	return (circle_idx >= 0 ? circle : circle_back)[circle_idx];
    }
}

// ==========================================================================
// Return a pointer to the null at the end of the given string.
//
static const char *endOf(const char *p)
{
    const char *e = p;
    while (*e != '\0') e++;
    return e;
}

// ==========================================================================
// Returns whether the buffer is at a position just past the given string.
// That is, true if the string matches the immediate history.
//
bool bufAt(CircleBuf &buf, const char *p, long count)
{
    if (count > buf.count()) {
	return false;
    }
    long idx = 0;
    do {
        idx -= 1;
        count -= 1;
        if (buf[idx] != p[count]) {
	    return false;
	}
    } while (count > 0);
    return true;
}

// ==========================================================================
// Extracts a short integer from the buffer, ending at the given offset.
//
short getBufShort(CircleBuf &buf, long offset)
{
    short result = buf[offset-1];
    result <<= 8;
    result |= buf[offset-2];
    return result;
}

// ==========================================================================
// Convert a byte's numeric value to printable hexadecimal form,
// e.g. 'A' -> "\21".  Oops, that's not right, but it was good enough for
// debugging.
//
static void hexof(char ch, char *buf)
{
    buf[0] = '\\';
    for (int i = 2; i >= 1; i -= 1) {
	short d = ch % 16;
	if (d < 0) d += 16;
	ch /= 16;
	buf[i] = "0123456789abcdef"[d];
    }
    buf[3] = '\0';
}

// ==========================================================================
// Write to cout the 'num' most recently tracked bytes.  For debugging.
//
void reportBuf(CircleBuf &c, long num)
{
    bool skipping_nulls = true;
    cout << "    ";
    for (int i = -num; i < 0; i += 1) {
	char ch = c[i];
	skipping_nulls = skipping_nulls && ch == '\0';
	if (skipping_nulls) {
	}
	else if (isprint(ch)) {
	    cout << ch;
	}
	else {
	    char hex[4];
	    hexof(ch, hex);
	    cout << hex;
	}
    }
    cout << endl;
}
