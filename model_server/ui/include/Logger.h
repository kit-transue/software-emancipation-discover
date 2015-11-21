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
#ifndef _Logger_h
#define _Logger_h

// Logger.h
//------------------------------------------
// synopsis:
// Scrolling output of textual messages
//
//------------------------------------------
#if 0
#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _gtBase_h
#include <gtBase.h>
#endif
#endif
#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif
#if 0
#ifndef _gtTopShell_h
#include <gtTopShell.h>
#endif
#ifndef _gtMainWindow_h
#include <gtMainWindow.h>
#endif
#ifndef _gtMenuBar_h
#include <gtMenuBar.h>
#endif
#endif
#ifndef _systemMessages_h
#include <systemMessages.h>
#endif

typedef struct {
    systemMessageType type;    
    genString text;
    genString help;
    unsigned long start;
    unsigned long end;
} messageRecord ;
typedef messageRecord* messageRecordPtr ;

genArr(messageRecordPtr);

class Logger
{
    friend void logger_start_transaction();
    friend void logger_end_transaction();
    friend void logger_flush_transactions();
    friend void internal_logger_help_disable();
    friend void internal_logger_help_enable();
    //    friend void logger_parent(gtDialogTemplate* parent);
    //    friend void logger_destroyed(gtBase*, void*);
    friend int logger_wm_close(void*);

  public:
    static Logger* logger_instance;

  public:
    void* operator new(size_t);
    void operator delete(void*);
    Logger(gtBase* parent, const char*,
	   void (*destroy_CB)(gtBase*, void*) = NULL);
    ~Logger();

    static void rundown();

    void size(int rows, int columns);
    void add_suspend_button();
    void popup(int modal = 0);
    void append(const char* text, const char* help_context = NULL,
		systemMessageType type = MSG_INFORM);
    void clear();

  private:
    static void suspend_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void clear_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void save_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void config_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    static void help_logger_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void help_selected_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    static void top_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void bottom_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void prev_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void next_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void curr_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    void scroll(int);
    void scroll0();

    gtTextEditor*	text_ed;
    gtTextEditor*	text_ed0;
    gtTopLevelShell*    top_level;
    gtMainWindow*       window;
    gtPushButton*       help_button;
    gtPushButton*       logger_help_button;
    gtLabel*       	error_label;
    unsigned int	save_done_help : 1;
    unsigned int	suspended : 1;
    int                 error_marker;
    messageRecord       last_record;
    static genArrOf(messageRecordPtr) records;
    static int transaction_level;
    static Logger* old_logger_instance;
};


/*
    START-LOG-------------------------------

    $Log: Logger.h  $
    Revision 1.9 1995/12/15 08:13:10EST wmm 
    Bug track: N/A
    Fix bugs 11345, 11352, 11353, 11360.
 * Revision 1.2.1.6  1994/01/10  23:47:40  kws
 * Fix problems with fatal transactional errors
 *
 * Revision 1.2.1.5  1993/11/20  00:08:13  kws
 * Bug track: 5002
 * Fix badly parented dialog box
 * Added -nologger & -stderr
 *
 * Revision 1.2.1.4  1993/11/18  22:44:20  kws
 * Bug track: 4952
 * Fix message logger and make it the default error system
 *
 * Revision 1.2.1.3  1993/01/28  15:04:34  glenn
 * Remove old "C" interface: popup_Logger, Logger_append.
 * Make Logger class more configurable so it can be used for
 * output from other types of commands.
 *
    END-LOG---------------------------------
*/

#endif // _Logger_h
