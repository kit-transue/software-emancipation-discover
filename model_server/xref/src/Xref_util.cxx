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
#ifdef _PSET_DEBUG
//   comment out the  following #define before doing the PUT
//#define  _PSET_DEBUG_PRINT
#endif

#include <cLibraryFunctions.h>

#include <genTmpfile.h>
#include <xref.h>
/* #include <xref_priv.h> * try not include *NY, 09.20.95 */
#include <xrefPair.h>
#include <xrefSymbolSet.h>
#include <XrefTable.h>
#include <SharedXref.h>
#include <link_type_converter.h>
#include <indHeaderInfo.h>
#include <linkTypes.h>

#include <RTL_externs.h>
#include <machdep.h>
#include <symbolSet.h>
#include <psetmem.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
using namespace std;
#endif /* ISO_CPP_HEADERS */

hashPair *mhp = 0;
hashPair *rhp = 0;
hashPair *get_remove_hashPair()
{
    return rhp;
}

#define SECOND_LEVEL_SIZE 59

// move all links to one place.
void XrefTable::tidy_xref(XrefTable *new_xref)
{
    Initialize(XrefTable::tidy_xref);
    merge_xref(new_xref);
}

void XrefTable::merge_xref(XrefTable *xref)
{
    Initialize(XrefTable::merge_xref);
    headerInfo* hi    = get_header();
    uint noi          = ntohl(header->next_offset_ind);
    int i;
    for (i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = get_ind_addr()+i;
            xrefSymbol* new_sym = xref->insert_symbol(sym,this);
            if (new_sym) {
                long new_sym_index = new_sym->get_symbol_index();
                sym->add_link(lt_sym, new_sym_index, this);
            }
        }
    for (i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = get_ind_addr() + i;
            long j = sym->get_link_offset(lt_sym, this);
            IF (j == 0) continue;	
            xrefSymbol* new_sym = xref->get_symbol_from_index((int) j);
            sym->copy_link(new_sym, xref);
        }
    for (i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = get_ind_addr() + i;
            sym->rm_link(lt_sym, this);
        }
}

// copy all links from this to sym
void xrefSymbol::copy_link(xrefSymbol* new_sym, XrefTable* xref)
{
    Initialize(xrefSymbol::copy_link);
    Xref* xr = xref->get_Xref();
    uint sind = get_symbol_index();
    Link* lnk = get_start_link(sind, xref);
    while (lnk && !last_link(lnk)) {
        if (lnk->symbol_link()) {
            xrefSymbol* lnk_sym = lnk->get_symbol_of_link(xref);
            long j = lnk_sym->get_link_offset(lt_sym, xref);
            if (j > 0) {
                xrefSymbol* nls = xref->get_symbol_from_index((int)j);
                new_sym->add_link(lnk->lt(), nls, xr);
            }
        }
        else if (lnk->num_link())
            new_sym->add_link(lnk->lt(), (long) lnk->get_index(), xref);
        lnk = get_next_link(lnk, sind, xref);
    }
    return;
}

// connect is_defined_in from xt to this
void XrefTable::link_xref(XrefTable *xt)
{
    Initialize(XrefTable::link_xref);
  
    headerInfo* hi    = get_header();
    Xref* xr          = get_Xref();
    uint noi          = ntohl(header->next_offset_ind);
    symbolArr as;
    for (int i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = get_ind_addr() + i;
            if (sym->get_has_def_file(this) == 0) {
                as.removeAll();
                xrefSymbol* sym1 = xt->lookup(as, sym->get_kind(), 
                                              sym->get_name(this), 0);
                xrefSymbol* dfs = 0;
                if (sym1 && (dfs = sym1->get_def_file())) {
                    xrefSymbol* file_sym = lookup_module(dfs->get_name(xt));
                    if (file_sym == 0) {
                        file_sym = newSymbol(DD_MODULE, dfs->get_offset_to_first_link(),
                                             dfs->get_name(xt));
                        if (file_sym) file_sym->add_link(is_defined_in, file_sym, xr);
                    }
                    if (file_sym) sym1->add_link(is_defined_in, file_sym, xr);
                }
            }
        }
}

// copy lt of sym to this, sym is of different xref presumably
void XrefTable::copy_link(xrefSymbol* sym, linkType lt, XrefTable* sym_xrt, xrefSymbol* lt_of)
{
    Initialize(XrefTable::copy_link);
    xrefSymbol* this_sym = insert_symbol(sym, sym_xrt);
    if (lt_of && this_sym) {
        xrefSymbol* this_ts = insert_symbol(lt_of, sym_xrt);
        this_sym->add_link(lt, this_ts,1,this);
        return;
    }
    symbolArr as;
    sym->get_link(lt, as, 1);
    for (int i = 0; i < as.size(); ++i) {
        xrefSymbol* this_ts = insert_symbol((xrefSymbol*) as[i], as[i].get_xref()->get_lxref());
        if (this_sym && this_ts) this_sym->add_link(lt, this_ts,1,this);
    }
}

// move info of module_name: this -> to
void XrefTable::copy_module(char const *module_name, XrefTable *to)
{
    Initialize(XrefTable::copy_module);
  
    to->make_room(250000);        // make sure there is at least 1/4mb of space in pmod
    xrefSymbol* file_sym = lookup_module(module_name);
    if (file_sym == 0) return;
    to->copy_link(file_sym, is_including, this);
    to->copy_link(file_sym, included_by, this);

    headerInfo* hi    = get_header();
    uint noi          = ntohl(header->next_offset_ind);
    for (int i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = get_ind_addr()+i;
            if (sym->get_has_def_file(this) && sym->get_def_file() == file_sym) {
                to->copy_link(sym, is_defined_in,this);
                to->copy_link(sym,is_using,this );
                to->copy_link(sym,has_superclass,this);
                to->copy_link(sym, has_vpub_superclass,this);
                to->copy_link(sym, has_vpri_superclass,this);
                to->copy_link(sym, has_pub_superclass,this);
                to->copy_link(sym, has_pri_superclass,this);
                to->copy_link(sym, has_vpro_superclass,this);
                to->copy_link(sym, has_pro_superclass,this);
            }
            else if (sym->has_link(ref_file, file_sym, this))
                to->copy_link(sym, ref_file, this, file_sym);
        }
    return;
}
// this == srce, to == destination

void XrefTable::copy_module1(char const *module_name, XrefTable *to)
{
    Initialize(XrefTable::copy_module1);
#ifdef _PSET_DEBUG
    is_consistent_hashElmts();
#endif

#ifdef _PSET_DEBUG_PRINT
    //    Debug information only .............     ?????????????  remove before doing put
    char const *name = symfile_name;
    if (units_are_backup) name = symbackup;
    cout << "---------- Copying module " << module_name << " from " << name << endl;
    name = to->symfile_name;
    if (to->units_are_backup) name = to->symbackup;
    cout << "                    to " << name << endl;
#endif

    to->make_room(250000);        // make sure there is at least 1/4mb of space in pmod
    hashPair hp;         // these are the symbols that are mentioned in this file (belong_to_file)
    hashPair def_hp;        // these are the symbols that are defined in this file (is_defined_in)
    xrefSymbol* file_sym = lookup_module(module_name);
    if (!file_sym)
        return;
    headerInfo* hi    = get_header();
    uint noi          = ntohl(header->next_offset_ind);
    xrefSymbol* nsym  = to->insert_symbol(file_sym, this);
    if (nsym) {
        hp.insert(file_sym, nsym);  // (src, trg)
        def_hp.insert(file_sym,nsym);                                 // ???????
    }
    for (int i = 0; i*SIZEOF_XREFSYM < noi; ++i)
        {
            xrefSymbol* sym = (xrefSymbol*) (get_ind_addr() + i);
            if (sym->belong_to_file(file_sym, this)) {

                nsym = to->insert_symbol(sym, this);
                if (nsym) {
                    hp.insert(sym, nsym);  // (src, trg)
                    if (sym->get_has_def_file(this)) {
                        symbolArr as;
                        sym->get_link(is_defined_in, as ,1);
                        if (as[0]== file_sym)
                            def_hp.insert(sym, nsym);  // (src, trg)
                    }
                }
            }

        }
    for (xrefPair* xp = hp.get_first(); xp; (xp = hp.get_next()) )
        {
            xrefSymbol* src = xp->src;
            xrefSymbol* trg = xp->trg;
            // iterate thru all links in src, if link belong to file_sym, copy it 
            // to trg, use has_link to test
            src->copy_module_link(&hp, &def_hp, trg, file_sym, this);
        }
#ifdef _PSET_DEBUG
    to->is_consistent_hashElmts();
#endif
}

static bool subsys_deletion(symbolSet& def_symbols, xrefSymbol* dfs, XrefTable* Xr) {
    Initialize(subsys_deletion);

    if (def_symbols.size() == 2) {
        symbolPtr subsys;
        ForEachT(subsys, def_symbols) {
            if (subsys.get_kind() == DD_SUBSYSTEM) {
                xref_notifier_report(-1, subsys);
                linkTypes lks;
                lks.add(grp_has_client);
                lks.add(grp_has_peer);
                lks.add(grp_is_peer_of);
                lks.add(grp_has_server);
                lks.add(grp_has_pub_mbr);
                lks.add(grp_has_pri_mbr);
                lks.add(grp_has_trans_mbr);
                // following links are not currently used by groups, but
                // are included for upward compatibility with old subsystems
                lks.add(is_using);
                lks.add(used_by);
                lks.add(has_superclass);
                lks.add(has_subclass);
                lks.add(ref_file);
                subsys->rm_link(lks, false, subsys.get_xref()->get_lxref());
                dfs->rm_link(file_ref, (int) false, Xr);
                dfs->rm_link(is_defined_in, (int) false, Xr);
                dfs->rm_link(is_defining, (int) false, Xr);
                return true;
            }
        }
    }
    return false;
}

extern Xref* last_Xref;

void XrefTable::remove_module(char const *module_name)
{
    Initialize(XrefTable::remove_module1);

#ifdef _PSET_DEBUG_PRINT
    //    Debug information only .............     ?????????????  remove before doing put
    char const *name = symfile_name;
    if (units_are_backup) name = symbackup;
    cout << "---------- Removing module " << module_name << " from " << name << endl;
#endif
    xrefSymbol * dfs = lookup_module(module_name);
    if (!dfs || !dfs->get_has_def_file(this)) return;
    last_Xref = get_Xref();
    symbolSet all_symbols(1);
    symbolSet def_symbols(1);
    // collecting all relavent symbols in this file
    all_symbols.insert(symbolPtr(last_Xref,dfs));
    def_symbols.insert(symbolPtr(last_Xref,dfs));
    dfs->get_all_links(all_symbols, def_symbols, last_Xref);
    // collecting all relavent link in this file

    if (!subsys_deletion(def_symbols, dfs, this)) {
        symbolPtr s1,sym;

        ForEachT(s1, all_symbols) {
            s1->remove_module_link(def_symbols, last_Xref);
        }

        ForEachT (sym, def_symbols) {

            ////////////////////////
            // BEGIN NASTY KLUDGE //
            ////////////////////////

            if (sym.get_kind() == DD_RELATION) {

                symbolArr	links;
                symbolPtr	sp;

                sym->get_link (is_using, links, 0);

                ForEachS (sp, links) {
                    if (sp.get_kind() == DD_CLASS)
                        xref_notifier_report (0, sp);
                }
            }

            //////////////////////
            // END NASTY KLUDGE //
            //////////////////////

            xref_notifier_report (-1, sym);   
        }
    }

    dfs->rm_link((linkType) lt_lmd, this);      // remove date link
    dfs->rm_link((linkType) lt_lmdhigh, this);      // remove date link
    if (!SharedXref::saving())
        xref_notifier_report(-1, dfs);
}

#ifdef _PSET_DEBUG_PRINT
extern void p_link(linkType lt, int i);
void print1_name_and_def(xrefSymbol* sym, XrefTable* sym_xrt)
{
    genString defined;
    symbolPtr defsym = sym->get_def_file();
    if (defsym.xrisnotnull()) {defined = " defined in "; defined += defsym.get_name();}
    else defined = " (no defined-in)";
    cout << "-> " << sym->get_name(sym_xrt) << (char const *)defined << endl;
}    
void print1_link(char const *descriptor, xrefSymbol* sym, int lt, xrefSymbol *trg, XrefTable* trg_Xr)
{
    cout << descriptor << ": " << (char const *)sym->get_name() << "-> ";
    p_link((linkType)lt, 0);
    print1_name_and_def(trg, trg_Xr);

    //    genString defined;
    //   symbolPtr defsym = trg->get_def_file();
    //    if (defsym.xrisnotnull()) {defined = " defined in "; defined += defsym->get_name();}
    //    else defined = " (no defined-in)";
    //    cout << "-> " << trg->get_name() << (char const *)defined << endl;

    //    cout << "-> " << trg->get_name() << " defined in " << trg->get_def_file()->get_name() << endl;

    //    lnk->get_symbol_of_link()->print(1);
}
#endif

static int copy_module_liberal_link(ddKind k)
{
    return (k == DD_SEMTYPE /*|| k == DD_ASSOCLINK*/ || k == DD_TYPEDEF);
}
// this is the source :-)
// iterate thru all links in src, if link belong to file_sym, copy it 
// to trg, use has_link to test
//   note: "this" is promised as belongs-to file
void xrefSymbol::copy_module_link(hashPair* hp, hashPair*def_hp, xrefSymbol* trg, xrefSymbol* fs, XrefTable* my_xrt)
{
    Initialize(xrefSymbol::copy_module_link);
    Xref* trg_xr = trg->get_Xref();
#ifdef _PSET_DEBUG_PRINT
    cout << "---Copying from symbol ";
    print1_name_and_def(this, my_xrt);                               // ??????????? remove before putting the file
#endif

    uint sind = get_symbol_index();
    Link* lnk = get_start_link(sind, my_xrt);
    while (lnk && !last_link(lnk)) {
        if (lnk->symbol_link()) {
#ifdef _PSET_DEBUG_PRINT
            int flag = 0;
#endif
            xrefSymbol* dest = lnk->get_symbol_of_link(my_xrt);
            if (link_type_converter::is_reverse_link(lnk->lt())) {      // example links:  included_by, friends_of, ...
                xrefPair* xp = def_hp->lookup(dest);        // make sure destination of link belongs-to file
                if (xp) {
                    trg->add_link(lnk->lt(), xp->trg, trg_xr);
#ifdef _PSET_DEBUG_PRINT
                    flag = 1;
                    print1_link("cp r",this, lnk->lt, dest, my_xrt);                 // ???  debug information only    REMOVE BEFORE PUTTING
                    trg->is_consistent(0);                        //   ???????  debug testing only  REMOVE all of these BEFORE PUTTING
#endif
                } 
#ifdef  _PSET_DEBUG_PRINT
                else   cout << "  not reverse:" << endl;                       // ???? debug information
#endif
            }
            if (link_type_converter::is_forward_link(lnk->lt())) {      // example links:  is_including, have_friends, ....
                xrefPair* xp = hp->lookup(dest);           // make sure destination belongs-to file
                xrefPair* xp2 = def_hp->lookup(this);      // make sure this symbol is defined-in the file
                if (xp && xp2) {
                    trg->add_link(lnk->lt(), xp->trg, trg_xr);
#ifdef _PSET_DEBUG_PRINT
                    flag = 1;
                    print1_link("cp f",this, lnk->lt, dest, my_xrt);                 // ???  debug information only    REMOVE BEFORE PUTTING
                    trg->is_consistent(0);                        //   ???????  debug testing only  REMOVE all of these BEFORE PUTTING
#endif
                }
#ifdef _PSET_DEBUG_PRINT
                else   cout << "  not forward:" << endl;                       // ???? debug information
#endif
            }
            if ( copy_module_liberal_link(dest->get_kind()) ||
                 copy_module_liberal_link(get_kind())) {
                xrefPair* xp = hp->lookup(dest);           // make sure destination belongs-to file
                if (xp) {
                    trg->add_link(lnk->lt(), xp->trg, trg_xr);
#ifdef _PSET_DEBUG_PRINT
                    flag = 1;
                    print1_link("cp h",this, lnk->lt, dest, my_xrt);                 // ???  debug information only    REMOVE BEFORE PUTTING
                    trg->is_consistent(0);                        //   ???????  debug testing only  REMOVE all of these BEFORE PUTTING
#endif
                }
#ifdef _PSET_DEBUG_PRINT
                else   cout << "   ERROR: HAS_TYPE and TYPE_HAS must lead to belongs-to:" << endl;                       // ???? debug information
#endif
            }
#ifdef _PSET_DEBUG_PRINT
            if (!flag) print1_link("nocp", this, lnk->lt, lnk->get_symbol_of_link(this_Xr),this_Xr);         // ??? debug information only  REMOVE BEFORE PUTTING
#endif
        }
        else if (lnk->num_link() && 
                 ((fs == this) || ((linkType)lnk->lt() == lt_filetype)))
            trg->add_link((linkType) lnk->lt(), (long) lnk->get_index(), trg_xr->get_lxref());
        lnk = get_next_link(lnk, sind, my_xrt);
    }
}

static bool remove_module_conservative_link(ddKind k)
//  any link that has an endpoint of one of these kinds does
//     NOT get removed when we do a remove_module_link()
{
    return ( //k == DD_REL_SRC || k == DD_REL_TRG ||
            //k == DD_ASSOCLINK || k == DD_SOFT_ASSOC ||
            k == DD_IFL_SRC || k == DD_IFL_TRG);
}
//   for a given file (module) that we are parsing or removing,
//  def_hp  is a full list of symbols that all are defined-in a file
//  this   is a particular symbol that    belongs-to a file
void xrefSymbol::remove_module_link(symbolSet & def_hp, Xref* xr)
{
    Initialize(xrefSymbol::remove_module_link);
    XrefTable* xrt = xr->get_lxref();
    uint sind = get_symbol_index();
    Link* lnk = get_start_link(sind, xrt);
    while (lnk && !last_link(lnk)) {
        if (link_type_converter::is_reverse_link(lnk->lt()) && type_has!=lnk->lt()) {
            // example links: included_by, friends_of, ...
            // note:  type_has  is not removed, since it could be coming from other files, too
            //     besides, it appears that insert_module does not put it back in
            xrefSymbol* dest = lnk->get_symbol_of_link(xrt);
            symbolPtr sym(xr,dest);	  
            if (def_hp.includes(sym)) {
                if (!remove_module_conservative_link(get_kind())
                    && !remove_module_conservative_link(dest->get_kind())) {
                    // we only remove links whose endpoints are not in the conservative list
#ifdef _PSET_DEBUG_PRINT
                    print1_link("remv",this, lnk->lt, dest,xrt);                 // ???  debug information only    REMOVE BEFORE PUTTING
#endif
                    rm_link((linkType) lnk->lt(), dest, xrt);      // remove both links
                }
            }
        }
        //    else if (lnk->num_link())
        //      rm_link((linkType) lnk->lt);
#ifdef _PSET_DEBUG_PRINT
        is_consistent(0);
#endif
        lnk = get_next_link(lnk, sind, xrt);
    }
}

void xrefSymbol::build_remove_module_link( symbolSet& def_hp, symbolSet& all_hp, hashPair *rhps, Xref* xr)
{
    Initialize(xrefSymbol::build_remove_module_link);
    if (xr == 0)
        xr = get_Xref();
    IF (xr == 0) return;
    XrefTable* xrt = xr->get_lxref();
    uint sind = get_symbol_index();
    Link* lnk = get_start_link(sind, xrt);
    while (!last_link(lnk)) {
        if (lnk->symbol_link()) {
            int conservative = 0;
            xrefSymbol* dest = lnk->get_symbol_of_link(xrt);
            symbolPtr sym(xr,dest);
            if (all_hp.includes(sym) &&
                link_type_converter::is_reverse_link(lnk->lt())) {
                int process_lnk = (type_has != lnk->lt());
                if (process_lnk && def_hp.includes(sym)) {
                    if (!remove_module_conservative_link(get_kind())
                        && !remove_module_conservative_link(dest->get_kind())) {
                        conservative = 1;
                    }
                }
                if (type_has == lnk->lt()) {
                    symbolArr rfa;
                    get_link(ref_file, rfa, 1);
                    if ((rfa.size() == 1) && def_hp.includes(rfa[0]))
                        process_lnk = 1;
                }
                if (process_lnk) {
                    xrefPair* xp = rhps->insert(this, (linkType) lnk->lt(), dest);
                    if (!conservative && xp)
                        xp->set_mark();
                    xp = rhps->insert(dest, get_reverse_link(lnk->lt()), this);
                    if (!conservative && xp)
                        xp->set_mark();
                }
            }
        }
        lnk = get_next_link(lnk, sind, xrt);
    }
}
 
void Xref::tidy_xref()
{
    Initialize(Xref::tidy_xref);

    XrefTable *To = get_lxref();
    if (!To)
        return;

    genTmpfile tmpfile;
    Xref* xref = new Xref(tmpfile.name(), 0, 0);

    projHeader* head = new projHeader ("Tidy_Xref", "/tmp");
    projNode* pr = new projNode ("/tmp", head, "/tmp");
    pr->set_xref(xref); 
    projList::search_list->add_proj (pr);

    To->tidy_xref(xref->get_lxref());
    XrefTable *From = xref->get_lxref();
    OS_dependent::bcopy(From->sym_header, To->sym_header, 
                        ntohl(From->header->next_offset_sym) + ntohl(From->sym_header->offset_to_data));
    OS_dependent::bcopy(From->ind_header, To->ind_header,
                        ntohl(From->header->next_offset_ind) + ntohl(From->ind_header->offset_to_data));

    save_lxref_by_whole();
 
    // delete the stuff we allocated here
    projList::search_list->rem_proj (pr);
    delete head;        // delete projHeader, and it'll delete projNode
    delete xref;        // delete the xref
}

void Xref::remove_module(char const *module_name)
{
    Initialize(Xref::remove_module);
    XrefTable * xrt = get_lxref();
    if (xrt)
        xrt->remove_module(module_name);

}


xrefPair::xrefPair(xrefSymbol* s)
{
    src = s;
    lt = (linkType)0xff;		// linktype not used (and neither is trg)
    int second_level_size = SECOND_LEVEL_SIZE;
    if (src->get_kind() == DD_MODULE)
        second_level_size = MAX_HASH;
    else if (src->get_kind() == DD_CLASS)
        second_level_size = 100;
    hp = new hashPair (SECOND_LEVEL_SIZE);
    next = 0;
}

xrefPair::~xrefPair()
{
    if (hp && (hp != (hashPair*) 0x1))
        delete hp;
}

hashPair::hashPair(int size)
{
    //  Initialize(hashPair::hashPair);
    table = (xrefPair**) psetmalloc(size * (sizeof(xrefPair*)));
    for(int i = 0; i < size; ++i)
        table[i] = 0;
    next_xp = 0;
    cur_ind = -1;
    cnt = 0;
    table_size = size;
}
hashPair::~hashPair()
{
    //  Initialize(hashPair::~hashPair);
    for(int i = 0; i < table_size; ++i){
        xrefPair* xp = table[i];
        while(xp) {
            xrefPair* pnext = xp->next;
            delete xp;
            xp=pnext;
        }
    }
    OSapi_free(table);
}
void hashPair::init()
{
    //  Initialize(hashPair::init);
    for(int i = 0; i < table_size; ++i){
        xrefPair* xp = table[i];
        while(xp) {
            xrefPair* pnext = xp->next;
            delete xp;
            xp=pnext;
        }
        table[i] = 0;
    }
}


inline int hashPair::hash(xrefSymbol* s)
{
    return (((unsigned int) s) % table_size);
}

xrefPair* hashPair::lookup(xrefSymbol* src)
{
    //  Initialize(hashPair::lookup(xrefSymbol*));
    int h = hash(src);

    for (xrefPair* xp = table[h]; xp; xp = xp->next)
        if (xp->src == src) return xp;
  
    return 0;
}

xrefPair* hashPair::lookup(xrefSymbol* src, linkType lt)
//  lookup a link (lt) to destination symbol (misnamed src)
{
    //  Initialize(hashPair::lookup(xrefSymbol*, linkType));
    int h = hash(src);

    for (xrefPair* xp = table[h]; xp; xp = xp->next)
        if ((xp->src == src) && (xp->lt == lt))
            return xp;
    return 0;
}
    

xrefPair* hashPair::lookup(xrefSymbol* src, xrefSymbol* targ)
//  lookup a connection between two symbols
//    (from 2 different pmods, the way it is currently used)
{
    //  Initialize(hashPair::lookup(xrefSymbol*,xrefSymbol*));
    int h = hash(src);

    for (xrefPair* xp = table[h]; xp; xp = xp->next)
        if ((xp->src == src) && (xp->trg == targ))
            return xp;
    return 0;
}
    

xrefPair* hashPair::insert(xrefSymbol* src, xrefSymbol* trg)
{
    //  Initialize(hashPair::insert(xrefSymbol*,xrefSymbol*));
    xrefPair* xp = lookup(src,trg);
    if (xp)
        return xp;                     // ignore if it is a duplicate
    int h = hash(src);
    xp = new xrefPair(src, trg);
    xp->next = table[h];
    table[h] = xp;
    cnt ++;
    return xp;
}

xrefPair* hashPair::lookup(xrefSymbol* src, linkType lt, xrefSymbol* trg)
{
    //  Initialize(hashPair::lookup(xrefSymbol*,linkType,xrefSymbol*));
    xrefPair * xp = lookup(src);
    if (xp == 0) return 0;
    return (xp->hp)->lookup(trg, lt);
}

xrefPair* hashPair::insert(xrefSymbol* src, linkType lt, xrefSymbol* trg)
{
    //  Initialize(hashPair::insert(xrefSymbol*,linkType,xrefSymbol*));
    xrefPair* xp = lookup(src);
    if (xp == 0) {
        int h = hash(src);
        xp = new xrefPair(src);
        xp->next = table[h];
        table[h] = xp;
    }
    cnt ++;
    return (xp->hp)->insert(trg, lt);
}


xrefPair* hashPair::insert(xrefSymbol* src, linkType trg)
{
    //  Initialize(hashPair::insert(xrefSymbol*,linkType));
    xrefPair* xp = lookup(src,trg);
    if (xp)
        return xp;                     // ignore if it is a duplicate
    int h = hash(src);
    xp = new xrefPair(src, trg);
    xp->next = table[h];
    table[h] = xp;
    cnt ++;
    return xp;
}


// Note:  get_first, get_next are not "re-entrant".  They store their
//  iterator information inside the hashPair structure, in
//        cur_ind and cur_xp
xrefPair* hashPair::get_first(int i)
// i=1 means we do not want lt_junk links
{
    //  Initialize(hashPair::get_first);
    next_xp = 0;
    cur_ind = -1;
    return get_next(i);
}

  
xrefPair* hashPair::get_next(int i)
// i=1 means we do not want lt_junk links
{
    //    Initialize(hashPair::get_next);
    while (1) {
	while (next_xp) {
	    xrefPair* xp = next_xp;
	    next_xp = next_xp->next;
	    if ((i==0) || (xp->lt != lt_junk))
		return xp;
	}
	cur_ind++;
	if (cur_ind >= table_size)
	    break;
	next_xp = table[cur_ind];
    }
    return 0;
}

static char const *SwtEntityStatus_cockpit_name(SwtEntityStatus MOD_KIND)
{
    switch (MOD_KIND)
        {
        case SWT_NEW:
            return "NEW";
        case SWT_MODIFIED:
            return "MOD";
        case SWT_UNCHANGE:
             return "UNC";
        }
    return "";
}

#define num_to_kind(x) ""
#define start()

static void sdo_encode_one_sym(symbolPtr sym, genString &str1)
{
    start();
    if (sym.xrisnull()) return;
    genString str;  
    if (sym->get_kind() == DD_MODULE) {
        str.printf("%s|0|\n", sym.get_name());
    }
    else {
        SwtEntityStatus MOD_KIND= (SwtEntityStatus) sym->get_attribute(SWT_Entity_Status,2);
        symbolPtr def=sym->get_def_file();
        char const *def_name = " ";
        if (def.xrisnotnull())
            def_name = def.get_name();
        time_t cl = sym.get_last_mod_date();
        char date[20];
        char time[20];
        struct tm *tmp = localtime(&cl);
        if (tmp) {
            strftime(date, 20, "%D", tmp);
            strftime(time, 20, "%T", tmp);
        }
        else {
            strcpy(date, "00:00:00");
            strcpy(time, "00:00:00");      
        }
        str.printf("%s|%s|%s|%s|%s|%s\n", sym.get_name(), num_to_kind((int)sym.get_kind()),
                   def_name, SwtEntityStatus_cockpit_name(MOD_KIND),date,time);
    }
    if (str.str())
        str1 += str;
}

void XrefTable::file_trigger(symbolArr &inp, genString &to_sdo)
{
    Initialize(XrefTable::file_trigger);
  
    symbolPtr sym1;
    ForEachS(sym1, inp) {
        sdo_encode_one_sym(sym1, to_sdo);
    }
}

int  file_get_modified_object(symbolPtr, symbolArr &, time_t );
void XrefTable::code_trigger(symbolArr &files, genString &to_sdo, int ti)
{
    Initialize(XrefTable::code_trigger);
  
    symbolArr modified;
    if (files.size() == 0) {
        ddSelector sel;
        sel.add(DD_MODULE);
        add_symbols(files, sel);
    }

    time_t sdo_cl = (time_t) -1;
    if (ti)
        get_last_sdo_update(&sdo_cl);

    symbolPtr file;
    ForEachS(file, files) {
        file_get_modified_object(file, modified, sdo_cl);
    }
  
    symbolPtr sym1;
    ForEachS(sym1, modified) {
        if (sym1.get_kind() != DD_MODULE)
            sdo_encode_one_sym(sym1, to_sdo); 
    }
    
}
