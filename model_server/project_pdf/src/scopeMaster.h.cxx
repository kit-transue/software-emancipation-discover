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
#include <scopeMaster.h>

init_relational(scopeSLL,scopeRoot);
init_relational(scopeEXE,scopeRoot);
init_relational(scopeDLL,scopeEXE);
init_rel_or_ptr(scopeEXE,exec,0,scopeDLL,dll,0);

extern const char* readable_name_of_ddKind(ddKind k);

scopeSLL::scopeSLL (const char *right_name)
{
    Initialize(scopeSLL::scopeSLL);
    internal_create (right_name, SCOPE_SLL);
}

scopeDLL::scopeDLL (const char *right_name) : exported_syms(2)
{
    Initialize(scopeDLL::scopeDLL);
    internal_create (right_name, SCOPE_DLL);
}

scopeEXE::scopeEXE (const char *right_name)
{
    Initialize(scopeEXE::scopeEXE);
    internal_create (right_name, SCOPE_EXE);
}

boolean scopeEXE::add_import (scopeDLL *dll, ostream& os)
{
    Initialize(scopeEXE::add_import);
    boolean res = false;
    if (!dll)
	os << "Error: dll \"(null)\" can not be imported. Ignored." << endl;
    else if (import_dlls.includes(dll))
	os << "Error: scope \"" << (char *)lname << "\" already imports dll \"" << dll->get_name () << "\". Ignored." << endl;
    else {
	import_dlls.insert_last(dll);
	exec_put_dll(this,dll);
	res = true;
    }

    return res;
}

void scopeEXE::scopes_imported (symbolArr& sarr) const
{
    Initialize(scopeEXE::scopes_imported);
    Obj *cur;
    ForEach(cur,import_dlls) {
	Relational *dll = (Relational *)cur;
	sarr.insert_last(dll);
    }
}

//returns size of array. C++ cnames ONLY
static int lookup_cname (scopeRoot *sroot, char const *cname, symbolArr& result)
{
    Initialize(lookup_cname);
    if (!sroot || !cname || !cname[0])
	return 0;

    int len = strlen(cname);
    symbolScope sym_scope;
    objArr domain;
    domain.insert_last(sroot);
    sym_scope.set_domain (domain);

    ddSelector ddsel;
    ddsel.add(DD_FUNC_DECL);
    symbolArr sarr;
    sym_scope.query(sarr,ddsel);
    int sz = sarr.size();
    for (int ii = 0 ; ii < sz ; ii++ ) {
	symbolPtr& cur = sarr[ii];
	char const *cur_cname = cur.get_name();
	int cur_len = strlen(cur_cname);
	if (cur_len <= len || cur_cname[len] != '(')
	    continue;

	if (strncmp(cname,cur_cname,len) == 0)
	    result.insert_last(cur);
    }
    int ret = result.size();
    return ret;
}

boolean scopeDLL::add_export (ostream& os, ddKind knd, char const *symbol_name, xrefSymbol *dfs)
{
    Initialize(scopeDLL::add_export__symbolPtr&);
    boolean ret = false;
    if (!symbol_name || !symbol_name[0])
	return ret;

    const char *knd_name = readable_name_of_ddKind (knd);
    symbolPtr sym = get_def_symbol(knd, symbol_name, dfs);
    if (!sym.is_xrefSymbol()) {
	symbolArr cnames;
	int sz = lookup_cname (this, symbol_name, cnames);
	if (sz > 1) {
	    os << "Warning: There are " << sz << " C++ functions for cname \"" << symbol_name 
	       << "\" in the DLL \"" << (char *)lname << "\". Use first" << endl;
	} else if (sz == 1) {
	    os << "Warning: Found C++ function for cname \"" << symbol_name 
	       << "\" in the DLL \"" << (char *)lname << "\"." << endl;
	}
	if (sz > 0)
	    sym = cnames[0];
    }
    if (!sym.is_xrefSymbol()) {
	os << "Warning: symbol \"" << symbol_name << "\" is not defined in DLL \"" << (char *)lname
	   << "\". Failed to export." << endl;
    } else {
	if (exported_syms.includes(sym)) {
	    os << "Error: DLL \"" << (char *)lname << " already exports symbol \""
	       << knd_name << " " << symbol_name << ". Ignored" << endl; 
	} else {
	    exported_syms.insert (sym);
	    ret = true;
	}
    }
    return ret;
}

boolean scopeDLL::add_export (symbolPtr& sym, ostream& os)
{
    Initialize(scopeDLL::add_export__symbolPtr&);
    boolean ret = false;
    if (!sym.is_xrefSymbol())
	return ret;

    ddKind k       = sym.get_kind();
    char const *sym_name = sym.get_name();
    ret = add_export (os, k, sym_name);
    return ret;
}

void scopeDLL::symbols_exported (symbolArr& sarr) const
{
    Initialize(scopeDLL::symbols_exported);
    scopeDLL * that = (scopeDLL*)this;
    symbolPtr sym;
    ForEachT(sym,that->exported_syms)
	sarr.insert_last(sym);
}

