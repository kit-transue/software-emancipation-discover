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
#ifndef _gtBlank_h
#define _gtBlank_h

// gtBlank.h
//------------------------------------------
// synopsis:
// class description of generic Blank (gtBlank)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtBlank : public gtPrimitive
{
  public:
    static gtBlank* create(
	gtBase *parent, const char *name, int width = 0, int height = 0);

    ~gtBlank();

    virtual void resize_policy(gtResizeOption opt) = 0;
    virtual void set_background(const char *c_name) = 0;
    virtual void clear_window() = 0;
    virtual void expose_callback(gtBlankCB callback, void* client_data) = 0;

  protected:
    gtBlank();
};

#endif // _gtBlank_h

/*
   START-LOG-------------------------------------------

 $Log: gtBlank.h  $
 Revision 1.1 1993/07/29 10:36:08EDT builder 
 made from unix file
 * Revision 1.2.1.3  1993/01/26  05:06:40  glenn
 * Change add_CB to expose_callback.
 * Change default arguments to ctor.
 *
 * Revision 1.2.1.2  1992/10/09  18:09:56  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:07  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:06  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:48  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:18  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:11:51  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:05  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:44:33  glenn
// GT method name changes
// 
// Revision 1.2  92/07/29  15:55:07  rfermier
// put callbacks
// 
// Revision 1.1  92/07/08  20:38:49  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------

*/
