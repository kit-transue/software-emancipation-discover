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
// gtTimerXm.h.C
//------------------------------------------
// synopsis:
// Interface to Xt timeout system
//
//------------------------------------------

// include files

#include <gtTimerXm.h>
#include <gtAppShellXm.h>

// function definitions

gtTimer* gtTimer::create(unsigned int delay, gtTimerCB cb, void* data)
{
    return new gtTimerXm(delay, cb, data);
}

gtTimer::gtTimer(unsigned int d, gtTimerCB cb, void* data)
: _delay(d), callback(cb), callback_data(data)
{}

gtTimer::~gtTimer()
{
#if 0
    // Here we're calling a pure virtual function in the destructor for
    // abstract base class gtTimer! Executing this code would lead to
    // unexpected behavior.

    stop();
#endif
}

gtTimerXm::gtTimerXm(unsigned int t, gtTimerCB cb, void* data)
: gtTimer(t, cb, data), timeout(NULL)
{}

gtTimerXm::~gtTimerXm()
{}

void gtTimerXm::timeout_callback(XtPointer timer_data, XtIntervalId*)
{
    gtTimerXm* timer = (gtTimerXm*)timer_data;

    timer->timeout = NULL;
    timer->start();

    if(timer->callback)
	(*timer->callback)(timer, timer->callback_data);
}

void gtTimerXm::stop()
{
    if(timeout)
    {
	XtRemoveTimeOut(timeout);
	timeout = NULL;
    }
}

void gtTimerXm::start()
{
    if(!timeout && _delay)
    {
	timeout = XtAppAddTimeOut(
	    gtApplicationShellXm::get_context(),
	    _delay, gtTimerXm::timeout_callback, XtPointer(this));
    }
}

void gtTimerXm::delay(unsigned int d)
{
    _delay = d;
    if(timeout)
    {
	stop();
	start();
    }
}

int gtTimerXm::active()
{
    return timeout != NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: gtTimerXm.h.C  $
   Revision 1.2 1997/03/18 12:08:47EST abover 
   irix6 port:
   We're calling a pure virtual function here in the destructor for base class gtTimer!
   All other compilers let this by, but sgi complains during the link phase. If this piece
   of code were ever executed (by destroying an object whose type is some derived class
   of gtTimer), the resulting behavior would be unpredictable.
Revision 1.2.1.3  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.2  1992/10/09  19:03:56  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
