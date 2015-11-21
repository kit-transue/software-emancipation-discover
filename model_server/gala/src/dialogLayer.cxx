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
//-----------------------------------------------------------------------------
// dialogLayer.C
//
// Browser layer class implementation.
//-----------------------------------------------------------------------------

#include <vport.h>
#include <vstdlib.h>
#include <vtext.h>

#include "gdialogLayer.h"
#include "ggeneric.h"
#include "gglobalFuncs.h"
#include "gview.h"
#include <gviewer.h>

#include <../../DIS_main/interface.h>


#define DIALOG_LAYER		4

extern int Y2K_FLAG;

//-----------------------------------------------------------------------------

static const vchar* PROCEDURES = (vchar*)("Procedures");
unsigned long dialogLayer::serial_ = 1;

//-----------------------------------------------------------------------------


dialogLayer::dialogLayer (const char * dialog_name)
{
    gInit (dialogLayer::dialogLayer);

    Tcl_Interp* interp = make_interpreter ();
    if (interp) {
	setInterp (interp);
        Tcl_SetVar(interp, "gd", "-1", TCL_GLOBAL_ONLY);
        // Save the layer id in the interpreter.
        char cLayerId[20];
        sprintf (cLayerId, "%d", this->getId());
        Tcl_SetVar (getInterp(), "layerId", cLayerId, TCL_GLOBAL_ONLY);
        if (interp_trace) printf ("DialogLayer Interp[%08x]\n", interp);

    }

    vresource paneRes;
    Pane *    ep;

    gString dnamepath = "DialogLayers:";
    dnamepath += dialog_name;

    if (getResource((const vchar*)dnamepath, &paneRes) ) {
        ep = new Pane(paneRes);
    } else {
        ep = new Pane;
        dis_message((vchar*)"Unable to find resource information for '%s' layer .",
               dialog_name);
    }

    setPane (ep);

    unloadedPane_ = 1;

    static vchar *visibleNameResTag = vstrCloneScribed(vcharScribeLiteral("VisibleName"));
    vresource visibleNameRes;
    vstr *visibleName;

    gString visibleNamePath;
    visibleNamePath += dnamepath;
    visibleNamePath += ":";
    visibleNamePath += visibleNameResTag;

    if (getResource((const vchar *)visibleNamePath, &visibleNameRes)) {
      vstr *visibleName = vresourceGetString(vresourceGetParent(visibleNameRes),
		          vnameInternGlobal(visibleNameResTag));
      name_ = (char *)visibleName;
    }
    else
      name_.sprintf_scribed(vcharScribeLiteral("%s"), dialog_name, serial_++);

    dialogName_ = (vchar *) vmemAlloc((strlen(dialog_name)+1) * sizeof(vchar));
    vcharCopyFromSystem(dialog_name, dialogName_);

    updateLayerList ();

    if(!Y2K_FLAG)
    rcall_dis_layer_create (Application::findApplication("DISmain"), 
			    getId(), 0, DIALOG_LAYER);
}

//-----------------------------------------------------------------------------

void dialogLayer::reopen ()
{
    gala_eval (getInterp(), "ReopenDialog");
}

//-----------------------------------------------------------------------------

void dialogLayer::getName (gString& nm) const
{
    gInit (dialogLayer::getName);

    nm = name_;
}

//-----------------------------------------------------------------------------

void dialogLayer::getResourceName (gString& nm) const
{
    gInit (dialogLayer::getName);

    nm = dialogName_;
}

//-----------------------------------------------------------------------------

const vname* dialogLayer::getState (void) const
{
    gInit (dialogLayer::getState);

    return (vnameInternGlobal(dialogName_));
}

//-----------------------------------------------------------------------------

dialogLayer::~dialogLayer (void)
{
    gInit (dialogLayer::~dialogLayer);

    removeFromView ();
    setView(NULL);

    // Delete the pane in parent class Layer.
    // Pane* pn = getPane ();
    // if (pn) delete pn;

    if (dialogName_) {
        vmemFree(dialogName_);
    }
}

//-----------------------------------------------------------------------------

void dialogLayer::display(void)
{
    gInit (dialogLayer::display);

    View* vp = getView ();
    if (!vp) return;
 
    Viewer* vr = vp->GetViewer();
    if (!vr) return;

    Pane* pn = getPane ();
    if (!pn) return;

    vp->SetPane (pn);

    if (unloadedPane_)  {
	setPane (getView()->GetPane());
	unloadedPane_ = vFALSE;
    }

    if (!isInited()) {
        char cv[20];
        char cvr[20];
        sprintf (cv, "%d", vp->GetId());
        sprintf (cvr, "%d", vr->GetId());
        Tcl_SetVar(getInterp(), "v", cv, TCL_GLOBAL_ONLY);
        Tcl_SetVar(getInterp(), "vr", cvr, TCL_GLOBAL_ONLY);
    
        // eval the procedures.
        gString procName = "DialogLayers.";
        procName += dialogName_;
        procName += ".";
        procName += PROCEDURES;
        eval_StringResource(getInterp(), procName);

	setInited();
    }

    gala_eval (getInterp(), "ReopenDialog");

}


//-----------------------------------------------------------------------------

void dialogLayer::undisplay (void)
{
    gInit (dialogLayer::undisplay);
    Tcl_Interp* interp = getInterp();
    if (interp) {
        if (gala_eval (interp, "CloseDialog") != TCL_OK) {
            if (interp_errTrace) {
                gString myName;
                getName(myName);
                dis_message((vchar*)"Problem with CloseDialog in '%s'.",
                            (vchar*) myName);
            }
    	}
    }
}


//-----------------------------------------------------------------------------

const char *dialogLayer::findPopupMenu (const int x, const int y) {
    return "";
}


//-----------------------------------------------------------------------------
// end of dialogLayer.C
//-----------------------------------------------------------------------------
