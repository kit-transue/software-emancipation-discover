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
#ifndef _aboutDialog_h
#define _aboutDialog_h

// aboutDialog.h
//------------------------------------------
// synopsis:
// About Discover dialog
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

class aboutDialog
{
  public:
    static aboutDialog* instance();

    ~aboutDialog();

    void popup();

  private:
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    gtDialogTemplate* shell;

    aboutDialog();

    void build_interface();
};


/*
    START-LOG-------------------------------

    $Log: aboutDialog.h  $
    Revision 1.3 2000/04/04 10:02:16EDT sschmidt 
    Port to HP 10.  Merging from /main/hp10port to mainline.
 * Revision 1.2.1.3  1993/02/03  20:21:49  glenn
 * Add OK button.  Use gtDlgTemplate instead of gtTopLevelShell.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:17  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _aboutDialog_h
