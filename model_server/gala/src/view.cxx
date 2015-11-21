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

/**********
 *
 * view.C - View and  Pane methods.
 *
 ***** Child View Trees
 *
 *    Panes which contain other views have a Child View Tree which is
 * separate from the main View Tree structure. The myOwnerPane link will
 * be non-Null for views which are the root of a Child View Tree.
 *
 **********/



#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcolorHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include vnotebookHEADER
#include vwindowHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gview.h>
#include <gsash.h>
#include <gviewer.h>
#include <glayer.h>
#include <ginterp.h>
#include <gpaneObjects.h>
#include <gglobalFuncs.h>
#include <geditorLayer.h>
#include <gviewerMenu.h>
#include <subwin.h>

#include <gdraw.h>


#ifdef sun4
extern "C" int __0FRvwindowGetXWindowP6Hvwindow (...);
#endif

#ifdef sun5
extern "C" int __0FRvwindowGetXWindowP6Hvwindow (...);
#endif

#ifdef hp700
extern "C" int vwindowGetXWindow__FP7vwindow (...);
#endif

vkindTYPED_FULL_DEFINITION(Pane, vcontainer, "Pane");

void show_interpreter_wait(int);

const int TCL_STRING_SIZE  = 1000;
const int MAX_LIGHT = 150;    // Determine shadows for 3-D pane borders
const float DARKNESS = (float)0.25;
const float LIGHTNESS = (float)1.5;
const vchar* LIST_STRING  = (vchar*)"ContentList";  // Additional string for content
                                                    //   lists
const vchar* NULL_STRING = (vchar*)"\0";
const vchar* UNSPLITABLE = (vchar*)"#";
const vchar* CURRENT_BORDER_COLOR = (vchar*)"BorderColor";
const vchar* DEFAULT_BORDER_COLOR = (vchar*)"DefaultColor";
const vchar* TARGET_BORDER_COLOR = (vchar*)"TargetColor";
const int BORDER = 1;      // Size of border for panes

const vchar* NO_STATE = (vchar*)"None";
const int MAX_STATE_LEN = 1024;


static int in_resize = 0;

void View::GetWindowInfo (int& win, int& vid, int& x, int& y, int& w, int& h)
{
#ifdef sun4
    win = __0FRvwindowGetXWindowP6Hvwindow(myPane->DetermineWindow());
#endif
#ifdef sun5
    win = __0FRvwindowGetXWindowP6Hvwindow(myPane->DetermineWindow());
#endif
#ifdef hp700
    win = vwindowGetXWindow__FP7vwindow(myPane->DetermineWindow());
#endif
    vid = id;
    x = myPane->GetX();
    y = myPane->DetermineWindow()->GetHeight() - myPane->GetY() - myPane->GetHeight();
    w = myPane->GetWidth();
    h = myPane->GetHeight();
}

// Destroy - Galaxy destructor for Pane class.
void Pane::Destroy(void)
{
    if (childViewTree_) {
        delete childViewTree_;
        childViewTree_ = NULL;
    }

    while (GetItemCount())  {
        vdialogItem* nextItem = GetItemAt(0);
        DeleteItem(nextItem);
        nextItem->DeleteLater();
    }

    vcontainer::Destroy();
}


// doInit - Common initialization for Init and LoadInit.
void Pane::doInit (void)
{
    gInit (Pane::doInit);
    window_ = NULL;
    ownerViewType = UnknownView;
    viewTreeBranch = NULL;
    childViewTree_ = NULL;
        myOwner = NULL;
    invalid_ = vFALSE;
}


// Init - Standard Galaxy Constructor.
void Pane::Init(void)
{
    gInit (Pane::Init);
    vcontainer::Init ();
    emptyPane = 1;
    doInit();
}


// LoadInit - Galaxy Constructor for loading from a resource.
void Pane::LoadInit (vresource res)
{
    gInit (PaneL::LoadInit);
    vcontainer::LoadInit(res);
    emptyPane = 0;
    m_Ratio = (float)-1;
    doInit ();
}



// SetViewBranch - Stores View-Tree inside the Pane.
//    This allows Multi-Views to be moved around.
void Pane::SetViewBranch( View* viewBranch, ViewType newType)
{
    ownerViewType = newType;
    viewTreeBranch = viewBranch;
}



// AttachView - Attaches the Pane to the given view.
//    It will save the view as myOwner.
//    If this Pane is a Sub-Win, it will be attached to this window.
//    It will set up the colors.
void Pane::AttachView(View* newOwner) 
{
    myOwner = newOwner;

    if (window_) {
        myOwner->myViewer->GetDialog()->AppendItem(window_);
    }

#ifdef OWNCOLOR
    vcolor* fg = GetForeground();
    vcolor* bg = GetBackground();

    SetForeground (fg ? fg : Viewer::GetPaneForeground());
    SetBackground (bg ? bg : Viewer::GetPaneBackground());
#endif

    SetBorder(0);
}



// HandleButtonDown - Intercepts Mouse-Down events.
//    Tells the Viewer that this should become the current View.
//    Passes the event on to the container.

int Pane::HandleButtonDown(vevent *event) {

    // Go down to the leaves of the view tree.
    
    if (myOwner && (childViewTree_ || !myOwner->ChildView) &&
        (!myOwner->Unsplitable_ && !myOwner->myOwnerPane)) {
        myOwner->MakeCurrent();

        // If the right mouse button was depressed, see if we need to display a popup menu.
        // find the popup menu in the viewer, and display it.

        if (event->GetBinding() == vname_Menu) {

            Viewer *vr = NULL;
            if (myOwner) vr = myOwner->GetViewer();
            if (vr) {

                // Ask the Layer what menu we should pop up.  Send the X and Y location of the pick
                // in case that has a bearing on which menu is choosen.
    
                Layer *layer = myOwner->GetLayer();
                if (layer) {
                    const char *menuName = layer->findPopupMenu (event->GetX(), event->GetY());

                    ViewerPopupMenu *menu = vr->findPopupMenu (menuName);
                    if (menu) {

                        menu->ExecutePopupCmd ();

                        vdialogItemList *itemList;
                        if (itemList = GetItemListIn()) {
                            itemList->TransformEvent (event);
                        }

                        menu->PlaceEvent (event);
                        menu->Popup (event);
                    }
                }
            }
        }
    }

    vcontainer::HandleButtonDown(event);
    return vTRUE;
}



// FindSmallestXElt - Returns the left-most element in the pane.
//    Also sets the ratio and removes the item from this pane..

Pane* Pane::FindSmallestXElt(int* ratio)
{
    vrect newRect;
    vdialogItem* nextItem;

    // Get the size of the first item.
    vdialogItem* smallestItem = GetItemAt(0);
    vrect smallestRect = *(smallestItem->GetRect());

    // Loop through all items to find the leftmost.
    for (int x = 1; x < GetItemCount(); x++) {
        nextItem = GetItemAt(x);
        newRect = *(nextItem->GetRect());
        if (newRect.GetX() < smallestRect.GetX()) {
            smallestItem = nextItem;
            smallestRect = newRect;
        }
    }

    // Compute the size ratio of this rect to the whole pane.
    newRect = *(GetRect());
    *ratio = (int)(100 * (float)(smallestRect.GetWidth())/
             ((float)(newRect.GetWidth() - smallestRect.GetX())) + 0.5);

    // Return the leftmost item.
    return (Pane*) smallestItem;
}



// FindLargestYElt - Finds the top-most item in this pane.
//    It also sets the ratio and removes the item from the container.
// Note - The largest Y value is needed because Galaxy coordinate system
//    has 0 at the bottom.

Pane* Pane::FindLargestYElt(int* ratio)
{
    vrect newRect;
    vdialogItem * nextItem;

    // Get the size of the first element.
    vdialogItem * largestItem = GetItemAt(0);
    vrect largestRect = *(largestItem->GetRect());

    // Loop through all items to find the top one.
    for (int x = 1; x < GetItemCount(); x++) {
        nextItem = GetItemAt(x);
        newRect = *(nextItem->GetRect());
        if (newRect.GetY() > largestRect.GetY()) {
            largestItem = nextItem;
            largestRect = newRect;
        }
    }

    // Compute the ratio of the top rect to the entire pane.
    newRect = *(GetRect());
    *ratio = (int)(100 * (float)(largestRect.GetHeight())/
             ((float)(largestRect.GetHeight() + largestRect.GetY())) + 0.5);

    // Return the top-most item.
    return (Pane*) largestItem;
}



// Inval - Invalidates the invalRect_ area.
void Pane::Inval(int invalFlag)
{
    vcontainer::InvalRect(&invalRect_, invalFlag);
}



// Draw - Draws the pane.
//    First draws the pane, then draws the border.
void Pane::Draw()
{
    vcontainer::Draw();
    DrawBorder();
}


/*  This method draws the 3-d border around the pane.  After first checking that
    this pane is actually supposed to have a border and that it hasn't been shrunk
    down to zero width or zero height (squashed between two sashes), it will look to
    see what sort of border it should have.  If it is the current or target view, it
    will load the color from the .vr, shading it to produce a second color for 3-d 
    effect.  Otherwise, it will just pick colors based on the colors of the pane.
    It then creates a rectangle that defines the actual border and calls the 3d
    routines to draw the rectangle on it.  Note: the built-in galaxy 3d border
    commands were not used because they were found to generate garbage all over the
    screen.
 */

void Pane::DrawBorder()
{
    if (myOwner && (myOwner->hasBorder_) && GetWidth() &&
        GetHeight() && !myOwner->Unsplitable_) {
        vdraw::GSave();
        DrawDevice();
        vcolor* topColor;
        vcolor* bottomColor = NULL;
        vcolor* colors[2];

        if ((myOwner->id == myOwner->myViewer->GetViewCode()) ||
            (myOwner->targ)) {
            vcolorSpec * newcolorspec = new vcolorSpec;
            if (myOwner->targ) 
                bottomColor = Viewer::GetCurrentPaneBorder();
            else 
                bottomColor = Viewer::GetTargetPaneBorder();
            if (bottomColor) {
                unsigned int hue, light, sat, newLight;

                newcolorspec->Copy(bottomColor->GetSpec());
                newcolorspec->GetHLS(&hue, &light, &sat);
                if (light < MAX_LIGHT)
                    newLight = (unsigned int)((float)light * LIGHTNESS);
                else
                    newLight = (unsigned int)((float)light * DARKNESS);

                newcolorspec->SetHLS(hue, newLight, sat);
                topColor = vcolor::Intern(newcolorspec);

                if (newLight < MAX_LIGHT) {
                    vcolor* temp = topColor;
                    topColor = bottomColor;
                    bottomColor = temp;
                }
            }
            if (newcolorspec)
                 delete newcolorspec;
        }
        if (!bottomColor) {    // No colors found.
            // Just load in the colors for the 3d border
            static int colorIDs[2] = { vdrawCOLOR_SHADOW_TOP,
                                       vdrawCOLOR_SHADOW_BOTTOM };
            if (myOwner->myViewer->GetDialog() )
                myOwner->myViewer->GetDialog()->DetermineColors(colorIDs, 2, colors);
            topColor = colors[1];
            bottomColor = colors[0];
        }

        Draw3DBottomRight(&invalRect_, BORDER, bottomColor);
        Draw3DTopLeft(&invalRect_, BORDER, bottomColor);

        vdraw::GRestore();
    }
}


/* This method moves the location of the view.  It first checks to see if there is
   a viewtree branch associated with this pane, in which case it will resize the
   children to the new size.  It then removes the branch, because from this point on
   the children will only need to respond to normal resizing events.  It was needed
   in this case to force the children to adjust to the new size when they were moved
   from some other view.  
  
   The method will then set its border, leaving room for a border if the view has a
   border, is not unsplitable, and has actual width and height.  Finally, it will
   move the subwin, if there is one associated with it, to match the location of the
   pane.  */

void Pane::SetLocation(int x, int y, int w, int h)
{
    if (childViewTree_)  {   // Change to internal coordinates.
        childViewTree_->MoveView(0, 0, w, h);
    }
        
    // Save the full rectangle as the area to invalidate.
    invalRect_.Set(x,y,w,h);

    // Inset draw-able area by the border.
    int borderSize = 0;
    if ((myOwner->hasBorder_) && w && h && !myOwner->Unsplitable_)
        borderSize = BORDER;

    vrect myRect = *(GetRect());
    // Inset the rectangle to hold the border on each side.
    myRect.Set(x + borderSize, y + borderSize,  // x,y are inset by the border.
               w - 2*borderSize, h - 2*borderSize);   // have border on each side.
    SetRect(&myRect);

    if (window_)  {
        vrect windowRect = *(window_->GetRect());
        windowRect.Set(myOwner->GetAbsoluteX() + borderSize,
                       myOwner->GetAbsoluteY() + borderSize,
                       GetWidth(), GetHeight());
        window_->SetRect(&windowRect);
    }
}



void Pane::GrowViewTree()
{
    // Set up the view's rectangle.
    Rect newRect;
    newRect.x = 0;
    newRect.y = 0;
    newRect.w = GetWidth();
    newRect.h = GetHeight();

    // Create the View.
    if (childViewTree_) { 
        printf("@GrowViewTree: childViewTree was non-NULL!!!\n");
    }
    childViewTree_ = new View(NULL, myOwner->myViewer, newRect);
    // Move the Child View Pane to the Owner.
    childViewTree_->DeleteItem(childViewTree_->myPane);
    childViewTree_->myOwnerPane = this;
    childViewTree_->AppendItem(childViewTree_->myPane);
}



/* Creates a subwin */
void Pane::OpenWindow()
{
#ifdef _WINDOWS
    window_ = NULL;
#else
    window_ =  NULL; //new subwin;
#endif
}



/* If there is a window and the pane is being hidden, it will remove the window 
   from the dialog */
void Pane::HideWindow()
{
#ifdef _WINDOWS
    if (window_)
        myOwner->myViewer->GetDialog()->DeleteItem(window_);
#endif
}




//--------------- View Class ----------------------



// View - Constructor for View class.
View::View(View* parent, Viewer* viewer, Rect rect, ViewType viewType,
          vchar* tagName)
{
    ParentView = parent;
    myViewer = viewer;
    myRect= rect;
    view_type = viewType;
    myOwnerPane = NULL;

    id = Viewer::uniqid++;
    rep_type = UnknownRep;
    targ = 0;
    LeftView = NULL;
    RightView = NULL;
    ChildView = NULL;
    myPane = NULL;
    hasBorder_ = vFALSE;
    Unsplitable_ = vFALSE;
    editor_on = vFALSE;
    mySash = NULL;      
    percentX_ = (float)100.0;
    percentY_ = (float)100.0;
    vLock_ = vFALSE;
    hLock_ = vFALSE;
    SetLayer(NULL);
    SetState(NULL);

    // Init my Pane.
    Pane * newPane;
    vresource   res;
    // If we can find the resource, load it.
    if (tagName && getResource(tagName, &res) ) {
        newPane = new Pane(res);
    } else
        newPane = new Pane;
    SetPane(newPane);

    // Init my interpreter.
    myInterp = make_interpreter();
    // Update the Viewer and View vars for my interpreter.
    char buffer[TCL_STRING_SIZE];
    sprintf(buffer,"set vr %d;set v %d", myViewer->id,id);
    Tcl_Eval(myInterp, buffer);

    if (myViewer->layerViews == NULL)
        myViewer->layerViews = this;
}


// ~View - Desctuctor for the View class.
// It recursively deletes all of its children, removes its pane (and sash),
//    it then deletes its TCL interpreter.
View::~View() 
{
//    fprintf(stderr, "Start of ~View:\n");
//    print(0);

    // Delete all my children.
    View* v = ChildView;
    while (v) {
        delete v;         // Recursively delete kids.
        v = ChildView;    // The delete will update ChildView.
    }

    // Update the View's Layer.
    Layer* l = GetLayer();
    if (l) l->setView(NULL);

    // Update any layers on the layer stack
    layerStack* ls = Layer::getList();
    for (int i = 0; i < ls->size(); i++) {
        Layer *lp = (*ls)[i];
        if (lp->getView() == this) lp->setView (NULL);
    }

    // Delete my sash and pane.
    RemovePane();    // Note - this should be after removeFromView.
    RemoveSash();

    if (RightView) {
         RightView->RemoveSash();
    }

    if (LeftView) {
        LeftView->RemoveSash();
    }

    // Update view list.
    if (myViewer->layerViews == this)
        myViewer->layerViews = RightView;

    // Update owner pane.
    if (myOwnerPane) {
        if (RightView)
            RightView->myOwnerPane = myOwnerPane;
        myOwnerPane->childViewTree_ = RightView;
        myOwnerPane = NULL;
    }

    // Update parent.
    if (ParentView && ParentView->ChildView == this) {
        ParentView->ChildView = RightView;
    }

    // Update left neighbor.
    if (LeftView) {
        // See if we need to adjust a group.
        if (ParentView && ParentView->view_type == HorizontalGroupView)
            LeftView->SetRight(myRect.x + myRect.w);
        else if (ParentView && ParentView->view_type == VerticalGroupView)
            LeftView->SetBottom(myRect.y);

        LeftView->RightView = RightView;
        LeftView = NULL;
    }

    // Update right neighbor.
    if (RightView) {
        // See if we need to adjust a group.
        if (ParentView && ParentView->view_type == HorizontalGroupView)      
            RightView->SetLeft(myRect.x);
        else if (ParentView && ParentView->view_type == VerticalGroupView)      
            RightView->SetTop(myRect.y + myRect.h);

        RightView->LeftView = LeftView;
        RightView = NULL;
    }

    // Delete my interpreter.
    Viewer* exViewer;
    View* exView;
    Viewer::GetExecutingView(exViewer, exView);
    if (exView != this) {
        Tcl_InterpDeleteEvent *e = new Tcl_InterpDeleteEvent;
        e->SetClientData (myInterp);
        e->Post();
    }

//    print(0);
//    fprintf(stderr, ":End of ~View\n");
}



// RemoveSash - Deletes the sash.
void View::RemoveSash() 
{
    if (mySash) {
        mySash->Inval(vwindowINVAL_IMMEDIATE);
        DeleteItem(mySash);
        mySash->DeleteLater();
        mySash = NULL;
    }

}



// RemovePane -  Removes the pane.
void View::RemovePane() 
{
    if (myPane) { 
        myPane->Inval(vwindowINVAL_IMMEDIATE);
            DeleteItem(myPane);
            if (myPane->isEmptyPane()) {
                    myPane->DeleteLater();
            }
        myPane = NULL;
    }
}



/* This method makes this view the target view if set is true.  It also finds the
   last target view and unsets it */
void View::target(int set)
{
    if (set) {
        View* newTarg = myViewer->find_target_view();
        if (newTarg) {
            newTarg->target(0);
            newTarg->myPane->DrawBorder();
        }
        targ = 1;
    } else
        targ = 0;

    myPane->DrawBorder();
//    dis_set_target(id);
}



/*
This method searches for a specific view.  It first searches all of its neighbors to see if
they are the target view, and then searches its children.
*/
View* View::find_view(int newId)
{
    for (View* v = this; v; v=v->RightView) {
        if (v->id == newId) return v;
        
        for (View* v2=v->ChildView; v2; v2=v2->RightView) {
            View* v3 = v2->find_view (newId);
            if (v3) return v3;
        }
    }
    if (myPane && myPane->childViewTree_)
        return myPane->childViewTree_->find_view(newId);
    return NULL;
}

/*
This method searches for a specific view.  It first searches all of its neighbors to see if
they are the target view, and then searches its children.
*/

View* View::find_view_for_layer()
{
    for (View* v = this; v; v=v->RightView) {
        if (myLayer && myLayer->isEditor()) return v;
        
        for (View* v2=v->ChildView; v2; v2=v2->RightView) {
            View* v3 = v2->find_view_for_layer ();
            if (v3) return v3;
        }
    }
    if (myPane && myPane->childViewTree_)
        return myPane->childViewTree_->find_view_for_layer ();
    return NULL;
}



/* This method searchs for the target view.  It first checks all of its
   neighbors to see if they are the target view, and then itself. */
View* View::find_target_view()
{
    for (View* v = this; v; v=v->RightView) {
        if (v->targ) return v;
        
        if (v->ChildView) {
            View* vv = (v->ChildView)->find_target_view();
            if (vv) return vv;
        }
    }
    return NULL;
}



/*
To be implemented later
*/
void View::close()
{
}


// AddParentView - Makes current view the only child of a new node.
void View::AddParentView(ViewType horizOrVert)
{
    // Create the parent.
    View * newParent = new View(ParentView, myViewer, myRect, horizOrVert);

    // If I have a sash, move it to my parent.
    if (mySash) {
        newParent->mySash = mySash;
        mySash->SetOwner(newParent);
        mySash->Inval(vwindowINVAL_IMMEDIATE);
        mySash = NULL;
    }

    // Remove my Pane and new Parent's Pane until heritage is settled.
    DeleteItem(myPane);
    newParent->DeleteItem(newParent->myPane);

    // If I was the top of the View tree, make the new parent the top.
    if (myViewer->layerViews == this)
        myViewer->layerViews = newParent;

    // If I have an Owner Pane, move it to the new parent.
    if (myOwnerPane) {
        newParent->myOwnerPane = myOwnerPane;
        myOwnerPane->childViewTree_ = newParent;
        myOwnerPane = NULL;
    }

    // If my old parent points to me, make it point to the new parent.
    if (ParentView && ParentView->ChildView == this) 
        ParentView->ChildView = newParent;

    // If I had a left neighbor, move it up to my new parent.
    if (LeftView) {
        LeftView->RightView = newParent;
        newParent->LeftView = LeftView;
        LeftView = NULL;
    }

    // If I had a right neighbor, move it up to my new parent.
    if (RightView) {
        RightView->LeftView = newParent;
        newParent->RightView = RightView;
        RightView = NULL;
    }

    // Make my new parent point to me and make me point to it.
    newParent->ChildView = this;
    ParentView = newParent;

    // Move my X and Y ratios to my new parent.
    newParent->percentX_ = percentX_;
    newParent->percentY_ = percentY_;

    // Now that our heritage is settled, add the panes back.
    AppendItem(myPane);
    newParent->AppendItem(newParent->myPane);

    // If my new parent's right neighbor has a sash, it needs to redraw too.
    if (ParentView->RightView && ParentView->RightView->mySash)
        ParentView->RightView->mySash->Inval(vwindowINVAL_IMMEDIATE);
}



// AddRightView - Adds new right neighbor.
void View::AddRightView()
{
    // Create a new Right neighbor and add it to this view.
    View * newRight = new View(ParentView, myViewer, myRect);

    // Update my right neighbor.
    if (RightView) {
        // The new guy is to the left of my old right neighbor.
        RightView->LeftView = newRight;
        // My old right is right of the new guy.
        newRight->RightView = RightView;
    }

    // Make the new guy my Right neighbor.
    RightView = newRight;

    // I must be my Right neighbor's Left neighbor.
    newRight->LeftView = this;
}



// split_vertical - Handles a horizontal split.
//    Insures that this view is Splitable and the the ration is non-Zero.
//    Creates a new parent view and and a new Right view for this view.
void View::split_vertical(int ratio)
{
//    fprintf(stderr, "Start of split_vertical:\n");
//    print(0);

    // If the ratio is zero, can't do anything.
    if (ratio == 0) {
        printf("@split_vertical:Zero ratio given!!!\n");

    // Else if this view is Un-Splitable, try the parent.
    } else if (Unsplitable_) {
        printf("@split_vertical: View is Unsplitable, trying the parent.\n");
        if (ParentView)
            ParentView->split_vertical(ratio);

    // Else split this this view and place in a new parent.
    } else {
        // Add new parent and make sure it doesn't have a border.
        AddParentView(HorizontalGroupView);
        ParentView->hasBorder_ = vFALSE;
        ParentView->UpdatePane();

        // Add new Right neighbor.
        AddRightView();

        // My width is my ratio of what the Sash doesn't use.
        float myWidth = ((ParentView->myRect.w - SASH_SIZE) * (float)ratio) / (float)100.0;
        int width = (int) (myWidth + 0.5);
    
        // Move me to the left side.
        MoveView(0, 0, width, ParentView->myRect.h);

        // Move my right neighbor to my right side.
        width = ParentView->myRect.w - width - SASH_SIZE;
        RightView->MoveView(myRect.x + myRect.w + SASH_SIZE,
                            myRect.y, width, myRect.h);
    
        // Add a sash to my new right neighbor.
        vrect *sashRect = new vrect;  // Create a horizontal sash
        sashRect->Set(myRect.w, myRect.y, SASH_SIZE, myRect.h);
        RightView->mySash = new Sash(sashRect, ParentView->myPane,
                                    Horizontal, RightView);
        delete sashRect;
        RightView->mySash->Open();

        // Update all of the changes
        RightView->UpdatePane();
        UpdatePane();

        Layer* layer = GetLayer();
                if (layer) {
                        layer->reopen();
                }
    }
//    print(0);
//    fprintf(stderr, ":End of split_vertical\n");
}



// split_horizontal - Handles a vertical split.
//    Insures that this view is Splitable and the the ration is non-Zero.
//    Creates a new parent view and and a new Right view for this view.
void View::split_horizontal(int ratio)
{
//    fprintf(stderr, "Start of split_horizontal:\n");
//    print(0);

    // If the ratio is zero, can't do anything.
    if (ratio == 0) {
        printf("@split_horizontal:Zero ratio given!!!\n");

    // Else if this view is Un-Splitable, try the parent.
    } else if (Unsplitable_) {
        printf("@split_horizontal: View is Unsplitable, trying the parent.\n");
        if (ParentView)
            ParentView->split_horizontal(ratio);

    // Else split this view and place in a new parent.
    } else {
        // Add new parent and make sure it doesn't have a border.
        AddParentView(VerticalGroupView);
        ParentView->hasBorder_ = vFALSE;
        ParentView->UpdatePane();

        // Add new right neighbor.
        AddRightView();

        // My neighbor's new height is determined by
        //     his ratio of what the Sash doesn't use.
        float newHeight = ((ParentView->myRect.h - SASH_SIZE) * ratio) / (float)100.0;
        int height = (int) (newHeight + 0.5);

        // Move my right neighbor to the bottom.
        RightView->MoveView(0, 0, ParentView->myRect.w, height);
    
        // My new height is what's left over.
        height = ParentView->myRect.h - height - SASH_SIZE;

        // Move me to above my neighbor.
        MoveView(RightView->myRect.x, (RightView->myRect.y +
                 RightView->myRect.h + SASH_SIZE), 
                 RightView->myRect.w, height);
    
        // Add a sash to my new right neightbor.
        vrect *sashRect = new vrect;  // Create a vertical sash
        sashRect->Set(0, RightView->myRect.h, myRect.w , SASH_SIZE);
        RightView->mySash = new Sash(sashRect, ParentView->myPane,
                                     Vertical, RightView);
        delete sashRect;
        RightView->mySash->Open();

        // Update all of the changes
        RightView->UpdatePane();
        UpdatePane();

                Layer* layer = GetLayer();
                if (layer) {
                        layer->reopen();
                }
    }
//    print(0);
//    fprintf(stderr, ":End of split_horizontal\n");
}



// AddPane - Adds a new  pane into the view tree.
//    Very similar to split_horizontal/vertical. 
//    It inserts a new view into the view tree.
void View::addPane(int ratio)
{
    // Make sure I have a parent view.
    if (!ParentView) { 
        printf("@addPane: No parent view!!!\n");

    // Else, if I am unsplitable, add the pane to my parent.
    } else if (Unsplitable_) {
        ParentView->addPane(ratio);

    // Else add a pane.
    } else {
        // Create my new right neighbor.
        AddRightView();
    
        // If this is a vertical group, set up the rectangles.
        if (ParentView->view_type == HorizontalGroupView) {
            // Save the original width.
            int totalWidth = myRect.w;

            // Get my new width.
            float newWidth = ((totalWidth - SASH_SIZE) * ratio)
                             / (float) 100.0;
            myRect.w = (int) (newWidth + 0.5);
        
            // Adjust me to the new width.
            MoveView(myRect.x, myRect.y, myRect.w, myRect.h);

            // Move the new guy to my right.
            RightView->MoveView( (myRect.x + myRect.w + SASH_SIZE),
                                RightView->myRect.y, 
                                (totalWidth - myRect.w - SASH_SIZE),
                                RightView->myRect.h);
        
            // Set up the new guy's sash.
            vrect *sashRect = new vrect;
            sashRect->Set(myRect.x + myRect.w, myRect.y,
                          SASH_SIZE, myRect.h);
            RightView->mySash = new Sash(sashRect, ParentView->myPane,
                                        Horizontal, RightView);
            RightView->mySash->Open();

        // Else set up for a vertical group.
        } else {
            int totalHeight = myRect.h; // Save the current height.
        
            // Get my new height.
            float newHeight = ((totalHeight - SASH_SIZE) * ratio) 
                              / (float) 100.0;
            myRect.h = (int) (newHeight + 0.5);

            // Move the new guy below me.
            RightView->MoveView(myRect.x, myRect.y, myRect.w,
                                (totalHeight - myRect.h - SASH_SIZE) );

            // Move me to the top with the new height.
            MoveView(RightView->myRect.x, (RightView->myRect.y +
                     RightView->myRect.h + SASH_SIZE),
                     RightView->myRect.w, RightView->myRect.h);
        
            // Set up the new guy's sash.
            vrect *sashRect = new vrect;
            sashRect->Set(myRect.x, myRect.y - SASH_SIZE,
                          myRect.w , SASH_SIZE);
            RightView->mySash = new Sash(sashRect, ParentView->myPane,
                                        Vertical, RightView);
            RightView->mySash->Open();
        }
    
        // Update me and the new guy.
        RightView->UpdatePane();
        UpdatePane();
    }
}


//  This function determines if the view is the only view displayed in the viewer
// besides the message box.  Returns TRUE if it is the only view, FALSE otherwise.


int View::ViewerOnlyView () {

    if (!ParentView) return vTRUE;

    View *view = this;
    while (view && view != myViewer->layerViews) {
        if (view->RightView || view->LeftView) return vFALSE;
        view = view->ParentView;
    }

    return vTRUE;
}

// remove - Removes the view from the view tree.
//    Updates the view and prunes parent if this is the last view in a group.

void View::remove() {

    // Make sure I am not the only view in the viewer.

    if (ViewerOnlyView()) {
        myViewer->DisplayMessage ((unsigned char *)"Cannot remove the last application view in a viewer!");
    } 

    // Else, delete the View.

    else {
        // Save a pointer to my grandparent and my Viewer.
        //    (They are needed to set the Current view.)
        View* parent = ParentView;
        Viewer* tmpViewer = myViewer;

            // Don't call delete later because it will cause too much of a delay before the window refreshes.
            // Delete the view immediately.
            //this->DeleteLater();
            delete this;

        // Find a new view to make Current.
                View *newView = parent;
        if (!newView) newView = tmpViewer->layerViews;    // Use the first view.

        while (newView && !newView->hasBorder_) {
            if (newView->ChildView)
                newView = newView->ChildView;

            // but don't use the Viewers layerview's right view as that is the message area.
            else if (newView != tmpViewer->layerViews && newView->RightView) 
                newView = newView->RightView;

            else
                newView = NULL;
        }

        if (newView) newView->MakeCurrent();

        // See if this left an only child.
        if (parent && parent->ChildView &&
            !parent->ChildView->RightView &&
                (tmpViewer->layerViews != parent)) {
            parent = parent->ChildView->RemoveParentView();
        }
    }    
}


// RemoveParentView - Removes the Parent view from the view tree.
View* View::RemoveParentView()
{
    View* newParent = this;

    // Make sure that I have no neighbors.
    if (LeftView || RightView || (ParentView->ChildView != this) ) {
        printf("@RemoveParentView: Attempt to delete a non-orphan view!!!\n");
        newParent = ParentView;
    
    // I am an orphan, I will replace my parent.
    } else {
        // Remove my Pane from whoever owns it.
        DeleteItem(myPane);

        // Delete my Sash.
        RemoveSash();

        // Remove my Parent's Pane and my Sash.
        ParentView->RemovePane();
        
        // Get my parent's Sash.
        if (ParentView->mySash) {
            mySash = ParentView->mySash;
            mySash->SetOwner(this);
            ParentView->mySash = NULL;
        }

        // Internal Check - Our height and width should be the same.
        if ( (myRect.w != ParentView->myRect.w) ||
             (myRect.h != ParentView->myRect.h) ) {
            fprintf(stderr, "@RemoveParentView: Internal Error!!!\n");
            fprintf(stderr, "\t\tmy (%d,%d) != Parent's (%d,%d)\n",
                            myRect.w, myRect.h,
                            ParentView->myRect.w, ParentView->myRect.h);
        }

        // Get my parent's x,y location.
        myRect.x = ParentView->myRect.x;
        myRect.y = ParentView->myRect.y;

        // Update the parent's parent.
        if (ParentView->ParentView &&
            (ParentView->ParentView->ChildView == ParentView) )
                ParentView->ParentView->ChildView = this;

        // Update the parent's owner Pane.
        if (ParentView->myOwnerPane) {
            myOwnerPane = ParentView->myOwnerPane;
            myOwnerPane->childViewTree_ = this;
            ParentView->myOwnerPane = NULL;
        }

        // Update the Viewer's list of Views.
        if (myViewer->layerViews == ParentView)
            myViewer->layerViews = this;

        // Update any new neighbors.
        if (ParentView->LeftView) { 
            LeftView = ParentView->LeftView;
            LeftView->RightView = this;
            ParentView->LeftView = NULL;
        }
        if (ParentView->RightView) {
            RightView = ParentView->RightView;
            RightView->LeftView = this;
            ParentView->RightView = NULL;
        }

        // Save a pointer to the parent.
        // We will need to to delete the parent when we are done.
        View * oldParent = ParentView;

        // Make my old parent's parent my parent.
        ParentView = oldParent->ParentView;

        // Delete the parent view. 
        // But make sure we don't delete stuff we want. (Or us!)
        oldParent->ChildView = NULL;         
        oldParent->ParentView = NULL;
        delete oldParent;

        // Add my pane back to the proper place.
        AppendItem(myPane);

        // Update my Pane.
        UpdatePane();
    }
    return (newParent);
}



// print - Prints out current view hierarchy.
//    Note - This method does not print childViewTree_ info.
void View::print(int level)
{
    // First check for full print.
    if (level == 0) {
        if (myViewer && myViewer->layerViews)
            myViewer->layerViews->print(1);

    // Else print this node and all below it.
    } else {
        // First, index 2 spaces per level.
        // (Well, actually for every level after the first.)
        for (int i=1; i<level; i++)
            fprintf(stderr, "  ");

        // Next, print this node.
        fprintf(stderr, "%x (%d,%d,%d,%d): Parent=%x Left=%x, Right=%x, Child=%x\n",
                this, myRect.x, myRect.y, myRect.w, myRect.h,
                ParentView, LeftView, RightView, ChildView);
        fflush(stderr);

        // Next, print Right neighbors.
        if (RightView) RightView->print(level);

        // Lastly, print any children.
        if (ChildView) ChildView->print(level+1);
    }
}



// SetLeft - Resizes the view to have the left side at the given location.
//    If the new left is beyond the old right
//        This view's size is reduced to zero.
//        Adjust the Right neighbor's Sash.
void View::SetLeft (const int newLeft)
{
    // If the new left side is beyond the old right side, ...
    // ... Set our width to 0 and see if our right neighber needs to move.
    int oldRight = myRect.x + myRect.w;
    if (newLeft > oldRight) {
        // If this is a Horizontal Group and ...
        //  ... our right neighbor has a sash, move it too.
        if (ParentView &&
            (ParentView->view_type == HorizontalGroupView) &&
            RightView && RightView->GetSash() ) {
                RightView->GetSash()->SetX(newLeft + SASH_SIZE +
                                           ParentView->GetAbsoluteX(), gside_left);
            }

        // We move our left side and set our width to zero.
        MoveView(newLeft, myRect.y, 0, myRect.h);

    // Else, Resize to the new left side and width.
    } else {
        int newWidth = myRect.w - (newLeft - myRect.x);
        MoveView(newLeft, myRect.y, newWidth, myRect.h);
    }
}



// SetRight - Resizes the view to have the right side at the given location.
//    If the new right is beyond the old left,
//        This view's size is reduced to zero.
//        See if the left neighbor's right side needs to be adjusted.
void View::SetRight (const int newRight)
{
    // If the new X is beyond the old left side, ...
    // ... Set width to Zero and see if our left neighber needs to move.
    if (newRight < myRect.x) {
        // If this is a Horizontal Group and ...
        //  ... the left neighbor has a sash, adjust his right side.
        if (ParentView &&
            (ParentView->view_type == HorizontalGroupView) &&
            LeftView && LeftView->GetSash() ) {
            LeftView->SetRight(newRight - SASH_SIZE + ParentView->GetAbsoluteX());
        }

        // Move to new location with a Zero width.
        MoveView(newRight, myRect.y, 0, myRect.h);

    // Else, move the view.
    } else {
        int newWidth = newRight - myRect.x;
        MoveView(myRect.x, myRect.y, newWidth, myRect.h);
    }
}



// SetTop - Resizes the view to have the top at the given location.
//    If the new top is beyond the old bottom, 
//        This view's size is reduced to zero.
//        If appropriate, the Right neighbor is adjusted.
void View::SetTop (const int newTop)
{
    // If the new top is below the old bottom, ...
    // ... Set height to Zero, and See if right neighbor needs to be adjusted.
    if (newTop < myRect.y) {
        // If this is a Vertical Group and ...
        //  ... the right neighbor has a sash, move it too.
        if (ParentView &&
            (ParentView->view_type == VerticalGroupView) &&
            RightView && RightView->GetSash() ) {
            RightView->GetSash()->SetY(newTop - SASH_SIZE +
                                       ParentView->GetAbsoluteY(), gside_top);
        }

        // Move to the new location with a zero height.
        MoveView(myRect.x, newTop, myRect.w, 0);

    // Else, move the view.
    } else {
        int newHeight = newTop - myRect.y;
        MoveView(myRect.x, myRect.y, myRect.w, newHeight);
    }
}



// SetBottom - Resize the view to have the bottom at the given location.
//    If the new Bottom is beyond the old Top, 
//        This view's size is reduced to zero.
//        If appropriate, the Left neighbor is also adjusted.
void View::SetBottom (const int newBottom)
{
    // If the new bottom is beyond the old top, ...
    // ... Set height to Zero and See if left neighbor needs to be adjusted..
    if (newBottom > myRect.y + myRect.h) {
        // If this is a Vertical Group and ...
        //  ... the left neighbor has a sash, move it too.
        if (ParentView &&
            (ParentView->view_type == VerticalGroupView) &&
            LeftView && LeftView->GetSash() ) {
            LeftView->GetSash()->SetY(newBottom + SASH_SIZE +
                      ParentView->GetAbsoluteY(), gside_bottom);
        }

        // Move to the new location with a zero height.
        MoveView(myRect.x, newBottom, myRect.w, 0);

    // Else, move the view.
    } else {
        int newHeight = myRect.h - (newBottom - myRect.y);
        MoveView(myRect.x, newBottom, myRect.w, newHeight);
    }
}

  

// UpdatePane - Sets the pane's rectangle to match that of the view.
//    Also checks for an editor, but that is currently disabled.
void View::UpdatePane()
{
    if (myPane) {
        myPane->SetLocation(myRect.x ,myRect.y, myRect.w, myRect.h);
        if (editor_enabled()) {
            printf("@View::UpdatePane:Emacs Disabled!!!\n");
        }
    }
}



// GetDraggedY - Returns Y value adjusted for other Sash sizes.
//    This is called by a horizontal sash after it is dragged.
int View::GetDraggedY(int newY, int oldY, sideType)
{
    int adjustedY = newY;   // Will probably return the new Y value.

    // Get the height of my sash.
    int sashHeight = mySash->GetRect()->GetHeight();

    // If moving up, make sure there is room for sashes above us.
    if (newY > oldY) {  // Moving up.
        // Start at the top but leave room for my sash and my border.
        int maxY = ParentView->myRect.h - sashHeight - BORDER;

        // Leave room for each of the my neighbors.
        for (View* v=this; v->LeftView; v=v->LeftView)
            maxY -= sashHeight;

        // Make sure the new Y has enough room for the sashes.
        if (newY > maxY) {
            adjustedY = maxY;
        }

    // Else if moving down, make sure there is room for sashes on the right.
    } else if (newY < oldY) {  // Moving down.
        // The minimum starts with the Border size.
        int minY = BORDER;

        // Leave room for each of my downstairs neighbors.
        for (View* v=this; v->RightView; v=v->RightView)
            minY += sashHeight;

        // Make sure the new Y has enough room for the sashes.
        if (newY < minY) {
            adjustedY = minY;
        }

    // Otherwise, we didn't move and we don't need to do anything.
    } else
        adjustedY = newY;

    return(adjustedY);
}



// GetDraggedX - Returns X value adjusted for other Sash sizes.
//    This is called by a horizontal sash after it is dragged.
int View::GetDraggedX(int newX, int oldX, sideType side)
{
    int adjustedX = newX;
    int sashWidth = mySash->GetRect()->GetWidth();

    // If moving left, adjust for sashes on the left.
    if (newX < oldX) {  // Moving left.
        // The min X starts with the Border.
        int minX = BORDER;
        for (View* v=this; v->LeftView; v=v->LeftView)
            minX += sashWidth;

        // Make sure there is enough room for all the sashes.
        if (newX < minX) {
            adjustedX = minX;
        }

    // Else if moving right, adjust for sashes on the right.
    } else if (newX > oldX) {  // Moving right.
        // The max X starts at the far right less my sash and Border and
        //    another sashs. (No, I don't know why.)
        int maxX = ParentView->myRect.w - sashWidth - BORDER - sashWidth;
        // Decrement the max X for each sash.
        for (View* v=this; v->RightView; v=v->RightView)
            maxX -= sashWidth;

        // Make sure there is enough room for the sashes.
        if (newX > maxX) {
            adjustedX = maxX;
        }

    // Otherwise, we didn't move and we don't need to do anything.
    } else
        adjustedX = newX;

    return(adjustedX);
}



// MakeCurrent - Tells the Viewer that this is the current view.
void View::MakeCurrent() 
{
    if ((ParentView) && (Unsplitable_))
        ParentView->MakeCurrent();
    else
        myViewer->SetCurrentView (id); 

    Layer* layer = GetLayer();
    if (layer) {
            gString str;
        gString title;
        layer->getName(str);
        title = "DIScover - ";
        title += (vchar*) str;
        myViewer->SetTitle (title);
    } else myViewer->SetTitle ((vchar*)"DIScover");
    
    Viewer::SetExecutingView(myViewer, this);

    if (interp_trace) {
        //printf ("Interp in %d[%d]\n", myViewer, this);
    }
}



// SetPane - Loads in the new pane.
//    Multi-view panes have a special init.
//    Non-multi-view panes are added to the current pane.
void View::SetPane(Pane* newPane)
{

    const vchar* paneTagName = newPane->GetTag();

    // If this is a Multi-View pane, load it.
    if ( paneTagName && 
         ((*paneTagName == '|') || (*paneTagName == '-')) ) {
        LoadMultiView(newPane);

    // Else, load in the view and set the pane.
    } else {
        // If I already have a pane, remove it from my item list.
        RemovePane();

        // Init Unsplitable and Border flags.
        if (paneTagName && (*paneTagName == *UNSPLITABLE) ) {
            Unsplitable_ = vTRUE;
            hasBorder_ = vFALSE;
        } else {
            Unsplitable_ = vFALSE;
            hasBorder_ = vTRUE;
        }

        // Attach the new pane.
        myPane = newPane;
        myPane->AttachView(this);
        UpdatePane();
        myPane->Open();
        AppendItem(myPane);

        // Load in the content list, if it is in the resource file.
        if (paneTagName)
            LoadContentList(paneTagName);
    }
}



//  UnsetPane - Removes myPane so a layer can put it somewhere else.
void View::UnsetPane()
{
    if (myPane) {
        if (ChildView)  {
            printf("Uh oh, Trying to UnSet Pane when there are children!!!\n");
            myPane->SetViewBranch(ChildView, view_type);
            ChildView = NULL;
            view_type = UnknownView;
        }
        myPane->HideWindow();
        //Layer* l = myPane->GetOwner()->GetLayer();
        //if (l->isEditor())
            //((editorLayer*)l)->closeTextData();
        myPane->Close();
        DeleteItem(myPane);
        // Note - No need to delete the pane because a layer 'owns' it.
        myPane = NULL;
        Pane* newPane = new Pane;
        SetPane(newPane);
    }
}



// LoadMultiView - Loads in multi-views.
//    It splits the current view based on the panes within the new pane.
void View::LoadMultiView(Pane* newPane)
{
    // Find out if this is a horizontal or verticle grouping.
    //    This is done by checking the first char.
    int isHorizontal = vFALSE;
    const vchar *tagName = newPane->GetTag();
    if (*tagName == '|')
        isHorizontal = vTRUE;
    else if (*tagName != '-') {    // Uh oh.
        printf("@LoadMutliView:Invalid tag name '%s' given!!!\n",
               tagName);
    }

    // Skip past the horizontal/verticle marker.
    tagName++;

    // Add a View tree to my Pane.
    myPane->GrowViewTree();

    View* v = NULL;
    View* nextView;

    Pane* subPane;
    int ratio;
        
    // Set up the first sup-pane.
    if (isHorizontal) {    // Load in next horizontal pane.
        subPane = newPane->FindSmallestXElt(&ratio);
        myPane->childViewTree_->split_vertical(ratio);
    } else {        // Load in next vertical pane.
        subPane = newPane->FindLargestYElt(&ratio);
        myPane->childViewTree_->split_horizontal(ratio);
    }
    // Remove the item from the new Pane.
    newPane->DeleteItem(subPane);


    // Recursively add the new pane to the view tree.
    myPane->childViewTree_->ChildView->SetPane(subPane);
    v = myPane->childViewTree_->ChildView->RightView;
        
    // Loop through all remaining sub-panes.
    while (newPane->GetItemCount()) {  // While there are still more panes
        if (isHorizontal)
            subPane = newPane->FindSmallestXElt(&ratio);
        else
            subPane = newPane->FindLargestYElt(&ratio);
        // Remove the item from the new Pane.
        newPane->DeleteItem(subPane);

        if (newPane->GetItemCount())
            v->addPane(ratio);  // Add a new view
        nextView = v->RightView;

        v->SetPane(subPane);      // Load the new pane into the new view
        if (nextView)
            v = nextView;
    }

    // Update this pane.
    if (*tagName == *UNSPLITABLE) {
        Unsplitable_ = vTRUE;
        hasBorder_ = vFALSE;
    } else {
        Unsplitable_ = vFALSE;
        hasBorder_ = vTRUE;
    }
    UpdatePane();
    MakeCurrent();

    // Resize all me and my neighbors.
    if (isHorizontal)
       v->ParentView->ResizeChildrenHorz();
    else
       v->ParentView->ResizeChildrenVert();
}



// LoadContentList - Loads in the pane content list if found.
//    Load in the Content List, if found.
//    The name of the content list is the Pane's tag name with
//        LIST_STRING added to the end.
//   This function has no return value.
void View::LoadContentList(const vchar * paneTagName)
{
    if (paneTagName) {
        vchar buffer[TCL_STRING_SIZE];
        vcharCopy(paneTagName, buffer);
        vcharAppendFromLiteral(buffer, LIST_STRING);
    
        vresource res;
        if (getResource(buffer, &res) ) {
            vliststr* itemList = new vliststr(res);
                
            // Loop through the lines in the content list.
            for (int x = 0; x < itemList->GetRowCount(); x++) {
                vcharCopyScribed ((itemList->ScribeCellValue(x,1)),buffer);
                vcharAppendFromLiteral(buffer, " {");
                // Selection command.
                vcharAppendScribed(buffer, itemList->ScribeCellValue(x,2));
                vcharAppendFromLiteral(buffer, "} {");
                // Selection command.
                vcharAppendScribed(buffer, itemList->ScribeCellValue(x,3));
                vcharAppendFromLiteral(buffer, "}");
                Tcl_Eval (interp(), (char*)buffer);  // Init the item.
            }
            itemList->DeleteLater();
        }
    }
}



// find_item_recurs - Calls the container's FindItem method.
vdialogItem* find_item_recurs(vdialogItemList* itemList, const vname* tagName)
{
  vdialogItem * result = NULL;

    // Loop through all items in this container.
    for (int i=0; i<itemList->GetCount() && !result; i++) {
        vobjectAttributed* vo = itemList->GetAt(i); // Get next object.

        // If it is a notebook, recursively check its items.
        if (vo->IsKindOf(&vnotebook::Kind)) {
            vdialogItemList* il = vnotebook::CastDown(vo)->GetItemList();
            result = find_item_recurs(il, tagName);
#if 0
           // I don't know why this Notebook page stuff isn't used.
           // If not found yet, check in this Notebook page.
           if (!result) {    
               vnotebookPage* page;
               page = vnotebook::CastDown(vo)->DetermineCurrentPage();
               result = page->FindItem(tagName);
            }
            // If still not found, recursively check this page.
            if (!result)
                result = find_item_recurs(page->GetItemList(), tagName);
#endif
        // Else if it is a container, look in it.
        } else if (vo->IsKindOf(&vcontainer::Kind)) {
            vcontainer* container = vcontainer::CastDown(vo);
            const vchar* containerTagName = container->GetTag();

            if (containerTagName && (vcharCompare(container->GetTag(),tagName) == 0))
                result = container;
            else result = container->FindItem(tagName);

            // If not found, recursively check the container.
            if (!result) {
                vdialogItemList* item_list = container->GetItemList();
                result = find_item_recurs(item_list, tagName);
            }
        }
    }    // End of item loop.

  return result;
}



// FindItem - Returns a pointer to the item matching the tag or NULL.
vdialogItem* View::FindItem(const vchar* tag)
{
    vdialogItem * result;

    // Convert tag into a  valid  tag name.
    const vname * name = vnameInternGlobal(tag);

    // First look in the view's Pane.
    if (myPane)
        result = myPane->FindItem(name);

    // If still not found, check the view's children.
    if (!result && ChildView) 
        for (View* v = ChildView; v && !result; v = v->RightView)
            result = v->myPane->FindItem(name);

    // If still not found, check the view's Pane's children.
    if (!result && myPane && myPane->childViewTree_)
        result = myPane->childViewTree_->FindItem(tag);

    // Finally, check the view's Pane's ItemList.
    if (!result && myPane)
        result = find_item_recurs(myPane->GetItemList(), name);

    return (result);
}



// MoveView - Moves the view to the new location.
//    It also adjusts the sizes of its children.
void View::MoveView(int x, int y, int w, int h)
{
    // Adjust the View's rectangle.
    myRect.x = x;
    myRect.y = y;
    myRect.h = h;
    myRect.w = w;

    // Update the X and Y percentages.
    if (ParentView) {
        percentX_ = ((float)myRect.w * (float)100.0) / (float)ParentView->myRect.w;
        percentY_ = ((float)myRect.h * (float)100.0) / (float)ParentView->myRect.h;
    } else {
        percentX_ = (float)100.0;
        percentY_ = (float)100.0;
    }
    
    // Update the view's Pane.
    in_resize = vTRUE;  
    UpdatePane();
    in_resize = vFALSE;

    // Update the view's children.
    if (ChildView) {
        if (view_type == HorizontalGroupView)
            ResizeChildrenHorz();
        else
            ResizeChildrenVert();
    }
}


 
// ResizeView - Resizes the view to use the given lower left corner.
//    Resises itself and all its children.
//    Corrects for small error when multiplying percentages by making
// sure that sum of childrens size matches the parent.
void View::ResizeView (int x, int y, int w, int h)
{
        if (w != -1) myRect.w = w;
        if (h != -1) myRect.h = h;
        ResizeView(x, y,0);
}

void View::ResizeView(int x, int y, int force_resize)
{
    myRect.x = x;    // Set X.
    myRect.y = y;    // Set Y.

    // Use the parent if it exists.
    if (ParentView) {          
        if (ParentView->view_type == HorizontalGroupView) {  // Horizontal
            // Set height.
            myRect.h = ParentView->myRect.h;

            // Set width.
            if (!RightView) 
                myRect.w = ParentView->myRect.w - myRect.x;
            else
                myRect.w = (int)(ParentView->myRect.w * percentX_ / 100.0);

            // See if height and width need to be adjusted.
            if (ParentView->hasBorder_ || ParentView->myOwnerPane)  {
                myRect.h -= BORDER;
                if (!RightView) 
                    myRect.w -= BORDER;
            }
        } else {    // Vertical
            // Set width.
            myRect.w = ParentView->myRect.w;

            // Set height.
            if (!LeftView) 
                myRect.h = ParentView->myRect.h - myRect.y;
            else if (!vLock_)
                myRect.h = (int)(ParentView->myRect.h * percentY_ / 100.0);
            // otherwise, don't change height, so leave it as is

            // See if height and width need to be adjusted.
            if (ParentView->hasBorder_ || ParentView->myOwnerPane)  {
                if (!LeftView)
                    myRect.h -= BORDER;
                myRect.w -= BORDER;
            }
        }

    // Else use the owner's pane if it exists.
    } else if (myOwnerPane) {
        myRect.w = myOwnerPane->rect.w - BORDER;
        myRect.h = myOwnerPane->rect.h - BORDER;

    // Else use the viewer.
    } else {
                if (force_resize) {
                myRect.w = myViewer->rect.w;
                myRect.h = myViewer->rect.h;
                }
    }
    
    in_resize = vTRUE;  
    UpdatePane();
    in_resize = vFALSE;

    // Resize any children.
    if (ChildView) {
        if (view_type == HorizontalGroupView)
            ResizeChildrenHorz();
        else
            ResizeChildrenVert();
    }
}



// ResizeChildrenHorz - Horizontal resizing of all children.
//    Remember, a horizontal group has vertical sashes.
void View::ResizeChildrenHorz()
{

    // Loop through all the children.
    int newX = 0;
    for (View* v = ChildView; v; v = v->RightView) {
        // Resize the child.
        v->ResizeView(newX, 0);

        // Adjust the sash rectangle.
        if (v->mySash) {
            vrect sashRect = *(v->mySash->GetRect());
            sashRect.SetX(v->myRect.x - SASH_SIZE);
            sashRect.SetY(v->myRect.y);
            sashRect.SetHeight(v->myRect.h);
            v->mySash->SetRect(&sashRect);
        }
        if (v->RightView)    // Get next x.
            newX = v->myRect.x + v->myRect.w + SASH_SIZE;
    }
}


// ResizeChildrenVert - Verticle resizing of all children.
//    Remember, a vertical group has horizontal sashes.
void View::ResizeChildrenVert()
{
    // Find the last child.
    View* lastChild;
    for (lastChild = ChildView; lastChild->RightView;
         lastChild = lastChild->RightView);
    
    // Loop back through all the children.
    vrect sashRect;
    int newY = 0;
    for (View* v = lastChild; v; v = v->LeftView) {
        // Resize the child.
        v->ResizeView(0, newY);

        if (v->mySash) {    // Update the sash.
            sashRect = *(v->mySash->GetRect());
            sashRect.SetX((v->myRect.x));
            sashRect.SetY((v->myRect.y + v->myRect.h));
            sashRect.SetWidth(v->myRect.w);
            v->mySash->SetRect(&sashRect);
        }

        // Get the next y.
        if (v->LeftView) 
            newY = v->myRect.y + v->myRect.h + SASH_SIZE;
    }
}



// GetAbsoluteX - Returns combined x value for this and parent views.
int View::GetAbsoluteX()
{
    int x = myRect.x;

    View* nextView = NULL;
    if (ParentView)
        nextView = ParentView;
    else if (myOwnerPane)
        nextView = myOwnerPane->GetOwner();

    while (nextView) {
        x += nextView->myRect.x;

        if (nextView->ParentView)
            nextView = nextView->ParentView;
        else if (nextView->myOwnerPane &&
                (nextView->myOwnerPane->GetOwner() != nextView) )
            nextView = nextView->myOwnerPane->GetOwner();
        else
            nextView = NULL;
    }

    return(x);
}



// GetAbsoluteY - Returns combined y value for this and parent views.
int View::GetAbsoluteY()
{
    int y = myRect.y;

    View* nextView = NULL;
    if (ParentView)
        nextView = ParentView;
    else if (myOwnerPane)
        nextView = myOwnerPane->GetOwner();

    while (nextView) {
        y += nextView->myRect.y;

        if (nextView->ParentView)
            nextView = nextView->ParentView;
        else if (nextView->myOwnerPane &&
                (nextView->myOwnerPane->GetOwner() != nextView) )
            nextView = nextView->myOwnerPane->GetOwner();
        else
            nextView = NULL;
    }

    return(y);
}



// SetInterpVariable - Sets the TCL variable to the given value.
void View::SetInterpVariable (const vchar* variable, const vchar* value)
{
    if (variable && value) {
        char * tclVar = (char *) vcharExportAlloc(vcharScribeSystemExport(variable));
        char * tclValue =  (char *) vcharExportAlloc(vcharScribeSystemExport(value));
        Tcl_SetVar(interp(), tclVar, tclValue, TCL_GLOBAL_ONLY);
        vcharExportFree(tclVar);
        vcharExportFree(tclValue);
    }
}



// GetInterpVariable - Returns TCL value of the given variable.
vchar* View::GetInterpVariable (vchar* variable)
{
vchar *  value = NULL;

    if (variable)
        value = (vchar *) Tcl_GetVar(interp(), (char *) variable,
                                      TCL_GLOBAL_ONLY);
    return value;
}



// interp - Returns the TCL interpreter for this view.
Tcl_Interp* View::interp()
{
    // Will default to this view's interpreter.
    Tcl_Interp* interp = myInterp;

    // Check if parent's interpreter is needed.
    if ((Unsplitable_) && (ParentView)) {
        interp = ParentView->interp();

    // Check if owner's interpreter is needed.
    } else if (myOwnerPane && myOwnerPane->GetOwner()) {
        interp = myOwnerPane->GetOwner()->interp();

    } else {
        Layer*  lp = GetLayer();
        if (lp) {
            Tcl_Interp* layerInterp = lp->getInterp();
            if (layerInterp)
                interp = layerInterp;
        }
    }


    return interp;
}



// SetState - Sets the state of the view.
void View::SetState(const vchar * newState)
{
    gInit(View::SetState);

    if (newState)
        vcharCopyBounded(newState, myState, MAX_STATE_LEN);
    else
        vcharCopyBounded(NO_STATE, myState, MAX_STATE_LEN);
}



// GetState - Updates the state of the view and returns it.
vchar* View::GetState()
{
    Layer * l = GetLayer();
    if (l)
        SetState(l->getState());
    else
        SetState(NO_STATE);

    return myState;
}



// UpdateColors - Sets colors for current, neighbor, and child views.
void View::UpdateColors()
{
  if (mySash) {
#ifdef OWNCOLOR
         mySash->SetForeground(Viewer::GetSashForeground());
     mySash->SetBackground(Viewer::GetSashBackground());
#endif
  }

  if (myPane) {
#ifdef OWNCOLOR
        vcolor* fg = myPane->GetForeground();
    vcolor* bg = myPane->GetBackground();

    myPane->SetForeground (fg ? fg : Viewer::GetPaneForeground());
    myPane->SetBackground (bg ? bg : Viewer::GetPaneBackground());
#endif
  }

  if (RightView)
    RightView->UpdateColors();

  if (ChildView)
    ChildView->UpdateColors();
}



// DrawBorders - Draws all borders.
//    First draws its own border, then does all of its children.
void View::DrawBorders()
{
    if (myPane)
        myPane->DrawBorder();    // Draw my border.

    for (View* v = ChildView; v; v = v->RightView)
        v->DrawBorders();    // Draw border on child.
}



// FindGroupParent - Parent is either this view or the parent's parent.
View* View::FindGroupParent()
{
View * parent;

    if (Unsplitable_ && ParentView)
        parent = ParentView->FindGroupParent();
    else
        parent = this;

    return (parent);
}



// Deletetem - Deletes the item from the view.
void View::DeleteItem(vdialogItem* itemToDelete)
{
  vdialog *dialog = NULL;
  if(GetViewer())
    dialog = GetViewer()->GetDialog();
  else
    if(myViewer)
      dialog = myViewer->dialog;
  if(dialog != NULL)
    dialog->SetFocusItem(NULL);
  Pane *container = NULL;
  if(myOwnerPane)
    container = myOwnerPane;
  else if(ParentView && ParentView->myPane)
    container = ParentView->myPane;
  
  if(container){
    container->GetItemList()->ChangedAttributeValue(vname_CurrentConfirm, itemToDelete, NULL);
    container->GetItemList()->SetDragItem(NULL);
  }
  if(ParentView && ParentView->myPane)
    ParentView->myPane->GetItemList()->ChangedAttributeValue(vname_CurrentConfirm, itemToDelete, NULL);
    

  if(GetViewer() && GetViewer()->GetDialog())
    GetViewer()->GetDialog()->SetFocusItem(NULL);

  if (myOwnerPane)
    myOwnerPane->DeleteItem(itemToDelete);
  else if (ParentView && ParentView->myPane)
    ParentView->myPane->DeleteItem(itemToDelete);
  else if (myViewer && myViewer->dialog)
    myViewer->dialog->DeleteItem(itemToDelete);
}




// AppendItem - Adds the item to the view.
void View::AppendItem(vdialogItem* itemToAppend)
{
    if (myOwnerPane) 
        myOwnerPane->AppendItem(itemToAppend);
    else if (ParentView)
        ParentView->myPane->AppendItem(itemToAppend);
    else
        myViewer->dialog->AppendItem(itemToAppend);
}



void View::refresh()
{
    if (myPane)
        myPane->Inval(vwindowINVAL_IMMEDIATE);
}

int  Pane::GetWindow() {
    gInit(Pane::GetOwner); return window_ ? window_->xwindow : 0; }
int  Pane::GetSubwin() {
    gInit(Pane::GetOwner); return window_ ? window_->xwindow : 0; }


/**********     end of view.C     ***********/
