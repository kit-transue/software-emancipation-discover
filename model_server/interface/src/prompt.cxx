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
// prompt.C - UI prompt utility routines.
//	This module contains routines which prompt the user for input.
//	In the aset/OLD_UI style, they usually instantiate a Prompt class
// object to do the work.
//	In the gala/NEW_UI style, they usually send a command to the gala
// 'client' to do the work.



#if defined(NEW_UI)
#include <vport.h>
#include vstrHEADER
#include vwindowHEADER

#include <../../DIS_ui/interface.h>
extern Application* DISui_app;

#include <galaxy_undefs.h>

#else
#include <NewPrompt.h>
#endif

#include <cLibraryFunctions.h>
#include <machdep.h>
#include <prompt.h>
#include <messages.h>
#include <genString.h>

int dis_prompt (const char* title, 
                const char* prompt, 
                const char* ok1, 
                const char* ok2, 
                const char* cancel, 
                const char* help, 
                genString &answer)
{

// We need to replace where this is called with a call to dis_prompt_string.
	bool status = true;

	if (ok2)
		status = dis_prompt_string(title, ok1, ok2, cancel, prompt, answer);
	else
		status = dis_prompt_string(title, ok1, cancel, prompt, answer);
	return(status);
}


bool dis_prompt_string( const char* title, const char* OK_button,
			const char* prompt, genString& answer)
{
	bool status = true;

#if defined(NEW_UI)
	const char* prompt_title = "DISCOVER";
	if (title && *title) prompt_title = title;

	const char* OK_title = "OK";
	if (OK_button && *OK_button) OK_title = OK_button;

	const char* initial_value = "";
	if (answer.length() > 0) initial_value = answer;

	genString command;
	command.printf("dis_prompt_string1 {%s} {%s} {%s} {%s}",
			prompt_title, OK_title, prompt, initial_value);
	vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());

	answer = (char*)(vchar*)results;

	vstrDestroy (results);


#else
	Prompt myprompt (NULL, title, prompt, "", NULL);
	return myprompt.ask(answer);
#endif

	return (status);
}



// This has the same return values as the 2-Button question:
//	Cancel == 0
//	OK == 1
//
int dis_prompt_string( const char* title,
			const char* OK_button, const char* cancel_button,
			const char* prompt, genString& answer)
{
	int status = 0;

#if defined(NEW_UI)
	const char* prompt_title = "DISCOVER";
	if (title && *title) prompt_title = title;

	const char* OK_title = "OK";
	if (OK_button && *OK_button) OK_title = OK_button;

	const char* cancel_title = "Cancel";
	if (cancel_button && *cancel_button) cancel_title = cancel_button;

	const char* initial_value = "";
	if (answer.length() > 0) initial_value = answer;

	genString command;
	command.printf("dis_prompt_string2 {%s} {%s} {%s} {%s} {%s}",
			prompt_title, OK_title, cancel_title,
			prompt, initial_value);
	vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());

	// The first byte is the return code.
	// The remainder is an optional space and the the optional answer.
	char* answerText = (char*)(vchar*) results;
	int button_selected = 1;   // Default is the cancel button.

	// Read the button number.
	//	0 == OK, 1 == Cancel.
	if (*answerText) button_selected = OSapi_atoi(answerText++);

	// We only care about the remainder if the OK button was selected.
	if (button_selected == 0) {
		// Set the return status.
		status = 1;

		// Skip the space.
		if (*answerText) answerText++;
		answer = answerText;
	} else 
		status = 0;

	vstrDestroy (results);

#else
	Prompt myprompt (NULL, title, prompt, "", NULL);
	return myprompt.ask(answer);
#endif

	return (status);
}


// This has the same return values as the 3-Button question:
//	Cancel	== 0
//	OK	== 1
//	OK_All	== 2
//
int dis_prompt_string( const char* title, const char* OK_button,
			const char* OK_All_button, const char* cancel_button,
			const char* prompt, genString& answer)
{
	int status = 0;

#if defined(NEW_UI)
	const char* prompt_title = "DISCOVER";
	if (title && *title) prompt_title = title;

	const char* OK_title = "OK";
	if (OK_button && *OK_button) OK_title = OK_button;

	const char* OK_All_title = "OK For All";
	if (OK_All_button && *OK_All_button) OK_All_title = OK_All_button;

	const char* cancel_title = "Cancel";
	if (cancel_button && *cancel_button) cancel_title = cancel_button;

	const char* initial_value = "";
	if (answer.length() > 0) initial_value = answer;

	genString command;
	command.printf("dis_prompt_string3 {%s} {%s} {%s} {%s} {%s} {%s}",
			prompt_title, OK_title, OK_All_title, cancel_title,
			prompt, initial_value);
	vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());

	// The first byte is the return code.
	// The remainder is an optional space and the the optional answer.
	char* answerText = (char*)(vchar*) results;
	int button_selected = 1;   // Default is the cancel button.

	// Read the button number.
	//	0 == OK, 1 == OK_For_All, 2 == Cancel.
	if (*answerText) button_selected = OSapi_atoi(answerText++);

	// We only care about the remainder if the an OK button was selected.
	if (button_selected == 0) {
		// Set the return status.
		status = 1;

		// Skip the space.
		if (*answerText) answerText++;
		answer = answerText;
	} else if (button_selected == 1) {
		// Set the return status.
		status = 2;

		// Skip the space.
		if (*answerText) answerText++;
		answer = answerText;
	} else 
		status = 0;

	vstrDestroy (results);


#else
	Prompt myprompt (NULL, title, prompt, "", OK_All_button);
	return myprompt.ask(answer);
#endif

	return (status);
}


int dis_prompt_string_OK_cancel( const char *title, const char *prompt_str,
                                 const char *err_msg, genString &result )
{
  int nRet = -1;
#ifdef NEW_UI
  Application *appl = Application::findApplication("DISui");
  if( appl && appl->mySession())
  {
    vstr *string = NULL;
    nRet = rcall_dis_display_string_OK_cancel( appl, (char *)title, 
                            (char *)prompt_str, (char *)err_msg, string );
    if( nRet > 0 && string )
    {
      result = (char *)string;
      vstrDestroy( string );
    }
  }
#else
  Prompt prompt(NULL, (char *)title, (char *)prompt_str, (char *)err_msg, NULL);
  nRet = prompt.ask(result);
#endif
  return nRet;
}
                                  
