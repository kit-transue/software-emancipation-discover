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
// This code is a -*- C++ -*- header file

#ifndef _sessionListUI_h
#define _sessionListUI_h

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _QualityCockpitUI_h
#include <QualityCockpitUI.h>
#endif

#ifndef CANT_FWD_DECL_NESTED
class QualityCockpitUI::sessionListUI
#else
class sessionListUI
#endif
{
public:

  // Constructor(s) / Destructor(s)
  sessionListUI(bool);
  ~sessionListUI(void);


  // Public Interface
  int getSession(genString const *, int, genString &);
  
private:
  
  static int  waitForSelection_CB(void *);
  static void OK_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void cancel_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void setSessionText_CB(gtList*, gtEventPtr, void *, gtReason);
  
private:
  
  int isSelectionSelected(void);
  
private:
  
  bool selection_made;
  bool cancelled;
  
  gtList           *session_list;
  gtDialogTemplate *dialog;
  gtStringEditor   *selection_txt;

};

#endif
// _sessionListUI_h

