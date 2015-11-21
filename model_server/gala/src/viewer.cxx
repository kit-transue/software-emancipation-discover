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
 * viewer.C - Viewer class decls.
 *
 ***** Notes
 *
 * Viewers are our implimentation of windows underneath galaxy
 *
 * by: Nate Kushman
 *
 **********/


#include <vport.h>
#include <vstartup.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER

#ifndef vprintINCLUDED
#include vprintHEADER
#endif

#include <ggeneric.h>
#include <ginterp.h>
#include <gdialog.h>
#include <gsash.h>
#include <gview.h>
#include <gviewer.h>
#include <gviewerMenu.h>
#include <gviewerMenuBar.h>
#include <gviewerMenuItem.h>
#include <gviewerPopupMenuList.h>
#include <gcommandLine.h>
#include <gtoolbar.h>
#include <gprefs.h>
#include <ggenDialog.h>
#include <gglobalFuncs.h>
#include <geditorLayer.h>
#include <Application.h>
#include <DIS_main/interface.h>

const vchar* MENU_BAR = (vchar*) "ViewerMenuBar";

int Y2K_FLAG = 0;

// Static class variables.

// Pointer to the linked list of viewers.
Viewer* Viewer::viewers;

// Holds next unique id for new views and viewers.
int Viewer::uniqid = 1;

//Contains info for view which is currently executing a tcl command
Viewer* Viewer::executing_vr;
View*   Viewer::executing_v;

// Buffers for colos.
vcolorSpec * Viewer::DialogForegroundSpec;
vcolorSpec * Viewer::DialogBackgroundSpec;

vcolorSpec * Viewer::ToolBarForegroundSpec;
vcolorSpec * Viewer::ToolBarBackgroundSpec;

vcolorSpec * Viewer::ProgressForegroundSpec;
vcolorSpec * Viewer::ProgressBackgroundSpec;

vcolorSpec * Viewer::MenuForegroundSpec;
vcolorSpec * Viewer::MenuBackgroundSpec;

vcolorSpec * Viewer::SashForegroundSpec;
vcolorSpec * Viewer::SashBackgroundSpec;

vcolorSpec * Viewer::PaneForegroundSpec;
vcolorSpec * Viewer::PaneBackgroundSpec;

vcolorSpec * Viewer::CommandLineForegroundSpec;
vcolorSpec * Viewer::CommandLineBackgroundSpec;

vcolorSpec * Viewer::CurrentPaneBorderSpec;
vcolorSpec * Viewer::TargetPaneBorderSpec;

ColorDialog * Viewer::currentColorChooser = NULL;

extern void SetToolbarSize();

/**********
 *
 * Viewer::Viewer - Constructor
 *
 **********/
Viewer::Viewer()
{
    gInit (Viewer::Viewer);

#ifdef WIN32
    char *selectedFont = "family:MS Sans Serif-face:medium-size:8-encoding:-foundry:misc";
#else
// This change is made for Y2K and it may be better to use flag here.

//    char *selectedFont = "family:MS Sans Serif-face:bold medium-size:12-encoding:-foundry:misc";
      char *selectedFont = "family:helvetica-face:bold-size:12-encoding:iso8859 1-foundry:adobe";
#endif

    vapplication::GetCurrent()->SetFont(vfont::FindScribed(vcharScribeLiteral(selectedFont)));

    // Set unique ID.
    id = Viewer::uniqid++;

    // Initialize the buffered colors, if not already done.
    if (!Viewer::DialogForegroundSpec)   InitBufferedColors();

    
    // Add this viewer to the global list.
    next = viewers;
    viewers = this;

    _view      = NULL;  
    layerViews = NULL;
    commandLineView_ = NULL;
    menuBar_ = NULL;  
    toolBar_ = NULL;  

    rect.x = 0;
        rect.y = 0;
        rect.w = 0;
        rect.h = 0;

    // Create a new dialog, use resource info if we can find it.
    vresource res;
    if (getResource("SashDialog", &res))
        dialog = new MyDialog(res);
    else {
        dialog = new MyDialog;
        printf("SashDialog not found.\n");
    }

    dialog->SetViewer(this);

        dialog->AddStyle(vwindowSTYLE_MAIN);

    dialog->Place(vwindow::GetRoot(), vrectPLACE_CENTER,
                      vrectPLACE_CENTER);

    dialog->SetMotion(1);

    // Create menu bar.
    menuBar_ = ViewerMenuBar::CastDown(dialog->FindItem(
                             vnameInternGlobal(MENU_BAR)));

    // Create the popup menus list
    menuPopups_ = new ViewerPopupMenuList;

        if (!menuBar_)
                exit (EXIT_FAILURE);;

    menuBar_->InitViewerMenuBar(this, dialog);
    menuPopups_->InitViewerPopupMenus(this, dialog);

    // Now open the dialog and the menu bar.
    const vrect * dialogRect = dialog->GetRect();
    LoadControlViews(dialogRect);

    int menuBarHeight = 0;
    int toolBarHeight = 0;


    dialog->Open();
//    commandLineView_->ResizeView (0,0,-1, 45);  // true size of command line

    if (menuBar_) {
        menuBar_->Open();
        menuBarHeight = menuBar_->GetHeight();
    }

    //makes a new pane using the CustomToolBar from the resource file
    vrect toolbarRect;

    SetToolbarSize();

    toolbarRect.SetX(0);
    toolbarRect.SetY(dialogRect->GetHeight() - menuBarHeight -
                     TOOLBAR_BUTTON_SIZE);
    toolbarRect.SetWidth(dialogRect->GetWidth());
    toolbarRect.SetHeight(TOOLBAR_BUTTON_SIZE);


    if (DIS_args->findArg("-toolbar")) {
        toolBar_ = new ToolBar;
        toolBar_->SetRect (&toolbarRect);
        toolBar_->initialize(this);
        GetDialog()->AppendItem(toolBar_);
    }


    if (toolBar_) toolBarHeight = toolBar_->GetHeight();

    rect.x = 0;
    rect.y = 0;
    rect.w = dialogRect->GetWidth();
    rect.h = dialogRect->GetHeight() - menuBarHeight -
             toolBarHeight - rect.y;

    _view            = new View (NULL, this, rect);
    layerViews       = _view;
    View* viewSystem = layerViews;
    
    currentView = _view->GetId();
    _view->MakeCurrent();
    
    _view->split_horizontal();
    
    /* Here is what I'm doing:  to get a working sash between the message box and views 
       (layerViews), I need to split the the main view.  However, in general, I want
       view splitting to work with the layerViews ptr, not the view ptr.  This, 
       therefore, is built into the view splitting code -- make sure that there is a
       top view in layerViews that won't get removed.  The problem is, when I try to
       split this top view (the view ptr) to get the sash and message box, the 
       view heirarchy is checked against the layerView ptr, not view.  Thus, things are
       going to get out of wack.  These next few lines fix those problems.  */

    _view      = layerViews;
    layerViews = _view->ChildView;  

    /*  Okay, everything should be better now.  view once again points to the top of
        the view tree, and layerViews points to the first child, which is the view 
        structure */
    
    View* viewToReplace = _view->ChildView->RightView;
    commandLineView_->DeleteItem(commandLineView_->myPane);
    commandLineView_->ParentView = viewToReplace->ParentView;
    commandLineView_->LeftView = viewToReplace->LeftView;
    commandLineView_->RightView = viewToReplace->RightView;
    commandLineView_->mySash = viewToReplace->mySash;
    commandLineView_->mySash->SetOwner(commandLineView_);
    commandLineView_->AppendItem(commandLineView_->myPane);
    viewToReplace->mySash = NULL;
    delete viewToReplace;
    _view->ChildView->RightView = commandLineView_;
    _view->Unsplitable_ = vTRUE;

    _view->ResizeView(rect.x, rect.y, rect.w, rect.h);

        printer = NULL;
}





/*deletes its view (which will in turn delete all of its children views), and removes
  itself form the viewers list.  If it is the only viewer, then it stops processing, and
  exits out of the program*/
Viewer::~Viewer()
{
    gInit (Viewer::~Viewer);

    if (toolBar_) {
        //toolBar_->DeleteLater(); Dialog deletes this?
        toolBar_ = NULL;
    }
    if (_view) {
        delete _view;
        _view = NULL;
    }
    if (dialog) {
        // Note - The Menu Bar will be deleted with the dialog because
        //        it is an item owned by the dialog.
        dialog->SetViewer(NULL);
        dialog->DeleteLater();
        dialog = NULL;
    }

    // The Command-Line is deleted with _view, but we still need to NULL it.
    if (commandLineView_) {
        commandLineView_ = NULL;
    }

    if (viewers == this) {    // This viewer is at the front of the viewers list.
      viewers = viewers->next;    // Remove this viewer from the list.
    } else {    // Must find the viewer.
        for (Viewer* v=viewers; v->next == this; v=v->next);
        v->next = v->next->next;    // Remove this viewer from the list.
    }

        if(printer) {
                delete printer;
                printer = NULL;
        }
}

void Viewer::SetTitle (const vchar* title)
{
    if (title) dialog->SetTitle(title);
    else dialog->SetTitle((vchar*)"DIScover");
}

void Viewer::InitBufferedColors ()
{
    int foundSchemeRes = vFALSE;
    vresource dictRes;
    vresource tempRes;

    if (getResource("Colors:Schemes", &dictRes) ) {
        if (getResource("Colors:CurrentScheme", &tempRes) ) {
            vstr * currentSchemeName = vresourceGetString(
                   vresourceGetParent(tempRes),
                   vnameInternGlobalLiteral("CurrentScheme") );
            if (getResource(dictRes, currentSchemeName, &dictRes) ) {
                foundSchemeRes = vTRUE;
            }
        vstrDestroy(currentSchemeName);
        }
    }


    if (foundSchemeRes == vFALSE) {
        printf("@Viewer::InitBufferedColors:");
        printf("Unable to load Color Scheme!!!\n");
        return;
    }

    vresource colorDictRes;
    vresource colorRes;

    // Dialog
    if (getResource(dictRes, "Dialog:Foreground", &colorRes) ) 
        DialogForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "Dialog:Background", &colorRes) ) 
        DialogBackgroundSpec = new vcolorSpec(colorRes);

    // Menu
    if (getResource(dictRes, "Menu:Foreground", &colorRes) ) 
        MenuForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "Menu:Background", &colorRes) ) 
        MenuBackgroundSpec = new vcolorSpec(colorRes);

    // Sash
    if (getResource(dictRes, "Sash:Foreground", &colorRes) ) 
        SashForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "Sash:Background", &colorRes) ) 
        SashBackgroundSpec = new vcolorSpec(colorRes);

    // Pane
    if (getResource(dictRes, "Pane:Foreground", &colorRes) ) 
        PaneForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "Pane:Background", &colorRes) ) 
        PaneBackgroundSpec = new vcolorSpec(colorRes);

    // Toolbar
    if (getResource(dictRes, "ToolBar:Foreground", &colorRes) ) 
        ToolBarForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "ToolBar:Background", &colorRes) ) 
        ToolBarBackgroundSpec = new vcolorSpec(colorRes);

    // CommandLine
    if (getResource(dictRes, "CommandLine:Foreground", &colorRes) ) 
        CommandLineForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "CommandLine:Background", &colorRes) ) 
        CommandLineBackgroundSpec = new vcolorSpec(colorRes);

    // ProgressIndicator
    if (getResource(dictRes, "ProgressIndicator:Foreground", &colorRes) ) 
        ProgressForegroundSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "ProgressIndicator:Background", &colorRes) ) 
        ProgressBackgroundSpec = new vcolorSpec(colorRes);

    // PaneBorders
    if (getResource(dictRes, "CurrentPaneBorder", &colorRes) ) 
        CurrentPaneBorderSpec = new vcolorSpec(colorRes);
    if (getResource(dictRes, "TargetPaneBorder", &colorRes) )
        TargetPaneBorderSpec = new vcolorSpec(colorRes);
}


// Loads the Quick Help Line at the bottom of the screen, 
// and the tool bar at the top.
void Viewer::LoadControlViews (const vrect* dialogRect)
{
    gInit (Viewer::LoadControlViews);

    //loads the resource file for the Quick Help Line
    vresource res;
    Pane* newControl;
    if (getResource("CommandLine", &res) ) {
        newControl = new Pane(vloadableDONT_LOAD);
        newControl->Load(res);
    } else
        newControl = new Pane;

    Rect cmdLineViewRect;
    cmdLineViewRect.y = 0;
    cmdLineViewRect.x = 0;
    cmdLineViewRect.h = newControl->GetHeight();
    cmdLineViewRect.w = dialogRect->GetWidth();
    commandLineView_ = new CommandLineView(this, cmdLineViewRect,newControl);
}


//returns the menu bar
ViewerMenuBar *Viewer::GetMenuBar ()
{
    gInit (Viewer::GetMenuBar);
    return menuBar_;
}

ViewerPopupMenuList *Viewer::GetPopupList () {
    gInit (Viewer::GetPopupList);
    return menuPopups_;
}

//returns the viewers dialog
MyDialog *Viewer::GetDialog ()
{
    gInit (Viewer::GetDialog);
    return dialog;
}

//returns the viewers view
View *Viewer::GetView()
{
    gInit (Viewer::GetView);
    return layerViews;
}

//returns the viewer id
int Viewer::GetViewerCode ()
{
    gInit (Viewer::GetViewerCode);
    return id;
}

//some function Kevin made that does some messed up stuff
int Viewer::GetViewCode ()
{
    gInit (Viewer::GetViewCode);
    return currentView;
}

//our own current view implimentation underneath Galaxy's
void Viewer::SetCurrentView (int newViewCode)
{
    gInit (Viewer::SetCurrentView);
    if (layerViews)  {
        View* oldView = layerViews->find_view(currentView);
        View* newView = layerViews->find_view(newViewCode);
        if (newView)  {
            currentView = newViewCode;
            if (menuBar_)
                menuBar_->PropagateState(newView->GetState());
            if (toolBar_)
                toolBar_->SetViewerState((const vchar *)newView->GetState());
            if (newView->GetPane())    // Highlight the new view.
                newView->GetPane()->DrawBorder();
        }
        if (oldView)
            if (oldView->GetPane())    // Unhighlight the old view.
                oldView->GetPane()->DrawBorder();

        if (newView) {    // Make the layer the current layer.
            Layer * vLayer = newView->GetLayer();
            if (vLayer && !Y2K_FLAG)
                rcall_dis_set_layer(Application::findApplication("DISmain"),
                                    vLayer->getId());
        }
    }
}


void Viewer::DisplayMessage(vchar* newMessage)
{
    gInit (Viewer::DisplayMessage);
    if (newMessage) {
        if (commandLineView_) commandLineView_->DisplayMessage (newMessage);
        gString cmd;
        cmd = (vchar*)"dis_ide_send_command {dis_ide_display_message 0 {";
        cmd += newMessage;
        cmd += (vchar*)"}}";
        Tcl_Eval (g_global_interp, (char*)(vchar*)cmd);
    }
}


//will be used to display quickhelp
void Viewer::DisplayQuickHelp(vchar* newMessage)
{
    gInit (Viewer::DisplayMessage);
    if (newMessage)
        commandLineView_->DisplayQuickHelp (newMessage);
}


// Gets the currently active view from the executing viewer.
void Viewer::GetExecutingView(Viewer*& vr, View*& v) {
    gInit(Viewer::GetExecutingView);
    vr = executing_vr;
    v = executing_v;
}


// Gets the viewer which is executing.
Tcl_Interp* Viewer::GetExecutingInterp() {
    gInit(Viewer::GetExecutingInterp);
    return(executing_v->interp() );
}


// Sets which viewer is executing.
void Viewer::SetExecutingView(Viewer* vr, View* v) {
    gInit(Viewer::SetExecutingView);
    executing_vr = vr;
    executing_v = v;
}


// Gets the first viewer from the list of viewers.
Viewer* Viewer::GetFirstViewer() {
    gInit(Viewer::GetFirstViewer);
    return viewers;
}

// Gets the next viewer from the list of viewers.
Viewer *Viewer::GetNextViewer() {
    return next;
}

// Gets viewer's Id.
int Viewer::GetId() {
    gInit(Viewer::GetId);
    return id;
}


//calls its view's find view
View* Viewer::find_view (int newId)
{
    gInit (Viewer::find_view);
    View* result = NULL;   // Init to no view found.
    // First, check the view.
    if (layerViews) result = layerViews->find_view(newId);
    // Next, check the Command Line.
    if (!result && commandLineView_) {
        if (newId == commandLineView_->GetId())
            result = commandLineView_;
    }
    return result;
}

//calls its view's find view
View* Viewer::find_view_for_layer ()
{
    gInit (Viewer::find_view);
    View* result = NULL;   // Init to no view found.
    // First, check the view.
    if (layerViews) result = layerViews->find_view_for_layer();
    return result;
}

//goes through the viewer linked list matching ids until it finds one that matches
Viewer* Viewer::find_viewer (int id)
{
    gInit (Viewer::find_viewer);
    for (Viewer* v=viewers; v; v=v->next) 
    {
        if (v->id == id) return v;
    }
    return NULL;
}


//it goes through the viewers, and asks their views to find the target view
View* Viewer::find_target_view()
{
    gInit (Viewer::find_target_view);
    Viewer* vs;

    for (vs=viewers; vs; vs = vs->next) {
        View* v = vs->layerViews->find_target_view();
        if (v) return v;
    }
    return NULL;
}


ViewerPopupMenu *Viewer::findPopupMenu (const char *menuTag) {
    if (menuPopups_) {
        return (ViewerPopupMenu *) menuPopups_->FindMenu ((const unsigned char *)menuTag);
    }
    else {
        return NULL;
    }
}


/*prints out with indention, all the current viewers and views (calling its view
  to print out the tree hierarcy, including sashes*/
void Viewer::print(int level)
{
    gInit (Viewer::print);
    for (int i=0; i<level; i++) fprintf (stderr, " ");
    fprintf (stderr, "Viewer:%d [%d,%d,%d,%d]\n", id, rect.x, rect.y, rect.w, rect.h);
    fflush(stderr);
    layerViews->print(level+1);
}

#ifdef _WIN32
extern "C" int __stdcall SetForegroundWindow(void *);
extern "C" int __stdcall IsWindow(void *);
extern "C" int __stdcall IsIconic(void *);
extern "C" int __stdcall PostMessageA( void *, unsigned, long, long );
#ifndef WM_SYSCOMMAND
#define WM_SYSCOMMAND                   0x0112
#endif
#ifndef SC_RESTORE
#define SC_RESTORE      0xF120
#endif
void * vwindowGetMSWWindow( vwindow * );
extern "C" void *_vsysMSWTaskWindow; 
#endif
/*Will eventually be used to bring a viewer to the top*/
void Viewer::bring_to_top()
{
  gInit (Viewer::bring_to_top);
  
  dialog->Uniconify();
  dialog->Raise();
#ifdef _WIN32
  if( dialog ){
    void *msW = vwindowGetMSWWindow( dialog );
    //void *msW = _vsysMSWTaskWindow;
    if( msW && IsWindow(msW) ){
      SetForegroundWindow( msW );
      if( IsIconic(msW) )
        PostMessageA( msW, WM_SYSCOMMAND, SC_RESTORE, 0L );
    }
  }
#endif
}



// ResizeWindow - Resizes all of the components of the Viewer to the new size.
//    The first two args, the x-ratio and y-ratio, are not used.
//    Resizes the Tool Bar, Command Line, and the View.
//    Note - The Menu Bar does not currently care about width and is not resized.
//           (The Menu Bar should probably have a Resize function too.)
//   This function is called by the HandleConfigure method of the parent dialog.
//      (The parent dialog is currently of class MyDialog, we should rename it.)
void Viewer::ResizeWindow(float, float, int newHeight, int newWidth)
{
    gInit (Viewer::ResizeWindow);

    // Resize the View.
    // Note - The X-location remains at 0.
    //        The Y-location remains unchanged.
    //        The height of the view rectangle is set to the new height less
    //            the Tool Bar height and Menu Bar height.
    //        The width is is set to the new width.
    this->rect.w = newWidth;

    // Get the new Height.
    int menuBarHeight = 0;

    if (menuBar_) menuBarHeight = menuBar_->GetHeight();

    int toolBarHeight = 0;
        
    SetToolbarSize();

    if (DIS_args->findArg("-toolbar"))
        toolBarHeight = TOOLBAR_BUTTON_SIZE;

    this->rect.h = newHeight - menuBarHeight - toolBarHeight - this->rect.y;
    // Resize the view.
    if (_view) _view->ResizeView(0, this->rect.y);

    // Resize the Tool Bar.
    // Note - The Y-location is set directly above the new view height.
    //        The width is is set to the new width.
    if (toolBar_)
        toolBar_->Resize(rect.w, rect.y + rect.h);

    // Resize the Command Line to lower left corner.
    // Note - The X-location remains at 0.
    //        The Y-location remains at 0.
    //        The height is remains unchanged.
    //        The width is set to the new width.
    commandLineView_->SetWidth(newWidth);
    commandLineView_->UpdatePane();
  }


//to be added, for color configuration

void Viewer::SaveColor (vcolorSpec *newcolorSpec, const vname *Object, const vname *Attribute, vresource colorRes)
{
    colorRes = vresourceMake(colorRes, Object);
    if (Attribute)
        colorRes = vresourceMake(colorRes, Attribute);
    newcolorSpec->Store (colorRes);
}


void Viewer::SaveDialogColors (vcolor * foreground, vcolor * background, vresource res)
{
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("Dialog"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("Dialog"), 
                    vnameInternGlobalLiteral ("Background"), res);
}



void Viewer::SaveSashColors (vcolor * foreground, vcolor * background, vresource res)
{
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("Sash"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("Sash"), 
                    vnameInternGlobalLiteral ("Background"), res);
}

void Viewer::SavePaneColors (vcolor * foreground, vcolor * background, vresource res)
{
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("Pane"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("Pane"), 
                    vnameInternGlobalLiteral ("Background"), res);
}

void Viewer::SaveMenuColors (vcolor * foreground, vcolor * background, vresource res)
{
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("Menu"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("Menu"), 
                    vnameInternGlobalLiteral ("Background"), res);
}

void Viewer::SaveToolBarColors(vcolor * foreground, vcolor * background, vresource res)
{ 
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("ToolBar"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("ToolBar"), 
                    vnameInternGlobalLiteral ("Background"), res);
}

void Viewer::SaveProgressColors(vcolor * foreground, vcolor * background, vresource res)
{ 
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("ProgressIndicator"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("ProgressIndicator"), 
                    vnameInternGlobalLiteral ("Background"), res);
}

void Viewer::SaveCommandLineColors(vcolor *foreground, vcolor * background, vresource res)
{
  Viewer::SaveColor (foreground->GetSpec(), 
                    vnameInternGlobalLiteral ("CommandLine"), 
                    vnameInternGlobalLiteral ("Foreground"), res);
  Viewer::SaveColor (background->GetSpec(), 
                    vnameInternGlobalLiteral ("CommandLine"), 
                    vnameInternGlobalLiteral ("Background"), res);
}


void Viewer::SaveCurrentPaneBorderColor (vcolor * color, vresource res)
{
    Viewer::SaveColor(color->GetSpec(), 
                      vnameInternGlobalLiteral ("CurrentPaneBorder"), 
                    NULL, res);
}


void Viewer::SaveTargetPaneBorderColor (vcolor * color, vresource res)
{
    Viewer::SaveColor (color->GetSpec(), 
                      vnameInternGlobalLiteral ("TargetPaneBorder"), 
                      NULL, res);
}


void Viewer::BufferDialogColors (vcolor * foreground, vcolor * background)
{
    DialogForegroundSpec = foreground->GetSpec();
    DialogBackgroundSpec = background->GetSpec();
}
void Viewer::BufferSashColors (vcolor * foreground, vcolor * background)
{
    SashForegroundSpec = foreground->GetSpec();
    SashBackgroundSpec = background->GetSpec();
}
void Viewer::BufferPaneColors (vcolor * foreground, vcolor * background)
{
    PaneForegroundSpec = foreground->GetSpec();
    PaneBackgroundSpec = background->GetSpec();
}
void Viewer::BufferMenuColors (vcolor * foreground, vcolor * background)
{
    MenuForegroundSpec = foreground->GetSpec();
    MenuBackgroundSpec = background->GetSpec();
}
void Viewer::BufferCommandLineColors (vcolor * foreground, vcolor * background)
{
    CommandLineForegroundSpec = foreground->GetSpec();
    CommandLineBackgroundSpec = background->GetSpec();
}
void Viewer::BufferToolBarColors (vcolor * foreground, vcolor * background)
{
    ToolBarForegroundSpec = foreground->GetSpec();
    ToolBarBackgroundSpec = background->GetSpec();
}
void Viewer::BufferProgressColors (vcolor * foreground, vcolor * background)
{
    ProgressForegroundSpec = foreground->GetSpec();
    ProgressBackgroundSpec = background->GetSpec();
}
void Viewer::BufferCurrentPaneBorderColor (vcolor * color)
{
    CurrentPaneBorderSpec = color->GetSpec();
}
void Viewer::BufferTargetPaneBorderColor (vcolor * color)
{
    TargetPaneBorderSpec = color->GetSpec();
}





vcolor * Viewer::GetDialogForeground ()
{
    if (DialogForegroundSpec)
        return(vcolor::Intern(DialogForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetDialogBackground ()
{
    if (DialogBackgroundSpec)
        return(vcolor::Intern(DialogBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetSashForeground ()
{
    if (SashForegroundSpec)
        return(vcolor::Intern(SashForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetSashBackground ()
{
    if (SashBackgroundSpec)
        return(vcolor::Intern(SashBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetPaneForeground ()
{
    if (PaneForegroundSpec)
        return(vcolor::Intern(PaneForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetPaneBackground ()
{
    if (PaneBackgroundSpec)
        return(vcolor::Intern(PaneBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetMenuForeground ()
{
    if (MenuForegroundSpec)
        return(vcolor::Intern(MenuForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetMenuBackground ()
{
    if (MenuBackgroundSpec)
        return(vcolor::Intern(MenuBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetCommandLineForeground ()
{
    if (CommandLineForegroundSpec)
        return(vcolor::Intern(CommandLineForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetCommandLineBackground ()
{
    if (CommandLineBackgroundSpec)
        return(vcolor::Intern(CommandLineBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetToolBarForeground ()
{
    if (ToolBarForegroundSpec)
        return(vcolor::Intern(ToolBarForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetToolBarBackground ()
{
    if (ToolBarBackgroundSpec)
        return(vcolor::Intern(ToolBarBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetProgressForeground ()
{
    if (ProgressForegroundSpec)
        return(vcolor::Intern(ProgressForegroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetProgressBackground ()
{
    if (ProgressBackgroundSpec)
        return(vcolor::Intern(ProgressBackgroundSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetCurrentPaneBorder ()
{
    if (CurrentPaneBorderSpec)
        return(vcolor::Intern(CurrentPaneBorderSpec));
    else
        return(NULL);
}

vcolor * Viewer::GetTargetPaneBorder ()
{
    if (TargetPaneBorderSpec)
        return(vcolor::Intern(TargetPaneBorderSpec));
    else
        return(NULL);
}


void Viewer::ApplyBufferedColors()
{    
    for (Viewer *v=viewers; v; v=v->next)
        v->_view->UpdateColors();
    
    for (GDialog * d = GDialog::GetDialogList(); d; d = d->GetNext())
    {
#ifdef OWNCOLOR
                d->SetForeground (vcolor::Intern (DialogForegroundSpec));
                d->SetBackground (vcolor::Intern (DialogBackgroundSpec));
#endif
        }
    
    for (v= viewers; v; v=v->next)
        if (v->menuBar_) v->menuBar_->UpdateColors();
    
    for (v= viewers; v; v=v->next)
        if (v->toolBar_) v->toolBar_->UpdateColors();
    
    for (v= viewers; v; v=v->next)
    {
#ifdef OWNCOLOR
                v->commandLineView_->GetPane()->SetForeground(
                        vcolor::Intern(CommandLineForegroundSpec));
                v->commandLineView_->GetPane()->SetBackground(
                        vcolor::Intern(CommandLineBackgroundSpec));
#endif
    }

    for (v= viewers; v; v=v->next)
        v->find_view (v->currentView)->GetPane()->DrawBorder();
    
    for (v= viewers; v; v=v->next)
    {
        View* targetView = v->find_target_view();
        if (targetView)
            targetView->GetPane()->DrawBorder();
    }
}



void Viewer::SetCurrentColorChooser (ColorDialog * newDialog)
{
    currentColorChooser = newDialog;
}

ColorDialog * Viewer::GetCurrentColorChooser ()
{
    return currentColorChooser;
}



ToolBar * Viewer::GetToolBar()
{
  return toolBar_;
}


void Viewer::AddLayerItems(const vchar* menuTitle, const vchar* tclCommand)
{
    ViewerAppMenu* historyMenu = NULL;
    if (menuBar_) historyMenu = (ViewerAppMenu*) menuBar_->FindMenu(menuTitle);

    if (historyMenu && !historyMenu->IsOpen()) {
        historyMenu->PurgeItemList();
        
        vchar buffer[1024];
        buffer[0] = NULL;
        for (viewLayer* vl = viewLayer::viewLayerList_; vl; vl = vl->getNext()) {
            vcharCopy(tclCommand, buffer);
            vcharAppendFromLiteral(buffer, " {");
            vcharAppend(buffer, vl->getTitle());
            vcharAppendFromLiteral(buffer, "}");
            historyMenu->AppendNewItem((vchar*) vl->getTitle(),
                                       NULL, buffer, NULL);
        }
        
        if (buffer[0])    // Already Added items to menu.
            historyMenu->AppendSepItem(NULL);    // Add a seperator.
        
        gString nm;
        int current_on = 0;
        const layerStack* ls = Layer::getList();

        for (int i = ls->size()-1; i>=0; i--) {
            Layer* l = (*ls)[i];
            if (!l->isTransient()) {
                gString res_name;
                l->getResourceName(res_name);
                vcharCopy(tclCommand, buffer);
                vcharAppend(buffer, (vchar*)" {");
                vcharAppend(buffer, res_name);
                vcharAppend(buffer, (vchar*)"}");
                l->getName(nm);
                historyMenu->AppendToggleItem(nm, NULL, buffer, buffer, NULL);
                // -- set the toggle ON
                if (!current_on) {
                    current_on = 1;
                    ViewerMenuItemList * list = historyMenu->GetMenuItemList();
                    if (list) {
                        while (list->GetNext())
                            list = list->GetNext();
                        vmenuItem * item = list->GetMenuItem();
                        if (item) {
                            if (item->IsToggle())
                                item->SetOn(1);
                        }
                    }
                }
            }
        }
    }
}

void Viewer::Disable_MenuToolBar()
{
    if(menuBar_) menuBar_->Disable();
    if(toolBar_) toolBar_->Disable();
}


void Viewer::Enable_MenuToolBar()
{
   if(menuBar_)  menuBar_->Enable();
   if(toolBar_)  toolBar_->Enable();
}

vprint* Viewer::GetPrinter()
{
        return printer;
}

vprint* Viewer::CreatePrinter()
{
        printer = vprint::CreatePreferred();
        return printer;
}

void Viewer::DestroyPrinter()
{
        if(printer) {
                delete printer;
                printer = NULL;
        }
}

/**********     end of viewer.C     **********/
