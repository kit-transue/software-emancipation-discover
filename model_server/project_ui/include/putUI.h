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
// file putUI.h
//   define class for put dialog box

#ifndef _putUI_h
#define _putUI_h

enum ModOp
{
    PUT_SELECTED = 1,
    UNLOCK_SELECTED,
    MERGE_SELECTED,
    PUT_ALL,
    CI_ANYWAY,
    PUT_TO_BROWSER
};

class RTLNode;

class putUI
{
  public:
    putUI(gtBase* parent);
    ~putUI();
    inline void set_mod_op(ModOp op);
    inline ModOp get_mod_op();
    void merge(int level, gtPushButton* a, gtEventPtr b, gtReason c);
    void set_sensitivities(ModOp op);
    void hide_put_buttons();

    void move_locked_module(projModule *mod);
    static void global_move_locked_module(projModule *mod);

    static putUI *load_putUI(symbolArr& symbols);

    int module_in_merge_column_for_put(projModule *pm);

    static void ready_to_put_select_CB (gtList*, gtEventPtr, void* data, gtReason);
    static void locked_modules_select_CB (gtList*, gtEventPtr, void* data, gtReason);
    static void merge_modules_select_CB (gtList*, gtEventPtr, void* data, gtReason);
    static void not_to_put_select_CB (gtList*, gtEventPtr, void* data, gtReason); 

  private:
    ModOp mod_operation;
    int one_at_a_time;
    int build_interface(gtBase* parent);
    int fill_modules(symbolArr& symbols, objArr& result);  // result used only during fast put/merge
    int fill_one_module(projModule *mod);
    gtLabel* ready_to_put_modules_label;
    gtRTL* ready_to_put_modules_rtl;
    gtLabel* locked_modules_label;
    gtRTL* locked_modules_rtl;
    gtLabel* merge_modules_label;
    gtRTL* merge_modules_rtl;
    gtLabel* not_to_put_modules_label;
    gtRTL* not_to_put_modules_rtl;
    gtForm* middle_action_form;
    gtLabel* status;
    gtToggleButton* tog;
    gtLabel* put_options_label;
    gtStringEditor* put_options_text;
    gtStringEditor* bugtrack;
    gtPushButton* ci_anyway_button;
    gtPushButton* quick_merge_button;
    gtPushButton* detail_merge_button;
    gtPushButton* clear_button;
    gtPushButton* verify_button;
    gtPushButton* pall_button;
    gtPushButton* put_sel_button;
    gtPushButton* browser_but;
    gtPushButton* unlock_lock_but;
    gtTextEditor* comment;

    void check_for_module_comment();
    bool get_first_module_comment(symbolArr);

    void do_diff(symbolArr);
    void do_put (symbolArr);
    void do_fast_put(symbolArr&, genString&, genString&);
    int  empty_windows();

    projModule *obtain_one_only_selected_module();

    static void put_all_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void diff_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void cancel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void put_sel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void unlock_lock_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void browser_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void quick_merge_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void detail_merge_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void clear_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void verify_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void one_time_CB (gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void ci_anyway_CB(gtPushButton*, gtEventPtr, void* data, gtReason);

    static int  module_is_already_in_put_dialog(projModule *mod);
    void initial_status_text();

    RTLNode* modules_ready_to_put_nodes;
    RTLNode* modules_need_to_lock_nodes;
    RTLNode* modules_3_file_merge_nodes;
    RTLNode* modules_not_to_put_nodes;
};

int performing_fast_put();
int performing_fast_merge();


// PutUI inline methods.
inline void putUI::set_mod_op(ModOp op) { mod_operation = op; }
inline ModOp putUI::get_mod_op() { return mod_operation; }

#endif

extern int check_module_in_put_merge(projModule *pm);
extern void get_put_pdf_file_name(genString &pdf_name);

