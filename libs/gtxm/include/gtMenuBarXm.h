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
#ifndef _gtMenuBarXm_h
#define _gtMenuBarXm_h

// gtMenuBarXm.h
//------------------------------------------
// synopsis:
// Class definitions and headers for XMotif implentation of gtMenuBar
//------------------------------------------

#include <gtMenuBar.h>


class gtMenuBarXm : public gtMenuBar
{
  public:
    gtMenuBarXm(gtBase *parent, const char *name);

    void set_help(gtCascadeButton *);
};


/*
   START-LOG-------------------------------------------

// $Log: gtMenuBarXm.h  $
// Revision 1.1 1993/02/23 23:47:50EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.2  1992/10/09  18:43:51  jon
 * RCS History Marker Fixup
 *
 * Revision 1.2.1.1  92/10/07  22:10:31  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  22:10:30  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:27:40  smit
 * Initial revision
 * 
// Revision 1.3  92/07/30  23:55:53  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/22  13:40:42  rfermier
// added set_help functions
// 
// Revision 1.1  92/07/08  20:41:01  rfermier
// Initial revision

   END-LOG---------------------------------------------
*/

#endif // _gtMenuBarXm_h
