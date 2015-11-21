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
#ifndef _FileSelDlg_h
#define _FileSelDlg_h

//
// "FileSelDlg.h"
//
// File selection browser to be used with and from Access/sw.
// You may have to tweak this to make it behave properly outside
// of Access.
//

#include <gtFileSel.h>
#include <genString.h>

typedef enum {
  FILE_SEL_UNKNOWN,
  FILE_SEL_OK,
  FILE_SEL_CANCEL
} FileSelCode;

class FileSelDlg
{
  public:

    FileSelDlg(gtBase *, const char *);
    ~FileSelDlg();

    FileSelCode WaitFileName(genString &);

  private:
    
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static int take_control_CB(void*);

    void buildInterface(gtBase*, const char*);
    void close(FileSelCode);

    gtFileSelector *dlg;
    FileSelCode status;
    genString filename;
};

#endif /* ! _FileSelDlg_h */
