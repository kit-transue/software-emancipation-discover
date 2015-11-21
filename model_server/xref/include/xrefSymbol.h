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
#ifndef _xrefSymbol_h
#define _xrefSymbol_h

#include <Relational.h>
#include <ddKind.h>
#include <_Xref.h>
#include <Link.h>
#include <dis_iosfwd.h>

class symbolArr;
class symbolSet;
class objSet;
class hashPair;
class ddElement;
class ddSymbol;
class Link;
RelClass(ddElement);
class smtTree;
RelClass(smtTree);
class RTLNode;
class ddSelector;
class projNode;
class linkTypes;
RelClass(RTLNode);

typedef unsigned char uchar;
typedef unsigned short ushort;
class linkType_selector;

#ifndef CVP
#define CVP(x) ((char *) (&(x))) 
#define CP(x) ((char *) (x)) 
#endif

#define READ_ONLY 1
//  Note:  remove references to "WRITE_ONLY", since it's a bad name.  Use !READ_ONLY instead
#define WRITE_ONLY 0
#define MAX_BROWSER 128

/*  xrefSymbol is the primary data structure found in a *pmod.sym file
   The data items are defined here as unsigned char, because there are no
   alignment restrictions, and the Sparc architecture at least will
   complain if we try to read unaligned data.

   Consequently, only the member functions should be used to access
   the data in the structure.  These member functions fetch the
   numeric fields assuming that a positive integer is at least 16
   bits, and is stored in conventional form, permitting operators <<,
   >> and | to work as expected.  They also assume that the char size
   is 8 bits.  They do *not* assume that the machine is big-endian,
   though the data in the structure is.

   Other structures used in the pmod.sym file are defined in
   xref_priv.h.  These include HeaderInfo, and SymHeaderInfo for
   header information, and Link, used to attach variable length
   information to the xrefSymbols.

   The pmod.ind file contains classes which are all defined in
   xref_priv.h.  These are IndHeaderInfo for header information, and
   SymInd, which describes the body of the file.

   Many other classes deal with these primitives, and most of these
   are also located in xref_priv.h.  But the most visible of these are
   Xref (defined here) and XrefTable.  A projNode points to a single
   Xref, which may have an XrefTable.  The XrefTable constructor
   attaches the pmod files, optionally creating them, and/or copying
   them to /usr/tmp or other designated temp location.

   Related enums are ddKind and linkType, both defined in
   dd/include/ddKind.h.

   Another important class is symbolPtr.  A symbolPtr may represent
   either an Obj* item or an xrefSymbol.  The way it represents an
   xrefSymbol is via a pointer to an Xref, and an offset within the
   mapped pmod file.  This symbolPtr remains valid when the pmod grows
   and gets remapped.  Any xrefSymbol* become invalid after such a
   remapping.  The remapping may happen any time that make_room() is
   called, such as at the beginning of an insert_module(), or a
   copy_module().  Class symbolPtr has a few member functions to deal
   with xrefSymbols, but it also has the operator -> function, which
   converts to an xrefSymbol*, and calls the appropriate member
   function of xrefSymbol.

*/

class xrefSymbol
{
friend class testpmod;
friend class sorted_link;
friend class spec_link;
friend struct linkSet;
friend class symbolPtr;
friend class Xref;
friend class Link;
friend class XrefTable;
//friend class xrefTable;  // ??? check with Trung
friend class xrefEntry;  //  ????
friend class ddElement;  // ????
friend void XrefTable_insert_module(XrefTable*, app*);
friend void XrefTable_insert_module(XrefTable*, app* , int);
friend void XREF_set_last_modified_date (app* sh);
//friend void XREF_add_link(xrefSymbol* sym, linkType lt, Relational* ah);
//friend void XREF_add_link(symbolPtr sym, linkType lt, Relational* ah);
friend void print_XREF();
friend void print_XREF(Xref* Xr);
friend symbolPtr Xref_file_is_newer(char const *ln, char const *fn);
friend symbolPtr Xref_insert_symbol(ddSymbol* dd, app* h);
friend void handle_user_of_used(ddElementPtr user, smtTreePtr smt, int oper);
friend void handle_used_of_user(ddElementPtr user, smtTreePtr smt, int oper);
friend void handle_used_of_user(ddElementPtr subsys, ddElementPtr mbr, int oper);
friend void handle_has_friend(ddElementPtr used, smtTreePtr smt, int oper);
friend void handle_def_smt_of_dd(ddElementPtr used, smtTreePtr , int oper);
friend void handle_decl_smt_of_dd(ddElementPtr used, smtTreePtr , int oper);
friend void handle_smt_decl_of_semtype(ddElementPtr used, smtTreePtr smt, int oper);
friend void handle_superclassof(ddElementPtr user, smtTreePtr smt, int oper);
friend void handle_superclassof_att(ddElementPtr , ddElement *, int, RelType *);
friend symbolPtr lookup_file_symbol(ddKind kind, char const *fn);
friend xrefSymbol* Xref_file_is_newer_priv(char const *ln, char const *fn);
friend xrefSymbol* Xref_file_is_newer_priv(char const *ln);
friend ddElementPtr new_dd_find_definition (ddKind kind, char const *name);
friend xrefSymbol* Xref_insert_symbol_priv(ddSymbol* dd, app* h);
friend void handle_ref_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper);
friend void new_xref_init_rtl(RTLNodePtr rtl_node, const ddSelector& selector);
friend projNode* lookup_all_proj_for_symbol(xrefSymbol* sym);
friend void handle_includes(ddElementPtr, ddElementPtr, int);
friend void handle_assoc (ddElementPtr, ddElementPtr, int);
friend void handle_grp_rel(ddElementPtr src, ddElementPtr trg, int oper, RelType*);
friend void xref_add_link(ddElementPtr, ddElementPtr, linkType);
friend int get_links_global_one_layer_internal(xrefSymbol*,linkTypes&,int,symbolArr&,xrefSymbol*,objSet&,objSet&,objSet&,Xref*);
friend void handle_struct_decl_of_ref(ddElementPtr cp, ddElementPtr fp, int oper);
friend void handle_ref_file_of_dd(ddElementPtr used, smtTreePtr , int oper);

public:      // public members of xrefSymbol
  symbolPtr get_alternate_def_file(linkType lt, ddKind k, symbolPtr & alt);
  int get_function_prototype(genString &);
// just added 3/10/93, so that the  xrefSymbol* of symbolPtr can be checked
//      for null or notnull, without explicitly producing an xrefSymbol*
    bool xrisnull();
    bool xrisnotnull();
  Xref* get_Xref() const;

  // usefull interface
//  void add_link(linkType lt, xrefSymbol* sym1);
    void add_link(linkType lt, symbolPtr sym1); // !!! this replaces previous
  int get_datatype(XrefTable*);
  ddElement* get_dds(app* header, XrefTable*);
  ddElement* get_def_dd();
    static ddElement* get_def_dd(symbolPtr);
  symbolPtr get_def_file();
  xrefSymbol* get_def_file(XrefTable*);
  // get all objects (global) that are defined in this.
  // eg.: if this is a class, return all methods, static vars, ...
  int get_define(symbolArr& as, ddKind k = (ddKind) 0);
  int get_files(symbolArr& as, XrefTable*); // all files related to this xrefSymbol
  void get_impacted_obj(symbolArr & fa, symbolArr & oa, XrefTable* Xr);
  ddKind get_kind() ;
  ddKind get_kind_real() ;
  void set_attribute(uint value, uint bitnum, uint numbits);
  uint get_attribute(uint bitnum, uint numbits);
  static void set_attribute(uint &new_value, uint value, uint bitnum, uint numbits);
  static uint get_attribute(uint & new_value, uint bitnum, uint numbits);
  // was: void set_attribute(ddSymbol * dd, XrefTable*);
  void set_attribute(ddSymbol * dd, XrefTable*);
  int get_language(XrefTable*);
  int get_language_local(XrefTable*);
  int get_link(linkType lt, symbolArr & as, int rec = 0);
//  int get_link_real(linkType lt, symbolArr & as, int rec = 0);
  int get_link(linkType lt, symbolArr& as, const objSet& projs, XrefTable*, int rec = 0);

  //boris: 121196  old way low level get_link_local () without scopes/units
  int get_local_link_internal (linkType, symbolArr& as, XrefTable*);

  int get_links(linkTypes &la, symbolArr & as, XrefTable* Xr, int rec = 0);
  int get_links_real(linkTypes &la, symbolArr & as, int rec = 0);
  void get_links(linkType_selector&, const objSet& projs, bool chase_typedefs, XrefTable*);
  void get_links_1pmod(linkType_selector&, const objSet& projs, bool chase_typedefs, XrefTable*);
  void get_all_links(symbolSet &, symbolSet&, Xref* xr = 0);
  char const *get_name() const;
  char const *get_name(XrefTable*) const; 
  char const *get_name_real(XrefTable*) const;
  symbolPtr get_parent_of();
  symbolPtr get_parent_of(const objSet& projs, XrefTable*);
  projNode* get_projNode(XrefTable*);
  int get_proj_ref_dds(objSet& os); // get all ref currently loaded
  xrefSymbol* get_real_sym();
  int get_ref_dds(objSet & os); // get all ref currently loaded
  // get all objects referenced in this. filtering out by kind
  int get_refs(symbolArr& as, ddKind k = (ddKind) 0);
  int get_refs(symbolArr& as, const ddSelector& sel); // not implemented yet
  int is_loaded(XrefTable*);    // whether the definition is currently loaded
  bool is_loaded_or_cheap(XrefTable* Xr);    // if is_loaded() or def file has .pset file
  app* load_file(XrefTable*);

  void print (XrefTable* Xr = 0, ostream& = cout, int level = 0) const;
  void put_signature(ostream& ostr, XrefTable*) const;
  void rm_link(linkTypes &lt, int, XrefTable*);
  void rm_link(linkType lt, int, XrefTable*);
  void rm_link(linkType lt, XrefTable*);
  void rm_link(linkType lt, symbolPtr sym);
  void set_datatype( char dt, XrefTable*);
  void set_language(int i, XrefTable* );
  symbolPtr update_name(char const *new_name);
  int get_ref_files(symbolArr& as); // all files related to this xrefSymbol
  bool is_override_for(char const *unqual_name, XrefTable*);
  void get_overrides_of(char const *unqual_name, symbolArr& overrides,
			symbolArr& processed_classes);
  void get_overrides_of(char const *unqual_name, symbolArr& overrides,
			symbolArr& processed_classes, int up);

  bool get_root_classes_with_virt_fcn(char const *unqual_name, symbolArr& root_classes);
  int get_overrides(symbolArr&, bool entire_hierarchy, XrefTable*);	// for virtual functions
  int get_base_dtors(symbolArr&, XrefTable*);  // inserts self + dtors from base classes
  bool get_has_def_file(XrefTable* ,int = 0);
  int belong_to_file(xrefSymbol* file_sym, XrefTable*);
  int get_link_chase_typedefs(linkType lt, symbolArr& as);
  int get_link_chase_typedefs(linkType lt, symbolArr& as, const objSet& projs, XrefTable*);
  bool is_symbol_in(Xref* xr) const;         // is this a symbol in the xref?
  Xref* get_subproj_Xref_of_symbol(projNode* pn, projNode** subprojptr=0) const;
  bool is_consistent(bool) const;         // test xrefSymbol for consistency
  int get_equivalence_obj(symbolArr & sa, XrefTable* );
  ddElement* restore_def_dd(projNode* pr);
  void set_att(ddSymbol*);
  void get_related_objs(symbolArr& destination, XrefTable*) const; // related objs for change prop
  void print_sym(ostream&, XrefTable*) const;
  void add_link(linkType lt, xrefSymbol* sym1, int i, XrefTable*); // add reverse link
  void add_link(linkType lt, xrefSymbol* sym, hashPair *rhps, int rev);  
  void set_need_to_mv_pmod_to_perm() ;
  void set_last_mod_date(time_t date, XrefTable*); // symbol has to be a file
  int local_only( linkType, int, XrefTable*);
  void msync_xref();
  void rm_dup_links(XrefTable*);
  time_t get_last_mod_date(XrefTable*); // symbol has to be a file  

private:    // private members of xrefSymbol
  void init(uint sym_ind, char knd, uint nm_off, int hsh);
  void add_link(linkType lt, long l, XrefTable*);

  long get_link_offset(linkType lt, XrefTable* xr); // symbol has to be a file
  long get_link_offset(linkType lt, int, XrefTable*); // symbol has to be a file
  long get_link_offset_global(linkType lt); // symbol has to be a file
  int is_newer(app* ah, XrefTable*);
  int is_newer(char const *fn, XrefTable*);

  bool is_writable();

  char const *get_unqual_name(XrefTable*);	// strip off class qualification
  void get_dtors(symbolArr& dtors, symbolArr& processed_classes);  // sym must be class
  ddElement* symbol_get_ref_dd();


//------------------------------------------------------------------------
  // cons, des, copy constructor
  xrefSymbol(){}
  //disallow:
  xrefSymbol(const xrefSymbol& old);// {*this = old;}

  // accessing private members
  int get_offset_to_first_link();
  void clear_has_def_file();
  uint get_no_link();
  int get_symbol_size(); // might not be useful at all
  uint get_symbol_index(); //get the index of the symbol
  int get_symbol_attr();  // get symbol attributes
  uint get_name_offset() const; //get offset to symbol name in pmod.sym file
  uint get_hash_value();  //get hash value of symbol name
  // modifying private members
  void set_kind(uchar k) {kind = k;}
  void set_offset_to_first_link(int );
  void set_has_def_file();
  void set_no_link(uint n);
  void set_name(char const *na);
  void set_symbol_index(uint ind);
  void set_symbol_attr(int); 
  void set_name_offset(uint);
  void set_hash_value(uint);
// still private:

  // internal interface for links
  void rm_link1(linkType lt, xrefSymbol* sym, XrefTable*);
  void remove_module_link(symbolSet &, Xref* );
  void build_remove_module_link(symbolSet&,symbolSet&, hashPair *, Xref* xr = 0);
  Link* get_start_link(uint ind, XrefTable* xr); // return first link
  Link* make_new_link(uint, XrefTable* xr);
  int last_link(Link* ln);
  int link_link(Link* ln);
  Link* get_next_link(Link* , uint, XrefTable*);
  Link* find_link(linkType lt, xrefSymbol* sym, XrefTable*);// return last link if not exist
  Link* get_spot_to_insert_link(Link* , XrefTable*); // used in add_link
  Link* get_spot_to_insert_link(linkType, xrefSymbol*, XrefTable* ); // used in add_link
//  Link* get_spot_to_insert_links(hashPair&, int* reserved);
  void put_def_file_for_sym(ddElement* dd); // might not be useful
  void create_link(Link& lin, linkType lt, xrefSymbol* sym);
//  void create_link(Link& lin, linkType lt, long);
  int equal(Link* lnk);
  void print1(ostream&, int level, XrefTable*);
  void print_link(linkType lt, ostream&, XrefTable*);
  char *belonged_to(XrefTable* xr) const;
  char *get_next_addr_offset();
  XrefTable* get_xrefTable();      // obsoleted because of bad naming:  use get_XrefTable()
  void update_sym_addr_offset(int n);
  int need_to_extend_file(char const *noa);
  XrefTable* get_XrefTable() const;
  void copy_link(xrefSymbol* sym, XrefTable* xref);
  int has_link(linkType lt, xrefSymbol* ref, XrefTable*);
  void copy_module_link(hashPair* hp, hashPair* def_hp, xrefSymbol* trg, xrefSymbol* file_s, XrefTable*);


//internal versions of public functions
  void add_link(linkType lt, xrefSymbol* sym1, Xref* xref);
  void add_links(hashPair& hp);
  void rm_links(hashPair& hp);  
  xrefSymbol* get_def_file_priv(XrefTable*);
  xrefSymbol* get_parent_of_priv();
  xrefSymbol* get_real_sym_priv();
  void rm_link(linkType lt, xrefSymbol* sym, XrefTable*);
  xrefSymbol* update_name_priv(char const *new_name);

//  private function to easily get xrefSymbol* address, so it can be
//     used with symbolPtr
    xrefSymbol* get_addr() { return this; }

// private function to add targets of a typedef to the result of
// get_link_chase_typedefs()
   void chase_typedef(symbolArr& results);
   void chase_typedef(symbolArr& results, const objSet& projs, XrefTable*);

  // -----------------start data section---------------
    unsigned char flags_byte;
 
  //attributes
    unsigned char attr_byte_0;
    unsigned char attr_byte_1;
    unsigned char attr_byte_2;

  //index of symbol in the array (in pmod.ind)
    unsigned char index_byte_0;
    unsigned char index_byte_1;
    unsigned char index_byte_2;

    unsigned char kind;

    unsigned int offset_to_start_link;  //offset to start link in pmod.lnk
    unsigned int name_offset;   //offset to xrefsymbol name on pmod.sym
    unsigned int hash_val;     // hash val of xrefsymbol name

/*   old version, not portable to little-endian machines
  unsigned name_length: 16;
  unsigned has_def_file:1;
  unsigned no_link: 15; // total number of links
  unsigned kind: 8;
  char name[1];
  */
};
#define   SIZEOF_XREFSYM  sizeof(xrefSymbol)
//#define   SIZEOF_XREFSYM  8+3*sizeof(int)

inline void xrefSymbol::set_has_def_file()
{
    flags_byte |= 0x80;
}

inline void xrefSymbol::clear_has_def_file()
{
    flags_byte &= 0x7f;
}

//  ADDED INLINE FCTS FOLLOWING:

#if 0 
inline void xrefSymbol::set_no_link(uint links)
{
    // number of links is a 15 bit unsigned number, big endian
    symbol_byte_2 = (0x80 & symbol_byte_2) | ((links >> 8) & 0x7f);
    symbol_byte_3 = links & 0xff;
}
inline uint xrefSymbol::get_no_link()
{
    return  ((symbol_byte_2 & 0x7f) << 8) | symbol_byte_3;
}
#endif
inline ddKind xrefSymbol::get_kind() 
{ 
  return (ddKind) kind; 
}
inline ddKind xrefSymbol::get_kind_real() 
{ 
  return (ddKind) kind; 
}

inline
linkType get_reverse_link(linkType lt)
{
  return (linkType) (lt^0x1) ;
}


#if 0
// will not be needing this, no lt_next links 
inline int xrefSymbol::link_link(Link* lnk)
{
  return ((lnk->lt() == lt_next) && (lnk->get_index() != 0));
}
#endif 

inline int xrefSymbol::last_link(Link* lnk) 
{
  return (lnk->lt() == lt_last);
}

extern char const *PSET_PRINT_LINK;
inline void dump_link(xrefSymbol *sym, Link *prev, Link *lnk, char const *where)
{
  if (PSET_PRINT_LINK) {
    if (sym) {
      cout << (void *) sym << ' ' << ddKind_name(sym->get_kind()) << ' ';
      cout << sym->get_name() << " in " << where << endl;
    }
    if (prev)
      prev->print(cout);
    if (lnk)
      lnk->print(cout);
  }
}

inline uint xrefSymbol::get_symbol_index()
{
  return (index_byte_0 << 16) | (index_byte_1 << 8) | index_byte_2;
}

inline void xrefSymbol::set_symbol_index(uint ind)
{
  ind &= 0xffffff;
  index_byte_0 = ind >> 16;
  index_byte_1 = (ind >> 8) & 0xff;
  index_byte_2 = ind & 0xff;
}

inline int xrefSymbol::get_symbol_attr()
{
  return ((attr_byte_0 << 16) | (attr_byte_1 << 8) | attr_byte_2);
}

inline void xrefSymbol::set_symbol_attr(int att)
{
  att &= 0xffffff;
  attr_byte_0 = att >> 16;
  attr_byte_1 = (att >> 8) & 0xff;
  attr_byte_2 = att & 0xff;
}

#if 0
extern void init_xrefSymLinkHash();
extern void delete_xrefSymLinkHash();
#endif
int assocAPI_find_assocs (symbolPtr, symbolArr&);
int assocAPI_find_files (symbolPtr, symbolArr&);
int linkAPI_getTargetFiles (symbolPtr, symbolArr&);
int linkAPI_getLinkSpecs(symbolArr&,const symbolPtr & item, bool = true);

#endif

