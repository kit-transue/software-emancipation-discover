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
#include <fcntl.h>

#include <genTmpfile.h>
#include <genWild.h>

#include <linkTypes.h> /* NY, 09.20.95 */
#include <_Xref.h>
#include <SharedXref.h>
#include <link_type_converter.h> /* NY, 09.20.95 */
#include <indHeaderInfo.h>       /* NY, 09.20.95 */
#include <xrefSymbol.h>          /* NY, 09.20.95 */
#include <XrefTable.h>           /* NY, 09.20.95 */
#include <proj.h>                /* NY, 09.20.95 */

#include <fileCache.h>
#include <ParaCancel.h>
#include <transaction.h>
#include <driver_mode.h>

#include <RTL_externs.h>

#include <systemMessages.h>
#include <Question.h>
#include <vpopen.h>
#include <machdep.h>
#include <symbolSet.h>
 
#include <transaction.h>
 
#include <cmd.h>

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <globals.h>
#include "gen_hash.h"

#ifndef _linkType_selector_h
#include <linkType_selector.h>
#endif

#include "hashtable.h"
#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <scopeMgr.h>
#include <scopeUnit.h>

#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */

init_relational(Xref,scopeNode);

int Xref_get_links_global(xrefSymbol*,linkTypes&,int,symbolArr&,xrefSymbol*);

XrefTable* XrefTableNull = (XrefTable*)1;
int get_links_global_one_layer_internal(xrefSymbol*,linkTypes&,int,symbolArr&,xrefSymbol*,objSet&,objSet&,objSet&,Xref*);
void get_projs_to_search(objSet & os);
// declaration for function used only in this file
static int get_all_subprojects_from_proj(objSet & obs, projNode* pn, int deep = 0);

int call_from_put();

int Xref::mode;

// Link type converter decls
char link_type_converter::belong_to_file[MAX_LINK_TYPE];
char link_type_converter::forward_link[MAX_LINK_TYPE];
char link_type_converter::reverse_link[MAX_LINK_TYPE];

// last_Xref is a global (should be file static) variable that records the most
//   recently used Xref.  This speeds up get_Xref().  It is NULLed again in
//   the destructor of an Xref, if it happens to point to the one being 
//   destroyed.
Xref* last_Xref = NULL;

extern "C" void connect_to_cockpit();

void Xref::symbols_get_links(symbolArr & syms1, objSet & prs, linkTypes &la,
                             symbolArr &outputs)
{
    Initialize(Xref::symbols_get_link);
    symbolSet symset;
    symset.insert(outputs);    


    /*Help in testing the triggers
      connect_to_cockpit();
  

      symbolArr sym_files;
      symbolPtr temp,temp2;
      temp=lookup_file_symbol("/aset/dd/src/Xref.h.C");
      temp2=lookup_file_symbol("/aset/driver/src/main.C");
      sym_files.insert_last(temp);
      sym_files.insert_last(temp2);



      static int fff=-1;
      if(fff==-1)
      {

      dis_ci_trigger(sym_files);
      printf("-------------------\n");
      dis_co_trigger(sym_files);
      printf("--------------------------------------------\n");
      dis_update_trigger(sym_files);
      fff=0;
      }
    */


    symbolPtr sym1;
    symbolArr syms = syms1;
    symbolArr syms_done;
    linkTypes la1 = la;
    symbolArr sa;
    Obj* obj;
    symbolArr output;
    ForEach(obj, prs) {
        projNode* subpr = checked_cast(projNode, obj);
        XrefTable* xrt;
        Xref* Xr = subpr->get_xref(1);
        if (Xr && (xrt = Xr->get_lxref())) {
            symbolPtr sym;
            symbolPtr sym2;
            symbolPtr loc_sym;
            ForEachS(sym, syms) {
                if (!sym.is_xrefSymbol()) continue;
                sa.removeAll();
                symbolPtr dfs_sym;
                if (sym.get_has_def_file())
                    dfs_sym = sym->get_def_file();
                if (multiple_psets)
                    loc_sym = xrt->lookup(sa, sym.get_kind(), sym.get_name(), 0);
                else
                    loc_sym = xrt->lookup(sa, sym.get_kind(), sym.get_name(), dfs_sym);
                for (int ii = 0; ii < sa.size() ; ++ii) {
                    loc_sym = sa[ii];
                    loc_sym.get_links(la1, output, 1);
                    ForEachS(sym2, output) {
                        symset.insert(sym2);    
                    }
                    output.removeAll();
                    if (la1.local_only())
                        syms_done.insert_last(sym);
                    la1.reset(la);
                }
            }
            symbolPtr sym3;
            ForEachS(sym3, syms_done) {
                syms.remove(sym3);
            }
            syms_done.removeAll();
        }
    }
    sym1 = symset.get_first();
    outputs.removeAll();
    while (sym1.isnotnull()) {
        outputs.insert_last(sym1);
        sym1 = symset.get_next();
    }

}

XrefTable* Xref::get_lxref (int i)
{
    if (lxref == XrefTableNull)
	if (i == 0) {
	    Initialize(Xref::get_lxref);
	    genString sym_file;
	    genString ind_file;
	    genString link_file;
	    Xref::sym_file(xref_name, sym_file);
	    Xref::ind_file(xref_name, ind_file);
	    Xref::link_file(xref_name, link_file);
	    int perm = unreal_flag ? 2 : perm_flag;
	    // suppress backup copy of other subproject pmods during buildxref, by making perm=3
	    if (!perm && is_model_build())  perm = 3;
	    lxref = new XrefTable(this, sym_file, ind_file, link_file, perm );
	    if (lxref && lxref->get_header() == 0) {
		delete lxref;
	        lxref = NULL;
  	    }
	    if (!lxref)
                writable_on_disk = 1;
	} else
	    return NULL;
    return lxref;
}


bool Xref::is_project_writable()
{
    Initialize(Xref::is_project_writable);
    return(perm_flag != READ_ONLY);
}


//  if there isn't enough room, move the xref symbol area now
bool Xref::make_room(int size)
{
    Initialize(Xref::make_room);

    int retval = 0;
    
    XrefTable *xr = get_lxref();
    if (xr) {
	retval = xr->make_room(size);
    }
    return retval;
}

void Xref::remove_all_symptrs_from_xrefTable(const ObjPtr)   // static function
//  Any project having an xrefTable
//  this function calls itself recursively for subprojects
//  BUGBUG -- ask Trung if an xrefTable is ever at any level lower than the root project ?????????????????
//     if not, remove "done" flag, and recursive call logic
{
    Initialize(Xref::remove_all_symptrs_from_xrefTable);
}


void Xref::xref_notifier_report_proj(const int action, const ObjPtr ob)
// call xref_notifier_report on all symbols corresponding to a project
// calling myself recursively for as many subprojects as have
// pmods already loaded
//  this is called to delete symbols when a project is hidden
{
    Initialize(xref_notifier_report_proj);
    projNode* pn = checked_cast(projNode,ob);
    Xref* Xr = pn->get_xref(1);
    // get Xref if there is one already located
    if (Xr) {
	XrefTable *xrl = Xr->get_lxref();
	if (xrl && !Xr->is_unreal_file() ) {
	    // process this pmod
            //	    cerr << "notifier_report_proj: start processing " << (char const *)Xr->xref_name << endl;
	    uint noi = ntohl(xrl->header->next_offset_ind);
	    for (int i = 0; i*SIZEOF_XREFSYM < noi; ++i) {
		xrefSymbol* sym = (xrefSymbol*) (xrl->get_ind_addr() + i);
		symbolPtr symp(Xr, sym);
		xref_notifier_report(-1,symp);
	    }
	    xref_notifier_apply();   // fake an "end_transaction" to flush more often
            //	    cerr << "notifier_report_proj: end processing   " << (char const *)Xr->xref_name << endl;
	    return;
	}
    }
    Obj* obtemp;
    Obj* os = parentProject_get_childProjects(pn);
    ForEach(obtemp, *os) {
	xref_notifier_report_proj(action, obtemp);
    }
}


symbolPtr Xref::lookup_in_lxref(symbolArr& arr, ddKind simKind, char const *name)
{
    return symbolPtr(get_lxref ()->lookup(arr, simKind, name, 0));
}

//------------------------------------------
// Xref::max_offset() const
//------------------------------------------

unsigned int Xref::max_offset() const {
    Initialize(Xref::max_offset);

    unsigned int retval = 0;

    if (lxref && lxref != XrefTableNull)
        retval = lxref->max_offset();

    return retval;
}

void Xref::insert_module_in_lxref(app* head)
{
    get_lxref ()->insert_module(head);
}
void Xref::insert_SWT_Entity_Status (app* head)
{
    XrefTable *xr = get_lxref();
    if (xr) 
	xr->insert_SWT_Entity_Status (head);
}

void Xref::sym_file (char const *symf, genString& s_f)
{
    s_f = symf;
    s_f += ".sym";
}

void Xref::ind_file(char const *symf, genString& s_f)
{
    s_f = symf;
    s_f += ".ind";
}

void Xref::link_file(char const *symf, genString& l_f)
{
    l_f = symf;
    l_f += ".lin";
}

// This function should be called only on a home project for which the pmod files did not exist
void Xref::make_real(char const *xref_file)
{
    if (!unreal_flag) return;
    if (get_lxref (1)) { //  && lxref->get_header() == 0) {
	delete lxref;
	lxref = NULL;
    }
    xref_name = xref_file;
    genString sym_file;
    genString ind_file;
    genString link_file;
    Xref::sym_file(xref_file, sym_file);
    Xref::ind_file(xref_file, ind_file);
    Xref::link_file(xref_file, link_file);
    lxref = new XrefTable(this, sym_file, ind_file, link_file, 0);
    if (lxref && lxref->get_header() == 0) {
        delete lxref;
        lxref = NULL;
    }
    perm_flag = 0 ;
    unreal_flag = 0;  //writable, real file  
    int test = OS_dependent::access(sym_file,W_OK);	
    writable_on_disk = lxref ? !((test == 0) || (errno == ENOENT)) : 1;

}


// perm   = 0 for writable, 1 for read only,
// 2 for read-only unreal file (do not save later)
Xref::Xref(char const *xref_file, int perm, projNode* pr)
{
    Initialize(Xref::Xref );

    if (perm == -1) {// XrefTable_temp
        crash_recovery = 0;
        perm_flag = 1;
        unreal_flag = 0;
        writable_on_disk = 1;
        is_root = 0;
        return;
    }
    if (perm==0 && pr && !pr->is_writable())
        perm = 1;
    crash_recovery = 0;
    xref_name = xref_file;
    genString sym_file;
    genString ind_file;
    genString link_file;
    Xref::sym_file(xref_file, sym_file);
    Xref::ind_file(xref_file, ind_file);
    Xref::link_file(xref_file, link_file);
    //  lxref = new XrefTable(this, sym_file, ind_file, link_file, perm );
    lxref = XrefTableNull; my_proj = pr;
    perm_flag = perm!=0 ; // 1 for readonly, 0 for writeable
    unreal_flag = (perm==2);   //  readonly temporary file, discard later
    //  if (lxref && lxref->get_header() == 0) {
    //    delete lxref;
    //    lxref = NULL;
    //  }
    int test = OS_dependent::access(sym_file,W_OK);	
    //  writable_on_disk = lxref ? !((test == 0) || (errno == ENOENT)) : 1;
    if ((test != 0) && (errno != ENOENT)) {
        writable_on_disk = 1; // read only
        perm_flag = 1;        // read only
    } else {
        writable_on_disk = 0; // disk write access
    }
    if (unreal_flag) writable_on_disk = 1;  // if unreal do not write to it
    is_root = 1;

    if (my_proj == projNode::get_control_project())
        home_flag = 1;
    else
        home_flag = 0;
}


// Implementation of class SharedXref follows

// The sharedXref is used only for saving the
// files. It goes away immediately after saving.
// Essentially, it is a shared map of the xref files
// on disk. Non-pertinent parts of the Xref are
// defaulted (eg, xref_table & lxref are set to NULL).

// *******************************************************

void Xref::init(int perm)
{
    Initialize(Xref::init );
    genString sym_file;
    genString ind_file;
    genString link_file;
    Xref::sym_file(xref_name, sym_file);
    Xref::ind_file(xref_name, ind_file);
    Xref::link_file(xref_name, link_file);
    if (get_lxref (1)) delete lxref;
    lxref = new XrefTable(this, sym_file, ind_file, link_file, perm );
    perm_flag = perm; // 1 for readonly, 0 for writeable
    if (lxref->get_header() == 0) {
        delete lxref;
        lxref = NULL;
    }
    int test = OS_dependent::access(sym_file,W_OK);	
    writable_on_disk = lxref ? !((test == 0) || (errno == ENOENT)) : 1;
    if (unreal_flag) writable_on_disk = 1;   // if unreal, do not write to it

    if (lxref && lxref->get_header())
        crash_recovery_pmod_from_pset();

}

Xref::~Xref()
{
    if (get_lxref (1)) {
        delete lxref;
        if (!call_from_put()) {
            genString crash_file_name = xref_name;
            crash_file_name += ".outdated_pset";
            OSapi_unlink(crash_file_name.str());
        }
    }
    lxref = NULL;

    if (last_Xref == this) last_Xref = 0;
}

// copy updated info from the private memory map of the Xref
// to the Xref file on disk, via an additional temporary shared map

void Xref::save_lxref_by_whole()
{
    Initialize(Xref::save_lxref_by_whole);

    IF (SharedXref::get_cur_shared_xref())
        return;

    if (get_lxref () && ! lxref->nosaveflag) {
        XrefTable *From = lxref;
        XrefTable *To = From->get_backup ();
        SharedXref SXr (To);

        if (To) {
            OS_dependent::bcopy(From->sym_header, To->sym_header, 
                                ntohl(From->header->next_offset_sym) + ntohl(From->sym_header->offset_to_data));
            OS_dependent::bcopy(From->ind_header, To->ind_header, 
                                ntohl(From->header->next_offset_ind) + ntohl(From->ind_header->offset_to_data));
            OS_dependent::bcopy(From->link_header, To->link_header, ntohl(From->header->next_offset_link) + ntohl(From->link_header->offset_to_data));
            delete From->backup;
            From->backup=0;
        }
    }
}

/* The following classes are used for the "file_proj_cache" for
 * Xref::lookup().
 */

class file_and_proj: public hash_item {
public:
    file_and_proj(char const *filename): fname(filename), projp(NULL) { }
    ~file_and_proj();
    file_and_proj* next() { return (file_and_proj*) hash_item::next(); }
    int operator==(const hash_item&) const;
    file_and_proj* clone(projNode*);
    char const *filename() { return fname; }
    projNode* proj() { return projp; }

private:
    char const *fname;
    projNode* projp;
};

file_and_proj::~file_and_proj() {
    Initialize(file_and_proj::~file_and_proj);

    if (projp) {	// was cloned
        OSapi_free((void*) fname);
    }
}

int file_and_proj::operator==(const hash_item& hi) const {
    Initialize(file_and_proj::operator==);

    return strcmp(fname, ((const file_and_proj&) hi).fname) == 0;
}

file_and_proj* file_and_proj::clone(projNode* projNodep) {
    Initialize(file_and_proj::clone);

    file_and_proj* p = new file_and_proj(strdup(fname));
    p->projp = projNodep;
    return p;
}

class file_proj_hash: public hash_table {
public:
    file_proj_hash(size_t num_buckets): hash_table(num_buckets) { }
    ~file_proj_hash();
    file_and_proj* find(const file_and_proj& item) {
        return (file_and_proj*) hash_table::find(item);
    }
private:
    size_t hash(const hash_item*);
};

file_proj_hash::~file_proj_hash() {
    Initialize(file_proj_hash::~file_proj_hash);

    for (size_t i = 0; i < bucket_count; i++) {
        hash_item* itemp = bucket_list[i].first();
        while (itemp) {
            hash_item* next = itemp->next();
            delete itemp;
            itemp = next;
        }
    }
}

size_t file_proj_hash::hash(const hash_item* itemp) {
    Initialize(file_proj_hash::hash);

    char const *name = ((file_and_proj*) itemp)->filename();
    size_t result = 0;
    for (char const *p = name; p && *p; p++) {
        result = (result << 1) + *p;
    }
    return result;
}


/* Xref::lookup been rewritten as follows:
 *
 * 1) If "file_name" is supplied, do a first pass through the projects
 * looking for that module and only look up the symbol in projects that
 * define the module.  If "file_name" is not supplied or if it was not
 * found in any project, then look up the symbol in all projects.  The
 * only exception is in cases where the symbol exists in the home project;
 * for efficiency, it is always returned if in the home project, whether
 * the file symbol was found there or not.
 *
 * 2) Cache the results of "get_projs_to_search" in static variable "projs."
 *
 * 3) Cache the project(s) in which filenames are found in the hash table
 * "file_proj_cache".
 *
 * This should be both more efficient and also less likely to find the
 * wrong symbol, that is, a symbol with the same name but that is defined
 * in a different file.  It can still be fooled if the same named file
 * appears in different projects.
 *
 * wmm, 960330.
 */

static objSet projs;
static file_proj_hash* file_proj_cache = NULL;
const size_t NUM_FILE_PROJ_HASH_BUCKETS = 11371; // empirical magic number

/* static */


xrefSymbol* Xref::lookup(symbolArr& as, ddKind k, char const *name, 
			 char const *file_name)
{
    Initialize(Xref::lookup);
    XrefTable* xr;
    Xref* Xr;
    xrefSymbol* sym;
    projNode* pr;
    file_and_proj key(file_name);
    if (file_name) {
        // First look in home project (assuming this is home proj Xref)
        xrefSymbol* fs = get_lxref ()->lookup_module(file_name);
        if (k == DD_MODULE && fs) return fs;
        if (sym = get_lxref()->lookup(as, k, name, fs)) return sym;

        // Now look in file_proj_cache to see where to look.
        if (file_proj_cache) {
            for (file_and_proj* itemp = file_proj_cache->find(key); itemp;
                 itemp = itemp->next()) {
                if (strcmp(itemp->filename(), file_name) == 0) {
                    pr = itemp->proj();
                    Xr = pr->get_xref(1);
                    if (Xr && (xr = Xr->get_lxref())) {
                        fs = xr->lookup_module(file_name);
                        if (sym = xr->lookup(as, k, name, fs)) return sym;
                    }
                }
            }
        }
        else file_proj_cache = new file_proj_hash(NUM_FILE_PROJ_HASH_BUCKETS);

        // Didn't find it, so we scan.  
        if (projs.size() == 0) {
            get_projs_to_search(projs);
        }
        Obj* ob;
        ForEach(ob, projs) {
            pr = checked_cast(projNode, ob);
            Xr = pr->get_xref(1);
            if (Xr && (xr = Xr->get_lxref())) {
                fs = xr->lookup_module(file_name);
                if (fs && fs->get_has_def_file(xr) &&
                    (sym = xr->lookup(as, k, name, fs))) {
                    file_proj_cache->insert(key.clone(pr));
                    return sym;
                }
            }
        }
    }

    // Didn't find it by looking for file, now try without defining file.
    // (File cache obviously doesn't help us here.)

    // First try home project:
    if (sym = get_lxref()->lookup(as, k, name, NULL)) return sym;

    // Nope, now search all projects:
    if (projs.size() == 0) {
        get_projs_to_search(projs);
    }
    Obj* ob2;
    ForEach(ob2, projs) {
        pr = checked_cast(projNode, ob2);
        Xref* Xr = pr->get_xref(1);
        if (Xr && (xr = Xr->get_lxref()))  {
            if (sym = xr->lookup(as, k, name, NULL)) return sym;
        }
    }

    return 0;
}

symbolPtr Xref::lookup_local(symbolArr& as, ddKind k, char const *name, 
                             char const *file_name)
//  If the pmod for this Xref is not mapped, try to map it
// then look up symbol in the pmod, returning the symbol, and
//    adding it to the symbolArr
{
    Initialize(Xref::lookup_local);
    xrefSymbol* fs = get_lxref()->lookup_module(file_name);
    if (k == DD_MODULE && fs) return fs;
    xrefSymbol* sym = get_lxref()->lookup(as, k, name, fs); 
    return symbolPtr(this, sym);
}

int Xref::get_links_global(xrefSymbol* sym, linkTypes &la, 
                           symbolArr& as, xrefSymbol* dfs)
{
    Initialize(Xref::get_links_global);
    return Xref_get_links_global(sym,la,0,as,dfs);
}
    
int  Xref::test_subproj_pmod(projNode* pn)                       // static
// This static member function finds out whether subproject pmods exist under
//  the specified project.  It returns:
//     int  -1   one or more of my children (at some level) have pmods
//           0    no pmods for me or any of my subprojects, at any level
//           1    I have a pmod, but none of my children do
{
    Initialize(Xref::test_subproj_pmod);
    // if this is the control project, it is not safe or necessary to do
    //   the "refresh_projects()"
    if (pn == projNode::get_control_project()) return 0;

    if(is_model_build())
	pn->refresh();
    else
	pn->refresh_projects();
    Obj* os = parentProject_get_childProjects(pn);
    // so lets get all its children
    ObjPtr ob;
    int ret=0;
    // now for each child:
    ForEach(ob, *os) {
	projNode* pr = checked_cast(projNode, ob);
	ret = test_subproj_pmod(pr);
	if (ret) return -1;      // if he or his children had pmod, we are thru
    }
    // if we got this far, none of our descendants had pmod files
    if (!pn) return 0;
    Xref* xr = pn->get_xref(1);
    // havepmod is true if I have a pmod
    int no_rec = -1;
    if (xr) {
        no_rec = xr->get_crash_recovery();
        xr->set_crash_recovery(1); // supress crash recovery
    }
    int havepmod= (xr && xr->get_lxref() && !xr->is_unreal_file());
    if (xr && (no_rec >= 0))
        xr->set_crash_recovery(no_rec);
    return havepmod;
}
    
extern void get_pmod_projs_from(objArr& subproj_Arr, projNode* pn);

void Xref::get_xreflist_from_proj(objArr& subproj_Arr, projNode* pn)       // static
{
    get_pmod_projs_from(subproj_Arr, pn);
}

static int get_all_subprojects_from_proj(objSet & obs, projNode* pn, int deep)
// Starting at pn, search for all subprojects, and add them to the objSet
// If deep is true, refresh as you go.  Otherwise assume that all subprojects
//   we are interested in are already refreshed.
// Return final number of projects in the set
//    (recursive fct)
{
    Initialize(get_all_subprojects_from_proj);
    if (deep)
        pn->refresh_projects();
    Obj* os = parentProject_get_childProjects(pn);
    ObjPtr ob;
    ForEach(ob, *os) {
        obs.insert(ob);
        projNode* pr = checked_cast(projNode, ob);
        get_all_subprojects_from_proj(obs, pr, deep);
    }
    return obs.size();
}

int Xref::find_all_syms(ddSelector & ds, symbolSet & syms)          // static
//find all xrefSymbols from all pmods that match the selector
//  If pmod scopes overlap, behavior is currently undefined
//  Return value is the number of new symbols added to the list
{
    Initialize(find_all_syms_in);
    int orig_size = syms.size();
    projNode *pr;
    objSet ps;
    // get list of all projects and subprojects, refreshing as needed,
    //    regardless of which ones have pmods
    ForEachProj(i,pr){
        //  for (int i = 0; (pr=projList::search_list->get_proj(i)) ; ++i) {
        if (ParaCancel::is_cancelled()) {
            return -1;	// flag cancellation
        }
        ps.insert(pr);
        get_all_subprojects_from_proj(ps, pr, 1);
    }
    symbolArr arr;
    Obj* ob;
    ForEach(ob, ps) {
        if (ParaCancel::is_cancelled()) {
            return -1;	// flag cancellation
        }
        pr = checked_cast(projNode, ob);
        Xref * xr = pr->get_xref(1);
        if (xr == 0) continue;
        if (xr->get_lxref())
            xr->get_lxref()->add_symbols(arr, ds);
    }
    syms.insert(arr);
    return syms.size() - orig_size;
}

void Xref::add_symbols(symbolArr& arr, const ddSelector& selector)
{
    Initialize(Xref::add_symbols);
    get_lxref ()->add_symbols(arr, selector);
}


int Xref::need_to_update(char const *ln, char const *fn)
{
    Initialize(Xref::need_to_update);
    int ret_val = 0;
    XrefTable* xr = get_lxref();
    if (!xr) return 0;

    xrefSymbol* sym = xr->lookup_module (ln);
  
    if (sym && sym->is_newer(fn, xr)) {
        ret_val = 1;
    }
    else if (sym == 0)
        ret_val = 1;
    return ret_val;
}


void Xref::merge (Xref *dst_xref)
{
    Initialize (Xref::merge);

    if (!dst_xref || !get_lxref () || !dst_xref->get_lxref ()) return;
    
    XrefTable *src = lxref;
    XrefTable *dst = dst_xref->lxref;
    src->merge_xref (dst);
}

bool Xref::is_unreal_file()
{
    Initialize(Xref::is_unreal_file);
    return unreal_flag;
}

void Xref::to_write()
{
    Initialize(Xref::to_writable);

    if (get_lxref (1) == 0) {
        init(!READ_ONLY);
        return;
    }
    if (perm_flag != READ_ONLY) {
	make_room(250000);
	return;
    }
    if (lxref->to_write(xref_name))
	perm_flag = !READ_ONLY;
}

// public version of following function
fsymbolPtr Xref::lookup_module(char const *file_name)
{
    Initialize(Xref::lookup_module);
    xrefSymbol* foo = lookup_module_priv(file_name);
    return symbolPtr(foo);
}
xrefSymbol* Xref::lookup_module_priv(char const *file_name)
{
    Initialize(Xref::lookup_module_priv);
    if (get_lxref ())
        return lxref->lookup_module(file_name);
    return 0;
}

void Xref::remove_module_from_xref(char const *filename)
{
    Initialize(Xref::delete_module_from_xref);
    
    if (my_proj == 0) {
	IF (SharedXref::get_cur_shared_xref())
	    return;

	if (filename && get_lxref() ) {
	    XrefTable *xrl = lxref;
	    XrefTable *To = xrl->get_backup ();
	    SharedXref SXr (To);
	    if (To && lookup_module_priv(filename)) 
		To->remove_module(filename);
	    if (xrl->backup) {
		delete xrl->backup;
		xrl->backup=0;
	    }
	}
    } else if (get_perm_flag() != READ_ONLY) {
	// register for err rec. save
	register_file_for_crash_recovery(filename);
    }
}

char Xref::get_perm_flag()
{
    return perm_flag;
}

time_t get_last_modified_date(char const *filename)
{
    if (!filename || *filename == '\0') return 0;
    struct OStype_stat sbuf;
    if (is_model_build()) {
        if (fileCache_stat(filename,&sbuf))
            return 0;
    }
    else if (OSapi_stat(filename,&sbuf))
        return 0;
    return  sbuf.st_mtime;
}

int Xref_get_links_global(xrefSymbol* sym, linkTypes & la, int nlinks,
			  symbolArr& as, xrefSymbol* dfs)
{
    Initialize(Xref::get_link_global);
    //  this should only be called with symbol-type linktypes
    //    int as_size = as.size();     // remember initial size of result array
    // first find the control project, so we can handle it specially inside loop

    Xref* oxr = sym->get_Xref();

    objSet proj_list;
    objSet proj_list2;          // this one alternates with proj_list
    objSet * from;
    objSet * next;
    
    objSet projs_searched;       // this accumulates the already-searched projects
    //    linkType lt = la;

#ifdef _HIER_HOME
    proj_list.insert(projNode::get_home_proj());
    next = &proj_list2;
    from = &proj_list;

    //  search all the subprojects of home project
    while (from->size()) {
	
	next->remove_all();
	if (get_links_global_one_layer_internal(sym, la, nlinks, as, dfs, *from, projs_searched, *next, oxr))
            return as.size();

	objSet* temp;
	temp=from;from=next;next=temp;    // swap from & next
    }
    proj_list2.remove_all();
    proj_list.remove_all();
    // note that the following will process the home project again, but no biggie, since
    //    is is already in  projs_searched
#endif

    get_projList(proj_list);       // get all the root projects
    // now, for each project & subproject, check if there is an xref, and if
    //   so, and the xref is different than the original symbol, lookup all the
    //   corresponding links there
    next = &proj_list2;
    from = &proj_list;

    while (from->size()) {

	next->remove_all();
	if (get_links_global_one_layer_internal(sym, la, nlinks, as, dfs, *from, projs_searched, *next, oxr))
            return as.size();
        //	if (local_only(lt, as.size()-as_size)) return as.size();

	objSet* temp;
	temp=from;from=next;next=temp;    // swap from & next
    }

    return as.size();
}

int get_links_global_one_layer_internal(xrefSymbol* sym, linkTypes &la, int nlinks, symbolArr& as, xrefSymbol* dfs, objSet& from, objSet& already, objSet& next, Xref* oxr)
{
    Initialize(get_links_global_one_layer_internal);
    projNode* cpr = projNode::get_control_project();
    XrefTable* cxr = cpr->get_xref()->get_lxref();
    int from_cxr = (sym->belonged_to(cxr) != 0);
    
    Obj* ob;
    ForEach(ob, from) {
	projNode* pr = checked_cast(projNode, ob);
	if (already.includes(pr)) continue;
	already.insert(pr);
	Xref* Xr = pr->get_xref(1);
	XrefTable * xr = NULL;
	if (Xr && Xr != oxr && (xr = Xr->get_lxref()))
	    if (xr->collect_symbols(sym,la,nlinks,as,dfs,from_cxr,oxr->get_lxref()) == 1)
		return 1;
    }
    //  now find the children of each of these projects, to search next time around the loop
    Obj* ob2;
    ForEach(ob2, from) {
	projNode* pr = checked_cast(projNode, ob2);
	if (pr==cpr) continue;     // skip the control project
	Xref * Xr = pr->get_xref();
        // only go down if there is no real xref from here to root
	int go_down = (Xr == 0 || Xr->is_unreal_file());

        if (go_down && pr->contain_sub_proj())
            {
                //  note:  refresh_projects() is expensive first time through a particular level of a project
                pr->refresh_projects();  // expand one level for this project
                Obj* children = parentProject_get_childProjects(pr);
                Obj* child;
                ForEach(child, *children) {
                    next.insert(child);
                }
            }
    }
    return 0;
}

objTree *Xref::get_root() const
{
    Initialize(Xref::get_root);
    objTree *rt = NULL;
    projNode *pn = get_projNode();
    if (pn) {
	scopeUnit *sunit = scope_mgr.unit (pn);
	rt = sunit;
    }
    return rt;
}
