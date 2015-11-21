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
/****************************
File: genDialog.C
Autor: Jeremy Rothman
Date: 7/20/95
****************************/

/****************************

A short brief on generic dialogs:

All of the objects in the GDialogs can potentially be functional if properly strung together
with tcl commands.  For basic functionality, the objects in the GDialog will take care of
themselves.  Whenever some attribute of an object is modified (a toggle button is selected,
a textfield is entered), it will change a TCL variable in the dialog's interpreter.  This variable
will have the same name as the object's tag name.  Similarly, if through TCL any of these variables
are changed, the object will change its display to match the new value.

If you wish to change one of these variables from outside the dialog box, use the dis_dialog_setvar command,
with the id number of the dialog, the name of the variable, and the value.  This way, you can do something
such as put a string of text in a text item when the dialog comes up.

To give objects special functionality, they need a line in the "Procedures" which sits inside the
dialog's heirarchy. This is an initialization string for the object, which is of the form 
"dis_gdInit<whatever> <dialog_id> <tagname> <TCL command>" where <whatever> would be Button, TextItem, 
Label, etc; and <TCL command> would be the TCL command that the object should execute when
it is activated.  Activation for a button is clicking on it.  For a textitem, it is typing in text, and so forth.
You can use this so that if the user changes the value in, perhaps, a pull-down menu, the text inside a textitem
changes to something else (the activation tcl command for the pull-down menu is to change the value of the variable
with the same tagname as the variable.

GDialogs are created by: dis_launch_dialog <tagname>, which returns the id code of the dialog, so you can refer to it.
GDialogs are deleted by: dis_close_dialog $gd, which destroys it.

***************************/

#include <vport.h>
#include <vevent.h>
#include vdialogHEADER
#include vliststrHEADER
#include vcharHEADER
#include vapplicationHEADER
#include vnotebookHEADER

#include <ggeneric.h>
#include <ggenDialog.h>
#include <ginterp.h>
#include <tcl.h>
#include <gglobalFuncs.h>
#include <gviewer.h>

const int STRING_SIZE = 1024;
const vchar* TARGET_ITEM = (vchar*)"targetItem";
const char* GDIALOG_VARNAME = "gd";

//pointer to the linked list of viewers
GDialog* GDialog::dialogs = NULL;

vkindTYPED_FULL_DEFINITION(GDialog, vdialog, "GDialog");


GDialog::GDialog() : vdialog (vsuppressDONT_INIT)
{
    InitGDialog();
    MaybeInit();
	blocking_=0;
}

GDialog::GDialog (const GDialog* original) : vdialog(vsuppressDONT_INIT)
{
    InitGDialog();
    MaybeCopy(original);
	blocking_=0;
}

GDialog::GDialog (vresource resource) : vdialog(vloadableDONT_LOAD)
{
    InitGDialog();
    MaybeLoad(resource);
	blocking_=0;
}

GDialog::GDialog (vsuppress *) : vdialog(vsuppressDONT_INIT)
{
	blocking_=0;
}



void GDialog::Destroy()
{
    GDialog* n = dialogs;
    if (dialogs == this)
	dialogs = this->nextDialog_;
    else {
	while (n && (n->nextDialog_ != this))
	    n = n->nextDialog_;
	
	if (n)
	    n->nextDialog_ = this->nextDialog_;
    }
}


void GDialog::InitGDialog()
{
    interp_ = make_interpreter();
    id_ = Viewer::uniqid++;
    nextDialog_ = dialogs;    
    char buffer[STRING_SIZE];
    sprintf(buffer,"%d",id_);
    Tcl_SetVar(interp_, (char*) GDIALOG_VARNAME, buffer, TCL_GLOBAL_ONLY);
    dialogs = this;

#ifdef OWNCOLOR
    SetForeground (Viewer::GetDialogForeground());
    SetBackground (Viewer::GetDialogBackground());
#endif
}
    

//
//  Call vdialog::FindItem to find the item in the dialog.  If this does not yield a 
//  result, then go through the item list looking for containers.  If a container is
//  found, the go through the container looking for the item.
//

vdialogItem *GDialog::FindItem (const vname *itemTag) {

    vdialogItem *result = this->vdialog::FindItem (itemTag);

    if (!result) {
        vdialogItemList* myItemList = GetItemList();
        result = find_item (myItemList, itemTag);
    }

    return (result);
}


//
//  Go through an item list looking for containers.  If a container is found, then
//  go through the container looking for the named item.
//

vdialogItem *GDialog::find_item (vdialogItemList *itemList, const vname *tag) {

    vdialogItem *result = NULL;

    for (int i = 0; i < itemList->GetCount() && !result; i++) {
        vobjectAttributed* vo = itemList->GetAt(i); // Get next object.

        // If it is a notebook, recursively check its items.
        if (vo->IsKindOf(&vnotebook::Kind)) {
            vdialogItemList* il = vnotebook::CastDown(vo)->GetItemList();
            result = find_item(il, tag);

        // Else if it is a container, look in it.
        } else if (vo->IsKindOf(&vcontainer::Kind)) {
            vcontainer* container = vcontainer::CastDown(vo);
            const vchar *containerTagName = container->GetTag();

            if (containerTagName && (vcharCompare(container->GetTag(), tag) == 0))
	   	result = container;
	    else
            	result = container->FindItem(tag);

            // If not found, recursively check the container.
            if (!result) {
                vdialogItemList* item_list = container->GetItemList();
                result = find_item(item_list, tag);
            }
        }
    }

    return result;
}




/***************
  This method stores the passed in variable to the specified value
***************/

void GDialog::SetInterpVariable (vchar* variable, vchar* value)
{
    if ((variable) && (value))
	Tcl_SetVar (interp_, (char*) variable, (char*) value, TCL_GLOBAL_ONLY);
}



/***************
  This method returns the value of the passed in variable
***************/

vchar* GDialog::GetInterpVariable (vchar* variable)
{
    if (variable)
	return (vchar*)(Tcl_GetVar (interp_, (char*) variable, TCL_GLOBAL_ONLY));
    else
	return NULL;
}


Tcl_Interp* GDialog::GetInterpreter()
{
    return interp_;
}

/***************
  This method initializes the objects in the gendialog.  It finds the appropriate
  list string based on the tag name of the dialog, and then steps through it, sending
  the item in the second cell value to be evaulated.
***************/


void GDialog::LoadObjects(vchar *procedures)
{
    if (procedures) {
      gala_eval (GetInterpreter(), (char*)procedures);
    }
}


GDialog* GDialog::FindGDialog (int id)
{
    for (GDialog* n = dialogs; n; n = n->nextDialog_) 
    {
	if (n->id_ == id) return n;
    }
    return NULL;
}


/***************
  This method finds a GDialog with the given tag name.
***************/

GDialog* GDialog::FindGDialog (char *theTag)
{

    for (GDialog* n = dialogs; n; n = n->nextDialog_) {
	if (strcmp (theTag, (const char *)n->GetTag()) == 0) return n;
    }

    return NULL;
}


/***************
  This method finds a gdListItem in a gDialog.  All current gDialogs are searched.
  This function may be called as an iterator by passing a NULL pointer as the tag name.
***************/

gdListItem* GDialog::FindListItem (char *tagName)
{
    static char *theTag;
    static GDialog *n;

    if (tagName) {
       if (theTag) vmemFree (theTag);
       theTag = (char *)vmemAlloc (sizeof (char) * (strlen (tagName) + 1));
       strcpy (theTag, tagName);
       n = dialogs;
    }

    if (!theTag) return NULL;

    while (n) {
        gdListItem* myItem = NULL;
        if (myItem = (gdListItem*)n->FindItem(vnameInternGlobal((const vchar*)theTag))) {
            n = n->nextDialog_;
            return myItem;
        }
        n = n->nextDialog_;
    }

    return NULL;
}


// This function will catch a close from the NT [x] in the window frame.
int GDialog::HookClose(vevent *event) 
{
	int stayOpen = vdialog::HookClose(event);

	if (!stayOpen) {
		if (interp_) {
			gala_eval (interp_, "info proc CloseDialog");
			if (strlen(Tcl_GetStringResult(interp_)) )
				gala_eval (interp_, "CloseDialog");
		}
		if (blocking_)
			vevent::StopProcessing();
	}

	return(stayOpen);
}

void GDialog::CloseDialog()
{
	if (interp_) {
		gala_eval (interp_, "info proc CloseDialog");
		if (strlen(Tcl_GetStringResult(interp_)) )
			gala_eval (interp_, "CloseDialog");
	}
	if (blocking_)
		vevent::StopProcessing();
	Close();
}


int GDialog::GetId()
{
    return id_;
}

//this coauses it to go into blocking mode where it pauses the calling dialog
int GDialog::SetBlocking()
{
	blocking_=1;
	vevent::Process();
	return blocking_;
}

int GDialog::IsBlocking()
{
	return blocking_;
}


GDialog * GDialog::GetNext()
{
    return nextDialog_;
}

GDialog * GDialog::GetDialogList()
{
    return dialogs;
}
