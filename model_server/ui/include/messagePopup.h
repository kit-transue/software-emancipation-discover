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
#ifndef _messagePopup_h
#define _messagePopup_h

// messagePopup.h
//------------------------------------------
// synopsis:
// defines the classes and method prototypes for utilizing various
// types of messagePopups. The type of popups include plain, error,
// warning, and informational.
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif
#include <gtBase.h>

//
// The following functions provide a convenient way to popup a message box
// without having to instantiate a messagePopup object. In most cases the
// programmer should one of these functions. It may be advantageous to
// declare an object when more control over the widget is desired. For example,
// it may be desirable to remove the widget from the display under programmatic
// control rather than waiting for the user to do it.
//

void popup_Error(const char*, const char*);
void popup_Warning(const char*, const char*);
void popup_Information(const char*, const char*);
void popup_Working(const char*, const char*);

class messagePopup
{
  public:
    messagePopup(const char* title, const char* message,
                 gtMessageType type, gtBase* parent = NULL);
    ~messagePopup();

    // sets the current message for this messagePopup
    void set_message(const char* new_message);

    // popup the message
    void popup(const int mode = 0);

    // disappear the messagePopup
    void popdown();
    
    gtMessageDialog* get_shell(void) { return shell; };

  private:
    static void OK_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void callback_handler(void *data);
    static void destroy_callback(gtBase *, void *);

    gtMessageDialog* shell;

    gtMessageDialog* create_popup_of_type(
	gtMessageType type, const char* title, gtBase* parent = NULL);
};

#endif // _messagePopup_h
