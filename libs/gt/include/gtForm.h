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
#ifndef _gtForm_h
#define _gtForm_h

// gtForm.h
//------------------------------------------
// synopsis:
// generic Form manager (gtForm)
//
//------------------------------------------

#ifndef _gtManager_h
#include <gtManager.h>
#endif

class gtForm : public gtManager
{
  public:
    static void vertStack(
	gtPrimitive*,     gtPrimitive* = 0, gtPrimitive* = 0,
	gtPrimitive* = 0, gtPrimitive* = 0, gtPrimitive* = 0,
	gtPrimitive* = 0, gtPrimitive* = 0);

    static void horzStack(
	gtPrimitive*,     gtPrimitive* = 0, gtPrimitive* = 0,
	gtPrimitive* = 0, gtPrimitive* = 0, gtPrimitive* = 0,
	gtPrimitive* = 0, gtPrimitive* = 0);

    // Creates a form suitable for specifying arbitrary geometries.
    // gtForms use the attachment functions available to any descendant
    // of the gtPrimitive class to figure out their geometries.
    // Pre: It must have a valid gt object parent passed to it.
    // Post: The returned pointer points to a valid gtForm specific to
    // to the chosen toolkit.
    static gtForm* create(gtBase *parent, const char *name);
    static gtForm* create(void *parent, const char *name);

    ~gtForm();

    virtual void help_callback(gtFormCB callback, void *client_data) = 0;

  protected:
    gtForm();
};

#endif // _gtForm_h

/*
   START-LOG-------------------------------------------

 $Log: gtForm.h  $
 Revision 1.2 1995/03/27 18:02:38EST azaparov 
 Bug track: N/A
 Minibrowser stuff & fixes around
 * Revision 1.2.1.2  1992/10/09  18:10:09  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:31  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:30  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:50  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:19  smit
 * Initial revision
 * 
// Revision 1.7  92/09/24  23:31:00  glenn
// Add vertStack, horzStack.
// 
// Revision 1.6  92/09/11  11:12:01  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  08:17:27  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.4  92/09/11  04:07:20  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:44:39  glenn
// GT method name changes
// 
// Revision 1.2  92/07/13  15:22:35  rfermier
// Added help callback hooks
// 
// Revision 1.1  92/07/08  20:38:56  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
