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
// openFromBrowser.h.C
//------------------------------------------
// synopsis:
// Implementation of browser::open_selected.
//------------------------------------------


// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <Question.h>
#include <messages.h>

#include <machdep.h>
#include <x11_intrinsic.h>
#include <openFromBrowser.h>
#include <view_creation.h>
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <cmd.h>
#include <cmd_enums.h>
#include <errorBrowser.h>

void view_flushing(int);

void errorBrowser_show_errors_from_apps();
// FUNCTION DEFINITIONS

openFromBrowser::openFromBrowser(void* , symbolArr& a)
: commandContext(Widget(NULL), "Open View", ESTIM_LONG), parent(NULL), array(a),
  rep_type(Rep_SmtText)
{}

openFromBrowser::openFromBrowser(void* , symbolArr& a, repType theRepType)
: commandContext(Widget(NULL), "Open View", ESTIM_LONG), parent(NULL), array(a),
  rep_type(theRepType)
{}

// Check if we are opening some errornotes
static int check_contains_errors (symbolArr& array)
{
    return 0;
}

static bool checkTooManyItems (const symbolArr& arr)
{
    Initialize (checkTooManyItems);

    int limit = 15;

    genString var = OSapi_getenv("PSET_VIEW_CREATE_WARNING");
    var.trim ();
    if (var.length() > 0) {
	int temp = OSapi_atoi ((char*) var.str());
	if (temp > 1)
	    limit = temp;
    }

    if (arr.size() >= limit) {
	if (dis_question(T_TOOMANYVIEWS, B_YES, B_NO, Q_OPENINGMANYVIEWS, arr.size()) <= 0)
	    return false;
    }
 
    return true;
}

void openFromBrowser::execute()
//
// Creates views of the objects selected in the browser.
//
{
    symbolPtr elem;
    ddElementPtr dd;
    Initialize(openFromBrowser::execute);

    if(array.size() == 0 && !(rep_type == Rep_ERD ||
			      rep_type == Rep_DataChart ||
			      rep_type == Rep_SubsystemMap))
	return;

    switch(rep_type)
    {
      case Rep_ERD:		// Rep-types that work on arrays.
      case Rep_DataChart:
      case Rep_SubsystemMap:
	view_create(array, rep_type);
	break;
      case Rep_FlowChart: 
	if(cmd_available_p(cmdNewFlowcharBehavior)) {
	  ForEachS(elem, array) {
	    if (elem.get_kind() != DD_FUNC_DECL) {
	      msg("Only functions can be displayed in flowcharts. Please make sure that the selection contains only functions.", error_sev) << eom;
	      return;
	    }
	  }
	}
      case Rep_VertHierarchy:	// Rep-types that work on single app-trees.
      case Rep_ExternalFile:
      case Rep_TextDiagram:
      case Rep_TextText:
      case Rep_RawText:
      case Rep_TextFullDiagram:
      case Rep_MultiConnection:
      case Rep_SmtText:
      case Rep_OODT_Scope:
      case Rep_Last:		// => Open view using default rep-type.
      case Rep_UNKNOWN:		// => Open view using default rep-type.
	{
            symbolPtr elem;
	    int contains_errors = check_contains_errors(array);
	    if (contains_errors)
		errorBrowserClear (); // reset error browser

	    if (!checkTooManyItems (array))
		break;
	    
	    view_flushing(0);
	    ForEachS(elem, array) {
		view_create(elem, rep_type, 1);
	    }
            view_flushing(1);
	    
	    if (contains_errors)
		errorBrowser_show_errors_from_apps(); // bring error browser to view
	}
	break;

      case Rep_OODT_Inheritance:
	 {

	    if (!checkTooManyItems (array))
		break;

	    symbolPtr elem;
            view_flushing(0);
	    ForEachS(elem, array) {
	       if (elem.is_xrefSymbol()) {
    	    	  ddKind k;
		  if ((k = elem.get_kind()) == DD_CLASS || k == DD_INTERFACE) {
		     view_create(elem, rep_type);
		  }
		  else msg("Cannot open inheritance view of non-class '$1'", error_sev) << elem.get_name() << eom;
	       }
	       else if (is_ddElement(RelationalPtr(elem))) {
    	    	  ddKind k;
		  ddElementPtr dd = ddElementPtr(RelationalPtr(elem));
		  if ((k = dd->get_kind()) == DD_CLASS || k == DD_INTERFACE) {
		     view_create(elem, rep_type);
		  }
		  else msg("Cannot open inheritance view of non-class '$1'", error_sev) << elem.get_name() << eom;
	       }
	       else msg("Cannot open inheritance view of non-class.", error_sev) << eom;
	    }
            view_flushing(1);
	 }
	 break;

      case Rep_OODT_Relations:
      case Rep_Tree:
	{

	    if (!checkTooManyItems (array))
		break;

            symbolPtr elem;
            view_flushing(0);
	    ForEachS(elem, array) {
		view_create(elem, rep_type);
	    }
            view_flushing(1);
	}
	break;

      case Rep_Grid:		// scrapbooks
	break;

      case Rep_DGraph:
	break;
    }
    view_create_flush();
}


/*
   START-LOG-------------------------------------------

   $Log: openFromBrowser.h.C  $
   Revision 1.30 2001/07/25 20:44:00EDT Sudha Kallem (sudha) 
   Changes to support model_server.
Revision 1.2.1.24  1994/06/27  23:39:29  aharlap
forgiving parse

Revision 1.2.1.23  1994/02/25  23:06:25  boris
Bug track: 6509
restore the module before opening frame doc

Revision 1.2.1.22  1994/01/14  15:01:37  himanshu
Bug track: n/a
Changed the logic such that ext file opening code is separated from the ParaSET view_create

Revision 1.2.1.21  1994/01/03  18:14:40  davea
Bug track: 5767
move two cases from common one, to avoid re-restoring
files when the pmod has all the information we're going
to use.

Revision 1.2.1.20  1993/12/22  22:35:33  davea
Bug track: 5767
separate out Rep_Tree from large case;  it
should *not* restore files, nor all the other
stuff that is done for things like Inheritance
diagrams.

Revision 1.2.1.19  1993/12/21  19:59:29  azaparov
Bug track: 5638
Fixed bug 5638

Revision 1.2.1.18  1993/12/17  19:24:12  himanshu
Bug track: Frame
Added Browser External file view openning

Revision 1.2.1.17  1993/12/14  21:08:52  kws
Bug track: 5384
Ask to unload raw file if modified - update icons for raw case

Revision 1.2.1.16  1993/12/08  16:58:10  himanshu
Bug track: n/a
Fixed the condition to check if the item selected from browser is a xrefsymbol or
not

Revision 1.2.1.15  1993/12/06  23:34:12  andrea
Bug track: n/a
change made by trung

Revision 1.2.1.14  1993/12/03  16:14:23  himanshu
Bug track: new code
added code to handle external (a la Frame) file

Revision 1.2.1.13  1993/01/10  06:04:25  glenn
*** empty log message ***

Revision 1.2.1.12  1993/01/07  14:47:02  glenn
Handle Rep_UNKNOWN

Revision 1.2.1.11  1992/12/28  19:30:54  wmm
Allow opening empty subsystem maps.

Revision 1.2.1.10  1992/12/19  20:32:36  wmm
Allow opening empty ERDs and DCs.

Revision 1.2.1.9  1992/12/16  00:40:50  aharlap
included "symbolArr.h"

Revision 1.2.1.8  1992/12/15  20:44:50  aharlap
changed for new xref

Revision 1.2.1.7  1992/12/12  07:17:56  glenn
New view creation interface.

Revision 1.2.1.6  1992/11/23  21:30:23  wmm
typesafe casts.

Revision 1.2.1.5  1992/11/18  14:38:33  trung
 change dd_of_xref to function

Revision 1.2.1.4  1992/11/10  23:45:26  trung
check for array, in calling is_subsystem, bug 1932

Revision 1.2.1.3  1992/10/19  20:07:04  sergey
Support global xrefs; added get_smt_node routine.

Revision 1.2.1.2  92/10/09  20:14:29  smit
*** empty log message ***

   END-LOG---------------------------------------------
*/


