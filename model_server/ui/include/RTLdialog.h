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
#ifndef _RTLdialog_h
#define _RTLdialog_h

// RTLdialog.h
//------------------------------------------
// synopsis:
// Simple dialog window containing a run-time list and push-buttons.
//------------------------------------------
// There are two ways to use this class:
//
// 1. Use the RTLdialog::run function (always modal).
//
//    RTLPtr rtl_head;
//    RTLdialog choose("widget_name", TXT("Window Title"));
//    choose.init(TXT("List Label"), rtl_head);
//
//    int response = 0;
//    while(response == 0) {      // While the last response was "Apply".
//        symbolArr selection;
//        if((response = choose.run(&selection)) >= 0) {
//            // Operate on contents of selection.
//        }
//    }
//
//
// 2. Use a callback function and run modally or non-modally (default).
//
//    class A {
//        static void callback(int, symbolArr*, void*);
//        void foo();
//    };
//
//    void A::callback(int response, symbolArr* selection, void* data
//                     RTLdialog* rtl_dialog) {
//        A* a = (A*)data;
//        if(response >= 0) {             // "OK" or "Apply" pressed.
//            // Operate on contents of selection.
//        }
//        if(response != 0) {             // "OK" or "Cancel" pressed.
//            delete rtl_dialog;
//        }
//    }
//
//    void A::foo() {
//        RTLPtr rtl_head;
//        RTLdialog* choose =
//            new RTLdialog("widget_name", TXT("Window Title")
//                          callback, this, modal_flag);
//        choose->init(TXT("List Label"), rtl_head);
//        choose->popup();
//    }
//
// NOTE: Some useful variations on the interface are possible by
// calling RTLdialog::list_style and/or RTLdialog::button before
// calling RTLdialog::popup (see comments below).
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

class RTL;
class symbolArr;
class runContext;

class RTLdialog
{
  public:
    RTLdialog(const char* name, const char* title,
	      void (*callback)(int, symbolArr*, void*, RTLdialog*) = 0,
	      void* callback_data = 0, int modal = 0);
    ~RTLdialog();

    void init (const char* label, RTL*, int sort_order = 0);
    void init (const char*, symbolArr&, int = 0); 

    // Change the selection style for the list in the dialog.
    // Use "gtExtended" for multiple selection.
    void list_style(gtListStyle);

    // Enable, disable, or hide buttons.
    // index: 0=OK 1=Apply 2=Cancel
    void button(int index, int visible = 1, int enabled = 1,
		const char* label = NULL);
    void popup();
    void popdown();
    void override_WM_destroy(int (*callback)(void*));
    int  run(symbolArr*);

    gtDialogTemplate* get_shell (void) const	{ return shell; };
    gtRTL* gtrtl (void) const			{ return gt_rtl; }
    gtLabel* gtlabel (void) const		{ return gt_label; }
    void* get_callback_data (void) const	{ return callback_data; }

    static void action_CB(gtList*, gtEventPtr, void*, gtReason); 

  private:
    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void apply_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
//  static void action_CB(gtList*, gtEventPtr, void*, gtReason);  // made public

    gtDialogTemplate*	shell;
    gtLabel*		gt_label;
    gtRTL*		gt_rtl;

    void		(*callback)(int, symbolArr*, void*, RTLdialog*);
    void*		callback_data;

    runContext*		run_context;
    short		modality;
    short		is_up;

    void push_button(int);
};


/*
    START-LOG-------------------------------

 * $Log: RTLdialog.h  $
 * Revision 1.5 1996/03/06 21:50:33EST abover 
 * sun4 SW port
 * Revision 1.7  1993/11/20  00:10:06  kws
 * Bug track: 5002
 * Fix badly parented dialog box
 * Added -nologger & -stderr
 *
 * Revision 1.6  1993/07/26  13:18:34  swu
 * added member function to override WM close menu option in RTLdialog
 *
 * Revision 1.5  1993/04/06  15:34:14  smit
 * Add gtRTL member function.
 *
 * Revision 1.4  1993/02/05  00:54:05  glenn
 * Clarify comments and sample code.
 *
 * Revision 1.3  1993/01/13  06:40:22  glenn
 * Add button() for changing visibility, sensitivity, and label of buttons.
 *
 * Revision 1.2  1992/12/21  20:03:09  glenn
 * *** empty log message ***
 *
    END-LOG---------------------------------
*/

#endif // _RTLdialog_h
