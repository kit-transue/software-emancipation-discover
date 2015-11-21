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
/*  sortlinks.C - implement spec_link and sorted_link classes
//      Created  5/15/94
//
// The purpose of this file is to provide functions to facilitate comparing
//    two pmods, or psets, or somesuch.  It is part of the model validation
//   project.

  classes:
  // both of these classes are defined and used only within this file
  // Their whole purpose is to define an iterator for links in a pmod,
  //   that walks through the pmod in an ordered way.
     spec_link - class that holds information about a particular link from a particular pmod
     sorted_link - iterator class that knows how to walk through a pmod in a sorted order


*/
#include <cLibraryFunctions.h>

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <xref.h>
#include <xref_priv.h>
#include <ddKind.h>
#include <math.h>
#include <projList.h>
#include <systemMessages.h>
#include <globals.h>
#include <db_intern.h>
#include <genTmpfile.h>

#ifdef _WIN32
#include <winsock.h>
#include <machdep.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <sys/stat.h>
#include <driver_mode.h>

// entry point from main.C --  declaration belongs in a header file
extern "C"  bool do_pmod_command (int num_of_pmod_files, genString* pmod_file, const char* pmod_command);

// Tell whether we are processing -pmoddiff  or -psetdiff or other special commands
//extern "C" bool is_pmodcmd();

static bool show_Details=0;        // static flag to tell whether to print full report
static bool is_pmod_command=0;  // static flag to tell whether we are doing one of these special commands,
             // to suppress certain error checking


//  This function really ought to be a genString member function
static void gen_truncate(genString& str, uint newlen)
// Truncate a string to the specified length
// Do nothing if newlen is larger than the string size,
//    and naturally do nothing if the size is the same.
{
    int len = str.length();
    if (len>newlen) {
	genString tmp=str;
	((char*)tmp)[newlen] = '\0';
	str = tmp;
    }
}

static bool fixup_pset_name(genString& name)
// Strip off the trailing .pset if present, so it apparently points to the C file
// Then check if name.pset exists, and return TRUE if so.
{
    Initialize(fixup_pset_name);
    int len = name.length();
    if (len>4 && !strncmp(name.str()+len-4,"pset",4)) {
	gen_truncate(name, len-4);
	len = name.length();
    }
    if (len>1 && !strncmp(name.str()+len-1,".",5))
	gen_truncate(name, len-1);
#ifdef _WIN32
    struct OStype_stat sbuf;
#else
    struct stat sbuf;
#endif
    genString tmp=name;
    tmp += ".pset";
    return (!(OSapi_stat(tmp,&sbuf)));
}

static bool is_pmod_file(genString& name)
// Convert name to standard form, and see if a pmod file exists.
// If it does, return true, and replace input name with revised name.
// If not, return false.
{
    Initialize(is_pmod_file);

    genString name1 = name;
    unsigned int len = name1.length();
    if (len>4) {
	// if string ends with .sym or .ind, lop it off after the period
	if (!strcmp(".sym", name1.str()+len-4) || !strcmp(".ind", name1.str()+len-4) || !strcmp(".lin", name1.str() + len -4)) {
	    gen_truncate(name1,len-3);
	    len = name1.length();
	}
    }
    
    if (len && name1[len-1]=='.')
	gen_truncate(name1,len-1);
    genString name1sym = name1.str();
    name1sym += ".sym";
    genString name1ind = name1.str();
    name1ind += ".ind";
    genString name1link = name1.str();
    name1link += ".lin";
    struct stat sbuf;
    if (!stat(name1sym,&sbuf) && !stat(name1ind,&sbuf) && !stat(name1link, &sbuf)) {
	name = name1;
	msg("(Validated pmod file name $1\n") << name.str() << eom;         // BUGBUG remove this line
	return 1;
    }
    return 0;
}

enum spec_type {
    symbol=0,      // the xrefSymbol itself 
    is_def=1,      // the is_defined flag in the xrefSymbol (present if the bit is one)
    numb_link=2,   // a numeric link
    symb_link=3,   // a symbolic link
    funny_link=4,  // any (non-junk) link that is neither numeric nor symbolic 
    trailer=99,    // special record to mark end of data
    dummy_type=399};

class spec_link
{
  public:
    spec_link(spec_type  type, symbolPtr sym, Link* link);
    spec_link(){};
    ~spec_link() {};
    spec_type get_type() const;
    symbolPtr get_sym() const;
    Link* get_link() const;
    symbolPtr get_dest_sym_slow() const;     // get destination symbol, if appropriate
    symbolPtr get_dest_sym() const;     // get destination symbol, if appropriate

    int  compare(const spec_link &link2) const;      // compare two special-links
    bool  is_greater2(const spec_link &link2) const;  // temporary holding place for following fct
    bool  is_greater(const spec_link &link2) const;  // compare two links, the way we want to sort them
    bool operator==(const spec_link & link2) const;    // compare for equivalent, though they may be in two different pmods
    bool operator>(const spec_link & link2) const;    // compare for *this greater than link2
    bool operator<(const spec_link & link2) const;    // compare for less
    bool operator!=(const spec_link & link2) const;
    bool operator<=(const spec_link & link2) const;
    bool operator>=(const spec_link & link2) const;
    bool is_inserted() const;
    bool is_inserted(bool);
    bool is_deleted() const;
    bool is_deleted(bool);
    
    void print() const;
    void show_inserted() const;
    void show_symbol_conditional() const;
    void show_link_conditional() const;
    static void print_setup();         // sandwich functions for print_pretty
    void print_pretty() const;
    static void print_cleanup();       // sandwich functions for print_pretty
  private:
    spec_type  type;
    symbolPtr sym;
    Link* link;
    symbolPtr destsym;        // redundant, but saves time for space
    
    uint  inserted:1;
    uint  deleted:1;
    uint  dummy:30;
};
/////////////////////////
#ifndef _PSET_DEBUG    //
#define _PSET_DEBUG    //
#endif                 //
/////////////////////////

#ifdef _PSET_DEBUG
#define  DEBUGTEST(link2)                                \
    Initialize(stuff);                                   \
    IF (compare(link2)+link2.compare(*this))                 \
	msg("Non-symmetrical compare", warning_sev) << eom;
#else
#define DEBUGTEST(link2)
#endif

inline bool spec_link::operator>(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) > 0;
}
inline bool spec_link::operator>=(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) >= 0;
}
inline bool spec_link::operator<(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) < 0;
}
inline bool spec_link::operator<=(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) <= 0;
}
inline bool spec_link::operator==(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) == 0;
}
inline bool spec_link::operator!=(const spec_link& link2) const
{
    DEBUGTEST(link2);
    return compare(link2) != 0;
}
inline bool spec_link::is_inserted() const
// fetch the flag indicating that this link was in 2nd file, not in 1st
{
    return inserted;
}
inline bool spec_link::is_inserted(bool flag)
// set the flag indicating that this link was in 2nd file, not in 1st
{
    deleted = !flag;
    return (inserted=flag);
}
inline bool spec_link::is_deleted() const
// fetch the flag indicating that this link was in 1st file, not in 2nd
{
    return deleted;
}
inline bool spec_link::is_deleted(bool flag)
// set the flag indicating that this link was in 1st file, not in 2nd
{
    inserted = !flag;
    return (deleted=flag);
}

spec_link::spec_link(spec_type ty, symbolPtr sy, Link* li)
: type(ty), sym(sy), link(li), inserted(0), deleted(0), dummy(0)
{
    if (ty==symb_link)
	destsym = get_dest_sym_slow();
    else
	destsym = NULL_symbolPtr;
}

inline spec_type spec_link::get_type() const
//  tell whether this link represents a symbol, an is_def, numeric_link, ...
{
    return type;
}

symbolPtr spec_link::get_sym() const
//  fetch the source symbol of this link
{
    return sym;
}

symbolPtr spec_link::get_dest_sym_slow() const
//  fetch the destination symbol of this link, explicit version,
//    used only in constructor
{
    XrefTable * xrt = (get_sym().get_xref())->get_lxref();
    xrefSymbol* destsym = link->get_symbol_of_link(xrt);
    return symbolPtr(destsym);                                    //BUGBUG  could optimize using sym->get_xref()
}
inline symbolPtr spec_link::get_dest_sym() const
//  fetch the destination symbol of this link
{
    return destsym;
}

inline Link* spec_link::get_link() const
//  fetch the link
{
    return link;
}


#ifdef _PSET_DEBUG
extern "C" void pl(spec_link* slnk)
//  debugging print of spec_link
{
    slnk->print();
}
#endif

void spec_link::print() const
// display to stdout complete information about one  spec_link
{
    Initialize(spec_link::print);
    XrefTable* xrt = get_sym().get_xref()->get_lxref();
    xrefSymbol* destsym;
    msg("SL:") << eom;
    switch (type) {
      case symbol:
	msg("symbol - ") << eom;
	sym->print1(cout, 0,xrt);
	break;
      case is_def:
	sym->print1(cout, 0,xrt);
	IF (!sym->get_has_def_file(xrt))
	    ;
	msg("   is_def = $1\n") << sym->get_has_def_file(xrt) << eom;
	break;
      case numb_link:
	sym->print1(cout, 0,xrt);
	msg("    numeric_link -  $1$2\n") << (int)link->lt() << eoarg << linkType_name(link->lt()) << eom;
	break;
      case symb_link:
	sym->print1(cout, 0,xrt);
	msg("    symbol_link -  $1$2\n") << (int)link->lt() << eoarg << linkType_name(link->lt()) << eom;
	destsym = link->get_symbol_of_link(xrt);
	destsym->print1(cout, 2,xrt);
	break;
      case funny_link:
	sym->print1(cout, 0,xrt);
	msg("    funny_link -  $1$2\n") << (int)link->lt() << eoarg << linkType_name(link->lt()) << eom;
	break;
      case trailer:
	msg("trailer record\n") << eom;
	break;
      default:
	msg("illegal_type - $1\n") << (int)type << eom;
    }
}

// static storage to avoid seeing duplicate information on adjacent similar links
static symbolPtr base_sym;
static linkType base_lt;
void spec_link::print_setup()                 // static
// Initialize the static fields, preparatory to printing out link information
{
    Initialize(spec_link::print_setup);
    base_sym = NULL_symbolPtr;
    base_lt = (linkType)-1;        // no links processed yet
}
void spec_link::print_cleanup()               // static
//  cleanup printout
{
    Initialize(spec_link::print_cleanup);
    //  maybe nothing to do here, but keep it just in case
}
void spec_link::show_inserted() const
// Output a + or - prefix, to indicate whether this record is inserted or deleted
// Output a blank if neither
{
	if (is_inserted())
	    msg("+") << eom;
	else if (is_deleted())
	    msg("-") << eom;
	else 
	    msg(" ") << eom;
}
void spec_link::show_symbol_conditional() const
// show symbol if it is different from the last one we showed
{
    if (sym.sym_compare(base_sym)) {
	msg("") << eom;                   // blank line between symbols
	msg("(symbol)- ") << eom;
	sym->print1(cout, 0, sym.get_xref()->get_lxref());
	base_sym = sym;
	base_lt = (linkType)-1;        // no links processed yet
    }
}
void spec_link::show_link_conditional() const
// show link type if it is different from the last one we showed
{
    if (link->lt() != base_lt) {
	base_lt = link->lt();
	msg("    symbol_link -  $1$2\n") << (int)base_lt << eoarg << linkType_name(base_lt) << eom;
    }
}

void spec_link::print_pretty() const
// Display to stdout information about one  spec_link
//   in a prettier format than spec_link::print() does
// This keeps state information in static variables, and
//   must be sandwiched between  print_setup() and print_cleanup()
{
    Initialize(spec_link::print_pretty);
    if (!show_Details)
	return;
    XrefTable* Xr = get_sym().get_xref()->get_lxref();
    switch (type) {
      case symbol:
	if (sym.sym_compare(base_sym)) {
	    msg("") << eom;                   // blank line between symbols
	    show_inserted();
	    msg("symbol - ") << eom;
	    sym->print1(cout, 0, Xr);
	    base_sym = sym;
	    base_lt = (linkType)-1;        // no links processed yet
	}
	break;
      case is_def:
	show_symbol_conditional();
	IF (!sym->get_has_def_file(Xr))
	    ;
	show_inserted();
	msg("    is_def - $1\n") << sym->get_has_def_file(Xr) << eom;
	break;
      case numb_link:
	show_symbol_conditional();
	show_inserted();
	msg("    numeric_link -  $1$2\n") << (int)link->lt() << eoarg << linkType_name(link->lt()) << eom;
	break;
      case symb_link:
	show_symbol_conditional();
 	show_link_conditional();
 	show_inserted();
	get_dest_sym()->print1(cout, 2, Xr);
 	break;
      case funny_link:
	show_symbol_conditional();
	show_inserted();
	msg("    funny_link -  $1$2\n") << (int)link->lt() << eoarg << linkType_name(link->lt()) << eom;
	break;
      case trailer:
	msg("trailer record\n") << eom;
	break;
      default:
	msg("illegal_type - $1\n") << (int)type << eom;
    }
}


class  sorted_link
{
  public:
    static bool setup_xref(Xref*& xref, const char* name, int perm, projNode** proj=0);
    static void cleanup_xref(Xref* xref);
    static uint do_difference(const char* name1, const char* name2);
    static uint do_differenceX(Xref* xr1, Xref* xr2);
    static void do_print(const char* name);
    static void do_print(Xref* xref, bool do_header);
    static uint do_pset_difference(const char* name1, const char* name2);
    static uint do_pmodpset(const char* pmodname, const char* psetname, bool order);
    static void do_pset_print(const char* name);

    sorted_link(const XrefTable *xrtable, const Xref *xr);
    ~sorted_link();
    spec_link get_current() const {return links[current_link];};

    bool is_done() const;
    void operator++();                          // increment to the next link
    void get_links(symbolPtr sym);         // build sorted array of links for one symbol
    void sorted_insert(spec_link link);          // insert one link into the sorted array "links"
    bool  test_links_are_sorted();
  
  private:
    const XrefTable *xrt;        // pointer to our XrefTable
    const Xref* xref;              // pointer to our Xref
    bool done;

    symbolArr  src_syms;   // sorted list of source symbols
    int current_sym;         // subscript in src_syms for next symbol to be processed
    xrefSymbol* sym;       // the particular symbol we are working on

    spec_link* links;           // array of links for this particular symbol
    int num_links;         //  number of links in the list
    int next_link_to_add;         // used only in building links array
    int current_link;         // subscript in links for this link being processed
    Link* link;            // the particular link we are working on
    static spec_link trailer_record;    // record to mark end of list
    static spec_link dummy_record;      // record which will never appear in list
};
inline bool sorted_link::is_done() const
{ return done; }

spec_link sorted_link::trailer_record = spec_link(trailer, NULL_symbolPtr, 0);
spec_link sorted_link::dummy_record = spec_link(dummy_type, NULL_symbolPtr, 0);

sorted_link::sorted_link(const XrefTable *xrtable, const Xref* xr)
    // constructor for the sorted_link iterator
: xrt(xrtable), xref(xr), links(0),
done(0), current_sym(-1), num_links(0), next_link_to_add(0), current_link(-1)
{
    // src_syms is empty right now, since we are in constructor
    uint num_syms = xrt->get_symbols(src_syms, xr);             // get all symbols from XrefTable
    src_syms.usort();                       // sort them

    
    if (num_syms) {
	done = 0;
	current_sym = -1;
	current_link = 2;
	num_links = current_link - 1;     // force it to read the first symbol
	(*this).operator++();
    } else {
	done = 1;
	(*this).operator++();
    }
}

sorted_link::~sorted_link()
//  destructor
{
    delete[] links;
}

void sorted_link::operator++()
// increment to next link
{
    if (current_link+1 < num_links) {
	current_link++;
    } else if (current_sym+1 < src_syms.size()) {
	current_sym++;
	symbolPtr sym = src_syms[current_sym];
	get_links(sym);
	(*this).operator++();
    } else {
	num_links = 1;
	links = new spec_link[num_links];
	current_link = 0;
	links[current_link] = trailer_record;
	done = 1;
    }
}

void  sorted_link::get_links(symbolPtr sym)
{
    // note:  the allocated array may be bigger than needed, since it
    //   reserves space for all links including lt_junk.
    // But lt_junk links are not actually written to it.
    delete[] links;
    num_links=0;

    uint cnt = 0;
    Link* temp;
    XrefTable* xref_t =  sym.get_xref()->get_lxref();
    uint sind = sym->get_symbol_index();
    temp = sym->get_start_link(sind, xref_t);

    while (!sym->last_link(temp)) {
	cnt++;
	temp = temp+1;
    }

    cnt += 2;      // one or two special links go in, in any case
    num_links = cnt;
    next_link_to_add = 0;
    links = new spec_link[cnt];
    
    // first put the symbol itself into the list:
    spec_link t = spec_link(symbol, sym, 0);
    sorted_insert(t);
    // now if is_defined=1, put the is_def link flag into the list
    if (sym->get_has_def_file(xref_t)) {
	t = spec_link(is_def, sym, 0);
	sorted_insert(t);
    }
    
    temp = sym->get_start_link(sind, xref_t);
	
    while (!sym->last_link(temp)) {
	spec_link t;
	if (temp->num_link())
	    t = spec_link(numb_link, sym, temp);
	else if (temp->symbol_link())
	    t = spec_link(symb_link, sym, temp);
	else {
	    if (temp->lt()==lt_junk)            // ignore lt_junk links
		t = dummy_record;
	    else
		t = spec_link(funny_link, sym, temp);
	}
	if (t.get_type() != dummy_record.get_type())
	    sorted_insert(t);
	temp = temp+1;
    }
#ifdef _PSET_DEBUG
    test_links_are_sorted();      // test if the links are still sorted
#endif
    
    num_links = next_link_to_add;                // shorten the array, because we ignored lt_junks
    current_link = -1;                        // begin accessing at beginning of array
}

bool sorted_link::test_links_are_sorted()
// debug function to check if the links are still sorted
    // returns nonzero if any errors, but also prints them 
{
    unsigned int err = 0;
#ifdef _PSET_DEBUG
    Initialize(sorted_link::test_links_are_sorted);
    Assert(next_link_to_add <= num_links);
    if (next_link_to_add>1) {
	for (int i=0; i<next_link_to_add-1; i++) {
	    if (links[i].is_greater(links[i+1])) {
		msg("SL: -------------\n") << eom;
		msg("SL: Links out of order --:\n") << eom;
		links[i].print();
		links[i+1].print();
		msg("SL: -------------\n") << eom;
		err++;
	    }
	}
    }
#endif
    return err > 0;
}



void sorted_link::sorted_insert(spec_link newlink)           // insert one link into the sorted array "links"
//  insert a link into the proper place of the sorted array "links"
//     technique is the "insertion sort"
{
    // at this point, next_link_to_add is used to point to indicate the current size of the sorted
    //   array, while  num_links indicates the allocated size
    Initialize(sorted_link::sorted_insert);
    Assert(next_link_to_add < num_links);
    
    int  insert_here = 0;
    for (int i=next_link_to_add; i>0 ; i--) {
	if (!links[i-1].is_greater(newlink)) {
	    insert_here = i;                    // we have found the place to insert
	    break;
	}
	links[i] = links[i-1];
    }

    links[insert_here] = newlink;
    next_link_to_add++;
}

bool  spec_link::is_greater(const spec_link &slink2) const
// temporary wrapper function, to check consistency/sym,metry of the real code
{
    Initialize(sorted_link::is_greater_wrapper);

    if (*this == slink2)
	return 0;
    bool temp1 = is_greater2(slink2);
    bool temp2 = !slink2.is_greater2(*this);
    IF (temp1 != temp2) {
	msg("Sorted_link::is_greater function not quite right\n") << eom;
    }
    return temp1;
}

bool  spec_link::is_greater2(const spec_link &slink2) const
// compare two links, the way we want to sort them
//  true means process link2 before link1
//  false means they are already in sorted order
{
    Initialize(sorted_link::is_greater);
    int ret = compare(slink2);
    return (ret>0);
}

inline int normalize(int arg)
//  return -1, 0 or +1, depending on sign of argument
{
    return (arg>0) -(arg<0);			  
}

int spec_link::compare(const spec_link &slink2) const
// compare two special-links, the way we want to sort them
// zero means they are equivalent
// 1 means process slink2 before *this
// -1 means slink2 is greater than *this, so process *this first
{
    int ret;
    if (get_type() == trailer) {
	if (slink2.get_type() == trailer)
	    return 0;
	return 1;         // special case for trailer - which always goes to end
    }
    if (slink2.get_type() == trailer)
	return -1;
    
    if (get_sym() != slink2.get_sym()) {
	symbolPtr sym1 = get_sym();
	symbolPtr sym2 = slink2.get_sym();
	ret = sym1.sym_compare(sym2);        // this will sort by source kind,name, and def_file
	if (abs(ret)>1)
	    return normalize(ret);
    }
    if (get_type() != slink2.get_type())
	return (get_type() > slink2.get_type()) ? 1 : -1;   // do lower-numbered types first

    if (get_type() != symb_link && get_type() != numb_link)   // if not symbolic or numeric link
	return 0;                                             //  ignore link field

    Link* link1 = get_link();
    Link* link2 = slink2.get_link();

    if (link1 == link2)
	return 0;

    if (link1->num_link()) {
	if (link2->symbol_link())
	    return -1;                 // process symbolic links after numeric ones
	if (link1->lt() != link2->lt())
	    return  (link1->lt() > link2->lt()) ? 1 : -1;      // do lower numbered links first

	if (link1->get_index() != link2->get_index())
	    return link1->get_index() > link2->get_index() ? 1 : -1;  // do lower offsets first
	return 0;
    }

    if (link1->symbol_link()) {
	if (link2->num_link())
	    return 1;                  // process numeric links first
	if (link1->lt() != link2->lt())
	    return  (link1->lt() > link2->lt()) ? 1 : -1;      // do lower numbered links first
	// at this point, the two symbolic links have the same kind
	ret = get_dest_sym().sym_compare(slink2.get_dest_sym());  // this will sort by destination kind, name, and def-file name
	if (abs(ret)>1)            // (fold symbol without def_file with a symbol that has one)
	    return normalize(ret);          // Done if any of these differ
    }
    // The links are neither numeric nor symbol, or else they agree in just about every aspect
    if (link1->lt() != link1->lt())
	return  (link1->lt() > link2->lt()) ? 1 : -1;      // do lower numbered links first
    return 0;
}


// This function can be dispensed with if we do a make-clean,
//   and set a default argument on the following function
uint XrefTable::get_symbols(symbolArr& as)
//  get all the symbols from a given pmod file
{
    Initialize(XrefTable::get_symbols1);
    return get_symbols(as, 0);
}

uint XrefTable::get_symbols(symbolArr& as, const Xref* xr_arg) const
//  get all the symbols from a given pmod file
{
    Initialize(XrefTable::get_symbols);
    XrefTable* myxrt = (XrefTable*)this;     // kludge for non-const functions
    const Xref* xr = xr_arg;
    if (!xr)
	xr = myxrt->get_Xref();
    Xref* myxr = (Xref*)xr;     // kludge for non-const functions
    
    headerInfo* hi = myxrt->get_header();
    uint noi = ntohl(header->next_offset_ind);
    int cnt = 0;
    for (int i = 0; i*SIZEOF_XREFSYM < noi; ++i) {

	xrefSymbol* sym = (xrefSymbol*) (ind_addr + i);
	as.insert_last(symbolPtr(myxr,sym));
	cnt ++;
    }
    return cnt;
}
void do_difference(int num, genString* pmods)
// Do difference on each pair of pmods in the array
// num is supposed to be an even number
{
    int i;
    uint diffs=0;
    for (i=0; i+1 < num; i+=2) {
	diffs += sorted_link::do_difference(pmods[i].str(), pmods[i+1].str());
    }
    if ((num & 1)) {
	msg("Error: odd number of arguments to -pmoddiff\n") << eom;
	msg("Ignoring last argument :$1\n") << pmods[num-1].str() << eom;
    }
    if (diffs) {
	globals::error_status(MINOR_ERROR);
	msg("Error: Total differences $1\n") << diffs << eom;
    } else {
	msg("Done with difference\n") << eom;
    }
}

void do_print(int num_of_pmod_files, genString* pmod_file)
// print out information from each pmod, in sequence
{
    int i;
    for (i=0; i< num_of_pmod_files; i++) {
//	genString temp=pmod_file[i].str();
//	cout << "-pmodprint file: " << temp.str() << endl;
//	sorted_link::do_print((char*)temp.str());
	sorted_link::do_print((char*)pmod_file[i].str());
    }
}

bool do_pmod_command(int num_of_pmod_files, genString* pmod_file, const char* pmod_command)
//  Called from xref_main ---  for any switch that begins  -pmod  or -pset
//     (currently we are using only -pmod switches)

//  This is called at two times:
//  During parsing of the command line, it is called with -1 as first argument, and its
//    only tasks are to set the write_flag if the home pmod is to be modified, and
//    to validate the pmod_command.  0 means it is legal, 1 means illegal

//  When it is actually called to do some work, num_of_pmod_files is positive, and the
//    return value is zero.
{
    Initialize(do_pmod_command1);
#ifndef _PSET_DEBUG
    return 1;
#else
    if (num_of_pmod_files != -1)
	is_pmod_command = 1;

    if (!strcmp(pmod_command, "-pmoddiff")) {
	if (num_of_pmod_files==-1)
	    return 0;
	show_Details = 1;
	do_difference(num_of_pmod_files, pmod_file);
    } else if (!strcmp(pmod_command, "-pmodcmp")) {
	if (num_of_pmod_files==-1)
	    return 0;
	show_Details = 0;
	do_difference(num_of_pmod_files, pmod_file);
    } else if (!strcmp(pmod_command, "-pmodprint")) {
	if (num_of_pmod_files==-1)
	    return 0;
	show_Details = 1;
	do_print(num_of_pmod_files, pmod_file);
    } else {
	return 1;       // illegal option
    }
    return 0;   // normal completion of valid option
#endif
}

bool sorted_link::setup_xref(Xref*& xref, const char* name, int perm, projNode** proj)
// Given a physical file pathname for a pmod, set up proj, Xref, and XrefTable
//   for it.  Return the Xref* to the users pointer.
// Optionally return the projNode to the users pointer (if he passes a nonzero pointer).
// Result is 1 for success, 0 for failure.
// perm is 1 for a read-only pmod, 0 for a writable one
{
    Initialize(sorted_link::setup_xref);
    bool ret = 1;
    const char* name1= name;
    Xref* xr1=new Xref((char*)name1, perm, (projNode*)0);
    projNode* proj1 = new projNode((char*)name1, xr1);
    if (!proj1)
	ret = 0;
    projList::search_list->add_proj(proj1);
    if (xr1) 
	xr1->my_proj = proj1;
    else
	ret = 0;
    genString name1sym = name1;
    unsigned int len = name1sym.length();
    if (len>4) {
	// if string ends with .sym or .ind or .lin, lop it off after the period
	if (!strcmp(".sym", name1sym.str()+len-4))
	    ((char*)name1sym)[len-3] = '\0';
	if (!strcmp(".ind", name1sym.str()+len-4))
	    ((char*)name1sym)[len-3] = '\0';
	if (!strcmp(".lin", name1sym.str()+len-4))
	    ((char*)name1sym)[len-3] = '\0';
	len = name1sym.length();
    }
    
    if (len && name1sym[len-1]!='.')
	name1sym += ".";
    genString name1ind = name1sym.str();
    genString name1link = name1sym.str();
    name1sym += "sym";
    name1ind += "ind";
    name1link += "lin";

    XrefTable *xrt1 = new XrefTable(xr1, (char*)name1sym, (char*)name1ind, name1link, 1, 0);
    if (xrt1->header) {
	xr1->lxref = xrt1;
#ifdef _PSET_DEBUG
//	cerr << "(XrefTable is from " << (void*)xrt1->header << " to " << (void*)(xrt1->sym_header + xrt1->header->next_offset_sym) << ")" << endl;
#endif
    } else {
	ret = 0;
    }
    if (ret) {
	xref = xr1;      // return point to the Xref
	if (proj)
	    *proj = proj1;   // optionally point to the project
    }
    return ret;
}   
void sorted_link::cleanup_xref(Xref* xref)
// Delete the dummy Xref, XrefTable, and projNode created by setup_xref()
{
    Initialize(sorted_link::cleanup_xref);
    
    delete xref->lxref;  xref->lxref = 0;
    projNode* proj = xref->get_projNode();
    delete xref;
//    xrefTable::delete_unref_xrefEntry_global();
    projList::search_list->rem_proj(proj);
    delete proj;
}

void sorted_link::do_print(const char* name)     // static
// print a pmod or a pset, in sorted order
// This logic is called in the case of -pmodprint, which
//   differs from -print in that the pmod involved is specified
//   explicitly, as a physical filename.  It does not even need
//   to be listed in the pdf.
{
    Initialize(sorted_link::do_print);
    genString name1 = name;
    bool flag1 = is_pmod_file(name1);
    
    if (flag1) {
	// it must be a pmod
	Xref* xref = 0;
	bool good = setup_xref(xref, name1, 1, 0);
	if (good)
	    do_print(xref, 1);
	else {
	    msg("Couldn\'t map the pmod... skipping pmod print function\n") << eom;
	}
	cleanup_xref(xref);
    } else {
	// it must be a pset
	do_pset_print(name);
    }
}
void sorted_link::do_print(Xref* Xr, bool do_header)        // static
// print contents of a pmod
{
  Initialize(sorted_link::do_print);
#ifdef  _PSET_DEBUG
  XrefTable* xref= Xr->get_lxref();
  if (do_header) {
      char* name = xref->symfile_name;
      if (xref->units_are_backup) name = xref->symbackup;
      msg("Information dump of $1\n") << name << eom;
      xref->header->print();
  }
  
  spec_link::print_setup();
  for (sorted_link  foo(xref, Xr); !foo.is_done(); foo.operator++()) {
      foo.get_current().print_pretty();
  }
  spec_link::print_cleanup();
  msg("All done") << eom;
  
#endif  
}    

uint sorted_link::do_difference(const char* name1, const char* name2)     // static
// do difference of two pmods, two psets, or one of each
// The pmod names are passed as physical path strings up to the word pmod, and
//   may optionally include the period.  They do not include either sym or ind.
//   Note that the name string may actually be anything at all, but ending
//   with pmod is customary. If it ends with . or with .sym or with .ind
//   these are effectively stripped to get the base name.
// The pset names are passed as physical path strings up to the word pset
//     optional  .pset  at end
{
    Initialize(sorted_link::do_difference);
    uint result=9999;

    // check if one or both are psets, versus pmods
    genString tmp1 = name1;
    bool flag1 = is_pmod_file(tmp1);
    genString tmp2 = name2;
    bool flag2 = is_pmod_file(tmp2);
    
    if (flag1 && flag2) {
	// both are pmods
	Xref* xr1=0;
	Xref* xr2=0;
	
	bool good1 = setup_xref(xr1, name1, 1, 0);
	bool good2 = setup_xref(xr2, name2, 1, 0);
	
	if (good1 && good2) {
	    result = do_differenceX(xr1, xr2);
	} else {
	    result=9999;
	    msg("Couldn\'t map the pmods.... skipping difference function\n") << eom;
	}
	cleanup_xref(xr1);
	cleanup_xref(xr2);
    } else if (!flag1 && !flag2) {
	// both are psets
	result = do_pset_difference(name1, name2);
    } else {
	// one is a pmod, and the other is a pset
	result = do_pmodpset(name1, name2, flag2);
    }
    if (result)
	globals::error_status(MINOR_ERROR);
    return result;
}

uint sorted_link::do_pset_difference(const char* name1, const char* name2)
// do difference of two psets
// The names are passed as physical path strings up to but not including
//  the string .pset
{
    Initialize(sorted_link::do_pset_difference);
    uint diffs=9999;
    
    //  In save/src/db_restore.C is a function db_restore(char*fname)
    //  and in db_io.C is db_read_buf(char* fname, char* lname,...)
    
    app * head;      // information for a single pset
    genString name1a = name1;
    fixup_pset_name(name1a);

    msg("Restoring from $1\n") << name1a.str() << eom;
    head = db_read_bufs(name1a, 0);        // restore pset to app* (defined in db_io.C)
    if (head) {
	genString xref_file;
	
	genTmpfile tmpfile1("Tmp1-");
	xref_file = tmpfile1.name();
	xref_file += ".pmod";
	Xref* xr1;
	projNode* proj1;
	bool flag1 = setup_xref(xr1, xref_file, 0, &proj1);      // define a scratch pmod
	appHeader_put_projectNode(head, proj1);
	if (flag1) {
	    xr1->make_room(500000);       // create some breathing room in this pmod
	    
	    msg("   Inserting into $1...") << xref_file.str() << eom;
	    xr1->insert_module_in_lxref(head);
	    obj_unload(head);
	    head=0;
//	    xrefTable::delete_unref_xrefEntry_global();
	    msg("") << eom;
	    
	    genString name2a = name2;
	    bool flag = fixup_pset_name(name2a);
#ifdef _PSET_DEBUG
//	    //  take out when it works
//	    cerr << "pset file exists :" << flag << endl;
#endif
	    msg("Restoring from $1\n") << name2a.str() << eom;
	    head = db_read_bufs(name2a, 0);        // restore pset to app* (defined in db_io.C)
	    
	    if (head) {

		genTmpfile tmpfile2("Tmp2-");
		xref_file = tmpfile2.name();
		xref_file += ".pmod";
		Xref* xr2;
		projNode* proj2;
		bool flag2 = setup_xref(xr2, xref_file, 0, &proj2);     // define a scratch pmod
		appHeader_put_projectNode(head, proj2);
	    
		if (flag2) {
		    xr2->make_room(500000);       // create some breathing room in this pmod
		
		    msg("   Inserting into $1...") << xref_file.str() << eom;
		    xr2->insert_module_in_lxref(head);
		    obj_unload(head);
		    head=0;
		    msg("") << eom;
		    
		    diffs = do_differenceX(xr1, xr2);
		    
		    cleanup_xref(xr2);
		} else {
		    msg("Error: Couldn\'t create temporary pmod file $1\n") << xref_file.str() << eom;
		}
		cleanup_xref(xr1);
	    } else {
		msg("Error: Failed to restore $1\n") << name2a.str() << eom;
	    }
	} else {
	    msg("Error: Couldn\'t create temporary pmod file $1\n") << xref_file.str() << eom;
	}
    } else {
	msg("Error: Failed to restore $1\n") << name1a.str() << eom;
    }
    msg("Done comparing\n") << eom;
    return diffs;
}


uint sorted_link::do_differenceX(Xref* xr1, Xref* xr2)
// do difference of two pmods
//
{
    Initialize(sorted_link::do_differenceX);
    XrefTable* xrt1= xr1->get_lxref();
    XrefTable* xrt2=xr2->get_lxref();
    if (!xrt1 || !xrt2)
	return 0;
    
    msg(" ********************************************************\n") << eom;
    int diffs = 0;
    char* name1 = xrt1->symfile_name;
    if (xrt1->units_are_backup) name1 = xrt1->symbackup;
    sorted_link links1(xrt1, xr1);
    
    char* name2 = xrt2->symfile_name;
    if (xrt2->units_are_backup) name2 = xrt2->symbackup;
    sorted_link links2(xrt2, xr2);
    spec_link::print_setup();
    
    msg("Taking difference\n") << eom;
    msg("   of  $1\n") << name1 << eom;
    msg("   and $1\n") << name2 << eom;
//    xrt1->header->print();
//    xrt2->header->print();
    msg("") << eom;
    
    while (!(links1.is_done() && links2.is_done())) {
	spec_link link1 = links1.get_current();
	spec_link link2 = links2.get_current();
	if (link1 < link2) {
	    diffs++;
	    link1.is_deleted(1);
	    link1.print_pretty();
	    links1.operator++();
	}
	else if (link2 < link1) {
	    diffs++;
	    link2.is_inserted(1);
	    link2.print_pretty();
	    links2.operator++();
	} else {
	    links1.operator++();
	    links2.operator++();
	}
    };
    spec_link::print_cleanup();
    if (diffs)
	msg("$1 differences found\n") << diffs << eom;
    else
	msg("No differences found\n") << eom;
    msg(" ********************************************************\n") << eom;

    return diffs;
}

uint sorted_link::do_pmodpset(const char* oldname, const char* newname, bool order)  // static
// Do difference of a pmod and a pset
//    Consider only the information of the pmod that came from the same module
//    as the pset represents
// If order is 0, the first is a pmod, second is a pset
// If order is 1, the reverse
//
{
    Initialize(sorted_link::do_pmodpset);
    const char* pmodname;
    const char* psetname;
    if (order) {
	pmodname = newname;
	psetname = oldname;
    } else {
	pmodname = oldname;
	psetname = newname;
    }

    uint diffs = 9999;
    app* head;
    genString name1a = psetname;
    bool flag = fixup_pset_name(name1a);
#ifdef _PSET_DEBUG
//	    //  take out when it works
//	    cerr << "pset file exists :" << flag << endl;
#endif
    msg("Restoring from $1\n") << name1a.str() << eom;
    head = db_read_bufs(name1a, 0);       // restore pset to app*
    if (head) {
	genTmpfile tmpfile1("Tmp1-");
	genString xref_file = tmpfile1.name();
	xref_file +=".pmod";
	Xref* xr1;
	projNode* proj1;
	bool flag1 = setup_xref(xr1, xref_file, 0, &proj1);
	XrefTable* xrt1 = xr1->get_lxref();
	appHeader_put_projectNode(head,proj1);
	if (flag1) {
	    xr1->make_room(500000);
	    msg("   Inserting into $1...") << xref_file.str() << eom;
	    xr1->insert_module_in_lxref(head);
	    obj_unload(head);
	    head = 0;
//	    xrefTable::delete_unref_xrefEntry_global();
	    genString log_mod_name;	    //  extract logical name of pset from pmod
	    symbolArr arr;
	    symbolPtr sym;
	    xrt1->get_symbols(arr);
	    ForEachS(sym, arr) {
		if (sym->get_kind()==DD_MODULE && sym.get_has_def_file()) {
		    msg("Logical name seems to be $1\n") << sym.get_name() << eom;
		    log_mod_name.put_value(sym.get_name());
		}
	    }
	    msg("") << eom;
	    
	    Xref* xr3=0;
	    bool flag3 = setup_xref(xr3, pmodname, 1, 0);  // set up the pmod for read-only access
	    XrefTable* xrt3 = xr3->get_lxref();
	    if (flag3) {
		genTmpfile tmpfile1("Tmp2-");
		genString xref_file = tmpfile1.name();
		xref_file +=".pmod";
		Xref* xr2;
		projNode* proj1;
		bool flag2 = setup_xref(xr2, xref_file, 0, &proj1);
		XrefTable* xrt2 = xr2->get_lxref();
		appHeader_put_projectNode(head,proj1);
		if (flag2) {
		    xr2->make_room(500000);
		    msg("Copying module $1 from $2\n") << log_mod_name.str() << eoarg << pmodname << eom;
		    msg("   into $1...") << xref_file.str() << eom;
		    xrt3->copy_module1(log_mod_name, xrt2);
		    msg("") << eom;

		    if (order)
			diffs = do_differenceX(xr1, xr2);
		    else
			diffs = do_differenceX(xr2, xr1);
		    
		    cleanup_xref(xr2);
		} else {
		    msg("Couldn\'t build pmod $1\n") << xref_file.str() << eom;
		}		
		cleanup_xref(xr3);
	    } else {
		msg("Couldn\'t load pmod $1\n") << pmodname << eom;
	    }
	    cleanup_xref(xr1);
	} else {
	    msg("Couldn\'t build pmod $1\n") << xref_file.str() << eom;
	}
    } else {
	msg("Couldn\'t restore $1\n") << name1a.str() << eom;
    }
    return diffs;
}


void sorted_link::do_pset_print(const char* name)  // static
//  print out contents of one pset file
{
    Initialize(sorted_link::do_pset_print1);
    
    //  In save/src/db_restore.C is a function db_restore(char*fname)
    //  and in db_io.C is db_read_buf(char* fname, char* lname,...)
    
    genString name1 = name;
    fixup_pset_name(name1);
    msg("Restoring from $1\n") << name1.str() << eom;
    app* head = db_read_bufs(name1, 0);        // restore pset to app tree (defined in db_io.C)
    if (head) {
	genTmpfile tmpfile("DUMMY-");
	genString xref_file = tmpfile.name();
	xref_file += ".pmod";
	Xref* xr;
	projNode* proj;
	bool flag = setup_xref(xr, xref_file, 0, &proj);
	appHeader_put_projectNode(head, proj);
	if (flag) xr->make_room(500000);       // create some breathing room in this pmod
	
	xr->insert_module_in_lxref(head);
	obj_unload(head);
	head=0;
	msg("Done restoring\n") << eom;
	do_print(xr, 0);
	cleanup_xref(xr);
    } else {
	msg("Error: Failed to restore $1\n") << name1.str() << eom;
    }
    return ;
}

/*  Documentation for new switches, to be used in debug version
of the code:

The switch -pmodprint takes each pmod file in the list, and shows its
contents, in sorted order.

The switch -pmoddiff interprets its list two files at a time, comparing
those two files, and showing the differences in detail.  Each file pair
may be two pmods, two psets, or one of each.  If a pset is compared with
a pmod, the logic is as follows:
   Check the logical name for the pset
   Copy that module information from the pmod to a scratch pmod
   Compare the pset to the scratch pmod.

The switch -pmodcmp works the same way, but it suppresses the detail
output.  It will still display the counts for each file-pair.

In both -pmoddiff and -pmodcmp, a nonzero exit-level is set whenever
any differences are found.  However, to determine which pair had the
differences, one must examine the stdout and stderr output.

The following script will compare two psets, branch on the exit level,
and display a different message for match and for nomatch:

-----------------------------------------------------------------------
Ideas for future enhancements:
1) use qsort() instead of insertion sort to sort the links.  Measure first.
2) add a new switch to take a list of pset files and a target directory, rather
than the list having to alternate new and old names.

*/

