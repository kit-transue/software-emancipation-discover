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
// This is a -*- C++ -*- header file

#ifndef _QC_ReportUI_h
#define _QC_ReportUI_h

#ifndef _gt_h
#include <gt.h>
#endif

#include "IMAttachment.h"

class genString;

class QC_ReportUI : public CIMAttachmentCallbackHandler
{
public:

  // Interface
  static void Invoke(const char *title, gtBase* pParent=NULL);
  static void Quit(void);
  ~QC_ReportUI();
  
private:
  
  QC_ReportUI(char const * = 0,gtBase* pParent=NULL);

private:

  enum selection
    {
      OK,
      CANCEL
    };

private:

  // INITIALIZATION FUNCTIONS
  void         buildInterface(char const *);

  // CALLBACKS 
  static void reportType_CB(gtPushButton *, gtEventPtr, void *, gtReason);
  static void browse_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void browse_OK_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void browse_cancel_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void run_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void cancel_CB(gtPushButton*, gtEventPtr, void *, gtReason);

  // UI API
  int  browseForFilename(void);
  int  handleFilenameSelection(selection);
  int  runReport(void);
  bool ProcessReportName(genString* filename,genString& type);

  // ACCESS API
  int  getReportTypes(genString *&);
  int  runReport(genString const &, genString const &, bool);
  int  debuggingAccess(void);

  // DIRECT API TO ACCESS
  int  sendCmd(genString, genString &);
  void notifyTclError(void);

public:
    // IMAttachmentCallback handler
    void ReadyToAttach(int nIsOK);
    CIMAttachment* m_pAttachment;

private:

  static QC_ReportUI        *instance; 
  int                        invoked;
  int                        debug_access;
  genString                  cur_report_type;
  genString                 *report_types;

private:

  // widgets we need to keep track of

  gtBase           *parent;
  gtDialogTemplate *dialog;
  gtToggleButton   *verbosity;
  gtToggleButton   *attachToIM;
  gtStringEditor   *filename_txt;
  gtFileSelector   *filebrowser;

};

#endif /* ! _QC_ReportUI_h */
