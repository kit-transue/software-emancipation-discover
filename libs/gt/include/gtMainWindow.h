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
#ifndef _gtMainWindow_h
#define _gtMainWindow_h

// gtMainWindow.h
//------------------------------------------
// synopsis:
// 
// Composite object containing menubar, work area,
// and (optional) message area.
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif


class gtMainWindow : public gtPrimitive
{
  public:
    static gtMainWindow* create(
	gtBase* parent, const char* name, int make_message_area = 1);

    ~gtMainWindow();

    virtual gtMenuBar* menu_bar() = 0;
    virtual void message(const char*) = 0;

  protected:
    gtMainWindow();
};


/*
   START-LOG-------------------------------------------

   $Log: gtMainWindow.h  $
   Revision 1.1 1993/07/29 10:36:13EDT builder 
   made from unix file
// Revision 1.1  1993/01/14  19:47:14  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtMainWindow_h
