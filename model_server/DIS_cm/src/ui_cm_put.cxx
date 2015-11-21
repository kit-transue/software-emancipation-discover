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
#ifdef _WIN32
// **********
//
// ui_cm_put.cxx - CM Put User Interface Operation Class.
//
// **********


#include "cLibraryFunctions.h"
#include "machdep.h"
#include "messages.h"
#include "customize.h"
#include "prompt.h"

#include "../include/ui_cm_oper.h"
#include "../include/ui_cm_put.h"

#ifndef vportINCLUDED
#include <vport.h>
#endif
#include vstrHEADER

#include "gRTListServer.h"

// Needed to send cmds to client via DISui.
#include "../../DIS_ui/interface.h"
extern Application* DISui_app;


// This should be in proj.h but I don't want to change it now.
extern void put_psets(objArr& modules);


// Class static info.
bool ui_cm_put::PutAlreadyInProgress = false;
symbolArr ui_cm_put::FailedModules;
symbolArr ui_cm_put::LockedFailedModules;
symbolArr ui_cm_put::MergeFailedModules;
RTListServer* ui_cm_put::HomeProjectRTL = NULL;



// Constructor.
ui_cm_put::ui_cm_put(const char* name, RTListServer* homeRTL) :
	ui_cm_oper(name), 
	needs_comment_string_(false), has_comment_string_(false),
	comment_string_(0)
{
	// Override settings from base class.

	// We don't want the module to be checked out before we check it out.
	module_must_be_checked_out_ = false;

	// We don't need to confirm each module before checking it out.
	module_must_be_confirmed_ = false;
	module_has_been_confirmed_ = true;


	// Make sure only one put operation is allowed.
	if (PutAlreadyInProgress) {
		dis_message(NULL, MSG_ERROR,
		"Internal Error: Attempted to start second put operation");
	} else {
		// Note - we don't set PutAlreadyInProgress unless a failure occurs.
		FailedModules.removeAll();
		LockedFailedModules.removeAll();
		MergeFailedModules.removeAll();
		HomeProjectRTL = homeRTL;
	}
}



// Destructor.
ui_cm_put::~ui_cm_put()
{
}


// Check pre conditions.
// Put will need a comment string.
bool ui_cm_put::CheckPreConditions(const symbolArr& file_syms)
{
	bool status = ui_cm_oper::CheckPreConditions(file_syms);

	if (status)
		needs_comment_string_ = true;

	return (status);
}





//----------------------- Protected Methods ------------------------------//


// See if this module needs to ask for a comment. 
bool ui_cm_put::NeedsMoreInfo(projModulePtr mod)
{
	bool needs_info = needs_comment_string_ && !has_comment_string_;
	if (needs_info) module_has_been_confirmed_ = false;
	return (needs_info);
}



// Do whatever to get more info.
bool ui_cm_put::GetMoreInfo(projModulePtr mod)
{
	bool status = true;

	if (needs_comment_string_) {
		// If the module already has a comment, use it.
		mod->get_module_comment(comment_string_);

		// Get the message to display in the prompt.
		genString prompt = "Please enter a brief explanation of why you are checking in\n";
		prompt += realOSPATH(mod->get_name());

		// If any modules are left if the global array, 
		// allow the user to apply this comment to all
		// remaining files.
		if (modules_.size() > 0)
			status = dis_prompt_string("CM Check In", "OK",
				"OK For All", "Cancel", prompt,
				comment_string_);
		else
			status = dis_prompt_string("CM Check In", "OK", 
				"Cancel", prompt, comment_string_);
		if ( status == 1) {	// OK Button was selected.
			module_has_been_confirmed_ = true;
		} else if ( status == 2) { 	// OK-For-All Button was selected.
			// Use this comment for any remaining files.
			has_comment_string_ = true;  
			module_has_been_confirmed_ = true;
		}
	}

	return (status);
}


int dis_cm_event_put_internal( projModule *mod, const char* comment );

// Perform the CM Put operation on the module.
bool ui_cm_put::OperateOnModule(projModule* mod)
{
	bool status = true;

	dis_message(NULL, MSG_INFORM,
		"Checking in '%s' ...", mod->get_name() );

	status = mod->cfg_fast_put(mod->get_target_project(0),
			"", comment_string_);

	if (status) {
		// Still need to check in the pset file.
		dis_cm_event_put_internal( mod, comment_string_ );
	  
		dis_message(NULL, MSG_INFORM,
			"%s check in completed.", mod->get_name() );
	} else {
		dis_message(NULL, MSG_INFORM,
			"Problem checking in '%s'.", mod->get_name() );
	}
	return (status);
}


// Tell user about any failed modules.
bool ui_cm_put::ProcessFailedModules(const symbolArr& failures)
{
	Initialize(ProcessFailedModules);

	// If there are any failurers, we need to set the In-Progress flag.
	if (failures.size() > 0) PutAlreadyInProgress = true;

	bool status = true;

	// Fill in the failure arrays.
	symbolPtr sym;
	ForEachS(sym, failures) {
		projModule* mod = checked_cast(projModule, sym);
		if (mod) {
			FailedModules.insert_last(mod);
			genString version;
			if (mod->is_locked() ) {
				LockedFailedModules.insert_last(mod);
			} else if (mod->need_3file_merge(version,
					mod->get_target_project(0) ) ) {
				MergeFailedModules.insert_last(mod);
			}
		}
	}

	// Send failure info over to the client/user.
	genString command;
	if (failures.size() > 0) {
		command.printf("dis_launch_dialog CMPutStatus");
		rcall_dis_DISui_eval_async(DISui_app, (vstr*)command.str() );
	}

	return (status);
}



//Print the reason for the failure using dis_message.
int ui_cm_put::GetFailureType(const symbolArr& file_syms,
				genString& failureType) 
{
	int status = file_syms.size();
	// We will return info only on the first seleted symbol.
	if (status) {
		symbolPtr sym = file_syms[0];
		if (LockedFailedModules.includes(sym) ) {
			failureType.printf(
				"%s is locked by another user", 
				sym.get_name() );
		} else if (MergeFailedModules.includes(sym) ) {
			failureType.printf(
				"%s has been modified and needs to be merged", 
				sym.get_name() );
		} else {
			failureType.printf(
				"%s encountered an internal error during check in", 
				sym.get_name() );
		}
	}

	return(status);
}





/**********     end of ui_cm_put.cxx     **********/
#endif // _WIN32
