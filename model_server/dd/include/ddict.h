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
#ifndef _ddict_h
#define _ddict_h

#include <objString.h>

#include <objTree.h>

#include <smt.h>

#include <objOper.h>

RelClass(RTLNode);
#include <ddKind.h>

#include <symbolArr.h>

class astTree;
class astDecl;
class astFunction_decl;

#include <representation.h>

#include "Set.h"//#include <Notifier.h>
#include "objRef.h"
#include <ddSymbol.h>

#define BUFFER_SIZE 1024
#define XREF_MAX 431
#define ddPAGE_SPACE 1000
#define ddMIN_STRING_SPACE 2

//class myTreeObj;
// Let class A {int x; friend class B}; then A has friend B and
// B is friend_of A
define_relation(has_friend, friend_of);

// class B; B* f(), then B has struct_ref_of_decl to f ...
define_relation(struct_ref_of_decl, struct_decl_of_ref);

define_relation(def_smt_of_dd, def_dd_of_smt);
define_relation(smt_of_dd, dd_of_smt);
define_relation(ref_smt_of_dd, ref_dd_of_smt);
define_relation(decl_smt_of_dd, decl_dd_of_smt);
define_relation(localHash_of_dd, dd_of_localHash);
define_relation(ref_app_of_dd, ref_dd_of_app);

define_relation(macro_of_ddRoot, ddRoot_of_macro);
define_relation(fdecl_of_ddRoot, ddRoot_of_fdecl);
define_relation(vdecl_of_ddRoot, ddRoot_of_vdecl);
define_relation(tdecl_of_ddRoot, ddRoot_of_tdecl);
defrel_many_to_many(appTree,user,appTree,used);
define_relation(cdtree_of_dd, dd_of_cdtree);

define_relation(ddRoot_of_smtHeader, smtHeader_of_ddRoot);

define_relation(superclassof,subclassof);
define_relation(vpub_baseclassof,vpub_subclassof);
define_relation(pub_baseclassof,pub_subclassof);
define_relation(vpri_baseclassof,vpri_subclassof);
define_relation(pri_baseclassof,pri_subclassof);
define_relation(vpro_baseclassof,vpro_subclassof);
define_relation(pro_baseclassof,pro_subclassof);
define_relation(smt_to_dd,dd_to_smt);
define_relation(smt_header_to_dd_header,dd_header_to_smt_header);
define_relation(smt_header_to_dd,dd_to_smt_header);
define_relation(nameof,definitionof);
define_relation(names,table_a);
define_relation(hidden_names,table_h);
define_relation(files,table_s);
define_relation(defined_in,elements_of);
define_relation(smt_tok_of,dd_elt_of);
define_relation(template_of_instance, instance_of_template);
define_relation(property_of_possessor, possessor_of_property);
// -- NESTED:

class xrefSymbol;
typedef  symbolArr symbolArr;
#ifdef XXX_ddString //class ddString {
  short allocated_length; // allocated space for memory management
  short length;           // length of the string in this space
  union {
    char info[ddMIN_STRING_SPACE]; // the string
    short next_offset;             // offset of next free string
    };
  public:
  ddString(); //{}
    int len(){return length;}
    char const *string(){return info;}
    void set_length( int len){length= len;}
    void print( ostream& str = cout, int = 0 )
                                      {  str << info;}
};
#endif
class hash_element;
class ddRoot;
class ddElement;
class ddToken;

Relational * Erestore(void * , app *, char const *);
Relational * Rrestore(void * , app *, char const *);
void restore_ptr(app* );
void dd_restore_ptr(ddElement* );
void dd_restore_list(ddElement* );


class db_buffer;
class db_app;

extern int ddPage_next_page_id;
#ifdef XXX_ddStrPage
class ddStrPage {
  private:
    ddStrPage *next;  // last allocated page
    int page_id;       // a buffer id to be used on disk
    int next_byte;     // next available byte
    int free_list;     // link list of space in this buffer
    char buffer[ddPAGE_SPACE];  // the actual memory used
  public:
    ddStrPage(){ page_id= ddPage_next_page_id++; next_byte= 0;}
    int PAGE_ID(){return page_id;}
    ddString *memory( int ){ return 0;}
    void set_NEXT( ddStrPage *p){ next= p;}
    //    friend class ddTable;
#endif //  };
//class ddTable;
#ifdef XXX_ddToken
class ddToken: public Relational {
  private:
    ddToken *nextofhash;       // another token with the same hash
    ddString *image;           // the spelling of this token
    int  token_id;             // Maybe an id for disk
    unsigned scope_head: 1;    // indicates a file name
    unsigned name_head: 1;     // indicates a definition name
    unsigned hidden_name: 1;   // indicates a field or parameter name
    unsigned pointers: 1;      // references are pointers
    unsigned ids: 1;           // references are by buffer id and offset
    unsigned old_file: 1;      // existed before current ddbuild analysis
    unsigned obsoletes: 1;     // this file may contain obsolete decls
    unsigned filler: 25;       // space to be used later (maintain 0 mod 32)
    ddToken();
  public:
    ~ddToken(){}
    define_relational(ddToken,Relational);
    void send_string(ostream &)const{}
    void Name( ddString *sp){ image= sp;}
    ddString *IMAGE(){ return image;}
    int LENGTH(){ return image->len();}
    char const *SPELLING()const{ return image->string();}
    ddToken *NEXT_SAME_HASH(){ return nextofhash;}
    virtual void print(ostream& = cout, int = 0)const{}
    virtual char const *get_name () const{return SPELLING();}
    int is_file(){return scope_head;}
    void set_old(){old_file= 1;}
    void set_new(){old_file= 0;}
    int is_old(){return old_file;}
    appPtr get_header() const;
    int marked_obsolete(){return obsoletes;}
    void clear_obsoletes(){obsoletes= 0;}
    void contains_obsoletes(){obsoletes= 1;}
    //    friend class ddTable;
  };


generate_descriptor(ddToken,Relational);
#endif
RelClass(ddElement);
RelClass(projModule);

class ddElement : public ddSymbol {
  // only save above this.

    char *sname;
    char *sdef;
    char const *old_name;
    ddElement *last_field;
    smtTreePtr  tid;                     // token id of the name
    hash_element* local_hash;
    app* xref_hash;
    astTree*   ast_node; 

  public:
  int get_related_tokens(objSet & os);
    friend class ddRoot;
    ddElement(ddKind k = DD_UNKNOWN, char const *ns="", int nl=1, 
	      char const *ds="", int dl=1, char const *as = "", int al = 1);
    ddElement(char const *name, appTree* node, int xref_type, ddKind k);
    ddElement(char const *name, appTree* node, int xref_type, ddKind k, RelType* rl,
	      char const *as = 0);	
    ddElement(char const *name, app* node, int xref_type, ddKind k, RelType* rl,
	      char const *as = 0);	
    // Next ctor "clones" reference to existing ddElement
    ddElement(ddElementPtr, appTree* node);
    // Next ctor "clones" reference to xrefSymbol
    ddElement(symbolPtr, appTree* node);
    ddElement( const ddElement& );
    define_relational(ddElement,ddSymbol);
    declare_copy_member(ddElement);

//
// definition of this function moved to ddict.h.C. The reason is that is has
// to set is_def field in xrefEntry object at the same time it sets it in ddElement.
// This addition makes it bigger and therefore unsuitable for inline.
//
    void set_is_def();
  void unset_is_def();
    HierarchicalPtr get_header() const; // return smtHeader
    virtual app* get_main_header();
    smtHeader* get_smt_header();
    void send_string(ostream &stream)const;
    virtual char const *get_name () const;
    virtual void set_name (char const *sp);
    void set_assembler_name (char const *sp);
    dd_member_rel_type get_struct_rel_type()const{return (dd_member_rel_type)struct_rel_type;}
    void set_struct_rel_type(dd_member_rel_type t){struct_rel_type = t;}
    int get_def_length()const;
    char const *get_def()const;
    char const *find_def()const;
    void set_def(char const *sp);
    void set_tid(smtTreePtr token_id){tid = (smtTreePtr) token_id;}
    smtTreePtr dd_get_tid(){return ((smtTreePtr)tid) ;}
    void print( ostream& str = cout, int level = 0)const;
    void set_ast( astTree* ob){ast_node= ob;}
    astTree* get_ast()const{return ast_node;}
    void regenerate_name();
    ddElement *get_field(int index);
    ddElement *get_last_field()const {return last_field;}
    void set_last_field(ddElement* ep) { last_field = ep;}
    void set_att(astDecl* decl, ddElement* ep= NULL, int i = 0); 
    void set_local_hash(hash_element* h) { local_hash = h;}
    hash_element* get_local_hash() { return local_hash;}
    ddRoot* get_dd_root()const;

    ~ddElement();
    void set_sname(char const *s);
    char const *get_sname(){return sname;}
    void set_sdef(char const *s);
    char const *get_sdef() {return sdef;}
    void set_drn() { drn = 1;}
    int get_drn() { return drn;}
    void unset_drn() { drn = 0; }
    ddElement* get_child(char const *ns, ddKind k);
    void set_def_file(char const *fn);
    void set_xrefSymbol(xrefSymbol* sy);
    bool is_changed_since_saved();
    bool is_changed_since_checked_in();
    bool is_file_not_checked_in();
    void set_changed_since_saved(int flag);
    void set_changed_since_checked_in(int flag);
    void set_file_not_checked_in(int flag);
  symbolArr* get_files(); // get all file_syms, user delete result later. 
  Obj* get_loaded_refs() ; // get all loaded refs (ddElements)
  Obj* get_loaded_dds() ;  // get all loaded  (ddElements), icluding this
  ddElement* get_loaded_definition(); // returns 0 if not loaded/exists

  Obj* get_refs() ; // get all refs of this dd, load if needed, no checkout
  Obj* get_dds() ;  // get all  (ddElements), icluding this
  ddElement* get_definition();
  symbolPtr get_definition_file();
  virtual void notify(int, Relational*, objPropagator*, RelType*);	
  virtual appTreePtr rel_copy_for_header(appPtr, objCopier *);

  int set_dec_cardObj(Obj* o, appTree* at);
  int set_inc_cardObj(Obj* o, appTree* at);
  
  // Following three functions expect caller to provide an objArr. Returned
  // information is appended to the objArr, which is passed by reference (&)

  // Given dd node for class, append list of its direct dd childrento arg ddArr
  void children_of_class(objArr& ddArr);

  // Given dd element for class, append list of its methods to arg ddArr
  void func_members_of_class(objArr& ddArr);


  // this function implements the inverse operation of member_get_class()

  // Given dd element for class, append list of fields and methods to arg ddArr
  void class_get_members(objArr& ddArr);

  // Given dd element for method, return class which declares it
  ddElementPtr method_get_class();


  // this function implements the inverse operation of class_get_members()

  // given dd element for a field or a method, return class which declares it
  ddElementPtr member_get_class();
  symbolPtr  get_def_file_from_proj(projNode * pr);
  int get_loaded_refs_same_file(objArr& os, ddKind k = (ddKind) 0);
  
  int get_has_def();
  void set_has_def(int i);
  void update_name(char const *on, char const *nn);
  void set_datatype(int dt);
  void set_xref_hash(app* ah) { xref_hash = ah; }
  app * get_xref_hash() const { return xref_hash; }

  virtual void put_signature(ostream& os) const;
  virtual int construct_symbolPtr(symbolPtr&) const;
private:
// private version of public function
  xrefSymbol* get_xrefSymbol_priv();
};


generate_descriptor(ddElement,ddSymbol);
#ifdef XXX_ddTable
class ddTable: public Relational {
    ddTokenPtr headers[MAX_HASH];     // lists of _tokens accessed thru hash
    ddStrPage *str_page_list;         // string buffers used in this table
    ddTokenPtr current_scope;
    void addHeader( ddTokenPtr newT);
    ddTokenPtr find_token( int slen, char const *str);
    ddTokenPtr lookAgain( ddTokenPtr old_tok);
    RTLNodePtr class_root;
    RTLNodePtr file_root;
  public:

    ddTable();
#ifdef __GNUG__
    ddTable( const ddTable& old_tbl){ *this= old_tbl;} 
#endif
    declare_copy_member(ddTable);
    define_relational(ddTable,Relational);
    RTLNodePtr get_class_list(){return class_root;}
    RTLNodePtr get_file_list(){return file_root;}
    int thash( int , char const *){return(0);}
    void dump_tokens( ostream& ){}
    void dump_names(ostream &){}
    void dump_files(ostream &){}
    ddTokenPtr find_name( int , char const *){return(0);}
    ddTokenPtr find_scope( int , char const *){return(0);}
    ddTokenPtr find_hidden_name(int , char const *){return(0);}
    void addName( ddTokenPtr ){}
    void addScope( ddTokenPtr ){}
    void addHiddenName( ddTokenPtr ){}
    void unlink_token( ddTokenPtr ){}
    ddString *Str_alloc(int ){return(0);}
    void add_string_page(ddStrPage *pg)
      { pg->next= str_page_list; str_page_list= pg;}
    ddString *make_string( int , char const *){return(0);}
    ddTokenPtr create_name_token( int , char const *){ return(0);}
    ddTokenPtr create_scope_token( int , char const *){ return(0);}
    ddTokenPtr create_hidden_token( int , char const *){ return(0);}
    ddTokenPtr this_scope(){return current_scope;}
    ddTokenPtr set_file( int , char const *);
    void reset_file( ddTokenPtr block){ current_scope= block;}
    void remove_file_decls( char const *){}
    void obsolete_file_decls( char const *){}
    ddElementPtr lookup( ddKind , int , char const *);
    ddElementPtr get_entry( ddKind e_kind, int lname, char const *pname)
      {return lookup( e_kind, lname, pname);}
    smtTreePtr get_smt_function( int , char const *){ return(0);}
    ddElementPtr add_field(
	       ddElementPtr , // the parent definition
	       ddKind ,       // What kind of definition
	       smtTreePtr ,   // Token id from SMT;
               int ,      // length of defined name
               char const *,    // the name
               int ,       // length of the definition
               char const *){ return(0);}    // the definition
    ddElementPtr add_field(
	       ddElementPtr , // the parent definition
	       ddKind ,       // What kind of definition
	       int ,   // Token id from SMT;
               int ,      // length of defined name
               char const *,    // the name
               int ,       // length of the definition
               char const *){ return(0); }    // the definition
    ddElementPtr add_field(
	       ddElementPtr , // the parent definition
	       ddElement *){ return(0); } // The element on disk
    ddElementPtr add_field( ddElementPtr ,
	        ddElementPtr ,
		ddKind , smtTreePtr ,
		int , char const *, int , char const *){ return(0); }
    ddElementPtr add_def( ddElement *){ return 0;}
    ddElementPtr add_def(
	       ddKind k,       // What kind of definition
	       smtTreePtr token_id,   // Token id from SMT; line number
	       int lname,      // length of defined name
	       char const *pname,    // the name
	       int ldef,       // length of the definition
	       char const *pdef);    // the definition
    ddElementPtr add_def(
	       ddKind k,       // What kind of definition
	       int token_id,   // Token id from SMT; line number
	       int lname,      // length of defined name
	       char const *pname,    // the name
	       int ldef,       // length of the definition
	       char const *pdef);    // the definition
    int remove_def( ddElement *){return(0);} // remove this element
    void obsolete_def( ddElement *){} // remove this element
    int remove_def(
	       ddKind ,       // What kind of definition
	       smtTreePtr ,   // Token id from SMT; line number
	       int ,      // length of defined name
	       char const *){ return(0);}   // the name
    void print_file_definitions( ddTokenPtr , ostream& , int ){
    }
    void print( ostream& = cout, int  = 0) const{ }
    void set_files_old(){}
    void remove_obsoletes(){}
    ddElementPtr generate_field( ddElementPtr ,
				ddElementPtr ,
				ddKind ,
				int , char const *,
				int , char const *){ return(0); }
  };

  generate_descriptor(ddTable,Relational);
#endif
class hash_element
{
public:
ddElement* el;
hash_element* next;
hash_element* prev;
hash_element(ddElementPtr ep) { el = ep; ep->set_local_hash(this); 
				prev = NULL; next = NULL;}
~hash_element() {if (el) el->set_local_hash(0); }
};



class ddRoot: public ddNode
{
public:
  ddElementPtr macro_list;       // also have dd_macro_to_root relation
  ddElementPtr fdecl_list;       // dd_fdecl_to_root ...
  ddElementPtr vdecl_list;  
  ddElementPtr tdecl_list;  
  RTLNodePtr  class_root;
  RTLNodePtr  file_root;
  hash_element* dd_hash[MAX_HASH];
  int num_defined;

  ddRoot(app* sh);
  ddRoot(smtHeader* sh);
  ddRoot(){};
  ddRoot(ddKind);
  ddRoot( const ddRoot& );
  declare_copy_member(ddRoot);
  define_relational(ddRoot, Relational);
  RTLNodePtr get_class_list(){return class_root;}
  RTLNodePtr get_file_list(){return file_root;}
  int Rhash(char const *s);
  ddElementPtr lookup(ddKind k, char const *ns); // 0 if failed, else ddelement
  void insert(ddElement *ep, ddKind kind); // add to dd_root
  void insert_hash(ddElement *ep); // add to local hash
  void insert_hash(hash_element* he); // add to local hash
  ddElementPtr add_def(ddKind kind, int nl, char const *ns, 
		       int dl, char const *ds, int al, char const *an);
  ddElementPtr add_field( ddElementPtr parent_def, ddElementPtr pred,
			 ddKind k, smtTreePtr token_id,
			 int nl, char const *ns, int dl, char const *ds,
			 int al, char const *an);
  ddElementPtr add_field( ddElementPtr parent_def,
			 ddKind k, smtTreePtr token_id,
			 int nl, char const *ns, int dl, char const *ds,
			 int al, char const *as, int pos = 0);

  app * get_main_header() 
    {return (app *)get_relation(smtHeader_of_ddRoot, this);}
  void update_hash(ddElement*, char const *, ddKind);
  void remove_hash(ddElement*, char const *, ddKind);
  void remove_hash(hash_element* he, int i);
  friend Relational * Erestore(void * , app *, char const *);
  friend Relational * Rrestore(void * , app *, char const *);
  friend void restore_ptr(app* );
  friend void dd_restore_ptr(ddElementPtr );
  friend void dd_restore_list(ddElementPtr );
  ~ddRoot();
  void tidy_els(ddKind k);
  void db_fill ();
  void init_dds();
  void setup_SWT_Entity_Status(int);
  int is_dd_just_built_from_scratch() { return num_defined;}
  void unset_dd_just_built_from_scratch() { num_defined = 0;}
};


generate_descriptor(ddRoot,ddNode);

class astType;
#ifdef XXX_myTreeObj //class myTreeObj : public Relational
{
public:
  smtTree* ref_smt;
  union {
    ddElement* dd_node;
    astType* at;
  };
  myTreeObj(smtTree* smt, ddElement* ep);
  myTreeObj(smtTree* smt, astType* a) { ref_smt = smt; at = a; }
#ifdef __GNUG__
  myTreeObj (const myTreeObj& oo) { *this = oo;}
#endif
  declare_copy_member(myTreeObj);
  define_relational(myTreeObj,Relational);
  virtual int compare(const Object&) const;
  virtual unsigned hash () const;
};
generate_descriptor(myTreeObj,Relational);
#endif
class cardObj : public Obj
{
public:
  void inc(appTree*);
  void dec(appTree*);
  cardObj(Obj* o);

  Obj* ob;
  objSet os;
};

struct typeDef
{
  astType* mv;
  astType* co;
};

#define IDFILE "ID"

#ifndef hp10
#define PAGE_SIZE 512
#endif

#define FI_SIZE (PAGE_SIZE/sizeof(short))

#define strequ(s1, s2)          (strcmp((s1), (s2)) == 0)
#define strnequ(s1, s2, n)      (strncmp((s1), (s2), (n)) == 0)
#define strnsav(s, n)           (strncpy(calloc(1, (n)+1), (s), (n)))



  void dd_set_baseclass_CPLUSPLUS( ddElementPtr, ddElementPtr, int, ddProtection);
  ddElementPtr dd_get_entry( ddKind kind, int lname, char const *pname);
  ddElementPtr dd_class_insert( ddElementPtr ep,
			       objOperType position,
			       ddKind k,
			       int lname, char const *pname,
			       int ltype, char const *ptype);
  void dd_set_inheritance_protection( ddElementPtr cp,
				  ddElementPtr basep,
				  ddProtection protect);
  void dd_set_inheritance_virtual( ddElementPtr cp,
			       ddElementPtr basep,
			       int is_virtual);
  bool dd_create_or_append_to_files(
      char const* entity_name,	// class or func, used if no file name specified
      fileLanguage lang,
      char const* hdr_phys_dir,
      char const* hdr_filename,	// NULL => use entity_name + suffix
      char const* hdr_contents,
      int hdr_node_type,
      projModulePtr& hdr_module,
      smtHeaderPtr& hdr_smt,
      char const* src_phys_dir,	// NULL => do header only
      char const* src_filename,	// NULL => use entity_name + suffix
      char const* src_contents,
      int hdr_node_type1,
      projModulePtr& src_module,
      smtHeaderPtr& src_smt);
  void dd_set_baseclass( ddElementPtr ep, ddElementPtr cp,
		 int is_virtual, ddProtection protect);
  smtTreePtr dd_find_base_clause( ddElementPtr cp, ddElementPtr basep);
  int dd_is_baseclass( ddElementPtr cp, ddElementPtr basep);
  void dd_generate_member( ddElementPtr parent, int lname, char const *pname, int ltype, char const *ptype, ddProtection prot);
  void dd_generate_struct_member( ddElementPtr parent, int lname, char const *pname, int ltype, char const *ptype);
//smtTreePtr dd_smt_find_tok( smtTreePtr tree, int len, char const *string);
  smtTreePtr dd_smt_find_tok(int len, char const *string, smtTreePtr start_tok,
			     smtTreePtr end_tok);
  smtTreePtr dd_get_smt( ddElementPtr ep );
  smtTreePtr dd_get_loaded_smt( ddElementPtr ep );
//  int dd_smt_is_protection( smtTreePtr);
//ddProtection dd_smt_which_protection( smtTreePtr);
  void dd_change_protection( ddElementPtr, ddProtection);
  int parse_cplusplus_typename( char const *);
  astDecl *dd_make_class_ast( char const *);
  void do_fields( ddRoot *tbl, ddElement *parent, astDecl *fdecl);
  void dd_generate_static_member_CPLUSPLUS( ddElementPtr parent, int lname, 
					   char const *pname, int ltype, char const *ptype, 
					   int livalue, char const *pivalue, 
					   ddProtection prot);
  void dd_change_characteristic( ddElementPtr ep, ddCharacteristic cc);
//int dd_smt_token_eq( smtTreePtr tok, int lstr, char const *pstr);
  void get_struct_rel_of(ddElementPtr, objArr& targ_classes, objArr_Int& rel_types,
			 objArr& responsible_members);
  ddElementPtr get_dd_ast(astDecl*, int, ddRoot*);
//  ddTable * dd_get_dictionary();
//xrefTable* get_xref();
//xrefTable* get_global_xref(char const *file);
  xrefEntry* get_xref_entry(ddElement*);
  ddElement* dd_get_def(ddKind, char const *);
  ddElement* dd_get_def(xrefEntry*);
  ddElement* smt_get_dd(smtTree*);
  smtHeaderPtr dd_smt_find_impl_file(ddElementPtr dd_class);
  ddElement* dd_lookup_or_create(char const *ns, appTree* node, int xref_type,
                       ddKind k, RelType* rel, char const *as_name);
  objSet *get_comp_unit_headers();
  void clean_comp_unit_headers();
  void dd_init_comp_unit_headers();
int get_time_of_day();

defrel_one_to_many(appTree,assocnode,ddElement,dd);
defrel_one_to_many(appTree,def_assocnode,ddElement,dd);

defrel_many_to_many(appTree,assoc_member,ddElement,dd);
defrel_many_to_many(appTree,app,ddElement,soft_assoc);
defrel_one_to_many(appTree,assoc_instance,ddElement,dd);

// create relationship between an SMT_decl node and its
// semantic and syntactic type strings in the dd. the
// semantic type string is derived from the AST while walking the
// completed SMT. The syntactic type string is not yet implemented
// (12/30/92), but is planned to be derived from the source tokens
// of the declaration in the SMT, so the syntype/smt_decl relation
// is not yet used.

defrel_many_to_one(appTree,smt_decl,ddElement,semtype);
defrel_many_to_one(appTree,smt_decl,ddElement,syntype);

// create relation between a type name in a typedef
// and its ddelement for type definition, eg, as in
// 	typedef struct {...} A.
//
// This would be applied to the token A and the ddElement
// for the class declaration, or the ddelement for the
// class A.

defrel_many_to_one(smtTree,typenm,smtTree,classorstruct);
defrel_many_to_one(smtTree,cd,ddElement,structure);
defrel_many_to_one(ddElement,propagate,ddElement,src);
defrel_many_to_one(ddElement,instance,ddElement,assocType);

defrel_many_to_one(ddElement,dd,appTree,ref_file);

#endif




