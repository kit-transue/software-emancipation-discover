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
/*
// c_main
//------------------------------------------
// synopsis:
// Main function to initialize the project window
//------------------------------------------
*/

/* INCLUDE FILES */

#ifndef NEW_UI

#include <signal.h>
#include <X11/Xlib.h>
#define Object XObject
#include <UxLib.h>
#undef Object
#include <ui-globals.h>
#include <graResources.h>

void browserShell_load();
extern swidget popup_PROJECT();
void ste_set_Xclient_id ();
int home_proj_selector();

swidget project_swidget;	/* The swidget for the project window */


/*
//------------------------------------------
// c_main
//
// Initialize UIM/X.
//------------------------------------------
*/

void c_main( argc, argv )
    unsigned int *argc;
    char **argv;
{
    UxTopLevel = UxrInitialize (APPLICATION_CLASS, argc, argv);
}

/*
//------------------------------------------
// popup_first_windows
//
// Create the system's first windows.
//------------------------------------------
*/

int popup_first_windows()
{
    logger_help_enable();

    /*Guy: enable the help for the dialogs*/
    gtDialogTemplate_enable_help();
    
    /* Initialize the ste/epoch interface. */
    project_swidget = popup_PROJECT();

    /* send paraset X id to Epoch (if Epoch started) */
    ste_set_Xclient_id (XtWindow(UxGetWidget(project_swidget)));

    /* Bring up the initial browserShell(s) */
    browserShell_load();

    return 1;
}

int popup_very_first_window()
{
    int retval = 0;

    logger_help_disable();
    gtDialogTemplate_disable_help(); 
    if (home_proj_selector()) 
	retval = 1;

    return retval;
}
#endif
