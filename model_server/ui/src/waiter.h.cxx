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
// waiter.h.C
//------------------------------------------
// synopsis:
// Implement functions of class waiter and commandContext.
//------------------------------------------

// INCLUDE FILES

#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

#include <genTry.h>
#include <genArr.h>
#include <top_widgets.h>
#include <ui.h>
#include <waiter.h>
#include <waiter_types.h>

#include <machdep.h>
#include <messagePopup.h>

// Data declarations

int  (*waiter::cancel_callback)(void *, int);
void *waiter::cancel_callback_data;

// CLASS DECLARATIONS & TYPEDEFS

typedef commandContext* commandContextPtr;
genArr(commandContextPtr);

class commandContextStack : private commandContextPtrArr
{
  public:
    void push(commandContextPtr);
    void remove(commandContextPtr);
    int empty();
    commandContextPtr& top();
};


// VARIABLE DEFINITIONS

static commandContextStack cc_stack;
static const char class_name_fmt[] = "AlphaSet.Feedback.Command.Duration.%s";
static const char resource_name_fmt[] = "aset.feedback.%s.%s.%s";

static const char* resource_names[waiter::NUM_FEEDBACK_VALUES] =
{
    NULL,
    "staticCursor",
    "animatedCursor",
    "dialog",
    "confirm",
    "noFeedback"
};


//------------------------------------------
// commandContextStack
//------------------------------------------

void commandContextStack::push(commandContextPtr cc)
{
    append(&cc);
}

void commandContextStack::remove(commandContextPtr cc)
{
    for(int i = size() - 1; i >= 0; --i)
    {
	if(*(*this)[i] == cc)
	{
	    commandContextPtrArr::remove(i);
	    break;
	}
    }
}

int commandContextStack::empty()
{
    return size() == 0;
}

commandContextPtr& commandContextStack::top()
{
    return *(*this)[size() - 1];
}

//------------------------------------------
// commandContext
//------------------------------------------

static int boolean_string(const char* string)
{
    Initialize(boolean_string);
    if(string && strcmp(string, "true") == 0)
	ReturnValue(1);
    else
	ReturnValue(0);
}

static const char* className(const char* leaf_name)
{
    Initialize(className);
    static char class_name[512];
    OSapi_sprintf(class_name, class_name_fmt, leaf_name);
    ReturnValue(class_name);
}

static const char* resourceName(
    const char* command_name, const char* duration, const char* leaf_name)
{
    Initialize(resourceName);
    static char resource_name[512];
    OSapi_sprintf(resource_name, resource_name_fmt,
	    command_name, duration, leaf_name);
    ReturnValue(resource_name);
}

static const char* find_duration(const char* command_name, int estim_time)
{
    Initialize(find_duration);

    if(estim_time < 0)
    {
	switch(estim_time)
	{
	  case commandContext::ESTIM_SHORT:
	    ReturnValue("short");

	  case commandContext::ESTIM_MEDIUM:
	    ReturnValue("medium");

	  case commandContext::ESTIM_LONG:
	    ReturnValue("long");

	  case commandContext::ESTIM_VERYLONG:
	  default:
	    ;   // (fall through to ReturnValue below)
	}
	ReturnValue("verylong");
    }
    else
    {
	// Class name:    "AlphaSet.Duration.Command.Wait.MinTime"
	// Resource name: "aset.duration.command.wait00.minTime"

	const char* class_name = className("MinTime");
	static const char duration_fmt[] = "wait%d";
	static char duration[16];

	int w;
	for(w = 0; w < 100; ++w)
	{
	    OSapi_sprintf(duration, duration_fmt, w);
	    const char* resource_name =
		resourceName(command_name, duration, "minTime");

	    uiValue value;
	    const char* type = ui_get_resource(resource_name, class_name, &value);
	    if(!type)
		break;

	    int min_time;
	    if(strcmp(type, XtRInt) == 0)
		min_time = *(int*)value.addr;
	    else if(strcmp(type, XtRImmediate) == 0)
		min_time = (int)value.addr;
	    else if(strcmp(type, XtRString) == 0)
		min_time = atoi((char*)value.addr);
	    else // Unsupported resource type.
		continue;

	    if(min_time > estim_time) // gotcha!
		break;
	}
	OSapi_sprintf(duration, duration_fmt, (w > 0 ? w - 1 : 0));
	ReturnValue(duration);
    }
}

static void load_feedback_values(
    const char* command_name, int estim_time, const char** values)
{
    Initialize(load_feedback_values);

    char class_name[500];
    OSapi_sprintf(class_name, class_name_fmt, "Feedback");

    const char* duration = find_duration(command_name, estim_time);

    OS_dependent::bzero(values, waiter::NUM_FEEDBACK_VALUES * sizeof(char*));
    values[waiter::COMMAND_NAME] = command_name;
    for(int f = waiter::COMMAND_NAME + 1; f < waiter::NUM_FEEDBACK_VALUES; ++f)
    {
	const char* resource_name =
	    resourceName(command_name, duration, resource_names[f]);

	uiValue value;
	const char* type = ui_get_resource(resource_name, class_name, &value);
	if(type && strcmp(type, XtRString) == 0)
	    values[f] = (const char*)value.addr;
    }
    Return
}

static int no_feedback()
{
    Initialize(no_feedback);
    static int no_feedback_ = -1;
    if(no_feedback_ < 0)
    {
	uiValue value;
	const char* type = ui_get_resource(
	    "aset.feedback.noFeedback", "AlphaSet.Feedback.NoFeedback", &value);
	if(type && strcmp(type, XtRString) == 0)
	    no_feedback_ = boolean_string((const char*)value.addr);
	else
	    no_feedback_ = 0;
    }
    ReturnValue(no_feedback_ > 0);
}

static waiter* summon_waiter(
    Widget parent_widget, const char* command_name, int estim_time)
{
    Initialize(summon_waiter);

    if(no_feedback())
	ReturnValue(new dumbWaiter);

    const char* values[waiter::NUM_FEEDBACK_VALUES];
    load_feedback_values(command_name, estim_time, values);

    waiter* feedback = NULL;

    if(boolean_string(values[waiter::NO_FEEDBACK]))
	feedback = new dumbWaiter;

    else if(values[waiter::DIALOG_FEEDBACK])
	feedback = new windowWaiter(values, parent_widget, estim_time);

    else if(values[waiter::ANIM_CURSOR_FEEDBACK])
	feedback = new animatedCursor(values);

    else if(values[waiter::STATIC_CURSOR_FEEDBACK])
	feedback = new staticCursor(values);

    else
	feedback = new dumbWaiter;

    ReturnValue(feedback);
}

static waiter* summon_waiter_gt(
    gtBase *gtb, const char* command_name, int estim_time)
{
    Initialize(summon_waiter);

    if(no_feedback())
	ReturnValue(new dumbWaiter);

    const char* values[waiter::NUM_FEEDBACK_VALUES];
    load_feedback_values(command_name, estim_time, values);

    waiter* feedback = NULL;

    if(boolean_string(values[waiter::NO_FEEDBACK]))
	feedback = new dumbWaiter;

    else if(values[waiter::DIALOG_FEEDBACK])
	feedback = new windowWaiter(values, gtb, estim_time);

    else if(values[waiter::ANIM_CURSOR_FEEDBACK])
	feedback = new animatedCursor(values);

    else if(values[waiter::STATIC_CURSOR_FEEDBACK])
	feedback = new staticCursor(values);

    else
	feedback = new dumbWaiter;

    ReturnValue(feedback);
}

commandContext::commandContext(Widget w, const char* n, int t)
: _feedback(summon_waiter(w, n, t)), name(n), logger(NULL)
{
    cc_stack.push(this);
}

commandContext::commandContext(gtBase *gtb, const char *n, int t)
: _feedback(summon_waiter_gt(gtb, n, t)), name(n), logger(NULL)
{
    cc_stack.push(this);
}

commandContext::~commandContext()
{
    cc_stack.remove(this);
    delete _feedback;
}

void commandContext::start()
{
    Initialize(commandContext::start);

    genError* err = NULL;
    genTry
    {
	switch(feedback()->request())
	{
	  case EXEC_IMMEDIATE:
	    feedback()->start();
	    execute();
	    feedback()->percent_completed(100);
	    break;

	  case EXEC_POSTPONE:
	    break;

	  case EXEC_ABORT:
	    break;
	}
    }
    genCatch(err)
    {
	reset_cursor();
    }
    genEndtry;

    Return
}


void commandContext::status(const char* str)	// class static
{
    if(!cc_stack.empty())
    {
	commandContextPtr cc = cc_stack.top();
	cc->feedback()->status(str);
    }
}


void commandContext::error(const char* str)	// class static
{
    const char* title = /*T*/"Error";	// default window title
    if(!cc_stack.empty())
    {
	commandContextPtr cc = cc_stack.top();
	title = cc->name;
    }
    popup_Error(title, str);
}


void commandContext::warning(const char* str)	// class static
{
    const char* title = /*T*/"Warning";	// default window title
    if(!cc_stack.empty())
    {
	commandContextPtr cc = cc_stack.top();
	title = cc->name;
    }
    popup_Warning(title, str);
}


void commandContext::information(const char*)	// class static
{
}


void commandContext::working(const char* str)	// class static
{
    const char* title = /*T*/"Working";	// default window title
    if(!cc_stack.empty())
    {
	commandContextPtr cc = cc_stack.top();
	title = cc->name;
    }
    popup_Working(title, str);
}


void commandContext::log(const char*)		// class static
{
}

//------------------------------------------
// waiter
//------------------------------------------

waiter::waiter()
: confirm(0)
{
    cancel_callback = NULL;
    cancel_callback_data = NULL;
}

waiter::waiter(const char** values)
: confirm(boolean_string(values[CONFIRM_FEEDBACK]))
{
    cancel_callback = NULL;
    cancel_callback_data = NULL;
}

waiter::~waiter()
{}

void waiter::time_remaining(int)
{
    Initialize(waiter::time_remaining);
    Return
}

void waiter::percent_completed(int)
{
    Initialize(waiter::percent_completed);
    Return
}

void waiter::title(const char*)
{
    Initialize(waiter::title);
    Return
}

void waiter::label(const char*)
{
    Initialize(waiter::label);
    Return
}

void waiter::status(const char*)
{
    Initialize(waiter::status);
    Return
}

commandContext::execRequest waiter::request()
{
    Initialize(waiter::request);

    if(confirm)
    {
    }
    ReturnValue(commandContext::EXEC_IMMEDIATE);
}

/*
   START-LOG-------------------------------------------

   $Log: waiter.h.C  $
   Revision 1.6 2000/07/10 23:11:51EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.9  1994/02/16  01:59:56  kws
Port

Revision 1.2.1.8  1993/11/18  22:42:37  kws
Bug track: 4952
Fix message logger and make it the default error system

Revision 1.2.1.7  1993/07/20  18:57:05  harry
HP/Solaris Port

Revision 1.2.1.6  1993/04/23  17:29:39  davea
bug 3482 - moved default case outside of switch, to keep buildxref happy

Revision 1.2.1.5  1993/02/22  22:23:40  oak
Fixed problem with cancel button.

Revision 1.2.1.4  1993/01/28  14:57:43  glenn
Comment out commandContext::information.
Capitalize "Working" in commandContext::working.
Comment out commandContext::log to avoid duplicating log messages.
Fix bug 2320.

Revision 1.2.1.3  1993/01/26  19:51:55  sharris
add support for working, warning and informational popups

Revision 1.2.1.2  1992/10/09  20:14:02  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
