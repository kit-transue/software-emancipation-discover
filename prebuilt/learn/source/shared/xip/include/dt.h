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
#ifndef DT_MACROS_H
#    define DT_MACROS_H

#define _NO_PROTO

#include <stdio.h>
#include <Mrm/MrmAppl.h>                /* Motif Toolkit and MRM */

#ifndef __FILE__
#define __FILE__ ""
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

/*
 * Global Defines
 */
#    define     DT_MAX_WIDGETS     100
#    define     DT_MAX_WIDGET_ARGS 50

#    define     DT_OK      1
#    define     DT_BAD    -1

#    define     DT_NO_FONT -1

/*
 *  external Defines
 */
    extern     Cardinal    DtNumArgs;
    extern     Arg         DtWidArgList[DT_MAX_WIDGET_ARGS];

    extern     int         Dt_debug;
    extern     int         Dt_into;
    extern     int         Dt_outof;
    extern     int         Dt_msg;
    extern     int         Dt_error;

/*
 *      Macros
 */
#    define DT_WID_ARGS        DtWidArgList, DtNumArgs
#    define RESET_NUM_ARGS     DtNumArgs = 0

#    define SET_WID_ARG( name, value )                                      \
        if (DtNumArgs >= XtNumber(DtWidArgList))                            \
         {fprintf(stderr,"%s:%d:  DT_WID_ARGS overrun!\n",__FILE__,__LINE__);} \
        XtSetArg( DtWidArgList[DtNumArgs], name,  value ); ++DtNumArgs

#    define GET_WID_ARG( name, value )                                      \
        if (DtNumArgs >= XtNumber(DtWidArgList))                            \
         {fprintf(stderr,"%s:%d:  DT_WID_ARGS overrun!\n",__FILE__,__LINE__);} \
        XtSetArg( DtWidArgList[DtNumArgs], name,  value ); ++DtNumArgs


#    define SET_WID_VALUES_AND_RESET( daWid )                          \
        XtSetValues( daWid, DtWidArgList,  DtNumArgs ); DtNumArgs = 0

#    define SET_WID_VALUES( daWid )                                    \
        XtSetValues( daWid, DtWidArgList,  DtNumArgs )

#    define GET_WID_VALUES_AND_RESET( daWid )                          \
        XtGetValues( daWid, DtWidArgList,  DtNumArgs ); DtNumArgs = 0

#    define GET_WID_VALUES( daWid )                                    \
        XtGetValues( daWid, DtWidArgList,  DtNumArgs )

#define DBG_ROUTINE(in_out, name) 				   	\
    if (Dt_debug) {							\
	fprintf(stderr,"%s %s \n",in_out, name) ;			\
    }

#define DBG_IN(name) 							\
    if (Dt_into) DBG_ROUTINE(">>>>",name)

#define DBG_OUT(name)							\
    if (Dt_outof) DBG_ROUTINE("<<<<",name)

#define DBG_MSG(string)							\
    if (Dt_debug && Dt_msg ) fprintf(stderr,"     %s\n",string)

#define DBG_FPRINT    if (Dt_debug) fprintf
#define DT_ERROR      if (Dt_error) fprintf


#endif /* DT_MACROS_H */
