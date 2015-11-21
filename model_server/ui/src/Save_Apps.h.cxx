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
// Save_Apps.h.C
//------------------------------------------
// synopsis:
// Save_Apps dialog in libGT
//
// description:
// ...
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <gtForm.h>
#include <gtList.h>
#include <gtLabel.h>
#include <gtDlgTemplate.h>
#include <genError.h>
#include <Save_Apps.h>
#include <genString.h>
#include <top_widgets.h>
#include <machdep.h>

#define TXT(a) a

// FUNCTION DEFINITIONS
int proj_hide_save_callback (SaveDialog *, void *, int *, int);
int proj_hide_notsave_callback (SaveDialog *, void *, int *, int );

int SaveDialog::state = 0;

static int do_nothing_callback(SaveDialog*,void*,int*,int) { return 1; }

SaveDialog::SaveDialog(
    gtBase* parent, void* obj_array, dlgFlavor flavor,
    SaveDialogCB select, SaveDialogCB save, SaveDialogCB cleanup,
    void (*ok)(void*), void* ok_data, SaveDialogCB notsave)
: modified(obj_array),
  client_select(select),
  client_save(save),
  client_cleanup(cleanup),
  client_ok(ok),
  client_ok_data(ok_data),
  our_flavor(flavor),
  client_notsave(notsave)
// constructor for the dialog box of "offer to save"
{
#ifndef NEW_UI
    // This dialog is used for quit, unload and save all. Here is where
    // we customize the buttons and help contexts to make sense.
    const char* title = NULL;
    const char* save_button_label = NULL;
    const char* nosave_button_label = NULL;
    const char* help_context = NULL;
    const char* name_label_text = TXT("These unsaved files\nhave been modified:");
    state                    = 0;
    switch(our_flavor)
    {
      case Flavor_Quitting:
	save_button_label = TXT("Save Then Quit");
	nosave_button_label = TXT("Quit Without Save");
	title = TXT("Quit");
	help_context = "Dialogs.Quit.Save";
	break;

      case Flavor_Unloading:
	save_button_label = TXT("Save Then Unload");
	nosave_button_label = TXT("Unload Without Save");
	title = TXT("Unload Files");
	help_context = "Dialogs.Unload.Save";
	break;

      case Flavor_Hiding:
	save_button_label = TXT("Save Then Unload");
	nosave_button_label = TXT("Unload Without Save");
	title = TXT("Unload Files");
	help_context = "Dialogs.Unload.Save";
	client_save = proj_hide_save_callback;
	client_notsave = proj_hide_notsave_callback;
	client_ok = 0;
	break;

      case Flavor_Putting:
	save_button_label = TXT("Save Then Put");
	title = TXT("Put Files");
	help_context = "Dialogs.Put.Save";
	break;

      case Flavor_ViewerClosing:
	save_button_label = TXT("Save Then Close Viewer");
	nosave_button_label = TXT("Close Viewer Without Save");
	title = TXT("Close Viewer");
	help_context = "Dialogs.CloseViewer.Save";
	break;

      case Flavor_Parsing:
	save_button_label = TXT("Save Then Reparse");
	nosave_button_label = TXT("Reparse Without Save");
	title = TXT("Reparse May Lose Changes To These Files");
	help_context = "Dialogs.Reparse.Save";
	break;

      case Flavor_Saving:
	save_button_label = TXT("Save");
	title = TXT("Save Files");
	help_context = "Dialogs.SaveAll";
	break;

     case Flavor_Browsing_Groups:
	save_button_label = TXT("Save to Model");
	nosave_button_label = TXT("Operate Without Save");
	client_notsave = &do_nothing_callback;
	title = TXT("Scratch Groups");
	help_context = "Pset.Help.Groups.Scratch";
	name_label_text = TXT("The following scratch groups will not participate\nin the operation unless they are saved:");
	break;
    }

    shell = gtDialogTemplate::create(NULL, "save_files", title);

    shell->add_button("apply", save_button_label, save_CB, this);
    if (nosave_button_label)
	shell->add_button("ok", nosave_button_label, ok_CB, this);
    shell->add_button("cancel", TXT("Cancel"), cancel_CB, this);
    shell->add_help_button();

    shell->help_context_name(help_context);

    name_form = gtForm::create(shell, "name_form");
    depend_form = gtForm::create(shell, "depend_form");

    name_label = gtLabel::create(name_form, "name_label",
        name_label_text);

    genString depend_text;
    depend_text += gettext(TXT("Click '"));
    depend_text += save_button_label;
    depend_text += gettext(TXT("' to save\nupdates to the following files."));
    depend_label = gtLabel::create(depend_form, "depend_label", depend_text);

    name_list = gtList::create(name_form,"modified_list",0,gtExtended,NULL,0);
    name_list->set_callback(name_CB, this);

    depend_list = gtList::create(depend_form,"save_list",0,gtExtended,NULL,0);
/*  depend_list->set_sensitive(0); */

    name_form->attach(gtTop);
    name_form->attach(gtBottom);
    name_form->attach(gtLeft);
    name_form->attach_pos(gtRight, 48);
    name_form->manage();

    depend_form->attach(gtTop);
    depend_form->attach(gtBottom);
    depend_form->attach(gtRight);
    depend_form->attach(gtLeft, name_form, 12);
    depend_form->manage();

    name_label->attach(gtTop);
    name_label->attach(gtLeft);
    name_label->attach(gtRight);
    name_label->manage();

    name_list->attach(gtLeft);
    name_list->attach(gtRight);
    name_list->attach(gtBottom);
    name_list->attach(gtTop, name_label);
    name_list->manage();

    depend_label->attach(gtTop);
    depend_label->attach(gtLeft);
    depend_label->attach(gtRight);
    depend_label->manage();

    depend_list->attach(gtLeft);
    depend_list->attach(gtRight);
    depend_list->attach(gtBottom);
    depend_list->attach(gtTop, depend_label);
    depend_list->manage();
#endif
}

void SaveDialog::call_client_select()
{
    Initialize(SaveDialog::call_client_select);
#ifndef NEW_UI
    if(client_select)
    {
	int* pos = NULL;
	int  count = 0;
	name_list->get_selected_pos(&pos, &count);

        (*client_select)(this, modified, pos, count);

	if(pos)
	    OSapi_free(pos);
    }
#endif
}

void SaveDialog::add_modified(const char* name)
{
#ifndef NEW_UI
    if(!name || !*name)
	return;
//	name = "<no name>";

    name_list->add_item_unselected(name, 0);
#endif
}

void SaveDialog::clear_to_be_saved()
{
#ifndef NEW_UI
    depend_list->delete_all_items();
#endif
}

void SaveDialog::add_to_be_saved(const char* depend)
{
#ifndef NEW_UI
    depend_list->add_item_unselected(depend, 0);
#endif
}

int SaveDialog::popup(bool wait)
{
    const int MAX_VISIBLE_ITEM_COUNT = 10;
#ifndef NEW_UI
    // Select all items in list
    name_list->select_all(0);
    call_client_select();

    // Adjust VisibleItemCount
    int count = name_list->num_items();
    int dcount = depend_list->num_items();
    if(count < dcount)
	count = dcount;
    if(count > MAX_VISIBLE_ITEM_COUNT)
	count = MAX_VISIBLE_ITEM_COUNT;
    else if(count <= 1)
	count = 4;

    // set the max width of the lists (they can get wide)
    // RPP - 10/4/98
    name_list->width(225);
    depend_list->width(225);

    name_list->num_rows(count);
    depend_list->num_rows(count);


    depend_list->remanage();
    name_list->remanage();

    shell->popup(3);

    if (wait) {
       shell->take_control(&check_state, this);
    }
    return state;
#else
    return 0;
#endif
}

int SaveDialog::check_state(void* data) {
   return ((SaveDialog*) data)->state != 0;
}

SaveDialog::~SaveDialog()
{
#ifndef NEW_UI
    if(client_cleanup)
        (*client_cleanup)(this, modified, NULL, 0);

    delete shell;
#endif
}

int SaveDialog::save_selected_items()
{
    int success = 0;
#ifndef NEW_UI
    if(client_save)
    {
	int* pos = NULL;
	int  count = 0;
	name_list->get_selected_pos(&pos, &count);

	success = (*client_save)(this, modified, pos, count);

	if (success)
	    for(int i = count - 1; i >= 0; --i)
		name_list->delete_pos(pos[i]);
	if(pos)
	    OSapi_free(pos);
    }
#endif
    return(success);
}

int SaveDialog::notsave_selected_items()
{
    int success = 0;
#ifndef NEW_UI
    if(client_notsave)
    {
	int* pos = NULL;
	int  count = 0;
	name_list->get_selected_pos(&pos, &count);

	success = (*client_notsave)(this, modified, pos, count);

	if (success)
	    for(int i = count - 1; i >= 0; --i)
		name_list->delete_pos(pos[i]);
	if(pos)
	    OSapi_free(pos);
    }
#endif
    return(success);
}

void SaveDialog::ok_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
// the callback routine of the ok button for the dialog box "offer to save"
{
    Initialize(SaveDialog::ok_CB);
#ifndef NEW_UI
    push_busy_cursor();
    SaveDialog* save_dlg = (SaveDialog*)data;
    if (save_dlg->notsave_selected_items())
    {
	state = 1;
	if(save_dlg->client_ok)
	    (*save_dlg->client_ok)(save_dlg->client_ok_data);
    }
    else
	state = -1;
    delete save_dlg;
    pop_cursor();
#endif
}
    
void SaveDialog::save_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
// the callback routine of the save button for the dialog box "offer to save"
{
    Initialize(SaveDialog::save_CB);
#ifndef NEW_UI
    push_busy_cursor();
    SaveDialog* save_dlg = (SaveDialog*)data;
    if (save_dlg->save_selected_items())
    {
	state = 1;
	if(save_dlg->client_ok)
	    (*save_dlg->client_ok)(save_dlg->client_ok_data);

	delete save_dlg;
    }
    else
	state = -1;
    pop_cursor();
#endif
}
    
void SaveDialog::cancel_CB(gtPushButton*, gtEventPtr, void* data, gtReason)
// the callback routine of the cancel button for the dialog box "offer to save"
{
    Initialize(SaveDialog::cancel_CB);
#ifndef NEW_UI
    SaveDialog* save_dlg = (SaveDialog*)data;
    state = -1;
    delete save_dlg;
#endif
}

void SaveDialog::name_CB(gtList*, gtEventPtr, void* data, gtReason)
{
    Initialize(SaveDialog::name_CB);
#ifndef NEW_UI
    SaveDialog* save_dlg = (SaveDialog*)data;
    save_dlg->call_client_select();
#endif
}
