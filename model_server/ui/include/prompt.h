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
#ifndef _prompt_h
#define _prompt_h

#include <general.h>	// for 'bool'.

class genString;

int dis_prompt (const char* title, const char* prompt,
		const char* ok1, const char* ok2, const char* cancel,
		const char* help, genString &answer);

bool dis_prompt_string(const char* title, const char* ok_label, 
		const char* prompt, genString& answer);

int dis_prompt_string(const char* title,
		const char* ok_button, const char *cancel_button,
		const char* prompt, genString& answer);

int dis_prompt_string(const char* title, const char* ok_button,
		const char *ok_all_button, const char* cancel_button,
		const char* prompt, genString& answer);


#define B_OK			"OK"
#define B_CANCEL		"Cancel"
#define B_CHANGEREMAINING 	"Change Remaining"

#define P_INSERTVALUES		"Insert Values"
#define P_TRYAGAIN		"Try again"
#define P_OUTPUTREPORT		"Output Session Report To"
#define P_SAVELOGOUTPUT		"Save the log output in file:"

#define PB_OK			"OK"
#define PB_CANCEL		"Cancel"

#define PT_CHANGEPROP           "Change Propagation"
#define PT_SAVELOG		"Save Log"
#define PT_INSERTVALUES		"Insert Values"
#define PT_MESSAGEFORUNLOCK	"Message for Unlock"

#define PC_CHANGEPROP		"Change_Propagation"
#define PC_MSGLOGGER		"Message_Logger"
#define PC_IMPACTCHANGEARGSVAL	"Pset.Help.ImpactAnalyzer.ChangeArgs.Value"
#define PC_UNLOCKERRORMESSAGE	"Browser.Manage.Unlock.EnterMessage"


#endif
