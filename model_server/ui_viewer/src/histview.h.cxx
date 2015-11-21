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
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstdio>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <systemMessages.h>
#include <viewerShell.h>
#include <OperPoint.h>
#include <appMarker.h>
#include <steView.h>
#include <steRawView.h>
#include <histview.h>
#include <viewGraHeader.h>

init_relation(view_of_historyObj,1,NULL, historyObj_of_view,MANY,NULL);
init_relation(marker_of_historyObj,1,NULL, historyObj_of_marker,1,NULL);

init_relational(historyObj, commonTree);


historyObj::historyObj (viewPtr curr_view):title("")
// constructor
{
    Initialize(historyObj::historyObj);
    // Check if the curr_view exists
    if (!curr_view) {
    // pop an error if it doesn't and return
	msg("The view has been deleted!!", error_sev) << eom;
	return;
    }
    // set relation between historyObj and curr_view
    put_relation(view_of_historyObj, this, curr_view);
    // get the current insertin point from view, if it is not viewGraHeader
    if (!is_viewGraHeader(curr_view)) {
	OperPoint *op = curr_view->insertion_point();
	if (op) {
	    // create a \'new\' marker object
	    appMarker * focus_obj = new appMarker (0 // marker is deleted
						   ,op->node, op->offset_of);
	    // check if the focus_obj appMarker object exists
	    if(focus_obj) // set relation between focus_obj and historyObj
		put_relation(marker_of_historyObj, this, focus_obj);
	}
    }
    // set the historyObj to be the name of the view
    title = curr_view->get_name();    
    add_view_type_to_name(curr_view, title);
}


historyObj::~historyObj()
{
    // To be filled in later   
}


void historyObj::focus()
{
    Initialize(historyObj::focus);
    viewPtr cv;
    appMarker *am;
    // get the view pointer
    cv = viewPtr(get_relation(view_of_historyObj,this));
    // get the marker pointer
    am = (appMarker *)(get_relation(marker_of_historyObj,this));
    
    if (!cv){
	// delete item from history
	msg("View missing!!", error_sev) << eom;
        return;
    }

    if (am) {
	OperPoint pt;
	pt.node = checked_cast(objTree, get_relation(node_of_marker,am));
	pt.offset_of = am->offset;
        // set the current focus of the view to be that specified 
	// by the marker.
	if (is_steView(cv)) {
	    if (is_steRawView(cv)) {
		steRawView *raw_v = checked_cast(steRawView,cv);
		raw_v->insertion_point (&pt);
            } else {
		steView *steVPtr = checked_cast(steView,cv);
		steVPtr->insertion_point(&pt);	
	    }
        }
    }
}
    
    
viewPtr   historyObj::get_viewPtr() {
    Initialize(historyObj::get_viewPtr);
    viewPtr v = checked_cast(view,get_relation(view_of_historyObj, this));
    if (!(v && v->get_id() > 0))
	v = NULL;
    return v;
}

appMarker * historyObj::get_markerPtr()
    {
    Initialize(historyObj::get_markerPtr);
	return (appMarker *)get_relation(marker_of_historyObj, this);
    }

// for future use
extern "C" void reset_history_views(viewPtr old_view, viewPtr currv)
{
	        Obj* hobjs = get_relation(historyObj_of_view,old_view);
		Obj* hobj;
		historyObjPtr histobj;
		viewPtr tempv;
		appMarker *mrkr;
		ForEach(hobj, *hobjs){ 
		// does a relation exist with historyObj
		// if yes then remove it.
		    histobj = (historyObjPtr)(hobj);
		    tempv = histobj->get_viewPtr();
		    // The next line eliminates the set....
                    // rem_relation(view_of_historyObj, histobj, NULL);
		    // also to be removed is a marker relation 
		    mrkr = (appMarker *)
				 get_relation(marker_of_historyObj,histobj);
		    if (mrkr) {
			// only ste views have markers for now
			//deleting the object
			rem_relation(historyObj_of_marker,mrkr);
			rem_relation(marker_of_historyObj,histobj);
			obj_delete(mrkr);
		    }
		// create a new relation between the historyObj and
		// this view.
		    put_relation(view_of_historyObj,histobj,currv);
                }

}
RelationalPtr historyObj::rel_copy () const
{
    return db_new (historyObj, (*this));
}


extern "C" void tflush()
{
    msg("") << eom;
}

/*
   START-LOG-------------------------------------------

   $Log: histview.h.C  $
   Revision 1.7 2000/10/10 15:19:47EDT Sudha Kallem (sudha) 
   add severity to messages
// Revision 1.9  1994/06/23  18:31:25  builder
// Port (hp only)
//
// Revision 1.8  1993/12/21  19:51:51  azaparov
// Bug track: 5638
// Fixed bug 5638
//
// Revision 1.7  1993/11/30  16:20:08  boris
// Bug track: Test
// fff
//
// Revision 1.6  1993/11/02  17:15:22  boris
// Bug track: #5114
// Fixed restoring previous view after closing the current one
//
// Revision 1.5  1993/10/26  20:22:25  himanshu
// Bug track: 5059
// reset_history_views revised
//
// Revision 1.3  1993/10/25  13:57:06  himanshu
// Bug track: 5057
// The historyObj constructor was fixed to allow views with no insertion points to
// be displayed too.
//
// Revision 1.2  1993/10/21  22:11:22  andrea
// Bug track: 4990
// I commented out the error message "Error in view"
//


   END-LOG---------------------------------------------
*/
