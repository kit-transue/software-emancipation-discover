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
#ifndef _gtSepar_h 
#define _gtSepar_h

// gtSepar.h
//------------------------------------------
// synopsis:
// class description of generic Separator (gtSeparator)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtSeparator : public gtPrimitive
{
  public:
    // Creates a gtSeparator appropriate to the toolkit, in the direction      
    // specified.
    // Pre: Must have a valid gt object parent passed to it, and the dir
    // argument must be a member of gtDirection.
    // Post: The returned pointer will point to a valid gtSeparator, which
    // is drawn in the directon specified by the dir argument.
    static gtSeparator* create(gtBase *parent, const char *name, gtDirection dir);

    ~gtSeparator();

    // Changes the style of the gtSeparator to one specified by the 
    // argument.
    // Pre: The gtSeparator must have already been created for the appropriate
    // toolkit.  The sty argument must be a member of gtSeparatorStyle.
    // Post: The style of the gtSeparator will be that specified by the
    // arguments.
    virtual void set_style(gtSeparatorStyle sty) = 0;

  protected:
    gtSeparator();
};

#endif // _gtSepar_h

/*
   START-LOG-------------------------------------------

// $Log: gtSepar.h  $
// Revision 1.1 1993/07/28 19:47:51EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:38  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:54  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:53  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:59  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:25  smit
 * Initial revision
 * 
// Revision 1.4  92/09/11  11:12:34  builder
// Make destructor public.
// 
// Revision 1.3  92/09/11  04:07:48  builder
// Add ctor and dtor.
// 
// Revision 1.2  92/07/30  23:45:03  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:19  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
