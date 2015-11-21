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
#ifndef _gtColormapXm_h
#define _gtColormapXm_h

// gtColormapXm.h
//------------------------------------------

#include <gt.h>
#include <gtColormap.h>
#include <X11/Xlib.h>

class gtColormapXm : public gtColormap
{
  public:
    gtColormapXm(gtBase *);

    void alloc_cells(char, unsigned long *, int, unsigned long *, int);
    void query_color(gtColor *);
    void store_colors(gtColor *, int);
    void free_colors(unsigned long *, int, unsigned long);
    void parse_color(char *, gtColor *);

  private:
    gtBase*	base_gt;
    Colormap	cmap;
};


/*
   START-LOG-------------------------------------------

// $Log: gtColormapXm.h  $
// Revision 1.1 1993/07/29 10:37:47EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * cleanup
 *
// Revision 1.1  92/08/18  12:22:14  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtColormapXm_h
