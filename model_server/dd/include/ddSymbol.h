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
#ifndef _ddSymbol_h
#define _ddSymbol_h

#include <objOper.h>
#include <symbolPtr.h>

#define MAX_HASH 431

class db_app;
class db_buffer;
class ddElement;
class xrefSymbol;
class ddRoot;

int get_time_of_day ();

class ddNode : public appTree
{
public:
  
 static void db_save(app*h, appTree*node, db_app&, db_buffer&);
 static ddNode* db_restore(app*h, appTree *parent, db_app&, db_buffer&);
 void  db_after_restore(db_buffer&); // optional member on root

  define_relational(ddNode,appTree);
  static Relational* restore(void * rec, app * h, char const *txt = 0);
  
  static void add_changed_ddElems(ddElement *node);
  static void remove_changed_ddElems();
  static objArr *obtain_changed_ddElems();

  void propagate(int,objArr*oa=0);  // virtual for delete
};
generate_descriptor(ddNode,appTree);

class ddSymbol: public ddNode {
public:
    char *name;  // our copy
    char *def;         // a definition string; our copy
    ddKind kind;   
    dd_member_rel_type struct_rel_type;
    int local_index;
    int xref_index;
    unsigned record: 1;
    unsigned enumerated: 1;
    unsigned private_member: 1;   // also set in case of Java "package"
    unsigned protected_member: 1; // also set in case of Java "package"
    unsigned virtual_attr: 1;
    unsigned static_attr: 1;
    unsigned const_attr: 1;
    unsigned mangled_attr: 1;
    unsigned obsolete: 1;
    unsigned expect_friend: 1;    // in case of mutual friend decl
    unsigned pure_virtual_attr: 1;
    unsigned volatile_attr: 1;
    unsigned inline_attr: 1;
    unsigned extern_def: 1;
    unsigned is_def:1;
    unsigned is_old:1;
    unsigned cons_des:1;
    unsigned kr:1;
    unsigned drn:1;
    unsigned from_method:1;  // created to store method
    unsigned native_attr:1;
    unsigned synchronized_attr:1;
  unsigned has_def:1;
  unsigned datatype:4;	// used with DD_SEMTYPE; if kind != DD_SEMTYPE, a value of 1
			// indicates a "cloned" reference ddElement, whose def_file is
			// valid and used to match with the correct xrefSymbol.
  unsigned changed_since_saved:1;
  unsigned changed_since_checked_in:1;
  unsigned file_not_checked_in:1;
  unsigned filler: 2;   // 1 for modified since checked out, 0 for created, 2 no change
                        // these bit should be set to 2 when check out 
  char *aname;  // our copy
  char *def_file; // our copy
  protected:
    symbolPtr dd_sym;

  public:
  define_relational(ddSymbol,ddNode);

    int is_public()const{ return !(private_member || protected_member);}
    int is_private()const{ return private_member && !protected_member;}
    int is_package_prot()const{ return private_member && protected_member;}
    int is_protected()const{ return protected_member && !private_member;}
    int is_virtual()const{ return virtual_attr;}
    int is_pure_virtual()const{ return pure_virtual_attr;}
    int is_static()const{ return static_attr;}
    int is_const()const{ return const_attr;}
    int is_inline()const{ return inline_attr;}
    int is_mangled()const{ return mangled_attr;}
    int is_obsolete()const{return obsolete;}
    int is_expect_friend()const{return expect_friend;}
    int is_synchronized()const{return synchronized_attr;}
    int is_native()const{return native_attr;}
    void set_private(){private_member= 1; protected_member= 0;}
    void set_protected(){protected_member= 1; private_member= 0;}
    void set_package_prot(){private_member= 1; protected_member= 1;}
    void set_public(){private_member= 0; protected_member= 0;}
    void set_virtual(){virtual_attr= 1; static_attr= 0;}
    void set_pure_virtual(){virtual_attr= 1; static_attr= 0;
			  pure_virtual_attr= 1;}
    void set_static(){static_attr= 1; virtual_attr= 0;
			  pure_virtual_attr= 0;}
    void set_ordinary(){static_attr= 0; virtual_attr= 0;
			  pure_virtual_attr= 0;}
    void set_const(){const_attr= 1;}
    void set_inline(){inline_attr= 1;}
    void set_const(bool tf){const_attr= tf;}
    void set_mangled(){mangled_attr= 1;}
    void set_not_mangled(){mangled_attr= 0;}
    void set_obsolete(){obsolete= 1;}
    void set_expect_friend(){expect_friend = 1;}
    void set_not_inline(){inline_attr= 0;}
    void set_synchronized(){synchronized_attr = 1;}
    void set_not_synchronized(){synchronized_attr = 0;}
    void set_native(){native_attr = 1;}
    void set_not_native(){native_attr = 0;}
    void clear_obsolete(){obsolete= 0;}
    void clear_expect_friend(){expect_friend = 0;}
    void set_struct(){record= 1;}
    void set_enum(){enumerated= 1;}
    void set_extern(){ extern_def = 1;}
    int get_extern(){return extern_def;}
    int get_is_def(){return is_def;}
    void set_is_old(){is_old = 1;}
    int get_is_old(){return is_old;}
    char const *get_ddname () const { return name; }
    char const *get_assembler_name () const { return aname;}    // return name
    void disconnect_name(){delete name;}
    int get_name_length()const{return strlen(name);}
    ddKind get_kind()const{return (ddKind)kind;}
    smtLanguage get_language() const {return smtLanguage(language);}
    void set_language(smtLanguage l) {language = l;}
    int is_volatile()const{return volatile_attr;}
    void set_volatile(){volatile_attr= 1;}
    void set_not_volatile(){volatile_attr= 0;}
    void set_cd() { cons_des = 1; }
    int get_cd() { return cons_des;}
    void set_kr() { kr = 1;}
    int get_kr() { return kr;}
    void set_from_method() {from_method = 1;}
    int get_from_method() {return from_method;}
    void set_comp_gen() { enumerated = 1; }
    int is_comp_gen() { return enumerated; }
    void reset_comp_gen() { enumerated = 0; }

    int get_datatype() { return (get_kind() == DD_SEMTYPE || datatype == 1) ? datatype : -1; }
    char const *get_def_file(); 
    char const *get_file_name();
    void set_def_file_with_good_name(char const *fn);

    symbolPtr find_xrefSymbol(); // like get, but doesn't self-insert
    void set_lmd(int lmd) {local_index = lmd; }
    int get_lmd() { return local_index;}
    void set_sw_tracking(int ii);
    unsigned int get_sw_tracking();
    
    symbolPtr get_xrefSymbol();
  protected:
    xrefSymbol* find_xrefSymbol_priv();
    xrefSymbol* get_xrefSymbol_priv(); 
};

generate_descriptor(ddSymbol,ddNode);

defrel_one_to_one(app,appHeader,appTree,dd);

int dd_can_have_ref_file(ddKind kind);
//int dd_can_be_subsys_member(ddKind kind);
int dd_can_be_selected(ddKind kind);
//int dd_can_be_defined_in_module(ddKind kind);
int dd_has_type(ddKind k);
int ddKind_is_local(ddKind k);
void fill_with_all_not_internal_dds(ddSelector& ddsel);
//int dd_can_be_associated(ddKind kind);
//int dd_can_be_linked(ddKind kind);
int dd_is_function_declaration(ddKind kind);
int dd_is_type_declaration(ddKind kind);
int dd_is_var_declaration(ddKind kind);
int dd_has_smt_direct(ddKind kind);
int dd_has_smt_via_main_header(ddKind kind);
//int dd_has_data(ddKind kind);
//int dd_has_function(ddKind kind);
//int dd_is_class(ddKind kind);
int dd_is_not_internal_object(ddKind kind);
//int dd_can_be_affected_by_prop(ddKind kind);
int dd_is_nested_type(ddKind kind);
int dd_notify_xref_if_modified(ddKind kind);
int dd_is_SWT_entity(ddKind kind);

#endif




