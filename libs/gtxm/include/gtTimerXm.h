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
#ifndef _gtTimerXm_h
#define _gtTimerXm_h

// gtTimerXm.h
//------------------------------------------
// synopsis:
// Timeouts
//
//------------------------------------------

#include "x11_intrinsic.h"

#ifndef _gtTimer_h
#include <gtTimer.h>
#endif

class gtTimerXm : public gtTimer
{
  public:
    gtTimerXm(unsigned int delay, gtTimerCB callback, void* callback_data);
    ~gtTimerXm();

    virtual void start();
    virtual void stop();
    virtual void delay(unsigned int);
    virtual int  active();

  private:
    static void timeout_callback(XtPointer, XtIntervalId*);

    XtIntervalId timeout;
};

#endif // _gtTimerXm_h

/*
   START-LOG-------------------------------------------

   $Log: gtTimerXm.h  $
   Revision 1.1 1993/07/28 19:49:19EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  18:44:30  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
