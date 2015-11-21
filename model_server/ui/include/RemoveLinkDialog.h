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
#ifndef _RemoveLinkDialog_h
#define _RemoveLinkDialog_h

// RemoveLinkDialog.h
//------------------------------------------
// synopsis:
// 
// A dialog box for prompting which type of hypertext link to remove
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif


class appTree;

class RemoveLinkDialog {
  public:
    RemoveLinkDialog(appTree*);
    void popup();
    void close();
    void doit();

  private:
    static void OK_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB (gtPushButton*, gtEventPtr, void*, gtReason);

    gtDialogTemplate*	shell;
    gtToggleBox*	options;
    appTree*		app_tree_ptr;
};

typedef RemoveLinkDialog* RemoveLinkDialogPtr;


/*
    START-LOG-------------------------------

   $Log: RemoveLinkDialog.h  $
   Revision 1.1 1993/05/27 16:22:54EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/01/08  16:05:48  glenn
 * Fix compilation errors due to altered include files.
 * Moved include files into this file from header file.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:07  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _RemoveLinkDialog_h
