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
#ifndef _gtLabelXm_h
#define _gtLabelXm_h

// gtLabelXm.h
//------------------------------------------
// synopsis:
// Class description for the gtLabel object in OSF Motif
//------------------------------------------

#include <gtLabel.h>


class gtLabelXm : public gtLabel
{
  public:
    gtLabelXm(gtBase* parent, const char* name, const char* contents);

    virtual gtBase* container();

    virtual void alignment(gtAlign a);
    virtual void text(const char*);
};


/*
   START-LOG-------------------------------------------

// $Log: gtLabelXm.h  $
// Revision 1.1 1993/02/23 23:45:19EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.4  92/07/30  23:55:51  rfermier
// GT method name changes
// 
// Revision 1.3  92/07/24  11:11:23  glenn
// Remove construct member function.
// Use const char* when appropriate.
// Remove unused member variables.
// 
// Revision 1.2  92/07/09  13:26:43  rfermier
// Allow changing of contents
// 
// Revision 1.1  92/07/08  20:40:59  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtLabelXm_h
