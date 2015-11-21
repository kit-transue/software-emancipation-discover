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
// messagePopup.C
//------------------------------------------
// synopsis:
// implements all methods for the various messagePopups.
// for important information regarding the best ways to use
// messagePopups please consult the header file messagePopup.h
//
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <genError.h>
#include <messagePopup.h>
#include <top_widgets.h>
#include <genArr.h>
#include <gtMessageDlg.h>
#include <gtPushButton.h>
#include <gtNoParent.h>

#define TXT(a) a

typedef messagePopup *messagePopupPtr;

genArr(messagePopupPtr);
genArrOf(messagePopupPtr) top_messages;
genArrOf(messagePopupPtr) already_destroyed;

// FUNCTION DEFINITIONS

messagePopup::messagePopup(
    const char* title, const char* message, gtMessageType type, gtBase* parent)
{
    Initialize(messagePopup::messagePopup);

    // create a popup of the type requested (defend against a bad title pointer)
    if (title == NULL)
	title = TXT("Message");
#ifndef NEW_UI
    shell = create_popup_of_type(type, title, parent);

    // cancel buttons make no sense for message popups
    shell->cancel_button()->unmanage();

    // the okay button will disappear the widget via the OK_CB callback
    shell->ok_button()->set_callback(OK_CB, this);

    // and finally install the message text
    set_message(message);
#endif
}


messagePopup::~messagePopup()
{
    Initialize(messagePopup::~messagePopup);

#ifndef NEW_UI
    delete shell;		// release the widget
#endif
}


void messagePopup::set_message(const char* new_message)
//
// sets the current message for this messagePopup
//
{
    Initialize(messagePopup::set_message);

    if (new_message == NULL)	// defend against a bad pointer
	new_message = "???";
#ifndef NEW_UI
    shell->message(new_message);   
#endif
}

void bring_error_messages_on_top(void)
//
// Scan array of messages and popup last one
//    
{
    Initialize(bring_error_messages_on_top);
#ifndef NEW_UI 
    int i = top_messages.size() - 1;
    if(i >= 0){
	messagePopup *mp = *top_messages[i];
	mp->get_shell()->bring_to_top();
    }
/*    
    for(int i = 0; i < top_messages.size(); i++){
	messagePopup *mp = *top_messages[i];
	mp->get_shell()->bring_to_top();
    }
    */    
#endif
}

void messagePopup::callback_handler(void *data)
//
// Bring message on top
//    
{
    Initialize(messagePopup::callback_handler);
   
#ifndef NEW_UI 
    // check that message is not destroyed yet
    for(int i = 0; i < already_destroyed.size(); i++)
	if(*already_destroyed[i] == data){
	    *already_destroyed[i] = NULL;
	    return;
	}
    messagePopup *mp = (messagePopup *)data;
    mp->shell->bring_to_top();
    // Add myself to array of messages (do not do it before this callback! because it will flush to many)
    top_messages.append(&mp);
#endif
}

void messagePopup::destroy_callback(gtBase *, void *data)
//
// Remove itself from array of messages
//    
{
    Initialize(messagePopup::destroy_callback);
   
#ifndef NEW_UI 
    messagePopup *mp = (messagePopup *)data;
    int i;
    for(i = 0; i < top_messages.size(); i++){
	messagePopup *message = *top_messages[i];
	if(message == mp){
	    top_messages.remove(i);
	    return;
	}
    }    
    // message is not in top list yet, but already got destroyed,  (this may happen)
    // do not try to bring it up later.
    for(i = 0; i < already_destroyed.size(); i++)
	if(*already_destroyed[i] == NULL){
	    *already_destroyed[i] = mp;
	    return;
	}
    already_destroyed.append(&mp);  
#endif
}

void messagePopup::popup(const int mode)
//
// popup the message 
//
{
    Initialize(messagePopup::popup);
#ifndef NEW_UI
    shell->user_destroy(destroy_callback, this);
    shell->popup(mode);
    pset_send_own_callback(callback_handler, this); // To insure that message will be brougth to top.
#endif
}


void messagePopup::popdown()
//
// disappear the messagePopup
//
{
    Initialize(messagePopup::popdown);
#ifndef NEW_UI
    shell->popdown();
#endif
}


gtMessageDialog* messagePopup::create_popup_of_type(
    gtMessageType type, const char* title, gtBase* parent)
//
// Return a pointer to a popup of the appropriate type being requested.
//
{
    Initialize(messagePopup::create_popup_of_type);
#ifndef NEW_UI
    if(parent == NULL)
	parent = gtNoParent::app_shell();
    switch (type)	// dispatch to the creation statement based on type
    {
      case gtMsgError:
	return gtMessageDialog::create(
	    parent, "error_message", title, gtMsgError);

      case gtMsgInformation:
	return gtMessageDialog::create(
	    parent, "information_message", title, gtMsgInformation);

      case gtMsgPlain:
      case gtMsgQuestion:
	return gtMessageDialog::create(
	    parent, "plain_message", title, gtMsgPlain);

      case gtMsgWarning:
	return gtMessageDialog::create(
	    parent, "warning_message", title, gtMsgWarning);

      case gtMsgWorking:
	return gtMessageDialog::create(
	    parent, "working_message", title, gtMsgWorking);
    }
#endif
    return NULL;   // just to keep -pedantic happy
}


////////////////////////////////////////////////////////////

void messagePopup::OK_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(messagePopup::OK_CB);
#ifndef NEW_UI
    delete (messagePopup*)cd;
#endif
}

////////////////////////////////////////////////////////////

// These function provide an easy interface to the various types of
// message popups. It protects the programmer from having to declare
// and manage the popup objects. He/She can simply call one of these routines.
// The OK button callback will take care of deleting the widget.

void popup_Error(const char* title, const char* message)
{
    Initialize(popup_Error);
#ifndef NEW_UI
    messagePopup* msg = new messagePopup(title, message, gtMsgError, NULL);
    msg->popup(1);
#endif
}

void popup_Warning(const char* title, const char* message)
{
    Initialize(popup_Warning);
#ifndef NEW_UI
    messagePopup* msg = new messagePopup(title, message, gtMsgWarning, NULL);
    msg->popup(1);
#endif
}


void popup_Information(const char* title, const char* message)
{
    Initialize(popup_Information);
#ifndef NEW_UI
    messagePopup* msg =
	new messagePopup(title, message, gtMsgInformation, NULL);
    msg->popup(1);
#endif
}


void popup_Working(const char* title, const char* message)
{
    Initialize(popup_Working);
#ifndef NEW_UI
    messagePopup* msg = new messagePopup(title, message, gtMsgWorking, NULL);
    msg->popup(1);
#endif
}


/*
   START-LOG-------------------------------------------

   $Log: messagePopup.h.C  $
   Revision 1.6 2000/07/07 08:17:39EDT sschmidt 
   Port to SUNpro 5 compiler
// Revision 1.4  1993/04/29  16:59:10  glenn
// Remove no-op sub-classes of messagePopup.
// Rewrite popup_* functions to create messagePopup directly.
//
// Revision 1.3  1993/04/23  17:28:25  davea
// bug 3482 - added return stmt for buildxref to be happy
//
// Revision 1.2  1993/04/14  15:36:30  glenn
// Avoid creating six (!) gtMessageDialogs in create_popup_of_type
// due to missing "break" statements in a "switch".
//
// Revision 1.1  1993/01/26  19:51:05  sharris
// Initial revision
//
   END-LOG---------------------------------------------
*/

