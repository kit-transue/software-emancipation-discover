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
 * gtoolbar.h - Tool Bar defs.
 *
 **********/

#ifndef _toolbar_h
#define _toolbar_h


#include <vport.h>
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vbuttonINCLUDED
   #include vbuttonHEADER
#endif
#ifndef vdragINCLUDED
   #include vdragHEADER
#endif

#ifndef _gview_h
   #include "gview.h"
#endif


class Viewer;
class View;

// Tool Bar Constants.

extern int TOOLBAR_BUTTON_SIZE;

// Viewer State constants.
const static int  NumViewerStates = 19;
const static vchar * ViewerStateList[NumViewerStates] = {
    (vchar *)"AutoTrial/sw",  
    (vchar *)"Browser", 
    (vchar *)"Create/sw", 
    (vchar *)"CM/sw",  
    (vchar *)"Dormant/sw",
    (vchar *)"Extract/sw",
    (vchar *)"Group",
    (vchar *)"Impact/Propagate",  
    (vchar *)"Partition/sw",
    (vchar *)"Preferences",
    (vchar *)"AutoDoc/sw",
    (vchar *)"SplashScreen",
    (vchar *)"Build/sw",
    (vchar *)"Parser",
    (vchar *)"Summary/sw",
    (vchar *)"Metrics/sw",
    (vchar *)"Graph",
    (vchar *)"Y2K",
    (vchar *)"None"};


/**********     Class toolbarView     **********/

class toolbarView : public View
{
public:
  toolbarView(Viewer*, Rect, Pane*);
};


//**********************ToolBar*******************************
class ToolBarButton;
class ScrollButton;
class ToolBarDialog;
class ToolBarButtonDrag;
class ViewerMenu;
class ScrollButtonTimer;

class ToolBar:public vcontainer
{
 public:
  vkindTYPED_FULL_DECLARATION(ToolBar);
  vloadableINLINE_CONSTRUCTORS(ToolBar, vcontainer);

    void addButton(ToolBarButton *, int, int movedButton = 0);
    void removeButton (ToolBarButton *);
    void removeScrollButtons();
    void addScrollButtons();
    void initialize(Viewer *);
    void SaveButtons ();
    void LoadButtons();
    void SetState (int);
    void RemoveAllButtons();
    void AddAllButtons();
    void BufferButtons();
    void UnBufferButtons();
    void DeleteBufferedButtons();
    void ScrollRight();
    void ScrollLeft();
    void Resize(int, int);
    Viewer * GetViewer();
    void UpdateColors();
    void SetViewerState(const vchar *);
    void ShowButtonSet();
    void HideButtonSet();
    void DeleteButtonSet (ToolBarButton *);
    int  GetHeight ();

  private:

    // Buttons for current Viewer state.
    ToolBarButton * Buttons;

    // Array of buttons for each viewer state.
    ToolBarButton * ViewerStateButtons[NumViewerStates];
    const vchar * ViewerStateName;
    int  ViewerStateIndex;
    
    ToolBarButton * ButtonsBuffer;
    ToolBarButton * LeftmostShown; //the button furthest to the left that is shown
    ToolBarButton * RightmostShown; // the button furthest to the right that is shown
    ScrollButton * LeftScrollButton;
    ScrollButton * RightScrollButton;
    int ScrollButtonState; //whether or not it has scrollbars
    Viewer * myViewer;
};

class ToolBarButton:public vbutton
{
 public:
    virtual void ObserveDialogItem (vevent *event);
    virtual int HandleButtonDown (vevent *event);
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ToolBarButton);
    vloadableINLINE_CONSTRUCTORS(ToolBarButton,vbutton);
    
    void initialize(const vname *, const vchar*, const vchar *, ToolBar *, ToolBarDialog *, vimage *);
    int initialize (const vname *, ToolBar *, ViewerMenu *);
    void Destroy();
    void SetNext (ToolBarButton *);
    ToolBarButton *GetNext();
    void SetPrev (ToolBarButton *);
    ToolBarButton *GetPrev();
    ToolBarButton * gCopy();
    Viewer * GetViewer();
    ToolBar * GetToolBar();
    void SetState (int);
    int GetState ();
    void Save (vliststr *, int);
    virtual void Hilite ();		// (const vrect*, const vrect*);
    virtual void Unhilite ();  		// (const vrect*, const vrect*);  
    const vchar* GetTip() {return tip;}
 
  private:
    ToolBarDialog * myDialog;
    ToolBarButton * next;
    ToolBarButton * prev;
    vchar * tcl_Command;
    vchar * contents_copy; 		// ***TESTING***TESTING***
    ToolBar * myToolBar;
    Viewer * myViewer;
    int State; //customize in toolbar(2), customize in dialog(1) or normal (0) 
    vchar * tip;
};  

class ScrollButton:public vbutton
{
  public:
    vkindTYPED_FULL_DECLARATION(ScrollButton);
    vloadableINLINE_CONSTRUCTORS(ScrollButton, vbutton);
    void initialize(int, ToolBar *);
    virtual int HandleButtonDown (vevent *);
    virtual int HandleButtonUp (vevent *);
    void ActivateButton ();
    void DisableButton();
    void EnableButton();
    void Destroy();
  private:
    int Side; //left side (0), or rigth (1);
    ToolBar * myToolBar;
    int State; //enabled (1), or disabled (0);
    ScrollButtonTimer * timer;
};  


class ScrollButtonTimer : public vtimer
{
  public:
    void SetOwner(ScrollButton*);
    void ObserveTimer();

  private:
    ScrollButton* owner;
    vtimestamp startTime, speedUpTime;
    int hasAccelerated;
};


class ToolBarDialogSaveButton:public vbutton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ToolBarDialogSaveButton);
    vloadableINLINE_CONSTRUCTORS(ToolBarDialogSaveButton,vbutton);
    virtual int HandleButtonDown (vevent *event);
    void initialize (ToolBar *);
  private:
    ToolBar* myToolBar;
};


class ToolBarDialogCancelButton:public vbutton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ToolBarDialogCancelButton);
    vloadableINLINE_CONSTRUCTORS(ToolBarDialogCancelButton,vbutton);
    virtual int HandleButtonDown (vevent *event);
    void initialize (ToolBar *);
  private:
    ToolBar* myToolBar;
};
    
class ToolBarDialogLocalButton:public vbutton
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ToolBarDialogLocalButton);
    vloadableINLINE_CONSTRUCTORS(ToolBarDialogLocalButton,vbutton);
    virtual int HandleButtonDown (vevent *event);
    void initialize (ToolBar *);
  private:
    ToolBar* myToolBar;
};


class ToolBarDialog:public vdialog
{
  public:
    vkindTYPED_FULL_DECLARATION(ToolBarDialog);
    vloadableINLINE_CONSTRUCTORS(ToolBarDialog, vdialog);
    void initialize(ToolBar*);
    void LoadButtons (ViewerMenu *, ToolBar *, vchar *);
  private:
};

class ToolBarListItem:public vlistitem
{
 public:
  // vkindTyped
    vkindTYPED_FULL_DECLARATION(ToolBarListItem);
  
  // the standard versions of the necessary constructor/destructor
  vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(ToolBarListItem, vlistitem);
  vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(ToolBarListItem, vlistitem);
  vkindTYPED_INLINE_DESTRUCTOR(ToolBarListItem, vlistitem);
  
  virtual void ObserveSelect (int);

  void initialize();
};
  

class ToolBarButtonDrag:public vdrag
{
 public: 

  virtual void Draw(vrect *rect);
  virtual void ObserveDrag(vwindow *from, vwindow *to, vevent *event);

  // vkindTyped
    vkindTYPED_FULL_DECLARATION(ToolBarButtonDrag);
  
  // the standard versions of the necessary constructor/destructor
  vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(ToolBarButtonDrag, vdrag);
  vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(ToolBarButtonDrag, vdrag);
  vkindTYPED_INLINE_DESTRUCTOR(ToolBarButtonDrag, vdrag);
  
  // virtual member function override
  
  void SetButton (ToolBarButton*);

 protected:
  ToolBarButton* myButton;   // a link to the ToolBarButton being moved
};


#endif





/**********     end of gtoolbar.h     **********/
