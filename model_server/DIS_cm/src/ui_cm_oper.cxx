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
// ui_cm_oper.cxx - CM User Interface Operation Base Class.
//
// **********


#include "cLibraryFunctions.h"
#include "machdep.h"
#include "messages.h"
#include "iQuestion.h"

#include "../include/ui_cm_oper.h"
#include "projModule_of_symbol.h"



// Constructor.
ui_cm_oper::ui_cm_oper(const char* name) :
		name_(name), modules_(0),
		module_must_be_checked_out_(true),
		module_must_be_confirmed_(true),
		module_has_been_confirmed_(false),
		home_project_(0), current_module_(0)
{
}



// Destructor.
ui_cm_oper::~ui_cm_oper()
{
}


// Check all pre-conditions for the operation.
// 1. Make sure that the home project exists.
// 2. Make sure that something was selected.
// 3. Make sure that we can create an array of modules.
// 4. Optionally, make sure that the modules are checked out.
bool ui_cm_oper::CheckPreConditions(const symbolArr& file_syms)
{
	bool status = true;

	// 1. Make sure that the home project exists.
	home_project_ = projNode::get_home_proj();
	if (home_project_ == NULL) {
		dis_message(NULL, MSG_ERROR,
			"M_HOME_PROJECT_SELECT", (char*)name_);
		status = false;
	}

	// 2. Make sure that something was selected.
	if (file_syms.size() == 0) {
		dis_message(NULL, MSG_ERROR,
			"Nothing Selected '%s' CM Operation",
			(char*)name_);
		status = false;
	}
	if (status) {
		symbolPtr sym;
		ForEachS(sym, file_syms) {
			projModulePtr mod = NULL;
			if (sym.is_xrefSymbol() )
				mod = projModule_of_symbol(sym);
			else if (is_projModule(sym) )
				mod = projModulePtr(RelationalPtr(sym) );
			if (mod)
				modules_.insert_last(mod);
		}
		if (modules_.size() > 1)
			modules_.remove_dup_syms();
	}

	// 4. Optionally, make sure that the modules are checked out.
	if (status && module_must_be_checked_out_) {
		symbolArr unchecked_modules(modules_);
		symbolPtr sym;
		ForEachS(sym, unchecked_modules) {
			projModule* mod = projModulePtr(RelationalPtr(sym) );
			if (!mod) {
				dis_message(NULL, MSG_INFORM,
					"Invalid project module found.");
				modules_.remove(sym);
			} else if (!mod->is_home_project() ) {
				dis_message(NULL, MSG_INFORM,
					"Module '%s' is not in the home project.",
					realOSPATH(mod->get_name()) );
				modules_.remove(sym);
			}
		}
		// Make sure we didn't empty the modules_ array.
		status = (modules_.size() > 0);
	}

	return(status);
}



// Check any post-conditions for the operation.
bool ui_cm_oper::CheckPostConditions()
{
	bool status = true;

	if (failed_modules_.size() > 0)
		status = ProcessFailedModules(failed_modules_);

	return(status);
}



// See if there any any modules still needing to be processed.
bool ui_cm_oper::AnyModulesToProcess()
{
	bool status = true;

	current_module_ = NULL;

	if (modules_.size() == 0)
		status = false;

	if (status) {
		symbolPtr next = modules_[0];
		modules_.remove(next);
		if (is_projModule(next) )
			current_module_ = projModulePtr(RelationalPtr(next) );
		else {
			dis_message(NULL, MSG_INFORM, "Invalid project module found.");
			current_module_ = NULL;
			status = false;
		}
	}

	if (status) {
		if (current_module_ == NULL) 
			status = false;
	}

	return (status);
}



// Process the next module.
bool ui_cm_oper::ProcessNextModule()
{
	bool status = true;

	if (NeedsMoreInfo(current_module_) )
		GetMoreInfo(current_module_);

	// Perform the operation on the module.
	if (module_has_been_confirmed_) {
		if (OperateOnModule(current_module_) == false) {
			// The operation failed.
			failed_modules_.insert_last(current_module_);
			status = false;
		}
	}

	current_module_ = NULL;

	return(status);
}



//----------------------- Protected Methods ------------------------------//


// See if this operation needs to get more info.
bool ui_cm_oper::NeedsMoreInfo(projModulePtr)
{
	bool needs_info = module_must_be_confirmed_;

	module_has_been_confirmed_ = !module_must_be_confirmed_;

	return (needs_info);
}



// Do whatever to get more info.
bool ui_cm_oper::GetMoreInfo(projModulePtr mod)
{
	bool status = true;

	if (module_must_be_confirmed_) {
		genString msg;
		msg.printf("Are you sure you want to %s module '%s'?",
			(char*)name_, realOSPATH(mod->get_name()) );

		// Need to check for >0 because the
		// module was already removed.
		if (modules_.size() > 0) {
			int answer = dis_question3(NULL, name_,
					"Yes", "Yes To All", "No", msg);
			if (answer == 1)
				module_has_been_confirmed_ = true;
			else if (answer == 2) {
				module_must_be_confirmed_ = false;
				module_has_been_confirmed_ = true;
			}
		} else {
			int answer = dis_question(name_, "Yes", "No", msg);
			if (answer == 1)
				module_has_been_confirmed_ = true;
			else
				module_must_be_confirmed_ = false;
		}
	}
	return (status);
}



// Perform the CM operation on the module.
bool ui_cm_oper::OperateOnModule(projModulePtr)
{
	bool status = true;
	dis_message(NULL, MSG_ERROR, "CM Operation %s is not implemented.",
		(char*)name_);
	return (status);
}


// Report is called during the operation to report status/error information.
void ui_cm_oper::Report(const char* cmdName, const int status,
			const projNode* source, const projNode* dest,
			const symbolPtr& module)
{
	if (status) {
		dis_message(NULL, MSG_ERROR, "CM %s of '%s' FAILED with code %d",
			cmdName, module.get_name(), status);
	} else {
		dis_message(NULL, MSG_INFORM, "CM %s of '%s' succeeded.",
			cmdName, module.get_name() );
	}
}



// By default, we ignore all failed modules.
bool ui_cm_oper::ProcessFailedModules(const symbolArr& failed_modules)
{
	bool status = true;
	return(status);
}





/**********     end of ui_cm_oper.C     **********/

