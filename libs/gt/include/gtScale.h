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
#ifndef _gtScale_h
#define _gtScale_h

// gtScale.h
//------------------------------------------
// synopsis:
// class description of generic Scale (gtScale)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif


class gtScale : public gtPrimitive
{
  public:
    static gtScale* create(
	gtBase *parent, const char *name, const char *contents,
	gtDirection dir, int minval, int maxval);

    static gtScale* create(
	gtBase *parent, const char *name, const char *contents,
	gtDirection dir);

    ~gtScale();

    virtual void add_callback(
	gtReason rea, gtScaleCB callback, void *client_data) = 0;
    virtual int value() = 0;
    virtual void value(int) = 0;
    virtual void decimal_points(int) = 0;
    virtual void show_value(int) = 0;

  protected:
    gtScale();
};


/*
   START-LOG-------------------------------------------

// $Log: gtScale.h  $
// Revision 1.1 1993/07/29 10:36:21EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:10:56  glenn
 * cleanup
 *
// Revision 1.5  92/09/11  11:12:27  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:43  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:44:58  glenn
// GT method name changes
// 
// Revision 1.2  92/07/10  13:22:44  rfermier
// added decimal_points and show_value
// 
// Revision 1.1  92/07/08  20:39:13  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtScale_h
