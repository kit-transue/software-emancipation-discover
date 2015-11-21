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
#ifndef _gtManager_h
#define _gtManager_h

// gtManager.h
//------------------------------------------
// synopsis:
// GT Manager class descripion
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtManager : public gtPrimitive
{
  public:
    ~gtManager();

  protected:
    gtManager();
};

#endif // _gtManager_h

/*
   START-LOG-------------------------------------------

 $Log: gtManager.h  $
 Revision 1.1 1993/07/28 19:47:44EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:17  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:55  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:54  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:52  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:21  smit
 * Initial revision
 * 
// Revision 1.4  92/09/11  11:12:11  builder
// Make destructor public.
// 
// Revision 1.3  92/09/11  04:07:28  builder
// Add ctor and dtor.
// 
// Revision 1.2  92/07/30  23:44:44  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:02  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
