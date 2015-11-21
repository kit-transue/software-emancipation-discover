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
#ifndef _gtBitmapXm_h
#define _gtBitmapXm_h

// gtBitmapXm.h
//------------------------------------------
// synopsis:
// 
// One-bit deep image.
//------------------------------------------

#ifndef _gtBitmap_h
#include <gtBitmap.h>
#endif
#ifndef _gtBaseXm_h
#include <gtBaseXm.h>
#endif


class gtBitmapXm : public gtBitmap
{
  public:
    gtBitmapXm(gtBase*, const char*, const char*, int, int);
    ~gtBitmapXm();

  protected:
    gtBitmapXm(void);

  protected:
    Pixmap pixmap;
};


/*
   START-LOG-------------------------------------------

   $Log: gtBitmapXm.h  $
   Revision 1.2 1999/08/11 17:37:15EDT rpasseri 
   adding protected default constructor for gtPixmapXm to use
// Revision 1.1  1993/02/05  23:35:03  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtBitmapXm_h
