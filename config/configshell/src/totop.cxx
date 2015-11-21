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
// totop.C
//------------------------------------------
// synopsis:
// 
// int totop(unsigned long windowid)
//------------------------------------------

// INCLUDE FILES

#include <stdio.h>
#include <X11/Xlib.h>


// VARIABLE DEFINITIONS

static Display* display;


// FUNCTION DEFINITIONS

int bring_to_top(unsigned long windowid)
//
// De-iconify and raise a window given its X window-id.
//
{
    // Initialize display just once.

    if(!(display  ||  (display = XOpenDisplay(NULL))))
    {
	fprintf(stderr, "Unable to open display \"%s\"\n", XDisplayName(NULL));

	return -1;		// Failure
    }

    // De-iconify the window and raise it to the top.

    XMapRaised(display, windowid);

    // Flush any buffered commands to X server.

    XFlush(display);

    return 0;			// Success
}

int iconify(unsigned long windowid)
//
// iconify a window given its X window-id.
//
{
    if(display) {
        XIconifyWindow(display, windowid, NULL);
	XFlush(display);
    } 
    return 0;
}
 

/*
   START-LOG-------------------------------------------

   $Log: totop.C  $
   Revision 1.1 1994/02/28 17:33:47EST builder 
   made from unix file
// Revision 1.3  1993/09/06  22:46:02  aharlap
// added function iconify()
//
// Revision 1.2  1993/04/16  19:37:30  glenn
// Just call XMapRaised, per X11R5 ICCCM.
//
// Revision 1.1  1993/04/15  21:52:57  davea
// Initial revision
//
   END-LOG---------------------------------------------
*/
