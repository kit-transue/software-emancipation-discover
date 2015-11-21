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
//
// "FileSelDlg.C"
//
// File selection browser to be used with and from Access/sw.
// You may have to tweak this to make it behave properly outside
// of Access.
//

#include <FileSelDlg.h>
#include <gtFileSel.h>
#include <genError.h>

FileSelDlg::FileSelDlg(gtBase *parent, const char *title) : dlg(NULL), status(FILE_SEL_UNKNOWN)
{
  Initialize(FileSelDlg::FileSelDlg);

  buildInterface(parent, title);
}

FileSelDlg::~FileSelDlg()
{
  Initialize(FileSelDlg::~FileSelDlg);

  if (dlg)
    delete dlg;
}

void FileSelDlg::buildInterface(gtBase *parent, const char *title)
{
  Initialize(FileSelDlg::buildInterface);

  dlg = gtFileSelector::create(parent, title,
			       ok_CB, this,
			       cancel_CB, this);
  dlg->title(title);
  dlg->manage();
}

FileSelCode FileSelDlg::WaitFileName(genString &name)
{
  Initialize(FileSelDlg::WaitFileName);

  //dlg->popup(3);
  dlg->take_control(&FileSelDlg::take_control_CB, this);
  //dlg->popdown();

  if (status == FILE_SEL_OK)
    name = filename;

  return status;
}

int FileSelDlg::take_control_CB(void *cd)
{
  Initialize(FileSelDlg::take_control_CB);

  FileSelDlg *fsd = (FileSelDlg *)cd;
 
  char *sel = fsd->dlg->selected();
  fsd->filename = sel;
  gtFree(sel);
  
  return (int)(fsd->status);
}

void FileSelDlg::ok_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
  Initialize(FileSelDlg::ok_CB);

  FileSelDlg *fsd = (FileSelDlg *)cd;

  fsd->status = FILE_SEL_OK;
  //fsd->dlg->popdown();
}

void FileSelDlg::cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
  Initialize(FileSelDlg::cancel_CB);

  FileSelDlg *fsd = (FileSelDlg *)cd;

  fsd->status = FILE_SEL_CANCEL;
  //fsd->dlg->popdown();
}
