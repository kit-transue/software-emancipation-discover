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
#ifndef XIP_H
#    define XIP_H

#include <stdio.h>
#include <Dt.h>
#include <stage.h>
#include <image_win.h>

/*
 *     Defines
 */

#define APP_NAME    "xip"
#define CLASS_NAME  "XIp"

/*
 *     Global Structures
 */

    Display     *G_dpy ;
    Screen      *G_screen ;
    Colormap     G_cmap ;
    char        *G_prog ;

    MrmType G_dummyClass ;

    struct _control {

        /* Display */
        char                *displayName ;

        /* Widget */
        XtAppContext        appContext ;
        Widget              mainFrame;
        Widget              appShell;

        /* Resource and uil files */
        MrmHierarchy s_MrmHierarchy ;


    };

struct _control Control ;

    /* Pipe line */


#define ST_PREPROC   	0
#define ST_FFT       	1
#define ST_FILTER    	2
#define ST_INVFFT    	(NUM_STAGES-2)
#define ST_POSTPROC  	(NUM_STAGES-1)
#define NUM_PIPES	(GPL.NumStages - 1)
#define NUM_STAGES	(GPL.NumStages)
#define NUM_IMAGE_WINS	6
#define PIPE_NUM_COLORS 4
#define STAGE_NUM_COLORS 5

struct GplObj {
  int             	NumStages;
  int             	curr_pipe_num;
  struct StageObj 	**stage;
  Pixel           	*stage_color;
  struct PipeObj  	**pipe;
  Pixel           	*pipe_color;
  int              	num_image_wins;
  struct ImageWin 	*iwo[NUM_IMAGE_WINS];
};

#ifndef __MAIN__
  extern struct		GplObj GPL ;
#else __MAIN__
  struct GplObj GPL;
#endif __MAIN__

#endif /* XIP_H */



