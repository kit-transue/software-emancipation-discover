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
#ifndef __projHeader_h
#define __projHeader_h

#ifndef _RTL_h
#include <RTL.h>
#endif

#include <symbolPtr.h>
#include <symbolArr.h>
#include <pathdef.h>

class projModule;
class projNode;
class genString;
class projMap;
class Xref;
class projContents;
class loadedFiles;
class fileEntry;
class smtHeader;
class module_hash_entry;

class projHeader : public RTL
{
  public:
    define_relational(projHeader, RTL);
    projHeader(char const *name, char const *dir);
  
    void print(ostream& = cout, int level = 0) const;
    virtual void remove_obj(objRemover *, objRemover * nr);

    //    static projModule* loaded_module_of_file (char const *fn);

    static projModule* find_module(char const *);
    static projModule* find_working_module(char const *);

    static int make_new_filename(char const *);

    static projNode* non_current_project(char const *ln);
    static appPtr load_module(char const *ln);
    static appPtr get_file(char const *phys_name);

    static projNode* fn_to_project (char const *, int is_prog=0);
    
    static void fn_to_ln (char const *, genString &ln, projNode ** pn=NULL, int is_prog=0);
    static void fn_to_ln (char const *, genString &ln, projNode ** pn, int is_prog, int no_hash);

    //boris: added genString *  to return filename after project_convert_filename()
    static void fn_to_ln (char const *, genString &ln, projNode ** pn, int is_prog, int no_hash, genString *);

    static void ln_to_fn (char const *, genString &fn);
    static projNode *ln_to_projNode(char const *ln);
    static void fn_to_ln_special_rule (char const *, genString &rule, projNode *pn = NULL);
    static void fn_to_ln_special_rule (char const *, genString &rule, int type, projNode * &);
    static void make (char const *target,
		      char const ** dirs, int dir_size, char const ** files, int file_size);

    static void save_all_projects();
    static int is_writable(char const *ln);

    static int make_path(char const *);
  static projNode* get_pmod_project_from_ln (char const *ln);

  virtual int construct_symbolPtr(symbolPtr & ) const;
};
generate_descriptor(projHeader, RTL);

// derived from appTree, commonTree, ...

class projNode : public RTLNode {

public:

  friend projNode *proj_make_project(char const *, projNode *, char const *, 
				     projMap *, char const *, int pmod_type = PMOD_UNKNOWN);

  define_relational(projNode, RTLNode);

  projNode(char const *name, Xref *xref);    // dummy constructor for use in sorted_link stuff

  projNode(char const *name, projHeader *, char const *dir,
	   projMap *map = 0, int open_xref = 0, int pmod_type = PMOD_UNKNOWN);

  ~projNode();

  void delete_projnode();
  //  void unload_module();
  //  void delete_module();

  void set_map(projMap *);
  projMap *get_map() const;

  void put_subproject(projNode *);
  projModule *put_module(char const *name, char const *ln);
  projModule *raw_put_module(char const *name, char const *ln, int included = 0);

  void update_xref();
  void update_metrics();
  void updateDFA();
  void force_refresh(int type);
  void refresh();
  void refresh_all();
  void refresh_projects();
  void update_projects(int level);

  // if proj_name is a legal name, it returns 1
  // otherwise it return 0

  int pdf_prt(char const *proj_name, int deep_count);
  void pdf_pr_one_name(char const *node_name, int deep_count, char file_type);

  bool visited();
  void set_visited();
  bool is_script();
  //  int get_pr_node_count(int level, int level_count);
  bool is_paraset_project();

  app *restore_file(char const *fn, char const *ln = 0, projModule *mod = 0);
  app *restore_file_wo_module(char const *ln);
  projModule *make_module(char const *ln);

  static projModule *make_new_module(char const *ln, int is_filename = 0);
  static fsymbolPtr lookup_xrefSymbol_in_cur_proj(ddKind kind, char const *fn);
  symbolPtr lookup_symbol(ddKind kind, char const *fn);

  static projNode *find_project(const Xref *);
  static projNode *find_project(char const *);
  static projNode *find_project(char const *, char const *s, int ln_tp=0 );
  projNode *find_sub_project(char const *);
  static int home_is_writeable();

  void      search_all_modules(symbolArr &s1);
  void      search_nodes_by_name(char const *proj_name, symbolArr &s);
  void      search_modules_by_name(char const *proj_name, symbolArr &s);
  projNode *search_project(char const *proj_name, char const *logic_name);
  //projModule *loaded_module_of_file (char const *file);
  //projNode *loaded_project (char const *proj_name);

  void ln_to_project_name(char const *ln, genString &proj_name);
  void ln_to_project_name(char const *ln, genString &proj_name, int file_type /* =1 -FILE, =0 -DIR */ );

  void update_projnode();
  void update_projnode_for_delete();
  projNode   *root_project();
  projNode   *find_parent() const;
  projModule *find_module(char const *);
  projModule *find_module(char const *, genString &fn);  
  int is_expand_all_level();
  void ln_to_fn(char const *ln, genString &);
  void fn_to_ln(char const *fn, genString &);
  void ln_to_fn_imp(char const *ln, genString &);
  void fn_to_ln_imp(char const *fn, genString &);

  void ln_to_fn(char const *ln, genString &, char file_type);
  void fn_to_ln(char const *fn, genString &, char file_type);
  void ln_to_fn_imp(char const *ln, genString &, char file_type);
  void fn_to_ln_imp(char const *fn, genString &, char file_type);

  void ln_to_fn(char const *ln, genString &, char file_type,
		int xx_f, int script_force_search);
  void fn_to_ln(char const *fn, genString &, char file_type,
		int xx_f, int script_force_search);
  void ln_to_fn_imp(char const *ln, genString &, char file_type,
		    int xx_f, int script_force_search);
  //  void fn_to_ln_imp(char const *fn, genString &, char file_type,
  //	    int xx_f, int script_force_search);

  void ln_to_fn_no_link_check(char const *ln, genString &fn);

  void fn_to_ln_internal(char const *fn, genString &, projNode **);
  void fn_to_ln_internal(char const *fn, genString &, char file_type,
			 projNode **);
  void fn_to_ln_internal(char const *fn, genString &, char file_type,
			 int xx_f, int script_force_search, projNode **);

  //appPtr load_module (char const *ln);
  projContents *get_contents_node() const;
  char const *get_ln() const;

  //  void verify_one_node();

  bool is_visible_proj();
  bool contain_sub_proj();
  bool is_implicit_proj(); // with =

  void print(ostream & = cout, int level = 0) const;
 
  static projNode *get_current_proj();
  static RTL *get_project_rtl();
  static projNode *get_home_proj();
  static projNode *get_home_proj_top_node();
  static projNode *set_home_proj(projNode *);
  static projNode *set_home_proj_temp(projNode *);
  static void update_xref_home_flag(projNode* , projNode* );
  static void set_current_proj (projNode *);
  static void proj_init();
  static void terminate();
  static projNode *get_control_project();
  Xref *get_xref();
  Xref *get_xref(int i);
  static projNode *fn_to_pmod_proj(char const *, genString &ln);
  void set_xref(Xref *);
  void delete_xref();		// delete Xref data member and mark has no Xref
  void xref_file_name(genString &);

  projNode *get_child_project(char const *pn);
  int is_younger(projNode *);

  //  int delete_proj();

  //  void set_has_progs(int flag);

  virtual void rtl_insert(symbolPtr symbol, int);
  virtual void rtl_remove(symbolPtr);	       

  static loadedFiles *loaded_files;
  static loadedFiles *unparsed_files;

  void insert_module_hash(projModule *m);
  void remove_module_hash(projModule *m);
  projModule *search_module_hash(char const *name);

  void insert_dir_hash(char const *dir_path);
  int  search_dir_hash(char const *dir_path);

  //  void recover_module(char const *mod_name);

  void update_all_hash_tables(projMap *pm);
  void update_path_hash_tables(projMap *pm);
  void update_path_hash_tables(projMap *pm, char const *name);

  projModule *create_script_module(char const *ln);
  static int create_global_script_modules(char const *phy_path, fileLanguage lang,
					  int create_new_file);

  bool is_writable();
  void set_writable_bit(int flag);
  projNode *get_pmod_project_from_proj_name(char const *proj_name);
  projNode *get_pmod_project_from_ln(char const *ln);

  virtual int construct_symbolPtr(symbolPtr &) const;
  virtual ddKind get_symbolPtr_kind() const { return DD_PROJECT; }
  virtual int is_symbolPtr_def() const { return 1; }   //analog to get_is_def()
  virtual app *get_symbolPtr_def_app();

private:

  module_hash_entry **module_hash_h;

  static projNode *control_proj;
  static genString control_proj_xref_file;
  static projNode *curr_home_proj;
  static projNode *curr_root_proj;
  static RTL *projects;
    
  genString   name;      // implicit name (left pdf name)
  projMap    *pm;
  Xref       *xref;

  unsigned int has_xref          : 1;
  unsigned int no_refresh_files  : 1;
  unsigned int no_refresh_dirs   : 1;
  unsigned int has_progs_flag    : 1; // project can contain c/c++ programs
  unsigned int subproj_flag      : 2; // indicate if there are any subprojects
  unsigned int subproj_pmod_flag : 2; // indicate if any subproj has a pmod
  unsigned int read_only_flag    : 1;
  unsigned int write_only_flag   : 1;
  unsigned int dummyproj         : 1; // this is a special dummy project, not in a pdf, used by sorted_link

public:

  genString fake_name;   // logical name  (right pdf name)
  fileEntry *pr_name;    // physical name (middle pdf name)
  fileEntry *sym_name;   // .pmod.sym
  fileEntry *link_name;   // .pmod.lnk
  fileEntry *ind_name;   // .pmod.ind
  fileEntry *prj_name;   // .pmod.prj
  
  int pmod_type;

  bool is_dummy_project() { return dummyproj; }

};

inline projNode::projNode(char const *myname, Xref* myxref)
// Dummy constructor for managing temp pmods used by sortlinks.  
// Example use is  -batch -pmoddiff
  :   RTLNode((char *)myname),
  name(myname), pm(0), has_xref(0), no_refresh_files(0), no_refresh_dirs(0),
  has_progs_flag(0), subproj_pmod_flag(0), 
  read_only_flag(0), write_only_flag(0),
  dummyproj(1),              // flag this as a special dummy project
  pr_name(0), sym_name(0), link_name(0), ind_name(0), pmod_type(0)
{
    Initialize(projNode::projNode_char);
    if (myxref) {
	xref = myxref;
	has_xref = 1;
    }
}
generate_descriptor(projNode, RTLNode);

inline void projNode::set_map(projMap* m) { pm = m; }

inline projMap* projNode::get_map() const { return pm; }


class projContents : public appTree
{
  public:
    projContents() {}
    define_relational(projContents, appTree);

    void print(ostream& = cout, int level = 0) const;
};
generate_descriptor(projContents, appTree);


projNode* proj_make_project(char const *name, projNode* parent, char const *dir, 
			    projMap*, char const *ln, int pmod_type);
projModule* proj_make_module(char const *name, projNode* parent, char const *ln);

void proj_generate_subs (projNode*);
defrel_one_to_many (projNode,parentProject,projNode,childProject);
defrel_one_to_many (projNode,projectNode,app,appHeader);

extern projNode *getDefaultProjectNode(char const *, char const *, char const *);
extern projNode* app_get_proj(app* ah);
extern projNode* sym_to_proj(symbolPtr&);

#endif
