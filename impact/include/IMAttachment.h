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
#ifndef __IM_ATTACHMENT_HEADER_FILE
#define __IM_ATTACHMENT_HEADER_FILE

#include "top_widgets.h"
#include "stl.h"

class CIMAttachmentCallbackHandler {
 public:
    virtual void ReadyToAttach(int nIsOK) {};
};

typedef list<CIMAttachmentCallbackHandler*> TAttachmentCallbacksList;
typedef TAttachmentCallbacksList::iterator TAttachmentCallbacksIterator;

class CIMAttachment {
  public :
    CIMAttachment();
    virtual ~CIMAttachment();
    bool AskForParameters(Widget parent);
    bool Attach(string szFile,Widget wParent);
    void AddCallback(CIMAttachmentCallbackHandler* pHandler);
    void RemoveCallback(CIMAttachmentCallbackHandler* pHandler);
    void FireCallback(int nIsOK);
 
 protected:
    static void OkPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
    static void CancelPressed(Widget dialog,XtPointer client_data, XtPointer call_data);
    static void CheckPassword(Widget text_w,XtPointer client_data,XtPointer call_data);
    Widget CreateParametersArea(Widget parent);

 public:
  
    Widget m_HostNameField;
    Widget m_PortField;
    Widget m_UserNameField;
    Widget m_PasswordField;
    Widget m_IssueIDField;

    static string m_szHostName;
    static string m_szPort;
    static string m_szUserName;
    static string m_szPassword;
    static string m_szIssueID;
    
    TAttachmentCallbacksList m_Handlers;
    Widget m_Dialog;
    int m_IsOK;
};
#endif // __IM_ATTACHMENT_HEADER_FILE

