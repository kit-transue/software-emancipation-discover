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
#ifndef _app_h
#define _app_h

#include <objCommon.h>


class projModule;
class projNode;
class Xref;
class db_buffer;
class objArr;

class app;
class appTree;
void obj_remove(app*, appTree*, int = 0);
class objPropagator;
class objCopier;

class app : public Hierarchical
{
 public:
  app(); 
  app(char const *title);
  app(char const *title, char const *file_name);

  void db_after_restore();  // new db interface

  app(char const *log_name, appType type, smtLanguage language);
  app(char const *title, char const *log_name, appType type, smtLanguage language);
  void set_filename (char const *ln);
  void set_filename (char const *ln, appType type, smtLanguage language);
  char const *get_filename();

  void set_phys_name (char const *);
  void set_phys_name (char const *, projNode *);
  char const *get_phys_name ();

  projModule *get_module();  // Get associated module
  projNode *  get_project(); // Get associated project
  Xref *      get_xref();    // Get associated xref
  projNode *  make_current();// Make associated project the writable 
				      //project
  projModule *to_write();    // If the app is read only, "get" it and 
				      // change associated project/module/xref
   
  int get_language();
  ~app();
  define_relational(app,Hierarchical);

  virtual void description(genString&);

  virtual void hilite_obj(objHiliter *oh, objHiliter *nh);
  virtual void focus_obj(objFocuser *of, objFocuser *nf);
  virtual void assign_obj(objAssigner  *oc, objAssigner *nc);
  virtual int  regenerate_obj(objTree*);

  static objArr *get_app_list();
  static objArr *get_new_app_list();
  static app * get_header(appType, char const *name);
  static app * find_from_phys_name(char const *name);
  static objArr *get_modified_headers();
  static objArr *get_modified_ext_headers(int);

  virtual void notify_softassoc_insert(objInserter*, objInserter *);

  virtual void notify_softassoc_remove(objRemover *, objRemover *);
    
  virtual void notify_softassoc_merge(objMerger *, objMerger *);
    
  virtual void notify_softassoc_split(objSplitter *, objSplitter *);
    
  boolean needs_to_be_saved();
  virtual boolean is_scratch();

  virtual int construct_symbolPtr(symbolPtr&) const;
  virtual ddKind get_symbolPtr_kind() const;
  virtual int is_symbolPtr_def () const;         //analog to get_is_def()
  virtual app *get_symbolPtr_def_app ();

 private:
  static objArr *app_list;
  static objArr *new_app_list;
  void append_to_list();
};
generate_descriptor(app,Hierarchical);

//class ldrTree;
//declare_rel(ldrTree,ldrtree,appTree,apptree);

class appTree : public commonTree
{
    //  define_ptr(appTree,apptree,ldrTree,ldrtree);

  public:
    void db_after_restore(db_buffer&);  // new db interface
    appTree();
    ~appTree();
    define_relational(appTree,commonTree);

    virtual void description(genString&);

    // makes copy of appTree for given header
    virtual appTree *rel_copy_for_header(app*, class objCopier *cp);
    //    virtual class ldr *make_ref_ldr ();// { return NULL;} // dead code. Do not use. -jef
                                                   // left here to preserve virtual function table order.

    app *get_header() const { return (app*)(commonTree::get_header()); }

    //    objPropagator *accept(objPropagator *, RelType *);
    virtual appTree *app_to_ste(); // { return NULL;}

// Returns the the node to open when open_view for this is called
    virtual appTree  *get_world (); // {return this;}
    virtual app  *get_main_header();
    void report_remove(int);	// Report "remove" operation to 
				// appMarker subsystem
    void report_split(int, appTree *, appTree *);// Report "split" 
    virtual int construct_symbolPtr (symbolPtr & ) const;
    virtual int get_linenum () const;
};
generate_descriptor(appTree,commonTree);


#endif
