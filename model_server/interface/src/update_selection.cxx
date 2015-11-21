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
#ifdef NEW_UI

// update_selection for NEW_UI ONLY!

#include <symbolArr.h>
#include <symbolPtr.h>
#include <proj.h>
#include <messages.h>
#include <merge_dialog.h>

typedef int (projModule::*int_method_const_char) (const char*);
typedef int (projModule::*int_method_projNode) (projNode *);
typedef int (projModule::*int_method) ();
typedef projModule* (*projModuleptr_func) (symbolPtr&);
typedef projModule* (projModule::*projModuleptr_method) ();
typedef appPtr (projModule::*appPtr_method) ();

extern projModule * projModule_of_symbol(symbolPtr &);

int els_reparse_modules(symbolArr& modules, symbolArr& rest);

inline int valid_selection(const symbolArr &array)
{
    int ret_val = (array.size() > 0);
    if(!ret_val) {
        dis_message(C_BROWSER_EMPTY, MSG_ERROR, "M_NO_MODULES_SELECTED");
    }
 
    return ret_val;
}


//
// Invoke a projModule member-function on the projModules in array.
// func returns int.
//

static void operate_on_modules(symbolArr& array, int_method func) {

    Initialize(operate_on_modules);

    if(!valid_selection(array))		// if nothing is selected, let user know
	return;

    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;

        if(sym.is_xrefSymbol())
	    module = projModule_of_symbol(sym);
	else if(is_projModule(sym))
	    module = projModulePtr(RelationalPtr(sym));

        if(module)
	    (module->*func)();
    }
}


//
// Invoke a projModule member-function on the projModules in array.
// func takes a string and returns int.
//

static void operate_on_modules(	symbolArr& array, 
				int_method_const_char func, 
				const char* message) {

    Initialize (operate_on_modules);

    if(!valid_selection(array))		// if nothing is selected, let user knwo
	return;

    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;

        if(sym.is_xrefSymbol())
	    module = projModule_of_symbol(sym);
	else if(is_projModule(sym))
	    module = projModulePtr(RelationalPtr(sym));

        if(module)
	    (module->*func)(message);
    }
}


//
// Invoke a projModule member-function on the projModules in array.
// func returns appPtr.
//

static void operate_on_modules(symbolArr& array, appPtr_method func)
{
    Initialize(operate_on_modules);
 
    if(!valid_selection(array))
        return;
 
    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;
 
        if(sym.is_xrefSymbol())
            module = projModule_of_symbol(sym);
        else if(is_projModule(sym))
            module = projModulePtr(RelationalPtr(sym));
 
        if(module)
            (module->*func)();
    }
}


static void operate_on_modules(symbolArr& array, projModuleptr_func func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns projModule*.
//
{
  Initialize(operate_on_modules);
 
  if(!valid_selection(array))           // if nothing is selected, let user know
    return;
 
  symbolPtr sym;
 
  ForEachS(sym, array) {
    (*func)(sym);
  }
}

static void operate_on_modules(symbolArr& array, projModuleptr_method func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns projModule*.
//
{
    Initialize(operate_on_modules);
 
    if(!valid_selection(array))         // if nothing is selected, let user know
        return;
 
    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;
 
        if(sym.is_xrefSymbol())
            module = projModule_of_symbol(sym);
        else if(is_projModule(sym))
            module = projModulePtr(RelationalPtr(sym));
 
        if(module)
            (module->*func)();
    }
}

static void convert_to_projModules(symbolArr& arr)
//  convert all symbols to projModule, that can be.  Leave the others
//     alone.
{
    Initialize(convert_to_projModules);
    symbolArr local;
    symbolPtr sym;
    ForEachS(sym, arr) {
        local.insert_last(sym);
    }
    arr.removeAll();
    symbolPtr sym2;
    projModulePtr pr;
    ForEachS(sym2, local) {
        if (sym2.relationalp() && !is_projModule(sym2)) {
            pr = projModule_of_symbol(sym2);
            if (pr)
                arr.insert_last(pr);
        }else
            arr.insert_last(sym2);
    }
}

void update_selection(symbolArr& mods)
{
    Initialize(update_selection);
 
    convert_to_projModules(mods);
    
    symbolArr* modules = &mods, rest;

    els_reparse_modules(mods, rest);
}

void preload_selection(symbolArr& mods)
{
    Initialize(preload_selection);
    
    convert_to_projModules(mods);
	operate_on_modules(mods, &projModule::load_module);
}

static void really_unload(void* data)
{
    Initialize(really_unload);

    symbolArr& array = *(symbolArr*)data;
    if (array.size()) operate_on_modules (array, &projModule::unload_module);
}


static symbolArr* modules_to_unload;

void unload_selection (symbolArr& modules)
{
    Initialize(unload_selection);

    objArr app_headers;
    delete modules_to_unload;
    modules_to_unload = NULL;
    symbolPtr el;
    ForEachS(el, modules)
    {
	// For each element of symbols, attempt to find a projModule and
	// assign it to mod (below).
	
	projModulePtr mod = NULL;
	
	if(el.is_xrefSymbol())
	{
	    if(el.get_kind() == DD_MODULE  &&  el.is_loaded())
		    mod = projModule_of_symbol(el);
	}
	else if(is_projModule(RelationalPtr(el)))
	{
	    mod = projModulePtr(RelationalPtr(el));
	}
	if (!mod)
	    mod = projModule_of_symbol(el);
	
	if(mod)
	{
	    appPtr ah = mod->find_app();
	    if(modules_to_unload == NULL)
	   	modules_to_unload = new symbolArr;
		modules_to_unload->insert_last(mod);
		
		if(ah)
		    app_headers.insert_last(ah);
	}
    }
    if(modules_to_unload  &&  modules_to_unload->size())
    {
	// offer_to_save_list copies its third argument (app_headers).
	
	// offer_to_save_list(
	    // NULL, Flavor_Unloading, &app_headers, really_unload,
	    // modules_to_unload);
    }
    else
    {
	dis_message(C_BROWSER_ERRORSUNLOAD, MSG_ERROR, "M_NO_SEL_UNLOAD");
    }
}
#endif
