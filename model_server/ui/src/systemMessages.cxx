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
// systemMessages.C
//------------------------------------------
// synopsis:
// Interface to UI/message system.
//
//------------------------------------------

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <malloc.h>

#include <gtDisplay.h>
#include <gtPushButton.h>
#include <genError.h>
#include <waiter.h>
#include <ste_send_message.h>
#include <autolog.h>
#include <Logger.h>
#include <Question.h>
#include <customize.h>
#include <systemMessages.h>
#include <driver_mode.h>

#ifdef NEW_UI

#include <vport.h>
#include vstrHEADER
#include <../../../clients/communications/include/Application.h>
#include <../../../clients/DIS_ui/interface.h>

#undef printf
#undef sprintf
#undef vsprintf

extern Application* DISui_app;

#endif

// VARIABLE DEFINITIONS

static systemMessageOptions message_options =
{
    0, // logFile

    1, // error
    1, // warning
    1, // informative
    1, // diagnostic

    1, // import
    1, // compile
    1  // propagate
};

// FUNCTION DEFINITIONS

int system_message_logger_init()
{
    // Set the local value from the preference value.

    // Whether or not to use the message logger.
    message_options.logFile = customize::getIntPref("DIS_msg.UseMsgLogger");

    // What kinds of messages to log.
    message_options.error = customize::getIntPref("DIS_msg.LogErrorMessages");
    message_options.warning = customize::getIntPref("DIS_msg.LogWarningMessages");
    message_options.informative = customize::getIntPref(
                                      "DIS_msg.LogInformativeMessages");
    message_options.diagnostic = customize::getIntPref(
                                      "DIS_msg.LogDiagnosticMessages");

    // When to log the messages.
    message_options.import = customize::getIntPref("DIS_msg.LogDuringImport");
    message_options.compile = customize::getIntPref("DIS_msg.LogDuringCompile");
    message_options.propagate = customize::getIntPref("DIS_msg.LogDuringPropagate");

    return(message_options.logFile);
}

system_message_logger(int val)
{
    // Force value to be boolean.
    bool value = val? true:false;

    // Set the preference value and the local value.
    customize::putPref("DIS_msg.UseMsgLogger", value);
    message_options.logFile = value;

    // Return the value.
    return(value);
}

void logger_destroyed(gtBase*, void* data)
{
   Logger* logger = (Logger*)data;

   if (logger) {
	if (Logger::logger_instance == logger) {
	    Logger::logger_instance = NULL;
	}
	logger->top_level = NULL;	// already destroyed
	delete logger;
   }

    SystemMessages_update(&message_options);
}


static Logger* logger()
{
    if(Logger::logger_instance == NULL)
	new Logger(NULL, TXT("Message Log"), logger_destroyed);
    return Logger::logger_instance;
}


static void vmessage(systemMessageType type, const char* fmt, va_list ap)
{
    if (!fmt || !fmt[0])
    {
        return;
    }

    genString vmessage_buffer;
    vmessage_buffer.vsprintf(gettext(fmt), ap);

#ifdef NEW_UI
	vstr* str = vstrClone((vchar*)(char*)vmessage_buffer);
        rcall_gc_display_message (Application::findApplication("DISui"), str);
	vstrDestroy (str);
        return;
#endif

    bool displayOpen = true;
#ifndef NEW_UI
    displayOpen = gtDisplay::is_open();
#endif

    if ( (vmessage_buffer.length() ) && !is_gui() )
    {
	const char* msg_type;

        switch (type) {
	case MSG_WARN:    msg_type = "Warning: ";	break;
	case MSG_STATUS:  msg_type = "Status: ";	break;
	case MSG_WORKING: msg_type = "Working: ";	break;
	case MSG_DIAG:    msg_type = "Diagnostic: ";	break;
	case MSG_INFORM:  msg_type = "Status: ";	break;
	case MSG_ERROR:   msg_type = "Error: ";	break;
	  default:        msg_type = "Unknown error: ";	break;
        }

	msg("$1") << msg_type << vmessage_buffer.str() << eom;

	if(type == MSG_DIAG || type == MSG_ERROR || type == MSG_INFORM ||
           type == MSG_WARN || type == MSG_WORKING) 
        {
          char *tmpstring = (char *)(vmessage_buffer.str());
          LOG_MESSAGE(tmpstring);
        }
    }
    else if(vmessage_buffer.length())
    {
        if (message_options.logFile) {
	    if(type == MSG_STATUS)
	    {
		commandContext::status(vmessage_buffer.str());
		ste_print_message(vmessage_buffer);

		return;
	    } 

	    commandContext::log(vmessage_buffer.str());
#ifndef NEW_UI
	    genString context_name = gtPushButton::next_help_context();
	    gtPushButton::next_help_context(NULL);
	    logger()->append (vmessage_buffer.str(), context_name, type);
#endif
	    commandContext::log(vmessage_buffer.str());

	    if(type == MSG_DIAG || type == MSG_ERROR || type == MSG_INFORM ||
	       type == MSG_WARN || type == MSG_WORKING) 
		LOG_MESSAGE((char *)vmessage_buffer.str());
        } else {
	    if(type == MSG_STATUS)
	    {
		commandContext::status(vmessage_buffer.str());
		ste_print_message(vmessage_buffer);
	    }
	    else if(type == MSG_ERROR)
	    {
		commandContext::error(vmessage_buffer.str());
	    }
	    else if(type == MSG_INFORM)
	    {
		commandContext::information(vmessage_buffer.str());
	    }
	    else if(type == MSG_WARN)
	    {
		commandContext::warning(vmessage_buffer.str());
	    }
	    else if(type == MSG_WORKING)
	    {
		commandContext::working(vmessage_buffer.str());
	    }
	    if(type == MSG_DIAG || type == MSG_ERROR || type == MSG_INFORM ||
	       type == MSG_WARN || type == MSG_WORKING) 
		LOG_MESSAGE((char *)vmessage_buffer.str());
	}
    }

    return;
}

static boolean message_type_enabled(systemMessageType type)
{
    switch(type)
    {
      case MSG_DIAG:
	return message_options.diagnostic;

      case MSG_INFORM:
	return message_options.informative;

      case MSG_WARN:
	return message_options.warning;

      case MSG_ERROR:
	return message_options.error;

      default:
	;
    }
    return true;
}

#define VMESSAGE(type,fmt)		\
if(message_type_enabled(type))		\
{					\
    va_list ap;				\
    va_start(ap, fmt);			\
    vmessage(type, fmt, ap);		\
    va_end(ap);				\
}

extern "C" void msg_type(systemMessageType type, const char* fmt...)
{
    VMESSAGE(type,fmt)
}

extern "C" void msg_diag(const char* fmt...)
{
    VMESSAGE(MSG_DIAG,fmt)
}

extern "C" void msg_inform(const char* fmt...)
{
    VMESSAGE(MSG_INFORM,fmt)
}

extern "C" void msg_warn(const char* fmt...)
{
    VMESSAGE(MSG_WARN,fmt)
}

extern "C" void msg_error(const char* fmt...)
{
    VMESSAGE(MSG_ERROR,fmt)
}

extern "C" void msg_status(const char* fmt...)
{
    VMESSAGE(MSG_STATUS,fmt)
}

systemMessageOptions& msg_options()
{
    return message_options;
}

void msg_options(systemMessageOptions& new_options, boolean notify)
{
    if(&message_options != &new_options)
    {
	
	if(message_options.logFile != new_options.logFile)
	{
	    if(new_options.logFile)
		logger();
	}
	message_options = new_options;
    }
    if(notify)
	SystemMessages_update(&message_options);
}


// msg_question - Ask a simple OK/Cancel question.
// Returns -1 on internal error, 0 for Cancel, 1 for OK.

int msg_question(const char* fmt...)
{
    Initialize(msg_question);

    va_list ap;
    va_start(ap, fmt);

    genString vmessage_buffer;
    vmessage_buffer.vsprintf(gettext(fmt), ap);

    int response = popup_Question(/*T*/"ParaSET", vmessage_buffer.str(),-1);

    va_end(ap);

    ReturnValue(response > 0);
}

