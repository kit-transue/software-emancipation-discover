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
#ifndef _ToggleImport_h
#define _ToggleImport_h

// ToggleImport.h
//------------------------------------------
// synopsis:
// Dialog to ask user if the file should be imported before toggling read/write flag
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

extern "C" int popup_ToggleImport(char*, int);

class ToggleImport
{
  public:
    ToggleImport(gtBase*, const char*, int);
    ToggleImport(const char*, int);
    ~ToggleImport();

    int ask();

  private:
    static void Import_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Toggle_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static int answer_func(void*);

    gtDialogTemplate*   shell;
    int                 answer;
};


/*
    START-LOG-------------------------------

   $Log: ToggleImport.h  $
   Revision 1.1 1993/05/27 16:23:00EDT builder 
   made from unix file
// Revision 1.1  1993/04/01  12:57:07  kws
// Initial revision
//

    END-LOG---------------------------------
*/

#endif // _ToggleImport_h
