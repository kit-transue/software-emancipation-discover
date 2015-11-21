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
// file  feedback.h
//   define class for configurator monitor

#ifndef _feedback_h
#define _feedback_h

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _gt_h
#include <gt.h>
#endif


enum StreamID
{
    CFG_SYNC = 1,
    CFG_ASYNC
};

class ErrorNode;

// class Error to hold error messages for a configuration command
class ErrorNode
{
  public:
    ErrorNode(int stat, const char* command, const char* ErrorNode_text,
              const char* misc_data, const char* file, symbolPtr& pm,
              projNode* sp, projNode* dp);
    ~ErrorNode();
    void set_status(int stat) { status = stat; }
    int get_status() { return status; }
    char *get_command() { return command; }
    char *get_error_text() { return error_text; }
    char *get_data() { return data; }
    char *get_filename() { return ln_filename; }
    ErrorNode* get_next_error() { return next; }
    ErrorNode* get_prev_error() { return prev; }
    void set_next_error(ErrorNode* err) { next = err; }
    void set_prev_error(ErrorNode* err) { prev = err; }
    symbolPtr &get_proj_mod() { return proj_mod; }
    projNode* get_src_pn() { return src_proj_node; }
    projNode* get_dest_pn() { return dest_proj_node; }

  private:
    ErrorNode* next;
    ErrorNode* prev;
    genString command;
    genString error_text;
    genString data;
    genString ln_filename;
    int status;
//    projModule* proj_mod;
    symbolPtr proj_mod;
    projNode* src_proj_node;
    projNode* dest_proj_node;
};

//-------------------------------------------------------------
class OpNode
{
  public:
    OpNode(const char *op, projModule* pm, projNode* d, const char* options,
           const char *bug, const char* comm, int oaat,
           char *src_file, char *dst_file, char *src_file_pset, char *dst_file_pset);
    ~OpNode();
    char *get_put_options() { return put_options; }
    char *get_bug_no() { return bug_no; }
    char *get_comment() { return comment; }
    char *get_ln() { return l_name; }
    char *get_src_file() { return src_file; }
    char *get_dst_file() { return dst_file; }
    char *get_src_file_pset() { return src_file_pset; }
    char *get_dst_file_pset() { return dst_file_pset; }
    OpNode* get_next_op() { return next; }
    OpNode* get_prev_op() { return prev; }
    void set_next_op(OpNode* op) { next = op; }
    void set_prev_op(OpNode* op) { prev = op; }
    int get_comment_tog() { return one_comment_at_a_time; }
    void set_comment_tog(int num) { one_comment_at_a_time = num; }
    projNode *get_src_proj() { return src_projNode; }
    projNode* get_dest_proj() { return dest_projNode; }
    char *get_command() { return command; }
    int get_merge_level() { return merge_level; }
    projModule *get_module() { return ln_module; }
    int is_put_command();
    int is_merge_command();
    char *get_rev() { return rev; }

  private:
    OpNode* next;              // for both
    OpNode* prev;              // for both
    genString l_name;          // for both
    genString put_options;     // only for put
    genString bug_no;          // only for put
    genString comment;         // only for put
    projNode* dest_projNode;   // only for put  // will become a projNode*
    projModule *ln_module;     // only for merge // will become a projModule*
    int one_comment_at_a_time; // only for put
    projNode* src_projNode;    // for both
    genString src_file;        // for both
    genString dst_file;        // only for put (but save for both)
    genString src_file_pset;   // only for put (but save for both)
    genString dst_file_pset;   // only for put (but save for both)
    genString command;         // for both
    int merge_level;           // only for merge
    genString rev;             // only for merge
};

//-------------------------------------------------------------

class FeedBack
{
  friend class FeedBackMonitor;

  public:
    // All message records are combined until a command record is seen
    // When a commandcmd is seen and the command was successful, all
    // current data is discarded.  When it fails, the combined messages
    // are "bound" with command and added to the error array.
    // The sync_command and async_command independently buffer data.

    FeedBack();
    ~FeedBack();

    static  void global_check_for_errors(projModule* projMod, const char *cmd);
    void init_fbm();

    void message(StreamID id, const char* msg);
    int find_dup_in_merge(projModule *pm);

    int find_dups(const char* cmd, int stat, projNode* srcn,
                  projNode* desn, symbolPtr& projmod);
    void report(StreamID id, const char* cmd, int stat, projNode* srcn,
                projNode* desn, symbolPtr& projm);
    void command(const char* op, projModule* pm, projNode* dpn, const char* opt, const char* bug,
                 const char* com, int oaat);
    // remember_put(char* ln, projModule*);
    ErrorNode* get_error_list() { return error_list; }
    OpNode* get_op_node_list() { return op_node_list; }
    ErrorNode* get_last_error();
    OpNode* get_last_op_node();
    ErrorNode* get_selected_error(int pos);
    FeedBackMonitor* get_fbm() { return fbm; }
    void set_fbm(FeedBackMonitor* f) { fbm = f; }
    void set_error_list(ErrorNode* f) { error_list = f; }
    void remove_selected_error(int pos);
    void check_for_errors(projModule* projMod);
    void clean_error_pos(int count, const char *msg);
    OpNode* get_first_op_node(int type, int &pos);
    OpNode* get_first_put_node(int &pos);
    OpNode* get_first_merge_node(int &pos);
    OpNode* get_selected_op_node(int pos);
    void remove_op(int pos);

    void clear_error_node();
    void clear_op_node(int type);
    void clear_put_node();
    void clear_merge_node();
    void close_dialog();
    void set_ln_being_put(char *p) { ln_being_put = p; }
    const char* get_ln_being_put() { return ln_being_put; }
    void set_src_proj_name_being_put(char *p) { src_proj_name_being_put = p; }
    const char* get_src_proj_name_being_put() { return src_proj_name_being_put; }
    void set_dst_proj_name_being_put(char *p) { dst_proj_name_being_put = p; }
    const char* get_dst_proj_name_being_put() { return dst_proj_name_being_put; }
    void put_fail_clean_up();
    void popup_fb_monitor();

  private:
    genString async_stream_messages;
    genString sync_stream_messages;
    ErrorNode* error_list;
    OpNode* op_node_list;
    FeedBackMonitor* fbm; //UI - NULL if not up
    genString ln_being_put;
    genString src_proj_name_being_put;
    genString dst_proj_name_being_put;
};

//---------------------------------------------------------------------

class FeedBackMonitor
{
  public:
    FeedBackMonitor(gtApplicationShell* app_shell);
    ~FeedBackMonitor();
    gtDialogTemplate* get_dlg() { return dlg; }
    void add_module(const char* text);
    void add_op_module(const char* text);
    void add_put_module(const char* text);
    void add_merge_module(const char* text);
    FeedBack* get_fb() { return feedback; }
    gtList* get_module_list() { return module_list; }
    gtList* get_modules_for_put_or_merge() { return modules_for_put_or_merge; }
    gtTextEditor* get_error_text() { return error_text; }
    gtLabel* get_status_text() { return status; }
    gtLabel* get_filename_text() { return filename_text; }
    gtLabel* get_command_text() { return cmd_text; }
    void update_put_status(const char *name, const char *act = 0, int type = 0);
    void update_merge_status(const char *name, const char *act = 0, int type = 0);
    int  get_put_module_num()   { return put_module_num;   }
    int  get_merge_module_num() { return merge_module_num; }
    void set_put_module_num(int n);
    void set_merge_module_num(int n);
    void clean_up_4_texts();

  private:
    FeedBack* feedback;
    gtDialogTemplate* dlg;
    gtForm* bottom_form;
    gtForm* top_form;
    gtList* module_list;
    gtList* modules_for_put_or_merge;
    gtLabel* cmd_text;
    gtLabel* status;
    gtLabel* filename_text;
    gtTextEditor* error_text;
    gtLabel* put_status;
    gtLabel* merge_status;
    gtPushButton *clear_put_button;
    gtPushButton *clear_merge_button;
    int put_module_num;
    int merge_module_num;

    static void module_select_CB(gtList*, gtEventPtr, void*, gtReason);
    static void module_for_put_or_merge_select_CB(gtList*, gtEventPtr, void*, gtReason);
    static void clear_fail_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void retry_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    void build_interface(gtApplicationShell* app_shell);
    static void clear_put_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void clear_merge_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
};

extern FeedBack* fb_instance;

extern int fb_put_one_file(genString &src_proj_name, genString &dst_proj_name, genString &module_name,
                           genString &put_opt, genString &comment,
                           genString &src_file, genString &dst_file,
                           genString &src_file_pset, genString &dst_file_pset,
                           genString &pdf_name);

extern void fb_status_one_file(const char *mod_name, int type, projNode *pn, projModule *pm);
extern bool fb_files_being_put(const char *ln);
extern bool fb_being_put();
extern int put_is_running();
extern void set_put_is_running();
extern char *create_one_tmp_file(char *msg);
extern int fb_merge_one_file();


#endif

/*
$Log: feedback.h  $
Revision 1.9 1996/08/13 14:11:39EDT jnp 
removed ifdefs for WIN32 to make NT compile
 * Revision 1.8  1994/07/29  17:09:24  so
 * Bug track: n/a
 * cm_create_new_file
 * cm_put_start
 * cm_put_end
 * cm_unget_src
 * cm_unget_pset
 * cm_obsolete_src
 * cm_obsolete_pset
 * When do 3file-merge, check out file should be put into control project directly
 *
 * Revision 1.7  1994/07/19  21:46:36  so
 * Bug track: n/a
 * create pdf for put process
 *
 * Revision 1.6  1994/05/12  16:28:20  so
 * Bug track: 6758, 71978, 7239
 * fix bug 6758, 7197, 7239, and improve put dialog box
 *
 * Revision 1.5  1994/03/13  11:58:38  so
 * Bug track: 6696
 * fix bug 6696
 *
 * Revision 1.4  1994/03/03  14:21:31  so
 * Bug track: 6578
 * fix bug 6578
 *
 * Revision 1.3  1994/02/24  16:08:41  so
 * Bug track: 6505
 * fix bug 6505
 *
 * Revision 1.2  1994/02/22  19:29:25  so
 * Bug track: 6468 and 6469
 * fix bug 6468 and 6469
 *
 * Revision 1.1  1994/02/16  13:37:26  so
 * Initial revision
 *
*/
