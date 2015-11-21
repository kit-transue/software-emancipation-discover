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
#ifndef _gtString_h
#define _gtString_h

// gtString.h
//------------------------------------------
// synopsis:
// Character sequence with multiple character sets.
//
//------------------------------------------

class genString;
class gtStringRep;

class gtString
{
  public:
    virtual ~gtString();

    // Create a new gtString for the current UI library.
    static gtString* create(const char* string = 0, const char* charset = 0);

    // Fetch the string that matches the specified charset.
    // Return 0 on failure.
    virtual int string(genString&, const char* charset) = 0;

    // Append a string to this one.
    virtual void append(gtString*) = 0;

    // Prepend a string to this one.
    virtual void prepend(gtString*) = 0;

    virtual gtStringRep* rep() = 0;

  protected:
    gtString();
};

#endif // _gtString_h

/*
   START-LOG-------------------------------------------

// $Log: gtString.h  $
// Revision 1.1 1993/07/28 19:47:52EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:41  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:01  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:00  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:00  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:26  smit
 * Initial revision
 * 
// Revision 1.1  92/07/28  00:10:45  glenn
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
