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
// Builtin_Relations.h.C
//------------------------------------------
// synopsis:
// Builtin_Relations dialog in libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <Builtin_Relations.h>
#include "gtTogB.h"
#include <OODT_apl_entries.h>
#include <gtPushButton.h>

#undef NULL
#define NULL 0

// external variables
static Builtin_Relations_Ptr BuR;

// external functions

extern "C" void popup_Builtin_Relations(void *viewp)
//If no dlg box exists, create one. Else bring up old one.
//bugfix 6952. 7/7/94. Removed a call to clear which used to
//wipe out old settings
{
    if (!BuR)
	BuR = new Builtin_Relations();
    BuR->init(viewp);
#ifndef NEW_UI
    BuR->shell->popup(0);
#endif
};

// pre-processor definitions

// variable definitions

// function definitions
void Builtin_Relations::init(void *viewp)
{
#ifndef NEW_UI
    my_viewp = viewp;
    int flag = apl_OODT_get_builtin_rel_filter(viewPtr(viewp));
    togbox->button("toggleButton1")->set((flag & contains_rel) != 0, 1);
    togbox->button("toggleButton2")->set((flag & points_to_rel) != 0, 1);
    togbox->button("toggleButton3")->set((flag & refers_to_rel) != 0, 1);
    togbox->button("toggleButton4")->set((flag & method_type_rel) != 0, 1);
    togbox->button("toggleButton5")->set((flag & method_argument_rel) != 0, 1);
    togbox->button("toggleButton6")->set((flag & has_friend_rel) != 0, 1);
    togbox->button("toggleButton7")->set((flag & nested_in_rel) != 0, 1);
    togbox->button("toggleButton8")->set((flag & contained_in_rel) != 0, 1);
    togbox->button("toggleButton9")->set((flag & pointed_to_by_rel) != 0, 1);
    togbox->button("toggleButton10")->set((flag & referred_to_by_rel) != 0, 1);
    togbox->button("toggleButton11")->set((flag & returned_by_function_rel) != 0, 1);
    togbox->button("toggleButton12")->set((flag & is_argument_rel) != 0, 1);
    togbox->button("toggleButton13")->set((flag & is_friend_rel) != 0, 1);
    togbox->button("toggleButton14")->set((flag & container_of_rel) != 0, 1);
#endif
};

Builtin_Relations::Builtin_Relations()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "Builtin_Relations", "Built-in Relations");
    shell->add_default_buttons(
	NULL, (gtPushCB)&Builtin_Relations::OK_CB, this,
	NULL, (gtPushCB)&Builtin_Relations::Apply_CB, this,
	NULL, (gtPushCB)&Builtin_Relations::Cancel_CB, this,
	NULL, (gtPushCB)NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.BuiltinRelations");

    togbox = gtToggleBox::create(
	shell, "togbox", NULL,
	"Has Member of Type", "toggleButton1", NULL, NULL,
	"Points to", "toggleButton2", NULL, NULL,
	"Refers to", "toggleButton3", NULL, NULL,
	"Method Returns", "toggleButton4", NULL, NULL,
	"Method Takes Argument", "toggleButton5", NULL, NULL,
	"Has Friend", "toggleButton6", NULL, NULL,
	"Container of", "toggleButton7", NULL, NULL,
	"Type of Member", "toggleButton8", NULL, NULL,
	"Pointed to by", "toggleButton9", NULL, NULL,
	"Referred to by", "toggleButton10", NULL, NULL,
	"Returned by Method", "toggleButton11", NULL, NULL,
	"Argument to Method", "toggleButton12", NULL, NULL,
	"Is Friend", "toggleButton13", NULL, NULL,
	"Nested in", "toggleButton14", NULL, NULL,
	NULL);
    togbox->num_columns(2);
    togbox->attach_side_top();
    togbox->attach_side_left();
    togbox->attach_side_right();
    togbox->manage();
    clear_toggles();
#endif
};

void Builtin_Relations::clear_toggles()
{
#ifndef NEW_UI
    togbox->button("toggleButton1")->set(0,1);
    togbox->button("toggleButton2")->set(0,1);
    togbox->button("toggleButton3")->set(0,1);
    togbox->button("toggleButton4")->set(0,1);
    togbox->button("toggleButton5")->set(0,1);
    togbox->button("toggleButton6")->set(0,1);
    togbox->button("toggleButton7")->set(0,1);
    togbox->button("toggleButton8")->set(0,1);
    togbox->button("toggleButton9")->set(0,1);
    togbox->button("toggleButton10")->set(0,1);
    togbox->button("toggleButton11")->set(0,1);
    togbox->button("toggleButton12")->set(0,1);
    togbox->button("toggleButton13")->set(0,1);
    togbox->button("toggleButton14")->set(0,1);
#endif
};

void Builtin_Relations::do_it()
{
#ifndef NEW_UI
  int flag;
  flag = 0;
  if (togbox->button("toggleButton1")->set())
    flag |= contains_rel;
  if (togbox->button("toggleButton2")->set())
    flag |= points_to_rel;
  if (togbox->button("toggleButton3")->set())
    flag |= refers_to_rel;
  if (togbox->button("toggleButton4")->set())
    flag |= method_type_rel;
  if (togbox->button("toggleButton5")->set())
    flag |= method_argument_rel;
  if (togbox->button("toggleButton6")->set())
    flag |= has_friend_rel;
  if (togbox->button("toggleButton7")->set())
	flag |= nested_in_rel;
  if (togbox->button("toggleButton8")->set())
	flag |= contained_in_rel;
  if (togbox->button("toggleButton9")->set())
	flag |= pointed_to_by_rel;
  if (togbox->button("toggleButton10")->set())
	flag |= referred_to_by_rel;
  if (togbox->button("toggleButton11")->set())
	flag |= returned_by_function_rel;
  if (togbox->button("toggleButton12")->set())
	flag |= is_argument_rel;
  if (togbox->button("toggleButton13")->set())
	flag |= is_friend_rel;
  if (togbox->button("toggleButton14")->set())
        flag |= container_of_rel;
  apl_OODT_set_builtin_rel_filter(viewPtr(my_viewp), flag);
#endif
}

void Builtin_Relations::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_Relations_Ptr bur = Builtin_Relations_Ptr(cd);
    bur->do_it();
    bur->shell->popdown();
#endif
};
    
void Builtin_Relations::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_Relations_Ptr bur = Builtin_Relations_Ptr(cd);
    bur->do_it();
#endif
};
    
void Builtin_Relations::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_Relations_Ptr bur = Builtin_Relations_Ptr(cd);
    bur->shell->popdown();
#endif
};
/*
   START-LOG-------------------------------------------

   $Log: Builtin_Relations.h.C  $
   Revision 1.9 1996/12/02 11:12:18EST pero 
   name changes of built-in relations
Revision 1.2.1.5  1994/07/07  18:58:49  bhowmik
Bug track: 6952
Preventing a call to clear_toggle in the init method.
This will not wipe out old settings anymore.

Revision 1.2.1.4  1993/06/18  21:22:02  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.3  1993/05/28  22:22:10  wmm
Fix bug 3241.

Revision 1.2.1.2  1992/10/09  19:54:38  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
