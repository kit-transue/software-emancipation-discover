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
  File:       viewerMenu.cxx
  Created by: Jeremy Rothman
  Date:       6/8/95
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
#include vapplicationHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gsash.h>
#include <gstate.h>
#include <gviewer.h>
#include <gviewerMenuBar.h>
#include <gviewerMenuItem.h>
#include <gviewerMenu.h>
#include <ginterp.h>
#include <gglobalFuncs.h>
#include <tcl.h>

static const vchar* MENU_BREAK = (vchar*)";";
static const vchar* SEPARATOR_ITEM = (vchar*)"-";
static const vchar* SUBMENU_ITEM = (vchar*)"+";
static const char* HIDE_CMD = "dis_ViewerHideMenu";
static const char* SHOW_CMD = "dis_ViewerShowMenu";
static const char* DISABLE_CMD = "dis_ViewerDisableMenu";
static const char ON_CMD = 'O';

int ViewerMenu::UniqId_ = 0;
ViewerMenu* ViewerMenu::sMenuList_ = NULL;

Viewer* myViewer;

vkindTYPED_FULL_DEFINITION (ViewerMenu, vmenu, "ViewerMenu");

ViewerMenu::ViewerMenu (const vchar *name, Viewer* viewer, 
			ViewerMenuBar* newMenuBar, ViewerMenu* prev, 
			const vchar *newTag, const vchar* arguments)
{
    myMenuBar_ = newMenuBar;
    prev_ = prev;
    next_ = NULL;
    hidden_ = vFALSE;
    viewer_ = viewer;
    myItemList_ = NULL;
    oldListItem_ = NULL;

    id_ = UniqId_++;
    nextMenu_ = sMenuList_;
    sMenuList_ = this;

    argList_ = new ArgList(arguments);


    vchar* newName = new vchar[vcharLength(name) + 1];
    const vchar* nextChar = name;
    vchar* nextSpot = newName;
    
    while (*nextChar != '\0')  {
	if (*nextChar == '_')
	    SetMnemonic(name[nextChar - name - 1]);
	else {
	    *nextSpot = *nextChar;
	    nextSpot++;
	}
	nextChar++;
    }

    *nextSpot = '\0';

    SetTitle (newName);
    delete newName;

    if (newTag != NULL)
        SetTag (vnameInternGlobal (newTag));

    SetupStates (arguments);

#ifdef OWNCOLOR
    this->SetForeground(viewer->GetMenuForeground());
    this->SetBackground(viewer->GetMenuBackground());
	
#endif
}

/*
Destroys the contents of the item list
*/

void ViewerMenu::Destroy()
{
  // Remove this menu from the global list.
  ViewerMenu* m = sMenuList_;
  if (!m)			// Null global list!
    printf("@ViewerMenu::sMenuList_ is NULL!\n");	
  else if ( m->id_ == id_)	// First in the list.
    sMenuList_ = sMenuList_->nextMenu_;
  else		// Check the rest of the list.
    {
    while (m->nextMenu_ && m->nextMenu_->id_ != id_)
      m = m->nextMenu_;
    if (m->nextMenu_)		// Found it.
      m->nextMenu_ = m->nextMenu_->nextMenu_;
    else
       {
       printf("@ViewerMenu::Destroy()\t");
       printf("Unable to find menu id #%d in global list!!!\n", id_);
       }
    }

  // Delete stuff we added.
  delete argList_;

  if (myItemList_) {
    myItemList_->DestroyList();
    delete myItemList_;
    myItemList_ = NULL;
  }

  // Call standard destroy.
  vmenu::Destroy();
}   // End of ViewerMenu::Destroy


// DestroyAllMenus - Recursively deletes ALL menus.
void ViewerMenu::DestroyAllMenus()
{
    if (sMenuList_) {
        ViewerMenu* m = sMenuList_->nextMenu_;
        while (m) {
            ViewerMenu* tmp = m;
            m = m->nextMenu_;
            delete tmp;
        }
    delete sMenuList_;
    sMenuList_ = NULL;
    }
}




/*
Changes who the menu item thinks is before it
*/

void ViewerMenu::SetNext (ViewerMenu* newNext)
{
    newNext->next_ = next_;
    next_ = newNext;
}

/*
Returns the position of the next menu in the menu bar.  If this menu is not hidden, it will return
its own position; otherwise, it will return the position of the menu behind it.  If there is no
menu behind it, it will return -1.
*/

int ViewerMenu::GetPosition()
{
    if (hidden_)
        {
	if (!next_)
	    return LAST_ITEM;
	else
	    return next_->GetPosition();
        }
    else if (myMenuBar_)
        return myMenuBar_->GetMenuIndex (this);
    else
        return LAST_ITEM;
}


/*
This method shows a menu item.  It uses GetPosition to determine its appropriate location and
then adds itself to the menubar
*/

void ViewerMenu::Show()
{
    int position;
    
    if (myMenuBar_ && hidden_) {
	myMenuBar_->AppendMenu(this);
	
	if (next_)
	    position = next_->GetPosition();
	else
	    position = LAST_ITEM;
	
	if (position != LAST_ITEM)
	    myMenuBar_->SetMenuIndex (this, position);
	
	hidden_ = vFALSE;

	if (!IsEnabled())
	    Enable();
    }
}

/*
This method hides the menu by deleting it from the menu bar
*/

void ViewerMenu::Hide()
{
    if (myMenuBar_ && !hidden_) {
	myMenuBar_->DeleteMenu(this);
	hidden_ = vTRUE;
    }
}

/*
  This method finds a menu item by first searching its list of items and then, if that is 
unsuccessful, asking its next menu.  If it is the last menu and it cannot find it, it will
return NULL.
*/

ViewerMenuItemList* ViewerMenu::FindItem (const vchar* tagName)
{
    ViewerMenuItemList* targetItem = NULL;
    if (myItemList_)
	targetItem = myItemList_->FindItem(tagName);
    
    if (targetItem)
	return targetItem;
    else if (next_)
	return next_->FindItem(tagName);
    else
	return NULL;
}

/*
This method goes through the list, of menus, searching for the menu item
*/

ViewerMenu* ViewerMenu::FindMenu (const vchar* tagName)
{
    if (vcharCompare(tagName, GetTag()) == 0)
	return this;
    else if (next_)
	return next_->FindMenu(tagName);
    else
	return NULL;
}

ViewerMenu* ViewerMenu::FindMenuByName (const vchar* Name)
{
    if (vcharCompare(Name, GetTitle()) == 0)
	return this;
    else if (next_)
	return next_->FindMenuByName(Name);
    else
	return NULL;
}


void ViewerMenu::Disable()
{
    if (hidden_)
	Show();

    vmenu::Disable();
}

void ViewerMenu::AppendNewItem(const vchar* name, const vchar* newTag, const vchar* tclCommand, const vchar* arguments)
{
  ViewerMenuItem* newItem = new ViewerMenuItem( name, tclCommand, viewer_, arguments);
  if (newTag != NULL)
      newItem->SetTag (vnameInternGlobal (newTag));
  ViewerMenuItemList* newListItem = new ViewerMenuItemList (this, oldListItem_, newItem);
  if (oldListItem_) oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
  if (!myItemList_) myItemList_ = newListItem;
  AppendItem (newItem);
  oldListItem_ = newListItem;
  newItem->AddPropagation(this);
}

void ViewerMenu::AppendLabelItem (const vchar *name, const vchar *newTag) {
  ViewerMenuLabelItem* newItem = new ViewerMenuLabelItem (name);
  if (newTag != NULL) newItem->SetTag (vnameInternGlobal (newTag));

  ViewerMenuItemList* newListItem = new ViewerMenuItemList (this, oldListItem_, newItem);
  if (oldListItem_) oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
  if (!myItemList_) myItemList_ = newListItem;
  AppendItem (newItem);
  oldListItem_ = newListItem;
}

void ViewerMenu::AppendSepItem(const vchar* newTag)
{
  vmenuSeparatorItem* newItem = new vmenuSeparatorItem;
  if (newTag != NULL)
      newItem->SetTag (vnameInternGlobal (newTag));
  newItem->SetTitle((vchar*)"-");
  ViewerMenuItemList* newListItem = new ViewerMenuItemList (this, oldListItem_, newItem);
  if (oldListItem_) oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
  if (!myItemList_) myItemList_ = newListItem;
  AppendItem (newItem);
  oldListItem_ = newListItem;
}


void ViewerMenu::AppendToggleItem(const vchar* name, const vchar* newTag,
                 const vchar* onTclCommand, const vchar* offTclCommand,
                 const vchar* arguments)
{
  ViewerMenuToggleItem* newItem = new ViewerMenuToggleItem(name, onTclCommand,
                        offTclCommand, viewer_, arguments, this);
  if (newTag != NULL)
      newItem->SetTag(vnameInternGlobal (newTag));
  ViewerMenuItemList* newListItem = new ViewerMenuItemList(this, oldListItem_,
                                                           newItem);
  if (oldListItem_)
      oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
  if (!myItemList_)
      myItemList_ = newListItem;
  AppendItem(newItem);
  oldListItem_ = newListItem;
}



ViewerMenu *ViewerMenu::AppendSubItem(const vchar* name, const vchar* newTag, const vchar* arguments)
{
    vmenuSubMenuItem* newItem = new vmenuSubMenuItem;
    if (newTag != NULL)
	newItem->SetTag(vnameInternGlobal(newTag));
    
    vchar* newName = new vchar[vcharLength(name) + 1];
    const vchar* nextChar = name;
    vchar* nextSpot = newName;
    
    while (*nextChar != '\0')  {
	if (*nextChar == '_')
	    newItem->SetMnemonic(name[nextChar - name - 1]);
	else {
	    *nextSpot = *nextChar;
	    nextSpot++;
	}
	nextChar++;
    }
    
    *nextSpot = '\0';
    
    newItem->SetTitle (newName);
    delete newName;

    ViewerMenuItemList* newListItem = new ViewerMenuItemList(
                        this, oldListItem_, newItem);
    if (oldListItem_)
        oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
    if (!myItemList_)
        myItemList_ = newListItem;
    AppendItem(newItem);
    oldListItem_ = newListItem;
    
    ViewerMenu* subMenu = new ViewerMenu(name, viewer_, NULL, NULL,
                                         newTag, arguments);
    newItem->SetSubMenu(subMenu);
    
    return subMenu;
}   // End of ViewerMenu::AppendSubItem



int ViewerMenu::AppendSubGroupItem(const vchar* name, const vchar* newTag, const vchar* arguments)
{
    vmenuSubMenuItem* newItem = new vmenuSubMenuItem;
    if (newTag != NULL)
	newItem->SetTag(vnameInternGlobal(newTag));
    
    vchar* newName = new vchar[vcharLength(name) + 1];
    const vchar* nextChar = name;
    vchar* nextSpot = newName;
    
    while (*nextChar != '\0')  {
	if (*nextChar == '_')
	    newItem->SetMnemonic(name[nextChar - name - 1]);
	else {
	    *nextSpot = *nextChar;
	    nextSpot++;
	}
	nextChar++;
    }
    
    *nextSpot = '\0';
    
    newItem->SetTitle (newName);
    delete newName;
    
    ViewerMenuItemList* newListItem = new ViewerMenuItemList(
                        this, oldListItem_, newItem);
    if (oldListItem_)
        oldListItem_->SetNext(newListItem);  // Maintain a list of the menus
    if (!myItemList_)
        myItemList_ = newListItem;
    AppendItem (newItem);
    oldListItem_ = newListItem;
    
    ViewerMenu* subMenu = new ViewerGroupMenu(name, viewer_, NULL, NULL,
                                              newTag, arguments);
    newItem->SetSubMenu(subMenu);
    
    return subMenu->GetId();
}   // End of ViewerMenu::AppendSubGroupItem.


ViewerMenu* ViewerMenu::FindMenu(int searchId)
{
  ViewerMenu* m = sMenuList_;
  while (m && (m->id_ != searchId))
    m = m->nextMenu_;
  
  return m;
}


int ViewerMenu::GetId()
{
  return id_;
}

void ViewerMenu::UpdateColors()
{
#ifdef OWNCOLOR
	SetForeground(Viewer::GetMenuForeground());
	SetBackground(Viewer::GetMenuBackground());
#endif
  if (nextMenu_)  nextMenu_->UpdateColors();
}


ViewerMenu* ViewerMenu::GetGlobalList()
{
  return sMenuList_;
}


ViewerMenuItemList *ViewerMenu::GetMenuItemList()
{
  return myItemList_;
}


ViewerMenu * ViewerMenu::GetNext()
{
	return next_;
}


void ViewerMenu::AddNewArgument(const vchar* newArg)
{
    if (argList_ && newArg)
	argList_->AddNewArgument(newArg);
}



void ViewerMenu::Open()
{
    vmenu::Open();
    vchar *vc = argList_->GetArg((vchar*) "qhelp");
    viewer_->DisplayQuickHelp(vc);
    if (vc) delete (vc);
}



void ViewerMenu::Close()
{
    vmenu::Close();
    vchar *vc = argList_->GetArg((vchar*) " ");
    viewer_->DisplayQuickHelp(vc);
    if (vc) delete (vc);
}
	
void ViewerMenu::SetupStates(const vchar* states)
{
   if (myMenuBar_){   
       vchar* buffer;
       vchar* argument;

       vchar hideCmd[1024];
       vcharCopyFromLiteral(HIDE_CMD, hideCmd);
       vcharAppendFromLiteral(hideCmd, " ");
       vcharAppend(hideCmd, (vchar*)GetTag());

       vchar showCmd[1024];
       vcharCopyFromLiteral(SHOW_CMD, showCmd);
       vcharAppendFromLiteral(showCmd, " ");
       vcharAppend(showCmd, (vchar*)GetTag());

       myMenuBar_->GetStateGroup()->transitionFlux (states, showCmd,
                                                    hideCmd);

   }
}


/*****************************************************
  ViewerAppMenu
  ***************************************************/

vkindTYPED_FULL_DEFINITION (ViewerAppMenu, ViewerMenu, "ViewerAppMenu");


ViewerAppMenu::ViewerAppMenu(const vchar *name, Viewer* viewer,
               ViewerMenuBar* newMenuBar, ViewerMenu* prev,
               const vchar *newTag, const vchar* tclBuildCmd) :
               ViewerMenu(name, viewer, newMenuBar, prev, newTag, 
                          (vchar*)"All=S")

{
    vcharCopy((const vchar*) tclBuildCmd, tclBuildCmd_);
    persistent = -1; 
}



// Cleans out all of the items in the menu.
void ViewerAppMenu::PurgeItemList()
{
    // -- remember all persistent items
    if (persistent < 0)
	persistent = GetItemCount();

    // First delete all of the attached items.
    // Note - This only works because this menu never has sub-menus.
    for (int iter = GetItemCount(); iter>persistent; iter--) {
        vmenuItem* i = GetItemAt(iter-1);
        DeleteItem(i);
        i->DeleteLater();
    }

    // Now delete the item list.
    if (persistent <= 0) {
	if (myItemList_) {
	    myItemList_->DestroyList();
	    delete myItemList_;
	    myItemList_ = NULL;
	}
	oldListItem_ = NULL;
    } else {
	for (oldListItem_ = myItemList_;
	     oldListItem_->GetNext() != NULL; 
	     oldListItem_ = oldListItem_->GetNext()) ;
    }
}



/*****************************************************
  ViewerGroupMenu
  ***************************************************/

vkindTYPED_FULL_DEFINITION (ViewerGroupMenu, ViewerMenu, "ViewerGroupMenu");


ViewerGroupMenu::ViewerGroupMenu(const vchar *name, Viewer* viewer, 
                 ViewerMenuBar* newMenuBar, ViewerMenu* prev, 
                 const vchar *newTag, const vchar* arguments) :
    ViewerMenu(name, viewer, newMenuBar, prev, newTag, 
                            arguments)

{
}

void ViewerGroupMenu::NewSelection(int choice)
{
    for (int x =0; x < GetItemCount(); x++)  {
	vmenuItem* nextChoice = GetItemAt(x);
	if ((x != choice) && (nextChoice->IsToggle()))
	    nextChoice->SetOn(vFALSE);
    }
}

/*****************************************************
  ViewerPopupMenu
  ***************************************************/

vkindTYPED_FULL_DEFINITION (ViewerPopupMenu, ViewerMenu, "ViewerPopupMenu");

ViewerPopupMenu::ViewerPopupMenu (	const vchar *name, 
					Viewer* viewer, 
					ViewerPopupMenuList* newMenuList, 
					ViewerMenu* prev, 
					const vchar *newTag, 
					const vchar* arguments,
					const vchar* command) :

				ViewerMenu (	name, 
						viewer, 
						NULL, 
						prev, 
						newTag, 
						arguments) {

    command_ = command;

}

void ViewerPopupMenu::ExecutePopupCmd () {
    gala_eval (g_global_interp, (char *)command_.str());
}
