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
#ifndef _proj_h
#define _proj_h

#include <objRelation.h>
#include <symbolArr.h>

#include <RTL.h>
#include <xref.h>
#include  <customize.h>
#include  <projList.h>
#include  <loadedFiles.h>

#include <proj_hash.h>

#include <projHeader.h>

// function prototype;  function used in cleanup, to kill configshell task
int configurator( char const *cmd, char const *tag, char const *arg1 = 0, char const *arg2 = 0,
                  char const *arg3 = 0, char const *arg4 = 0, char const *arg5 = 0, char const *file_type = 0);

class Xref;
class projMap;
class projModule;
class projHeader;
class projNode;
class projContents;
class fileEntry;
class projLine;

extern int multiple_psets;

defrel_one_to_many(appTree,proj,appTree,module);
defrel_one_to_many(appTree,proj,appTree,sub);

class projModule : public appTree
{
  public:
    define_relational(projModule,appTree);
    
    projModule(char const *ln, projContents *proj, char const *phy_path, int i_selector = 0);
    projModule(projNode *prnt, projLine& pline);      // restoring saved Module

    ~projModule();
    
    projNode * get_project() const;
    projNode * get_project_internal() const;
    projNode * get_target_project(int force_display);
    void set_target_project(projNode *pn);

    symbolPtr get_def_file_from_proj (projNode *pr = NULL);

    appPtr restore_module();
    appPtr restore_module(int update_xref);
    appPtr restore_module(char const *ln);
    appPtr load_module();
    appPtr load_module_internal(int do_import);
    //appPtr update_reparse_module(); // update module, or load & update it
    appPtr import_module();
    void   update_xref();
    int unload_module();
    int lock_module();
    int unlock_module();
    int unlock_module_c(char const *);
    int unget_module();
    int obsolete_module();
    int is_locked();

    static int obtain_unlock_message(genString &msg, char const *name = 0, char const *button_txt = 0);

    void get_file_names_internal(genString& src_file,         genString& dst_file,
                                 genString& src_paraset_file, genString& dst_paraset_file);

    static projModule* get_module(symbolPtr&);
    static projModule* get_module_with_data(symbolPtr&, void *);
  
    static projModule* copy_module(symbolPtr&);
    projModule* copy_module_c(char const *tar_dir);
    projModule* input_module(int type, char const *rev, char const *tar_dir); // will work for copy_module, copy_module, get_prev_module
    projModule* input_module(int type, char const *rev, char const *tar_dir, void *data);   // used for getting with comment 
    int get_module_comment(genString& comment);
    int put_start(char const *bug, char const *comment, char const *files);
    int put_end(char const *bug, char const *comment, char const *files);
    int put_module(projNode*, char const *opt, char const *message, char const *rev, char const *xterm_file_name);
    int fast_put_module(projNode*, char const *opt, char const *message, char const *rev, char const *xterm_file_name);
    int notify_put_module(projNode*, char const *opt, char const *message, char const *rev, char const *xterm_file_name);
    int fast_merge_module();
    int check_in_pset(projModule *dst_mod, char const *rev, char const *opt, char const *comment, char const *vf,
                       char const *src_file, char const *src_paraset_file, char const *dst_paraset_file,
                      char const *xterm_file);
    int check_out_pset(projModule *local_mod, char const *local_file, char const *vf,
                       char const *sys_paraset_file, char const *local_paraset_file,
                       char const *xterm_file, char const *tar_dir, int search_pset);
    int diff_module(projNode*);
    int diff_module_c(projNode*, int display_flag);
    int save_module();
    int delete_module();
    int delete_module(int new_flag);
    int delete_module(int new_flag, int type);
    projModule *get_prev_module(char const *rev);
    static int create_new_file(char const *file_name);
    int         cfg_fast_put(projNode*, char const *, char const *);
    int         cfg_fast_merge();
    int         cfg_diff(projNode*);

    char const **get_version_list(char const *date, int &length);
    void update_local_rtl_pmod(); // it will delete itself

    void update_path_hash_tables(projMap *pm, projNode *pn);

    void update_module(); // Update list icons

    // type == 0 from paraset
    // type == 1 form -batch mode
    static int merge_module(projModule *pm, char const *file, char const *paraset_file, char type, char backup_flag,
                            char unload_flag, int &donot_save_flag);

    void language (fileLanguage);
    fileLanguage language () const; // {return commonTree::language;}
    Obj* get_dds();

    bool is_project_writable() const;
    bool is_home_project() const;
    bool is_loaded();
    bool is_modified();
    bool is_paraset_file();
    bool is_good_paraset_file();
    bool is_raw_modified(char type);
    bool is_raw_modified(char type, projModule* &del);
    static bool is_raw_modified(char const *fn, char type, app* &del);

    projModule *remove_link(projNode *pn, char const *ln, int &flag);
    projModule *create_script_module(projNode *pn, char const *ln);

    int need_3file_merge(genString &rev, projNode *sys_proj);
    void get_rev(genString &rev, int type); // type = 0; for put; type = 1 for get
    void get_rev_from_source(genString &rev);

    int really_do_3_fmerge(char const *rev, char const *fn, char const *dst_file, char const *pset_file);
    void merge_3f_fail(int type);

    char const *paraset_file_name();
    void invalidate_cache();
    int check_write_priv(int type);

    void touch_module(int need_reparse_bit);

    int paraset_file_modified ();

    void set_cur_ver(char const *cv);
    char const *get_cur_ver();

    appPtr get_app();
    static appPtr get_app(char const *ln, char const *pn);
    static appPtr get_app(char const *ln);
    static projModule *projModule_of_app (app *sh);
    int outdated_pset();
    char const *get_phys_filename ();
    void get_phys_filename (genString&pn); // temp notinline {pn = get_phys_filename();}
    void dependency_analysis(app* head, projNode *proj, int not_save_flag = 0);
    void dependency_analysis_for_check_in(app* head);
    void get_destination_candidates(objArr&);
    void set_delete_flag(int i) { del_flag = i;}
    int get_delete_flag() { return del_flag;}
    appPtr reparse_module();
    appPtr find_app(); //app or raw app
    
    void set_app_ln(char const *ln);
    char const *get_app_ln();

    void setup_SWT_Entity_Status(app *);

    virtual void put_signature(ostream& os) const;
    void print (ostream& str = cout, int level = 0) const;
    
    void save_signature(ostream& os);

    virtual int construct_symbolPtr(symbolPtr&) const;
    virtual ddKind get_symbolPtr_kind() const; // { return DD_MODULE; }
    virtual int is_symbolPtr_def () const; // { return 1; }   //analog to get_is_def()
    virtual app *get_symbolPtr_def_app ();

    virtual char const *get_name () const;
    char const *get_sname() const { return (char const *)get_pr_name(); }
    void set_sname(char const *s);

    ddKind get_kind () const { return DD_MODULE; }

    char const *get_pr_name () const;
    void        set_pr_name (char const *);

    void        set_psetfile_name (char const *);

    char const *get_implicit_name () const;

  private:
    app* app_head;
    genString cur_ver; // If you want to use it, initialize it
    unsigned int cache_valid        : 1;
    unsigned int cache_paraset_file : 1;
    unsigned int del_flag           : 1;
    unsigned int included           : 1;
    unsigned int junk               : 4;
    
    projNode *target_proj;
    
    genString ln_name;      // right, logical file name (former name of ddElement)
    genString imp_name;       // left, project file name
    fileEntry *projfile_name; // middle, physical project file name
    fileEntry *psetfile_name; // .pset physical file name
	
    // If pass logical name to db_restore() to force not to check the logic name in the
    // .pset. Basically, the logic name of this module is different from the logic name in
    // .pset. The logic name in .pset is stored in app_ln.
    genString app_ln;

  public:
    int  do_3_fmerge_for_branch(char const *rev, char const *fn, char const *dst_file, char const *pset_file);
    void branch_revision(genString &rev, int type);

    virtual HierarchicalPtr get_header_virtual () const;
    projHeader *get_header () const;
    int is_included () { return included; }
};
generate_descriptor(projModule,appTree);

extern projModulePtr app_get_mod(appPtr);
defrel_one_to_one (projModule,projectModule,app,appHeader);

class projFile : public appTree
{
  private:
    genString text_file;                // File name of text data
    int type;
  public:
    define_relational(projFile,appTree);
    projFile();
    projFile(char const *);
    ~projFile();
    void file_type(int);
    int file_type();
    virtual char const *get_name() const;
    void set_name(char const *);
};    
generate_descriptor(projFile,appTree);

//uses global pset rules (reallocates memory with each call)
extern char const *get_paraset_file_name(char const *src_file);

//uses the subproject pset rule
extern int smt_get_pset_name(char const *src_file, projNode* proj, genString& pname);

//use first the subproject pset rule and if failed the global level pset rule
extern char const *get_paraset_file_name(char const *fn, projModule *);
extern char const *get_paraset_file_name(char const *fn, projNode *);

extern int paraset_cmp(char const *local_fn, char const *global_fn);

extern int existsOrCreateDirectory(char const *, char const *);
extern bool has_paraset_file(char const *pset_file);

void proj_get_modules(projNode *proj, objArr& modules, int depth);
void proj_get_modules(projNode *proj, objArr& modules);

Xref *proj_attach_existing_xref (projNode *pn);

#endif // _proj_h
