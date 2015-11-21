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
#ifndef _waiter_h
#define _waiter_h

//------------------------------------------
// waiter.h
//------------------------------------------

#ifndef _x11_intrinsic_h
#include <x11_intrinsic.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _gtBase_h
#include <gtBase.h>
#endif

//------------------------------------------
// commandContext
//
// Abstract base class for command contexts.
//------------------------------------------

class waiter;

class commandContext
{
  public:
    enum estimTime {
	ESTIM_SHORT = -4, ESTIM_MEDIUM, ESTIM_LONG, ESTIM_VERYLONG
    };
    enum execRequest {
	EXEC_IMMEDIATE, EXEC_POSTPONE, EXEC_ABORT
    };

    static void status(const char*);
    static void error(const char*);
    static void information(const char*);
    static void warning(const char*);
    static void working(const char*);
    static void log(const char*);

    commandContext(Widget parent, const char* name, int estim_time);
    commandContext(gtBase *parent, const char* name, int estim_time);
    virtual ~commandContext();

    void    start();

  protected:
    virtual void execute() = 0;
    waiter* feedback() { return _feedback; }

  private:
    waiter* _feedback;
    genString name;
    Widget logger;
};


//------------------------------------------
// waiter
//
// Abstract base class for waiter feedback.
//------------------------------------------

class waiter
{
  public:
    static int   (*cancel_callback)(void *, int);
    static void  *cancel_callback_data;
    enum waiterResources
    {
	COMMAND_NAME,
	STATIC_CURSOR_FEEDBACK,
	ANIM_CURSOR_FEEDBACK,
	DIALOG_FEEDBACK,
	CONFIRM_FEEDBACK,
	NO_FEEDBACK,

	NUM_FEEDBACK_VALUES
    };

    virtual ~waiter();

    virtual void start() = 0;
    virtual void title(const char*);
    virtual void label(const char*);
    virtual void status(const char*);
    virtual void time_remaining(int seconds);
    virtual void percent_completed(int percent);
    virtual void set_cancel_callback(int (*arg_callback)(void *, int), 
				 void *arg_callback_data) { 
	cancel_callback = arg_callback;
	cancel_callback_data = arg_callback_data;
    };

    commandContext::execRequest request();

  protected:
    waiter();
    waiter(const char**);

  private:
    const int confirm;
};

/*
    START-LOG-------------------------------

    $Log: waiter.h  $
    Revision 1.2 1995/07/27 20:38:20EDT rajan 
    Port
 * Revision 1.2.1.6  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.5  1993/02/22  22:22:20  oak
 * Fixed cancel callback problem.
 *
 * Revision 1.2.1.4  1993/01/26  19:49:49  sharris
 * add support for working, information, and warning dialogs
 *
 * Revision 1.2.1.3  1992/10/23  17:49:07  glenn
 * Minor cleanup.
 *
 * Revision 1.2.1.2  92/10/09  19:57:31  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/


#endif // _waiter_h
