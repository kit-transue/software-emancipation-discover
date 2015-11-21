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
#ifndef _motifString_h
#define _motifString_h

// motifString.h
//------------------------------------------
// synopsis:
// Utility class for using XmString.
//
// description:
//------------------------------------------

#ifndef _Xm_h
#define XTFUNCPROTO
#define Object XObject
#include <Xm/Xm.h>
#undef Object
#endif


class motifString
{
  public:
    motifString(XmString xms)
	: string(0), xm_string(xms), xm(0)
    { XmStringGetLtoR(xm_string, XmSTRING_DEFAULT_CHARSET, &string); }

    motifString(const char* s)
	: string((char *)s), xm_string(XmStringCreateSimple((char *)s)), xm(1)
    {}

    motifString(char* s)
	: string(s), xm_string(XmStringCreateSimple(s)), xm(1)
    {}

    ~motifString()
    { (xm ? (XmStringFree(xm_string),0) : (string ? (XtFree(string),0) : 0)); }

    int is_valid()
    { return string != 0; }

    operator char*()
    { return string; }

    operator XmString()
    { return xm_string; }

    char* operator ()()
    { return string; }

  private:
    char* string;
    XmString xm_string;
    const short xm;
};

#endif // _motifString_h

/*
    START-LOG-------------------------------

    $Log: motifString.h  $
    Revision 1.2 1999/11/11 13:39:14EST sschmidt 
    HP 10 port
 * Revision 1.2.1.2  1992/10/09  19:57:04  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

