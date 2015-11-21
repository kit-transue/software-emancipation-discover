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
#ifndef _customize_h
#define _customize_h

// customize.h
//------------------------------------------
// synopsis:
// Storage, access, and interface for user-customizable
// portions of application.
//
//------------------------------------------


#ifndef _genString_h
#include <genString.h>
#endif      
#ifndef _genArr_h
#include <genArr.h>
#endif
#ifndef _customize_util_h
#include <customize_util.h>
#endif


class prefRegistry;



typedef struct { char *key;  char *env_var; } Envpair;	



// There is exactly one instance of class customize and
// it is completely invisible to the outside world. 
// All access to customization features is through class
// static functions.

class customize
{
  public:
    static customize* instance;

    // Editor Mode enumeration.
    enum EditorMode { EMACS_EDITOR_MODE, VI_EDITOR_MODE };

    // String preference expansion enumeration.
    enum expandString { EXPAND_NONE, EXPAND_META, EXPAND_COLONS, UNEXPAND_COLONS };



    // Dictionary get methods.
    static       bool       getBoolPref(const char* key);
    static const int&       getIntPref(const char* key);
    static const genString& getStrPref(const char* key,
                                       const int expansion=EXPAND_META);

    // Dictionary put methods.
    static bool putPref(const char* key, const int      value,
                        const bool persistent=true);
    static bool putPref(const char* key, const genString value,
                        const int  expand=EXPAND_META,
                        const bool persistent=true);

    // Utility to convert customize expand type into prefRegistry expand type.
    static int convertExpansion(const int expansion);


    ////////////////////////////////////////////////////////////////////

    // Pathname to root of product installation,
    // under which you can find: lib, bin, epoch, etc.
    static const char* install_root();

    // Functions to set and get the write_flag -- to specify whether xref is writable or not !!

    static void set_write_flag(int flag);
    static int  get_write_flag();


    // Default project definition file:
    static const char* default_project_definition();
    static void        default_project_definition(const char*);

    static const char* subsys_setting_file(int write_flag);
    static void subsys_setting_file(const char* string);

// This section is in the configurator dialog box
    // Pathname to configurator executable
    static const char* configurator();
    static void        configurator(const char*);

    // configurator_diff text
    static const char* configurator_diff();
    static void        configurator_diff(const char*);

    // configurator_stat text
    static const char* configurator_stat();
    static void        configurator_stat(const char*);
    // configurator_get text
    static const char* configurator_get();
    static void        configurator_get(const char*);

    // configurator_copy text
    static const char* configurator_copy();
    static void        configurator_copy(const char*);

    // configurator_put text
    static const char* configurator_put();
    static void        configurator_put(const char*);

    // configurator_lock text
    static const char* configurator_lock();
    static void        configurator_lock(const char*);

    // configurator_unlock text
    static const char* configurator_unlock();
    static void        configurator_unlock(const char*);

    // configurator_unget text
    static const char* configurator_unget();

    // configurator_obsolete text
    static const char* configurator_obsolete();

    // configurator_create a new file
    static const char* configurator_create();

    // configurator_delete text (menu pick  File->Delete)
    static const char* configurator_delete();
    static void        configurator_delete(const char*);

    // configurator_terse text (bool - should we suppress use of configshell window)
    static int         configurator_terse();
    static void        configurator_terse(const int);

    // True if we want to track bugs with the put dialog
    static int         put_track_bugs();
    static void        put_track_bugs(const int);

    // say yes if no cm
    static bool        no_cm();

    // Type of configuration management system being used
    static const char* configurator_system();
    static void        configurator_system(const char*);
    
    // Overide for custom scripts directory
    static const char* configurator_custom_dir();
    static void        configurator_custom_dir(const char*);
    
    // Remove previous version of pset when putting new one
    static int         configurator_rm_psets();
    static void        configurator_rm_psets(const int);

    // Use fast put & merge versions 
    static int         configurator_fast_putmerge();
    static void        configurator_fast_putmerge(const int);

    // Lock files while getting them
    static int         configurator_use_locks();
    static void        configurator_use_locks(const int);

    // Ask for comments before "Get"ting file(s).
    static int         configurator_get_comments();
    static void        configurator_get_comments(const int);
    
    // Lock files while putting them
    static int         configurator_put_locks();
    static void        configurator_put_locks(const int);
    
    // Create softlinks for new directories
    static int         configurator_use_softlinks();
    static void        configurator_use_softlinks(const int);
    
    // Optional scripts directory for extra user defined configurator scripts
    static const char* configurator_optscripts_dir();
    static void        configurator_optscripts_dir(const char*);
    
    // Backup files before putting them
    static int         configurator_do_backups();
    static void        configurator_do_backups(int);

    // Pathname to gdb executable
    static const char* gdb();
    static void        gdb(const char*);

    // Pathname to make executable
    static const char* make();
    static void        make(const char*);

    // Pathname to paraMaker executable
    static const char* paraMaker();
    static void        paraMaker(const char*);

    // Pathname to cpp executable
    static const char* cpp();
    static void        cpp(const char*);

    // Pathname of esql executable
    static const char* esql();
    static void        esql(const char*);

    // Relative pathname to search in for Makefiles
    static const char* makef_dir();
    static void makef_dir(const char*);

    // Make targets 
    static const char* makefile_targets();
    static void makefile_targets(const char* targets);

    ////////////////////////////////////////////////////////////////////

    // Default C++ compiler flags
    static const char* cplusplus_flags();
    static void        cplusplus_flags(const char*);

    // Default C++ compiler name
    static const char* cplusplus_compiler();
    static void        cplusplus_compiler(const char*);

    // C++ esql preprocessor name
    static const char* cplusplus_esql_pp();
    static void        cplusplus_esql_pp(const char*);

    // Default C++ compiler pre-processor defines
    static const char* cplusplus_defines();
    static void        cplusplus_defines(const char*);

    // Default C++ compiler include path
    static const char* cplusplus_includes();
    static void        cplusplus_includes(const char*);

    // Default C++ esql preprocessor arguments
    static const char* cplusplus_esql_args();
    static void        cplusplus_esql_args(const char*);

    // Default C++ file suffixes
    static const char* cplusplus_default_src_suffix();
    static void        cplusplus_default_src_suffix(const char*);

    static const char* cplusplus_default_hdr_suffix();
    static void        cplusplus_default_hdr_suffix(const char*);

    // C++ file suffixes
    static const char* cplusplus_suffix();
    static void        cplusplus_suffix(const char*);

    static const char* cplusplus_hdr_suffix();
    static void        cplusplus_hdr_suffix(const char*);

    // C++ esql file suffixes
    static const char* cplusplus_esql_suffix();
    static void        cplusplus_esql_suffix(const char*);
    
    ////////////////////////////////////////////////////////////////////

    // Default C compiler name
    static const char* c_compiler();
    static void        c_compiler(const char* );

    // C esql preprocessor name
    static const char* c_esql_pp();
    static void        c_esql_pp(const char*);

    // Default C compiler flags
    static const char* c_flags();
    static void        c_flags(const char* );

    // Default C compiler pre-processor defines
    static const char* c_defines();
    static void        c_defines(const char*);

    // Default C compiler include path
    static const char* c_includes();
    static void        c_includes(const char*);

    // Default C esql preprocessor arguments
    static const char* c_esql_args();
    static void        c_esql_args(const char*);

    // Default C file suffixes
    static const char* c_default_src_suffix();
    static void        c_default_src_suffix(const char*);

    static const char* c_default_hdr_suffix();
    static void        c_default_hdr_suffix(const char*);

    // C file suffixes
    static const char* c_suffix();
    static void        c_suffix(const char*);

    static const char* c_hdr_suffix();
    static void        c_hdr_suffix(const char*);

    // C esql file suffixes
    static const char* c_esql_suffix();
    static void        c_esql_suffix(const char*);

    // C style
    static int         c_style();
    static void        c_style(int);

    // shell user
    static const char* user();
    static void        user(const char*);

    // shell user's Real name
    static const char* user_real_name();
    static void        user_real_name(const char*);

    // shell date & time 
    static const char* date();
    

    // shell home directory
    static const char* home();
    static void        home(const char*);

    // shell host
    static const char* host();
    static void        host(const char*);

    // batch mode
    static int	       batch_mode();
    static void	       batch_mode(int);

    // RTL file stats
    static int	       rtl_file_stats();
    static void        rtl_file_stats(int);
    
    static int	       forgive();
    static void        forgive(int);
    
    // Mini Browser
    static int         miniBrowser_history_length();
    static void        miniBrowser_history_length(int);
    static int         miniBrowser_show_titles();
    static void        miniBrowser_show_titles(int);

    // Printer
    static const char* printer();
    static void        printer(const char*);

    // C Parser name and path
    static const char* c_parser();
    static void        c_parser(const char*);

    // CP Parser name and path
    static const char* cp_parser();
    static void        cp_parser(const char*);


    ////////////////////////////////////////////////////////////////////

    // Text editor mode
    static int         editorMode();
    static void        editorMode(int);

    // ParaDOC name
    static const char* paraDoc_name();
    static void        paraDoc_name(const char*);

    // ParaDOC CommanDLine
    static const char* paraDoc_commandLine();
    static void        paraDoc_commandLine(const char*);

    // Default Editor startup string
    static const char* editor();
    static void        editor(const char*);


    ////////////////////////////////////////////////////////////////////

    // True if a particular language is enabled.
    static int language_enabled(unsigned int smt_language);
    static void language_enabled(unsigned int smt_language, int);
    static int handle_language_enabled(unsigned int smt_language, int, int);


    // Files under these directories should not generate
    // entries in the cross-reference database.
    // See xref_is_excluded_file().
    static const char* xrefExcludedDirs();
    static void  xrefExcludedDirs(const char*);
   
 // Preference file routines
    static void load_preferences(const char* project = NULL);
    static void save_preferences(const char* project = NULL);
    static void save_configurator(const char* project = NULL);
    static void save_buttonbar(const char* project = NULL);
    static void save_database(const char* project = NULL);
    static void save_boilerplate(const char* project = NULL);
    static void save_specific_boilerplate(int i,genString save_string,const char* project=NULL);

    ////////////////////////////////////////////////////////////////////
    // Button bar save and restore.

    // STE view button bar
    static const char* ste_buttonbar();
    static void        ste_buttonbar(const char*);

    // SMT view button bar
    static const char* smt_buttonbar();
    static void        smt_buttonbar(const char*);

    // CallTree view button bar
    static const char* calltree_buttonbar();
    static void        calltree_buttonbar(const char*);

    // Class view button bar
    static const char* class_buttonbar();
    static void        class_buttonbar(const char*);

    // ERD view button bar
    static const char* erd_buttonbar();
    static void        erd_buttonbar(const char*);

    // SubsysBrowser view button bar
    static const char* subsysBrowser_buttonbar();
    static void        subsysBrowser_buttonbar(const char*);

    // SubsysMap view button bar
    static const char* subsysMap_buttonbar();
    static void        subsysMap_buttonbar(const char*);

    // DC view button bar
    static const char* dc_buttonbar();
    static void        dc_buttonbar(const char*);

    // Whether or not the button bar was saved as visible.
    static int  button_bar_visible();
    static void button_bar_visible(int);


///////////////////////////////////////////////////////////////////
//Boilerplate save and retreive functions

    static const char* func_init();
    static void func_init(const char* string);

    static const char* func_final();
    static void func_final(const char* string);

    static const char* before_func_decl();
    static void before_func_decl(const char* string);

    static const char* after_func_decl();
    static void after_func_decl(const char* string);

    static const char* before_func_def();
    static void before_func_def(const char* string);

    static const char* after_func_def();
    static void after_func_def(const char* string);

    static const char* pub_decl();
    static void pub_decl(const char* string);

    static const char* prot_decl();
    static void prot_decl(const char* string);

    static const char* priv_decl();
    static void priv_decl(const char* string);

    static const char* before_class_decl();
    static void before_class_decl(const char* string);

    static const char* after_class_decl();
    static void after_class_decl(const char* string);

    static const char* before_class_def();
    static void before_class_def(const char* string);

    static const char* after_class_def();
    static void after_class_def(const char* string);

    static const char* before_struct();
    static void before_struct(const char* string);

    static const char* after_struct();
    static void after_struct(const char* string);

    static const char* member_def_init();
    static void member_def_init(const char* string);

    static const char* member_def_final();
    static void member_def_final(const char* string);

    static const char* before_member_def();
    static void before_member_def(const char* string);

    static const char* after_member_def();
    static void after_member_def(const char* string);

    static const char* before_member_decl();
    static void before_member_decl(const char* string);

    static const char* after_member_decl();
    static void after_member_decl(const char* string);

    static const char* relation_src_member();
    static void relation_src_member(const char* string);

    static const char* relation_src_header();
    static void relation_src_header(const char* string);
    
    static const char* relation_src_definition();
    static void relation_src_definition(const char* string);

    static const char* relation_trg_member();
    static void relation_trg_member(const char* string);

    static const char* relation_trg_header();
    static void relation_trg_header(const char* string);
    
    static const char* relation_trg_definition();
    static void relation_trg_definition(const char* string);

    static const char* c_src_file_top();
    static void c_src_file_top(const char* string);

    static const char* c_src_file_bottom();
    static void c_src_file_bottom(const char* string);

    static const char* c_hdr_file_top();
    static void c_hdr_file_top(const char* string);

    static const char* c_hdr_file_bottom();
    static void c_hdr_file_bottom(const char* string);

    static const char* cpp_src_file_top();
    static void cpp_src_file_top(const char* string);

    static const char* cpp_src_file_bottom();
    static void cpp_src_file_bottom(const char* string);

    static const char* cpp_hdr_file_top();
    static void cpp_hdr_file_top(const char* string);

    static const char* cpp_hdr_file_bottom();
    static void cpp_hdr_file_bottom(const char* string);

///////////////////////////////////////////////////////////////////

    customize();
    ~customize();

///////////////////////////////////////////////////////////////////

    static bool is_init();  // To return if customize is initialized.

  private:
    static prefRegistry* dicptr;

    static void set_multiple_psets(); 
    static const char *logfile_dir(int, const char *);
    static const char *handle_user_defined_segfile(int , const char *);

    static bool init_flag;
};



#endif // _customize_h
