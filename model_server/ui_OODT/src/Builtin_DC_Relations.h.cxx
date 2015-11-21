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
// Builtin_DC_Relations.h.C
//------------------------------------------
// synopsis:
// Builtin_DC_Relations dialog in libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <Builtin_DC_Relations.h>
#include "gtTogB.h"
#include "OODT_apl_entries.h"
#include <gtPushButton.h>

#undef NULL
#define NULL 0

// external variables
static Builtin_DC_Relations_Ptr BuR;

extern "C" void popup_Builtin_DC_Relations(viewPtr viewp)
//If no dlg box exists, create one else bring up old one.
//bugfix 6952. 7/7/92. Removed a call to clear which was
//wiping up out settings.
{
    if (!BuR)
	BuR = new Builtin_DC_Relations();
    BuR->init(viewp);
#ifndef NEW_UI
    BuR->shell->popup(0);
#endif
};

// pre-processor definitions

// variable definitions

// function definitions
void Builtin_DC_Relations::init(viewPtr viewp)
{
#ifndef NEW_UI
    my_viewp = viewp;
    int flag = apl_OODT_get_builtin_rel_filter(viewp);
    togbox->button("toggleButton1")->set((flag & contains_rel) != 0, 1);
    togbox->button("toggleButton2")->set((flag & points_to_rel) != 0, 1);
    togbox->button("toggleButton3")->set((flag & contained_in_rel) != 0, 1);
    togbox->button("toggleButton4")->set((flag & pointed_to_by_rel) != 0, 1);
#endif
};

Builtin_DC_Relations::Builtin_DC_Relations()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "Builtin_DC_Relations", "Built-in Relations");
    shell->add_default_buttons(
	NULL, &Builtin_DC_Relations::OK_CB, this,
	NULL, &Builtin_DC_Relations::Apply_CB, this,
	NULL, &Builtin_DC_Relations::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.BuiltinDCRelations");

    togbox = gtToggleBox::create(
	shell, "togbox", NULL,
	"Has Member of Type", "toggleButton1", NULL, NULL,
	"Points to", "toggleButton2", NULL, NULL,
	"Type of Member", "toggleButton3", NULL, NULL,
	"Pointed to by", "toggleButton4", NULL, NULL,
	NULL);
    togbox->num_columns(2);
    togbox->attach_side_top();
    togbox->attach_side_left();
    togbox->attach_side_right();
    togbox->manage();
    clear_toggles();
#endif
};

void Builtin_DC_Relations::clear_toggles()
{
#ifndef NEW_UI
    togbox->button("toggleButton1")->set(0,1);
    togbox->button("toggleButton2")->set(0,1);
    togbox->button("toggleButton3")->set(0,1);
    togbox->button("toggleButton4")->set(0,1);
#endif
};

void Builtin_DC_Relations::do_it()
{
#ifndef NEW_UI
  int flag;
  flag = 0;
  if (togbox->button("toggleButton1")->set())
    flag |= contains_rel;
  if (togbox->button("toggleButton2")->set())
    flag |= points_to_rel;
  if (togbox->button("toggleButton3")->set())
	flag |= contained_in_rel;
  if (togbox->button("toggleButton4")->set())
	flag |= pointed_to_by_rel;
  apl_OODT_set_DC_builtin_rel_filter(my_viewp, flag);
#endif
}

void Builtin_DC_Relations::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_DC_Relations_Ptr bur = Builtin_DC_Relations_Ptr(cd);
    bur->do_it();
    bur->shell->popdown();
#endif
};
    
void Builtin_DC_Relations::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_DC_Relations_Ptr bur = Builtin_DC_Relations_Ptr(cd);
    bur->do_it();
#endif
};
    
void Builtin_DC_Relations::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Builtin_DC_Relations_Ptr bur = Builtin_DC_Relations_Ptr(cd);
    bur->shell->popdown();
#endif
};
/*
   START-LOG-------------------------------------------

   $Log: Builtin_DC_Relations.h.C  $
   Revision 1.7 1996/12/02 11:12:16EST pero 
   name changes of built-in relations
// Revision 1.4  1994/07/07  19:02:34  bhowmik
// Bug track: 6952
// Preventing a call to clear toggle in the pop up method so as
// not to wipe out old settings.
//
// Revision 1.3  1993/06/18  21:21:55  wmm
// Hook up "help" buttons, change some terminology to be consistent with
// C++ and ParaSET usage.
//
// Revision 1.2  1993/05/28  22:22:04  wmm
// Fix bug 3241.
//
// Revision 1.1  1992/12/17  21:32:22  wmm
// Initial revision
//

   END-LOG---------------------------------------------

*/

