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
// editorLayer.C
//
// Editor layer class implementation.
//-----------------------------------------------------------------------------

#include <vport.h>
#include vapplicationHEADER

#include "geditorLayer.h"
#ifndef _generic_h
   #include "ggeneric.h"
#endif
#ifndef _gglobalFuncs_h
   #include "gglobalFuncs.h"
#endif
#ifndef _gviewer_h
   #include "gviewer.h"
#endif
#ifndef _gview_h
   #include "gview.h"
#endif
#ifndef _gpaneObjects_h
   #include "gpaneObjects.h"
#endif
#ifndef _rtlClient_h
   #include "grtlClient.h"
#endif

#include <../../DIS_main/interface.h>

#define EDITOR_LAYER 1


//-----------------------------------------------------------------------------

unsigned long	editorLayer::serial_ = 1;

int		viewLayer::uniqId_ = 0;
viewLayer*	viewLayer::viewLayerList_ = 0;

//-----------------------------------------------------------------------------


editorLayer::editorLayer (void)
{
    gInit (editorLayer::editorLayer);

    window_ = 0;
    viewlayer_ = 0;
    oldTextData_ = 0;

    vresource paneRes;
    Pane *    ep;
    if (getResource("DialogLayers:EDITOR/sw", &paneRes) ) 
        ep = new Pane(paneRes);
    else
        ep = new Pane;

    ep->OpenWindow();

    setPane(ep);

    name_.sprintf_scribed(vcharScribeLiteral("Editor <%lu>"), serial_++);

    has_frame_ = 0;
  
    updateLayerList();
}

//-----------------------------------------------------------------------------

int editorLayer::isEditor (void) const
{
    gInit (editorLayer::isEditor);

    return 1;
}

//-----------------------------------------------------------------------------

void editorLayer::getName (gString& nm) const
{
    gInit (editorLayer::getName);

    if (viewlayer_) nm = viewlayer_->getTitle();
    else nm = name_;
}

//-----------------------------------------------------------------------------

const vname* editorLayer::getState (void) const
{
    gInit (editorLayer::getState);

    return vnameInternGlobalLiteral("Editor");
}

//-----------------------------------------------------------------------------

editorLayer::~editorLayer (void)
{
    gInit (editorLayer::~editorLayer);

    removeFromView ();
}

//-----------------------------------------------------------------------------
void editorLayer::SetTextData(TextFile* td) 
{
    oldTextData_ = td;
}

void editorLayer::removeFromView (void)
{
    gInit (Layer::removeFromView);

    View* vp = getView();

    if (vp) {
        vtextview*	ti = (vtextview*)vp->FindItem ((vchar*)"TextWindow");
        if (ti) 
            ti->SetTextData   (oldTextData_);

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
    }
}

//-----------------------------------------------------------------------------

void editorLayer::displayViewLayer (viewLayer* vl)
{
    gInit (editorLayer::displayViewLayer);

    viewlayer_ = vl;

    //rcall_dis_show_file (Application::findApplication("DISmain"),
                         //(vchar*) vl->getTitle() );
    //------
    // FIXME
    //------
}

//-----------------------------------------------------------------------------

void editorLayer::display (void)
{
    gInit (editorLayer::display);

    View* v = getView ();
    if (!v)
	return;

    Pane* pn = getPane ();
    if (!pn)
	return;

    v->SetPane (pn);

#ifndef _WINDOWS
    if (!window_) {
	window_ = pn->GetWindow ();
        rcall_dis_layer_create (Application::findApplication("DISmain"), 
				getId(), window_, EDITOR_LAYER);
    }
#endif

}

//-----------------------------------------------------------------------------

void editorLayer::undisplay (void)
{
    gInit (editorLayer::undisplay);
}

//-----------------------------------------------------------------------------

transientEditorLayer::transientEditorLayer (void)
{
    gInit (transientEditorLayer::transientEditorLayer);
    
    layerStack* ls = getList ();
    ls->takeOut (this);

    updateLayerList ();
}

//-----------------------------------------------------------------------------

int transientEditorLayer::isTransient (void) const
{
    gInit (transientEditorLayer::isTransient);

    return 1;
}

//-----------------------------------------------------------------------------

viewLayer::viewLayer (void)
{
    gInit (viewLayer::viewLayer);

    id_		= uniqId_++;
    title_	= 0;
    next_	= viewLayerList_;
    textFile_ = 0;

    viewLayerList_ = this;
}

//-----------------------------------------------------------------------------

viewLayer::~viewLayer (void)
{
    gInit (viewLayer::~viewLayer);

    if (title_)
	delete title_;
}

//-----------------------------------------------------------------------------

const vchar* viewLayer::getTitle (void) const
{
    gInit (viewLayer::~viewLayer);

    return title_;
}

//-----------------------------------------------------------------------------

void viewLayer::setTitle (const vchar* newTitle)
{
    gInit (viewLayer::setTitle);

    if (newTitle) {
	    if (title_)
	       delete title_;
	    title_ = new vchar [vcharLength (newTitle) + 1];
	    vcharCopy (newTitle, title_);
        for (Viewer* vr = Viewer::GetFirstViewer(); vr; vr = vr->next)
           vr->AddLayerItems ((vchar*) "HistoryMenu",
	 				          (vchar*) "dis_view -layer");

        if (textFile_) delete textFile_;
        textFile_ = new TextFile(newTitle);    
    }
}

//-----------------------------------------------------------------------------

void viewLayer::displayInView (View* newView)
{
    gInit (viewLayer::displayInView);

    Layer* layerInView = newView->GetLayer ();

   editorLayer* el = NULL;

    if (layerInView && layerInView->isEditor()) {
	    el = (editorLayer*) layerInView;
	    el->displayViewLayer (this);
    } else {
	    transientEditorLayer* tel = new transientEditorLayer;
	    tel->displayInView (newView);
	    tel->displayViewLayer (this);
	    el = tel;
    }

    vtextview*	ti = (vtextview*)newView->FindItem ((vchar*)"TextWindow");
    if (ti) {
        ti->SetTextData   (textFile_);
    }
    newView->MakeCurrent();
}

//-----------------------------------------------------------------------------

viewLayer* viewLayer::findViewLayer (int id)
{
    gInit (viewLayer::FindViewLayer);

    for (viewLayer* v = viewLayerList_; v; v = v->next_)
	if (v->id_ == id)
	    return v;
    return NULL;
}

//-----------------------------------------------------------------------------

viewLayer* viewLayer::findViewLayer (const vchar* title)
{
    gInit (viewLayer::findViewLayer);

    for (viewLayer* v = viewLayerList_; v; v = v->next_)
	if (!vcharCompare (title, v->title_))
	    return v;

   return NULL;
}

//-----------------------------------------------------------------------------

viewLayer* viewLayer::getNext (void) const
{
    gInit (viewLayer::getNext);

    return next_;
}

//-----------------------------------------------------------------------------
// end of editorLayer.C
//-----------------------------------------------------------------------------
