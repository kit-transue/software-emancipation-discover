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
  File:       viewerMenuItem.C
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
#include vmemHEADER
#include vkeyHEADER

#include <ggeneric.h>
#include <gdialog.h>
#include <gglobalFuncs.h>
#include <gsash.h>
#include <gview.h>
#include <gviewer.h>
#include <gviewerMenu.h>
#include <gviewerMenuItem.h>
#include <ginterp.h>
#include <gstate.h>


const vchar ARG_SEPERATOR = '|';
const char* const ARG_SEPERATOR_STRING = "|";


// definitions for overrides from vloadable
vkindTYPED_FULL_DEFINITION(ViewerMenuItem, vmenuItem, "ViewerMenuItem");
vkindTYPED_FULL_DEFINITION(ViewerMenuLabelItem, vmenuItem, "ViewerMenuLabelItem");
vkindTYPED_FULL_DEFINITION(ViewerMenuToggleItem, vmenuToggleItem, "ViewerMenuToggleItem");


extern  StateGroup* DIS_stateGroup;

/*
This method takes a scribe, allocates memory for a vchar, and copies the data over
*/

vchar* ConvertScribeToString (vscribe *s)
{
  vchar* dest = NULL;

  if (s) {
     dest = new vchar [ vcharLengthScribed (s)+1 ];
     vcharCopyScribed (s, dest);
  }

  return dest;
}



ArgList::ArgList(const vchar* list)
{
    if (list)  {
	contents_ = new vchar [ vcharLength (list) + 1];
	vcharCopy(list, contents_);
    }
    else
	contents_ = NULL;
}


ArgList::~ArgList()
{
    delete contents_;
}



vchar* ArgList::GetArg(const vchar* name)
{
    vchar* valueToReturn;

    if ((name) && (contents_))  {
	vchar searchBuffer[2028];
	vcharCopy(name, searchBuffer);
	vcharAppendFromLiteral(searchBuffer, "=");
	vchar* result = vcharSearch(contents_, searchBuffer);
	if (result) {
	    result = result + vcharLength(searchBuffer);
	    vchar* nextChar = result;
	    
	    while ((*nextChar != ARG_SEPERATOR) && (*nextChar != '\0'))
		nextChar++;
	    
	    valueToReturn = new vchar [ nextChar - result + 2];
	    vcharCopyBounded(result, valueToReturn, nextChar - result);
	    valueToReturn[nextChar - result] = '\0';
	}
	else
	    valueToReturn = NULL;
    }
    else
	valueToReturn = NULL;

    return valueToReturn;
}	


void ArgList::AddNewArgument(const vchar* newArg)
{
    vchar* newList = new vchar[vcharLength(contents_) + vcharLength(newArg) + 2];
    vcharCopy(contents_,newList);
    vcharAppendFromLiteral(newList, ARG_SEPERATOR_STRING);
    vcharAppend(newList,newArg);
    
    delete contents_;
    contents_ = newList;
}
	



/*
This method initializes the object.  It stores its passed in variables and sets its default
values to Null
*/

ViewerMenuItemList::ViewerMenuItemList (ViewerMenu* newMenu, ViewerMenuItemList* prev, vmenuItem* newItem)
{
  myItem_ = newItem;

  myMenu_ = newMenu;
  prev_ = prev;
  next_ = NULL;
  hidden_ = vFALSE;
}


void ViewerMenuItemList::SetNext (ViewerMenuItemList* newNext)
{
  next_ = newNext;
}

/*
Returns the position of the next menu item in the menu.  If this menu item is not hidden, it 
will return its own position; otherwise, it will return the position of the menu item behind
it.  If there is no menu item behind it, it will return -1.
*/
 
int ViewerMenuItemList::GetPosition()
{
  if (hidden_)
        if (!next_)
                return LAST_ITEM;
        else
                return next_->GetPosition();
 
  return myMenu_->GetItemIndex (myItem_);
}

/*
This method will attach the item back into the menu bar, and then query the menu item after it
for the correct position in the list.
*/

void ViewerMenuItemList::Show()
{
  int position;

  myMenu_->AppendItem(myItem_);
      
  if (next_)
    position = next_->GetPosition();
  else
    position = LAST_ITEM;
  
  if (position != LAST_ITEM)
      myMenu_->SetItemIndex (myItem_, position);
  
  hidden_ = vFALSE;
}

/*
This method removes the menu item from the menu
*/

void ViewerMenuItemList::Hide()
{
  myMenu_->DeleteItem(myItem_);
  hidden_ = vTRUE;
}


void ViewerMenuItemList::Enable()
{
  myItem_->Enable();
}


void ViewerMenuItemList::Disable()
{
  myItem_->Disable();
}


void ViewerMenuItemList::SetOn(int state)
{
  myItem_->SetOn(state);
}


ViewerMenu* ViewerMenuItemList::GetSubMenu (void)
{
    ViewerMenu * ret_val = (ViewerMenu *)0;

    if (! (myItem_->IsToggle()) )
	ret_val = ViewerMenu::CastDown(myItem_->GetSubMenu());

    return ret_val;
}


/*
This method searches for the given menu item.  If the item is not itself, it will
first check to see if it has a submenu.  If it does, it will ask the submenu to find
the item.  If that comes up blank, it will ask its neighbor
*/
ViewerMenuItemList* ViewerMenuItemList::FindItem (const vchar* tagName)
{
  const vchar* mytag = myItem_->GetTag ();
  if ((mytag != NULL) && (vcharCompare(tagName, mytag) == 0))
    return this;
  else {
      ViewerMenuItemList* itemToReturn;

      ViewerMenu* mySubMenu = GetSubMenu ();
      if (mySubMenu) {
	  itemToReturn = mySubMenu->FindItem(tagName);
	  if (itemToReturn)
	    return itemToReturn;
	}
      if (next_)
	return next_->FindItem(tagName);
      else
	return NULL;
    }
}

/*
This method resets the state of the menuitems.  It is passed in a vliststr and searches
all of the elements in it for its own name.  If it is not in there, it will make itself visible
and enabled.  If it does find its tag name, it will either show or disable itself, depending on
the value in the second column, and then delete that row, to prevent unnecessary checking by the
rest of the list.  Finally, it will pass the list on to the other menu items.
*/


void ViewerMenuItemList::ResetStates(vliststr *offList)
{
    int foundFlag = vFALSE;
    int x = 0;
    vchar* tagName, *typeOfChange;
    
    while ((x < offList->GetRowCount()) && (!foundFlag))  // If there are still more rows and 
    {                                                     // this item has not been found
	tagName = ConvertScribeToString((offList->ScribeCellValue(x,0)));
	if (vcharCompare(tagName, myItem_->GetTag())==0)  // If cell x,0 matches my tag name
	{
	    typeOfChange = ConvertScribeToString((offList->ScribeCellValue(x,1)));
	    if (vcharCompare(typeOfChange,HIDE_CODE)==0)  // if cell x,1 says to hide me
	    {
		if (!hidden_)
		    Hide();
	    }
	    else                          // otherwise disable me
	    {
		if (myItem_->IsEnabled())
		    Disable();
	    }
	    offList->RemoveRow(x);
	    foundFlag = vTRUE;
	}
	x++;
    }

    if (!foundFlag)         // If I was never found, show and enable
    {
	if (hidden_) Show();
	if (!myItem_->IsEnabled()) Enable();
    }

    if (offList->GetRowCount() > 0)  // if there are still more elts, tell other menu items to reset
	if (next_) next_->ResetStates(offList);
}    



// This method destroys the menu items.  It deletes the rest of the list.
//    It is the responsibility of the Menu calling this to delete its items.
//    We don't delete the items explicitely because it blows up. (It blows up
//    because of some problem with deleting menu items which are also menus.)
//    We will let the menuItems get deleted automatically when the menu is 
//    deleted.
void ViewerMenuItemList::DestroyList()
{
    if (next_) {
	next_->DestroyList();
	delete next_;
        next_ = NULL;
    }
}


void ViewerMenuItemList::AddNewArgument(const vchar* newArg)
{
    ViewerMenuItem* obj = ViewerMenuItem::CastDown(myItem_);
    if (obj)
	obj->AddNewArgument(newArg);
    else {
	ViewerMenu* menu = ViewerMenu::CastDown(myItem_->GetSubMenu());
	if (menu)
	    menu->AddNewArgument(newArg);
	else {
	    ViewerMenuToggleItem* tog = ViewerMenuToggleItem::CastDown(myItem_);
	    if (tog)
		tog->AddNewArgument(newArg);
	}
    }
}


/*
This method sets up the item.  It sets its name and stores links to its command and the viewer
*/

ViewerMenuItem::ViewerMenuItem(const vchar *name, const vchar *tclCommand,
                               Viewer* viewer, const vchar* arguments)
{
    argList_ = new ArgList(arguments);

    // Set up Modifier
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

    SetUpAccelerator();

    if (tclCommand) {
	   tclCommand_ = new vchar[vcharLength(tclCommand) + 1];
	    vcharCopy((const vchar*) tclCommand, tclCommand_);
    }
    else
    	tclCommand_ = NULL;
    myViewer_ = viewer;
}

void ViewerMenuItem::AddPropagation(ViewerMenu* menu)
{
    vchar* state = argList_->GetArg((vchar*)"State");
     
    if (state)  {
        vchar ECmd[1024];
        vcharCopyFromLiteral((vchar*)"dis_ViewerEnableItem -all", ECmd);
        vcharAppendFromLiteral(ECmd, " ");
        vcharAppend(ECmd, (vchar*)GetTag());

        vchar DCmd[1024];
        vcharCopyFromLiteral((vchar*)"dis_ViewerDisableItem -all", DCmd);
        vcharAppendFromLiteral(DCmd, " ");
        vcharAppend(DCmd, (vchar*)GetTag());

        StateGroup* sg = myViewer_->GetMenuBar()->GetStateGroup();
        if (sg) sg->transitionFlux (state, ECmd, DCmd);
        DIS_stateGroup->transitionFlux (state, ECmd, DCmd);

        myViewer_->GetMenuBar()->DisableItem ((vchar*)GetTag());

        delete state;
    }
}

void ViewerMenuItem::Destroy()
{
    delete argList_;
    delete tclCommand_;

    vmenuItem::Destroy();
}


/*
Handles the evaluation of a tcl command.
*/

void ViewerMenuItem::ObserveMenuItem(vevent* event)
{
    vmenuItem::ObserveMenuItem(event);

    if (tclCommand_) {
	int idCode = myViewer_->GetViewCode();
	View *v = myViewer_->find_view(idCode);
	if (v) {
	  Viewer::SetExecutingView (myViewer_, v);
	  int code = gala_eval (v->interp(), (char*)tclCommand_);
	}
    }
}


/*
Tells the viewer to display quickhelp information
*/

void ViewerMenuItem::Hilite ( const vrect* bounds, const vrect* content)
{
    vchar *vc = argList_->GetArg((vchar*) "qhelp");
	vdraw::GSave ();
    myViewer_->DisplayQuickHelp(vc);
	vdraw::GRestore();
    if (vc) delete (vc);
    vmenuItem::Hilite (bounds, content);
}


/*
Tells the viewer to display quickhelp information
*/

void ViewerMenuItem::Unhilite ( const vrect* bounds, const vrect* content)
{
	vdraw::GSave();
    myViewer_->DisplayQuickHelp((vchar*) " ");
	vdraw::GRestore();
    vmenuItem::Unhilite (bounds, content);
}

ViewerMenuLabelItem::ViewerMenuLabelItem(const vchar *name) {

    SetTitle (name);
}

void ViewerMenuLabelItem::Hilite (const vrect *bounds, const vrect *content)
{
}

void ViewerMenuLabelItem::HiliteDefault (const vrect *bounds, const vrect *content) 
{
}

void ViewerMenuLabelItem::Unhilite (const vrect *bounds, const vrect *content)
{
}


vmenuItem *ViewerMenuItemList::GetMenuItem()
{
  return myItem_;
}


vchar * ViewerMenuItem::GetTcl_Command()
{
    return (tclCommand_);
}

ArgList * ViewerMenuItem::GetArgList()
{
    return (argList_);
}


void ViewerMenuItem::SetUpAccelerator()
{
    if (argList_) {
	vchar* accel = argList_->GetArg((vchar*)"accel");
	if (accel) {
	    vkeyModifiers mods =  vkeyParseModifiers((const vchar*) (accel+2));
	    vkeyStroke myStroke = vkeyComputeStroke( vkeyFromChar(*accel), vkeyMODIFIER_KEY_CTRL);
	    SetAccelerator( myStroke );
            delete (accel);
/*
	    vdict* myDict = new vdict;
	    myDict->SetElem((const void*)GetTag(), (const void*)myStroke);
	     
	    vmenu* owner = vmenu::CastDown(GetParentObject());
	    if (owner) {
		owner->MapAccelerators(GetTag(), myDict);

	    }
*/
	}
    }
}



void ViewerMenuItem::AddNewArgument(const vchar* newArg)
{
    if (argList_ && newArg) {
	argList_->AddNewArgument(newArg);
	SetUpAccelerator();
    }
}

ViewerMenuItemList *ViewerMenuItemList::GetNext()
{
	return next_;
}



/*
This method sets up the item.  It sets its name and stores links to its command and the viewer
*/

ViewerMenuToggleItem::ViewerMenuToggleItem (const vchar *name, 
                      const vchar *onTclCommand, const vchar* offTclCommand,
                      Viewer* viewer, const vchar* arguments, ViewerMenu* owner)
{
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

    //Set up accelerator
    vchar* accel = argList_->GetArg((vchar*)"accel");
    if (accel) {
	SetAccelerator(vkeyComputeStroke(vkeyFromChar(*accel),
                      vkeyParseModifiers((const vchar*) (accel+2))));
        delete (accel);
    }

    if (onTclCommand) {
	onTclCommand_ = new vchar[ vcharLength(onTclCommand) + 1];
	vcharCopy((const vchar*) onTclCommand, onTclCommand_);
    }
    if (offTclCommand) {
	offTclCommand_ = new vchar[ vcharLength(offTclCommand) + 1];
	vcharCopy((const vchar*) offTclCommand, offTclCommand_);
    }
    myViewer_ = viewer;
    owner_ = owner;
}


void ViewerMenuToggleItem::Destroy()
{
    delete argList_;
    delete onTclCommand_;
    delete offTclCommand_;

    vmenuToggleItem::Destroy();
}


/*
Handles the evaluation of a tcl command.
*/

void ViewerMenuToggleItem::ObserveMenuItem(vevent*)
{
    vchar* commandToExecute;
    if (IsOn())
	commandToExecute = onTclCommand_;
    else
	commandToExecute = offTclCommand_;

    SetOn(IsOn());

    int idCode = myViewer_->GetViewCode();
    View *v = myViewer_->find_view(idCode);
    if (v) {
      Viewer::SetExecutingView (myViewer_, v);
      int code = gala_eval (v->interp(), (char*)commandToExecute);
    }

}


/*
Tells the viewer to display quickhelp information
*/

void ViewerMenuToggleItem::Hilite ( const vrect* bounds, const vrect* content)
{
    vchar *vc = argList_->GetArg((vchar*) "qhelp");
    vdraw::GSave ();
    myViewer_->DisplayQuickHelp(vc);
    vdraw::GRestore();
    if (vc) delete (vc);
    vmenuToggleItem::Hilite (bounds, content);
}


/*
Tells the viewer to display quickhelp information
*/

void ViewerMenuToggleItem::Unhilite ( const vrect* bounds, const vrect* content)
{
    vdraw::GSave ();
    myViewer_->DisplayQuickHelp((vchar*) " ");
    vdraw::GRestore();
    vmenuToggleItem::Unhilite(bounds, content);
}



void ViewerMenuToggleItem::SetOn (int flag)
{
    vmenuItem::SetOn(flag);

    if (flag) {
	ViewerGroupMenu* parent = ViewerGroupMenu::CastDown(owner_);
	if (parent)
	    parent->NewSelection(owner_->GetItemIndex(this));
    }
}



void ViewerMenuToggleItem::AddNewArgument(const vchar* newArg)
{
    if (argList_ && newArg) {
	argList_->AddNewArgument(newArg);
	SetUpAccelerator();
    }
}



void ViewerMenuToggleItem::SetUpAccelerator()
{
    if (argList_) {
	vchar* accel = argList_->GetArg((vchar*)"accel");
	if (accel) {
	    vkeyModifiers mods =  vkeyParseModifiers((const vchar*) (accel+2));
	    vkeyStroke myStroke = vkeyComputeStroke( vkeyFromChar(*accel), mods);
	    SetAccelerator( myStroke );
            delete (accel);

/*
	    vdict* myDict = new vdict;
	    myDict->SetElem((const void*)GetTag(), (const void*)myStroke);
	     
	    vmenu* owner = vmenu::CastDown(GetParentObject());
	    if (owner) {
		owner->MapAccelerators(GetTag(), myDict);

	    }
*/
	}
    }
}
