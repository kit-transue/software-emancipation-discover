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
#ifndef __button_h
#define __button_h
// button
//------------------------------------------
// synopsis:
// Alpha-set definitions for events.
//
// description:
// Alpha-set definitions for events.  This includes
// mouse buttons and keyboard events.  Event modifers
// (ctrl, shift, etc.) are also defined.
//------------------------------------------
// Restrictions:
// Note that the mouse-button-down events have
// special modifiers.
//------------------------------------------


//
//  Button types
//
typedef int eventType;
#define eventType_Undefined     (eventType) 0

#define eventType_Left          (eventType) 1
#define eventType_Middle        (eventType) 2
#define eventType_Right         (eventType) 3
// Mouse events are button types

#define eventType_MouseMove     (eventType) 4
#define eventType_EnterEvent    (eventType) 5
#define eventType_ExitEvent     (eventType) 6

// Keyboard Events
#define eventType_KeyEvent      (eventType) 7
#define eventType_FocusInEvent  (eventType) 8
#define eventType_FocusOutEvent (eventType) 9

// Number of button types
#define eventType_size 9

//
//  Button modifiers
//
typedef unsigned long eventModifier;

#define eventMod_None       (eventModifier) 0x0000
#define eventMod_Meta	  (eventModifier) 0x0001
#define eventMod_Shift	  (eventModifier) 0x0002
#define eventMod_Ctrl	  (eventModifier) 0x0004
#define eventMod_Down	  (eventModifier) 0x0008
#define eventMod_Up 	  (eventModifier) 0x0010
#define eventMod_Double	  (eventModifier) 0x0020
#define eventMod_LeftDown	  (eventModifier) 0x0040
#define eventMod_MiddleDown (eventModifier) 0x0080
#define eventMod_RightDown  (eventModifier) 0x0100
#define eventMod_ShiftLock  (eventModifier) 0x0200


// Number of possible modifier combinations
#define eventModifier_size (int) 512


class button
{
 public:
   eventType     which;
   eventModifier type;

   button(const eventType& et, const eventModifier& em) { which = et; type = em; }

   int operator==(const button& btn)
     { return ( (which == btn.which) && (type == btn.type) ); }
};
/*
   START-LOG-------------------------------------------

   $Log: button.h  $
   Revision 1.1 1994/03/23 19:06:55EST builder 
   made from unix file
 * Revision 1.2.1.5  1994/02/09  17:28:56  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.4  1994/02/09  17:17:01  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.3  1994/02/08  19:20:46  builder
 * ort
 *
 * Revision 1.2.1.2  1992/10/09  18:52:48  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
