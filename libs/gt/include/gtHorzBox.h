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
#ifndef _gtHorzBox_h
#define _gtHorzBox_h

// gtHorzBox.h
//------------------------------------------
// synopsis:
// Manager suitable for aligning children horizontally.
//------------------------------------------

#ifndef _gtManager_h
#include <gtManager.h>
#endif


class gtHorzBox : public gtManager
{
  public:
    static gtHorzBox* create(gtBase* parent, const char* name);

    ~gtHorzBox();

    virtual void margins(int h, int v) = 0;
    virtual void packing(gtPacking) = 0;
    virtual void rows(int) = 0;

  protected:
    gtHorzBox();
};


/*
   START-LOG-------------------------------------------

   $Log: gtHorzBox.h  $
   Revision 1.1 1993/02/24 13:29:26EST builder 
   made from unix file
 * Revision 1.2.1.3  1992/12/30  19:08:54  glenn
 * Add margins, packing, rows.
 *
 * Revision 1.2.1.2  1992/10/09  18:10:10  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:37  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:36  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:50  smit
 * Initial revision
 * 
   END-LOG---------------------------------------------
*/

#endif // _gtHorzBox_h
