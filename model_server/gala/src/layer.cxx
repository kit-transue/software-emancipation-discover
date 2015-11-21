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
// layer.C
//
// Layer class implementation.
//-----------------------------------------------------------------------------

#include <vport.h>
#include vapplicationHEADER
// remember to put these back when you put back layer::preparetoshutdown
//#include <pdumem.h>
//#include <pdupath.h>
#include "ggeneric.h"
#include "gglobalFuncs.h"
#include "gview.h"
#include "gviewer.h"
#include "gpaneObjects.h"
#include "gviewerMenuBar.h"
#include "glayer.h"

#include "galaxy_undefs.h"
#include "cLibraryFunctions.h"
#include "machdep.h"





//-----------------------------------------------------------------------------

int		Layer::uniqId_;
layerStack*	Layer::list_;

//-----------------------------------------------------------------------------

layerStack* Layer::getList (void)
{
    gInit (Layer::getList);

    if (!list_) list_ = new layerStack;

    return list_;
}

void Layer::updateLayerList (void)
{
    gInit (Layer::updateLayerList);

    for (Viewer* vr = Viewer::GetFirstViewer(); vr; vr = vr->next)
        vr->AddLayerItems ((vchar*) "HistoryMenu", (vchar*) "dis_view -layer");
}

//-----------------------------------------------------------------------------

Layer::Layer (void)
{
    gInit (Layer::Layer);

    id_	= Layer::uniqId_++;

    inited_ = 0;

    setInterp (NULL);
    setView (NULL);
    setPane (NULL);

    if (!list_) list_ = new layerStack;

    Layer* instance = this;
    (*list_).append (instance);
}

//-----------------------------------------------------------------------------

int Layer::isInited ()
{
    return inited_;
}

//-----------------------------------------------------------------------------

void Layer::setInited ()
{
    inited_ = 1;
    return;
}

//-----------------------------------------------------------------------------

void Layer::reopen ()
{
}

//-----------------------------------------------------------------------------

void Layer::displayInView (View* vp)
{
    gInit (Layer::displayInView);

	View *oldView = getView();

    // Check for re-dislaying in the same view.
    if (vp != getView() ) { 
        removeFromView();
        addToView(vp);
    }
	else if (oldView) oldView->refresh();  	// Update the old view
    
    moveToTop();	// Move this layer to the top of the stack.
}

//-----------------------------------------------------------------------------

void Layer::moveToTop()
{
    gInit(Layer::moveToTop);

    if (getView() ) {
        layerStack* ls = Layer::getList();
        ls->takeOut(this);
        ls->push(this);
        Layer::updateLayerList();

        //getView()->UnsetPane();
        getView()->SetLayer(this);
        getView()->MakeCurrent();
        display();
        getView()->GetViewer()->GetMenuBar()->Inval(vwindowINVAL_IMMEDIATE);
    }
}

//-----------------------------------------------------------------------------

void Layer::addToView(View* vp)
{
    gInit(Layer::addToView);

    if (vp) {
        // Handle existing layer, if any.
        Layer* oldLayer = vp->GetLayer();
        if (oldLayer) {
            oldLayer->undisplay();
            if (oldLayer->isTransient() ) {
                delete oldLayer;
            } else {
                oldLayer->setView(NULL);
            }
        }

        // Add this layer to the view.
        vp->SetLayer(this);
        setView(vp);
    }
}

//-----------------------------------------------------------------------------

void Layer::removeFromView (void)
{
    gInit (Layer::removeFromView);

    if (getView() ) {
        View* vp = getView();
        vp->SetLayer(NULL);    // Clear the Layer.
        vp->UnsetPane();
        undisplay();

        // See if another layer should take over the old view.
        setView(NULL);    // Make sure that this layer isn't found.
        Layer* l = Layer::findPreviousLayer(this);
        view_ = vp;    // But set up so we can find it later.
                       // Note - This doesn't use setView because of interp stuff.
        if (l) {   // Display the layer in the view.
            l->addToView(vp);
            l->moveToTop();
        }

	else {	// else set the current view to an empty view.
            vp->SetState (NULL);
	    vp->MakeCurrent();
	}
	    
    }
}

//-----------------------------------------------------------------------------

int Layer::isEditor (void) const
{
    gInit (Layer::isEditor);

    return 0;
}

//-----------------------------------------------------------------------------

int Layer::isTransient (void) const
{
    gInit (Layer::isTransient);

    int is_transient = 0;

    Tcl_Interp* interp = getInterp();
    gString layerName;
    getName(layerName);

    if (interp) {
        char* transient = Tcl_GetVar(interp, "IsTransient", TCL_GLOBAL_ONLY);
        if (transient && (*transient=='1') ) {
            is_transient = 1;
        }
    } else if (interp_errTrace) {
        dis_message((const vchar*)"'%s' does not have an interp.", layerName );
    }

    return is_transient;
}

//-----------------------------------------------------------------------------

void Layer::getName (gString& nm) const
{
    gInit (Layer::getName);

    nm = vcharGetEmptyString ();
}

void Layer::getResourceName (gString& nm) const
{
    gInit (Layer::getResourceName);

    nm = vcharGetEmptyString ();
}


//-----------------------------------------------------------------------------

const vname* Layer::getState (void) const
{
    gInit (Layer::getState);

    return 0;
}

//-----------------------------------------------------------------------------

Layer::~Layer (void)
{
    gInit (Layer::~Layer);

    removeFromView ();

    (*list_).takeOut (this);

    updateLayerList();

    if (pane_) {
//        pane_->DeleteLater();
    }
}

//-----------------------------------------------------------------------------

Layer* Layer::find_layer (const char* name)
{
    gString nm1;
    gString nm2((vchar*)name);

    for (int i=0; i<(*list_).size(); i++) {
	(*list_)[i]->getResourceName(nm1);
	if (strcmp ((char*)(vchar*)nm1,(char*)(vchar*)nm2) == 0)
            return (*list_)[i];
    }
    return NULL;
}

Layer* Layer::find_layer (int id)
{
    for (int i=0; i<(*list_).size(); i++) {
	if ((*list_)[i]->id_ == id)
            return (*list_)[i];
    }
    return NULL;
}

Layer* Layer::findPreviousLayer (Layer *l)
{
    // Remember, the stack goes from 0 to size-1.
    int last = (*list_).size() - 1;
	// looking for the <l> layer in stack
    for(register i=last; i>=0; i--)  {
		if((*list_)[i] == l) break;
	}
	if(i>0) return (*list_)[i-1];
    /*for (int i=last; i>=0; i--) {
	if (!(*list_)[i]->getView() && (*list_)[i] != l)
            return (*list_)[i];
    }*/
    return NULL;
}


const char *Layer::findPopupMenu (const int x, const int y) {
    return "";
}

void Layer::setView (View* vp)
{
    gInit (Layer::setView);

    view_ = vp;

    int	view_id = 0;
    int	viewer_id = 0;

    Tcl_Interp* intr = getInterp ();
    if (!intr)
	return;

    if (vp) {
	view_id = vp->GetId ();
	Viewer* vrp = vp->GetViewer ();
	if (vrp)
	    viewer_id = vrp->GetId ();
    }

    gString cmd;
    cmd.sprintf_scribed (vcharScribeLiteral("set vr %d; set v %d"), viewer_id, view_id);
    Tcl_Eval (intr, (char*) cmd.str());
}

//-----------------------------------------------------------------------------

void layerStack::push (Layer* lp)
{
    gInit (layerStack::push);

    append (lp);
}

//-----------------------------------------------------------------------------

void layerStack::pop (void)
{
    gInit (layerStack::pop);

    int sz = size ();
    if (sz > 0)
        remove (sz - 1);
}

//-----------------------------------------------------------------------------

void layerStack::takeOut (Layer* lp)
{
    gInit (layerStack::takeOut);

    int sz = size ();

    for (int i = 0; i < sz; i++)
        if (operator [] (i) == lp) {
            remove (i);
            break;
        }
}

//-----------------------------------------------------------------------------

Layer* layerStack::top (void) const
{
    gInit (layerStack::top);

    int sz = size ();
    if (sz > 0)
        return operator [] (sz - 1);

    return 0;
}

//-----------------------------------------------------------------------------
// end of layer.C
//-----------------------------------------------------------------------------

