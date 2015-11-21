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
#ifndef _Xref_h
#define _Xref_h

#include <genError.h>
#include <Relational.h>
#include <ddKind.h>
#include <ddSymbol.h>
#include <objArr.h>
#include <ddSelector.h>
#include <scopeNode.h>
 
class xrefSymbol;
class linkTypes;
class symbolArr;
class symbolSet;
RelClass(RTL);
RelClass(RTLNode);
RelClass(ddElement);
class Application;

class Xref : public scopeNode
{
    friend Xref* get_XREF(char const *);
    friend class CPManager;
    friend class SharedXref;
    friend class XrefTable;
    friend class XrefTable_remote;
    friend class XrefTable_temp;
    friend class ddElement;
    friend class propManager;
    friend class sorted_link;
    friend class symbolPtr;
    friend class testpmod;
    friend class xrefEntry;
    friend class xrefSymbol;
    friend ddElementPtr dd_find_definition_from_sym (ddKind kind, char const *name);
    friend ddElementPtr new_dd_find_definition (ddKind kind, char const *name);
    friend ddElementPtr new_dd_find_loaded_definition (ddKind kind, char const *name);
    friend int attach_Xref_remote(projNode*, Application*);
    friend int lookup_all_file_symbol(symbolArr& res, char const *fn);
    friend int xref_main(int argc, char const **argv, char const **envp);
    friend projNode* dd_find_proj_which_has_loaded_def_of (ddKind kind, char const *name);
    friend symbolPtr Xref_insert_symbol(ddSymbol* dd, app* h);
    friend symbolPtr lookup_file_symbol(char const *fn);
    friend symbolPtr lookup_file_symbol(ddKind kind, char const *fn);
    friend symbolPtr lookup_file_symbol_in_xref(char const *fn, Xref* xr);
    friend symbolPtr lookup_xrefSymbol(ddKind kind, char const *fn);
    friend void  ddNode::db_after_restore (db_buffer&);
    friend void XREF_insert_module(app* ah); // for building xref
    //friend void XREF_insert_module(app* ah, int );
    friend void XREF_set_last_modified_date (app* sh);
    friend void XREF_tidy_file(app* ah);
    friend void dd_get_symbols_wild(ddKind type, char const *wild, symbolArr &sym);
    friend void dd_merge_xref (int count, char const **src_projs, char const *projname);
    friend void dd_print_stat(ddKind dkind, char const *option_value,char const *projname );
    friend void dd_tidy_xref (char const *projname);
    friend void lookup(projNode*, int, char const *, char const *, symbolArr&);
    friend void merge_XREF();
    friend void new_xref_init_rtl(RTLNodePtr rtl_node, const ddSelector& selector);
    friend void print_XREF();
    friend void print_XREF(Xref* Xr);
    friend xrefSymbol* Xref_file_is_newer_priv(char const *ln, char const *fn);
    friend xrefSymbol* Xref_insert_symbol_priv(ddKind k, char const *ns);
    friend xrefSymbol* Xref_insert_symbol_priv(ddSymbol* dd, app* h);
    friend xrefSymbol* lookup_file_symbol_priv(ddKind kind, char const *fn);
    friend projNode* lookup_all_proj_for_symbol(xrefSymbol* sym);
 public:     // public member functions of class Xref
    ~Xref();
    Xref(char const *xref_file, int perm, projNode* pr);
    define_relational(Xref,scopeNode);
    symbolPtr lookup_module(char const *file_name);
    void remove_module(char const *module_name);
    void save_lxref_by_module(char const *filename); // save filename's changes to local Xref
    void save_lxref_by_whole(); // save whole xref, used for merge
    void remove_module_from_xref(char const *filename); // delete filename from local Xref
    void to_write();
    static void ind_file (char const *symf, genString& i_file);
    static void sym_file (char const *symf, genString& s_file);
    static void link_file (char const *, genString& l_file);
    bool is_writable () { return writable_on_disk == 0; }
    bool is_project_writable ();
    void set_home_flag() { home_flag = 1; }
    void reset_home_flag() { home_flag = 0; }
    unsigned char get_home_flag() { return home_flag; }
    bool has_lxref() { return (lxref!=0); }
    void insert_module_in_lxref(app*head);
    //  if there isn't enough room, move the xref table now
    bool make_room(int size);
    symbolPtr find_symbol(symbolArr& res, ddKind k, char const *name, char const *file_name = 0)
    {  return lookup(res, k, name, file_name);}
    symbolPtr lookup_in_lxref(symbolArr& arr, ddKind kind, char const *name);
    unsigned int max_offset() const;

    void make_real(char const *name);     // create an xref for a root project which previously was unreal
    bool is_unreal_file();
    void add_symbols(symbolArr& arr, const ddSelector& selector);   // add symbols corresponding to this selector
    static int  test_subproj_pmod(projNode* pn);  //figure out if subprojs have pmod
    projNode* get_projNode() const { return my_proj; }
    static void symbols_get_links(symbolArr & syms, objSet & prs, linkTypes &la,
				  symbolArr &output);
    static void remove_all_symptrs_from_xrefTable(const ObjPtr proj);  // remove all symbolPtrs in the xrefEntry structs
    static void xref_notifier_report_proj(const int action, const ObjPtr ob);  // call xref_notifier_report for all syms in proj
    char get_perm_flag();
    void register_file_for_crash_recovery(char const *filename);
    void crash_recovery_pmod_from_pset();
    void set_crash_recovery(int i) {crash_recovery = i;}
    int get_crash_recovery() {return crash_recovery;}
    int need_to_update(char const *, char const *);
    static int find_all_syms(ddSelector &, symbolSet &);
    XrefTable *get_lxref (int=0);
    void insert_SWT_Entity_Status(app*);
    void init(int perm);
    virtual objTree *  get_root() const;

    xrefSymbol *lookup(symbolArr &res, ddKind k, char const *name,
                       char const *file_name = 0);
    // lookup only in the one pmod 
    symbolPtr lookup_local(symbolArr &res, ddKind k, char const *name,
                           char const *file_name = 0);

 private:
    int get_links_global(xrefSymbol* sym, linkTypes &lt, symbolArr& as, xrefSymbol* dfs);
    static void get_xreflist_from_proj(objArr& arr, projNode* pn);

    XrefTable *get_which_xref(app* ) { return lxref; }
    void tidy_xref();
    void merge (Xref *dst);
  
    // still private
 private:

#if 0
    unsigned int signature;	// signature & placeholder, remove when
    // we derive it from Relational, and it gets a virtual function or 3
    unsigned kind:8;		// to be able to recognize it in a symbolPtr
#endif
    unsigned char perm_flag:1;
    unsigned char unreal_flag:1;   // fake sym file, not to be written to or saved (use subprojects instead)
    unsigned char is_root:1;	 // true if this xref belongs to a root project.
    unsigned char crash_recovery:1;	 // true if this xref already perform crash rec
    unsigned char filler:4;    // to pad it to a byte
    unsigned char writable_on_disk;
    unsigned char home_flag;
    XrefTable *lxref;
    objArr     gxref_arr;
    //xrefTable*    xref_table;    // hash table containing all loaded def dd
    static int mode; // one for global, 0 for local
    genString xref_name;
    int merged_xref;
  
    projNode* my_proj;
    //  internal versions for greater speed
#ifdef _WIN32
 protected:
#endif
    xrefSymbol* lookup_module_priv(char const *file_name);

 protected:
    // special do-nothing constructor, for use by SharedXref

    Xref(projNode* proj) {my_proj=proj;};
};
generate_descriptor(Xref,scopeNode);

extern Xref * XREF;
extern Xref * last_Xref;
#endif
