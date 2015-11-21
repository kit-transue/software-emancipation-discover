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
#ifndef STAGE_H
#    define STAGE_H

#include <Dt.h>
struct StageObj;
typedef int (*FP)();

static int count;

struct PipeObj {

    Widget da_wid ;
    int     id ;
    FP user_init ;

    struct _line {
        int      s_x, s_y ;                     /* start coord */
        int      e_x, e_y ;                     /* end coord */
        unsigned width  ;
        Pixel    fg  ;
        Pixel    bg  ;
    } line ;

    struct _gc_pipe {
        GC fg ;
    } gc ;

    struct ActSite {
        Widget id ;
        Widget popup ;
        int      x, y ;                      /* top left coord */
        unsigned width  ;
        unsigned height ;
        int      cent_x, cent_y ;            /* center left coord */
        Pixel    fg  ;
        Pixel    bg  ;
        XtCallbackProc pb_callback ;
        XtCallbackProc pu_callback ;
    } act_site ;

    struct StageObj *from_stage ;
    struct StageObj *to_stage ;

};

struct StageObj {

    char    name[32] ;
    Widget  da_wid ;
    FP user_init;

    struct _wid {
        char *name ;
        Widget id ;
        int x, y ;                      /* top left coord */
        unsigned width  ;
        unsigned height ;
        int cent_x, cent_y ;            /* center left coord */
        Pixel    fg  ;
        Pixel    bg  ;
    } wid ;

    struct PipeObj *in_pipe ;
    struct PipeObj *out_pipe ;
} ;

#define StageName(stgp)   (stgp->wid.name)
#define StageX(stgp)      (stgp->wid.x)
#define StageY(stgp)      (stgp->wid.y)
#define StageWidth(stgp)  (stgp->wid.width)
#define StageHeight(stgp) (stgp->wid.height)
#define StageCX(stgp)      (stgp->wid.cent_x)
#define StageCY(stgp)      (stgp->wid.cent_y)


#endif /* STAGE_H */
