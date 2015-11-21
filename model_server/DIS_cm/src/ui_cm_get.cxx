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
// **********
//
// ui_cm_get.cxx - CM Get User Interface Operation Class.
//
// **********


#include "cLibraryFunctions.h"
#include "machdep.h"
#include "messages.h"
#include "customize.h"
#include "prompt.h"

#include "../include/ui_cm_oper.h"
#include "../include/ui_cm_get.h"

// Needed to send cmds to client via DISui.
#include "../../DIS_ui/interface.h"
extern Application* DISui_app;



// Constructor.
ui_cm_get::ui_cm_get(const char* name) : ui_cm_oper(name), 
	needs_comment_string_(false), has_comment_string_(false),
	comment_string_(0)
{
	// Override settings from base class.

	// We don't want the module to be checked out before we check it out.
	module_must_be_checked_out_ = false;

	// We don't need to confirm each module before checking it out.
	module_must_be_confirmed_ = false;
	module_has_been_confirmed_ = true;
}



// Destructor.
ui_cm_get::~ui_cm_get()
{
}


// Check pre conditions.
bool ui_cm_get::CheckPreConditions(const symbolArr& file_syms)
{
	bool status = ui_cm_oper::CheckPreConditions(file_syms);

	if (status) {
		// See if the user want to specify a comment on get.
		needs_comment_string_ = customize::configurator_get_comments();
	}

	return (status);
}




//----------------------- Protected Methods ------------------------------//


// See if the get operation needs to a comment.
bool ui_cm_get::NeedsMoreInfo(projModulePtr mod)
{
	bool needs_info = needs_comment_string_ && !has_comment_string_;
	if (needs_info) module_has_been_confirmed_ = false;
	return (needs_info);
}



// Do whatever to get more info.
bool ui_cm_get::GetMoreInfo(projModulePtr mod)
{
	bool status = true;

	if (needs_comment_string_) {
		genString prompt = "Please enter a brief explanation of why you are checking out\n";
		prompt += realOSPATH(mod->get_name());

		// If any modules are left in the module array,
		// Give the user the option to apply this comment
		// to all remaining modules.
		if (modules_.size() > 0 ) 
			status = dis_prompt_string("CM Check Out", "OK",
				"OK For All", "Cancel", prompt,
				comment_string_);
		else
			status = dis_prompt_string("CM Check Out", "OK",
				"Cancel", prompt, comment_string_);

		if (status == 1) { // OK button selected.
			module_has_been_confirmed_ = true;
		} else if (status == 2) { // OK-For-All button selected.
			has_comment_string_ = true;
			module_has_been_confirmed_ = true;
		}
	}

	return (status);
}



// Perform the CM Get operation on the module.
bool ui_cm_get::OperateOnModule(projModule* mod)
{
	bool status = true;

	dis_message(NULL, MSG_INFORM,
		"Checking out '%s' ...", mod->get_name() );

	symbolPtr sym = mod;
	if (needs_comment_string_) {
		projModule* gotten = projModule::get_module_with_data(sym, comment_string_);
		if (gotten == NULL) status = false;
	} else {
		projModule* gotten = projModule::get_module(sym);
		if (gotten == NULL) status = false;
	}

	if (status)
		dis_message(NULL, MSG_INFORM,
			"%s check out completed.",
			mod->get_name() );
	else
		dis_message(NULL, MSG_INFORM,
			"Problem checking out '%s'.",
			mod->get_name() );

	return (status);
}





/**********     end of ui_cm_get.cxx     **********/

