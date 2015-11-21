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
#ifndef _symbolPtr_h_
#define _symbolPtr_h_

#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef _ddKind_h
#include <ddKind.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif

//  The fsymbolPtr definition is a temporary one, to allow us to make
//     a global change.  Eventually, every reference to fsymbolPtr should
//     change to symbolPtr.
#define  fsymbolPtr  symbolPtr


class xrefSymbol;
class Hierarchical;
class commonTree;
class smtTree;
class appTree;
class app;
class ddSymbol;
class ddElement;
class scopeNode;
class Xref;
//class xrefTable;
class symbolPtr;
class propManager;
class CPManager;
class symbolArr;
class projNode;
class MetricSet;
class MetricElement;
class ddSelector;
class scopeLeaf;
class astNode;
class astXref;
class dataCell;
class GDGraph;

class linkTypes;
class linkType_selector;

class symbolPtr {

    friend class xrefSymbol;
    friend class symbolSet;
    friend class symbolIndSet;
    friend class Xref;
    friend class Link;
    friend class ddChange;
    friend class xrefPair;
    friend class hashPair;
    friend class headerInfo;
    friend class XrefTable;
    friend class XrefObj;
    friend class SharedXref;
    friend class link_iter;
    friend class ddSymbol;
    friend class ddElement;
    friend class xrefEntry;  
    friend class propManager;
    friend class CPManager;

    friend void XREF_set_last_modified_date (app* sh);
    friend xrefSymbol* Xref_file_is_newer_priv(char const * ln);
    friend app* XREF_get_files_including(app* ah);
    friend app* XREF_get_files_including(char const *ln);
    //friend bool XREF_file_included_by(app* ah);
    friend int  XREF_check_files_including(char const *fn);
    friend void get_filtered_link (ddKind kind, symbolPtr xr, 
                                   linkType lt, symbolArr& as);
    //     note:  in following declarations, I had to use ddElement* and smtTree*
    //          rather than ddElementptr and smtTreePtr, because I couldn't find
    //          the declarations of the latter
    friend void handle_user_of_used(ddElement* user, smtTree* smt, int oper);
    friend void handle_used_of_user(ddElement* user, smtTree* smt, int oper);
    friend void handle_used_of_user(ddElement*, ddElement*, int);
    friend void handle_has_friend(ddElement* used, smtTree* smt, int oper);
    friend void handle_ref_smt_of_dd(ddElement* used, smtTree* smt, int oper);
    friend void handle_def_smt_of_dd(ddElement* used, smtTree*, int oper);
    friend void handle_decl_smt_of_dd(ddElement* used, smtTree*, int oper);
    friend void handle_smt_decl_of_semtype(ddElement* used, smtTree* smt, int oper);
    friend void handle_superclassof(ddElement* user, smtTree* smt, int oper);
    friend void handle_superclassof_att(ddElement * , ddElement *, int, RelType *);
    friend void handle_template_of_instance(Relational*, Relational*, int);
    friend void handle_property_of_possessor(ddElement *, ddElement *, int);
    //friend void XREF_add_link(symbolPtr sym, linkType lt, Relational* ah);
    friend symbolPtr lookup_xrefSymbol(ddKind kind, char const * fn);
    friend xrefSymbol* lookup_file_symbol_priv(ddKind kind, char const * fn);
    friend xrefSymbol* Xref_file_is_newer_priv(char const * ln, char const *fn);
    friend xrefSymbol* Xref_insert_symbol_priv(ddSymbol* dd, app* h);
    friend void handle_includes(ddElement*,ddElement*,int);
    friend void handle_assoc(ddElement*,ddElement*,int);
    friend void xref_add_link(ddElement*, ddElement*, linkType);
    friend void handle_struct_decl_of_ref(ddElement* cp, ddElement* fp, int oper);
    friend void handle_instance_of_assocType(ddElement *cp, ddElement *fp,
                                             int oper);
    friend void handle_ifl_targ_of_src(ddElement *, ddElement *, int);
    friend void handle_ref_app_of_dd(ddElement *, appTree *, int);
    friend void handle_ref_file_of_dd(ddElement *used, smtTree *, int oper);
    friend void handle_grp_rel(ddElement *src, ddElement *trg,
                               int oper, RelType *);
    friend  app *get_app_for_file_sym(symbolPtr fs);  
  
    union {
        Obj        *rp;
        scopeNode  *sc;
        ddElement  *dp;
        char       *cp;
        MetricSet  *mp; // added for Y2K
        astXref    *ap; // added for Y2K
        GDGraph *graph; // added for DFA
    } un;

    unsigned int offset;  //in case of xrefsymbol offset is actually index of symbol in pmod.ind

    // the offset will be NOTXREFSYM if this SymbolPtr is used to represent an
    //   Obj or a ddElement.  If it's used to represent an xrefSymbol,
    //   the offset is the offset from the beginning of the header, and it will
    //   be nonzero, since symbols only start after the header.

 public:

    symbolPtr();
    symbolPtr(const symbolPtr &that);
    ~symbolPtr();

    symbolPtr(Obj *obj);
    symbolPtr(xrefSymbol *sym);
    symbolPtr(MetricElement *mel, MetricSet *mset);
    symbolPtr(astNode *nd, astXref *axr);
    symbolPtr(dataCell *cell);

    symbolPtr &operator=(const symbolPtr &oo);
    symbolPtr &operator=(ObjPtr oo);
    symbolPtr &operator=(appTree *oo);
    symbolPtr &operator=(Object *oo);

    int is_xrefSymbol() const;
    int is_instance() const;
    int is_smt() const;
    int is_ast() const;
    int is_dataCell() const;
 
    operator int() const;
    operator Obj*();
    operator Relational*();
    operator commonTree*();
    operator appTree*();
    operator app*();

    // make this one private:   operator xrefSymbol* ();

    //  The following function will return a symbolPtr, but inside, it will be
    //      an xrefSymbol.

    symbolPtr get_xrefSymbol() const;
    symbolPtr find_xrefSymbol() const;

    ddKind get_kind() const;
    projNode *get_projNode();
    int relationalp() const;

    int get_language();
    int get_language_local();
    Relational *get_noxref_def() const;

    /* Note: the following functions can be applied only on symbolPtrs that 
       are xrefSymbols */

    const char* get_unqual_name();
    bool is_override_for(const char*);
    int get_overrides(symbolArr& , bool);
    int get_base_dtors(symbolArr& );
    void rm_dup_links();
    int get_links(linkTypes& la, symbolArr& as, int rec =0);
    void get_links(linkType_selector&, const objSet&, bool);
    int get_link(linkType lt, symbolArr& as, const objSet& projs, int rec = 0);
    int get_link_chase_typedefs(linkType lt, symbolArr& as, const objSet& proj);
    ddElement* get_dds(app*);
    void rm_link(linkType);
    time_t get_last_mod_date();
    void set_datatype(char);
    int get_datatype();
    symbolPtr get_parent_of(const objSet&);
    bool get_has_def_file(int remote =0);
    bool is_loaded_or_cheap();
    int is_loaded();
    void get_equivalence_obj(symbolArr &);
    app* load_file();
    void put_signature(ostream& ostr) const;
    void get_files(symbolArr&);
    /* end of functions that cane be applied only on xrefSymbols */

    char const *get_name() const;        // dangerous function, please use next one
    //  In particular, if get_name() is called, you must use the string immediately in the same 
    //    expression, and not store it.  If you need to keep the string around, use get_name(genString&)
    const char *get_name(genString &result) const; // Use this format;   it is safer

    int operator == (const symbolPtr& o2) const;
    int operator != (const symbolPtr& o2) const;
    void print (ostream& = cout, int level = 0) const;
    ddElement* get_def_dd ();
    ddElement* get_module_dd ();
    app*       get_def_app ();
    bool is_consistent() const;    // test a symbolPtr for self-consistency
    int sym_compare(const symbolPtr second) const;    // compare two xrefSymbols for kind, name, and deffile
    int sym_compare_mp(const symbolPtr second, int mp) const;    // compare two xrefSymbols for kind, name, and deffile


    symbolPtr(Xref* Xrefptr, xrefSymbol* sym);

    // operator->  means to convert this object to an xrefSymbol*, and perform whatever
    //    action that class knows how to do on it.  It cannot be used on an Obj* unless
    //    such a conversion is meaningful.

    xrefSymbol*  operator->  () const;
    bool  isnotnull() const;			// check pointer in the symbolPtr for null
    bool  isnull() const;			//
    bool  xrisnotnull() const;		// convert to xrefSymbol* before checking
    bool  xrisnull() const;
    bool  without_xrefp() const;        // for projNode, projModule and projHeader
    void add_link_extend(linkType lt, symbolPtr sym, int rev) const;  // add a pair of links between this & sym

    
    //  constructor  symbolPtr(int) is removed
    //    it seemed necessary because of the constructor for xrefSymbol*
    //    to avoid "ambiguous call of ... errors, when passing 0 to a function
    //    that wants a symbolPtr.  Code should use NULL_symbolPtr instead
    //  symbolPtr(int foo);

    int operator==(const xrefSymbol* ptr) const;
    int operator==(xrefSymbol* ptr) const;
    int operator!=(const xrefSymbol* ptr) const;
    void node_prtS ();


 private:
    operator xrefSymbol* () const;
    xrefSymbol* xrefSymbol_priv() const;  // helper function for the preceding operator
    xrefSymbol* get_or_find_xrefSymbol (int is_find) const;
    xrefSymbol*  addr() const;

 public:
    Xref * get_xref() const;// { return is_xrefSymbol() ? un.xp : 0 ;}
    scopeNode * get_scope() const {return is_xrefSymbol() ? un.sc : 0 ;}		       

    unsigned int get_offset() const { return offset; }
    void put_info(scopeNode *nd, unsigned int o_s) { un.sc = nd; offset = o_s;}
    void put_scope(scopeNode *nd) { un.sc = nd; }
    int get_all_symbols(symbolArr& result, int mode = 0);
    int get_all_symbols(symbolArr& result, objArr& proj_list, int mode = 0);

    symbolPtr get_def_symbol(int mode = 0);
    symbolPtr get_def_symbol(objArr& proj_list, int mode = 0);
    symbolPtr get_def_symbol(objArr& proj_list, xrefSymbol *def_file, int mode = 0);

    static symbolPtr get_def_symbol_for_leaf(scopeLeaf*, ddKind, const char*, xrefSymbol*);
    static symbolPtr get_def_symbol_for_project(projNode*, ddKind, const char*, xrefSymbol*);
    static xrefSymbol* lookup_all_for_project(projNode*, symbolArr&, ddKind, const char*, xrefSymbol*);
    unsigned int get_xrefsym_offset_real(); //returns real offset for xrefSymbol instead of index(as returned by get_offset)

    int is_def();			       
    int get_def_symbols(symbolArr & result, int mode = 0);
    int get_scopes (objArr & result, int mode = 0);
    int get_units (objArr & result, int mode = 0);
    int get_link(linkType lt, symbolArr& result);
    int get_local_link(linkType lt, symbolArr& result);
    int get_define(symbolArr& as,const ddSelector& sel );
    

    //  The following methods are for instances (Y2K)
 public:
    int line_number();
    int char_offset();
    int line_offset();
    symbolPtr scope();
    symbolPtr sym_of_instance() const;
    symbolPtr sym_of_astNode() const;
    symbolPtr sym_of_dataCell() const;

    MetricElement get_MetricElement() const;
    MetricSet* get_MetricSetPtr() const;

    symbolPtr get_astNode();
    astXref* get_astXref() const;
    GDGraph *get_graph() const;
};

/** This version of get_name does not give the user a pointer into the
 *   xrefSymbol, or into the object name field.  Thus it is safer in
 *   case the pmod must be remapped, or if the object gets deleted
 *   while we would still have the pointer.  It returns a const
 *   pointer into the resultant genString, which is safe till the
 *   caller modifies that genString.
 **/
inline const char* symbolPtr::get_name(genString& result) const
{
    result.put_value(get_name());
    return result.str();
}


extern "C" void node_prtS (symbolPtr&);
extern const symbolPtr NULL_symbolPtr;



#ifndef  PSET_SPECIAL_DEBUG

inline xrefSymbol*  symbolPtr::operator->  () const
{ 
    return operator xrefSymbol*(); 
}

inline xrefSymbol*  symbolPtr::addr() const
{
    return  operator xrefSymbol*();
}
#endif

inline int symbolPtr::operator==(const xrefSymbol* ptr) const
{ 
    return (ptr == (xrefSymbol*) get_xrefSymbol());
}

inline int symbolPtr::operator==(xrefSymbol* ptr) const
{ 
    return (ptr == (xrefSymbol*) get_xrefSymbol());
}

inline int symbolPtr::operator!=(const xrefSymbol* ptr) const
{
    return (ptr != (xrefSymbol*) get_xrefSymbol());
}

#endif

/*
  START-LOG-------------------------------

  $Log: symbolPtr.h  $
  Revision 1.40 2000/11/14 10:15:30EST Scott Turner (sturner) 
  adding support for 'language' and 'checksum' properties to the model,
  for bug 19353 and for better detection of the Java ELS language
  * Revision 1.43  1994/06/14  15:53:26  davea
  * Bug track: n/a
  * To workaround cfront bug:  "internal cfront error: bus error (or something nasty like that)"
  *
  * Revision 1.42  1994/06/09  18:52:33  davea
  * Bug track: 7210
  * Add safer symbolPtr::get_name(genString&), that does not return a
  * pointer into either mapped pmod area or into a Relational object.  This
  * uses a passed genString instead, and returns a pointer into its string.
  *
  * Revision 1.41  1994/05/10  23:41:20  trung
  * Bug track: 0
  * misc. fixes for subsystem, reparse
  *
  * Revision 1.40  1994/03/21  19:47:24  trung
  * Bug track: 6625
  * submitting for cpp_open_file, etc.
  *
  * Revision 1.39  1994/02/25  17:26:27  davea
  * Bug track: 6485
  * new member function symbolPtr::add_link_extend(),
  * which automatically extends pmod as needed
  *
  * Revision 1.38  1994/01/12  00:48:30  davea
  * bugs 5940, 5670, 5693, 5882,5896, 5889, and 5900
  *

  END-LOG---------------------------------
*/


