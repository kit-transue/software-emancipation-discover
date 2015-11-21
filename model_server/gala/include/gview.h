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
#ifndef _gview_h
#define _gview_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vcontainerINCLUDED
#include vcontainerHEADER
#endif

#ifndef vkindINCLUDED
#include vkindHEADER
#endif

#ifndef _TCL
   #include <tcl.h>
#endif

#ifndef _generic_h
   #include "ggeneric.h"
#endif
#ifndef _rect_h
   #include "grect.h"
#endif

#ifdef NEW_UI
#ifndef _sash_h
   #include "gsash.h"    // Needed only for sideType.
#endif
#endif


class Viewer;
class Sash;
class subwin;
class Layer;


/*  The types of possible views */

typedef enum {
    UnknownView 	=1,
    DialogView          =2,
    GraphicView         =4,
    EditorView          =8,
    ListView            =16,
    HorizontalGroupView = 32,
    VerticalGroupView   = 64
} ViewType;

/* The types of representations */

typedef enum {
    UnknownRep,
    Text,
    Outline,
    FlowChart,
    Tree
} RepType;





/* The graphical representation of the the view */

class Pane : public vcontainer
{
    friend class View;
  public:
    vkindTYPED_FULL_DECLARATION (Pane);
    vloadableINLINE_CONSTRUCTORS (Pane, vcontainer);

    void Init (void);
    void LoadInit (vresource);
    void Destroy (void);
    void doInit (void);

    void SetViewBranch(View*, ViewType);
    void GrowViewTree();
    virtual int HandleButtonDown(vevent *event);
    void AttachView(View* myOwner);
    View *GetOwner() ;
    Pane* FindSmallestXElt(int*);
    Pane* FindLargestYElt(int*);
    virtual void Draw();
    void Inval(int);
    void DrawBorder();
    void SetLocation (int, int, int, int);
    void OpenWindow();
    int GetWindow();
    void HideWindow();
    int GetSubwin();
    View* GetChildViewTree();
    int isEmptyPane();

    float m_Ratio; //used in panes separated my adjustable sash
    
  private:
    View* myOwner;    // A link to the owner of this pane
    View* childViewTree_;
    subwin* window_;
    ViewType ownerViewType;
    View* viewTreeBranch;
    vrect invalRect_;
    int invalid_;
    int emptyPane;
};


class View : public vkindTyped
{
    friend class Pane;
    friend class Sash;
    friend class Viewer;
  public:
    View(View*, Viewer*, Rect, ViewType vtype=UnknownView,
         vchar* tagName = NULL);
    ~View();
    View* find_view (int id);
    View* find_view_for_layer ();
    View* find_target_view ();
    void close();
    void target(int);
    void remove();
    void split_horizontal(int ratio = 50);
    void split_vertical(int ratio = 50);
    void addPane (int ratio = 50);
    void refresh();
    void print(int level=0);
    void UpdatePane();
    int GetId();
    int GetHeight();
    void SetWidth(const int);
    void SetHeight(const int);
    Viewer *GetViewer();
    void MakeCurrent();
    void SetPane (Pane*);
    Pane *GetPane();
    void UnsetPane();
    vdialogItem* FindItem(const vchar*);
    void ResizeView (int,int,int force_resize=1);
    void ResizeView (int,int,int,int);
    void GetWindowInfo (int& window, int& vid,
			int& x_off, int& y_off, int& width, int& height);
    void editor_enabled(int f);
    int editor_enabled();
    vchar* GetState();
    void SetState(const vchar*);
    Layer* GetLayer();
    void SetLayer(Layer*);
    void DrawBorders();
    View* FindGroupParent();
    void UpdateColors();

    void SetInterpVariable(const vchar*, const vchar*);
    vchar* GetInterpVariable(vchar*);
    Tcl_Interp* interp();
    int ViewerOnlyView();

    View* GetParentView();

  protected:
    int Unsplitable_;
    int hasBorder_;  
    int vLock_, hLock_;

  private:
    void MoveView (int, int, int, int);
    Rect GetRect();
    View* GetLeftView();
    View* GetChildView();
    ViewType GetViewType();
    void SetViewType(ViewType);
    Sash *GetSash();
    void RemoveSash();
    void RemovePane();
#ifdef NEW_UI
    int GetDraggedX(int, int, sideType side=gside_left);
    int GetDraggedY(int, int, sideType side=gside_top);
#endif
    void SetTop(const int);
    void SetBottom(const int);
    void SetLeft(const int);
    void SetRight(const int);
    int GetAbsoluteX();
    int GetAbsoluteY();
    int GetX();
    int GetY();
    int GetWidth();
    void AddParentView(ViewType vtype=UnknownView);
    View* RemoveParentView();
    void AddRightView();
    void LoadMultiView(Pane*);
    void LoadContentList(const vchar*);
    void ResizeChildrenHorz ();
    void ResizeChildrenVert ();
    void DeleteItem(vdialogItem*);
    void AppendItem(vdialogItem*);

    int id;			// the unique id of this view
    View* ParentView;  		// Link to parent view.
    View* ChildView;            // Link to any children.
    View* LeftView;             // Link to left neighbor.
    View* RightView;            // Link to right neighbor.
    Rect myRect;     		// The boundaries of the view
    ViewType view_type;         // The type of view
    RepType rep_type;		// The type of representation
    int targ; 			// Kevin
    Viewer* myViewer;           // A link to the viewer
    Pane* myPane;               // A link to the pane, the graphical rep. of this view
    Sash* mySash;		// A link to the sash, which separates adjacent views
    Tcl_Interp* myInterp;	// a tcl interpreter for the view
    int editor_on;		// Kevin
    float percentX_, percentY_; // The relative position of the view before being resized
    vchar myState[1024];
    Layer* myLayer;
    Pane* myOwnerPane;
};




//***************** Inline functions for class Pane *************//

inline int Pane::isEmptyPane() {
    gInit(Pane::isEmptyPane); return emptyPane; }
inline View * Pane::GetOwner() {
    gInit(Pane::GetOwner); return myOwner; }
inline View* Pane::GetChildViewTree() {
    gInit(Pane::GetOwner); return childViewTree_; }




//***************** Inline functions for class View *************//

inline int View::GetId() {
    gInit(View::GetId); return id; }
inline Rect View::GetRect() {
    gInit(View::GetRect); return myRect; }
inline int View::GetX() {
    gInit(View::GetX); return myRect.x; }
inline int View::GetY() {
    gInit(View::GetY); return myRect.y; }
inline int View::GetWidth() {
    gInit(View::GetWidth); return myRect.w; }
inline void View::SetWidth(const int newWidth) {
    gInit(View::SetWidth); myRect.w = newWidth; }
inline int View::GetHeight() {
    gInit(View::GetHeight); return myRect.h; }
inline View* View::GetParentView() {
    gInit(View::GetParentView); return ParentView; }
inline View* View::GetLeftView() {
    gInit(View::GetLeftView); return LeftView; }
inline View* View::GetChildView() {
    gInit(View::GetChildView); return ChildView; }
inline ViewType View::GetViewType() {
    gInit(View::GetViewType); return view_type; }
inline void View::SetViewType(ViewType newType) {
    gInit(View::SetViewType); view_type = newType; }
inline Sash * View::GetSash() {
    gInit(View::GetSash); return mySash; }
inline Viewer * View::GetViewer(){
    gInit(View::GetViewer); return myViewer; }
inline Pane* View::GetPane() {
    gInit(View::GetPane); return myPane; }

inline void View::editor_enabled(int f) {
    gInit(View::editor_enabled); editor_on = f; }
inline int View::editor_enabled() {
    gInit(View::editor_enabled); return editor_on; }

inline void View::SetLayer(Layer* l) {
    gInit(View::SetLayer); myLayer = l; }
inline Layer* View::GetLayer() {
    gInit(GetLayer); return myLayer; }

#endif
