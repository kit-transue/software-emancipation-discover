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

#include <vport.h>

#include <gviewerPopupMenuList.h>
#include <gviewerMenu.h>
#include <gviewerMenuItem.h>
#include <gapl_menu.h>

vchar *ViewerPopupMenuList::custom_popupMenus = NULL;


ViewerPopupMenuList::ViewerPopupMenuList() {
    myMenuList_ = NULL;
    previousMenu_ = NULL;
    viewer_ = NULL;
    interp_ = NULL;
}


void ViewerPopupMenuList::InitViewerPopupMenus (Viewer* viewer, MyDialog* ownerDialog) {

    myMenuList_ = NULL;
    previousMenu_ = NULL;
    viewer_ = viewer;
    interp_ = NULL;

    // Create the TCL interpreter and passit the viewer address.
    interp_ = make_interpreter();
    char buffer[10];
    sprintf (buffer, "%d", viewer->GetId());
    Tcl_SetVar (interp_, "vr", buffer, TCL_GLOBAL_ONLY);

    if (!custom_popupMenus) 
      SetPopupMenuFile ((const vchar *) "popupmenus.dat");

    // Load the menus
    aplMenuLoad (this, (const char *) custom_popupMenus);

}


int ViewerPopupMenuList::AddNewMenu(	const vchar* newtag, 
					const vchar* name, 
					const vchar* arguments, 
					const vchar* command) {

    ViewerPopupMenu* newMenu = new ViewerPopupMenu (	name, 
							viewer_, 
							this, 
							previousMenu_, 
							newtag, 
							arguments,
							command);

    if (previousMenu_) previousMenu_->SetNext(newMenu);
    if (!myMenuList_) myMenuList_ = newMenu;
    previousMenu_ = newMenu;

    return newMenu->GetId();
}

ViewerMenu* ViewerPopupMenuList::FindMenu (const vchar *tagName) {

    ViewerMenu *menu = NULL;
    if (myMenuList_) {

	menu = myMenuList_->FindMenu(tagName);
	if (!menu) {
	    ViewerMenuItemList *item = FindItem (tagName);
	    if (item) menu = item->GetSubMenu();
	}
    }
    return menu;

}

ViewerMenuItemList* ViewerPopupMenuList::FindItem (const vchar *tagName) {
    return myMenuList_ ? myMenuList_->FindItem (tagName) : NULL;
}


void ViewerPopupMenuList::SetPopupMenuFile (const vchar *newFile) {
    if( custom_popupMenus )
     delete [] custom_popupMenus;
    custom_popupMenus = new vchar [vcharLength (newFile) + 1];
    vcharCopy (newFile, custom_popupMenus);
}
