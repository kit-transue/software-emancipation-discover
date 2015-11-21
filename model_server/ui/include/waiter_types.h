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
#ifndef _waiter_types_h
#define _waiter_types_h

// waiter_types.h
//------------------------------------------
// synopsis:
// Declaration of classes derived from waiter.
//
//------------------------------------------
// Restrictions:
// This is an implementation file that is not needed by most users of
// the waiter facilities.  Use "waiter.h" instead, which declares the
// interface to the appropriate base classes.
//------------------------------------------

#ifndef _x11_intrinsic_h
#include <x11_intrinsic.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif
#ifndef _waiter_h
#include <waiter.h>
#endif
#ifndef _Progress_h
#include <Progress.h>
#endif

//------------------------------------------
// dumbWaiter
//
// Quiet waiter for when no feedback is desired.
//------------------------------------------

class dumbWaiter : public waiter
{
  public:
    dumbWaiter();
    ~dumbWaiter();

    virtual void start();
};


//------------------------------------------
// animatedCursor
//
// Exciting cursor feedback.
//------------------------------------------

class animatedCursor : public waiter
{
  public:
    animatedCursor(const char** values);
    ~animatedCursor();

    virtual void start();
    virtual void time_remaining(int);
    virtual void percent_completed(int);

  protected:
    animatedCursor(const char** values, const char* name);

    genString cursor_name;
    bool started : 1;
};


//------------------------------------------
// staticCursor
//
// Boring cursor feedback.
//------------------------------------------

class staticCursor : public animatedCursor
{
  public:
    staticCursor(const char** values);
    ~staticCursor();

    virtual void time_remaining(int);
    virtual void percent_completed(int);
};


//------------------------------------------
// windowWaiter
//
// Exciting cursor feedback.
//------------------------------------------

class windowWaiter : public animatedCursor
{
  public:
    windowWaiter(const char**, Widget, int estim_time);
    windowWaiter(const char**, gtBase *, int estim_time);
    ~windowWaiter();

    virtual void start();
    virtual void title(const char*);
    virtual void label(const char*);
    virtual void status(const char*);
    virtual void time_remaining(int);
    virtual void percent_completed(int);

  protected:
    genString dialogTitle;
    genString labelString;
    gtBase* parent_gt;
    int total_time;
    Progress* prog;
};


/*
    START-LOG-------------------------------

    $Log: waiter_types.h  $
    Revision 1.1 1993/05/27 16:23:35EDT builder 
    made from unix file
 * Revision 1.2.1.4  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.3  1992/10/23  17:49:28  glenn
 * Remove parent_widget.
 *
 * Revision 1.2.1.2  92/10/09  19:57:32  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif // _waiter_types_h
