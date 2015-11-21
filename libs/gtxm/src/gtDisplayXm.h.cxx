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
// gtDisplayXm.h.C
//------------------------------------------
// synopsis:
// 
// Interface to the visual display hardware.
//------------------------------------------

// INCLUDE FILES

#include <x11_intrinsic.h>
#include <gt.h>
#include <gtDisplay.h>


// EXTERNAL VARIABLES

extern Display* UxDisplay;

// FUNCTION DEFINITIONS

int gtDisplay::is_open()
{
    return (UxDisplay != NULL);
}

int gtDisplay::height(void)
{
  int retval = -1;

  if ( is_open() )
    retval = XDisplayHeight(UxDisplay, XDefaultScreen(UxDisplay));

  return retval;
}

int gtDisplay::width(void)
{
  int retval = -1;

  if ( is_open() )
    retval = XDisplayWidth(UxDisplay, XDefaultScreen(UxDisplay));

  return retval;
}

int gtDisplay_is_open()
{
    return gtDisplay::is_open();
}


/*
   START-LOG-------------------------------------------

   $Log: gtDisplayXm.h.C  $
   Revision 1.3 1999/06/25 11:01:31EDT rpasseri 
   adding functions to determine height and width of display in pixels
// Revision 1.3  1993/06/02  20:28:52  glenn
// Remove Initialize macros.  Remove genError.h.
//
// Revision 1.2  1993/04/22  22:32:11  builder
// removed Initialize: used in msg
//
// Revision 1.1  92/10/15  18:51:53  glenn
// Initial revision
// 

   END-LOG---------------------------------------------
*/

