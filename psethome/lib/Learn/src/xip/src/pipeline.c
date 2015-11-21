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
/* File: pipeline.c                                                 */
/* Author: Doug Abel                                                */
/* The following file implements the graphics of the pipeline of an */
/* image processor.                                                 */

#define _NO_PROTO

#include <xip.h>
#include <externals.h>
#include <stage.h>
#include <image_win.h>


/*
 * Local defines
 */

#define PL_NONE			-1
#define PL_NEW_VIEW_WIN		0
#define PL_REM_VIEW_WIN		1

#define IMO_REMOVE		0
#define IMO_SEE_OPTIONS		1
#define IMO_SCALE		2

extern Display                  *G_dpy;

/*
 *    Routine Declaration
 */
    int PreProcInit() ;
    int FFTInit() ;
    int PipeInit() ;
    int FilterInit() ;
    int InvFFTInit() ;
    int PostProcInit() ;
    int ImageInit() ;


AddStages(DAwid,i)
Widget DAwid;
int i;
{
   GPL.stage_color[i] =
     (Pixel) DtAllocateColor(G_dpy, "orange");

   GPL.stage[i] = (struct StageObj *)
     CreateStage(FilterInit,
                 DAwid, "Filter",
                 GPL.stage_color[i]);
}


void InitStages(DAwid)
    Widget DAwid;
{
    int i = 0;
    int x;

    NUM_STAGES = 5;

    GPL.stage = (struct StageObj **) malloc( sizeof(struct StageObj *) * NUM_STAGES );
    GPL.pipe = (struct PipeObj **) malloc( sizeof(struct PipeObj *) * NUM_PIPES);

    GPL.stage_color = (Pixel *) malloc( sizeof(Pixel) * NUM_STAGES);
    GPL.pipe_color = (Pixel *) malloc( sizeof(Pixel) * NUM_PIPES);

    /* first stage */
    printf("\nfirst stage");
    GPL.stage_color[i] =
      (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.stage[ST_PREPROC] = (struct StageObj *)
      CreateStage(PreProcInit, DAwid, "PreProcess", GPL.stage_color[i]);
    i++;

    /* second stage */
    printf("\nsecond stage");
    GPL.stage_color[i] =
      (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.stage[ST_FFT] = (struct StageObj *)
      CreateStage(FFTInit, DAwid, "FFT", GPL.stage_color[i]);
    i++;

    /* third stage */
    printf("\nthird stage");
    GPL.stage_color[i] =
      (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.stage[i] = (struct StageObj *)
      CreateStage(FilterInit, DAwid, "Filter", GPL.stage_color[i]);
    i++;

    /* fourth stage */
    printf("\nfourth stage");
    GPL.stage_color[i] =
      (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.stage[i] = (struct StageObj *)
      CreateStage(InvFFTInit, DAwid, "Inv FFT", GPL.stage_color[i]);
    i++;
    
    /* fifth stage */
    printf("\nfifth stage");
    GPL.stage_color[i] =
      (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.stage[i] = (struct StageObj *)
      CreateStage(PostProcInit , DAwid, "PostProcess", GPL.stage_color[i]);
    i++;

}

/*
 * Create the image pipeline
 */

PipeLineInit(DAwid)
    Widget DAwid ;
{
    int i ;

    /* Init variables */
    GPL.curr_pipe_num  = PL_NONE ;
    GPL.num_image_wins = 0 ;

    InitStages(DAwid);

    /**************** Create the connection pipes ******************/

    i = 0;

    GPL.pipe_color[i] = (Pixel) DtAllocateColor(G_dpy, "purple");
    GPL.pipe[i]       = (struct PipeObj *) CreatePipe(PipeInit , DAwid, i, GPL.pipe_color[i]);
    i++;

    GPL.pipe_color[i] = (Pixel) DtAllocateColor(G_dpy, "yellow");
    GPL.pipe[i]       = (struct PipeObj *) CreatePipe(PipeInit , DAwid, i, GPL.pipe_color[i]);
    i++;

    for ( ; i< NUM_STAGES-2; i++) {
       GPL.pipe_color[i] = (Pixel) DtAllocateColor(G_dpy, "green");
       GPL.pipe[i]       = (struct PipeObj *) CreatePipe(PipeInit , DAwid, i, GPL.pipe_color[i]);
     }

    GPL.pipe_color[i] = (Pixel) DtAllocateColor(G_dpy, "blue");
    GPL.pipe[i]       = (struct PipeObj *) CreatePipe(PipeInit , DAwid, i, GPL.pipe_color[i]);


    /**************** Connect the stages with the pipes **************/

    i = 0 ;

    StageSetOutputPipe(GPL.stage[ST_PREPROC] , GPL.pipe[ST_PREPROC]) ;

    StageSetInputPipe (GPL.stage[ST_FFT]     , GPL.pipe[i++]) ;
    StageSetOutputPipe(GPL.stage[ST_FFT]     , GPL.pipe[i]) ;

    for ( ; i< NUM_STAGES-3; i++) {
      StageSetInputPipe (GPL.stage[i+1]  , GPL.pipe[i]) ;
      StageSetOutputPipe(GPL.stage[i+1]  , GPL.pipe[i+1]) ;
    }

    StageSetInputPipe (GPL.stage[NUM_STAGES-2]  , GPL.pipe[i++]) ;
    StageSetOutputPipe(GPL.stage[NUM_STAGES-2]  , GPL.pipe[i]) ;

    StageSetInputPipe (GPL.stage[NUM_STAGES-1], GPL.pipe[i]) ;

    ConnectPipeline() ;

    DrawShadowsOnCanvas();

    return(DT_OK) ;
}



 /* AAA */
#define X_SH_OFF 10
#define Y_SH_OFF 10

DrawShadowsOnCanvas()
{
    struct StageObj *stg, *from, *to ;
    struct PipeObj  *pipe ;
    int i ;

    for ( i=0; i<NUM_STAGES ; i++) {
        stg = GPL.stage[i] ;
        CanvasRectShadow(stg->wid.x, stg->wid.y ,
 			 stg->wid.width, stg->wid.height);
    }

    for ( i=0; i<NUM_PIPES ; i++) {
        from = (struct StageObj *)GPL.pipe[i]->from_stage ;
        to   = (struct StageObj *)GPL.pipe[i]->to_stage ;
        CanvasLineShadow(from->wid.cent_x, from->wid.cent_y,
                                    to->wid.cent_x,  to->wid.cent_y) ;
    }

    for ( i=0; i<NUM_PIPES ; i++) {
        pipe = GPL.pipe[i] ;
        CanvasRectShadow(pipe->act_site.x, pipe->act_site.y ,
                                pipe->act_site.height, pipe->act_site.width);
    }

    return(DT_OK);
}

ConnectPipeline()
{
    int i ;
    DBG_IN("ConnectPipeline") ;

    for (i=0; i<NUM_PIPES ; i++) {
        PipeConnect(GPL.pipe[i]) ;
    }

    DBG_OUT("ConnectPipeline") ;
    return(DT_OK);
}


/*
 * Pre Process  routines
 * The init routines are called back from the create routines
 * for these objects.
 */

PreProcInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("PreProcInit") ;
    stg->wid.name = "preProcPB" ;

    DBG_OUT("PreProcInit") ;
}

XtCallbackProc
UPreProcActivatePB ( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UPreProcActivatePB") ;
    DBG_OUT("UPreProcActivatePB") ;
    return(0) ;
}

/*
 * FFT
 */


FFTInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("FFTInit") ;
    stg->wid.name = "fftPB" ;

    DBG_OUT("FFTInit") ;
}

XtCallbackProc
UFFTActivatePB ( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UFFTActivatePB") ;
    DBG_OUT("UFFTActivatePB") ;
    return(0) ;
}

/*
 * Filter
 */


FilterInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("FilterInit") ;
    stg->wid.name = "filterPB" ;

    DBG_OUT("FilterInit") ;
}

XtCallbackProc
UFilterActivatePB ( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UFilterActivatePB") ;
    DBG_OUT("UFilterActivatePB") ;
    return(0) ;
}

/*
 * Inverse FFT
 */


InvFFTInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("InvFFTInit") ;
    stg->wid.name = "invfftPB" ;

    DBG_OUT("InvFFTInit") ;
}

XtCallbackProc
UInvFFTActivatePB ( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UInvFFTActivatePB") ;
    DBG_OUT("UInvFFTActivatePB") ;
    return(0) ;
}

/*
 * Post Processing
 */


PostProcInit(stg)
    struct StageObj *stg ;
{
    DBG_IN("PostProcInit") ;
    stg->wid.name = "postprocPB" ;

    DBG_OUT("PostProcInit") ;
}

XtCallbackProc
UPostProcActivatePB ( widget, tag, reason)
    Widget widget;
    int *tag;
    unsigned long *reason;
{
    DBG_IN("UPostProcActivatePB") ;
    DBG_OUT("UPostProcActivatePB") ;
    return(0) ;
}

/*
 * Pipe routines
 */

PipeInit(pipe)
    struct PipeObj *pipe ;
{
    DBG_IN("PipeInit") ;

    pipe->act_site.pb_callback = ActiveSiteActivatePB ;
    pipe->act_site.pu_callback = ActiveSitePU ;

    DBG_OUT("PipeInit") ;
    return(DT_OK) ;
}

/*
 * Image Routines 
 */

NewImageWin(pipe_num)
    int pipe_num ;
{
    DBG_IN("NewImageWin") ;

    /* ENHANCE :- make this into an info popup */
    if (GPL.num_image_wins >= NUM_IMAGE_WINS) {
        DT_ERROR(stderr,"%s:%d: You have the max number of image wins open\n",
                                                             __FILE__,__LINE__);
        return(DT_OK);
    }


    GPL.iwo[GPL.num_image_wins] =
                CreateImageWin(ImageInit, GPL.num_image_wins, pipe_num) ;

    GPL.num_image_wins ++ ;

    DBG_OUT("NewImageWin") ;
    return(DT_OK) ;
}

ImageInit(iwo)
    struct ImageWin *iwo ;
{
    DBG_IN("ImageInit") ;

    /* get frame color from the pipes active site */
    iwo->pixel.fg = GPL.pipe[iwo->pipe_num]->act_site.fg ;
    iwo->pixel.bg = GPL.pipe[iwo->pipe_num]->act_site.bg ;

    DBG_OUT("ImageInit") ;
    return(DT_OK) ;
}



/*
 * Callbacks
 */

XtCallbackProc
ActiveSiteActivatePB( widget, pipe, pbStruct)
    Widget widget;
    struct PipeObj *pipe;
    XmPushButtonCallbackStruct *pbStruct;
{
    DBG_IN("ActiveSiteActivatePB") ;

    DBG_FPRINT(stderr,"\tPipe number %d\n", pipe->id ) ;

    /* Keep this as the selected pipe */
    GPL.curr_pipe_num = pipe->id ;

    XmMenuPosition(pipe->act_site.popup, pbStruct->event) ;
    XtManageChild(pipe->act_site.popup) ;

    DBG_OUT("ActiveSiteActivatePB") ;
    return(0) ;
}

XtCallbackProc
ActiveSitePU( menu, item_no, cbs)
    Widget menu;
    int item_no ;
    XmAnyCallbackStruct *cbs;
{
    DBG_IN("ActiveSitePU") ;

    switch(item_no) {

        case PL_NEW_VIEW_WIN:
            DBG_FPRINT(stderr,"\tNew view on site  number %d\n",
                                                GPL.curr_pipe_num ) ;

            NewImageWin(GPL.curr_pipe_num) ;
            break ;

        case PL_REM_VIEW_WIN:
            DBG_FPRINT(stderr,"\tRem view on site  number %d\n",
                                                GPL.curr_pipe_num ) ;
            break ;

        default:
            DT_ERROR(stderr,"ERROR:\t%s:%d: Bad item number %d\n",
                                        __FILE__,__LINE__, item_no ) ;
            break ;

    }
    DBG_OUT("ActiveSitePU") ;
    return(0);
}


void
UiwoCBMenu( widget, button_num, pbStruct)
    Widget widget;
    int    *button_num;
    XmPushButtonCallbackStruct *pbStruct;
{
    DBG_IN("UiwoCBMenu") ;

    switch(*button_num) {

        case IMO_REMOVE:
            DBG_FPRINT(stderr,"\tIMO_REMOVE \n") ;

            break ;

        case IMO_SEE_OPTIONS:
            DBG_FPRINT(stderr,"\tIMO_SEE_OPTIONS \n");
            break ;

        case IMO_SCALE:
            DBG_FPRINT(stderr,"\tIMO_SCALE  \n");
            break ;

        default:
            DT_ERROR(stderr,"ERROR:\t%s:%d: Bad item number %d\n",
                                        __FILE__,__LINE__, button_num ) ;
            break ;

    }
    DBG_OUT("UiwoCBMenu");
  }

