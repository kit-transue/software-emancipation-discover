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
// Browser_Explode.h.C
//------------------------------------------
// synopsis:
// conversion of Browser_Explode dialog to libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include "machdep.h"
#include "Browser_Explode.h"
#include "gtTogB.h"
#include <gtPushButton.h>

#undef NULL
#define NULL 0

// external variables
static Browser_Explode_Ptr be;

// external functions
extern "C" void apl_OODT_setup_class_selection(void *, void *);
extern "C" void apl_OODT_explode_entities(void *, int);

extern "C" void popup_Browser_explode(void *arg_viewp, void *popup_node)
{
    if (!be)
	be = new Browser_Explode();
    be->init_fields(arg_viewp, popup_node);
#ifndef NEW_UI
    be->shell->popup(0);
#endif
};

// pre-processor definitions

// variable definitions

// function definitions

Browser_Explode::Browser_Explode()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "Browser_explode", "Display Related Entities");
    shell->add_default_buttons(
	NULL, &Browser_Explode::OK_CB, this,
	NULL, &Browser_Explode::Apply_CB, this,
	NULL, &Browser_Explode::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.DisplayRelatedEntities");
    
    group1 = gtToggleBox::create(
	shell, "group1", NULL,
	"Members", "browser_explode_members", NULL, NULL,
	"Superclasses", "browser_explode_superclasses", NULL, NULL,
	"Subclasses", "browser_explode_subclasses", NULL, NULL,
	"User-Defined Relations", "browser_explode_rels", NULL, NULL,
	NULL);

    group1->attach_side_top();
    group1->attach_side_left();
    group1->attach_side_right();
    group1->manage();
#endif
};

void Browser_Explode::do_it()
{
#ifndef NEW_UI
  int flags;
  flags = 0;
  if (group1->button("browser_explode_members")->set())
    flags |= MEMBERS_EXPLODED;
  if (group1->button("browser_explode_superclasses")->set())
    flags |= SUPERCLASSES_EXPLODED;
  if (group1->button("browser_explode_subclasses")->set())
    flags |= SUBCLASSES_EXPLODED;
  if (group1->button("browser_explode_rels")->set())
    flags |= RELATIONS_EXPLODED;
  apl_OODT_setup_class_selection(popnode, viewp);
  apl_OODT_explode_entities(viewp, flags);
#endif
};

void Browser_Explode::init_fields(void *arg_viewp, void *popup_node)
{
    popnode = popup_node;
    viewp = arg_viewp;
};

void Browser_Explode::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Explode_Ptr BE = Browser_Explode_Ptr(cd);
    BE->do_it();
    BE->shell->popdown();
#endif
};

void Browser_Explode::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Explode_Ptr BE = Browser_Explode_Ptr(cd);
    BE->do_it();
#endif
};

void Browser_Explode::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Explode_Ptr BE = Browser_Explode_Ptr(cd);
    BE->shell->popdown();
#endif
};


/*
   START-LOG-------------------------------------------

   $Log: Browser_Explode.h.C  $
   Revision 1.4 1996/08/12 16:43:25EDT pero 
   help_context_name() cannot be called before the corresponding          
   help_button is created (fixing all related problems to 12238)
 * Revision 1.1  1994/09/21  18:44:32  jerry
 * Initial revision
 *
Revision 1.2.1.4  1993/06/18  21:21:31  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.3  1993/05/29  14:45:16  wmm
Fix bug 3250.

Revision 1.2.1.2  1992/10/09  19:54:30  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
