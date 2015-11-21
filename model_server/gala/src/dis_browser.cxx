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
/*********
*
* dis_browser.cxx - dis/tcl commands for generic browser layers.
*
* dis_brower_get_askList - Get the Browser ask list contents.
* dis_brower_get_categoriesList - Get the Browser categories list contents.
*
**********/

#include <vport.h>
#include vcharHEADER

#include <dis_browser.h>
#include <gglobalFuncs.h>
#include <grtlClient.h>
#include <SelectionIterator.h>

#include <Application.h>
#include "../../DIS_main/interface.h"

#include <galaxy_undefs.h>

#include <machdep.h>


// AskList contents.
static gString AskList;

// Categories List content
static gString CategoriesList;


//-----------------------------------------------------------------------------
// dis_browser_get_askList 
int dis_browser_get_askList (ClientData, Tcl_Interp* interp, int , char*[])
{
	gInit (dis_browser_get_askList);
	int status = TCL_OK;

	gString OldAskList = AskList;

	if (AskList.length() == 0 ) {
		AskList = rcall_server_eval(
				Application::findApplication("DISmain"),
				(vstr*) "DISmain", 
				(vstr*) "dis_browser_get -askList");
	}
	Tcl_SetResult(interp, (char*)AskList, TCL_VOLATILE);

	int NeedUpdateAsk = AskList != OldAskList;
	gString NeedUpdateStr;
	NeedUpdateStr.sprintf((vchar*)"%d", NeedUpdateAsk);
	Tcl_SetVar(interp, "BROWSER_NeedUpdateAsk", (char*)(vchar*)NeedUpdateStr, TCL_GLOBAL_ONLY);

	return (status);
}


//-----------------------------------------------------------------------------
// dis_browser_get_categoriesList
int dis_browser_get_categoriesList (ClientData, Tcl_Interp* interp, int , char*[])
{
	gInit (dis_browser_get_categoriesList);
	int status = TCL_OK;

	if (CategoriesList.length() == 0 ) {
		CategoriesList = rcall_server_eval(
				Application::findApplication("DISmain"),
				(vstr*) "DISmain", 
				(vstr*) "dis_browser_get -categoriesList");
	}
	Tcl_SetResult(interp, (char*)CategoriesList, TCL_VOLATILE);

	return (status);
}

int dis_browser_update_categoriesList (ClientData, Tcl_Interp* interp, int, char**)
{
	gInit(dis_browser_update_categoriesList);
	int status = TCL_OK;
	
	CategoriesList = rcall_server_eval(
		Application::findApplication("DISmain"),
		(vstr*) "DISmain",
		(vstr*) "dis_browser_get -categoriesList");

	Tcl_SetResult(interp, (char*)CategoriesList, TCL_VOLATILE);
	return status;
}

//-----------------------------------------------------------------------------
// dis_browser_update_askList_for_rtl rtlClientId
int dis_browser_update_askList_for_rtl (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	gInit (dis_browser_update_askList_for_rtl);
	int status = TCL_OK;

	gString OldAskList = AskList;

	if (argc == 2) {
		gString cmd;
		// Make sure that the arg is a valid rtl Id.
		const char* rtl = argv[1];
		if (isdigit(*rtl) ) {
			int rtlId = OSapi_atoi(rtl);
			rtlClient* clientRTL = rtlClient::find(rtlId);
			cmd.sprintf((vchar*)"dis_browser_get -askListForRTL %d", 
				clientRTL->getServerId() );
			AskList = rcall_server_eval(
					Application::findApplication("DISmain"),
					(vstr*) "DISmain", 
					(vstr*) (vchar*) cmd);
		} else if (interp_errTrace) {
			dis_message((vchar*)"'%s' is not a valid rtl ID.",
				rtl);
		}
		Tcl_SetResult(interp, (char*)AskList, TCL_VOLATILE);
	} else if (argc == 1) {
		AskList = rcall_server_eval(
				Application::findApplication("DISmain"),
				(vstr*) "DISmain", 
				(vstr*) "dis_browser_get -askList");
		Tcl_SetResult(interp, (char*)AskList, TCL_VOLATILE);
	} else if (interp_errTrace) {
		dis_message((vchar*)"dis_browser_update_askList_for_rtl needs an RTL id.");
	}

	int NeedUpdateAsk = AskList != OldAskList;
	gString NeedUpdateStr;
	NeedUpdateStr.sprintf((vchar*)"%d", NeedUpdateAsk);
	Tcl_SetVar(interp, "BROWSER_NeedUpdateAsk", (char*)(vchar*)NeedUpdateStr, TCL_GLOBAL_ONLY);

	return (status);
}



//-----------------------------------------------------------------------------
// dis_browser_update_askList_for_categories categories_selection
int dis_browser_update_askList_for_categories (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	gInit (dis_browser_update_askList_for_categories);
	int status = TCL_OK;

	gString OldAskList = AskList;

	if (argc == 2) {
		gString cmd;
		// Make sure that the arg is a valid rtl Id.
		const char* selection = argv[1];
		cmd.sprintf((vchar*)"dis_browser_get -askListForCategories %s", 
			selection);
		AskList = rcall_server_eval(
				Application::findApplication("DISmain"),
				(vstr*) "DISmain", 
				(vstr*) (vchar*) cmd);
		Tcl_SetResult(interp, (char*)AskList, TCL_VOLATILE);
	} else if (argc == 1) {
		AskList = rcall_server_eval(
				Application::findApplication("DISmain"),
				(vstr*) "DISmain", 
				(vstr*) "dis_browser_get -askList");
		Tcl_SetResult(interp, (char*)AskList, TCL_VOLATILE);
	} else if (interp_errTrace) {
		dis_message((vchar*)"dis_browser_update_askList_for_categories needs a selection.");
	}

	int NeedUpdateAsk = AskList != OldAskList;
	gString NeedUpdateStr;
	NeedUpdateStr.sprintf((vchar*)"%d", NeedUpdateAsk);
	Tcl_SetVar(interp, "BROWSER_NeedUpdateAsk", (char*)(vchar*)NeedUpdateStr, TCL_GLOBAL_ONLY);

	return (status);
}

static int convert_selection(Tcl_Interp* interp, int argc, char* argv[], 
						 const gString& NameList)
{
	gInit (convert_selection);
	int status = TCL_OK;

	if (argc == 2) {
		// Loop through the selection,
		// Convert indices from the current AskList to the FullAskList.
		SelectionIterator oldSel(argv[1]);
		gString newSel;
		while( oldSel.Next() ) {
			int Index = oldSel.GetNext();
			// Get the name for this index.
			gString cmd;
			cmd.sprintf((vchar*)"lindex {%s} %d", NameList, Index);
			gala_eval(interp, cmd);
			gString Name = Tcl_GetStringResult(interp);
		
			newSel += "{ ";
			newSel += Name;
			newSel += " } ";
		}
		Tcl_SetResult(interp, newSel, TCL_VOLATILE);
	} else if (interp_errTrace) {
		dis_message((vchar*)"convert_selection needs a selection.");
	}

	return (status);
}
//-----------------------------------------------------------------------------
// dis_browser_convert_ask_selection categories_selection
int dis_browser_convert_ask_selection (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	return convert_selection(interp, argc, argv, AskList);
}


int dis_browser_convert_category_selection (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
	return convert_selection(interp, argc, argv, CategoriesList);
}


//          End of dis_browser.cxx          //
