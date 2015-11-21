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
#include <stdio.h>
#include "xip.h"
#include "stage.h"

/* Stage */

typedef int (*FPF)();
struct StageObj *CreateStage(user_init, stgDA_wid, name, color)
     FPF user_init ;
     Widget stgDA_wid ;
     char* name;
     Pixel color;
{
    static int LeftMargin=10;
    struct StageObj *stg ;
    XmString item;

    DBG_IN("CreateStage") ;

    stg = (struct StageObj *) malloc(sizeof(struct StageObj)) ;

    stg->user_init = user_init ;
    stg->da_wid    = stgDA_wid ;

    /* Stage Pre-Initialization */
    StagePreInit(stg);

    /* Call to the Signal Processing Function */
    (* stg->user_init)(stg) ;

    /* Drawing the stage box */

    sprintf(stg->name, "%s", name);

    stg->wid.x = LeftMargin; 
    LeftMargin += 1000/NUM_STAGES;

    stg->wid.y = 40 ;
    stg->wid.cent_x = (stg->wid.width>>1)  + stg->wid.x ;
    stg->wid.cent_y = (stg->wid.height>>1) + stg->wid.y ;

    item = XmStringCreateSimple(stg->name) ;

    RESET_NUM_ARGS;
    SET_WID_ARG(XmNlabelString,item);
    SET_WID_ARG(XmNlabelType,XmSTRING);
    SET_WID_ARG(XmNbackground,color);
    SET_WID_ARG( XmNx,      stg->wid.x );
    SET_WID_ARG( XmNy,      stg->wid.y );
    SET_WID_ARG( XmNwidth,  stg->wid.width);
    SET_WID_ARG( XmNheight, stg->wid.height);

    stg->wid.id = (Widget) XmCreatePushButton(stg->da_wid,NULL,
					      DT_WID_ARGS);
    XmStringFree(item);
    XtManageChild((Widget) stg->wid.id);

    DBG_OUT("CreateStage") ;
    return(stg);
}

StagePreInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("StagePreInit") ;

    stg->wid.name   = NULL ;
    stg->wid.id     = NULL ;
    stg->wid.width  = 100 ;
    stg->wid.height = 50 ;

    stg->in_pipe  = NULL ;
    stg->out_pipe = NULL ;

    DBG_OUT("StagePreInit") ;
    return(DT_OK);
}

StageReset(stg)
    struct StageObj *stg ;
{
    DBG_IN ("StageReset") ;
    DBG_OUT("StageReset") ;
    return(DT_OK);
}

StageUnmap(stg)
    struct StageObj *stg ;
{
    DBG_IN ("StageUnmap") ;
    XtUnmapWidget(stg->wid.id) ;
    DBG_OUT("StageUnmap") ;
    return(DT_OK);
}


StageSetColor(stg)
    struct StageObj *stg ;
{
    DBG_IN ("StageSetColor") ;
    DBG_OUT("StageSetColor") ;
    return(DT_OK);
}

StageSetInputPipe(stg, pipe)
    struct StageObj *stg ;
    struct PipeObj  *pipe ;
{
    DBG_IN ("StageSetInputPipe") ;
    stg->in_pipe = pipe ;
    PipeTo(pipe,stg) ;
    DBG_OUT("StageSetInputPipe") ;
    return(DT_OK);
}

StageSetOutputPipe(stg, pipe)
    struct StageObj *stg ;
    struct PipeObj  *pipe ;
{
    DBG_IN ("StageSetOutputPipe") ;
    stg->out_pipe = pipe ;
    PipeFrom(pipe,stg) ;
    DBG_OUT("StageSetOutputPipe") ;
    return(DT_OK);
}

/* Pipe Routines */

struct PipeObj *CreatePipe(user_init, pipeDA_wid, id, color)
    FPF user_init ;
    Widget pipeDA_wid ;
    int id ;
    Pixel color ;
{
    struct PipeObj *pipe ;
    DBG_IN("CreatePipe") ;

    pipe = (struct PipeObj *) malloc(sizeof(struct PipeObj)) ;

    pipe->da_wid    = pipeDA_wid ;
    pipe->user_init = user_init ;
    pipe->id        = id ;

    pipe->gc.fg     = NULL ;

    pipe->act_site.bg = color ;

    PipePreInit(pipe)         ;
    (* pipe->user_init)(pipe) ;
    PipePostInit(pipe)        ;

    CreatePipeGCs(pipe) ;

    DBG_OUT("CreatePipe") ;
    return(pipe);
}

PipePreInit(pipe)
    struct PipeObj *pipe ;
{
    DBG_IN("PipePreInit") ;

    pipe->line.s_x   = 0 ;
    pipe->line.s_y   = 0 ;
    pipe->line.e_x   = 10 ;
    pipe->line.e_y   = 10 ;
    pipe->line.width = 4  ; /* AAA */

    pipe->line.bg = DtAllocateColor(G_dpy, "black") ;
    pipe->line.fg = DtAllocateColor(G_dpy, "blue") ;

    pipe->act_site.width  = 25 ; /* AAA */
    pipe->act_site.height = 25 ; /* AAA */
    pipe->act_site.id = 0x0 ;

    pipe->act_site.fg = DtAllocateColor(G_dpy, "blue") ;

    DBG_OUT("PipePreInit") ;
    return(DT_OK);
}

PipeReset(pipe)
    struct PipeObj *pipe ;
{
    DBG_IN("PipeReset") ;
    DBG_OUT("PipeReset") ;
    return(DT_OK);
}

PipePostInit(pipe)
    struct PipeObj *pipe ;
{
    XmString item_1, item_2 ;
    DBG_IN("PipePostInit") ;

    if (MrmFetchWidget(Control.s_MrmHierarchy, "actSitePB",
	pipe->da_wid, &pipe->act_site.id, &G_dummyClass) != MrmSUCCESS) {
	fprintf(stderr,"%s:%d: can't fetch %s\n",
				__FILE__,__LINE__,"actSitePB");
	return(1);
    }

    XtManageChild(pipe->act_site.id);

    XtAddCallback(pipe->act_site.id,
		XmNactivateCallback,
		pipe->act_site.pb_callback,
		(XtPointer)pipe
		);

    RESET_NUM_ARGS ;

    SET_WID_ARG( XmNwidth,  pipe->act_site.width);
    SET_WID_ARG( XmNheight, pipe->act_site.height);
    /* 
    SET_WID_ARG( XmNforeground, pipe->act_site.fg);
    */
    SET_WID_ARG( XmNbackground, pipe->act_site.bg);
    SET_WID_VALUES(pipe->act_site.id) ;

    item_1 = XmStringCreateSimple("New Image") ;
    item_2 = XmStringCreateSimple("Remove Image") ;

    pipe->act_site.popup =  
		XmVaCreateSimplePopupMenu(pipe->da_wid,"act_popup", 
					pipe->act_site.pu_callback,
		XmVaPUSHBUTTON, item_1, NULL, NULL, NULL,
		XmVaPUSHBUTTON, item_2, NULL, NULL, NULL,
		NULL) ;

    XmStringFree(item_1) ;
    XmStringFree(item_2) ;

    DBG_OUT("PipePostInit") ;
    return(DT_OK);
}

CreatePipeGCs(pipe)
    struct PipeObj *pipe ;
{
    XGCValues gcv ;

    DBG_IN("CreatePipeGCs") ;

    if ( pipe->gc.fg != NULL ) XFreeGC( G_dpy, pipe->gc.fg ) ;
    pipe->gc.fg    = XCreateGC (G_dpy, XtWindow(pipe->da_wid), 0, &gcv);
    XSetBackground (G_dpy, pipe->gc.fg, pipe->line.bg ) ;
    XSetForeground (G_dpy, pipe->gc.fg, pipe->line.fg ) ;
    XSetLineAttributes(G_dpy, pipe->gc.fg, pipe->line.width ,
                                        LineSolid, CapRound, JoinRound);


    DBG_OUT("CreatePipeGCs") ;
    return(DT_OK);
}

PipeTo(pipe, stg)
    struct PipeObj *pipe ;
    struct StageObj *stg ;
{
    DBG_IN("PipeTo") ;

    pipe->to_stage = ( struct StageObj *) stg ;

    DBG_OUT("PipeTo") ;
    return(DT_OK);
}

PipeFrom(pipe, stg)
    struct PipeObj *pipe ;
    struct StageObj *stg ;
{
    DBG_IN("PipeFrom") ;

    pipe->from_stage = ( struct StageObj *) stg ;

    DBG_OUT("PipeFrom") ;
    return(DT_OK);
}

PipeConnect(pipe)
    struct PipeObj *pipe ;
{
    struct StageObj *from, *to ;
    DBG_IN("PipeConnect") ;

    DBG_FPRINT(stderr,"\tPipe number %d\n", pipe->id ) ;

    from = (struct StageObj *)pipe->from_stage ;
    to   = (struct StageObj *)pipe->to_stage ;

    pipe->line.s_x = from->wid.cent_x ;
    pipe->line.s_y = from->wid.cent_y ;

    pipe->line.e_x = to->wid.cent_x ;
    pipe->line.e_y = to->wid.cent_y ;

    pipe->act_site.cent_x = (pipe->line.s_x + pipe->line.e_x) >> 1 ;
    pipe->act_site.cent_y = (pipe->line.s_y + pipe->line.e_y) >> 1 ;
    pipe->act_site.x = pipe->act_site.cent_x - (pipe->act_site.width  >> 1) ;
    pipe->act_site.y = pipe->act_site.cent_y - (pipe->act_site.height >> 1) ;

    RESET_NUM_ARGS ;
    SET_WID_ARG( XmNx,  pipe->act_site.x);
    SET_WID_ARG( XmNy,  pipe->act_site.y);
    SET_WID_VALUES(pipe->act_site.id) ;

    XDrawLine ( G_dpy, XtWindow(pipe->da_wid), pipe->gc.fg,
				pipe->line.s_x, pipe->line.s_y,
					pipe->line.e_x, pipe->line.e_y);

    XtMapWidget(pipe->act_site.id) ;

    XFlush(G_dpy) ;

    DBG_OUT("PipeConnect") ;
    return(DT_OK);
}


