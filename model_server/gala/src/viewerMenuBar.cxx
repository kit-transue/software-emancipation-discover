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
/**************************
  File:       viewerMenuBar.cxx
  Created by: Jeremy Rothman
  Date:       6/17/95
***************************/

#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vmenubarHEADER
#include vmenuHEADER


#include <ggeneric.h>
#include <gdialog.h>
#include <gsash.h>
#include <gviewer.h>
#include <gviewerMenu.h>
#include <ginterp.h>
#include <gviewerMenuBar.h>
#include <gviewerMenu.h>
#include <gviewerMenuItem.h>
#include <gapl_menu.h>
#include <gglobalFuncs.h>

// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(ViewerMenuBar, vmenubar, "ViewerMenuBar");

// Static member initializations
vchar* ViewerMenuBar::custom_menus = NULL; // "menus.dat";
vchar* ViewerMenuBar::custom_qhelp = NULL; // "qhelp.dat";

// Constants
//const char* DEFAULT_MENUS = "menus.dat";
//const char* DEFAULT_QHELP = "qhelp.dat";


/**********
*
* InitViewerMenuBar - Initializes the menubar.
*
**********/

void ViewerMenuBar::InitViewerMenuBar(Viewer* viewer,
                                      MyDialog* ownerDialog) 
{
  // Init vars.
  myMenuList_ = NULL;
  oldMenu_ = NULL;
  viewer_ = viewer;
  oldState_ = NULL;
  interp_ = NULL;

  // This poorly named method forces the last menu to be right justified.
  SetHelp(vTRUE);
  
  // Create the TCL interpeter and pass it the viewer address.
  interp_ = make_interpreter();
  char buffer[10];
  sprintf(buffer,"%d",viewer_->GetId());
  Tcl_SetVar(interp_, "vr", buffer, TCL_GLOBAL_ONLY);

  stateGroup = new StateGroup(interp_);

#ifdef OWNCOLOR
  SetForeground(viewer_->GetMenuForeground());
  SetBackground(viewer_->GetMenuBackground());
#endif

  if (!custom_menus)
    SetMenuDataFile( (const vchar *)"menus.dat" );
  if (!custom_qhelp) 
    SetQHelpDataFile( (const vchar *)"qhelp.dat" );

  // Get menu info and help info from data files.
  aplMenuLoad (this, (const char*) custom_menus);
  aplMenuLoad (this, (const char*) custom_qhelp);

  ViewerMenu* ml = myMenuList_;
  while (ml) {
     ml->Hide();
     ml = ml->GetNext();
  }

  stateGroup->changeState ((vchar*)"All=S");
}


/* This method deletes all of the menus */

void ViewerMenuBar::Destroy()
{
    // We cannot destroy any menus because other viewers may be using them.

    if (oldState_)
        delete [] oldState_;

    if (interp_)
        delete_interpreter(interp_);

    vmenubar::Destroy();
}

/*
These next eight methods are all very similar.  They search for the target item and then
either show, hide, enable, or disable it.
*/

void ViewerMenuBar::ShowItem (const vchar* tagName)
{
  ViewerMenuItemList * targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->Show();
}


void ViewerMenuBar::HideItem (const vchar* tagName)
{
  ViewerMenuItemList *targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->Hide();
}


void ViewerMenuBar::EnableItem (const vchar* tagName)
{
  ViewerMenuItemList * targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->Enable();
}


void ViewerMenuBar::DisableItem (const vchar* tagName)
{
  ViewerMenuItemList * targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->Disable();
}


void ViewerMenuBar::ActivateItem (const vchar* tagName)
{
  ViewerMenuItemList * targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->SetOn(vTRUE);
}


void ViewerMenuBar::DeactivateItem (const vchar* tagName)
{
  ViewerMenuItemList * targetItem = NULL;

  if (myMenuList_)
    targetItem = myMenuList_->FindItem(tagName);

  if (targetItem)
    targetItem->SetOn(vFALSE);
}


void ViewerMenuBar::ShowMenu (const vchar* tagName)
{
  ViewerMenu * targetMenu = NULL;

  if (myMenuList_)
    targetMenu = myMenuList_->FindMenu(tagName);

  if (targetMenu)
    targetMenu->Show();
}


void ViewerMenuBar::HideMenu (const vchar* tagName)
{
  ViewerMenu * targetMenu = NULL;

  if (myMenuList_)
    targetMenu = myMenuList_->FindMenu(tagName);

  if (targetMenu)
    targetMenu->Hide();
}


void ViewerMenuBar::EnableMenu (const vchar* tagName)
{
  ViewerMenu * targetMenu = NULL;

  if (myMenuList_)
    targetMenu = myMenuList_->FindMenu(tagName);

  if (targetMenu) {
    targetMenu->Enable();
    ChangedMenu();
  }
}


ViewerMenu* ViewerMenuBar::FindMenu(const vchar* tagName)
{

    if (myMenuList_) {
        ViewerMenu* menu = myMenuList_->FindMenu(tagName);
        if (menu) return menu;

        ViewerMenuItemList* item = FindItem(tagName);
        if (item) menu = item->GetSubMenu();
        if (menu) return menu;
    }

    return NULL;
}

ViewerMenu* ViewerMenuBar::FindTopMenuByName(const vchar *Name)
{
    ViewerMenu *ret = NULL;
    int nCount = GetMenuCount();
    for( int i=0; i<nCount && !ret; i++ )
    {
      vmenu *menu = GetMenuAt( i );
      if( menu && !vcharCompare( Name, menu->GetTitle() ) )
        ret = ViewerMenu::CastDown( menu );
    }
    if( !ret && myMenuList_ )
      ret = myMenuList_->FindMenuByName( Name );
    return ret;
}



ViewerMenuItemList* ViewerMenuBar::FindItem (const vchar* tagName)
{
    return myMenuList_ ? myMenuList_->FindItem (tagName) : NULL;
}


void ViewerMenuBar::DisableMenu (const vchar* tagName)
{
  ViewerMenu * targetMenu = NULL;

  if (myMenuList_)
    targetMenu = myMenuList_->FindMenu(tagName);

  if (targetMenu) 
  {
    targetMenu->Disable();
    ChangedMenu();
  }
}

ViewerMenu *ViewerMenuBar::AddNewMenu(const vchar* newtag, const vchar* name, const vchar* arguments)
{
  ViewerMenu* newMenu = new ViewerMenu( name, viewer_, this, oldMenu_, newtag, arguments);
  if (oldMenu_) oldMenu_->SetNext(newMenu);  // Maintain a list of the menus
  if (!myMenuList_) myMenuList_ = newMenu;
  AppendMenu (newMenu);
  oldMenu_ = newMenu;
  
  return newMenu;
}
    



int ViewerMenuBar::AddGroupMenu(const vchar* newtag, const vchar* name, const vchar* arguments)
{
  ViewerMenu* newMenu = new ViewerGroupMenu( name, viewer_, this, oldMenu_, newtag, arguments);
  if (oldMenu_) oldMenu_->SetNext(newMenu);  // Maintain a list of the menus
  if (!myMenuList_) myMenuList_ = newMenu;
  AppendMenu (newMenu);
  oldMenu_ = newMenu;
  
  return newMenu->GetId();
}
    



int ViewerMenuBar::AddAppMenu(const vchar* newtag, const vchar* name, const vchar* tclCmd)
{
  ViewerMenu* newMenu = new ViewerAppMenu( name, viewer_, this, oldMenu_, newtag, tclCmd);
  if (oldMenu_) oldMenu_->SetNext(newMenu);  // Maintain a list of the menus
  if (!myMenuList_) myMenuList_ = newMenu;
  AppendMenu (newMenu);
  oldMenu_ = newMenu;
  
  return newMenu->GetId();
}


void ViewerMenuBar::UpdateColors()
{
  //SetForeground(viewer_->GetMenuForeground());
  //SetBackground(viewer_->GetMenuBackground());
  if (myMenuList_)
    myMenuList_->GetGlobalList()->UpdateColors();
}


ViewerMenu *ViewerMenuBar::GetMenuList()
{
    return myMenuList_;
}


void ViewerMenuBar::PropagateState(const vchar* newState)
{
    if (!oldState_ || vcharCompare(oldState_, newState)) {
        vchar stateChange[1024];
        if (oldState_) {
            vcharCopy (oldState_, stateChange);
            vcharAppendFromLiteral(stateChange, "=H,");
        } else vcharCopyFromLiteral("", stateChange);

        vcharAppend(stateChange, (vchar*)newState);
        vcharAppendFromLiteral(stateChange, "=S");

        stateGroup->changeState (stateChange);
 


        delete oldState_;
	    oldState_ = new vchar[vcharLength(newState)+1];
	    vcharCopy (newState, oldState_);
    }
}




Tcl_Interp* ViewerMenuBar::interp()
{
    return interp_;
}


void ViewerMenuBar::SetMenuDataFile(const vchar* newFile)
{
    if( custom_menus )
      delete [] custom_menus;
    custom_menus = new vchar[vcharLength(newFile) + 1];
    vcharCopy(newFile, custom_menus);
}



void ViewerMenuBar::SetQHelpDataFile(const vchar* newFile)
{
    if( custom_qhelp )
      delete [] custom_qhelp;
    custom_qhelp = new vchar[vcharLength(newFile) + 1];
    vcharCopy(newFile, custom_qhelp);
}
