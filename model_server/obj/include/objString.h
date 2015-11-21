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
//////////////  objString.h 
//
#ifndef _objString_h
#define _objString_h

#ifndef _genString_h
#include "genString.h"
#endif

#ifndef _objDb_h
#include "objDb.h"
#endif

DefinePtr (objString);

class objString : public Obj, public genString
{
 public:
    objString() {}
    ~objString();
    objString(objString const &) {}
    objString(char const *st) : genString(st) {};
    objString(char const *st, char* buffer); //Copy st to buffer (malloc if buffer null)
    objString& operator=(char const *p) {genString::operator=(p); return *this;}
};

#endif				// _objString_h

/*
   START-LOG-------------------------------------------

   $Log: objString.h  $
   Revision 1.1 1993/07/28 16:27:33EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/07/28  20:23:28  mg
 * uninlined virtuals
 *
 * Revision 1.2.1.2  1992/10/09  18:58:19  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
