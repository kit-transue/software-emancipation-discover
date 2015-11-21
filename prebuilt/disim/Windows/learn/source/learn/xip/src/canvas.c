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
#include <stdlib.h>
#include "xip.h"
#include "canvas.h"

CanvasInit()
{
    DBG_IN("CanvasInit") ;

    Gcvs.wid.win   = XtWindow (Gcvs.wid.id) ;

    /* Get necessary data from the widget internals */


    RESET_NUM_ARGS ;
    GET_WID_ARG(XmNcolormap,   &Gcvs.wid.cmap) ;
    GET_WID_ARG(XmNforeground, &Gcvs.pixel.fg) ;
    GET_WID_ARG(XmNbackground, &Gcvs.pixel.bg) ;

    GET_WID_VALUES(Gcvs.wid.id) ;

    /* Get related shadow colors */
    {
        Pixel nu_fg, nu_select, nu_top;

        /* Get background_shadow */
        XmGetColors(XtScreen(Gcvs.wid.id), Gcvs.wid.cmap, 
			Gcvs.pixel.bg,
                            &nu_fg, &nu_top,
                                &Gcvs.pixel.sh_bg, &nu_select) ;

        /* Get foregrournd shadow */
        XmGetColors(XtScreen(Gcvs.wid.id), Gcvs.wid.cmap,
			Gcvs.pixel.fg,
                            &nu_fg, &nu_top,
                                &Gcvs.pixel.sh_fg, &nu_select) ;
    }


    Gcvs.pixmap.bg      = (Pixmap)NULL ;
    Gcvs.pixmap.bg_stip = (Pixmap)NULL ;

    Gcvs.gc.fg		= NULL ;
    Gcvs.gc.shadow	= NULL ;

    CanvasInitDimensions() ;
    CreateCanvasGCs() ;
    CreateCanvasPixmaps() ;

    DBG_OUT("CanvasInit") ;
    return(DT_OK) ;
}

CanvasInitDimensions()
{
    DBG_IN("CanvasInitDimensions") ;
    RESET_NUM_ARGS ;
    GET_WID_ARG(XmNx,        &Gcvs.wid.x) ;
    GET_WID_ARG(XmNy,        &Gcvs.wid.y) ;
    GET_WID_ARG(XmNwidth,    &Gcvs.wid.width) ;
    GET_WID_ARG(XmNheight,   &Gcvs.wid.height) ;
    GET_WID_VALUES(Gcvs.wid.id) ;


    DBG_OUT("CanvasInitDimensions") ;
    return(DT_OK);
}
static char *G_bitmapFile;

get_bitmap_name (exec_name)
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

    strcat (tmp,"cross_hair13x13.bm");
    G_bitmapFile = tmp;
    return;
}

CreateCanvasGCs()
{
    int x, y ;
    unsigned int st_width, st_height;
    XGCValues gcv ;
    char *home;
    static char tmp[1024];

    DBG_IN("CreateCanvasGCs") ;

    /*
     * Drawing Area GCs and Pixmaps
     */

    /* read in the da background stipple from resource file */

   

    XReadBitmapFile(G_dpy, Gcvs.wid.win,
                           G_bitmapFile, /* AAA */
                            &st_width, &st_height,
                                        &Gcvs.pixmap.bg_stip, &x, &y);

    if ( Gcvs.gc.fg != NULL ) XFreeGC( G_dpy, Gcvs.gc.fg ) ;
    Gcvs.gc.fg        = XCreateGC (G_dpy, Gcvs.wid.win, 0, &gcv);
	XSetFillStyle     (G_dpy, Gcvs.gc.fg, FillOpaqueStippled );
	XSetStipple       (G_dpy, Gcvs.gc.fg, Gcvs.pixmap.bg_stip );
	XSetBackground    (G_dpy, Gcvs.gc.fg, Gcvs.pixel.bg);
	XSetForeground    (G_dpy, Gcvs.gc.fg, Gcvs.pixel.fg);
	XSetLineAttributes(G_dpy, Gcvs.gc.fg, 4 /* AAA */,
                                        LineSolid, CapRound, JoinRound);


    if ( Gcvs.gc.shadow != NULL ) XFreeGC( G_dpy, Gcvs.gc.shadow ) ;
    Gcvs.gc.shadow    = XCreateGC (G_dpy, Gcvs.wid.win, 0, &gcv);
	XSetFillStyle     (G_dpy, Gcvs.gc.shadow, FillOpaqueStippled );
	XSetStipple       (G_dpy, Gcvs.gc.shadow, Gcvs.pixmap.bg_stip );
	XSetBackground    (G_dpy, Gcvs.gc.shadow, Gcvs.pixel.sh_bg);
	XSetForeground    (G_dpy, Gcvs.gc.shadow, Gcvs.pixel.sh_fg);
	XSetLineAttributes(G_dpy, Gcvs.gc.shadow, 4 /* AAA */,
                                        LineSolid, CapRound, JoinRound);

    DBG_OUT("CreateCanvasGCs") ;
    return(DT_OK) ;
}


CreateCanvasPixmaps()
{

    DBG_IN("CreateCanvasPixmaps") ;

    if (Gcvs.pixmap.bg != (Pixmap)NULL) {
        XFreePixmap(G_dpy, Gcvs.pixmap.bg ) ;
    }
    Gcvs.pixmap.bg =
        XCreatePixmap (G_dpy, Gcvs.wid.win,
                            Gcvs.wid.width, Gcvs.wid.height,
                                DefaultDepthOfScreen(G_screen) ) ;

    XFillRectangle ( G_dpy, Gcvs.pixmap.bg, Gcvs.gc.fg,
                         0, 0,   Gcvs.wid.width,  Gcvs.wid.height);

    XSetWindowBackgroundPixmap(G_dpy, Gcvs.wid.win, Gcvs.pixmap.bg ) ;

    /* repaint the background */
    XClearWindow(G_dpy, Gcvs.wid.win) ;


    DBG_OUT("CreateCanvasPixmaps") ;
    return(DT_OK) ;
}

/*
 * Methods 
 */

Widget
GetCanvasWidId()
{
    DBG_IN("GetCanvasWidId") ;
    DBG_OUT("GetCanvasWidId") ;
    return(Gcvs.wid.id) ;
}

 /* AAA */
#define X_SH_OFF 30
#define Y_SH_OFF 30


CanvasRectShadow( x, y , width, height)
    int x, y ;
    unsigned width, height ;
{
    DBG_IN("CanvasRectShadow") ;
    
    XFillRectangle ( G_dpy, Gcvs.pixmap.bg, Gcvs.gc.shadow,
				     x+X_SH_OFF, y+Y_SH_OFF,   width,  height);

    XClearWindow(G_dpy, Gcvs.wid.win) ;

    DBG_OUT("CanvasRectShadow") ;
    return(DT_OK);
}

CanvasLineShadow( x1, y1 , x2, y2)
    int x1, y1, x2, y2 ;
{
    DBG_IN("CanvasLineShadow") ;
    
    XDrawLine ( G_dpy, Gcvs.pixmap.bg, Gcvs.gc.shadow,
		 x1+X_SH_OFF, y1+Y_SH_OFF, x2+X_SH_OFF, y2+Y_SH_OFF) ;

    XClearWindow(G_dpy, Gcvs.wid.win) ;

    DBG_OUT("CanvasLineShadow") ;
    return(DT_OK);
}

/*
 * Callbacks
 */
void
UCrCanvasDA( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UCrCanvasDA") ;
    Gcvs.wid.id = widget ;
    DBG_OUT("UCrCanvasDA") ;
}

void
UExposeCanvasDA( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    XEvent xevent ;

    DBG_IN("UExposeCanvasDA") ;

    if (Gcvs.wid.win == 0 ) {
	CanvasInit();
	PipeLineInit(GetCanvasWidId());
    }



    /* Remove any expose events that follow */
    while( XCheckWindowEvent( G_dpy, Gcvs.wid.win, ExposureMask, &xevent)) ;

    if ( Gcvs.wid.win ) {
	ConnectPipeline() ;
    }
    DBG_OUT("UExposeCanvasDA") ;
}

void
UResizeCanvasDA( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    XEvent xevent ;

    DBG_IN("UResizeCanvasDA") ;

    if (Gcvs.wid.win == 0 ) return;

    /* Remove any expose events that follow */
    while( XCheckWindowEvent( G_dpy, Gcvs.wid.win, ExposureMask, &xevent)) ;


    CanvasInitDimensions() ;

    if ( Gcvs.wid.win ) {
	CreateCanvasPixmaps() ;
	DrawShadowsOnCanvas() ;
	ConnectPipeline() ;
    }

    DBG_OUT("UResizeCanvasDA") ;
}

