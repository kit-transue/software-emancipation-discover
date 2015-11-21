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
#ifndef _gtNoParent_h
#define _gtNoParent_h

// gtNoParent.h
//------------------------------------------
// Raw GT object that does not need a parent.
//------------------------------------------

#ifndef _gtBase_h
#include <gtBase.h>
#endif


class gtNoParent : public gtBase
{
  public:
    static gtNoParent* create();
    static gtNoParent* create(void* widget);
    static gtNoParent* app_shell();

    ~gtNoParent();

  private:
    gtNoParent();
    gtNoParent(void*);
};


/*
   START-LOG-------------------------------------------

// $Log: gtNoParent.h  $
// Revision 1.1 1993/02/24 07:52:11EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:08:06  glenn
 * Add create with void* widget.
 * Add ctor with void* widget.
 *
// Revision 1.5  92/09/11  11:12:14  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  08:17:54  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.3  92/09/11  04:07:32  builder
// Add ctor and dtor.
// 
// Revision 1.2  92/07/30  23:44:48  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:04  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtNoParent_h
