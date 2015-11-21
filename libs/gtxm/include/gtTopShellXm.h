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
#ifndef _gtTopShellXm_h
#define _gtTopShellXm_h

// gtTopShellXm.h
//------------------------------------------
// synopsis:
// Class definitions and headers for XMotif implentation of gtTopLevelShell
//
//------------------------------------------

#include <gtTopShell.h>
#include <gtCallbackMgrXm.h>


class gtTopLevelShellXm : public gtTopLevelShell, public gtCallbackMgrXm
{
  public:
    gtTopLevelShellXm(gtBase*, const char*);
    ~gtTopLevelShellXm();

    void title(const char*);
    void clear_background();
    void destroy_callback(gtTLShellCB, void*);
    void min_width(int width);
    void min_height(int height);
    void max_width(int width);
    void max_height(int height);
};


/*
   START-LOG-------------------------------------------

   $Log: gtTopShellXm.h  $
   Revision 1.1 1993/12/08 09:59:32EST builder 
   made from unix file
 * Revision 1.2.1.4  1993/12/07  20:45:08  andrea
 * Bug track: 5197
 * I added min_width, min_height, max_width, max_height member functions
 *
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.2  1992/10/09  18:44:34  jon
 * RCS History Marker Fixup
 *
   END-LOG---------------------------------------------
*/

#endif // _gtTopShellXm_h
