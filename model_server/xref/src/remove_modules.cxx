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
#include <xref.h>
#include <xref_priv.h>
#include <symbolSet.h>
#include <fileCache.h>
#include <systemMessages.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <ddSelector.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

// currently called only from  xref_main
extern "C"    void Remove_Old_Modules();

static void save_pset_name(char *name)
{
    Initialize(save_pset_name);

    static int error_reported = 0;
    
    char const *list_file_name = OSapi_getenv("LIST_ORPHAN_PSET");
    if(list_file_name == NULL)
	return;
    FILE *f = OSapi_fopen(list_file_name, "a");
    if(f == NULL)
	if(!error_reported){
	    msg("Can not open/create ORPHAN PSET LIST file\n") << eom;
	    error_reported = 1;
	    return;
	} else
	    return;
    fputs(name, f);
    fputs("\n", f);
    fclose(f);
}

void Remove_Old_Modules()
// Remove any modules from home project pmod that no longer have physical
//   files in the file system.
{
    Initialize(Remove_Old_Modules);

    projNode* pr = projList::search_list->writable();
    Xref* Xr = pr->get_xref();
#ifdef _PSET_DEBUG
    cout << "(Removing old modules from home)" << endl;
#endif
    
    symbolArr arr;
    ddSelector selector;
    selector.add(DD_MODULE);
    Xr->add_symbols(arr, selector);
    symbolPtr sym;
#ifdef _PSET_DEBUG
    cout << "(Found " << arr.size() << " modules)" << endl;
#endif
    
    genString logicalname;
    genString physicalname;
    bool title=0;    // flag to indicate whether we have displayed title yet
    ForEachS(sym, arr) {
	if (sym->get_kind()==DD_MODULE && sym.get_has_def_file()) {
	    sym.get_name(logicalname);
#ifdef _PSET_DEBUG
	    cerr << "(Name is " << logicalname.str() << ")" << endl;
#endif
	    pr->ln_to_fn(logicalname, physicalname);  // convert to physical
	    if (physicalname.length()) {
		// file belongs to this project
		if (fileCache_access(physicalname, R_OK)) {
		    if (!title) {
			msg("Removing modules for files that do not exist:\n") << eom;
			title = 1;
		    }
		    char *pset_name = (char *)get_paraset_file_name(physicalname, pr);
		    if(pset_name)
			save_pset_name(pset_name);
		    msg("   $1\n") << (char*)physicalname << eom;
		    Xr->remove_module(logicalname);
		}
	    } else {
		if (!title) {
		    msg("Removing modules for files that do not exist:\n") << eom;
		    title = 1;
		}
		char *pset_name = (char *)get_paraset_file_name(physicalname, pr);
		if(pset_name)
		    save_pset_name(pset_name);
		msg("Error: File $1 not in project\n") << (char*)logicalname << eom;
		Xr->remove_module(logicalname);
	    }
	}
    }
    if (title)
	msg("----------------------------------\n") << eom;
}

ddSelector::ddSelector()
{
    OSapi_bzero(this, sizeof(ddSelector));
}

ddSelector& ddSelector::operator =(const ddSelector& that)
//  simple assignment operator
{
    OSapi_bcopy(&that, this, sizeof(ddSelector));

    return *this;
}

