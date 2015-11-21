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
// SymManager.h.C
//------------------------------------------
// synopsis:
// Class implimenting a.out symbol table extractor/query functionality
//------------------------------------------
// Restrictions:
// Currently we don't handle constructor/destructor functions appropriately.
// Also this function reads a.out format. Other formats will eventually
// have to be read??? (COFF??) 
//
// Currently only SYM_MAX_MATCHES concurrent matches on unqualified names may 
// be found at one time
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "machdep.h"
#include <msg.h>
#include <genString.h>
#include <SymManager.h>

#include <cLibraryFunctions.h>

#include <messages.h>
#include <shell_calls.h>

/*  externs for SO */

extern "C" {

    struct sharedLibraryInfo *
    newSharedLibraryInfo(struct sharedLibraryInfo *, char *);

    int
    disposeSharedLibraryInfo(struct sharedLibraryInfo * info_item);

    struct sharedLibrary *
    disposeSharedLibrary(struct sharedLibrary *);

    //    char *
    //    ast_shared_malloc(int);

    //    void
    //ast_shared_free(void *);

    datum
    dbm_retrieve(DBM *, char *);
}

SymbolManager*          symmgr_instance;
genString SymbolManager::build_exec_path = "$PSETHOME/bin/symtab";

//
// SymbolManager::SymbolManager
//
// Constructor
//
// This class manages retrieving symbols from the symbol table, splitting names
// and retrieving a list of all matches. The executable name is passed to the
// constructor and the symbol table is read from the file.
//

SymbolManager::SymbolManager(char* executable)
{
    MangleType m_type = CFRONT_MANGLE;

    if(dbm = dbm_open (executable, O_RDONLY, 0)) 
    {
        if(!SymbolManager::linked_correctly(executable) ||
           !SymbolManager::good_pag_file(executable)) 
	{
	    dbm_close(dbm);
	    dbm = 0;
	} else {
	    datum dbm_data = dbm_retrieve(dbm, "<MANGLE_TYPE>");
	    if (dbm_data.dptr && dbm_data.dsize >= sizeof (mangle_type))
		memcpy (&m_type, dbm_data.dptr, sizeof (mangle_type));
	}
    }
    mangle_type = m_type;

    executableName(executable);
    shared_header = newSharedLibraryInfo(0, executable);
}

SymbolManager::SymbolManager(char * so_name, int so_lib_status)
{
    MangleType m_type = CFRONT_MANGLE;

    if(dbm = dbm_open (so_name, O_RDONLY, 0)) {
        if(!SymbolManager::good_pag_file(so_name)) {
	    dbm_close(dbm);
	    dbm = 0;
	} else {
	    datum dbm_data = dbm_retrieve(dbm, "<MANGLE_TYPE>");
	    if (dbm_data.dptr && dbm_data.dsize >= sizeof (mangle_type))
		memcpy (&m_type, dbm_data.dptr, sizeof (mangle_type));
	}
    }
    mangle_type = m_type;
    executableName(so_name);
    shared_header = 0;
}

SymbolManager::~SymbolManager ()
{
    if(dbm) {
	dbm_close(dbm);
	dbm = 0;
    }
    if (shared_header) {
	disposeSharedSymbols(shared_header->current_list);
	disposeSharedSymbols(shared_header->commons);
	disposeSharedLibraryInfo(shared_header);
    }
}

int SymbolManager::good_pag_file(char *executable)
{    
    DBM *local_dbm;
    int ret_val = 0;

    if (executable) {
	if(local_dbm = dbm_open (executable, O_RDONLY, 0)) {
	    genString table_name = executable;
	    table_name += ".pag";
	    int len = table_name.length();

	    if (len > 0 && len < MAXPATHLEN) {
		struct OStype_stat exec_status, table_status;
		
		int exec_status_ret = OSapi_stat(executable, &exec_status);
		int table_status_ret = OSapi_stat(table_name, &table_status);
		
		if (!(exec_status_ret || table_status_ret))
		    ret_val = (long)exec_status.st_mtime <= (long)table_status.st_mtime;
	    }
	    dbm_close(local_dbm);
	}
    }

    return ret_val;
}

// returns 1 if the executable is linked with mixlib and dl
int SymbolManager::linked_correctly(const char *executable)
{
    int found_lmix = 0;
    int found_ldl = 0;

    if(executable)
    {
	found_lmix = found_ldl = 1;
    }
    return found_lmix && found_ldl;
}

//
// SymbolManager::get_global_function
//
// This function takes a function name (qualified is optional) and retrieves
// a list of all matches
// and qualifier part
//

int SymbolManager::get_global_function (char *fnname, SymValue **values)
{
    int ret_val = 0;

    iter_handle.function_name = fnname;
    iter_handle.vptr = values;
    ret_val = traverse(&iter_handle, &SymbolManager::getGlobalFunction);

    return ret_val;
}

int SymbolManager::traverse(struct symMgrIterHandle * handle, symMgrIterMethod method)
{
    int ret_val = 0;

    if (handle && method != 0) {
	struct sharedLibraryInfo * header = sharedHeader();

	ret_val = (this->*method)(handle);

	if (!ret_val && header) {
	    // iterate through shared libraries
	    SymbolManager * so_sym_mgr;
	    struct sharedLibrary * lib = header->current_list;
	    while (lib && !ret_val) {
		so_sym_mgr = (SymbolManager *)(lib->symtab_manager);
		if (so_sym_mgr) {
		    ret_val = (so_sym_mgr->*method)(handle);
		    if (ret_val && handle->vptr && *(handle->vptr)) {
			void * sym_value = (*handle->vptr)->addr();
			(*handle->vptr)->addr((void *)((long)lib->mapped_addr+(long)sym_value));
		    }
		}
		lib = lib->next;
	    }
	    // check the list of common symbols
	    if (!ret_val) {
		if (lib = header->commons) {
		    if (so_sym_mgr = (SymbolManager *)(lib->symtab_manager))
			ret_val = (so_sym_mgr->*method)(handle);
		}
	    }
	}
    }

    return ret_val;
}

int SymbolManager::getGlobalFunction(struct symMgrIterHandle * handle)
{
    char newfn[1024];
    datum dbm_key;
    datum dbm_data;
    static SymValue matches;

    if (!dbm || !handle)
	return 0;

    char *fnname = handle->function_name;
    SymValue * * values = handle->vptr;

    newfn[0] = 'F';
    strcpy (&newfn[1], fnname);

    dbm_key.dptr = &newfn[0];
    dbm_key.dsize = strlen ((char *)dbm_key.dptr);

    dbm_data = dbm_fetch (dbm, dbm_key);
    if (!dbm_data.dptr)
	return 0;

    // get addr of symbol
    void *addr;
    memcpy (&addr, dbm_data.dptr, sizeof (void *));
    matches.addr(addr);

    // get mangled name
    if (dbm_data.dsize > sizeof (void *))
        matches.mangled_name((const char *) ((char *)dbm_data.dptr + sizeof (void *)));
    else
        matches.mangled_name(NULL);

    matches.qualifier("");
    *values = &matches;
    return 1;
}


static char *get_filename (char *flname)
{
    char *slash = strrchr (flname, '/');

    if (slash)
	slash++;
    else
	slash = flname;

    char *dot = strrchr (flname, '.');

    int len = dot ? dot - slash : strlen (slash);

    char *r_val = new char [len+1];

    r_val[len] = '\0';

    strncpy (r_val, slash, len);

    return r_val;
}

int SymbolManager::get_static_function (char *flname, char *fnname,
					SymValue **values)
{
    int ret_val = 0;

    if (!flname)
	ret_val = get_global_function (fnname, values);
    else {
	iter_handle.function_name = fnname;
	iter_handle.filename = flname;
	iter_handle.vptr = values;

	ret_val = traverse(&iter_handle, &SymbolManager::getStaticFunction);
    }
	
    return ret_val;
}

int SymbolManager::getStaticFunction(struct symMgrIterHandle * handle)
{
    char *newfn;
    datum dbm_key;
    datum dbm_data;
    static SymValue matches;

    if (!dbm || !handle)
	return 0;

    char * flname = handle->filename;
    char * fnname = handle->function_name;
    SymValue * * values = handle->vptr;

    int len = 3; // length of dbm key (initialized to 3 for "F.\0")

    char *file = get_filename (flname);

    len += strlen (fnname) + strlen (file);
    newfn = new char[len];

    newfn[0] = 'F';
    newfn[1] = '\0';
    strcat (newfn, fnname);
    strcat (newfn, ".");
    strcat (newfn, file);

    delete []file;

    dbm_key.dptr = newfn;
    dbm_key.dsize = strlen ((char *)dbm_key.dptr);

    dbm_data = dbm_fetch (dbm, dbm_key);

    if (!dbm_data.dptr)
    {
	delete []newfn;
	return 0;
    }

    void * addr;
    memcpy (&addr, dbm_data.dptr, sizeof(void *));
    matches.addr(addr);
    matches.qualifier("");
    if (dbm_data.dsize > sizeof (void *))
        matches.mangled_name((const char *) ((char *)dbm_data.dptr + sizeof (void *)));
    else
        matches.mangled_name(NULL);

    *values = &matches;

    delete []newfn;
    return 1;
}

int SymbolManager::get_global_variable (char *fnname, SymValue **values)
{
    int ret_val = 0;

    iter_handle.function_name = fnname;
    iter_handle.vptr = values;
    ret_val = traverse(&iter_handle, &SymbolManager::getGlobalVariable);

    return ret_val;
}

int SymbolManager::getGlobalVariable(struct symMgrIterHandle * handle)
{
    char newfn[512];
    datum dbm_key;
    datum dbm_data;
    static SymValue matches;
    if (!dbm || !handle)
	return 0;

    char * fnname = handle->function_name;
    SymValue * * values = handle->vptr;

    newfn[0] = 'D';
//    newfn[1] = '_';
    strcpy (&newfn[1], fnname);

    dbm_key.dptr = &newfn[0];
    dbm_key.dsize = strlen ((char *)dbm_key.dptr);
    dbm_data = dbm_fetch (dbm, dbm_key);
    if (!dbm_data.dptr)
	return 0;

    void *addr;
    memcpy (&addr, dbm_data.dptr, sizeof(void *));
    matches.addr(addr);
    matches.qualifier("");
    if (dbm_data.dsize > sizeof (void *))
        matches.mangled_name((const char *) ((char *)dbm_data.dptr + sizeof (void *)));
    else
        matches.mangled_name(NULL);

    *values = &matches;

    return 1;
}

int SymbolManager::get_static_variable (char *flname, char *fnname, SymValue **values)
{
    int ret_val = 0;

    if (!flname)
	ret_val = get_global_variable (fnname, values);
    else {
	iter_handle.function_name = fnname;
	iter_handle.filename = flname;
	iter_handle.vptr = values;

	ret_val = traverse(&iter_handle, &SymbolManager::getStaticVariable);
    }
	
    return ret_val;
}

int SymbolManager::getStaticVariable(struct symMgrIterHandle * handle)
{
    char *newfn;
    datum dbm_key;
    datum dbm_data;
    static SymValue matches;

    if (!dbm || !handle)
	return 0;

    char * flname = handle->filename;
    char * fnname = handle->function_name;
    SymValue * * values = handle->vptr;

    int len = 3; // length of dbm key (initialized to 3 for "D.\0")

    char *file = get_filename (flname);
    len += strlen (fnname) + strlen (file);
    newfn = new char[len];
 
    newfn[0] = 'D';
    newfn[1] = '\0';
    strcat (newfn, fnname);
    strcat (newfn, ".");
    strcat (newfn, file);

    delete []file;

    dbm_key.dptr = &newfn[0];
    dbm_key.dsize = strlen ((char *)dbm_key.dptr);

    dbm_data = dbm_fetch (dbm, dbm_key);
    if (!dbm_data.dptr){
	delete []newfn;
	return 0;
    }
	
    void *addr;
    memcpy (&addr, dbm_data.dptr, sizeof(void *));
    matches.addr(addr);
    matches.qualifier("");
    if (dbm_data.dsize > sizeof (void *))
        matches.mangled_name((const char *) ((char *)dbm_data.dptr + sizeof (void *)));
    else
        matches.mangled_name(NULL);

    *values = &matches;

    delete []newfn;
    return 1;
}

int SymbolManager::executableName(char * name)
{
    if (name)
	filename = name;

    return (filename.length() == 0);
}

int SymbolManager::buildExecPath(char * path)
{
    if (path) 
	build_exec_path = path;

    return (build_exec_path.length() == 0);
}

//
//
//----------------------------------------------
// SymValue class methods
//

SymValue::SymValue() : qual(0), value(0), name(0)
{
}

SymValue::~SymValue() {
  if (qual) delete []qual;
  if (name) delete []name;
}

void *SymValue::addr()
{
  return value;
}

void SymValue::addr(void *v)
{
   value = v;
}

void SymValue::mangled_name(const char *nm)
{
   if (name) delete []name;
   if (nm) {
     name = new char[strlen(nm)+1];
     strcpy (name, nm);
   } else
     name = 0;
}

const char *SymValue::mangled_name () {
   return name;
}

void SymValue::qualifier(const char *q)
{
   if (qual) delete []qual;
   if (q) {
     qual = new char[strlen(q)+1];
     strcpy (qual, q);
   } else
     qual = 0;
}

const char *SymValue::qualifier() {
   return qual;
}

/*
  -1 : error,
   0 : OK
*/
int SymbolManager::rebuildSymbolTable(char * name, char * table_type)
{
    int ret_val = -1;

    if (name) {
	genString exec_path = buildExecPath();

	if (exec_path.length() > 0) {
	    msg("INFORM: BUILDING symbol table for object ($1)") << name << eom;
	    vsysteml_redirect ("/dev/null", exec_path, table_type, name, NULL);
	    ret_val = 0;
	}
    }

    return ret_val;
}

/*
  0 : do not build symbol table for it,
  1 : build symbol table
*/
int SymbolManager::listedLibrary(char * lib_name)
{
    int ret_val = 0;

    if (lib_name) {
	char * mix_str  = "/libmix.so";
	char * gmix_str = "/libgmx.so";
	char * ptr = strstr(lib_name, mix_str);
	if(!ptr)
	    ptr = strstr(lib_name, gmix_str);
	ret_val = (ptr == 0);
    }

    return ret_val;
}

/*
  -1 : error, 
   0 : OK
*/
int SymbolManager::loadSharedSymbols(struct sharedLibrary * a_library)
{
    int table_OK = 0;
    int ret_val = 0;

    while (a_library) {
	if (listedLibrary(a_library->name)) {
	    if ((table_OK = good_pag_file(a_library->name)) == 0) {
		table_OK = rebuildSymbolTable(a_library->name);
		if (!ret_val)
		    ret_val = table_OK;
		table_OK = good_pag_file(a_library->name);
	    }
	    /* --- create SymbolManager and link it ---- */
	    if (table_OK) {
	        msg("INFORM: LOADING symbol table for object ($1)") << a_library->name << eom;
		SymbolManager * sym_mgr = new SymbolManager(a_library->name, SO_SYMBOLS);
		a_library->symtab_manager = (void *)sym_mgr;
	    } else {
		msg("INFORM: FAILED to build symbol table for object ($1)") << a_library->name << eom;
	    }
	}
	a_library = a_library->next;
    }

    return ret_val;
}

int SymbolManager::disposeSharedSymbols(struct sharedLibrary * a_library)
{
    SymbolManager * sym_mgr;
    int ret_val = 0;

    while (a_library) {
	/* ---- unlink SymbolManager list items ---- */
	sym_mgr = (SymbolManager *)(a_library->symtab_manager);
	if (sym_mgr) {
	    msg("INFORM: UNLOADING symbol table for object ($1)") << a_library->name << eom;
	    delete sym_mgr;
	    a_library->symtab_manager = (void *)0;
	}
	a_library = a_library->next;
    }

    return ret_val;
}

/*
   -1: header is zero !
    0: OK
    1: consistency problems ("mismatch" does not match)
*/
int SymbolManager::updateSharedLibraries(struct sharedLibraryInfo * header)
{
    int ret_val = -1;

    if (header) {
	int count = 0;
	int mismatch = header->list_compare_status;  // 0:empty, 1 (0th element), ...
	struct sharedLibrary * item_old = header->current_list;
	struct sharedLibrary * item_new = header->new_list;
	struct sharedLibrary * prev_old, * prev_new;

	prev_old = prev_new = (struct sharedLibrary *)0;
	while (item_old && item_new && count<(mismatch-1)) {
	    prev_old = item_old;
	    prev_new = item_new;
	    item_old = item_old->next;
	    item_new = item_new->next;
	    count++;
	}
	ret_val = (mismatch > 0) ? (count != (mismatch-1)) : (count != mismatch);
	if (!ret_val) {
	    header->disposed_list = item_old;
	    header->recache_status = (item_old != (struct sharedLibrary *)0);

	    if (prev_old) {
		prev_old->next = item_new;
		if (item_old)
		    item_old->prev = (struct sharedLibrary *)0;
	    } else
		header->current_list = item_new;

	    if (header->disposed_list)
	        disposeSharedSymbols(header->disposed_list);

	    if (item_new) {
		item_new->prev = prev_old;
		if (prev_new)
		    prev_new->next = (struct sharedLibrary *)0;
		else
		    header->new_list = (struct sharedLibrary *)0;
		loadSharedSymbols(item_new);
	    }

	    if (header->new_list)
	        header->new_list = disposeSharedLibrary(header->new_list);

	    updateCommonSymbols(header);
	}
	header->list_compare_status = -1;
    }

    return ret_val;
}

/*
  returns:
   -1: header is 0,
 >=0 : number of items in the list
*/
int SymbolManager::printSOList(genString & outp)
{
    struct sharedLibraryInfo * header = sharedHeader();
    int ret_val = -1;

    if (header) {
	genString temp;
	struct sharedLibrary * item = header->current_list;

	ret_val = 0;
	outp += "Current List of Attached Shared Objects\n\n";
	while (item) {
	    ret_val ++;
	    temp.printf("%d\t", ret_val);
	    outp += temp;
	    outp += item->name;
	    temp.printf(" (0x%lX <==> 0x%lX)\n", (long)item->mapped_addr, 
			(long)item->mapped_addr+item->text_size);
	    outp += temp;
	    item = item->next;
	}
    }

    return ret_val;
}

/*
  returns:
  -1 : error,
   0 : OK
*/
int SymbolManager::updateCommonSymbols(struct sharedLibraryInfo * header)
{
    int ret_val = -1;

    if (header) {
	ret_val = 0;
	if (header->commons) {
	    disposeSharedSymbols(header->commons);
	    if (header->commons->text_size && header->commons->name) {
		rebuildSymbolTable(header->commons->name, "-common");
		ret_val = loadSharedSymbols(header->commons);
	    }
	}
    }

    return ret_val;
}

extern "C" struct sharedLibraryInfo *
newSharedLibraryInfo(struct sharedLibraryInfo * info_item, char * executable)
{
    if (!info_item)
	info_item =  ALLOCATE(struct sharedLibraryInfo, 1);

    if (info_item) {
	char * exec_name = (char *)0;

	info_item->dynamic = (void *)0;
	info_item->current_list = 
	    info_item->disposed_list =
	    info_item->new_list = 
	    info_item->commons = (struct sharedLibrary *)0;
	info_item->list_compare_status = -1;
	info_item->recache_status = 0;

	if (executable) {
	    if (exec_name = ALLOCATE(char, strlen(executable)+1))
		strcpy(exec_name, executable);
	}   
	info_item->executable = exec_name;
    }

    return info_item;
}

extern "C" int
disposeSharedLibraryInfo(struct sharedLibraryInfo * info_item)
{
    int ret_val = 0;

    if (info_item) {
	ret_val = (info_item->new_list != 0) || 
	    (info_item->disposed_list != 0);

	disposeSharedLibrary(info_item->current_list);
	disposeSharedLibrary(info_item->disposed_list);
	disposeSharedLibrary(info_item->new_list);
	disposeSharedLibrary(info_item->commons);

	DISPOSE(info_item->executable);
	DISPOSE(info_item);
    }

    return ret_val;
}

extern "C" datum
dbm_retrieve(DBM * dbase_ptr, char * key_ptr)
{
    datum ret_val;

    ret_val.dptr = (char *)0;
    ret_val.dsize = 0;

    if (dbase_ptr && key_ptr) {
	datum dbm_key;

	dbm_key.dptr = key_ptr;
	dbm_key.dsize = strlen(key_ptr);

	ret_val = dbm_fetch(dbase_ptr, dbm_key);
    }

    return ret_val;
}

/*
$Log: SymManager.h.cxx  $
Revision 1.18 2000/07/12 18:11:19EDT ktrans 
merge from stream_message branch
Revision 1.2.1.22  1994/07/27  21:51:16  aharlap
called vpopen_sync()

Revision 1.2.1.21  1994/04/24  16:30:35  azaparov
Added support for libmix & libgmx

Revision 1.2.1.20  1994/04/13  19:41:46  builder
Port

Revision 1.2.1.19  1994/04/12  14:44:54  aharlap
*** empty log message ***

Revision 1.2.1.18  1994/03/22  14:18:45  pero
support for COMMON symbols (runtime resolve)

Revision 1.2.1.17  1994/03/21  14:38:38  kol
fix for bug #6767
 
Revision 1.2.1.16  1994/03/17  00:08:27  pero
support for shared libraries

Revision 1.2.1.15  1994/02/24  20:53:00  builder
Port

Revision 1.2.1.14  1994/02/23  01:18:31  aharlap
Port

Revision 1.2.1.13  1993/09/13  22:57:02  so
fix bug 4709

Revision 1.2.1.12  1993/07/22  16:58:23  harry
HP/Solaris Port

Revision 1.2.1.11  1993/06/26  20:53:35  aharlap
matches should be static

Revision 1.2.1.10  1993/06/08  19:35:58  so
if symbol table does not exist, tell the user when the executable is set.

Revision 1.2.1.9  1993/06/08  00:24:44  so
create a static function to check if the pag file is good for the executable.

Revision 1.2.1.8  1993/06/02  22:32:35  aharlap
added case gnu mangle

Revision 1.2.1.7  1993/04/16  21:41:43  smit
Fix mixlib problem

Revision 1.2.1.6  1993/04/15  23:54:59  smit
Fix bug#3278 3370

Revision 1.2.1.5  1993/02/23  21:16:17  glenn
Remove warning for failed dbm_open.
Repair damage from Log macro manipulations.

Revision 1.2.1.4  1992/12/15  21:26:52  smit
Added support for routing file static functions.

Revision 1.2.1.3  1992/12/10  23:13:49  smit
Fix bug# 1903

//Revision 1.10  92/08/10  11:27:58  aharlap
//implemented destructor
//
//Revision 1.9  92/08/09  21:43:02  builder
//added checking to SymbolManager::SymbolManager
//
//Revision 1.8  92/06/26  09:20:27  aharlap
//removed #include for iostream.h
//
//Revision 1.7  92/06/17  10:33:53  smit
//declare instance here.
//
//Revision 1.6  92/02/05  12:26:20  aharlap
//Using demangled names
//
//Revision 1.5  92/01/22  13:24:41  kws
//Remove message
//
//Revision 1.4  91/12/05  10:20:08  kws
//New Symbol table manager using dbm
//
//Revision 1.3  91/11/21  09:31:29  kws
//Removed redefinition errors and just printed count at end of reading symbol
//table
//
//Revision 1.2  91/11/20  20:32:30  kws
//Extend some static buffers and put "_" in front of symbols to be found
//in get_global_functions
//
//Revision 1.1  91/11/07  20:05:13  kws
//Initial revision
//
*/
