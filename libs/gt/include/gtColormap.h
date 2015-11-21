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
#ifndef _gtColormap_h
#define _gtColormap_h

// gtColormap.h
//------------------------------------------
// synopsis:
// ...
//
// description:
// ...
//------------------------------------------

typedef XColor gtColor;

class gtColormap {
  public:
    static gtColormap *create(gtBase *);
    virtual void alloc_cells(char, unsigned long *, int, unsigned long *, int) = 0;
    virtual void query_color(gtColor *) = 0;
    virtual void store_colors(gtColor *, int) = 0;
    virtual void free_colors(unsigned long *, int, unsigned long) = 0;
    virtual void parse_color(char *, gtColor *) = 0;
};    

#endif // _gtColormap_h

/*
   START-LOG-------------------------------------------

 $Log: gtColormap.h  $
 Revision 1.1 1993/07/28 19:47:39EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:09:58  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:14  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:13  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:48  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:18  smit
 * Initial revision
 * 
// Revision 1.1  92/08/18  12:22:23  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/

