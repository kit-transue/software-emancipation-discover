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
#define _NO_PROTO

#include <xip.h>
#include <image_win.h>

typedef int (* FPF1)();
struct ImageWin *
CreateImageWin (user_init, id,  pipe_num)
    FPF1 user_init  ;
    int id ;
    int pipe_num ;
{
    struct ImageWin *iwo ;

    DBG_IN("CreateImageWin") ;

    iwo = (struct ImageWin *) malloc(sizeof(struct ImageWin)) ;

    iwo->user_init = user_init ;
    iwo->id        = id ;
    iwo->pipe_num  = pipe_num ;
    iwo->frame     = 0x0 ;

    IWPreInit(iwo) ;
    ( * iwo->user_init )(iwo) ;
    IWPostInit(iwo) ;

    DBG_OUT("CreateImageWin") ;
    return(iwo) ;
}


IWPreInit(iwo)
    struct ImageWin *iwo ;
{
    DBG_IN("IWPreInit") ;
    DBG_OUT("IWPreInit") ;
    return(DT_OK);
}

IWPostInit(iwo)
    struct ImageWin *iwo ;
{
    char name[20] ;

    DBG_IN("IWPostInit") ;

    sprintf(name,"Image Win %d", iwo->id ) ;


    RESET_NUM_ARGS;
    SET_WID_ARG( XmNx, 0 );
    SET_WID_ARG( XmNy, 0);
    SET_WID_ARG( XmNwidth, 128);
    SET_WID_ARG( XmNheight, 128);
    /* SET_WID_ARG( XmNmappedWhenManaged, False); */
    iwo->appShell = XtAppCreateShell( name, "iwo",
                                          topLevelShellWidgetClass,
                                          G_dpy,
                                          DT_WID_ARGS) ;

    if (MrmFetchWidget(Control.s_MrmHierarchy, "iwoFrame",
        iwo->appShell, &iwo->frame, &G_dummyClass) != MrmSUCCESS) {
        DT_ERROR(stderr,"%s:%d: can't fetch iwoFrame \n", __FILE__,__LINE__);
        exit(1) ;
    }

    /* Match the frame color to the pipe color */
    {
        Pixel top, bottom, nu_select;

        /* Get foregrournd, and background_shadow */
        XmGetColors(G_screen, G_cmap, iwo->pixel.bg,
                            &iwo->pixel.fg, &top,
                                &bottom, &nu_select) ;

        SET_WID_ARG( XmNforeground, iwo->pixel.fg);
        SET_WID_ARG( XmNbackground, iwo->pixel.bg);
        SET_WID_ARG( XmNtopShadowColor   , top);
        SET_WID_ARG( XmNbottomShadowColor, bottom);
        SET_WID_VALUES(iwo->frame) ;

    }


    XtManageChild(iwo->frame);
    XtRealizeWidget(iwo->appShell) ;

    DBG_OUT("IWPostInit") ;
    return(DT_OK);
}
