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
#ifndef _gtLabel_h
#define _gtLabel_h

// gtLabel.h
//------------------------------------------
// synopsis:
// class description of generic Label (gtLabel)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtLabel : public gtPrimitive
{
  public:
    // Creates a gtLabel appropriate for the toolkit.
    // Pre: A valid gt object parent must be passed to it
    // Post: The returned pointer points to a valid gtForm 
    // specific to the toolkit.
    static gtLabel* create(
	gtBase *parent, const char *name, const char *contents);

    ~gtLabel();

    // Change the alignment of the contents of the label.  The argument must be
    // a selection from the choices available in the gtAlign enum.
    virtual void alignment(gtAlign a) = 0;

    // Set the text in the contents of the label.
    virtual void text(const char *) = 0;

  protected:
    gtLabel();
};

#endif // _gtLabel_h

/*
   START-LOG-------------------------------------------

 $Log: gtLabel.h  $
 Revision 1.1 1993/07/28 19:47:42EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:12  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:41  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:40  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:50  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:20  smit
 * Initial revision
 * 
// Revision 1.7  92/09/11  11:12:06  builder
// Make destructor public.
// 
// Revision 1.6  92/09/11  08:17:48  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.5  92/09/11  04:07:24  builder
// Add ctor and dtor.
// 
// Revision 1.4  92/07/30  23:44:42  glenn
// GT method name changes
// 
// Revision 1.3  92/07/24  11:06:21  glenn
// Use const char* when appropriate.
// 
// Revision 1.2  92/07/09  13:26:56  rfermier
// Allow changing of contents; more documentation
// 
// Revision 1.1  92/07/08  20:39:00  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
