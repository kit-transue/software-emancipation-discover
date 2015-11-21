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
#include <cLibraryFunctions.h>
#include <msg.h>
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <machdep.h>
#include <genError.h>
#include <gen_hash.h>
#include <symbolPtr.h>
#include <symbolSet.h>
#include <symbolArr.h>
#include <RTL_externs.h>
#include <proj.h>
#include <fileCache.h>
#include <cmd.h>
#include <ddict.h>

#include <linkTypes.h>
#include <link_type_converter.h>
#include <linkType_selector.h>
#include <Link.h>
#include <xrefSymbol.h>
#include <xrefSymbolSet.h>
#include <XrefTable.h>
#include <_Xref.h>
#include <SharedXref.h>
#include <xrefPair.h>
#include <driver_mode.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#define ONEHUNDRED 100

#ifdef _PSET_DEBUG
static int pmodtest_flag = 0;		// flag used to enable
#endif

symbolPtr lookup_xrefSymbol(ddKind kind, char const *fn, char const *root);
int ddKind_item(char const *name);

#define NONAME "<NoName>";

extern "C" xrefSymbol* symbol_prt(char const *kndn, char const *symn, char const *root)
{
   ddKind knd = (ddKind)ddKind_item(kndn);
   symbolPtr sym = lookup_xrefSymbol(knd, symn, root);
   if(sym.isnotnull()){
     node_prtS(sym);
     return  sym.operator->();
   } else {
     msg("symbol $1 not found\n") << symn << eom;
     return 0;
   }
}

static const link_type_converter  link_type_is;

static bool force_global = false;

char const * PSET_PRINT_LINK = OSapi_getenv("PSET_PRINT_LINK");
int get_all_subprojects_from_proj(objSet & obs, projNode* pn, int deep = 0);

extern "C" {
	int test_simple_option(char const *);
}

static int unique_link(linkType lt )
{
  return (lt == has_type);
}

int is_regular_code_link(linkType lt)
  /* these are links that are not duplicated across psets, some of them might
   be, but will be filtered out by is_ambig_kind
   */
{
  int ret_val = 0;
  switch (lt)
  {
  case used_by:
  case is_using :
  case has_superclass:
  case has_subclass:
  case is_defining:
  case is_defined_in:    
  case is_including:
  case included_by:
  case ref_file:
  case file_ref:
  case have_friends:    
  case friends_of:
//  case have_arg_type:
//  case arg_type_of:
  case has_vpub_superclass:
  case has_vpub_subclass:  
  case has_vpri_superclass :
  case has_vpri_subclass:  
  case has_pub_superclass:
  case has_pub_subclass:  
  case has_pri_superclass:
  case has_pri_subclass:  
  case has_vpro_superclass:
  case has_vpro_subclass:  
  case has_pro_superclass:
  case has_pro_subclass:  
  case declared_in:
  case declares:
  case has_template:
  case template_of:
  case has_property:
  case has_possessor:
    ret_val = 1;
    break;
  default:
    break;
  }
  return ret_val;
}

int is_ambig_kind(ddKind k)
  /* kinds where duplicate of links is possible */
{
  return (k == DD_SEMTYPE);
}

inline void xrefSymbol::set_need_to_mv_pmod_to_perm()
{
  XrefTable * xrt = get_XrefTable();
  xrt->set_need_to_mv_pmod_to_perm();
}

/***************************************************************************/
// The following function is called to determine whether a given invocation
// of xrefSymbol::get_link can be satisfied by looking only in the current
// pmod or whether a global scan is required.  There are three conditions
// under which a global scan is required:
//
// 1) The "force_global" flag is set.  This flag is used to support an
// unfortunate choice of linkTypes used to implement user-defined relations:
// the oodt_relation is "used_by" the source class and "is_using" the
// target class.  Since the source, target, and relation can all be in
// different pmods, a global scan is required, even though "is_using" is
// normally satisfied by looking only in the pmod of the definition of the
// symbol.  The "force_global" flag is set when xrefSymbol::get_link is
// called with the "rec" argument set to -1 (normally, a value of 0 is
// supplied and 1 is used to prevent infinite recursion when the function
// is called as part of a global scan).
//
// 2) The symbol is a "reference" symbol, i.e., it was created only to have
// a placeholder for links with a symbol in a different pmod from the one
// that contains the definition of the symbol.  Reference symbols are
// identified by either "has_def_file" being false or by their definition
// file symbol's "has_def_file" being false.  (The latter case arises to
// support group/subsystem references -- the group's symbol will likely
// be in a different pmod from the member's, and the only way to keep
// track of different symbols with the same name is to supply a "reference"
// symbol for the definition file.)
//
// 3) The linkType itself is one that indicates that a global scan is
// needed -- for instance, "used_by" requires a global scan to find all
// the symbols that reference this one.

int xrefSymbol::local_only(linkType lt, int /*size */, XrefTable* Xr)
{ //Initialize(local_only);
   bool result;
   if (force_global) {
      result = false;
   }
   else {
      bool is_reference;
      if (get_has_def_file(Xr)) {
	 if (lt == is_defined_in) {	// prevent infinite recursion
	    is_reference = false;
	 }
	 else {
	    symbolArr def_files;
	    // look only in current pmod for def file symbol:
	    if (get_link(is_defined_in, def_files, 1)) {
	       symbolPtr dfs = def_files[0];
	       is_reference = !dfs.get_has_def_file();
	    }
	    else is_reference = true;
	 }
      }
      else is_reference = true;
      if (is_reference) {
	 result = false;
      }
      else result = linkTypes::local_search[lt];
   }
   return result;
}

struct linkSet : public Set 
{
   public:
    xrefSymbol *symptr;
    XrefTable *xref;

    	virtual bool isEqualObjects(const Object&, const Object&) const;
	virtual unsigned hash_element(const Object*) const;
};

unsigned linkSet::hash_element(const Object *o) const
{
    Link *lnk        = (Link *)o;
    linkType lt      = lnk->lt();
    xrefSymbol *dest = lnk->get_symbol_of_link (xref);
    
    unsigned sym_offset = (dest > symptr) ? dest - symptr : symptr - dest;

    unsigned i = ((lt << 24) | sym_offset);
    return i;
}

bool linkSet::isEqualObjects(const Object& o1, const Object& o2) const
{
    Link *lnk1   = (Link *) &o1;
    Link *lnk2   = (Link *) &o2;
    linkType lt1 = lnk1->lt();
    linkType lt2 = lnk2->lt();

    bool ret = false;
    if (lt1 == lt2) {
	xrefSymbol *dest1 = lnk1->get_symbol_of_link (xref);
	xrefSymbol *dest2 = lnk2->get_symbol_of_link (xref);
	if (dest1 == dest2)
	    ret = true;
    }
    return ret;
}

void xrefSymbol::rm_dup_links(XrefTable* Xr)
{
  Initialize(xrefSymbol::rm_dup_links());
  uint sind = get_symbol_index();
  Link* lnk = get_start_link(sind, Xr);
  if (!lnk) return ;
  linkSet lset;
  lset.symptr = this;
  lset.xref = Xr;

  while (!last_link(lnk)) {
      if (lnk->symbol_link()) {
	  Link *found_lnk = (Link *)lset.findObjectWithKey(*((const Object *)lnk));
	  if (found_lnk)
	      lnk->assign(lt_junk);
	  else
	      lset.add (*((Object *)lnk));
      }
      lnk = get_next_link(lnk, sind, Xr);
  }
}

bool xrefSymbol::is_writable()
{
  Xref* xr = get_Xref();
  return (xr && xr->perm_flag != READ_ONLY);
}

//=============================================
// xrefSymbol::is_override_for: returns true if this is a virtual or pure
// virtual function and either its name matches the unqualified name or
// both names are destructors.
//=============================================

bool xrefSymbol::is_override_for(char const *unqual_name, XrefTable* Xr) {
   Initialize(xrefSymbol::is_override_for);

   if (kind == DD_FUNC_DECL && (get_attribute(VIRT_ATT, 1) ||
				get_attribute(PVIR_ATT, 1))) {
      char const *my_unqual_name = get_unqual_name(Xr);
      if ((*unqual_name == '~' && *my_unqual_name == '~') ||
	  strcmp(unqual_name, my_unqual_name) == 0) {
	 return true;
      }
   }
   return false;
}

//=============================================
// xrefSymbol::get_root_classes_with_virt_fcn: fills array with least-
// derived classes containing specified virtual function and returns
// true if this or one of its base classes contains a definition of
// the function.
//=============================================

bool xrefSymbol::get_root_classes_with_virt_fcn(char const *unqual_name,
						symbolArr& root_classes) {
   Initialize(xrefSymbol::get_root_classes_with_virt_fcn);

   if (root_classes.includes(this)) {
      return true;
   }

   bool found_in_base = false;
   symbolArr bases;
   get_link(has_superclass, bases);
   symbolPtr base;
   ForEachS(base, bases) {
      if (base->get_root_classes_with_virt_fcn(unqual_name, root_classes)) {
	 found_in_base = true;
      }
   }

   if (!found_in_base) {
      symbolArr members;
      get_link(is_using, members);
      symbolPtr mem;
      ForEachS(mem, members) {
	 if (mem.is_override_for(unqual_name)) {
	    root_classes.insert_last(this);
	    return true;
	 }
      }
   }
   return false;
}

//=============================================
// xrefSymbol::get_overrides: fills array with overriding member fcns
// (or, if "entire_hierarchy" is true, all matching virtual functions
// from both base and derived classes) and returns the number of elements
// in the array.
//=============================================

int xrefSymbol::get_overrides(symbolArr& overrides, bool entire_hierarchy, XrefTable* Xr) {
    Initialize(xrefSymbol::get_overrides);

    symbolArr processed;
    if (kind == DD_FUNC_DECL && (get_attribute(VIRT_ATT, 1) ||
				 get_attribute(PVIR_ATT, 1))) {
       char const *unqual_name = get_unqual_name(Xr);
       symbolArr users;
       get_link(used_by, users);
       symbolPtr user;
       ForEachS(user, users) {
    	  ddKind k;
	  if ((k = user->get_kind()) == DD_CLASS || k == DD_INTERFACE) {
	     symbolArr root_classes;
	     if (entire_hierarchy) {
		user->get_root_classes_with_virt_fcn(unqual_name, root_classes);
	     }
	     else root_classes.insert_last(user);
	     symbolPtr root_class;
	     symbolArr processed_classes;
	     ForEachS(root_class, root_classes) {
		root_class->get_overrides_of(unqual_name, overrides,
					     processed_classes);
	     }
	  }
       }
    }
    return overrides.size();
}

//==========================================
// xrefSymbol::get_base_dtors: fills array with self (whether dtor or not) plus
// any dtors from bases of this symbol's class.  Returns size of array.
//==========================================

int xrefSymbol::get_base_dtors(symbolArr& dtors, XrefTable* Xr) {
   Initialize(xrefSymbol::get_base_dtors);

   char const *unqual_name = get_unqual_name(Xr);
   if (*unqual_name == '~') {
      symbolArr users;
      get_link(used_by, users);
      symbolPtr user;
      ForEachS(user, users) {
    	 ddKind k;
	 if ((k = user->get_kind()) == DD_CLASS || k == DD_INTERFACE) {
	    symbolArr processed_classes;
	    user->get_dtors(dtors, processed_classes);
	    return dtors.size();
	 }
      }
   }
   dtors.insert_last(this);
   return 1;
}


//==========================================
// xrefSymbol::get_dtors: fills array with this class's dtor and with those of
// any bases of this class (recursively).
//==========================================

void xrefSymbol::get_dtors(symbolArr& dtors, symbolArr& processed_classes) {
   Initialize(xrefSymbol::get_dtors);

   if (!processed_classes.includes(this)) {
      processed_classes.insert_last(this);
      symbolArr useds;
      get_link(is_using, useds);
      symbolPtr used;
      ForEachS(used, useds) {
	 if (used->get_kind() == DD_FUNC_DECL) {
	    char const *unqual_name = used.get_unqual_name();
	    if (*unqual_name == '~') {
	       dtors.insert_last(used);
	       break;
	    }
	 }
      }
      symbolArr bases;
      get_link(has_superclass, bases);
      symbolPtr base;
      ForEachS(base, bases) {
	 base->get_dtors(dtors, processed_classes);
      }
   }
}


char const *xrefSymbol::get_unqual_name(XrefTable* Xr_t) {
    Initialize(xrefSymbol::get_unqual_name);

    char const *myname = get_name(Xr_t);
    int myname_l = strlen(myname);
    char const *colon = strchr(myname, ':');
    if (!colon)
	return myname;
    char const *lparen = strchr(colon + 2, '(');
    if (!lparen)
	lparen = myname + myname_l;
    char const *next_colon = strchr(colon + 2, ':');
    while (next_colon && next_colon < lparen) {
	colon = next_colon;
	next_colon = strchr(colon + 2, ':');
    }
    return colon + 2;
}

void xrefSymbol::set_datatype(char dt, XrefTable* Xr)
{
  Initialize(xrefSymbol::set_datatype);
  if (kind != DD_SEMTYPE) return;
  if (dt < 0) return;
  rm_link(lt_filetype, Xr);
  add_link(lt_filetype,dt+1, Xr);  
}

int xrefSymbol::get_datatype(XrefTable* Xr)
{
  Initialize(xrefSymbol::get_datatype);
  if (kind != DD_SEMTYPE) return -1;
  int i = (int) get_link_offset(lt_filetype, Xr);
  return i-1;  // since set already add 1
}

void xrefSymbol::set_attribute(uint& oldvalue, uint value, uint bitnum, uint numbits)   // static
{
//    Initialize(xrefSymbol::set_attribute);
    uint mask = ((1<<numbits)-1) << bitnum;
    oldvalue &= (~mask);
    uint newvalue = value << bitnum;
    oldvalue += newvalue;
}
uint xrefSymbol::get_attribute(uint& oldvalue, uint bitnum, uint numbits)   // static
// Return the value set by the set_attribute function.  See that description.
// This reads one or more bits out of oldvalue
{
    Initialize(xrefSymbol::get_attribute);
    uint value = oldvalue;
    value >>= bitnum;
    value &= (1<<numbits) - 1;
    return value;
}
void xrefSymbol::set_attribute(uint value, uint bitnum, uint numbits)
// Store value in attr_byte' s  This can pack many fields, total cannot be 
// more than 24 bits. Field is specified as two uints.  First is bitnum, the 
// location of the rightmost bit of the field.  Second is numbits, the width
//    of the field.  The sum of the two must not be more than 24, since
//    the whole thing must fit in the attr_byte_0, 1, and 2 of symbol.
// If value is too big for the field, it gets dropped, with a diagnostic.
//  (default value for numbits is 1)
{
    Initialize(xrefSymbol::set_attribute);
    IF (numbits==0 || bitnum+numbits>24 || value >= (1<<numbits)) {
        // this indicates a coding error in calling function
        return;
    }
    
    long old = get_symbol_attr();
    uint oldvalue = 0;
    if (old != -1)
      oldvalue = (uint) old;

    set_attribute(oldvalue, value, bitnum, numbits);
    if (is_writable() == 0) return;
    set_symbol_attr(oldvalue);
}

static xrefSymbol* cur_xrefSymbol = 0;
static uint  cur_attribute = 0;
void xref_attribute_invalidate()
{
  cur_xrefSymbol = 0;
  cur_attribute = 0;
}

unsigned int
xrefSymbol::get_attribute(unsigned int bitnum,
			  unsigned int numbits)
// Return the value set by the set_attribute function.  See that description.
{
  Initialize(xrefSymbol::get_attribute(unsigned int, unsigned int));

  IF (bitnum + numbits > 24)     // this indicates a coding error in the calling function
    return 0;

  if (cur_xrefSymbol != this) {
    cur_xrefSymbol = this;

    if (is_model_build()) {
      long tmp = get_symbol_attr();

      if (tmp == -1) // not found
	cur_attribute = 0;
      else
	cur_attribute = (uint)tmp;
    } else
      cur_attribute = (uint)get_symbol_attr();
  }

  return
    get_attribute(cur_attribute, bitnum, numbits);
}

void xrefSymbol::set_attribute(ddSymbol *dd, XrefTable *Xr)
{	
  Initialize(xrefSymbol::set_attribute);
  ddKind k = get_kind();

  // macro: only defining symbol can have attributes
  if(k==DD_MACRO && !dd->get_is_def())
    return;

  long val = (uint) get_symbol_attr();
  uint value = 0;    
  if (val != -1)
    value = (uint) val;
  uint atts =  get_attribute(value, ALL_ATTRIB, ALL_ATTRIB_LEN);
  uint previous_atts = atts;
  
  set_attribute(atts, dd->get_cd(), CONS_DES, 1);
  set_attribute(atts, dd->is_const (),CNST_ATT , 1);
  set_attribute(atts, dd->is_static (),STAT_ATT , 1);
  set_attribute(atts, dd->is_pure_virtual (),PVIR_ATT , 1);
  set_attribute(atts, dd->is_virtual (),VIRT_ATT , 1);
  set_attribute(atts, dd->is_protected (),PROT_ATT , 1);
  set_attribute(atts, dd->is_package_prot (),PAKG_ATT , 1);
  set_attribute(atts, dd->is_private (),PRIV_ATT , 1);
  set_attribute(atts, dd->is_inline (),INLI_ATT , 1);
  set_attribute(atts, dd->is_volatile (),VOLT_ATT , 1);
  set_attribute(atts, dd->is_native (),NATV_ATT , 1);
  set_attribute(atts, dd->is_synchronized (),SYNC_ATT , 1);
  
  if (dd->struct_rel_type == POINTER_TO)
    set_attribute(atts, 1, POINTER, 1);
  else if (dd->struct_rel_type == REFERENCE_TO)
    set_attribute(atts, 1, REF, 1);

  if (k == DD_FUNC_DECL)
    set_attribute(atts, dd->is_comp_gen(), COMP_GEN_ATT, 1);

  if (k == DD_FUNC_DECL || k == DD_FIELD || k == DD_VAR_DECL)
    set_attribute(atts, dd->get_from_method(), METHOD, 1);  

  if (dd->get_is_def()) {
    set_attribute(atts, dd->get_sw_tracking(),SWT_Entity_Status , 2);
    if (dd->get_sw_tracking() == SWT_NEW)
      set_last_mod_date((time_t)get_time_of_day(), Xr);
    else if (dd->get_lmd() > 0)
      set_last_mod_date((time_t) dd->get_lmd(), Xr);
  }

  if (previous_atts != atts) set_attribute(atts, ALL_ATTRIB, ALL_ATTRIB_LEN);
//
// notify of change for gui operation:
//
  if (is_gui() && previous_atts != atts)
  {
      xref_notifier_report(0, this);
  }
}

void xrefSymbol::put_signature(ostream& ostr, XrefTable* my_Xr) const
{
 xrefSymbol *xxx = (xrefSymbol*) this;
 ostr << ddKind_name(xxx->get_kind()) + 3 << '.' << xxx->get_name(my_Xr);
}

void gen_print_indent (ostream&, int);
void xrefSymbol::print (XrefTable* Xr, ostream& os, int level) const
{
  Initialize(xrefSymbol::print);
  os << '<' << "xrefSymbol";
  os << ' ' << (void *)this  << ' ';
  os << '>' << endl;
  gen_print_indent (os, level + 1);
  if (!Xr) Xr = this->get_XrefTable();
  print_sym(os, Xr);
}

//  pass it a true if you want diagnostics displayed on cout
//     bugbug - ???   we are not yet testing the bool flag passed in
bool xrefSymbol::is_consistent(bool) const
{
#ifndef _PSET_DEBUG
    return 1;                    // simple stub in optimized mode
#else
    if (pmodtest_flag < 0) return 1;	// do nothing if debugging is off
    if (pmodtest_flag == 0) {
	char const *envir=OSapi_getenv("PSET_PMODTEST");
	if ( (envir && *envir=='1')  ||    test_simple_option("-p")) {
	    pmodtest_flag = 1;  // enable debug testing
	    if (envir && !strcmp(envir,"1X")) pmodtest_flag = 2;	
	    cout << "(Note: pmod consistency testing turned on " 
		<< pmodtest_flag << ")" << endl;
	}
	else {
	    pmodtest_flag = -1;
	    return 1;	// turn off debugging, and do nothing any more
	}
    }

   Initialize(xrefSymbol::is_consistent);

//  maybe should also add an optional test for checking every reference to a link   later ?????


    xrefSymbol* sym = (xrefSymbol*)this;
    Xref* xref = sym->get_Xref();
    XrefTable * xrt = xref->get_lxref();
    headerInfo * hi = xrt->get_header();
    char *beginpmod_ind =(char *) xrt->ind_addr;
    char *endpmod_ind = beginpmod_ind + ntohl(hi->next_offset_ind);
    char *beginpmod_link =(char *) xrt->link_addr;
    char *endpmod_link = beginpmod_link + ntohl(hi->next_offset_link);
    /*
    int num1 = sym->get_no_link();
    */

    if (sym->get_kind() < 1 || sym->get_kind() >= NUM_OF_DDS) {
	sym->print1(cout, 0, xrt);
	cout << "%%%error - illegal ddKind value" << endl;
	IF(1) return 0;
    }
    int lmdcount=0;
    int lmdhighcount=0;
    int filetypecount=0;
    int num2 = 0;
    uint s_ind = sym->get_symbol_index();
    Link* lnk = sym->get_start_link(s_ind, xrt);
    while (lnk && !sym->last_link(lnk)) {
	if ((char *)lnk < beginpmod_link || (char *)lnk >= endpmod_link) {
	    sym->print1(cout, 0, xrt);
	    cout << "%%%error - link " << num2 << ", type" 
		<< (int)lnk->lt() << " out of range" << endl;
	    if ((char *)lnk >=endpmod_link) 
		cout << "  "<< endpmod_link-(char *)lnk << " bytes beyond end"
		     << endl;
	    else
		cout << "  "<< ((char *)lnk)-beginpmod_link << " bytes before begin"
		     << endl;

	    IF(1) return 0;
	}
	num2++;
	switch (lnk->lt()) {
	case  used_by:
	case  is_using:
	case  has_superclass: 
	case  has_subclass:
	case  is_defining: 
	case  is_defined_in: 
	case  is_including:
	case  included_by:
	case  member_of:
	case  parent_of:
	case  ref_file:
	case  file_ref:
	case  lt_sym:
	case  has_type:
	case  type_has:
	case  have_friends:
	case have_arg_type:
	case arg_type_of:
	case  friends_of:
	case  lt_next:
	case sym_of_assoc:
	case assoc_of_sym:
	case assocType_of_instances:
	case instances_of_assocType:
	case ifl_src_of_targ:
	case ifl_targ_of_src:
	case has_vpub_superclass:
	case has_vpri_superclass:
	case has_pub_superclass:
	case has_pri_superclass:
	case has_vpro_superclass:
	case has_pro_superclass:
	case has_pro_subclass:
	case has_vpub_subclass:
	case has_vpri_subclass:
	case has_pub_subclass:
	case has_pri_subclass:
	case has_vpro_subclass:
	case linknode_of_src_sym:
	case src_sym_of_linknode:
	case linknode_of_trg_sym:
	case trg_sym_of_linknode:
	case linkspec_of_linknode:
	case linknode_of_linkspec:
	case reverse_of_linkspec:
	case linkspec_of_reverse:
	case grp_has_peer:
	case grp_is_peer_of:
	case grp_has_server:
	case grp_has_client:
	case grp_has_pub_mbr:
	case grp_pub_mbr_of:
	case grp_has_pri_mbr:
	case grp_pri_mbr_of:
	case grp_has_trans_mbr:
	case grp_trans_mbr_of:
          {
	    if (!lnk->get_index()) {
		sym->print1(cout, 0, xrt);
		cout << "%%%error - link " << num2 << " offset is zero" << endl;
		cout << "   " << (int)lnk->lt() << linkType_name(lnk->lt()) << endl;
		IF(1) return 0;
	    }
	    char *newaddress;
	    /*
	    if (lnk->way())
      		newaddress -= lnk->get_index(); // symbol or link
	    else
      		newaddress += lnk->get_index();
		*/
	    int ind = lnk->get_index();
	    newaddress = (char *)(xrt->get_symbol_from_index(ind));
	    if (newaddress < beginpmod_ind || newaddress >= endpmod_ind) {
		sym->print1(cout, 0, xrt);
		cout << "%%%error - link " << num2 << ", type"
		     << linkType_name(lnk->lt()) << " offset too large" << endl;
		IF(1) return 0;
	    }
	    if (lnk->lt() != lt_last) {  // all the other links had better point to an xrefSymbol
		xrefSymbol* destsym = lnk->get_symbol_of_link(xrt);
		ddKind destkind = destsym->get_kind();
		if (destkind == 0 ||  destkind >= NUM_OF_DDS) {
		    sym->print1(cout, 0, xrt);
		    cout << "%%%error - link " << num2 << " of type " << linkType_name(lnk->lt()) <<
			" doesn't point to a symbol" << endl;
		    IF(1)return 0;
		}
	    }
	    break;
          }
	    
	  case  ltRelational:          // lt=0 (nobody writes this, but apparently the mapped area starts=0)
	    sym->print1(cout, 0, xrt);
	    cout << "%%%warning - link " << num2 << " is 'ltRelational' which is zero" << endl;
		IF(1) ;
	    break;
	  case  lt_junk:
	  case  lt_misc:
	    break;             // these two links are harmless ?
	  case  lt_lmd:
	    if (lmdcount++) {
		sym->print1(cout, 0, xrt);
		cout << "%%%error - too many lt_lmd links - " << lmdcount << endl;
	    	IF(1) return 0;
	    }
	    break;
	  case  lt_lmdhigh:
	    if (lmdhighcount++) {
		sym->print1(cout, 0, xrt);
		cout << "%%%error - too many lt_lmdhigh links - " << lmdhighcount << endl;
	    	IF(1) return 0;
	    }
	    break;
	  case  lt_filetype:
	    if (filetypecount++) {
		sym->print1(cout, 0, xrt);
		cout << "%%%error - too many lt_filetype links - " << filetypecount << endl;
	    	IF(1) return 0;
	    }
	    break;
	case lt_attribute:
	  break;
	  case  lt_link:     // no refs, no reasonable meaning
	  default: 
	    sym->print1(cout, 0, xrt);
	    cout << "%%%error - link " << num2 << " has illegal link type " << 
		(int)lnk->lt() << endl;
	    IF(1) return 0;
	}
	lnk = sym->get_next_link(lnk, s_ind, xrt);
    }
    bool flag1 = sym->get_has_def_file(xrt);
    bool flag2 = -1 != sym->get_link_offset(is_defined_in,0,xrt);
    if (flag1 != flag2) {
	sym->print1(cout, 0, xrt);
	cout << "%%%error - inconsistent has_def_file " << flag1 << "," << flag2 <<endl;
	IF(1) return 0;
    }
    cout << flush;
    return 1;
#endif
}

void  xrefSymbol::print_sym(ostream&os, XrefTable* Xr) const
{
  xrefSymbol* sym = (xrefSymbol *) this;
      sym->print1(os, 0, Xr);

  //  here might be a good place to indicate if the links are fragmented;  how many lt_next there are ?????
  os << "            ";
  if (sym->get_attribute(PRIV_ATT, 1)) os << "private ";
  else if (sym->get_attribute(PAKG_ATT, 1)) os << "package ";
  else if (sym->get_attribute(PROT_ATT, 1)) os << "protected ";
  else os << "public ";
  if (sym->get_attribute(VIRT_ATT, 1)) os << "virtual ";
  if (sym->get_attribute(PVIR_ATT, 1)) os << "pure virtual ";
  if (sym->get_attribute(STAT_ATT, 1)) os << "static ";
  if (sym->get_attribute(CNST_ATT, 1)) os << "const ";
  if (sym->get_attribute(INLI_ATT, 1)) os << "inline ";
  if (sym->get_attribute(VOLT_ATT, 1)) os << "volatile ";
  if (sym->get_attribute(CONS_DES, 1)) os << "ctor/dtor ";
  if (sym->get_attribute(SYNC_ATT, 1)) os << "synchronized ";
  if (sym->get_attribute(NATV_ATT, 1)) os << "native ";
  if (sym->get_attribute(METHOD, 1)) {
    if (sym->get_kind() == DD_VAR_DECL)
      os << "member ";
    if (sym->get_kind() == DD_FUNC_DECL)
      os << "method ";
  }
  if (sym->get_attribute(POINTER, 1)) os << "pointer ";
  if (sym->get_attribute(REF, 1)) os << "ref ";
  os << "language=" << sym->get_attribute(LANGUAGE, 3) << " ";
  os << " " << SwtEntityStatus_name(sym->get_attribute(SWT_Entity_Status, 2)) << " ";
  if (sym->get_attribute(COMP_GEN_ATT, 1)) os << "compiler-generated ";
  if (sym->get_has_def_file(Xr))  os << "has_def_file";
  os << endl;

  int expected=0;
  if (sym->get_kind() == DD_MODULE) {
      // get first (only) filetype link
      int fileType = (int) (sym->get_link_offset(lt_filetype,0,Xr));
      //  (we have -1 if no link was found)
      if (fileType==-1) {
	  if (!sym->get_has_def_file(Xr)) os << " (Foreign file " << sym->get_name(Xr)
			     << " has no filetype link)" << endl;
	  else os << "%%% Missing filetype for DD_MODULE " << sym->get_name(Xr) << endl;
      } else {
	  if (fileType<0 || fileType>FILE_LANGUAGE_LAST) os << "%%% Invalid ";
	  else            os << "            ";
	  os << "filetype is " << fileType-1 << endl;
      }
      expected = 1;
  }
//
// DD_SEMTYPE also uses link lt_filetype, however for that kind of symbol
// the offset field of the link designates data type.
//
  if (sym->get_kind() == DD_SEMTYPE)
  {
    int data_type = (int) (sym->get_link_offset(lt_filetype, 0, Xr));
    if (data_type == -1) 
      os << "%%% Missing data type for DD_SEMTYPE " << sym->get_name(Xr) << "." << endl;
    else
      os << "  Data type for this symbol is " << data_type << "." << endl;
    expected = 1;
  }
// count the number of actual links for filetype.  Should be either zero or 1 (=expected)
  int count;
  for (count=0; -1 != sym->get_link_offset(lt_filetype,count,Xr) && count<99; count++)
       {}
  if (count > expected)  {
    os << "%%% This symbol has too many links of type lt_filetype." << endl;
    os << "%%% Expected number of such links is " 
      << expected << "; real number is " << count << "." << endl;
  }

#ifdef _PSET_DEBUG
  this->is_consistent(1);                   // test if symbol is self-consistent
#endif

  time_t date = sym->get_last_mod_date(Xr);   // find lt_lmd and lt_lmdhigh links
  if (date) {
      char const *cdate = OSapi_ctime(&date);
      os << "  Module date is " << cdate << endl;
  }
  for(int lt=4; lt<NUM_OF_LINKS; ++lt) switch(lt){
  case ltRelational:  // 0
  case lt_next:       // 1
  case lt_junk:       // 2
  case lt_link:       // 3
  case lt_lmd:        // 16
  case lt_sym:
  case lt_filetype:
  case lt_misc:       // 19
  case lt_lmdhigh:
  case lt_attribute:
      break;
   default:
     sym->print_link((linkType)lt, os, Xr);
  }  
}

char const *xrefSymbol::get_name(XrefTable* Xr_t) const
{ 
  uint sym_name_off = get_name_offset();
  return (Xr_t->get_sym_addr() + sym_name_off);
}

char const *xrefSymbol::get_name() const 
{
  Xref* Xr = get_Xref();
  XrefTable* Xr_t = Xr->get_lxref();
  return get_name(Xr_t);
}

int xrefSymbol::equal(Link* lnk)
{
  return (lnk->get_index() == this->get_symbol_index());
}

int xrefSymbol::get_link(linkType lt, symbolArr& as, int rec)
{
    Initialize(xrefSymbol::get_link);
    int retval = 0;
    symbolPtr sym = this;
    if (rec <= 0)
	retval = sym.get_link(lt,as);
    else
	retval = sym.get_local_link(lt,as);
    return retval;
}

//boris: 121196  old way low level get_link_local () without scopes/units
int xrefSymbol::get_local_link_internal (linkType lt, symbolArr &as, XrefTable *Xr)
{
    Initialize(xrefSymbol::get_local_link_internal);
    int retval = 0;
    uint s_ind = get_symbol_index();
    register Link* lnk = get_start_link(s_ind, Xr);
    while (lnk && !last_link(lnk)) {
      if (lnk->lt() == lt) {
	as.insert_last(lnk->get_symbol_of_link(Xr));
	retval++;
	if (lt == is_defined_in)
	  break;
      }
      lnk = get_next_link(lnk, s_ind, Xr);
    }
    return retval;
}

// should not call with links is_defined_in
int xrefSymbol::get_links(linkTypes &la, symbolArr& as, XrefTable* Xr, int rec)
{
  Initialize(xrefSymbol::get_links);
// this should only be called with symbol-type linktypes
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);
  while (lnk && !last_link(lnk)) {
    if (la.contain(linkType(lnk->lt()))) {
      as.insert_last(lnk->get_symbol_of_link(Xr));
      la.found(linkType(lnk->lt()));
    }
    lnk = get_next_link(lnk, s_ind, Xr);
  }

  if (rec==0 && !la.local_only()) {
    xrefSymbol* dfs = 0;
    if (!la.contain(is_defined_in)) {
      symbolArr aas;
      if (get_link(is_defined_in, aas, 1))
	dfs = aas[0];
    }
    Xref * xr  = get_Xref();
    xr->get_links_global(this, la, as, dfs);
  }
  return as.size();
}


int xrefSymbol::get_link(linkType lt, symbolArr& as, const objSet& projs, XrefTable* my_Xr, int rec)
//  search for links of specified type, through all projects specified in
//     the objSet.  We do not descend into subprojects, as the objSet is
//     already presumed to point to the actual projects containing pmods.
{
    Initialize(xrefSymbol::get_link);
    xrefSymbol* sym = this;
    xrefSymbol* dfs = 0;                 // dfs is file this symbol was defined in
    if (lt != is_defined_in) {
      symbolArr aas;
      if (get_link(is_defined_in, aas, 1))
	dfs = aas[0];
    }
    projNodePtr pr;
    ObjPtr ob;
    ForEach(ob, projs) {
	pr = checked_cast(projNode, ob);
	if (pr) {
	    Xref* Xr = pr->get_xref(1);
	    if (Xr) {
		linkTypes la;
		la = lt;
		int coll = Xr->get_lxref()->collect_symbols(sym,la,1,as,dfs,0,my_Xr);
		if (rec==1 && coll) return as.size();
	    }
	}
    }
    return as.size();
}


// only work if ref and this are in the same xref
int xrefSymbol::has_link(linkType lt, xrefSymbol* ref, XrefTable* my_Xr)
{
  Initialize(xrefSymbol::has_link);
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, my_Xr);
  while (lnk && !last_link(lnk)) {
    if ((lnk->lt() == lt) && (ref->get_symbol_index() == lnk->get_index())) return 1;
    lnk = get_next_link(lnk, s_ind, my_Xr);
  }
  return 0;
}

int xrefSymbol::belong_to_file(xrefSymbol* file_sym, XrefTable* my_Xr)
{
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, my_Xr);
  while (lnk && !last_link(lnk)) {
    if (link_type_is.is_belong_to_file(lnk->lt()) &&
	(file_sym->get_symbol_index() == lnk->get_index())) return 1;
    lnk = get_next_link(lnk, s_ind, my_Xr);
  }
  return 0;
}

//------------------------------------------
// xrefSymbol::get_link_chase_typedefs(...)
//------------------------------------------

int xrefSymbol::get_link_chase_typedefs(linkType lt, symbolArr& results) {
   Initialize(xrefSymbol::get_link_chase_typedefs);

   get_link(lt, results);
   size_t sz = results.size();
   for (size_t i = 0; i < sz; i++) {
      symbolPtr sym = results[i];
      if (sym->get_kind() == DD_TYPEDEF)
         sym->chase_typedef(results);
   }

   return results.size();
}

//------------------------------------------
// xrefSymbol::get_link_chase_typedefs(...)
//------------------------------------------

int xrefSymbol::get_link_chase_typedefs(linkType lt, symbolArr& results,
      const objSet& projs, XrefTable* my_Xr) {
   Initialize(xrefSymbol::get_link_chase_typedefs);

   get_link(lt, results, projs, my_Xr);
   size_t sz = results.size();
   for (size_t i = 0; i < sz; i++) {
      symbolPtr sym = results[i];
      if (sym->get_kind() == DD_TYPEDEF)
         sym->chase_typedef(results, projs, sym.get_xref()->get_lxref());
   }

   return results.size();
}

// return the symbol associated with lnk

inline void xrefSymbol::msync_xref()
{
  if (PSET_MSYNC_XREF) {
    XrefTable * xrt = get_XrefTable();
    if (xrt) 
      xrt->msync_xref();
  }
}

void xrefSymbol::rm_link(linkType lt, XrefTable* Xr)
{
  rm_link(lt, 0, Xr);
}

void xrefSymbol::rm_link(linkType lt, int ow, XrefTable* Xr)
{
  linkTypes la;
  la = lt;
  rm_link(la, ow, Xr);
}

void xrefSymbol::rm_link(linkTypes &lt, int ow, XrefTable* Xr)
{
  if ((Xr->my_Xref && Xr->my_Xref->get_home_flag() == 0) || is_writable() == 0) return;
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);
  if (lt.contains(lt_lmd,lt_lmdhigh,lt_attribute,
		  lt_sym,lt_filetype,lt_misc, 0)) {
    while (lnk && !last_link(lnk)) {	
      if (lt == lnk->lt()) {
	set_need_to_mv_pmod_to_perm();
	lnk->assign(lt_junk);
	if (lt == is_defined_in) clear_has_def_file();
	return;
      }
      lnk = get_next_link(lnk, s_ind, Xr);
    }
    msync_xref();
    return;
  }
  while (lnk && !last_link(lnk)) {
    if (lt == lnk->lt()) {
      set_need_to_mv_pmod_to_perm();
      
      xrefSymbol* lnk_sym = lnk->get_symbol_of_link(Xr);
      lnk->assign(lt_junk);
      if (lt == is_defined_in) clear_has_def_file();
      if (!ow) {
	for (int ii = 0; ii < linkTypes::MAX_LINKS; ++ii)
	  if (lt.get_link(ii))
	    lnk_sym->rm_link1(get_reverse_link((linkType)ii), this, Xr);
      }
    }
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  msync_xref();  
}


// public version of following function
void xrefSymbol::rm_link(linkType lt, symbolPtr sym)
{
    rm_link(lt, sym->get_addr(), sym.get_xref()->get_lxref());
}
void xrefSymbol::rm_link(linkType lt, xrefSymbol* sym, XrefTable* Xr)
{
  Initialize(xrefSymbol::rm_link);
  if ((Xr->my_Xref && Xr->my_Xref->get_home_flag() == 0) || is_writable() == 0) return;
  rm_link1(lt, sym, Xr);
  if (sym->xrisnotnull())
    sym->rm_link1(get_reverse_link(lt), this, Xr);
}

void xrefSymbol::rm_link1(linkType lt, xrefSymbol* sym, XrefTable* Xr)
{
  Initialize(xrefSymbol::rm_link1);
  if ((Xr->my_Xref && Xr->my_Xref->get_home_flag() == 0) || is_writable() == 0) return;
  if (lt==lt_junk) return;
  Link* lnk = find_link(lt, sym, Xr);

  while (lnk) {
    set_need_to_mv_pmod_to_perm();
    lnk->assign(lt_junk);
    if (lt == is_defined_in) clear_has_def_file();
    else if ((lt == is_defining) && (get_kind() == DD_MODULE))
      set_last_mod_date((time_t) 1, Xr);
    // notify, but not during a save operation!!!!!!
    if (!SharedXref::saving()) {
      xref_notifier_report(0, this);
      xref_notifier_report(0, sym);
    }

    lnk = 0;
    VALIDATE {
      lnk = find_link(lt, sym, Xr);
      if (lnk) {
	genString msg;
	msg.printf("Duplicate link found in rm_link1 %s - %s --> %s",
		   get_name(Xr), linkType_name(lt), sym->get_name(Xr));
	cmd_validation_error((char const *)msg);
      }
    }
  }
  msync_xref();
}

void xrefSymbol::create_link(Link& lin, linkType type, xrefSymbol* sym)
{ 
  Initialize(xrefSymbol::create_link);
  lin.set_link_type(type);
  lin.set_index((uint) sym->get_symbol_index());
}

// return 0 if not exist
Link* xrefSymbol::find_link(linkType lt, xrefSymbol* sym, XrefTable* Xr)
{  
  Initialize(xrefSymbol::find_link);
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);
  while (lnk && !last_link(lnk)) {
      if ( lnk->lt() == lt) {
	  if (lt == lt_lmd || lt == lt_lmdhigh) return lnk;
	  if (sym->equal(lnk)) return lnk;
      }
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  return 0;
}

//  add numeric link
void xrefSymbol::add_link(linkType lt, long l, XrefTable* Xr)
//  note:  this does not use the sign of the long
{
  Initialize(xrefSymbol::add_link);
  if ((Xr->my_Xref && Xr->my_Xref->get_home_flag() == 0) || is_writable() == 0) return;
  set_need_to_mv_pmod_to_perm();
  Link lin;
  lin.assign(lt, (uint) l);
  Link* lnk = get_spot_to_insert_link(&lin, Xr);
  if (lnk) {
    lnk->assign(lin);
  }
  dump_link(this, 0, lnk, "add num link");
  msync_xref();
}

void xrefSymbol::add_link(linkType lt, xrefSymbol* sym, hashPair *rhps, int rev)
{
  Initialize(xrefSymbol::add_link(linkType,xrefSymbol*,hashPair*,int));
  
  xrefPair *xp = (rhps)? rhps->lookup(this, lt, sym) : 0;
  if (!xp)  {
    add_link(lt, sym);
    if (rhps)
      xp = rhps->insert(this, lt, sym);
  }
  if (xp) 
    xp->set_mark();

  if (rev) {
    xp = (rhps)? rhps->lookup(sym, get_reverse_link(lt), this) : 0;      
    if (!xp) {
      sym->add_link(get_reverse_link(lt), this);
      if (rhps)
	xp = rhps->insert(sym, (linkType) get_reverse_link(lt), this);
    }
    if (xp)
      xp->set_mark();
  }
}

void xrefSymbol::add_link(linkType lt, xrefSymbol* sym, int rev, XrefTable* sym_Xr)
{
  Initialize(xrefSymbol::add_link(linkType, xrefSymbol*, int));
  if (is_writable() == 0) return;
  Xref* xref = get_Xref();
  if (xref && xref->get_home_flag() == 0) return;
  if (rev && sym) {
      XrefTable* xr = xref->get_lxref();
      if (!sym->belonged_to(xr))
	  sym = xr->insert_symbol(sym, sym_Xr);
      add_link(lt, sym, xref);
      sym->add_link(get_reverse_link(lt), this, xref);
  }
}

//  public interface to add_link()
void xrefSymbol::add_link(linkType lt, symbolPtr sym)
// assume both symbols are in the same pmod, so we can get xref from sym
{
//  get_Xref()->make_room(1000);	// move the table if not big enough for this addition
    xrefSymbol* foo=sym;
    Xref* xref = sym.get_xref();
    add_link(lt, foo, xref);
}

// do not add if same link already exists.
void xrefSymbol::add_link(linkType lt, xrefSymbol* sym, Xref* sym_xref)
{
  Initialize(xrefSymbol::add_link(linkType, xrefSymbol*, Xref*));
  if (is_writable() == 0) return;
  IF (sym == 0) return;
  if (lt==lt_junk) return;

  Xref* xref;
  xref = get_Xref();
  if (xref && xref->get_home_flag() == 0) return; 
  if (xref && xref->get_home_flag() == 0) return; 
  XrefTable *xrt = xref->get_lxref();
  if (!sym->belonged_to(xrt)) {
    genString msg;
    msg.printf("%s->add_link(%s, %s, Xref*): called directly bypassing add_link(lt,sym,rev)",
	       get_name(xrt), linkType_name(lt), sym->get_name(sym_xref->get_lxref()));
    cmd_validation_error((char const *) msg);

    sym = xrt->insert_symbol(sym, sym_xref->get_lxref());
    IF (sym == 0) return;
  }
  set_need_to_mv_pmod_to_perm();  
  Link* lnk = get_spot_to_insert_link(lt, sym, xrt);
  if (lnk) {
    if (lt == is_defined_in) {
      if (get_kind()!=DD_MODULE || this!=sym) {
	set_has_def_file();
	if (!SharedXref::saving())
	  xref_notifier_report(1, symbolPtr(xref, this));

      } else {  
	/* we have a proposed link, of type is_defined_in,
	   between a DD_MODULE and itself
	   */
	  genString fn="ok";
	  XrefTable *xrt2;
	  
	  // if this is home project, then make sure sym is in same subproject
	  projNode* pr = projNode::get_home_proj();
	  Xref* Xr = pr?pr->get_xref(1):0;
	  if (Xr && (xrt2 = Xr->get_lxref()) &&
	      ((belonged_to(xrt2))|| (xrt2->backup && belonged_to(xrt2->backup))))
	  pr->ln_to_fn (sym->get_name(xrt), fn);

	//  Test whether this file is in the same subproject
	if ((char const *)fn || (pr == 0)) { // pr == 0 during crash recovery
		  set_has_def_file();
		  if (!SharedXref::saving())
		    xref_notifier_report(1, symbolPtr(xref, this));
	} else {
	  msg("Link is suppressed by setting sym to 0.", normal_sev) << eom;
	  msg("Source sym name: $1, Dest sym name: $2, Project name: $3.", normal_sev) << get_name(xrt) << eoarg << sym->get_name(xrt) << eoarg << pr->get_ln() << eom;
	  
	  lt = lt_junk;		// suppress is_defined_in link if file is in diff subproj
	  sym = 0;
	}
      }
    }
    else if (lt == has_type && lnk->lt() == lt) {
      /* assume that lnk is not cleaned yet!! */
      xrefSymbol * dest = lnk->get_symbol_of_link(xrt);
      if (dest)
	dest->rm_link1(type_has, this, xrt);
    }
    lnk->assign(lt, sym);
    dump_link(this, 0, lnk, "add sym link");
    // notify, but not during a save operation!!!!!!
    if (!SharedXref::saving()) {
      xref_notifier_report(0, symbolPtr(xref, this));
      if (sym) xref_notifier_report(0, symbolPtr(xref, sym));
    }
  } 
  msync_xref();
}

void xrefSymbol::add_links(hashPair& hp)
{
  Initialize(xrefSymbol::add_links(hashPair&));
  Xref* xref = get_Xref();
    for (xrefPair* xp = hp.get_first(1); xp; (xp = hp.get_next(1)) )
    {
      xrefSymbol* sym = xp->src;
      linkType lt = xp->lt;
      add_link(lt, sym, xref);
    }
}
       
       
void xrefSymbol::rm_links(hashPair &hp)
{
  Initialize(xrefSymbol::add_links(hashPair&));
  Xref* xref = get_Xref();
  for (xrefPair* xp = hp.get_first(1); xp; (xp = hp.get_next(1)) )
  {
    xrefSymbol* sym = xp->src;
    linkType lt = xp->lt;
    if (!xp->get_mark()) {
      rm_link1(lt, sym, xref->get_lxref());
      if (lt == is_defined_in)
	xref_notifier_report(-1, this);
    }
  }
}

// for now
XrefTable* xrefSymbol::get_XrefTable() const
{
  Xref* xr = get_Xref();
  return xr->get_lxref();
}

// Search one project subtree for a particular symbol
// This function calls itself recursively, till it finds a projNode
//    which has an Xref containing this symbol.   Zero if none found.
//    If it finds one, and if subprojptr is nonzero, it returns the project as well.
Xref* xrefSymbol::get_subproj_Xref_of_symbol(projNode* pn, projNode** subprojptr) const
{
//    cout << get_name() << " "  << pn->get_name() << endl;
    Initialize(get_subproj_Xref_of_symbol);
    Xref* Xr;
    Xr = pn->get_xref(1);

    if (Xr && is_symbol_in(Xr)) {
      if (subprojptr) *subprojptr = pn;
      last_Xref = Xr;
      return Xr;
    }
    if (Xr && ! Xr->is_unreal_file())
	return 0;  // stop stupid recursion
//    if (!pn->contain_subproj_pmod()) return 0;
    Obj* os = parentProject_get_childProjects(pn);
    ObjPtr ob;
    ForEach(ob, *os) {
	projNode* pr = checked_cast(projNode, ob);
	Xref* xr = get_subproj_Xref_of_symbol(pr, subprojptr);
	if (xr) return xr;
    }
    return 0;
}

Xref* xrefSymbol::get_Xref() const
{
  if (is_symbol_in(last_Xref))
    return last_Xref;

  Initialize(xrefSymbol::get_Xref);

  Xref* Xr = get_XREF();

  if (is_symbol_in(Xr)) {
    last_Xref = Xr;
    return Xr;
  }

  Xr = SharedXref::get_cur_shared_xref();
  if (is_symbol_in(Xr)) {
    last_Xref = Xr;
    return Xr;
  }

  projNode* pr;

  ForEachProj(i,pr){
//  for (i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) { //...}
    Xr = pr->get_xref(1);
    if (is_symbol_in(Xr)) {
      last_Xref = Xr;
      return Xr;
    }
  }

  //  Now search subprojects of all the main projects, as deep as possible
  ForEachProj(ii,pr){
//  for (i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) { //...}
    if (Xr=get_subproj_Xref_of_symbol(pr)) {
      last_Xref = Xr;
      return Xr;
    }
  }

  return 0; // serious err
}

projNode* xrefSymbol::get_projNode(XrefTable* my_xr)
{

  static projNode *last_proj = 0;

  Xref * Xr = 0;
  if (last_proj)
    Xr = last_proj->get_xref(1);
  if (is_symbol_in(Xr))
    return last_proj;

  Initialize(xrefSymbol::get_projNode);
  XrefTable *xref;
  char *next_addr;
  projNode* answer = 0;
  projNode* pr;

  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    Xr = pr->get_xref(1);
    if (Xr && (xref = Xr->get_lxref()) &&
	((next_addr = belonged_to(xref)) ||
	 (xref->backup && belonged_to(xref->backup)))) {
      last_Xref = Xr;
      answer = pr;
      break;
    }
  }

  //  If no luck yet, search subprojects of all the main projects, as deep as possible
  projNode* parent;
  if (!answer) {
  ForEachProj(i,parent){
//    for (i = 0; (parent=projList::search_list->get_proj(i)) ; ++i) {
      Xr = get_subproj_Xref_of_symbol(parent, &pr);
      if (Xr) {
	last_Xref = Xr;
	answer = pr;
	break;
      }
    }
  
  }

  if (get_kind() == DD_MODULE && answer == projNode::get_control_project()) {
    answer=0;
    xrefSymbol* dfs = get_def_file_priv(my_xr);
    projNode* result;
    ForEachProj(i,pr){
      //    for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i)
      if (result = pr->get_child_project(dfs->get_name())) {
	last_Xref = result->get_xref(1);
	last_proj = result;
	return result;
      }
    }
    // (if this loop does not find one, we will return 0)
  last_proj = answer;  return answer;
  }
  return answer;
}
// public function to find if a symbol is in an Xref
bool xrefSymbol::is_symbol_in(Xref* xr) const
//  If the Xref is not mapped, do *not* map it in.  (We used to)
{
    XrefTable* xrt;
    if (!xr || !(xrt=xr->get_lxref(1))) return 0;
    return  (0!=belonged_to(xrt));
}

char *xrefSymbol::belonged_to(XrefTable* xrt) const
{
//    Initialize(xrefSymbol::belonged_to__XrefTable*);
    char const *_my_name = "xrefSymbol::belonged_to__XrefTable*";

    char *retval = 0;
    if (xrt) {
	    headerInfo* hi = xrt->get_header();
	    char *saddr    = (char *)(xrt->get_ind_addr());
	    char *eaddr    = saddr + ntohl(hi->next_offset_ind);
	    if ((CP(this) >= saddr) &&(CP(this)  < eaddr))
		retval = eaddr;
    }
    return retval;
}

// get the next_offset_sym for  xref of this xrefSymbol
char *xrefSymbol::get_next_addr_offset()
{   
  Initialize(xrefSymbol::get_next_addr_offset);
  Xref* Xr = get_Xref();
  XrefTable* xref = Xr->get_lxref();
  char *next_addr;
  if (xref) {
    next_addr = belonged_to(xref);
    if (next_addr)
      return next_addr;   
  }
  projNode* pr;
  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    Xr = pr->get_xref(1);
    if (Xr && (xref = Xr->get_lxref()) &&
	(next_addr = belonged_to(xref)))
      return next_addr;
  }

  Xr = SharedXref::get_cur_shared_xref();
  if (Xr) {
    xref = Xr->get_lxref();
    if (xref) {
      next_addr = belonged_to(xref);
      if (next_addr)
        return next_addr;   
    }
  }

  return 0;			// some err
}

XrefTable * xrefSymbol::get_xrefTable()
{
  Initialize(xrefSymbol::get_xrefTable);
  Xref* Xr = get_Xref();
  XrefTable* xref = Xr->get_lxref();
  char *next_addr ;
  if (xref) {
    next_addr = belonged_to(xref);
    if (next_addr)
      return xref;
  }
  projNode* pr;
  ForEachProj(i,pr){
//  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
    Xr = pr->get_xref(1);
    if (Xr && (xref = Xr->get_lxref()) &&
	(next_addr = belonged_to(xref)))
      return xref;
  }

  Xr = SharedXref::get_cur_shared_xref();
  if (Xr) {
    xref = Xr->get_lxref();
    if (xref) {
      next_addr = belonged_to(xref);
      if (next_addr)
        return xref;   
    }
  }

  return 0; // some err
}
//  make enough room for n bytes at end of xref
void xrefSymbol::update_sym_addr_offset(int n)
{
  Initialize(xrefSymbol::update_sym_addr_offset);
  XrefTable* xr = get_xrefTable();
  IF (xr == 0 )  // some error
    return;
  xr->update_sym_addr_offset(n);
}

// return 0 if lin already exist
// else return an empty spot (reuse old deleted links)
Link* xrefSymbol::get_spot_to_insert_link(Link* lin , XrefTable* Xr)
{
  Initialize(xrefSymbol::get_spot_to_insert_link(Link*));

#ifdef _PSET_DEBUG
  this->is_consistent(1);
#endif
  if (!lin) return 0;
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);
  if (!lnk) return 0;
  int not_junk = 1;
  Link* junk_link = 0;
  bool uniqlinkflag = lin->num_link() || unique_link(lin->lt());
  while (lnk && !last_link(lnk)) {
    if (*lnk == *lin) return 0;
    // for numeric link, reuse same location (only one is legal)
    if (uniqlinkflag && lnk->lt() == lin->lt()) return lnk;
    if (lnk->lt() == lt_junk && not_junk) {
      junk_link = lnk;
      not_junk = 0;
    }
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  if (junk_link) return junk_link;
  lnk = make_new_link(s_ind, Xr);
  lnk->make_last_link();
  dump_link(this, 0, lnk, "get num spot");
  
  return lnk;
}


Link* xrefSymbol::get_spot_to_insert_link(linkType lt, xrefSymbol* sym, XrefTable* Xr)
{
  Initialize(xrefSymbol::get_spot_to_insert_link(linkType,xrefSymbol*));

#ifdef _PSET_DEBUG
  this->is_consistent(1);
  sym->is_consistent(1);
#endif
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);
  if (!lnk) return 0;
  int not_junk = 1;
  Link* junk_link = 0;
  int is_unique_link = unique_link(lt);
  int ambig_kind = (is_ambig_kind(get_kind()) || is_ambig_kind(sym->get_kind()));
  while (lnk && !last_link(lnk)) {
    if (lnk->lt() == lt) {
      if (sym->equal(lnk)) return 0;
      if (is_unique_link) {
	junk_link = lnk;
	break;
      }
    }
    if (lnk->lt() == lt_junk && not_junk) {
      junk_link = lnk;
      not_junk = 0;
    }
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  if (junk_link) return junk_link;
  lnk = make_new_link(s_ind, Xr);
  lnk->make_last_link();
  dump_link(this, 0, lnk, "get sym spot");

  return lnk;
}


symbolPtr xrefSymbol::get_def_file()
{
  Initialize(xrefSymbol::get_def_file);
    XrefTable* Xr = get_XrefTable();
    xrefSymbol* foo = get_def_file_priv(Xr);
    return symbolPtr(foo);
}

xrefSymbol* xrefSymbol::get_def_file(XrefTable* xrt)
{
  Initialize(xrefSymbol::get_def_file);
  return get_def_file_priv(xrt);
}

// return the file_symbol which *this* is defined in
// 
xrefSymbol* xrefSymbol::get_def_file_priv(XrefTable* Xr)
{  
  Initialize(xrefSymbol::get_def_file_priv);
//  Assert(kind != DD_LINK);
  if ((get_kind() == DD_MODULE) && (get_has_def_file(Xr)==0))
    return this;
    
  symbolArr as;
  if (get_link(is_defined_in, as))
    return as[0];
  return NULL;
}


int xrefSymbol::get_files(symbolArr& as, XrefTable* Xr)
{
  Initialize(xrefSymbol::get_files);

  get_link(ref_file, as);

  xrefSymbol* sym = get_def_file_priv(Xr);
  if (sym)
    as.insert_last(sym);

  return as.size();
}

int xrefSymbol::get_ref_files(symbolArr& as)
{Initialize(xrefSymbol::get_ref_files);
  int n = get_link(ref_file, as);
  return n;
}

// True if no date for symbol in pmod file, or if physical file is newer
int xrefSymbol::is_newer(app* ah, XrefTable* Xr)
{
  Initialize(xrefSymbol::is_newer);
  time_t last_mod_date =  get_last_mod_date(Xr);
  if (last_mod_date == 0) return 1;
#ifdef _WIN32
// File time on windows FAT partitions saved with 2 sek precision only
// we need to handle this
  return (last_mod_date + 2) < get_last_modified_date(ah->get_filename());
#else
  return last_mod_date < get_last_modified_date(ah->get_filename());
#endif
}

// True if no date for symbol in pmod file, or if physical file is newer
int xrefSymbol::is_newer(char const *filename, XrefTable* Xr)
{
  Initialize(xrefSymbol::is_newer);
  time_t last_mod_date =  get_last_mod_date(Xr);
  if(last_mod_date == 0) return 1;
  char fname[MAXPATHLEN];
  fileCache_realpath(filename, fname);
#ifdef _WIN32
// File time on windows FAT partitions saved with 2 sek precision only
// we need to handle this
  return (last_mod_date + 2) < get_last_modified_date(fname);
#else
  return last_mod_date < get_last_modified_date(fname);
#endif
}

long xrefSymbol::get_link_offset(linkType lt, XrefTable* Xr)
{
  Initialize(xrefSymbol::get_link_offset);
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);	
  while (lnk && !last_link(lnk)) {
    if (lnk->lt() == lt)
      return ((long) lnk->get_index());
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  return get_link_offset_global(lt);
}
// find the count'th  link matching the specified linkType
//    (i.e. if count==0, return the first one found)
// return -1 if not found
long xrefSymbol::get_link_offset(linkType lt, int count, XrefTable* Xr)
{
  Initialize(xrefSymbol::get_link_offset);
  uint s_ind = get_symbol_index();
  Link* lnk = get_start_link(s_ind, Xr);	
  while (lnk && !last_link(lnk)) {
    if (lnk->lt() == lt)
      if (0==(count--)) return ((long) lnk->get_index());
    lnk = get_next_link(lnk, s_ind, Xr);
  }
  return -1;
}

long xrefSymbol::get_link_offset_global(linkType lt)
{
  Initialize(xrefSymbol::get_link_offset_global);
  xrefSymbol* dfs = 0;
  if (lt != is_defined_in) {
    symbolArr aas;
    if (get_link(is_defined_in, aas, 1))
      dfs = aas[0];
  }  
    // first find the control project, so we can handle it specially inside loop
  projNode* cpr = projNode::get_control_project();
  XrefTable* cxr = cpr->get_xref()->get_lxref();

  Xref* oxr = get_Xref();

    XrefTable* xr;
    projNode* pr;
    objSet os;
    // build up an objSet os, which will contain projNodes for all projects and
    //        all subprojects
  ForEachProj(i,pr){
//    for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) { // readable later
	os |= pr;
	get_all_subprojects_from_proj(os, pr);
    }
    // now, for each project & subproject, check if there is an xref, and if
    //   so, and the xref is different than the original symbol, lookup all the
    //   corresponding links there
    ObjPtr ob;
    ForEach(ob, os) {
	projNode* subpr = checked_cast(projNode, ob);

	Xref* Xr = subpr->get_xref(1);
	if (Xr && (Xr != oxr) && (xr = Xr->get_lxref())) {
	    symbolArr xra;
	    xrefSymbol* xr_sym = xr->lookup(xra, get_kind(), get_name(oxr->get_lxref()),dfs);
	    if (xra.size() == 0) {
		if (belonged_to(cxr))
		    xr_sym = xr->lookup(xra, get_kind(), get_name(oxr->get_lxref()),0);
	    }
	    if (xra.size() == 0) continue;
	    else if (xr_sym == 0) xr_sym = xra[0];
	    long i = xr_sym->get_link_offset(lt, 0, Xr->get_lxref());
	    if (i >= 0) return i;
	}
    }
  return 0;
}

// Note, this function and the following both have intimate knowledge of
//    the date format, since only 24 bits can be stored in the link offset
//    If you change one, change the other the same way
//  The date value matches that returned by the Unix stat() request
//    in the st_mtime field of the structure.
//  zero will be returned if there is no date field for this symbol
static int const datemask=0xffffff;    // mask for low part of date field
static int const dateshift=24;  	  // number of bits to shift for high part
time_t xrefSymbol::get_last_mod_date(XrefTable* Xr)
{
  Initialize(xrefSymbol::get_last_mod_date);
#ifdef _PSET_DEBUG
	//Try to fetch second date link, there should not be one
  unsigned long n1 = get_link_offset(lt_lmdhigh,1, Xr);
  unsigned long n2 = get_link_offset(lt_lmd    ,1, Xr);
  // kludge to try to fix it in-place, only if PSET_PMODTEST is "1X"
  if (n1!=-1 && n2!=-1 && pmodtest_flag>1) {
	  rm_link(lt_lmd,0, Xr);
	  rm_link(lt_lmdhigh,0, Xr);
	  rm_link(lt_lmd,0, Xr);
	  rm_link(lt_lmdhigh,0, Xr);
  	n1 = get_link_offset(lt_lmdhigh,1, Xr);
  	n2 = get_link_offset(lt_lmd    ,1, Xr);
  }
  if (n1!=-1 && n2!=-1) {
      cout << "%%%error - Multiple date links " << n1 << "," << n2
	   << " in " << this->get_name(Xr) << endl;
      int iii;
      cout << "%%%error - High digits:";
	iii=0;
      while(-1 != (n1=get_link_offset(lt_lmdhigh,iii++, Xr)))
	  cout << " " << n1;
      cout << endl;
      cout << "%%%error - Low digits:";
	iii=0;
      while(-1 != (n2=get_link_offset(lt_lmd,iii++, Xr)))
	  cout << " " << n2;
      cout << endl;
      iii=0;           // display the first 10, if there are that many
      while (iii<10) {
	  n1 = get_link_offset(lt_lmdhigh,iii, Xr);
	  n2 = get_link_offset(lt_lmd,iii, Xr);
	  if (n1==-1 && n2==-1) break;
	  if (n1==-1) n1=0;
	  if (n2==-1) n2=0;
	  time_t date = (n2 & datemask) | (n1 << dateshift);
	  cout << "%%%- date= " << OSapi_ctime(&date) << endl;
	  iii++;
      }
  }
#endif
  unsigned long high = get_link_offset(lt_lmdhigh,0,Xr);
  unsigned long low = get_link_offset(lt_lmd,0,Xr);
  if (high==-1) {
      if (low==-1) return 0;
      high = (low>0x070000) ? 0x2b : 0x2c;
  }
  if (!(high|low)) low=1;
  time_t result =  (low & datemask) | (high << dateshift);
  return result;
}

//  note:  date of zero is not allowed;  it probably came from
//      a nonexistent file name. Exception is made for -pmoddiff type
//      operations, which can deal with a pset having no source file.
void xrefSymbol::set_last_mod_date(time_t date, XrefTable* Xr)
// Set the lmd/lmdhigh links for the symbol.  Date is in the 
//   usual Unix format, seconds since 1970.
{
    Initialize(xrefSymbol::set_last_mod_date);
    if (!date) {
#ifdef _PSET_DEBUG
    cout << "%%%error - attempt to set date to zero in " 
	<< this->get_name(Xr) << endl;
    IF(1) ;
#endif
	return;
    }
    if (Xr->my_Xref && Xr->my_Xref->get_home_flag() == 0) return;
    int i=10;		// (to avoid any risk of infinite loop)
    while (get_last_mod_date(Xr) && i--) {
	rm_link(lt_lmd,0, Xr);
	rm_link(lt_lmdhigh,0, Xr);
    }
    add_link(lt_lmd,     date & datemask, Xr);
    add_link(lt_lmdhigh, ((unsigned long)date) >> dateshift, Xr);
#ifdef _PSET_DEBUG
    IF (get_last_mod_date(Xr) != date);
#endif
}


int xrefSymbol::get_language(XrefTable* Xr)
// fetch the lt_filetype link for this DD_MODULE symbol
//  if there isn't one, check all the other pmods, which
//  could be very slow.  Faster alternative is
//  get_language_local()
// if this is not a module, first try to find definition file 
// and return language for it; if fail, try to find a file 
// where this symbol is referenced
{
  Initialize(xrefSymbol::get_language);
  
  int ret = smt_UNKNOWN;

  if (get_kind() == DD_MODULE)
    {
      int i = (int) get_link_offset(lt_filetype, Xr);
      ret = (i<0) ? -1 : i-1;  // since set already add 1
    }
  else
    {
      symbolPtr deffile;
      deffile = get_def_file();
      if (deffile.xrisnotnull())
	{
	  ret = deffile.get_language();
	}
      else
	{
	  symbolArr files;
	  get_ref_files(files);
	  if (files.size() > 0)
	    ret = files[0].get_language();
	}
    }
  return ret;
}


int xrefSymbol::get_language_local(XrefTable* Xr)
//  fetch the lt_filetype link for this DD_MODULE symbol
//    do not check other pmods for the link
{
  Initialize(xrefSymbol::get_language);
  if (get_kind() != DD_MODULE) return -1;
  int i = (int) get_link_offset(lt_filetype,0, Xr);
  return (i<0) ? -1 : i-1;  // since set already add 1
}

// use i + 1 to account for -1
void xrefSymbol::set_language(int i, XrefTable* Xr)
{
  Initialize(xrefSymbol::set_language);
  Assert(get_kind() == DD_MODULE);
  if (i < 0) return;
  rm_link(lt_filetype, Xr);
  add_link(lt_filetype, i+1, Xr);  
  return;
}

extern int loadedFiles_check(char const *lname);
int xrefSymbol::is_loaded(XrefTable* Xr)
{

  if (! get_has_def_file(Xr)) 
    return 0;

  symbolPtr dfs = get_def_file();
  if (dfs.isnull())
    return 0;
 
  return
    loadedFiles_check(dfs.get_name());
}

bool xrefSymbol::is_loaded_or_cheap(XrefTable* Xr) {
   Initialize(xrefSymbol::is_loaded_or_cheap);

   if (is_loaded(Xr))
      return true;

   xrefSymbol* file_sym = (get_kind() == DD_MODULE) ? this : get_def_file_priv(Xr);
   if (file_sym) {
      projModulePtr mod = projHeader::find_module(file_sym->get_name());
      if (mod && mod->is_paraset_file())
	 return true;
   }

   return false;
}
//  public version of following function
symbolPtr xrefSymbol::update_name(char const *new_name)
{
  Initialize(xrefSymbol::update_name);
    xrefSymbol* foo = update_name_priv(new_name);
    return symbolPtr(foo);
}
// need to search thru ALL xrefs to update this symbol ? probably not
// return the new xrefSymbol
xrefSymbol* xrefSymbol::update_name_priv(char const *new_name)
{
  Initialize(xrefSymbol::update_name_priv);
  Xref* xr = get_Xref();
  XrefTable *xref = xr->get_lxref();
  if ((xref->my_Xref && xref->my_Xref->get_home_flag() == 0) ) return 0;
  IF (!xref->sym_is_writable) return 0;
  int h = xref->Mhash(this->get_name(xref)) % ntohl(xref->get_header()->table_max);
  hashElmt* hel = xref->table[h];

  if (hel== 0) return NULL;
  do {
    xrefSymbol * sym = hel->el;
    if (sym == this) {
      sym->set_hash_value(xref->Mhash(new_name));
      uint new_name_l = strlen(new_name);
      uint sym_name_offset = xref->update_sym_addr_offset(new_name_l+1);
      xref->insert_xrefSymbol_name(sym_name_offset, new_name, new_name_l);
      sym->set_name_offset(sym_name_offset);
      int ind = ntohl(sym->get_hash_value()) % ntohl(xref->get_header()->table_max);
      if (hel->prev) {
	hel->prev->next = hel->next;
	if (hel->next)
	  hel->next->prev = hel->prev;
      }
      else {
	if (hel->next)
	  hel->next->prev = NULL;
	xref->table[h] = hel->next;
      }
      if (xref->table[ind])
	xref->table[ind]->prev = hel;
      hel->next = xref->table[ind];
      hel->prev = NULL;
      xref->table[ind] = hel;

      // notify, but not during a save operation!!!!!!
      if (!SharedXref::saving()) {
        xref_notifier_report(0, this);
      }
      return this;
    }
  } while (hel = hel->next);
  return NULL; // should be internal err message
}

//------------------------------------------
// xrefSymbol::chase_typedef(linkType, symbolArr&)
//------------------------------------------

void xrefSymbol::chase_typedef(symbolArr& results) {
   Initialize(xrefSymbol::chase_typedef);
   static int level = 0;
   level ++;
   char const *fname = NONAME;

   symbolArr extras;
   get_link(have_arg_type, extras);
   symbolPtr extra;
   ForEachS(extra, extras) {
      if (!results.includes(extra)) {
	  results.insert_last(extra);
	  if (extra->get_kind() == DD_TYPEDEF)
	      extra->chase_typedef(results);
      }
   }
   level --;
}

//------------------------------------------
// xrefSymbol::chase_typedef(linkType, symbolArr&, const objSet&)
//------------------------------------------

void xrefSymbol::chase_typedef(symbolArr& results,
      const objSet& projs, XrefTable* Xr) {
   Initialize(xrefSymbol::chase_typedef);

   symbolArr extras;
   get_link(have_arg_type, extras, projs, Xr);
   symbolPtr extra;
   ForEachS(extra, extras) {
      if (!results.includes(extra)) {
	 results.insert_last(extra);
	 if (extra->get_kind() == DD_TYPEDEF)
            extra->chase_typedef(results, projs, extra.get_xref()->get_lxref());
      }
   }
}


// get all object referenced in this 
int xrefSymbol::get_refs(symbolArr& sa, ddKind k)
{
  Initialize(xrefSymbol::get_refs);
  int i = sa.size();
  ddKind knd = get_kind();

  if (k == 0) {
    if (knd == DD_MODULE) 
      return get_link(file_ref, sa) - i; 
    else
      return get_link(is_using, sa) - i;
  }
  else {
    symbolArr nsa;
    if (knd == DD_MODULE) 
      get_link(file_ref, nsa); 
    else
      get_link(is_using, nsa) ;
    int cnt = 0;
    const int sz = nsa.size();
    for (int i = 0; i < sz; ++i){
      symbolPtr & sym = nsa[i];
      if (sym->get_kind() == k) {
	sa.insert_last(sym);
	cnt++;
      }	
    }
    return cnt;
  }
}

static const int oneK = 1024;
int xrefSymbol::need_to_extend_file(char const *noa)
{
  Initialize(xrefSymbol::need_to_extend_file);
  Xref* xr = get_Xref();
  XrefTable *xrt = xr->get_lxref();
  char const *sa = xrt->get_sym_addr();
  return (
    (((noa - sa) + oneK) > xrt->sym_map_size) ||
    ((ntohl(xrt->get_header()->next_offset_ind) + ONEHUNDRED) > xrt->ind_map_size));
}


void xrefSymbol::get_all_links(symbolSet & all_s, symbolSet & def_s, Xref* xr)
{
  Initialize(xrefSymbol::get_all_links);
  if (xr == 0)
    xr = get_Xref();
  IF (xr == 0) return;
  XrefTable* Xr_t = xr->get_lxref();
  uint s_ind = get_symbol_index();
  register Link* lnk = get_start_link(s_ind, Xr_t);
  xrefSymbol * fs;
  while (lnk && !last_link(lnk)) {
    if (lnk->symbol_link()) {
      fs = lnk->get_symbol_of_link(Xr_t);
      all_s.insert(symbolPtr(xr,fs));
      if (lnk->lt() == is_defining)
	def_s.insert(symbolPtr(xr,fs));
    }
    lnk = get_next_link(lnk, s_ind, Xr_t);
  }
}

void xrefSymbol::print1(ostream&os, int i, XrefTable* Xr)
{
    if (i==0) {
	os << (char const *)get_name(Xr) << ' ' << ddKind_name(get_kind()) << endl;
    } else {
	for (int j = 0; j < i; ++j) os << "   ";
	os << this << ' ' << (char const *)get_name(Xr) << ' ' 
	  << ddKind_name(get_kind()) << endl;
    }
} 

void xrefSymbol::print_link(linkType lt, ostream&os, XrefTable* Xr)
{
    Initialize(xrefSymbol::print_link );
    if (lt == lt_own_offset) {
	long n1 = get_link_offset(lt,0,Xr);
	if (n1 != -1)
	    os << ' ' << linkType_name(lt) << ' ' << n1 << endl;
    } else {
	symbolArr as;
	if (get_link(lt, as, 1)) {
	    os << "   " << linkType_name(lt) << endl;
	    for (int ind = 0; ind < as.size(); ++ind)
		((xrefSymbol*)as[ind])->print1(os, 2, as[ind].get_xref()->get_lxref());
	}
    }
}

// public version of following function
symbolPtr xrefSymbol::get_parent_of()
{
//    xrefSymbol* foo = get_parent_of_priv();
    return  symbolPtr(get_parent_of_priv());
}
xrefSymbol *xrefSymbol::get_parent_of_priv()
{
  symbolArr as;
  get_link(used_by, as);

  for (int i = 0; i < as.size(); ++i) {
    ddKind k;
    xrefSymbol *sym = (xrefSymbol *)as[i];
    if ((k = sym->get_kind()) == DD_CLASS || k == DD_INTERFACE) {
      return sym;
    }
  }
  return NULL;
}
int is_constructor(char const *name)
{
  char new_name[512];
  int j = 0;
  for (int i = 0; name[i]; ++i) {
    if (name[i] != ' ')
      new_name[j++] = name[i];
  }
  new_name[j] = 0;
  char const *colon = strchr(new_name, ':');
  if (colon == 0) return 0;
  colon += 2;
  char const *p = new_name;
  while (*colon && (*p == *colon)) {
    colon++;
    p++;
  }
  if (*colon && (*colon == '(') && (*p == ':'))
    return 1;
  return 0;
}

void xrefSymbol::get_impacted_obj(symbolArr & fa, symbolArr & oa, XrefTable* Xr)
{
  Initialize(xrefSymbol::get_impacted_obj);

  xrefSymbol* dfs = get_def_file_priv(Xr);
  symbolArr eqo;
  get_equivalence_obj(eqo,Xr);
  int i;
  for (i = 0; i < eqo.size(); ++i) {
    xrefSymbol *sym = eqo[i];
    sym->get_files(fa, Xr);
    sym->get_link(used_by, oa);
  }
  fa.usort();
  if (dfs) {
  for (i = 0; i < fa.size(); ++i) {
    xrefSymbol* sym = fa[i];
    if (strcmp(sym->get_name(Xr), dfs->get_name()) == 0)
      break;
  }
  if (i > 0) {
    symbolPtr tmp;
    tmp = fa[0];
    fa[0] = fa[i];
    fa[i] = tmp;
  }
}
  oa.usort();
}

// for now only get hard assoc, should extend later for other objs.
int xrefSymbol::get_equivalence_obj(symbolArr & sa , XrefTable* Xr)
{
  Initialize(xrefSymbol::get_equivalence_obj);
  int start_set_size, res_set_size;
  xrefSymbolSet s1;
  xrefSymbolSet s2;
  xrefSymbolSet *start_set;
  xrefSymbolSet *res_set;
  xrefSymbolSet *tmp_set;
  xrefSymbol *  sym1;
  symbolArr sa1;
  symbolArr mem_arr;
  start_set = &s1;
  res_set = &s2;
  s1.insert(this);
  s2.insert(this);
  do {
    
    tmp_set = start_set;
    start_set = res_set;
    res_set = start_set;
    
    start_set_size = start_set->size();
    Iterator it1(*start_set);
    while(++it1,sym1 = (xrefSymbol*) it1()){
      res_set->insert(sym1);
      linkTypes la;
      la.add(assoc_of_sym);
      la.add(sym_of_assoc);
      la.add(linknode_of_src_sym);
      la.add(trg_sym_of_linknode);
      sym1->get_links(la, sa1, Xr, 0);
      for (int i = 0; i < sa1.size(); ++i) {
	xrefSymbol * sym = sa1[i];
	res_set->insert(sym);
      }
      sa1.removeAll();
      if (sym1->get_kind() == DD_CLASS) {
	mem_arr.removeAll();
	if (sym1->get_link(is_using, mem_arr)) {
	  const int sz = mem_arr.size();
	  for (int i = 0; i < sz; ++i) {
	    xrefSymbol * mem = mem_arr[i];
	    if (is_constructor(mem->get_name(mem_arr[i].get_xref()->get_lxref())))
	      res_set->insert(mem);
	  }
        }
      }	
    }
    res_set_size = res_set->size();
  
  } while (start_set_size < res_set_size);
  Iterator it2(*res_set);
  while(++it2,sym1 = (xrefSymbol*) it2())
    sa.insert_last(sym1);

  return sa.size();
}


//=============================================
// xrefSymbol::get_overrides_of: For a this->kind == DD_CLASS, find
// member functions that have the same name (or are both destructors
// in their respective classes) as the unqualified name "func".
//=============================================

void xrefSymbol::get_overrides_of(char const *func, symbolArr& overrides,
				  symbolArr& processed) {
   Initialize(xrefSymbol::get_overrides_of);

   if (kind == DD_CLASS || kind == DD_INTERFACE) {
      if (!processed.includes(this)) {
	 symbolArr members;
	 get_link(is_using, members);
	 symbolPtr member;
	 ForEachS(member, members) {
	    if (member.is_override_for(func)) {
	       overrides.insert_last(member);
	       break;
	    }
	 }
	 processed.insert_last(this);
	 symbolArr subclasses;
	 get_link(has_subclass, subclasses);
	 symbolPtr subclass;
	 ForEachS(subclass, subclasses) {
	    subclass->get_overrides_of(func, overrides, processed);
	 }
      }
   }
}

bool xrefSymbol::xrisnull()
{
    return  (this == NULL);
}
bool xrefSymbol::xrisnotnull()
{
    return  (this != NULL);
}
symbolPtr xrefSymbol::get_alternate_def_file(linkType lt, ddKind k, symbolPtr & alt)
    /*
      * should add attributes for instance of templates?
      
      */
      
{
    Initialize(get_alternate_def_file);
    
    char const *name = get_name();
    if (strchr(name, '<') == 0) return NULL_symbolPtr;
    symbolArr res;
    get_link(lt, res);
    symbolPtr s;
    ForEachS(s, res) {
	if (s->get_kind() == k) {
	    alt = s->get_def_file();
	    return s;
	}
    }
    return NULL_symbolPtr;
}

//------------------------------------------
// xrefSymbol::get_links(linkType_selector&, objSet&, bool);
//------------------------------------------

void xrefSymbol::get_links(linkType_selector& sel, const objSet& projs,
			   bool chase_typedefs, XrefTable* my_Xr) {
   Initialize(xrefSymbol::get_links);

   xrefSymbol* dfs = 0;      // definition file
   symbolArr arr;
   if (get_link(is_defined_in, arr, 1)) {
      dfs = arr[0];
      if (dfs->get_has_def_file(arr[0].get_xref()->get_lxref()) && sel.local_only()) {
	 get_links_1pmod(sel, projs, chase_typedefs, my_Xr);
	 return;
      }
   }

   projNodePtr pr;
   ObjPtr ob;
   ForEach(ob, projs) {
      pr = checked_cast(projNode, ob);
      if (pr) {
	 Xref* Xr = pr->get_xref(1);
	 XrefTable* xr;
	 if (Xr && (xr = Xr->get_lxref())) {
	    arr.removeAll();
	    symbolPtr sym = xr->lookup(arr, get_kind(), get_name(my_Xr), dfs);
	    if (arr.size()) {      /* used/defined in this proj */
	       if (sym.xrisnull()) {
		  sym = arr[0];
	       }
	       xrefSymbol* xr_sym = (xrefSymbol*) sym;
	       bool return_now = false;
	       if (sel.local_only()) {
		  if (xr_sym->get_has_def_file(xr)) {
		     arr.removeAll();
		     if (xr_sym->get_link(is_defined_in, arr, 1)) {
			dfs = arr[0];
			return_now = dfs->get_has_def_file(xr);
		     }
		  }
	       }
	       if (!sel.local_only() || return_now) {
		  xr_sym->get_links_1pmod(sel, projs, chase_typedefs, xr);
		  if (return_now) {
		     return;
		  }
	       }
	    }
	 }
      }
   }
}


//------------------------------------------
// xrefSymbol::get_links_1pmod(linkType_selector&, objSet&, bool);
//------------------------------------------

void xrefSymbol::get_links_1pmod(linkType_selector& sel, const objSet& projs,
				 bool chase_typedefs, XrefTable* Xr) {
   Initialize(xrefSymbol::get_links_1pmod);

   uint s_ind = get_symbol_index();
   Link* lnk = get_start_link(s_ind, Xr);
   linkType lt;
   while ((lt = lnk->lt()) != lt_last) {
     if (sel.wants(lt)) {
       xrefSymbol* trg;
       if (trg = lnk->get_symbol_of_link(Xr)) {
	 sel[lt].insert_last(trg);
	 if (trg->get_kind() == DD_TYPEDEF && chase_typedefs) {
	   trg->chase_typedef(sel[lt], projs, Xr);
	 }
       }
     }
     lnk = get_next_link(lnk, s_ind, Xr);
   }
}


//------------------------------------------
// xrefSymbol::get_parent_of(const objSet&)
//------------------------------------------

symbolPtr xrefSymbol::get_parent_of(const objSet& projs, XrefTable* my_xr) {
   Initialize(xrefSymbol::get_parent_of [objSet]);

   ddKind parent_kind;
   ddKind my_kind = get_kind();
   if (my_kind == DD_FUNC_DECL || my_kind == DD_VAR_DECL || my_kind == DD_FIELD) {
      parent_kind = DD_CLASS;
   }
   else if (my_kind == DD_ENUM_VAL) {
      parent_kind = DD_ENUM;
   }
   else return NULL_symbolPtr;

   xrefSymbol* dfs = 0;      // definition file
   symbolArr arr;
   if (get_link(is_defined_in, arr, 1)) {
      dfs = arr[0];
   }

   projNodePtr pr;
   ObjPtr ob;
   ForEach(ob, projs) {
      pr = checked_cast(projNode, ob);
      if (pr) {
	 Xref* Xr = pr->get_xref(1);
	 XrefTable* xr;
	 if (Xr && (xr = Xr->get_lxref())) {
	    arr.removeAll();
	    symbolPtr sym = xr->lookup(arr, get_kind(), get_name(my_xr), dfs);
	    if (arr.size()) {      /* used/defined in this proj */
	       if (sym.xrisnull()) {
		  sym = arr[0];
	       }
	       xrefSymbol* xr_sym = (xrefSymbol*) sym;
	       uint s_ind = xr_sym->get_symbol_index();
	       Link* lnk = xr_sym->get_start_link(s_ind, xr);
	       linkType lt;
	       while ((lt = lnk->lt()) != lt_last) {
		 if (lt == used_by) {
		   ddKind k;
		   xrefSymbol* trg = lnk->get_symbol_of_link(xr);
		   if (   (   (k = trg->get_kind()) == parent_kind
			   || parent_kind == DD_CLASS && k == DD_INTERFACE)
		       && trg->get_has_def_file(xr)) {
		     return symbolPtr(trg);
		   }
		 }
		 lnk = get_next_link(lnk, s_ind, xr);
	       }
	     }
	 }
      }
   }
   return NULL_symbolPtr;
}

void xrefSymbol::init(uint sym_index, char knd, uint sym_name_off, int hsh)
{
    flags_byte = 0;                              //set has_def to Zero
    attr_byte_0 = attr_byte_1 = attr_byte_2 = 0; //set attribute to zero
    set_symbol_index(sym_index);     //set index of symbol
    kind = knd;
    set_offset_to_first_link(0);
    set_name_offset(sym_name_off);
    set_hash_value(hsh);
}

app * xrefSymbol_load_file(xrefSymbol*, XrefTable*);
// need to be DD_MODULE
app* xrefSymbol::load_file(XrefTable* this_Xr)
{
  return xrefSymbol_load_file(this, this_Xr);
}

// load the file if needed
ddElement* xrefSymbol::get_def_dd()
{  
  return get_def_dd(symbolPtr(this));
}

ddElement* app_get_dd(app*ah, ddKind k, char const *n);

ddElement* xrefSymbol_get_def_dd(symbolPtr that)
{  
  Initialize(xrefSymbol_get_def_dd);
  ddKind knd = that.get_kind();
  genString nm = that.get_name();
  symbolPtr dfs;
  if (knd == DD_MODULE) 
    dfs=that;
  else if (that.xrisnotnull())
    dfs= that->get_def_file();
  if (dfs->xrisnull()) return 0; // do not know about this 

  app* ah = dfs.load_file();
  if (ah == 0)
    return 0;
 
  ddElement* dd = app_get_dd(ah, knd, nm);

  //els::101797, might need to change this
  if (dd && dd->get_language() == smt_UNKNOWN)
     dd->set_language(smtLanguage(that.get_language()));

  return dd;
}

ddElement* xrefSymbol::get_def_dd(symbolPtr that)
{  
 return
   xrefSymbol_get_def_dd(that);
}

ddElement* xrefSymbol::get_dds(app* ah, XrefTable* Xr)
{
   return
    app_get_dd(ah, get_kind(), get_name(Xr));
}


bool xrefSymbol::get_has_def_file(XrefTable* Xr, int remote)
{
    int retval = (flags_byte & 0x80) != 0;

    return retval;
}

void xrefSymbol::set_offset_to_first_link(int ui)
{
  offset_to_start_link = htonl(ui);
}

int xrefSymbol::get_offset_to_first_link()
{
    return ntohl(offset_to_start_link);
}

void xrefSymbol::set_name_offset(uint nm_off)
{
  name_offset = htonl(nm_off);
}

uint xrefSymbol::get_name_offset(void) const
{
  return (ntohl(name_offset));
}

char const *xrefSymbol::get_name_real(XrefTable* xr) const 
{ 
  return ((char const *)(xr->get_sym_addr()) + ntohl(name_offset));
}

void xrefSymbol::set_hash_value(uint h_val)
{
  hash_val = htonl(h_val);
}

uint xrefSymbol::get_hash_value()
{
  return ntohl(hash_val);
}

xrefSymbol* Link::get_symbol_of_link(XrefTable* Xr_t)
{
  xrefSymbol * temp = NULL;
  int in = get_index();
  temp = (xrefSymbol *)(Xr_t->get_ind_addr() + in - 1);
  return temp;
}

Link* xrefSymbol::get_start_link(uint si, XrefTable *xrt_local)
{
  Link* lnk = 0;
  if (!xrt_local->symbolIndex_table)
    lnk = (Link*)((char *)xrt_local->get_link_addr() + get_offset_to_first_link());
  else {
    SymbolIndex sin = *((*xrt_local->symbolIndex_table)[si]);
    if (sin)
      lnk = (*sin)[0];
    else
      lnk = (Link*)((char *)xrt_local->get_link_addr() + xrt_local->get_orig_link_offset(si));
  }
  return lnk;
}

Link * xrefSymbol::make_new_link(uint sind, XrefTable* Xr)
{
  Link * lnk = 0;
  if (!Xr->symbolIndex_table && Xr->link_is_writable)
    Xr->init_symbolIndex_table();
  if (Xr->symbolIndex_table) {
    SymbolIndex* sym_ind_ptr = (*Xr->symbolIndex_table)[sind];
    if (*sym_ind_ptr == 0) 
      *sym_ind_ptr = new LinkArr;
    SymbolIndex my_sym_ind = *sym_ind_ptr;
    int sz = my_sym_ind->size();
    lnk = my_sym_ind->grow(1);
    lnk->make_last_link();
    if (sz && last_link((*my_sym_ind)[sz-1]))
      lnk--;
  }
  return lnk;
}


Link* xrefSymbol::get_next_link(Link* lnk, uint si, XrefTable* xref_t)
{
  Link* next_link = 0;
  if (!xref_t->symbolIndex_table)
    next_link = ++lnk;
  else {
    SymbolIndex sind = *((*xref_t->symbolIndex_table)[si]);
    if (sind) {
      int nl= sind->size();
      if (lnk != (*sind)[nl -1])
	next_link = ++lnk;
      else
	if (!last_link(lnk))
	  next_link = (Link*)((char *)xref_t->get_link_addr() + xref_t->get_orig_link_offset(si));
    }
    else
      next_link = ++lnk;
  }
  return next_link;
}
