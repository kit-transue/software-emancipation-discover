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
#include "minidom.h"

using namespace std;

namespace MiniXML {

//===========================================================================
DOMString::DOMString()
    : buf(new XMLCh[1]),
      len(0)
{
    buf[0] = 0;
}

//===========================================================================
DOMString::DOMString(const DOMString &that)
    : buf(new XMLCh[that.len + 1]),
      len(that.len)
{
    for (unsigned long i = 0; i <= len; i += 1) {
	buf[i] = that.buf[i];
    }
}

// ===========================================================================
DOMString::DOMString(const XMLCh *p)
    : len(XMLs_len(p))
{
    buf = new XMLCh[len + 1];
    for (unsigned long i = 0; i <= len; i += 1) {
	buf[i] = p[i];
    }
}

// ===========================================================================
DOMString::DOMString(const XMLCh *p, unsigned int length)
    : len(length),
      buf(new XMLCh[length + 1])
{
    for (unsigned long i = 0; i < len; i += 1) {
	buf[i] = p[i];
    }
    buf[len] = 0;
}

// ===========================================================================
DOMString::DOMString(const XMLCh ch)
    : len(1),
      buf(new XMLCh[2])
{
    buf[0] = ch;
    buf[1] = 0;
}

// ===========================================================================
DOMString::~DOMString()
{
    delete [] buf;
}

// ===========================================================================
DOMString &DOMString::operator = (const DOMString &that)
{
    if (&that != this) {
	delete [] buf;
	len = that.len;
	buf = new XMLCh[len + 1];
	for (unsigned long i = 0; i <= len; i += 1) {
	    buf[i] = that.buf[i];
	}
    }
    return *this;
}

// ===========================================================================
unsigned long DOMString::length() const
{
    return len;
}

// ===========================================================================
DOMString &DOMString::operator +=(const DOMString &that)
{
    unsigned long new_len = len + that.len;
    XMLCh *new_buf = new XMLCh[new_len + 1];
    for (unsigned long i = 0; i < len; i += 1) {
	new_buf[i] = buf[i];
    }
    for (unsigned long j = 0; j < that.len; j += 1) {
	new_buf[len + j] = that.buf[j];
    }
    new_buf[new_len] = 0;
    
    delete [] buf;
    buf = new_buf;
    len = new_len;

    return *this;
}

// ===========================================================================
DOMString &DOMString::operator +=(const XMLCh *p)
{
    unsigned long pl = XMLs_len(p);
    unsigned long new_len = len + pl;
    XMLCh *new_buf = new XMLCh[new_len + 1];
    for (unsigned long i = 0; i < len; i += 1) {
	new_buf[i] = buf[i];
    }
    for (unsigned long j = 0; j < pl; j += 1) {
	new_buf[len + j] = p[j];
    }
    new_buf[new_len] = 0;
    
    delete [] buf;
    buf = new_buf;
    len = new_len;

    return *this;
}

// ===========================================================================
DOMString &DOMString::operator +=(XMLCh ch)
{
    unsigned long new_len = len + 1;
    XMLCh *new_buf = new XMLCh[new_len + 1];
    for (unsigned long i = 0; i < len; i += 1) {
	new_buf[i] = buf[i];
    }
    new_buf[len] = ch;
    new_buf[new_len] = 0;
    
    delete [] buf;
    buf = new_buf;
    len = new_len;

    return *this;
}

// ===========================================================================
bool DOMString::equals(const DOMString &that)
{
    if (len != that.len) {
	return false;
    }
    else {
	for (int i = 0; i < len; i += 1) {
	    if (buf[i] != that.buf[i]) {
		return false;
	    }
	}
    }
    return true;
}

// ===========================================================================
// The caller must not assume that the returned
// string is null terminated.  Its length is given by length().
//
const XMLCh *DOMString::rawBuffer() const
{
    return buf;
}

// ===========================================================================
XMLCh DOMString::charAt(unsigned int index) const
{
    if (index < 0 || len <= index) {
	return 0;
    }
    else {
	return buf[index];
    }
}

// ===========================================================================
string trivialStringFromDOMString(const DOMString &src)
{
    char *p = trivialFromDOMString(src);
    string retval = p;
    delete [] p;
    return retval;
}

// ===========================================================================
void trivialToDOMString(DOMString &str, const char *p)
{
    XMLCh *tmp = charsToXMLs(p);
    str = tmp;
    delete [] tmp;
}

// ===========================================================================
DOMString trivialToDOMString(const char *p)
{
    DOMString s;
    trivialToDOMString(s, p);
    return s;
}

// ===========================================================================
DOMString trivialToDOMString(const string &s)
{
    return trivialToDOMString(s.c_str());
}

// ===========================================================================
char *trivialFromDOMString(const DOMString &str)
{
    unsigned long len = str.length();
    char *buf = new char[len + 1];
    const XMLCh *strbuf = str.rawBuffer();
    for (int i = 0; i < len; i += 1) {
	buf[i] = (char)strbuf[i];
    }
    buf[len] = '\0';
    return buf;
}

}
