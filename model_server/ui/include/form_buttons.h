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
/*
 * form_buttons.h
 *
 * Macros for positioning buttons in a form widget.
 *
 * To use this file:
 *
 *   #define NUM_BUTTONS to the number of buttons in the form's "action area."
 *
 *   Set the form's FractionBase resource to BUTTON_BASE.
 *
 *   Set each button's left and right constraints to attach_position, and
 *   the left and right position to BUTTON_LEFT(i) and BUTTON_RIGHT(i)
 *   respectively, where i is the index of the button, starting at zero.
 *
 *   You may also redefine BUTTON_SPACE and/or BUTTON_WIDTH to adjust the
 *   ratio of spacer to button width.
 *
 */

#define BUTTON_BASE \
(((NUM_BUTTONS + 1) * BUTTON_SPACE) + (NUM_BUTTONS * BUTTON_WIDTH))

#define BUTTON_LEFT(i) \
((((i) + 1) * BUTTON_SPACE) + ((i) * BUTTON_WIDTH))

#define BUTTON_RIGHT(i) \
((((i) + 1) * BUTTON_SPACE) + ((i + 1) * BUTTON_WIDTH))

#define BUTTON_SPACE 1
#define BUTTON_WIDTH 10

/*
    START-LOG-------------------------------

    $Log: form_buttons.h  $
    Revision 1.1 1993/05/27 16:23:16EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  19:56:50  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

