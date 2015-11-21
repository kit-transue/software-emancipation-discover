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
#ifndef _gtScaleXm_h
#define _gtScaleXm_h

// gtScaleXm.h
//------------------------------------------
// synopsis:
// Class description for the gtScale object in OSF Motif
//------------------------------------------

#include <gtScale.h>
#include <gtCallbackMgrXm.h>


class gtScaleXm : public gtScale, public gtCallbackMgrXm
{
  public:
    gtScaleXm(gtBase *parent, const char *name, const char *contents,
	      gtDirection dir, int minval, int maxval);
    gtScaleXm(gtBase *parent, const char *name, const char *contents,
	      gtDirection dir);
    ~gtScaleXm();

    void add_callback(gtReason rea, gtScaleCB callback, void *client_data);
    int  value();
    void value(int);
    void decimal_points(int);
    void show_value(int);

    gtBase *container();
};


/*
   START-LOG-------------------------------------------

// $Log: gtScaleXm.h  $
// Revision 1.1 1993/07/29 10:37:54EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.3  92/07/30  23:55:59  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/10  13:22:31  rfermier
// added decimal_points and show_value
// 
// Revision 1.1  92/07/08  20:41:10  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtScaleXm_h
