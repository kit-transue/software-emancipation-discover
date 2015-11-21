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
// PROJ_View_Pref.h.C
//------------------------------------------
// synopsis:
// PROJ_View_Pref dialog in libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include "machdep.h"
#include <PROJ_View_Pref.h>
#include <genError.h>
#include <systemMessages.h>
#include <representation.h>
#include <ui-globals.h>
#include <gtPushButton.h>

// external variables
static PROJ_View_Pref_Ptr pvpp;
static int magfact = 11;

// external functions
extern "C" void popup_PROJ_view_pref()
{
    if (!pvpp)
	pvpp = new PROJ_View_Pref();
#ifndef NEW_UI
    pvpp->shell->popup(0);
#endif
}

extern "C" int getmagfact()
{
    return magfact;
}

// pre-processor definitions

// variable definitions

// function definitions
PROJ_View_Pref::PROJ_View_Pref()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "PROJ_View_Pref", TXT("View Preferences")
	    ,30,30);
    shell->add_default_buttons(
	NULL, &PROJ_View_Pref::OK_CB, this,
	NULL, &PROJ_View_Pref::Apply_CB, this,
	NULL, &PROJ_View_Pref::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Viewer.View.ViewPrefs.Help");
    shell->pos(400,300);

    magFactorScale = gtScale::create(shell, "magFactorScale", "Magnification Factor",
				     gtHORZ, 11, 20);
    magFactorScale->show_value(1);
    magFactorScale->decimal_points(1);
    magFactorScale->value(11);
    magFactorScale->attach_side_left();
    magFactorScale->attach_side_right();
    magFactorScale->attach_side_top();
    magFactorScale->attach_side_bottom();
    magFactorScale->manage();
#endif
}

void PROJ_View_Pref::do_it()
{
    Initialize(PROJ_View_Pref::do_it);
#ifndef NEW_UI
    magfact = magFactorScale->value();
#endif
    Return
}

void PROJ_View_Pref::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
    Initialize(PROJ_View_Pref::OK_CB);
#ifndef NEW_UI
    PROJ_View_Pref_Ptr PVPP = PROJ_View_Pref_Ptr(cd);
    PVPP->do_it();
    PVPP->shell->popdown();
    Return
#endif
}
    
void PROJ_View_Pref::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
    Initialize(PROJ_View_Pref::Apply_CB);
#ifndef NEW_UI
    PROJ_View_Pref_Ptr PVPP = PROJ_View_Pref_Ptr(cd);
    PVPP->do_it();
    Return
#endif
}
    
void PROJ_View_Pref::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
    Initialize(PROJ_View_Pref::Cancel_CB);
#ifndef NEW_UI
    PROJ_View_Pref_Ptr PVPP = PROJ_View_Pref_Ptr(cd);
    PVPP->shell->popdown();
    Return
#endif
}


/*
   START-LOG-------------------------------------------

   $Log: PROJ_View_Pref.h.C  $
   Revision 1.3 1996/08/13 11:08:37EDT pero 
   valid path, but context() got set before the button got created (12433)
Revision 1.2.1.4  1994/01/12  21:35:28  andrea
Bug track: 5967
Hooked up help path

Revision 1.2.1.3  1992/10/11  11:31:35  oak
Moved log to end of file.


   END-LOG---------------------------------------------
*/
