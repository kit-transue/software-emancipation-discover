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
// Minidom provides a DOM-like API.
// Unlike minisax, there is not an attempt to supply a
// strict subset of DOM.

#ifndef MINIDOM_H
#define MINIDOM_H

#include "minixml.h"
#include <string>
namespace std {};
using namespace std;

namespace MiniXML {

class DOMString {
public:
    DOMString();
    DOMString(const DOMString &);
    DOMString(const XMLCh *);
    DOMString(const XMLCh *, unsigned int length);
    DOMString(const XMLCh);
    ~DOMString();
    DOMString &operator = (const DOMString &);
    unsigned long length() const;
    DOMString &operator +=(const DOMString &);
    DOMString &operator +=(const XMLCh *);
    DOMString &operator +=(XMLCh);
    bool equals(const DOMString &);
    const XMLCh *rawBuffer() const; // Don't assume null terminated.
    XMLCh charAt(unsigned int index) const;
private:
    XMLCh *buf;  // always non-null, and null-terminated
    unsigned long len;  // not including final null
};

// class DOM_Node;

const XMLCh chLatin_a = (XMLCh)'a';
const XMLCh chLatin_b = (XMLCh)'b';
const XMLCh chLatin_c = (XMLCh)'c';
const XMLCh chLatin_d = (XMLCh)'d';
const XMLCh chLatin_e = (XMLCh)'e';
const XMLCh chLatin_f = (XMLCh)'f';
const XMLCh chLatin_g = (XMLCh)'g';
const XMLCh chLatin_h = (XMLCh)'h';
const XMLCh chLatin_i = (XMLCh)'i';
const XMLCh chLatin_j = (XMLCh)'j';
const XMLCh chLatin_k = (XMLCh)'k';
const XMLCh chLatin_l = (XMLCh)'l';
const XMLCh chLatin_m = (XMLCh)'m';
const XMLCh chLatin_n = (XMLCh)'n';
const XMLCh chLatin_o = (XMLCh)'o';
const XMLCh chLatin_p = (XMLCh)'p';
const XMLCh chLatin_q = (XMLCh)'q';
const XMLCh chLatin_r = (XMLCh)'r';
const XMLCh chLatin_s = (XMLCh)'s';
const XMLCh chLatin_t = (XMLCh)'t';
const XMLCh chLatin_u = (XMLCh)'u';
const XMLCh chLatin_v = (XMLCh)'v';
const XMLCh chLatin_w = (XMLCh)'w';
const XMLCh chLatin_x = (XMLCh)'x';
const XMLCh chLatin_y = (XMLCh)'y';
const XMLCh chLatin_z = (XMLCh)'z';
const XMLCh chSemiColon = (XMLCh)';';
const XMLCh chOpenAngle = (XMLCh)'<';
const XMLCh chCloseAngle = (XMLCh)'>';
const XMLCh chAmpersand = (XMLCh)'&';
const XMLCh chDoubleQuote = (XMLCh)'"';
const XMLCh chNull = (XMLCh)0;
const XMLCh chDigit_0 = (XMLCh)'0';
const XMLCh chDigit_1 = (XMLCh)'1';
const XMLCh chDigit_2 = (XMLCh)'2';
const XMLCh chDigit_3 = (XMLCh)'3';
const XMLCh chDigit_4 = (XMLCh)'4';
const XMLCh chDigit_5 = (XMLCh)'5';
const XMLCh chDigit_6 = (XMLCh)'6';
const XMLCh chDigit_7 = (XMLCh)'7';
const XMLCh chDigit_8 = (XMLCh)'8';
const XMLCh chDigit_9 = (XMLCh)'9';
const XMLCh chSpace = (XMLCh)' ';
const XMLCh chEqual = (XMLCh)'=';
const XMLCh chForwardSlash = (XMLCh)'/';
const XMLCh chUnderscore = (XMLCh)'_';

string trivialStringFromDOMString(const DOMString &src);
void trivialToDOMString(DOMString &, const char *);
DOMString trivialToDOMString(const char *);
DOMString trivialToDOMString(const string &);
char *trivialFromDOMString(const DOMString &);

}

#endif // MINIDOM_H
