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
#ifndef _Boilerplate_h
#define _Boilerplate_h

// Boilerplate.h
//------------------------------------------
// synopsis:
//------------------------------------------

#include <gtGenArr.h>
#include <genString.h>
#include <symbolArr.h>

#define No_of_boilerplates 35


void init_default_boilerplates (int);

extern  void set_class_name(genString);
extern  void set_function_name(genString);
extern  void set_struct_name(genString);
extern  void set_member_name(genString);
extern  void set_physical_file_name(genString);
extern  void set_logical_file_name(genString);
extern  void set_source_name(genString);
extern  void set_target_name(genString);
extern  void set_author_name(genString);
extern  void set_user_name(genString);
extern  void set_Date(genString);
extern  void set_project_name(genString);

RelClass(projModule);

class Boilerplate
{

  protected:  
    static Boilerplate* instance;
    int not_in_pdf;  
    GenArr  main_array;
    GenArr  intermediate_array;
    GenArr type_array; 
    GenArr temp_array; 
    genString home_proj_name;
    genString user_name;
    genString boilerplate_type;
    symbolArr s1;
    int default_bit;
    bool rel_impl_mode;
    genString dialog;

  public:
    Boilerplate ();
    ~Boilerplate ();

    static void refresh_after_delete ();

    static void initialize (const char *dialog);
    static void close ();

    static void do_same_thing (void* cd, int index);

    static void load_bplist (char *filename);
    static int GetInstanceBpCount ();
    static const char *GetInstanceBoilerplate (int index);
    static const char *GetInstanceBoilerplateType ();
    static const char *GetInstanceDefault ();
    static void SetInstanceDefault (int index, int global);
    static void SaveInstanceDefaults ();
    static void RestoreInstanceDefaults ();
    static void RemoveInstanceDefault ();
    static void DeleteInstanceBoilerplate (int index);
    static void NewInstanceBoilerplate (const char *bpName);

    static genString insert_func_init();
    static genString insert_func_final();
    static genString insert_before_func_decl();
    static genString insert_after_func_decl();
    static genString insert_before_func_def();
    static genString insert_after_func_def();
    static genString insert_pub_decl();
    static genString insert_priv_decl();
    static genString insert_prot_decl();
    static genString insert_before_class_decl();
    static genString insert_before_class_def();
    static genString insert_after_class_decl();
    static genString insert_after_class_def();
    static genString insert_before_struct();
    static genString insert_after_struct();
    static genString insert_member_def_init();
    static genString insert_member_def_final();
    static genString insert_before_member_def();
    static genString insert_after_member_def();
    static genString insert_before_member_decl();
    static genString insert_after_member_decl();
    static genString insert_relation_src_member();
    static genString insert_relation_src_header();
    static genString insert_relation_src_definition();
    static genString insert_relation_trg_member();
    static genString insert_relation_trg_header();
    static genString insert_relation_trg_definition();
    static genString insert_c_src_file_top();
    static genString insert_c_src_file_bottom();
    static genString insert_c_hdr_file_top();
    static genString insert_c_hdr_file_bottom();
    static genString insert_cpp_src_file_top();
    static genString insert_cpp_src_file_bottom();
    static genString insert_cpp_hdr_file_top();
    static genString insert_cpp_hdr_file_bottom();

    static genString insert_boilerplate(genString name);
    static genString filter_boilerplate(genString original_string);

  private:
    const char *add_type_array (projModule* module);
    void  init_sorted_array(GenArr &temp_array);
    void  init_type_array();
    bool  construct_name (const char* base, genString& name);

  protected:
    int   get_boilerplate_default (genString& default_type);
    int   set_boilerplate_default (const char* name);
    void  set_boilerplate_default (char *bpName, int global_bit);
    int   default_index();
    void  save_defaults ();
    void  restore_defaults ();
    void  delete_boilerplate (char *);
    const char *new_boilerplate (const char *);

    int is_default_bit_set(){ return default_bit;}
    void set_default_bit(){ default_bit = 1; }
    void reset_default_bit(){ default_bit = 0;}
};

#endif // _Boilerplate_h
