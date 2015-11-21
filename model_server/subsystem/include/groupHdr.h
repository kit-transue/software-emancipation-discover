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
// /aset/subsystem/include/groupHdr.h
//------------------------------------------
// description:
// 
// Declares class groupHdr.      
//------------------------------------------
// History:
//
// Tue Apr 11 16:29:45 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _groupHdr_h
#define _groupHdr_h
#include <cLibraryFunctions.h>

//========================================
// Class groupHdr
//	Created Tue Apr 11 16:29:45 1995 by William M. Miller
//----------------------------------------
// Description:
//
// This is the app header for groups and subsystems.  It contains all member
// functions for dealing with them (i.e., unlike bare RTLs, adding and removing
// members, etc., is done using the header, not the tree root).
//========================================

#ifndef _RTL_h
#include <RTL.h>
#endif      

#ifndef _subsys_h
#include <subsys.h>
#endif

#ifndef _objOper_h
#include "objOper.h"
#endif     

#ifndef _ddict_h
#include "ddict.h"
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif      
      
#ifndef _proj_h
#include <proj.h>
#endif
      
//#ifndef steDocument_h
//#include <steDocument.h>
//#endif

#ifndef _genString_h
#include <genString.h>
#endif
      
#ifndef __objSet
#include <objSet.h>
#endif      
      
#ifndef _symbolSet_h
#include <symbolSet.h>
#endif


      
class groupHdr : public RTL {
public:
   define_relational(groupHdr,RTL);
   copy_member(groupHdr);

   enum nested_projects_action {
      LOCAL_FILES_ONLY,
      ALL_NESTED_FILES,
      MIRROR_HIERARCHY,
      IGNORE_PROJECTS
   };
   
   enum added_member_access {
      DEFAULT_ACC,
      PUBLIC_ACC,
      PRIVATE_ACC
   };
   
   friend class groupTree;
   friend app* subsysHeader::db_restore(char*, byte, char*);
   
	groupHdr( const char* group_name, bool make_subsys,bool scratch_group);
	groupHdr(const char* group_name, const char* phys_file_name);
	groupHdr(const groupHdr&);

	bool delete_group(bool confirm, bool yes_to_all);

	void insert_obj(objInserter* oi, objInserter* ni);
	void remove_obj(objRemover* oi, objRemover* ni);
	void notify(int flag, RelationalPtr obj, objPropagator*, RelType*);
	static groupHdr* db_restore(char* group_name, byte, char* phys_file_name);
	void db_after_restore();
	void add_contents(ddNodePtr dd_node);
	bool add_members( const symbolArr& selections, nested_projects_action proj_action,added_member_access new_access);
	size_t get_members( symbolArr& members,bool include_transitives) const;
	bool validate_members(const symbolArr& putative_members, bool accept_transitives) const;
	size_t remove_members(const symbolArr& old_members);
	size_t make_members_public(const symbolArr& new_pub_members);
	size_t make_members_private(const symbolArr& new_pri_members);
	size_t get_public_members(symbolArr& pub_members, bool add_transitives) const;
	size_t get_private_members(symbolArr& pri_members) const;
	bool make_client(groupHdr* targ);
	bool make_server(groupHdr* targ);
	bool make_peer(groupHdr* targ);
	bool make_unrelated(groupHdr* targ);
	size_t get_clients(symbolArr& clients) const;
	size_t get_servers(symbolArr& servers) const;
	size_t get_peers(symbolArr& peers) const;
	groupHdr* split(const symbolArr& members, const genString& new_grp_name);
	size_t make_server_members_visible(const symbolArr& svr_members);
	size_t make_server_members_invisible(const symbolArr& svr_members);
	bool is_subsystem() const { return is_subsys; }
	bool is_saved() const { return saved_as_scratch; }
	bool is_access_set() const { return mbr_access_has_been_set; }
	bool convert_to_subsystem();
	bool convert_to_group();
	bool file_should_be_member(const genString& logical_name) const;
        //	bool validate(projNode* matching_proj_root, steDocument* report) const;
	bool reduce_to_files();
	const groupHdr* get_root_group() const;
        groupHdr* get_root_group();
	static bool validate_new_clients_servers(const objArr& clients, const objArr& servers);
	static groupHdr* sym_to_groupHdr(symbolPtr sym);
	symbolPtr get_xrefSymbol()const;
	symbolPtr get_def_file()const;

        void validate_group_with_project(projNode *pr, FILE *out, FILE *summary = NULL);
        int  match_hierarchy(projNode *pr, objArr &groupArr, FILE *out);
        void validate_module(projNode *pr, symbolSet &symSet, FILE *out);
        void collect_all_modules(symbolSet &symSet, objArr &groupArr, FILE *out);
        void collect_all_members(symbolArr &symArr, objArr &groupArr, FILE *out);
        int validate_member_of_groups_single_hierarchy(symbolArr &symArr, FILE *out);
        int validate_program_member_not_in_file_member(symbolArr &symArr, symbolSet &symSet, FILE *out);
        int validate_program_member_has_file_member(symbolArr &symArr, symbolSet &symSet, FILE *out);
        int validate_private_member_used_outside_group(FILE *out);
        int validate_private_member_is_used(FILE *out);
        int validate_public_member_used_not_client_peer_group(FILE *out);
         int validate_public_member_is_used(FILE *out);

        size_t get_transitives(symbolArr& transitives);
        void pr(FILE *out);
	virtual boolean is_scratch();	// => is_scratch_group && !saved_as_scratch
	void get_entire_hierarchy(objArr& all_groups);
	static bool adding_to_scratch_group();
	virtual void set_name(char *);
        static groupHdr * create(const char *, bool, bool);
	static bool offer_to_save_scratch_groups();	// false => cancelled
	static bool offer_to_save_scratch_groups(const symbolArr& selected_groups);
	// ditto
	bool make_nonscratch();
	bool rename_if_scratch(const char* new_name);
	static void name_to_module_ln(const genString& group_name, genString& module_ln);
        static void load_ascii_subsys(char *);
        static void load_ascii_subsys(char *, int);
        static void get_rtl_name(char *, genString &);
        static groupHdr *find_create_group(const char *);
        static int import_group_from_file(groupHdr *, char *, symbolArr&);
        static void process_entry(groupHdr *, symbolArr&, char *);
        static ddKind ddKind_internal_val(char *);
        static void relate_groups(groupHdr *, char *, char *, char);

        static void (*pb_subsys_hook)(int, const char *);

private:
	bool is_subsys;
	bool mbr_access_has_been_set;
	bool is_scratch_group;
	bool saved_as_scratch;
	ddElementPtr get_def_dd() const;
	bool find_all_leaves( objSet& leaves,const objArr&clients,const objArr&servers);
	bool find_all_roots( groupHdr*& root,const objArr&clients,const objArr&servers);
	bool visited;
	bool rem_rel_with(groupHdr* targ, RelType* rel);
	bool add_rel_with(groupHdr* targ, RelType* rel);

        void add_members_impl(added_member_access acc, const symbolArr& mbrs);
        void remove_members_impl(added_member_access acc, symbolSet& old_mbrs);
        void prop_subsys_file_removal(symbolSet& contents);
	bool check_subsys_restrictions(symbolSet& all_members);
	void prop_subsys_flag(bool make_subsys);
	void collect_servers(objArr& all_groups);
	bool restoring_old_subsys;
	void rename_force(const char* new_name);
};
generate_descriptor(groupHdr,RTL);

defrel_many_to_many(ddElement,peer1,ddElement,peer2);
defrel_many_to_many(ddElement,server_grp,ddElement,client_grp);
defrel_many_to_many(ddElement,pub_mbr,ddElement,grp);
defrel_many_to_many(ddElement,pri_mbr,ddElement,grp);
defrel_many_to_many(ddElement,trans_mbr,ddElement,grp);

#endif
