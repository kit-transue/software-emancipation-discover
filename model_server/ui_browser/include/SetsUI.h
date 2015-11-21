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
#ifndef _SetsUI_h
#define _SetsUI_h

//
// "SetsUI.h"
//
// Description: 
//
//     Dialog box for set-theoretic operations.
//     Manipulates Access/sw "sets."
//

#include <gt.h>
#include <general.h>    /* For bool */
#include <genString.h>
#include <TclList.h>

class symbolArr;

class SetsUI
{
  public:
    
    SetsUI(const char *);
    ~SetsUI();

    static void Invoke(const char *);      // Invoker
    static void CaptureAndInvoke(const char *title, symbolArr& syms);
    static void Refresh();                 // Fill in the list
    static void ListSelected(TclList&);

    //
    // Callbacks:
    //
    static int destroy_CB(void *);
    static void convert_to_subsystem_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void create_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void rename_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void delete_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void save_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void print_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void refresh_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void quit_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void intersection_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void union_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void diff_ab_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void diff_ba_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void ModelBrowser_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void MiniBrowser_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void instances_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void browse_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void list_double_click_CB(gtList*, gtEventPtr, void*, gtReason);
    static void ElementsAdd_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void ElementsRemove_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void ElementsMove_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static char *GetGroupName(void);
    static void UpdateGroupName(void);

    static int execCmd(genString cmd, int outFlag=0);

  private:

    //
    // Special enums:
    //

    typedef enum { 
      UNARY,
      BINARY, 
      ONE_OR_MORE,
      TWO_OR_MORE
    } OperandNumCode;

    typedef enum {
      FORWARD,
      BACKWARD
    } ListDirCode;

    typedef enum {
      SIZE,
      LINDEX,
      SEARCH,
      MODELBROWSE,
      MINIBROWSE,
      IVIEW,
      CREATE,
      RENAME,
      DELETE,
      SAVE,
      PRINT,
      INTERSECTION,
      UNION,
      DIFFERENCE,
      ADD,
      REMOVE,
      MOVE
    } CmdCode;

    typedef enum {
      MINI_B,
      MODEL_B,
      INSTANCE_B
    } BrowseCode;

    int    invoked;
    static void sendToBrowser(BrowseCode);            // Send current selections to the MiniBrowser
    static int sendCmd(genString&);
    static int sendCmd(const genString&, genString&);
    static void tclList(genString&, ListDirCode);
    static void tclList2(TclList&);
    static void notifyTclError();
    static bool inList(const char *);                 // Is the string in our list?
    static void quitWindow();                         // Make interface dissapear
    static bool setDefaultName();                     // Put some default string in the name box
    static bool checkOperands(OperandNumCode);
    static char *targetName();                        // Get text in name box; do some error checking

    void buildInterface(const char *);                // Initialize the dialog
    void checkDebug();

    //
    // Data:
    //
    static SetsUI *instance;                          // Active instance
    static gtList *sets_list;                         // Run-time list of sets
    static int name_idx;
    static gtStringEditor *name_txt;
    static gtTopLevelShell *toplev;
    static const int max_name_idx;
    static const char *api_cmd_prefix;
    static const char *api_cmd[];
    static bool debug_flag;
    static const char *default_title;
};

#endif /* ! _SetsUI_h */
