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
/* File: xip.c                                                       */
/* Author: Doug Abel                                                 */
/* The following file implements the graphics of an image processor. */

#define _NO_PROTO

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* X11 and Motif includes */
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */
#include <Dt.h>                         /* Dt Toolkit */

#define __MAIN__

/* Local Includes */
#include <xip.h>
#include <externals.h>
#include <canvas.h>

/* Local Defines */

#define APP_NAME    "xip"
#define CLASS_NAME  "XIp"


/* Static Globals */

    /* XXX  Find out how to use this */
    static XrmOptionDescRec CommandLineOptions[] = {
    {"-debug",       "*debug",           XrmoptionNoArg,   (caddr_t)"on"  },
    };

    /* Mrm Registered routines */
    static MRMRegisterArg reglist[] = {

        {"UCrCanvasDA",     (caddr_t) UCrCanvasDA},
        {"UExposeCanvasDA", (caddr_t) UExposeCanvasDA},
        {"UResizeCanvasDA", (caddr_t) UResizeCanvasDA},

        {"UPreProcActivatePB"  , (caddr_t) UPreProcActivatePB},
        {"UFFTActivatePB"      , (caddr_t) UFFTActivatePB},
        {"UFilterActivatePB"   , (caddr_t) UFilterActivatePB},
        {"UInvFFTActivatePB"   , (caddr_t) UInvFFTActivatePB},
        {"UPostProcActivatePB" , (caddr_t) UPostProcActivatePB},

        {"UiwoCBMenu" , (caddr_t) UiwoCBMenu}
    };

    static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    static char *G_dbFileList[1];

    static int G_dbNumFiles = 1;
              
void get_uid_name (exec_name)
char *exec_name;
{
    char *dir = strrchr (exec_name, '/');
    static char tmp[1024];
    int len;

    if (dir == 0)
      strcpy (tmp, "./");
    else {
      len = dir-exec_name+1;
      strncpy (tmp, exec_name, len);
      tmp[len] = '\0';
    }

    strcat (tmp, "xip.uid");
    G_dbFileList[0] = tmp;
    return;
}

InterfaceInit (argc, argv)
   int argc ;
   char **argv ;
{
    int i;

    DBG_IN("InterfaceInit") ;

    Control.displayName  = NULL ; /* Init the display name */
    MrmInitialize();              /* Initialize MRM */
    XtToolkitInitialize () ;      /*   Initialize The Toolkit   */

    /* Register the items MRM needs to bind for us. */
    MrmRegisterNames(reglist, reglist_num);

    /*   Get the Initial context for the application */
    Control.appContext = XtCreateApplicationContext();
    Control.mainFrame  = 0x0 ;

    /*   Open the Display */
    G_dpy = XtOpenDisplay(Control.appContext,
			Control.displayName,
                        APP_NAME,
			CLASS_NAME,
                        CommandLineOptions,
			XtNumber(CommandLineOptions),
                        &argc,
			argv
			);

    /* DEBUG */
        XSynchronize(G_dpy, 1 ) ;

    if (G_dpy == NULL ) {
        DT_ERROR( stderr, "%s: cannot connect to X server %s\n", APP_NAME,
                       XDisplayName(Control.displayName));
        exit ( -1 );
    }

    G_screen  = DefaultScreenOfDisplay(G_dpy);
    G_cmap    = DefaultColormap(G_dpy, DefaultScreen(G_dpy));

    DBG_OUT("InterfaceInit") ;
    return(DT_OK) ;
}

CreateMainApplication()
{
    DBG_IN("CreateMainApplication") ;

    RESET_NUM_ARGS;
    SET_WID_ARG(XmNscreen, G_screen);

    SET_WID_ARG( XmNx, 10 );
    SET_WID_ARG( XmNy, 10);
    SET_WID_ARG( XmNwidth, 1000);
    SET_WID_ARG( XmNheight,150);


    /*   Create the application Shell   */
    Control.appShell = XtAppCreateShell( "X Image Processor", "xip",
                                          applicationShellWidgetClass,
                                          G_dpy,
                                          DT_WID_ARGS) ;

    /* Open the UID files (the output of the UIL compiler) in the hierarchy*/

    if (MrmOpenHierarchy(
        G_dbNumFiles,                   /* Number of files. */
        G_dbFileList,                   /* Array of file names.  */
        NULL,                           /* Default OS extenstion. */
        &Control.s_MrmHierarchy)        /* Pointer to returned MRM ID */
        !=MrmSUCCESS) {
            DT_ERROR( stderr, "%s: cannot open uid hierarchy \n", APP_NAME);
            exit (-1);
    }


    /* Go get the main part of the application. */
    if (MrmFetchWidget(Control.s_MrmHierarchy, "xipFrame",
        Control.appShell, &Control.mainFrame, &G_dummyClass) != MrmSUCCESS){
            DT_ERROR( stderr, "%s: can't fetch xipFrame\n", APP_NAME);
            exit (-1);
    }

    /* Map and manage the widgets */
    XtManageChild(Control.mainFrame);
    XtRealizeWidget(Control.appShell) ;
    XtMapWidget(Control.appShell);

    DBG_OUT("CreateMainApplication") ;
    return(DT_OK) ;
}

/* main function */

main (argc, argv)
   int argc ;
   char **argv ;
{
    get_uid_name(argv[0]);

    get_bitmap_name ( argv[0] );
    
#   ifdef DEBUG
    Dt_debug = 1 ;
    Dt_into  = 0 ;
    Dt_outof = 0 ;
    Dt_msg   = 1 ;
    Dt_error = 1 ;
#   endif

    DBG_IN("main") ;

    InterfaceInit (argc, argv) ;
    DtInit() ;
    CreateMainApplication() ;
    /* CanvasInit(); 
    PipeLineInit(GetCanvasWidId()); */

    DBG_MSG("XtAppMainLoop") ;
    XtAppMainLoop(Control.appContext);

    DBG_OUT("main") ;
}
