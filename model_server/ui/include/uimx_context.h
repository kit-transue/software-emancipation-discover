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
#ifndef _uimx_context_h
#define _uimx_context_h

/*
 ***************************************************************************
 * uimx_context.h
 *
 * Macros for managing instance-specific contexts in the auxdecl section.
 *
 ***************************************************************************
 * TO USE THIS FILE:
 *
 * Place the following lines in the gbldecl section (NOT auxdecl).
 *
 *   #include "uimx_context.h"
 *   #define CONTEXT_PTR CONTEXT_PTR_NAME(uimx_name)
 *   #define CURRENT_CONTEXT CURRENT_CONTEXT_NAME(Uimx_name)
 *
 * Where uimx_name is the interface name, and Uimx_name is the same thing,
 * only capitalized (if it wasn't already).
 *
 ***************************************************************************
 * NOTES:
 *
 * The macro "DESIGN_TIME" is defined only in UIM/X, and only when an
 * interface is opened for editing.  It is not defined for files that are
 * "loaded" into the interpreter.
 * 
 * The types and variables used in these macros do not exist in the UIM/X
 * editing environment, and are only necessary in the compiled versions.
 *
 * This whole thing would be a LOT cleaner if we could rely on having a
 * better C pre-processor.
 *
 ***************************************************************************
 */

#ifndef _general_h
#include <general.h>
#endif

#ifndef DESIGN_TIME	/* Real macros for compiled code. */

#ifndef __STDC__

/* The context typename uses the interface name as is. */
#define CONTEXT_PTR_NAME(uimx_name)paste(_UxC,uimx_name)*

/* The current context uses the capitalized version of the interface name. */
#define CURRENT_CONTEXT_NAME(Uimx_name)paste3(Ux,Uimx_name,Context)

#else

/* The context typename uses the interface name as is. */
#define CONTEXT_PTR_NAME(uimx_name) _UxC##uimx_name*

/* The current context uses the capitalized version of the interface name. */
#define CURRENT_CONTEXT_NAME(Uimx_name) Ux##Uimx_name##Context

#endif


#define PUSH_CONTEXT\
{\
    swidget UxThisWidget;\
    CONTEXT_PTR UxSaveCtx;\
\
    UxThisWidget = UxWidgetToSwidget(UxWidget);\
    UxSaveCtx = CURRENT_CONTEXT;\
    CURRENT_CONTEXT = (CONTEXT_PTR)UxGetContext(UxThisWidget);

#define POP_CONTEXT\
    CURRENT_CONTEXT = UxSaveCtx;\
}

#else			/* Stub macros for UIM/X editor environment */

#define CONTEXT_PTR_NAME(uimx_name) quote(CONTEXT_PTR_NAME)
#define CURRENT_CONTEXT_NAME(Uimx_name) quote(CURRENT_CONTEXT)
#define PUSH_CONTEXT
#define POP_CONTEXT

#endif

#endif /* _uimx_context_h */

/*
    START-LOG-------------------------------

    $Log: uimx_context.h  $
    Revision 1.2 2000/02/09 14:27:19EST builder 
    Merging clearmake system builds to main line
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:27  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

