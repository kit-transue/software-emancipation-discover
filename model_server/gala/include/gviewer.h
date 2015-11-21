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
/**************************************************************************
 * gviewer.h
 * by: Nate Kushman
 * viewers are our implimentation of windows underneath galaxy
 *
 *************************************************************************/


#ifndef _gviewer_h
#define _gviewer_h

#include <vport.h>
#include vcolorHEADER
#include vcolorchsrHEADER

#include <ginterp.h>
#include <gdialog.h>
#include <grect.h>

#ifndef vprintINCLUDED
#include vprintHEADER
#endif

class ToolBar;
class View;
class CommandLineView;
class MyDialog;
class ViewerMenuBar; 
class ViewerPopupMenuList;
class ViewerPopupMenu;
class ColorDialog;
class GDialog;

class Viewer
{
    
friend class View;
friend class GDialog;
    
  public:
    Viewer();
    virtual ~Viewer();
 
    static void InitBufferedColors();

    void LoadControlViews(const vrect*);
    
    View* find_target_view();
    View* find_view_for_layer ();
    View* find_view (int id);
    ViewerPopupMenu *findPopupMenu (const char *);
    static Viewer* find_viewer (int id);
    
    static int CMD_ChangeView(ClientData, Tcl_Interp*, int, char**);
    static int CMD_Close(ClientData, Tcl_Interp*, int, char**);
    static int CMD_CloseView(ClientData, Tcl_Interp*, int, char**);
    static int CMD_Create(ClientData, Tcl_Interp*, int, char**);
    static int CMD_OpenLayout(ClientData, Tcl_Interp*, int, char**);
    static int CMD_Print(ClientData, Tcl_Interp*, int, char**);
    static int CMD_RemoveView(ClientData, Tcl_Interp*, int, char**);
    static int CMD_SaveLayout(ClientData, Tcl_Interp*, int, char**);
    static int CMD_Split(ClientData, Tcl_Interp*, int, char**);
    static int CMD_Add(ClientData, Tcl_Interp*, int, char**);
    static int CMD_ShowItem(ClientData, Tcl_Interp*, int, char**);
    static int CMD_HideItem(ClientData, Tcl_Interp*, int, char**);
    static int CMD_ShowMenu(ClientData, Tcl_Interp*, int, char**);
    static int CMD_HideMenu(ClientData, Tcl_Interp*, int, char**);
    static int CMD_EnableItem(ClientData, Tcl_Interp*, int, char**);
    static int CMD_DisableItem(ClientData, Tcl_Interp*, int, char**);
    static int CMD_EnableMenu(ClientData, Tcl_Interp*, int, char**);
    static int CMD_DisableMenu(ClientData, Tcl_Interp*, int, char**);
    static int CMD_SetToggle(ClientData, Tcl_Interp*, int, char**);
    
    void bring_to_top();

    void SetTitle(const vchar*);
    
    void refresh();
    
    void print(int level=0);
    
    int GetViewerCode();
    int GetViewCode();
    void SetCurrentView (int);

    ViewerMenuBar *GetMenuBar ();
    ViewerPopupMenuList *GetPopupList ();

    MyDialog *GetDialog();
    View *GetView();
    ToolBar *GetToolBar();
    void ResizeWindow(float, float, int , int);
    void DisplayMessage(vchar*);
    void DisplayQuickHelp(vchar*);
    
    static void GetExecutingView (Viewer*& vr, View*& v);
    static Tcl_Interp* GetExecutingInterp();
    static void SetExecutingView (Viewer* vr, View* v);

    static void SaveColor (vcolorSpec *, const vname*, const vname*, vresource);
    static void SaveDialogColors (vcolor *, vcolor *, vresource);
    static void SaveSashColors (vcolor *, vcolor *, vresource);
    static void SavePaneColors (vcolor *, vcolor *, vresource);
    static void SaveMenuColors (vcolor *, vcolor *, vresource);
    static void SaveCommandLineColors (vcolor *, vcolor *, vresource);
    static void SaveToolBarColors (vcolor *, vcolor *, vresource);
    static void SaveProgressColors (vcolor *, vcolor *, vresource);
    static void SaveCurrentPaneBorderColor (vcolor *, vresource);
    static void SaveTargetPaneBorderColor (vcolor *, vresource);

    static void BufferDialogColors (vcolor * foreground, vcolor * background);
    static void BufferSashColors (vcolor * foreground, vcolor * background);
    static void BufferPaneColors (vcolor * foreground, vcolor * background);
    static void BufferMenuColors (vcolor * foreground, vcolor * background);
    static void BufferCommandLineColors (vcolor * foreground, vcolor * background);
    static void BufferToolBarColors (vcolor * foreground, vcolor * background);
    static void BufferProgressColors (vcolor * foreground, vcolor * background);
    static void BufferCurrentPaneBorderColor (vcolor * color);
    static void BufferTargetPaneBorderColor (vcolor * color);

    static vcolor * GetDialogForeground ();
    static vcolor * GetDialogBackground ();
    static vcolor * GetSashForeground ();
    static vcolor * GetSashBackground ();
    static vcolor * GetPaneForeground ();
    static vcolor * GetPaneBackground ();
    static vcolor * GetMenuForeground ();
    static vcolor * GetMenuBackground ();
    static vcolor * GetCommandLineForeground ();
    static vcolor * GetCommandLineBackground ();
    static vcolor * GetToolBarForeground ();
    static vcolor * GetToolBarBackground ();
    static vcolor * GetProgressForeground ();
    static vcolor * GetProgressBackground ();
    static vcolor * GetCurrentPaneBorder();
    static vcolor * GetTargetPaneBorder();
    
    static void ApplyBufferedColors ();
    
    void AddLayerItems(const vchar*, const vchar*);
    int GetId();


   // Returns first Viewer in the global viewer list.
    static Viewer* GetFirstViewer();
 
   // Returns next Viewer in the global viewer list.
    Viewer *GetNextViewer ();

    // points to the next viewer in the singly linked list
    Viewer* next;

    static void SetCurrentColorChooser(ColorDialog *);
    static ColorDialog * GetCurrentColorChooser();
    
    void Disable_MenuToolBar();
    void Enable_MenuToolBar(); 
	vprint* GetPrinter();
	vprint* CreatePrinter();
	void DestroyPrinter();

  private:

    //Buffer for colors
    static vcolorSpec * DialogForegroundSpec;
    static vcolorSpec * DialogBackgroundSpec;

    static vcolorSpec * ToolBarForegroundSpec;
    static vcolorSpec * ToolBarBackgroundSpec;

    static vcolorSpec * MenuForegroundSpec;
    static vcolorSpec * MenuBackgroundSpec;

    static vcolorSpec * SashForegroundSpec;
    static vcolorSpec * SashBackgroundSpec;

    static vcolorSpec * PaneForegroundSpec;
    static vcolorSpec * PaneBackgroundSpec;

    static vcolorSpec * CommandLineForegroundSpec;
    static vcolorSpec * CommandLineBackgroundSpec;

    static vcolorSpec * CurrentPaneBorderSpec;
    static vcolorSpec * TargetPaneBorderSpec;

    static vcolorSpec * ProgressForegroundSpec;
    static vcolorSpec * ProgressBackgroundSpec;

    static ColorDialog * currentColorChooser;

   // points to the first viewer in a singly linked list
    static Viewer* viewers;

    // Static info for finding executing views
    static Viewer* executing_vr;
    static View* executing_v;

    Rect rect;
    
    // the unique id of the viewer, used to reference it
    int id;

    //the viewer's tool bar, and command line
    View* _view; // contains commandLineView_ and system view which is root of layers
    View* layerViews; 

    ToolBar *toolBar_;

    CommandLineView* commandLineView_;
    
    //the viewer's menu bar
    ViewerMenuBar *menuBar_;
    ViewerPopupMenuList *menuPopups_;

    //static counter the generates new ids for new viewers and new views
    static uniqid;
    
    //it's dialog
    MyDialog *dialog;
    
    /* the view that is currently in focus (may end up using galaxy's focus
       function instead*/
    int currentView;
	vprint *printer;
};

#endif



