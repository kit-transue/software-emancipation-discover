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
#ifndef _graResources_h
#define _graResources_h

/*
// graResources.h
//------------------------------------------
// synopsis:
// Names used when accessing X Resource Manager values
//------------------------------------------
// RESTRICTION:
// This file must remain C language compatible.
//------------------------------------------
*/

#ifndef _prototypes_h
#include <prototypes.h>
#endif


BEGIN_C_DECL
psetCONST char* ui_get_value PROTO((const char*, const char*));
END_C_DECL

/* the following definition changed to this werid name because 
   on HP Xt toolkit uses APPLICATION_CLASS name for some searching 
   purpouses.   And if we have  subdirectory with this name we get
   segmentation error. */
#define APPLICATION_CLASS	"ParaSET"

#define ALPHASET_CLASS		APPLICATION_CLASS
#define DRAWING_WINDOW_CLASS	"DrawParaSET"

#define BASE_FOREGROUND		"baseForeground"
#define BASE_BACKGROUND 	"baseBackground"
#define BASE_FONT		"baseFont"

#define NORMAL_FOREGROUND	"normalForeground"
#define NORMAL_BACKGROUND	"normalBackground"
#define NORMAL_FONT      	"normalFont"

#define HILITE_FOREGROUND	"hiliteForeground"
#define HILITE_BACKGROUND	"hiliteBackground"
#define HILITE_FONT		"hiliteFont"
#define HILITE_BINDING		"hiliteBinding"

#define SELECT_FOREGROUND	"selectForeground"
#define SELECT_BACKGROUND	"selectBackground"
#define SELECT_FONT		"selectFont"

#define COLLAPSE_FOREGROUND	"collapseForeground"
#define COLLAPSE_BACKGROUND	"collapseBackground"
#define COLLAPSE_FONT		"collapseFont"

#define HILITE_TEXT_FOREGROUND	"hiliteTextForeground"
#define HILITE_TEXT_BACKGROUND	"hiliteTextBackground"
#define HILITE_TEXT_FONT	"hiliteTextFont"

#define CONNECTION_FOREGROUND	"connectionForeground"
#define CONNECTION_BACKGROUND	"connectionBackground"
#define HILITE_CONNECTION_FOREGROUND "hiliteConnectionForeground"

#define BACKGROUND		"background"
#define FOREGROUND		"foreground"

#define CURSOR_FOREGROUND	"cursorForeground"
#define CURSOR_BACKGROUND	"cursorBackground"

#define FLOW_LABEL_TEXT		"flowchartLabelFont"

#define HAS_FRIEND_CLR		"hasFriendRelClr"
#define IS_FRIEND_CLR		"isFriendRelClr"
#define CONTAINS_CLR		"containsRelClr"
#define CONTAINED_IN_CLR	"containedInRelClr"
#define POINTS_TO_CLR		"pointsToRelClr"
#define POINTED_TO_BY_CLR	"pointedToByRelClr"
#define REFERS_TO_CLR		"refersToRelClr"
#define REFERRED_TO_BY_CLR	"referredToByRelClr"
#define FUNCTION_RETURNS_CLR	"functionReturnsRelClr"
#define RETURNED_BY_FUNCTION_CLR "returnedByFunctionRelClr"
#define TAKES_ARGUMENT_CLR	"takesArgumentRelClr"
#define IS_ARGUMENT_CLR		"isArgumentRelClr"
#define INHERITANCE_CLR		"inheritanceRelClr"
#define NESTED_IN_CLR           "nestedInClr"
#define CONTAINER_OF_CLR         "containerOfClr"
#define ERD_ARROWHEAD_LENGTH	"erdArrowheadLength"
#define ERD_ARROWHEAD_WIDTH	"erdArrowheadWidth"

#define RTL_PROJECT_FORMAT	"rtlProjectFormat"
#define RTL_MODULE_FORMAT	"rtlModuleFormat"
#define RTL_ASSOC_FORMAT	"rtlAssocFormat"


/*
   START-LOG-------------------------------------------

   $Log: graResources.h  $
   Revision 1.3 2000/07/07 08:10:03EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.9  1994/04/26  17:34:47  builder
 * Port
 *
 * Revision 1.2.1.8  1994/03/17  18:28:13  builder
 * Port
 *
 * Revision 1.2.1.7  1994/03/17  00:13:14  builder
 * Chnaged APPLICATION NAME
 * ,
 *
 * Revision 1.2.1.6  1994/01/04  23:53:00  boris
 * Bug track: Macro CONST
 * Renamed macro CONST into psetCONST
 *
 * Revision 1.2.1.5  1993/06/01  18:08:51  glenn
 * Make file C language compatible.
 * Add APPLICATION_CLASS macro.
 *
   END-LOG---------------------------------------------
*/

#endif /* _graResources_h */
