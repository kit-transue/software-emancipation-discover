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
#ifndef _gtFrame_h
#define _gtFrame_h

// gtFrame.h
//------------------------------------------
// synopsis:
// generic Frame manager (gtFrame), used for drawing boxes around
// things.
//
//------------------------------------------
#ifndef _gtManager_h
#include <gtManager.h>
#endif

class gtFrame : public gtManager
{
  public:
    // Creates a gtFrame object for making boxes around other objects.
    static gtFrame* create(gtBase *parent, const char *name);

    static gtFrame* create(gtBase *parent, const char *name,
			   gtFrameStyle style, int thickness, int border);
    ~gtFrame();

    // Change the style of the gtFrame.
    virtual void set_style (gtFrameStyle sty) = 0;

    // Highlights the frame if val is true, otherwise removes the highlight.
    virtual void highlight(int val) = 0;

    // Sets the thickness of the shadow.
    virtual void thickness(int val) = 0;

  protected:
    gtFrame();
};

#endif

/*
   START-LOG--------------------------------------

   $Log: gtFrame.h  $
   Revision 1.1 1993/07/29 10:36:11EDT builder 
   made from unix file
 * Revision 1.2  1993/01/22  17:17:22  glenn
 * Add create function that takes style, thickness, and border.
 *
 * Revision 1.1  1992/10/10  21:32:19  builder
 * Initial revision
 *

   END-LOG----------------------------------------

*/
