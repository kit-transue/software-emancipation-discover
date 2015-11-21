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
#ifndef _group_h
#define _group_h

// group.h
//------------------------------------------
// synopsis:
// define some UI for groups/subsystems
//
//------------------------------------------

#ifndef _projectBrowser_h
#include <projectBrowser.h>
#endif
      
#ifndef _groupHdr_h      
#include <groupHdr.h>
#endif      

RelClass(ldrSubsysMapHierarchy);

enum
{
   SCRATCH_GROUP_SEL = 0,
   SAVED_GROUP_SEL,
   SUBSYSTEM_SEL
};

enum
{
    DIRECT_PROJS_SEL = 0,
    ALL_PROJS_SEL,
    MIMIC_PROJS_SEL
};

class Create_Group
{
  public:
    Create_Group(char *ln, groupHdrPtr client, ldrSubsysMapHierarchyPtr ldrh);
    ~Create_Group();

    static int get_group_sub_def();

    void tog_add_selected();

    void set_def_add_selected();
    void set_def_group_sub(int v);
    void set_def_selected_type(int v);

    static int obtain_group_sub_flag();
    static int obtain_add_selected_flag();
    static int obtain_selected_type_flag();

    static void load_Create_Group(char *ln, groupHdrPtr client = NULL,
				  ldrSubsysMapHierarchyPtr ldrh = NULL);

    gtDialogTemplate* dlg;

  private:
    static int group_sub;     // 0: scratch_group; 1: saved_group, 2: subsystem
    static int add_selected;  // 0: No;    1: Yes
    static int selected_type; // 0: Direct Project Members; 1: All Project Members; 2: Mimic Project Structure

    // work buffer
    int def_group_sub;
    int def_add_selected;
    int def_selected_type;
    groupHdrPtr client_group;
    ldrSubsysMapHierarchyPtr into_ldrh;

    gtRadioBox* group_sub_sel;
    gtToggleButton* tog;
    gtRadioBox* project_def_sel;
    gtForm* name_form;
    gtLabel* name_label;
    gtStringEditor* group_name;

    static void make_group(void *data, bool delete_dialog);
    static void ok_CB     (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void apply_CB  (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void cancel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);

    static void scratch_group_sel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void saved_group_sel_CB   (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void subsystem_sel_CB     (gtPushButton*, gtEventPtr, void* data, gtReason);

    static void tog_add_selected_CB (gtToggleButton*, gtEventPtr, void* data, gtReason);

    static void direct_projs_sel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void all_projs_sel_CB    (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void mimic_projs_sel_CB  (gtPushButton*, gtEventPtr, void* data, gtReason);

    void save_selected();
    void build_interface();
    int  obtain_def_add_selected_flag();
    void set_project_def_sel(int flag);
};

enum
{
    GROUP_CLIENT_REL = 0,
    GROUP_SERVER_REL,
    GROUP_PEER_REL,
    GROUP_UNRELATED_REL
};

enum
{
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT
};

class Relate_Group
{
  public:
    Relate_Group(objArr &a);
    ~Relate_Group();

    gtList* left_group_list;
    gtList* right_group_list;
    gtPushButton* transfer_button;
    int transfer_direction;
    bool set_relation();
    void transfer(int direction);

    static void load_Relate_Group(objArr &a);

    objArr ga;

    gtDialogTemplate* dlg;

  private:
    static int relation_sel;

    int def_relation_sel;

    objArr group_arr_save;

    gtRadioBox* group_relate_sel;

    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void select_left_group_CB(gtList*, gtEventPtr, void*, gtReason);
    static void select_right_group_CB(gtList*, gtEventPtr, void*, gtReason);
    static void action_CB(gtList*, gtEventPtr, void*, gtReason);
    static void transfer_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    static void set_relate_client_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void set_relate_server_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void set_relate_peer_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void set_relate_unreated_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void set_relate_self_CB(gtPushButton*, gtEventPtr, void* data, gtReason);

    void save_selected();
    void build_interface();

    void set_group_relation(int v);
};

class Browse_Group_Filter
{
  public:
    Browse_Group_Filter();
    ~Browse_Group_Filter();

    static char get_targets_groups_flag();
    static char get_targets_members_flag();
    static char get_targets_nonmembers_flag();
    static char get_group_relation_peers_flag();
    static char get_group_relation_clients_flag();
    static char get_group_relation_servers_flag();
    static char get_group_relation_unrelated_flag();
    static char get_group_relation_self_flag();
    static char get_member_file_flag();
    static char get_member_direct_entity_flag();
    static char get_member_indirect_entity_flag();
    static char get_access_public_flag();
    static char get_access_private_flag();
    static char get_transparency_local_flag();
    static char get_transparency_transitive_flag();

    static char get_default_flag();

    static void load_Browse_Group_Filter();

    gtDialogTemplate* dlg;

  private:

    static char targets_groups_flag;
    static char targets_members_flag;
    static char targets_nonmembers_flag;
    static char group_relation_peers_flag;
    static char group_relation_clients_flag;
    static char group_relation_servers_flag;
    static char group_relation_unrelated_flag;
    static char group_relation_self_flag;
    static char member_file_flag;
    static char member_direct_entity_flag;
    static char member_indirect_entity_flag;
    static char access_public_flag;
    static char access_private_flag;
    static char transparency_local_flag;
    static char transparency_transitive_flag;

    static char default_flag;

    gtPushButton* select_all_button;
    gtPushButton* clear_all_button;

    gtToggleButton* targets_groups_tog;
    gtToggleButton* targets_members_tog;
    gtToggleButton* targets_nonmembers_tog;

    gtToggleButton* group_relation_peers_tog;
    gtToggleButton* group_relation_clients_tog;
    gtToggleButton* group_relation_servers_tog;
    gtToggleButton* group_relation_unrelated_tog;
    gtToggleButton* group_relation_self_tog;

    gtToggleButton* member_file_tog;
    gtToggleButton* member_direct_entity_tog;
    gtToggleButton* member_indirect_entity_tog;

    gtToggleButton* access_public_tog;
    gtToggleButton* access_private_tog;

    gtToggleButton* transparency_local_tog;
    gtToggleButton* transparency_transitive_tog;

    static void apply_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void default_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void done_CB(gtPushButton*, gtEventPtr, void* data, gtReason);

    static void select_all_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void clear_all_CB(gtPushButton*, gtEventPtr, void* data, gtReason);

    static void targets_groups_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void targets_members_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void targets_nonmembers_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);

    static void group_relation_peers_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void group_relation_clients_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void group_relation_servers_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void group_relation_unrelated_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void group_relation_self_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);

    static void member_file_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void member_direct_entity_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void member_indirect_entity_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);

    static void access_public_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void access_private_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);

    static void transparency_local_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);
    static void transparency_transitive_CB(gtToggleButton*, gtEventPtr, void* data, gtReason);

    void build_interface();
};

class Edit_Wildcard_Group
{
  public:
    Edit_Wildcard_Group(groupHdr *gr);
    ~Edit_Wildcard_Group();

    static void load_Edit_Wildcard_Group(groupHdr *gr);
    gtDialogTemplate* dlg;
    groupHdr *lgr;

    static void static_sort_wildcard(genString &txt);
    static void project_action_CB(gtList*, gtEventPtr, void* data, gtReason); 
    static void project_select_CB(gtList*, gtEventPtr, void* data, gtReason);
    static void module_select_CB(gtList*, gtEventPtr, void* data, gtReason);
    static void module_action_CB(gtList*, gtEventPtr, void* data, gtReason);
 
  private:
    gtCascadeButton* cascade_button;

    char wildcard_changed_flag;

    gtLabel* group_name;

    gtTextEditor *wildcard_list;
    gtRTL *project_list;
    gtRTL *module_list;

    genString saved_wildcard;

    static void expand_edit_wildcard_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void set_wildcard_group_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void save_edit_wildcard_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void quit_edit_wildcard_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void sort_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void up_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
//  static void module_select_CB(gtList*, gtEventPtr, void* data, gtReason);  // made public
//  static void module_action_CB(gtList*, gtEventPtr, void* data, gtReason);  // made public
//  static void project_select_CB(gtList*, gtEventPtr, void* data, gtReason); // made public
//  static void project_action_CB(gtList*, gtEventPtr, void* data, gtReason); // made public
    static void wildcard_changed_CB(gtTextEditor*, gtEventPtr, void* data, gtReason);
    static void remove_mark_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void remove_unused_CB(gtPushButton*, gtEventPtr, void* data, gtReason);

    void build_interface(const char *name);

    int project_selected_num();
    void set_wildcard_button(int v);
    void set_wildcard_group();

    void expand();

    void down();
    void up();

    void blank_module_list(int warn_flag);
    void modules_selected();

    void get_wildcard(genString &s, int only_remove_mark);
    void set_wildcard(const char *s);
    projNode *cur_proj;

    void wildcard_changed();

    void set_remove_mark_button_sensitive(int v);
    void set_remove_unused_button_sensitive(int v);
    void report_unused();

    int load_projects(projNode *pn);
    void sort_wildcard(int warn_flag);
    void save_wildcard();

    void set_wildcard_used_mark(char **sel_items, int size);
    void remove_mark();
    void remove_unused();
};

enum
{
    PUBLIC_MEMBER_SEL,
    PRIVATE_MEMBER_SEL
};

class Add_Member
{
  public:
    Add_Member(groupHdr *g, symbolArr &sel);
    ~Add_Member();

    static void load_Add_Member(groupHdr *g, symbolArr &sel);

  private:
    groupHdr  *grp;
    symbolArr contents;

    static int selected_type; // 0: Direct Project Members; 1: All Project Members; 2: Mimic Project Structure
    static int member_type;   // 0: public; 1: private

    // work buffer
    int def_selected_type;
    int def_member_type;

    gtDialogTemplate* dlg;
    gtRadioBox* project_def_sel;
    gtRadioBox* member_type_sel;

    static void ok_CB     (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void cancel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);

    static void pub_member_type_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void pri_member_type_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    void set_def_member_type(int v);

    static void direct_projs_sel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void all_projs_sel_CB    (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void mimic_projs_sel_CB  (gtPushButton*, gtEventPtr, void* data, gtReason);
    void set_def_selected_type(int v);

    void save_selected();
    void build_interface();
};

extern void create_wildcard(projNode *pn, genString &wild_text, char mod_only);
extern char **split_wildcard(genString &txt, int &count);
extern void free_wildcard_split_buffer(char **buf, int count);
extern void generate_wildcard_match_pattern(char *wild_text);
extern int  wildcard_accept(const char *name);
#endif






