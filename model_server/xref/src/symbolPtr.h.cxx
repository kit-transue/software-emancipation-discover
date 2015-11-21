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
/*  symbolPtr.h.C  implement symbolPtr class
    
classes defined:
    symbolPtr - generalized pointer class, points to either xrefSymbol or
    to a Relational object (from which most classes are derived)

    non-class functions:
    // helper function equivalent to SGN() function in some languages
       inline int normalize(int arg);g
    // function for debugger to display a symbolPtr
       extern "C" void node_prtS( symbolPtr& sym)

*/

/* NOTE: 
   The offset field of symbolPtr used to be divided into two parts: 1 byte for 
   type of symbol (xref, met, ast, dfa ...) and 3 bytes for the offset. 
   The 3 bytes put limitation on the size of pmod ( eg pmod.met can be only 
   64Meg). To  get more out of this structure, only 4 bits(instead of 1 byte) 
   will be used for kind and 28 bits will be used for offset. If we later 
   realize that 4 bits are not enough to represent all kinds of symbols then 
   we should restructure the symbolPtr class.*/ 

#include <cLibraryFunctions.h>
#include <msg.h>

#include "symbolPtr.h"
#include "objOper.h"
#include "xref.h"
#include <xref_priv.h>
#include <systemMessages.h>
#include <scopeMgr.h>
#include <metric.h>
#include <smtMarker.h>
#include <dfa.h>
#include <dfa_db.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif
#include <scopeClone.h>
#include <scopeLeaf.h>
#include <scopeUnit.h>
#include <scopeMaster.h>

RelClass(ddElement);
extern void gen_print_indent(ostream&, int);

class dataCell;
class GDGraph;
dataCell *dataCell_of_symbol(const symbolPtr &symbol, bool load);
void dataCell_offload(dataCell *&cell, bool mode);
const symbolPtr &dataCell_get_symbol(dataCell *cell);
const symbolPtr &dataCell_get_xrefSymbol(dataCell *cell);
void dataCell_print(dataCell *cell, ostream &os, int level);

const symbolPtr NULL_symbolPtr;
int astNode_print(const symbolPtr&, ostream&,int);
static scopeNode * last_scope;
void symbolPtr_reset_last_scope () {
    last_scope = 0;
}

inline bool symbol_is_special(const symbolPtr*sp)
{
  return sp->is_instance() || sp->is_ast() || sp->is_dataCell();
} 

static objArr pmod_search_list;
void symbolPtr_reset_pmod_search_list ()
{
    pmod_search_list.removeAll();
}

static int doing_second_pass;

int xref_get_doing_second_pass()
{
    return doing_second_pass;
}

void xref_set_doing_second_pass(int val)
{
    if (doing_second_pass != val) {
	symbolPtr_reset_pmod_search_list ();
	doing_second_pass = val;
    }
}

int scope_get_global_mode();
objArr & symbolPtr_get_projs_to_search();
int ATT_hidden(symbolPtr&sym);

#define NOTXREFSYM 0x00000000

ddElement * xrefSymbol_get_def_dd(symbolPtr);
//  return 1 if this consistency check works

class Handle_smtMarker : public Relational {
  public:
    Handle_smtMarker(smtMarker *tmp) : smt_marker(tmp) {}
    ~Handle_smtMarker();

  private:
    smtMarker *smt_marker;
};

// -----------------------------------------------------------------------------------

Handle_smtMarker::~Handle_smtMarker()
{
    if(smt_marker->ref <= 0){
//	printf("Destroying %x\n", smt_marker);
	obj_delete(smt_marker);
    }
}

// -----------------------------------------------------------------------------------

static void post_to_delete(smtMarker *sm)
{
    Handle_smtMarker *h = new Handle_smtMarker(sm);
    obj_delete(h);
}

// -----------------------------------------------------------------------------------

static Xref * scopeNode_get_xref(scopeNode* sc)
{
    Xref * Xr = NULL;
    if (is_Xref(sc)) {
	Xr = (Xref*)sc;
    } else if (is_scopeLeaf(sc)) {
	scopeLeaf *leaf = (scopeLeaf *)sc;
	projNode * pr = clone_get_proj(leaf);
	Xr = (pr) ? pr->get_xref(1) : NULL;
    }
    return Xr;
}
    
bool symbolPtr_is_symbol_in(xrefSymbol *sym, scopeNode *sc)
{
    Xref * Xr = scopeNode_get_xref(sc);
    return sym->is_symbol_in(Xr);
}

bool symbolPtr::is_consistent() const
{
    if (un.cp==NULL) return 1;
    bool good_offset=1;
    bool foo1 = ((offset==NOTXREFSYM) || (offset & 0xF0000000));  // false iff  it's an xrefSymbol
    if (foo1) return(0);
    bool  foo2 =(!is_scopeNode(un.sc)); 
    bool  ok = (foo1 == foo2);
    if (!foo1) {   // if it's an xrefSymbol, test the upper limit of the offset
	Xref * xp = scopeNode_get_xref(un.sc);
	XrefTable* xr = xp->get_lxref();
	if (xr) {
	    headerInfo* hi = xr->get_header();
	    if (xr->get_sym_offset_from_ind(offset) >= ntohl(hi->next_offset_ind))
		good_offset = 0;
	}
    }

    if (ok && good_offset) return ok;

    Initialize(symbolPtr::is_consistent);
    Assert(ok && good_offset);  //  should never get here
    return ok && good_offset;
}


//  The following macro is to make it easier to conditionally include
//     a single statement, only in the debugging version
static char const *dis_is_debug = OSapi_getenv("DIS_SYMBOL_DEBUG");

#ifdef _PSET_DEBUG
#define IF_DEBUGGING(foo) if(dis_is_debug)(foo)
#else
#define IF_DEBUGGING(foo)
#endif


//.............................................
// The section in here was in-line code, and probably should become
//    that again.   later...


symbolPtr::symbolPtr(const symbolPtr& that)
{
    un.cp  = that.un.cp;
    offset = that.offset;

    if(is_smt()) {
	smtMarker *sm = (smtMarker *)un.rp;
	sm->ref++;
//	printf("Reference incremented %x\n", (void *)sm);
    }
}

extern "C" int driver_exiting() ;

symbolPtr::~symbolPtr()
{
    if(!driver_exiting()){
	if(is_smt()){
	    smtMarker *sm = (smtMarker *)un.rp;
	    sm->ref--;
	    //      printf("Reference decremented %x   (%d)\n", (void *)sm, sm->ref);
	    if(sm->ref <= 0)
		post_to_delete(sm);
	}
    }
    un.cp  = 0;
    offset = 0;
}

symbolPtr& symbolPtr::operator =(const symbolPtr& oo)
{
    IF_DEBUGGING(oo.is_consistent());

    int update_smt_ref = 0;

    if (un.sc != oo.un.sc) {
	update_smt_ref = 1;
    }

    if(update_smt_ref && is_smt()) {
	smtMarker *sm = (smtMarker *)un.rp;
	sm->ref--;
//	printf("Reference decremented %x   (%d)\n", (void *)sm, sm->ref);
	if(sm->ref <= 0)
	    post_to_delete(sm);
    }

    un.cp  = oo.un.cp;
    offset = oo.offset;
    IF_DEBUGGING(is_consistent());

    if(update_smt_ref && is_smt()) {
	smtMarker *sm = (smtMarker *)un.rp;
	sm->ref++;
//	printf("Reference incremented %x\n", (void *)sm);
    }

    return *this;
}

symbolPtr& symbolPtr::operator =(ObjPtr oo)
{
    un.rp  = oo;
    offset = NOTXREFSYM;
    IF_DEBUGGING(is_consistent());
    if(relationalp() && is_smtMarker((Relational *)oo)){
	((smtMarker *)oo)->ref++;
	offset = 0x20000000;
    }
    return *this;
}

symbolPtr& symbolPtr::operator =(Object* oo)
{
    un.rp  = (Obj*)oo;
    offset = NOTXREFSYM;
    IF_DEBUGGING(is_consistent());
    return *this;
}

symbolPtr& symbolPtr::operator =(appTree * oo)
{
    un.rp  =  oo;
    offset = NOTXREFSYM;
    IF_DEBUGGING(is_consistent());
    return *this;
}


symbolPtr::operator int () const
{
    IF_DEBUGGING(is_consistent());
    return un.cp ? 1 : 0;
}

symbolPtr::symbolPtr (Obj* obj)
{
    un.rp  = obj;
    offset = NOTXREFSYM;
    IF_DEBUGGING(is_consistent());
    if(relationalp() && is_smtMarker((Relational *)obj)){
	((smtMarker *)obj)->ref++;
	offset = 0x20000000;
    }
}

symbolPtr::symbolPtr ()
{
    un.cp = NULL;
    offset=0;
    IF_DEBUGGING(is_consistent());
}

//...................................

symbolPtr::operator Obj* ()
{
    if (!un.cp) 
	return NULL;

    if (is_xrefSymbol ()) {
     Initialize(symbolPtr::operatorObjPtr);
     IF_DEBUGGING(is_consistent());
	if ((*this)->get_kind() == DD_MODULE)
	    return (*this)->load_file(get_xref()->get_lxref());

	Obj * dd = (Obj*)xrefSymbol_get_def_dd (*this);
	return dd;
    }

    if (is_instance ()) {
     Initialize(symbolPtr::operatorObjPtr);
     IF_DEBUGGING(is_consistent());
     symbolPtr sym = get_xrefSymbol();
     if (sym.xrisnull()) return NULL;
     if (sym.get_kind() == DD_MODULE)
       return sym->load_file(get_xref()->get_lxref());
     
     Obj * dd = (Obj*)xrefSymbol_get_def_dd (sym);
     return dd;
   }
   if (is_ast())
       return NULL;
   return un.rp;
}

symbolPtr::operator Relational* ()
{
//    Initialize(symbolPtr::operatorRelationalPtr);
    IF_DEBUGGING(is_consistent());
    Obj* obj = operator Obj* ();
    if(!obj) return NULL;
    if (!obj->relationalp()) return NULL;
    return (Relational*) obj;
}

symbolPtr::operator commonTree* ()
{
    Initialize(symbolPtr::operatorcommonTreePtr);
    IF_DEBUGGING(is_consistent());
    return checked_cast(commonTree, (operator Relational* ()));
}

symbolPtr::operator appTree* ()
{
    Initialize(symbolPtr::operatorappTreePtr);
    IF_DEBUGGING(is_consistent());
    Relational * node = operator Relational* (); 
    if(!node) return NULL;
    if(is_appTree(node)) return appTreePtr(node);
    else if(is_app(node)) return appTreePtr(appPtr(node)->get_root());
    else return 0;
}

int symbolPtr::relationalp () const
{ 
    IF_DEBUGGING(is_consistent());

    return (!un.cp || (offset != NOTXREFSYM && offset != 0x20000000)) ? 0 : un.rp->relationalp (); 
}



symbolPtr::operator xrefSymbol* ()  const
{
    xrefSymbol* temp = NULL;
    if (un.cp) {
	// an offset with a 1 in the first byte signifies an instance
	// high half byte is non zero signifies special type (instance, smt, ast...)
	if ((offset != NOTXREFSYM) && !(offset & 0xF0000000)) { 
	    Xref * Xr = scopeNode_get_xref(un.sc);
	    temp = (xrefSymbol*)(Xr->lxref->get_ind_addr()+offset-1);
	    last_scope = un.sc;
	} else
	    temp = xrefSymbol_priv();
    }
    return temp;
}


xrefSymbol* symbolPtr::xrefSymbol_priv() const
//  call with a symbolPtr which does *not* have a null pointer, 
    // and is not an xrefSymbol (it needs to be converted)
// (private, used only within the symbolPtr class by the inline operator)
{
    xrefSymbol *xr_ptr = get_or_find_xrefSymbol ( 0 );     // get
    return xr_ptr;
}

xrefSymbol* symbolPtr::get_or_find_xrefSymbol (int is_find) const
//  call with a symbolPtr which does *not* have a null pointer, 
    // and is not an xrefSymbol (it needs to be converted)
// (private, used only within the symbolPtr class by the inline operator)
{
    Initialize(symbolPtr::get_or_find_xrefSymbol__int);
    xrefSymbol* retval = NULL;
    Relational* rel = RelationalPtr (un.rp);

    symbolPtr sym;
    int success = rel->construct_symbolPtr(sym);
    if (success && sym.isnotnull()) {
	if (sym.relationalp()) {
	    ddSymbol * ds = checked_cast(ddSymbol, sym);
	    if (ds) {
		if (is_find)
		    sym = ds->find_xrefSymbol();
		else
		    sym = ds->get_xrefSymbol();
	    }
	} 
    } else 
      if (is_instance()) 
	sym = get_xrefSymbol();
    
    
    if (sym.is_xrefSymbol())
	retval = sym;

	
    return retval;
}

symbolPtr symbolPtr::find_xrefSymbol () const
{
    symbolPtr xrsym = NULL_symbolPtr;
    if (un.cp) {
	if (is_xrefSymbol ())
	    xrsym = *this;
	else {
	    Initialize(symbolPtr::find_xrefSymbol);
	    IF_DEBUGGING(is_consistent());
	    xrefSymbol *xr_ptr = get_or_find_xrefSymbol ( 1 );  // find
	    if (xr_ptr || xr_ptr->xrisnotnull())
		xrsym = symbolPtr(xr_ptr);
	}
    }
    return xrsym;
}
    
symbolPtr
symbolPtr::get_xrefSymbol() const
{
  symbolPtr xrsym = NULL_symbolPtr;

  if (un.cp) {
    if (is_xrefSymbol())
      xrsym = *this;
    else {
      Initialize(symbolPtr::get_xrefSymbol);
      IF_DEBUGGING(is_consistent());

      if (is_instance())
	xrsym = sym_of_instance();
      else if (is_ast())
	xrsym = sym_of_astNode();
      else if (is_dataCell()) {
	dataCell *cell = dataCell_of_symbol(*this, false);

	if (cell) {
	  xrsym = dataCell_get_xrefSymbol(cell);
	  dataCell_offload(cell, false);
	}
      } else {
	xrefSymbol *xr_ptr = get_or_find_xrefSymbol(0);    // get

	if (xr_ptr || xr_ptr->xrisnotnull())
	  xrsym = symbolPtr(xr_ptr);
      }
    }
  }

  return xrsym;
}

inline int normalize(int arg)
//  return -1, 0 or +1, depending on sign of argument
{
    return (arg>0) -(arg<0);			  
}

int get_file_fn(const symbolPtr& symbol, genString& gs)
{
    int retval = 0;
    projNode* proj = NULL;

    Xref* xr = symbol.get_xref();
    char const *nm = symbol.get_name();
    if (xr)
	proj = xr->get_projNode();
    if (proj)
	proj->ln_to_fn(nm, gs);
    if (gs.length() == 0)
	gs = nm ? nm : "";
    retval = gs.length();

    return retval;
}
int  symbolPtr::sym_compare(const symbolPtr secondsym) const
{
  return sym_compare_mp(secondsym, 0);
}

int  symbolPtr::sym_compare_mp(const symbolPtr secondsym, int mp) const

// compare "this" with "second", to check two symbols for
//    being equivalent (kind, name, and def_file)
//  5 values may be returned:
//    2  or -2 mean that the two syms differ in either kind or name,
//        or that both have def_files, and the def_file are different
//        -2 means that "this" is "less than" secondsym.
//    1  or -1 mean that the two syms have the same kind & same name,
//        but that exactly one of the symbols has no def_file
//       -1 means that the ~this" sym has a def_file, but the secondsym
//          symbol does not have a def_file
//        1 means that the secondsym symbol has a def_file, but 
//          that "this" symbol does not
//    0 means that the two symbols are identical in all 3 respects
//        or identical in kind & name, and neither has def_file
{
    Initialize(symbolPtr::sym_compare);

    int res = 0;
    symbolPtr first = (symbolPtr)( *this);
    symbolPtr second = (symbolPtr)secondsym;

    if (first == second)
	return 0;

    ddKind first_kind = DD_UNKNOWN;
    ddKind second_kind = DD_UNKNOWN;
    char const *first_name = ""; 
    char const *second_name = "";
    
    appPtr first_hdr = NULL,
	second_hdr = NULL;
    
    appTreePtr first_t = NULL,
	second_t = NULL;

    int is_first_xref = first.is_xrefSymbol ();
    int is_second_xref = second.is_xrefSymbol ();

    first_kind = first.get_kind ();
    second_kind = second.get_kind ();

    int kind_diff = (int)first_kind - (int)second_kind;

    if (kind_diff > 0)
      return 2;
    else if (kind_diff < 0)
      return -2;

    if( first_kind == DD_INSTANCE ){
       MetricSet* MetSet1 = first.get_MetricSetPtr();
       MetricSet* MetSet2 = second.get_MetricSetPtr();
       if( MetSet1 != MetSet2 )
	   return
	     (MetSet1 - MetSet2 < 0) ? -2 : 2;

       unsigned int offset1 = first.get_offset();
       unsigned int offset2 = second.get_offset();

       if( offset1 == offset2 )
	   return 0;

       return ( offset1 < offset2 ) ? -2 : 2; 

    }

    if( first_kind == DD_AST) {
       astXref* axr1 = first.get_astXref();
       astXref* axr2 = second.get_astXref();
       if( axr1 != axr2)
	   return
	     (axr1 - axr2 < 0) ? -2 : 2;

       unsigned int offset1 = first.get_offset();
       unsigned int offset2 = second.get_offset();

       if( offset1 == offset2 )
	   return 0;

       return ( offset1 < offset2 ) ? -2 : 2; 

    }

    if (first_kind == DD_DATA) {
      GDGraph *graph1 = first.get_graph();
      GDGraph *graph2 = second.get_graph();

      if (graph1 != graph2)
	return
	  (((char *) graph1) - ((char *) graph2) < 0) ? -2 : 2;

      unsigned int offset1 = first.get_offset();
      unsigned int offset2 = second.get_offset();

      if (offset1 == offset2)
	return 0;

      return
	(offset1 < offset2) ? -2 : 2;
    }

    if (first_kind != DD_UNKNOWN) {
      first_name = first.get_name ();
      if (!first_name) 
	first_name = "";
      second_name = second.get_name ();
      if (!second_name)
	second_name = "";

      int name_diff = strcmp (first_name, second_name);
      if(name_diff)
        return 2* normalize( name_diff);

      if (first_kind == DD_SEMTYPE)
	  return 0;

      char const *first_file = "";
      char const *second_file = "";
      genString first_file_gs;
      genString second_file_gs;

      symbolPtr first_symbol = first.get_xrefSymbol();
      if(!first_symbol.xrisnull() && first_symbol.get_has_def_file(1)){
	  first_symbol = first_symbol->get_def_file();
	  if(!first_symbol.isnull()){
	      if (mp) {
		  get_file_fn(first_symbol,first_file_gs);
		  first_file = first_file_gs;
	      } else
		  first_file = first_symbol.get_name();
	  }
      }
      symbolPtr second_symbol = second.get_xrefSymbol();
      if(!second_symbol.xrisnull() && second_symbol.get_has_def_file(1)){
	second_symbol = second_symbol->get_def_file();
	if(!second_symbol.isnull()) {
	  if(mp) {
	    get_file_fn(second_symbol, second_file_gs);
	    second_file = second_file_gs;
	  } else
	    second_file = second_symbol.get_name();
	}
      }
      if (first_kind == DD_MODULE) {
	Xref * cxr = projNode::get_control_project()->get_xref();
	if ((second_symbol.get_xref() == cxr) || (first_symbol.get_xref() == cxr))
	  return 0;
      }
      // if both have def files, or neither does, compare the strings
      if ((*first_file=='\0') == (*second_file=='\0')) {
	int result = strcmp(first_file, second_file);
	if (result == 0){
	  int mode = scope_get_global_mode();
	  if(mode == 0) {
	    return 0;
          } else if(mode < 2 && first_file && first_file[0]) {
	    return 0;
	  } else if (is_first_xref && is_second_xref) {
	    scopeNode * sc1 = first.get_scope();
	    scopeNode * sc2 = second.get_scope();
	    scopeRoot * rt1 = NULL;
	    scopeRoot *rt2 = NULL;
	    if (sc1 && !is_Xref(sc1))
		rt1 = checked_cast(scopeRoot,sc1->get_root());
	    if (sc2 && !is_Xref(sc2))
		rt2 = checked_cast(scopeRoot,sc2->get_root());
	    if (rt1 < rt2)
		result = -1;
	    else if (rt1 > rt2)
		result = 1;
	  }
	}
        return 2* normalize(result);    
      }
      // If we got to this point, the kinds are the same, the names are the same,
      //    and exactly one of the defining files is missing.  So we will 
      //    consider the one that has a defining file to be smaller than the other.
      //  ie.  if first symbol has a defining file, return -1
      return (*first_file)? -1 : +1;
    }
    
    if (!is_first_xref) {
	Relational* first_obj = first;
	if (first_obj && is_appTree(first_obj)) {
	    first_t = checked_cast(appTree,first_obj);
	    first_hdr = first_t->get_header();
	}
    }

    if (!is_second_xref) {
	Relational* second_obj = second;
	if (second_obj && is_appTree(second_obj)) {
	    second_t = checked_cast(appTree,second_obj);
	    second_hdr = second_t->get_header();
	}
    }

    if (first_t && second_t && first_hdr == second_hdr) {
      if (first_t != second_t) {
	int yes_before = first_t->is_before(second_t);
	if (yes_before)
	  res = -1;
	else
	  res = 1;
      }
    }

    return res;
}
char const *dfa_get_name(const symbolPtr& sym);
char const *symbolPtr::get_name () const
{
  char const *nm = NULL;
  if (isnotnull()) 
    {
      if (relationalp()) 
	{
	  Relational *rel = (Relational *)un.rp;
	    nm = rel->get_name();
	} 
      else if (is_instance()) 
	{
	  symbolPtr sym = get_xrefSymbol();
	  if (sym.xrisnotnull())
	    nm = sym.get_name();
	} 
      else if(is_ast() || is_dataCell()) 
	{
	  nm = dfa_get_name(*this);
	} 
      else 
	{
	  xrefSymbol *xr_sym = operator->();
	  if (xr_sym) {
	    Xref *Xr = get_xref();
	    nm = xr_sym->get_name(Xr->get_lxref());
	  }
	}
    }
  return nm ? nm : "";
}

int symbolPtr::is_xrefSymbol () const 
{ 
    IF_DEBUGGING(is_consistent());
    // only need the fancier test if "NOTXREFSYM" is nonzero ??????
    return un.cp ? ((offset != NOTXREFSYM) && !(offset & 0xF0000000)) : 0;
//    return (offset != NOTXREFSYM);
//  return  un.cp ? (*(un.cp + sizeof (int)) != 0) : 0; 
}

int
symbolPtr::is_instance() const 
{ 
  IF_DEBUGGING(is_consistent());
  return
    un.cp ? (offset & 0x10000000) : 0;
}

int
symbolPtr::is_smt() const
{
  IF_DEBUGGING(is_consistent());
  return
    un.cp ? (offset & 0x20000000) : 0;
}

int
symbolPtr::is_ast() const
{
  IF_DEBUGGING(is_consistent());
  return
    un.cp ? (offset & 0x40000000) : 0;
}

int
symbolPtr::is_dataCell() const
{
  IF_DEBUGGING(is_consistent());
  return
    un.cp ? (offset & 0x80000000) : 0;
}

void symbolPtr::print(ostream &os, int level) const
{
  Initialize(symbolPtr::print(ostream &, int) const);
  IF_DEBUGGING(is_consistent());

  if (!un.cp) {
    os << "<symbolPtr 0x0) >" << endl;
  } else if (is_ast()) {
    astNode_print(*this, os, level);
  } else if (is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(*this, false);

    if (cell) {
      dataCell_print(cell, os, level);
      dataCell_offload(cell, false);
    }
  } else if (is_xrefSymbol()) {
    symbolPtr foo = *this;
    foo->print(get_xref()->get_lxref(), os, level);
  } else if (relationalp()) {
    ((Relational *) un.rp)->Relational::print(os, level);
    os << endl;
    gen_print_indent (os, level);
    ((Relational *) un.rp)->print(os, level);
    os << endl;
  } else 
    os << "<ObjPtr " << un.rp << " >" << endl;
}

ddElement * symbolPtr_get_def_dd(symbolPtr & that);

ddElement* symbolPtr::get_def_dd ()
{
    Initialize (symbolPtr::get_def_dd);
    IF_DEBUGGING(is_consistent());
    
    return symbolPtr_get_def_dd(*this);
}

app* symbolPtr::get_def_app()
{
    Initialize(symbolPtr::get_def_app);
    IF_DEBUGGING(is_consistent());
    app *h = NULL;

    if (relationalp()) {
	Relational *rel = (Relational *)un.rp;
	h = rel->get_symbolPtr_def_app();
    } else {
	ddKind ki = get_kind ();
	if (ki == DD_PROJECT) {
	    char const *nm = get_name();
	    projNode* pn = projNode::find_project(nm);
	    if (pn)
		h = checked_cast(app,pn->get_header());
	}
    }

    if (!h) {
      ddElement * dd = get_def_dd();
      if (dd)
	h = appTreePtr(dd)->get_main_header();
    }
    return h;
}

// isnull() and isnotnull() test the pointer that this symbolPtr represents
//     for null
bool  symbolPtr::isnotnull() const
{
	return (un.cp != NULL);
}

bool  symbolPtr::isnull() const
{
    return (un.cp == NULL);
}
//  the xrisnull() and xrisnotnull() member functions convert to
//     an xrefSymbol before testing whether it'll be a null
bool symbolPtr::xrisnotnull() const
{
    IF_DEBUGGING(is_consistent());
    bool res = false;
    if (!without_xrefp())
	res = ((operator xrefSymbol*()) != NULL);

    return res;
}

bool symbolPtr::xrisnull() const
{
    IF_DEBUGGING(is_consistent());
    bool res = true;
    if (!without_xrefp())
	res = ((operator xrefSymbol*()) == NULL);

    return res;
}

void symbolPtr::add_link_extend(linkType lt, symbolPtr sym, int rev) const
// add a pair of links between this & sym,
// extending the pmod if needed
{
    //  Note:  could be more efficient if there were an entry point to add_link()
    //  that took an xref as an extra argument.  Could pass  this->get_xref().
    Xref* xr = get_xref();
    xr->make_room(100);
    (*this)->add_link(lt, sym, rev,sym.get_xref()->get_lxref());
}

// constructors
   symbolPtr::symbolPtr(Xref* Xrefptr, xrefSymbol* sym)
// Construct symbolPtr from xrefSymbol and a suggested Xref
//    If the Xref is zero, or incorrect, then figure it out
//       outselves.
//    If the symbol is zero, set up a null symbolPtr
{
//  Initialize(symbolPtr::symbolPtr(Xref *, xrefSymbol *));

  static int flag_last_error;
  
    if(sym) {
	if (last_scope && symbolPtr_is_symbol_in(sym,last_scope))
	    un.sc = last_scope;
	else if (Xrefptr && sym->is_symbol_in(Xrefptr))
	    un.sc = Xrefptr;
	else
	    un.sc = sym->get_Xref();
	if (un.sc == NULL)
	{
	    if(!flag_last_error)
	      msg("WARN: The Information Model may be corrupted. Please rebuild the model. If these persist please call SET support.") << eom;	     
	    flag_last_error=1;
	}
	else
	{
	    flag_last_error=0;
	}
	Xref * Xr = scopeNode_get_xref(un.sc);
	offset = sym->get_symbol_index();
    } else {
	//  if no xrefSymbol, do not save Xref either
	un.cp = NULL;
	offset=NOTXREFSYM;
    }
    IF_DEBUGGING(is_consistent());
}

Xref * symbolPtr::get_xref() const
{
    Xref * retval = 0;
    symbolPtr sym = get_xrefSymbol();
    scopeNode * sc = sym.get_scope();
    if (sc)
	retval = scopeNode_get_xref(sc);

    return retval;
}

symbolPtr::symbolPtr (xrefSymbol* sym)
{
    char const *_my_name = "symbolPtr::symbolPtr(xrefSymbol *)";

    if (sym) {
	if (last_scope && symbolPtr_is_symbol_in(sym,last_scope))
	    un.sc = last_scope;
	else
	    un.sc = sym->get_Xref();
	if (un.sc == NULL)
	    msg("WARN: The Information Model may be corrupted. Please rebuild the model. If these persist please call SET support.") << eom;
	Assert (un.sc);
	Xref * Xr = scopeNode_get_xref(un.sc);
        offset = sym->get_symbol_index();
    } else {
    // the xrefSymbol was null
    //  so we can't store a valid value here
      un.sc = NULL;
      offset=NOTXREFSYM;
    }
    IF_DEBUGGING(is_consistent());

}

symbolPtr::symbolPtr (MetricElement* mel, MetricSet* mset)
{
  Initialize("symbolPtr::symbolPtr(MetricElement*, MetricSet*)");

    if (mel && mset) {
	un.mp = mset;
        offset = mel->get_offset();
	offset |= 0x10000000;  // mark offset as instance, not xref

    } else {
      un.cp = NULL;
      offset=NOTXREFSYM;
    }
    IF_DEBUGGING(is_consistent());
}

extern "C" void node_prtS( symbolPtr& sym)
{
    sym.node_prtS();
}

void  symbolPtr::node_prtS()
{
    if (is_ast() || is_xrefSymbol()) {
	msg("\n====== node_prt:\n") << eom;
	print ();
    } else if (is_dataCell()) {
        print();
    } else {
	Obj* obj = operator Obj* ();
	node_prt (obj);
    }
}

projNode *symbolPtr::get_projNode()
{
    Initialize(symbolPtr::get_projNode);
    projNode *pr = NULL;
    if (isnotnull()) {
	if (without_xrefp()) {
	    Relational *rel = (Relational *)un.rp;
	    if (rel) {
		if (is_projNode(rel))
		    pr = (projNode *)rel;
		else if (is_projModule(rel)) {
		    projModule *mod = (projModule *)rel;
		    pr = mod->get_project();
		} else if (is_projHeader(rel)) {
		    projHeader *hdr = (projHeader *)rel;
		    pr = checked_cast(projNode,hdr->get_root());
		}
	    }
	} else {
	    if (xrisnotnull()) {
		xrefSymbol *xsym = operator->();
		XrefTable* Xr= get_xref()->lxref;
		if (xsym)
		    pr = xsym->get_projNode(Xr);
	    }
	}
    }
    return pr;
}

Relational *symbolPtr::get_noxref_def () const
{
//    Initialize(symbolPtr::get_noxref_def__const);
    Relational *rel = 0;    //boris: for projNode or projModule or projHeader
    if (isnotnull() && without_xrefp()) {
	Relational *loc_rel = (Relational *)un.rp;
	if (loc_rel->is_symbolPtr_def())
	    rel = loc_rel;
    }
    return rel;
}
     
ddKind symbolPtr::get_kind () const
{
//    Initialize(symbolPtr::get_kind__const);
    ddKind ret = DD_UNKNOWN;
    if (is_instance())
	ret = DD_INSTANCE;
    else if (is_ast())
	ret = DD_AST;
    else if (is_dataCell())
	ret = DD_DATA;
    else if (isnotnull()) {
        Relational *rel = 0;
	if (relationalp() && (rel = get_noxref_def())) {
	    ret = rel->get_symbolPtr_kind();
	} else {
	    xrefSymbol *x_sym = operator->();
	    if (x_sym)
		ret = x_sym->get_kind();
	}
    }
    return ret;
}

int symbolPtr::get_language ()
{
    Initialize(symbolPtr::get_language);
    int lang =  FILE_LANGUAGE_UNKNOWN;
    if (isnotnull()) {
	Relational *rel = NULL;
	if (relationalp() && (rel=get_noxref_def()) && is_projModule(rel)) {
	    projModule *mod = checked_cast(projModule,rel);
	    lang = mod->language();
	} else {
	    xrefSymbol *x_sym = operator->();
	    if (x_sym) {
	      Xref* Xr = this->get_xref();
	      XrefTable* Xr_t = Xr->get_lxref();
	      lang = x_sym->get_language(Xr_t);
	    }
	}
    }

    return lang;
}

int symbolPtr::get_language_local ()
{
    Initialize(symbolPtr::get_language_local);
    int lang =  FILE_LANGUAGE_UNKNOWN;
    if (without_xrefp()) {
	Relational *rel = (Relational *)un.rp;
	if (is_projModule(rel)) {
	    projModule *mod = checked_cast(projModule,rel);
	    lang = mod->language();
	}
    } else {
        Xref* Xrf = get_xref();
        XrefTable* Xr = Xrf != NULL ? Xrf->get_lxref() : NULL;
	xrefSymbol *x_sym = operator->();
	if (x_sym)
	    lang = x_sym->get_language_local(Xr);
    }

    return lang;
}

bool symbolPtr::without_xrefp () const
{
//    Initialize(symbolPtr::without_xrefp__const);
    bool res = false;
    if (relationalp()) {
	Relational *rel = (Relational *)un.rp;
	res = rel->is_symbolPtr_def();
    }
    return res;
}

symbolPtr::operator app* ()
{
    Initialize(symbolPtr::operatorappPtr);
    app * header = NULL;
    Relational * node = operator Relational* (); 
    if(!node) return NULL;
    if(is_app(node)) return appPtr(node);
    if(is_ddSymbol(node)){
       ddSymbol *dd = ddSymbolPtr(node);
       header = dd_get_appHeader(dd);
       if(!header)
          header = dd->get_main_header();      
    } else if(is_appTree(node)){
        header = appTreePtr(node)->get_header();        
    }
    return header;
}

symbolPtr symbolPtr::get_def_symbol(int mode)
{
    Initialize(symbolPtr::get_def_symbol);

    symbolPtr def_symbol;

    if (is_xrefSymbol()) {
	if (is_def())
	    def_symbol = *this;
	else {
	    objArr & pmod_list = symbolPtr_get_projs_to_search();
	    def_symbol = get_def_symbol (pmod_list, mode);
	}
    } else if(get_kind() == DD_MODULE) {
      symbolPtr xsym = get_xrefSymbol();
      if(xsym.isnotnull())
	def_symbol = xsym.get_def_symbol();
    }
    return def_symbol;
}

int symbolPtr::get_def_symbols(symbolArr & result, int  mode)
{
    Initialize(symbolPtr::get_def_symbols);

    if (is_def()) {
	result.insert_last(*this);
    } else {
	mode = scope_get_global_mode ();

	if (mode == 0) {
	    symbolPtr def = get_def_symbol();
	    if (def.isnotnull() && def.is_def())
		result.insert_last(def);
	} else if (mode == 1) {
	    objArr units;
	    get_units(units);
	    
	    Obj * el;
	    
	    ForEach(el,units) {
		scopeUnit *unit = checked_cast(scopeUnit,el);
		unit->get_def_symbols(*this, result);
	    }
	} else if (mode == 2) {
	    objArr scopes;
	    get_scopes(scopes);
	    
	    Obj * el;
	    
	    ForEach(el,scopes) {
		scopeRoot * sroot = checked_cast(scopeRoot,el);
		sroot->get_def_symbols(*this, result);
	    }
	}
    }

    return result.size();
}

xrefSymbol *symbolPtr::lookup_all_for_project(projNode *pr, symbolArr& as, ddKind knd, char const *name,
				      xrefSymbol *def_file)
{
    Initialize(symbolPtr::lookup_all_for_project);
    xrefSymbol *found = 0;
    if (pr) {
	Xref * Xr = pr->get_xref(1);
	XrefTable * xr = Xr->get_lxref();
	if (xr)
	    found = xr->lookup(as,knd,name,def_file);
	    
    }
    return found;
}

symbolPtr symbolPtr::get_def_symbol_for_project(projNode *pr, ddKind knd, char const *name, xrefSymbol *def_file)
{
    Initialize(symbolPtr::get_def_symbol_for_project);
    symbolPtr def_symbol = NULL_symbolPtr;
    symbolArr as;
    xrefSymbol *found = symbolPtr::lookup_all_for_project (pr, as, knd, name, def_file);
    if (found && as[0].is_def())
	def_symbol = as[0];

    return def_symbol;
}

symbolPtr symbolPtr::get_def_symbol_for_leaf(scopeLeaf *lf, ddKind knd, char const *name, xrefSymbol *def_file)
{
    Initialize(symbolPtr::get_def_symbol_for_leaf);
    symbolPtr def_symbol = NULL_symbolPtr;
    projNode *pr = NULL;
    if (lf) pr = clone_get_proj(lf);
    if (pr) {
	symbolArr as;
	xrefSymbol *found = symbolPtr::lookup_all_for_project (pr, as, knd, name, def_file);
	if (found) {
	    scopeUnit *un = lf->unit();
	    if (!un->is_hidden_symbol(as[0]) && as[0].get_has_def_file()) {
		def_symbol = as[0];
		def_symbol.put_scope(lf);
	    }
	}
    }
    return def_symbol;
}

symbolPtr symbolPtr::get_def_symbol(objArr& pmod_list, int mode)
{
    return get_def_symbol (pmod_list, (xrefSymbol *)NULL, mode);
}

symbolPtr symbolPtr::get_def_symbol(objArr& pmod_list, xrefSymbol *def_file, int /* mode */)
{
    Initialize(symbolPtr::get_def_symbol);
    symbolPtr def_symbol = NULL_symbolPtr;
    if (is_xrefSymbol() && pmod_list.size()) {
	Xref * Xr_this = get_xref();
	ddKind knd = get_kind();
	char const *name = get_name();
	Obj * ob;
	ForEach(ob,pmod_list) {
	    if (is_projNode((Relational*)ob)) {
		projNode *pr = checked_cast(projNode, ob);
		def_symbol = symbolPtr::get_def_symbol_for_project(pr, knd, name, def_file);
	    } else {
		scopeLeaf *lf = checked_cast(scopeLeaf,ob);
		def_symbol = symbolPtr::get_def_symbol_for_leaf(lf, knd, name, def_file);
	    }
	    if (def_symbol.isnotnull())
		break;
	}
    }
    return def_symbol;
}

static int symbolPtr_get_exported_scopes(symbolPtr & sym, objArr & result)
{
    Initialize(symbolPtr_get_exported_scopes);
    
    int retval = 0;

    scopeNode *sc = sym.get_scope();
    ddKind knd = sym.get_kind();
    if ((knd == DD_VAR_DECL || knd == DD_FUNC_DECL) && is_scopeLeaf(sc)) {
	objTree *sr = sc->get_root();
	if (is_scopeDLL(sr)) {
	    scopeDLL * dll = (scopeDLL*)sr;
	    if (dll->exported_syms.includes(sym)) {
		Obj * execs = dll_get_execs(dll);
		Obj * ob;
		if (execs)
		    ForEach(ob,*execs) {
			scopeRoot * exe = checked_cast(scopeRoot,ob);
			if (!exe->is_hidden())
			    result.insert_last(ob);
		    }
	    }
	}
    }
    
    retval = result.size();
    return retval;
}

int symbolPtr::get_all_symbols(symbolArr & result, int mode)
{
    Initialize(symbolPtr::get_all_symbols);

    int retval = 0;

    mode = scope_get_global_mode();

    if (mode == 0) {
	objArr & pmod_list = symbolPtr_get_projs_to_search();
	get_all_symbols (result, pmod_list, mode);
    } else if (mode == 1) {
	objArr units;
	get_units(units,1);
    
	Obj * el;
	
	ForEach(el,units) {
	    scopeNode * sc = checked_cast(scopeNode,el);
	    sc->lookup_all(*this, result);
	}
    } else if (mode == 2) {
	objArr scopes;
	get_scopes(scopes);
	symbolPtr_get_exported_scopes(*this,scopes);

	Obj * el;
	
	ForEach(el,scopes) {
	    scopeNode * sc = checked_cast(scopeNode,el);
	    sc->lookup_all(*this, result);
	}
    }
    retval = result.size();
	
    return retval;
}

int symbolPtr::get_all_symbols(symbolArr & result, objArr& pmod_list, int /* mode */)
{
    Initialize(symbolPtr::get_all_symbols__symbolArr&__objArr&);

    int retval = 0;

    if (is_xrefSymbol() && pmod_list.size()) {
	ddKind knd = get_kind();
	char const *name = get_name();
	symbolPtr dfs;
	symbolArr ds;
	if (get_local_link(is_defined_in,ds))
	    dfs = ds[0];

	Obj * ob;
	ForEach(ob,pmod_list) {
	    if (is_projNode((Relational*)ob)) {
		projNode *pr = checked_cast(projNode, ob);
		Xref * Xr = pr->get_xref(1);
		XrefTable * xr = Xr->get_lxref();
		if (xr) {
		    symbolArr as;
		    if (xr->lookup(as,knd,name,dfs))
			result.insert_last(as[0]);
		}
	    } else {
		scopeLeaf * sc = checked_cast(scopeLeaf,ob);
		projNode *pr = clone_get_proj(sc);
		Xref * Xr = (pr) ? pr->get_xref(1) : NULL;
		XrefTable * xr = (Xr) ? Xr->get_lxref() : NULL;
		if (xr) {
		    symbolArr as;
		    if (xr->lookup(as,knd,name,dfs)) {
			as[0].put_scope(sc);
			result.insert_last(as[0]);
		    }
		}
	    }
	}
    }

    retval = result.size();

    return retval;
}

int symbolPtr_add_projs_to_search(objArr& obs, projNode *pn)
{
    Initialize(symbolPtr_add_projs_to_search);

    int retval = 0;
    int init_size = obs.size();

    projNode* curproj=pn;
    while (curproj && !curproj->get_xref(1))
	curproj=checked_cast(projNode, curproj->find_parent());
    Xref* xr = (curproj) ? curproj->get_xref(1) : 0;
    if (xr && xr->get_lxref() && !xr->is_unreal_file()) {
	// if project has an xref (pmod)
	obs.insert_last(curproj);
    } else  { // project does not have an xref
	pn->refresh_projects(); // so lets get all its children
	Obj*  os = parentProject_get_childProjects(pn);
    	Obj* ob;
    	ForEach(ob, *os) {
	    projNode* pr = checked_cast(projNode, ob);
	    symbolPtr_add_projs_to_search(obs, pr);
	}
    }

    retval = obs.size() - init_size;

    return retval;
}

objArr &  symbolPtr_get_projs_to_search()
{
    Initialize(symbolPtr_get_projs_to_search);

    if (!pmod_search_list.size()) {
	if (xref_get_doing_second_pass()) {
	    projNode * pr = projNode::get_home_proj();
	    symbolPtr_add_projs_to_search(pmod_search_list,pr);
	} else {
	    projNode* cpr = projNode::get_control_project();
	
	    projNode * pr;
	    objSet os;
	    ForEachProj(ii,pr)
		if (pr != cpr && !os.includes(pr)) {
		    os |= pr;
		    symbolPtr_add_projs_to_search(pmod_search_list,pr);
		}
	}
    }

    return pmod_search_list;
}

int is_template_symbol(char const *name)
{
  bool retval = 0; //  FALSE
  int open = 0;
  
  if (name) 
    {
      for(; *name; ++name)
	if (!open)
	  {
	    if (*name == '<')
	      open = 1;
	  }
	else if (*name == '>')
	  {
	    retval = 1; // TRUE
	    break;
	  }
    }
      
  return retval;
}
      

int symbolPtr::get_link(linkType lt, symbolArr& as)
{
    Initialize(symbolPtr::get_link);

    int retval = 0;
    
    SearchType s_type = (SearchType)linkTypes::search_from[lt];

    if (s_type ==  FROM_DEF_SYMBOLS && get_kind () == DD_PACKAGE)
      s_type =  FROM_ANY_SYMBOLS;
 
    switch (s_type)
      {
      case FROM_DEF_SYMBOLS:
	{
	  symbolArr defs;
	  int nn = get_def_symbols(defs);
	  if (nn > 0) 
	    { 
	      symbolPtr def;
	      ForEachS (def,defs)
		{
		  if (def.isnotnull())
		    def.get_local_link(lt,as);
		}
	    } 
	  else if (is_template_symbol(get_name())) 
	    {
	      get_local_link(lt,as);
	    }
	}
	break;

      case FROM_ALL_SYMBOLS:
	{
	  symbolArr refs;
          symbolArr tmp;
	  get_all_symbols(refs);
	  symbolPtr ref,el;
	  ForEachS(ref,refs)
	    ref.get_local_link(lt,tmp);
	  ForEachS(el,tmp)
	    if(is_template_symbol(el.get_name()) 
	       || el.is_def() || el.get_kind() == DD_PACKAGE)
	      as.insert_last(el);
	}
	break;
	
      case FROM_ANY_SYMBOLS:
	{
	  symbolArr defs;
	  int nn = get_def_symbols(defs);
	  if (nn > 0) 
	    { 
	      symbolPtr def;
	      ForEachS (def,defs)
		{
		  if (def.isnotnull())
		    def.get_local_link(lt,as);
		}
	    } 
	  else 
	    {
	      get_local_link(lt,as);
	    }
	}
	break;

      default:
	break;
      }
    
    retval = as.size();
    return retval;
}

void get_link(symbolArr& arr, linkTypes& la, symbolArr& result)
{
    /* server */
    Initialize(get_link(symbolArr&,linkTypes&,symbolArr&));
    
    int sz = arr.size();
    linkType lt = la;

    for (int ii = 0; ii < sz; ++ii) {
	symbolPtr & sym = arr[ii];
	sym.get_link(lt, result);
    }
}

int symbolPtr::get_local_link(linkType lt, symbolArr& as)
{
    Initialize(symbolPtr::get_local_link);

    int retval = 0;
    xrefSymbol * sym = operator->();
    if (sym) {
        Xref *Xr = get_xref();
	if (!(lt == is_defined_in && is_def() == 0)) {
	    Xref* Xr = get_xref();
	    retval = sym->get_local_link_internal (lt, as, Xr->get_lxref());
	}
    }
    return retval;
}

int
ATT_strange(symbolPtr &sym)
{
  Initialize(ATT_strange(symbolPtr &));

  if (!sym.is_xrefSymbol())
    return 0;

  int retval = 0;
  xrefSymbol * xsym = sym.operator->();

  if (sym.get_has_def_file()) {
    bool is_reference;
    symbolArr def_files;

    // look only in current pmod for def file symbol:
    if (xsym->get_link(is_defined_in, def_files, 1)) {
      symbolPtr dfs = def_files[0];
      is_reference = !dfs.get_has_def_file();

      if (is_reference)
	retval = 1;
    } else {  
      is_reference = true;
      retval = 2;
    }
  }

  return retval;
}

int symbolPtr::get_define(symbolArr& as, const ddSelector & sel)
{
  Initialize(symbolPtr::get_define);

  symbolArr  nsa;
  linkType link;
  if (sel.want_used==0) 
    link = is_defining;
  else if (get_kind() == DD_MODULE)
    link = file_ref;
  else
    link = is_using;

  get_link(link, nsa); 

  int cnt = 0;
  for (int i = nsa.size() - 1; i >= 0; --i) {
    symbolPtr & sym = nsa[i];
    ddKind kind = sym.get_kind();
    if (sel.selects(kind)){
      as.insert_last(sym);
      cnt++;
    } 
    if (sel.want_used == 0 && kind == DD_TEMPLATE && sel.selects(DD_CLASS)) {
      symbolArr users;
      if (sym.get_link(template_of,users) || sym.get_link(used_by, users)) {
	symbolPtr user;
	ForEachS(user, users) {
	  if (user.get_kind() == DD_CLASS) {
	    as.insert_last(user);
	    cnt++;
	  }
	}
      }
    }
  }
  return cnt;
}

static int symbolPtr_add_imported(symbolPtr & sym, objArr & result, objSet &os)
{
    Initialize(symbolPtr_add_imported);

    int retval = 0;

    ddKind knd = sym.get_kind();
    if (knd == DD_VAR_DECL || knd == DD_FUNC_DECL) {
	Obj * ob;
	ForEach (ob,result) {
	    Relational * sr = (Relational*)ob;
	    if (is_scopeEXE(sr)) {
		scopeEXE *se = (scopeEXE*)sr;
		symbolArr temp;
		se->scopes_imported(temp);
		objArr dlls = temp;
		Obj *el;
		ForEach(el,dlls) {
		    scopeDLL * dll = checked_cast(scopeDLL,el);
		    symbolPtr * ptr;
		    if (ptr = (symbolPtr*)dll->exported_syms.includes(sym)) {
			if (!os.includes(el)) {
			    os.insert(el);
			    result.insert_last(el);
			    break;
			}
		    }
		}
	    }
	}
	retval = result.size();
    }
    return retval;
}

int symbolPtr::get_scopes(objArr & result, int /* mode */)
{
    Initialize(symbolPtr::get_scopes);

    int retval = 0;
    Relational *sc = NULL;
    if (is_xrefSymbol()) {
	sc = get_scope();
	if (is_Xref(sc)) {
	    Xref * Xr = (Xref*)sc;
	    sc = Xr->get_projNode();
	}
    } else
	sc = RelationalPtr (un.rp);

    objArr temp;
    objSet os;
    retval = scopeMgr::get_scopes_of(sc, temp);
    Obj * ob;
    ForEach(ob,temp) {
	scopeRoot *sroot = checked_cast(scopeRoot,ob);
	if (!sroot->is_hidden()) {
	    result.insert_last(ob);
	    os.insert(ob);
	}
    }

    if (is_xrefSymbol())
	symbolPtr_add_imported(*this, result, os);

    retval = result.size();
    return retval;
}

static int symbolPtr_add_exported(symbolPtr & sym, objArr & result, objSet & os)
{
    Initialize(symbolPtr_add_exported);

    int retval = 0;

    ddKind knd = sym.get_kind();
    if ((knd == DD_VAR_DECL || knd == DD_FUNC_DECL)) {
	Obj *el;
	ForEach(el,result) {
	    scopeRoot * sr = checked_cast(scopeRoot,el); 
	    if (is_scopeDLL(sr)) {
		scopeDLL * dll = (scopeDLL*)sr;
		if (dll->exported_syms.includes(sym)) {
		    Obj * execs = dll_get_execs(dll);
		    Obj * ob;
		    if (execs) {
			ForEach(ob,*execs) {
			    scopeRoot * exe = checked_cast(scopeRoot,ob);
			    if (!exe->is_hidden() && !os.includes(exe)) {
				result.insert_last(ob);
				os.insert(ob);
			    }
			}
		    }
		}
	    }
	}
    }
    
    retval = result.size();
    return retval;
}

int symbolPtr::get_units(objArr & result, int add_exported)
{
    Initialize(symbolPtr::get_units);

    int retval = 0;

    objArr scopes;
    get_scopes(scopes);
    
    objSet os;
    Obj * ob;
    ForEach(ob,scopes)
	os.insert(ob);
    
    if (add_exported) 
	symbolPtr_add_exported(*this, scopes, os);

    symbolArr cloned;
    Obj *el;
    ForEach (el,scopes) {
	scopeNode * sc = checked_cast(scopeNode,el);
	sc->scopes_cloned(cloned);
    }

    symbolArr units;
    symbolPtr sym;
    ForEachS(sym,cloned) {
	if (is_scopeUnit(sym))
	    units.insert_last(sym);
    }
    units.usort();

    result = units;
    retval = result.size();
    
    return retval;
}

scopeUnit *symbolPtr_get_unit (symbolPtr& sym)
{
    Initialize(symbolPtr_get_unit);

    scopeUnit *unit = scope_mgr.unit(sym);
    return unit;
}

int symbolPtr::is_def()
{
    int retval = 0;
    
    if(is_xrefSymbol()) {
	xrefSymbol *sym = operator->();
	if (sym && sym->get_has_def_file(get_xref()->lxref)) {
	  // Works in old way if the mode is 0
	  // boris: Fixing nonlocal projects with right name "/" - slash projects

	  int mode = scope_get_global_mode();
	  if (mode == 0 || ! ATT_hidden(*this) ) {
	    scopeUnit *unit = symbolPtr_get_unit(*this);

//Boris 11/10/97: In batch mode units are not initialized. In batch mode if somebody wants to check if a symbol is hidden or not, update_units() has to be called.
	    if (!unit || !unit->is_hidden_symbol(*this))
	      retval = 1;
	  }
	}
    }

    return retval;
}

int symbolPtr::operator == (const symbolPtr& o2) const
{
    int retval = 0;
    
    if (offset==o2.offset) {
	int mode = scope_get_global_mode();
	if (symbol_is_special(this) || mode == 2 || !offset)
	    retval = (un.cp == o2.un.cp);
	else
	    retval = (get_xref() == o2.get_xref());
    }

    return retval;
}

int symbolPtr::operator != (const symbolPtr& o2) const
{
    return ! (operator==(o2));
}

int astNode_get_line_number(const symbolPtr&);
int astNode_get_char_offset(const symbolPtr&);

int symbolPtr::line_number() 
{
  Initialize("symbolPtr::line_number");
  int result = 0;
  if (is_instance()) {
    MetricElement mel(un.mp->get_table(),offset&0x0fffffff);
    result = mel.get_value(MET_INSTANCE_LINE);
  }
  if (is_ast())
      result = astNode_get_line_number(*this);

  return result;
}
int symbolPtr::char_offset()
{
  Initialize("symbolPtr::char_offset");
  int result = 0;
  if (is_instance()) {
    MetricElement mel(un.mp->get_table(),offset&0x0fffffff);
    result = mel.get_value(MET_INSTANCE_OFFSET);
  }
  if (is_ast() || is_dataCell()) 
      result = astNode_get_char_offset(*this);

  return result;
}
int symbolPtr::line_offset()
{
  Initialize("symbolPtr::line_offset()");
  int result = 0;
  if (is_instance()) {
    MetricElement mel(un.mp->get_table(),offset&0x0fffffff);
    result = mel.get_value(MET_INSTANCE_LN_OFFSET);
  }
  return result;
}
symbolPtr symbolPtr::scope()
{
  Initialize("symbolPtr::scope");
  symbolPtr result = NULL_symbolPtr;
  if (is_instance()) {
    MetricElement mel(un.mp->get_table(),offset&0x0fffffff);
    int scope_offset = mel.get_value(MET_INSTANCE_SCOPE);
    
    Xref* xr = NULL;
    projNode* p = set_get_proj(un.mp);    
    if (p) xr = p->get_xref();
    if (xr) {
      result = (xrefSymbol*)(xr->lxref->get_ind_addr()+scope_offset-1);
    }
  }
  return result;
}

symbolPtr symbolPtr::sym_of_instance() const
{
    Initialize("symbolPtr::sym_of_instance");
    symbolPtr result = NULL_symbolPtr;
    if (is_instance()) {
	MetricElement mel(un.mp->get_table(),offset&0x0fffffff);
	int sym_offset = mel.get_value(MET_INSTANCE_SYMBOL);
	if (sym_offset >= 0) {
	  Xref* xr       = NULL;
	  projNode* p    = set_get_proj(un.mp);    
	  if (p) 
	    xr = p->get_xref();
	  if (xr)
	if (xr) {
	    result = (xrefSymbol*)(xr->lxref->get_ind_addr()+sym_offset-1);
	  }
	}
    }
    return result;
}

void symbolPtr_sym_of_astNode(const symbolPtr& that, symbolPtr& result);
symbolPtr symbolPtr::sym_of_astNode() const
{
    Initialize(symbolPtr::sym_of_astNode);
    symbolPtr result;
    symbolPtr_sym_of_astNode(*this, result);
    return result;
}

MetricElement
symbolPtr::get_MetricElement() const
{
  MetricElement result;

  if (is_instance())
    result = MetricElement(un.mp->get_table(),offset&0x0fffffff);

  return result;
}

MetricSet *
symbolPtr::get_MetricSetPtr() const
{
  MetricSet *result = NULL;

  if (is_instance()) 
    result = un.mp;

  return result;
}

GDGraph *
symbolPtr::get_graph() const
{
  if (is_dataCell())
    return un.graph;
  else
    return NULL;
}

astXref *
symbolPtr::get_astXref() const
{
  astXref *result = NULL;

  if (is_ast())
    result = un.ap;
  else if (is_dataCell()) {
    dataCell *cell = dataCell_of_symbol(*this, false);

    if (cell) {
      const symbolPtr &symbol = dataCell_get_symbol(cell);
      result = symbol.get_astXref();
      dataCell_offload(cell, false);
    }
  }

  return result;
}

void symbolPtr::rm_dup_links()
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  xr_sym->rm_dup_links(Xr->get_lxref());
}

int symbolPtr::get_links(linkTypes &la, symbolArr& as, int rec)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_links(la, as, Xr->get_lxref(), rec);
}

void symbolPtr::get_links(linkType_selector& sel, const objSet& projs, bool chase_typedefs)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  xr_sym->get_links(sel, projs, chase_typedefs, Xr->get_lxref());
}

ddElement* symbolPtr::get_dds(app* ah)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_dds(ah, Xr->get_lxref());
}

void symbolPtr::rm_link(linkType lt)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  xr_sym->rm_link(lt, Xr->get_lxref());
}

time_t symbolPtr::get_last_mod_date()
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_last_mod_date(Xr->get_lxref());
}

void symbolPtr::set_datatype(char dt)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  xr_sym->set_datatype(dt, Xr->get_lxref());
}

int symbolPtr::get_datatype()
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_datatype(Xr->get_lxref());
}

symbolPtr symbolPtr::get_parent_of(const objSet& obs)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_parent_of(obs, Xr->get_lxref());
}

bool symbolPtr::get_has_def_file(int remote)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_has_def_file(Xr->get_lxref(), remote);
}

void symbolPtr::get_files(symbolArr & ar)
{
  Xref* Xr = get_xref();
  xrefSymbol* xr_sym = operator->();
  xr_sym->get_files(ar, Xr->get_lxref());
}

bool symbolPtr::is_loaded_or_cheap()
{
  Xref* Xr = get_xref();
  xrefSymbol* xr_sym = operator->();
  return xr_sym->is_loaded_or_cheap(Xr->get_lxref());
}

int symbolPtr::is_loaded()
{
  Xref* Xr = get_xref();
  xrefSymbol* xr_sym = operator->();
  return xr_sym->is_loaded(Xr->get_lxref());  
}

void symbolPtr::get_equivalence_obj(symbolArr & sa )
{
  Xref* Xr = get_xref();
  xrefSymbol* xr_sym = operator->();
  xr_sym->get_equivalence_obj(sa,Xr->get_lxref());
}

unsigned int symbolPtr::get_xrefsym_offset_real()
{
  unsigned int ret_val = 0;
  if (is_xrefSymbol()) {
    Xref* Xr = get_xref();
    XrefTable* xr_t = Xr->get_lxref();
    if (xr_t)
      ret_val = xr_t->get_sym_offset_from_ind(offset);
  }
  return ret_val;
}

char const *symbolPtr::get_unqual_name()
{
  Xref* Xr = get_xref();
  XrefTable* Xr_t = Xr->get_lxref();
  xrefSymbol *xr_sym = operator->();
  return (Xr_t) ? xr_sym->get_unqual_name(Xr_t) : 0;
}

bool symbolPtr::is_override_for(char const *unqual_name)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->is_override_for(unqual_name, Xr->get_lxref());  
}

int symbolPtr::get_overrides(symbolArr& overrides, bool entire_hierarchy)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_overrides(overrides, entire_hierarchy, Xr->get_lxref());
}

int symbolPtr::get_base_dtors(symbolArr&  dtors)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_base_dtors(dtors, Xr->get_lxref());
}

app* symbolPtr::load_file()
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->load_file(Xr->get_lxref());
}

void symbolPtr::put_signature(ostream& ostr) const
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  xr_sym->put_signature(ostr, Xr->get_lxref());
}

int symbolPtr::get_link(linkType lt, symbolArr& as, const objSet& projs, int rec)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_link(lt, as, projs, Xr->get_lxref(), rec); 
}

int symbolPtr::get_link_chase_typedefs(linkType lt, symbolArr& as, const objSet& proj)
{
  Xref* Xr = get_xref();
  xrefSymbol *xr_sym = operator->();
  return xr_sym->get_link_chase_typedefs(lt, as, proj, Xr->get_lxref());
}
