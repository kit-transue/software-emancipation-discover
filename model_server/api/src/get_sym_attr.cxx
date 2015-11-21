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
#include <msg.h>
#include <ldrList.h>
#include <customize.h>
#include <proj.h>
#include <smt.h>
#include <fileCache.h>
#include <genWild.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//#include <steDocument.h>
#include <instanceBrowser.h>
#include <messages.h>
#include <attribute.h>
#include <format.h>
#include "get_sym_attr.h"
#ifndef _groupHdr_h
#include <groupHdr.h>
#endif
#include <machdep.h>
#include <objOperate.h>
#include <attribute.h>
#include <format.h>
#include <metric.h>
#include <attribute.h>
#include <elsLanguage.h>
#include <XrefTable.h>
#include <aseticons.h>

char* astNode_get_src_info(symbolPtr& msym);
int ATT_line(symbolPtr& sym);

static unsigned char file_language_icon (fileLanguage lang)
{
  switch (lang) {
     case FILE_LANGUAGE_RAW:      return PIX_DOC_UNKNOWN;
     case FILE_LANGUAGE_UNKNOWN:  return PIX_DOC_UNKNOWN;
     case FILE_LANGUAGE_CPP:      return PIX_DOC_CPP;
     case FILE_LANGUAGE_C:        return PIX_DOC_C;
     case FILE_LANGUAGE_EXT:      return PIX_DOC_FRAME;
     case FILE_LANGUAGE_MAKEFILE: return PIX_DOC_MAKEFILE;
     case FILE_LANGUAGE_ESQL_C:   return PIX_SQL_C;
     case FILE_LANGUAGE_ESQL_CPP: return PIX_SQL_CPLUSPLUS;
     case FILE_LANGUAGE_ELS:      return PIX_DOC_UNKNOWN;
     default:                     return PIX_XUNKNOWN;
    }
}

static unsigned char file_ELS_language_icon(const symbolPtr &xe)
{
    fileLanguage lang = (fileLanguage)xe->get_language(xe.get_xref()->get_lxref());
    unsigned char icon = file_language_icon(lang);
    if (lang == FILE_LANGUAGE_ELS) {
        const elsLanguage *els_lang =
		elsLanguage::get_els_language((symbolPtr&)xe);
        if (els_lang != NULL) {
            unsigned char els_icon = els_lang->fileIconCode;
            if (els_icon != 0 && els_icon == els_lang->fileIconCode) {
    	    	icon = els_icon;
	    }
        }
    }
    return icon;
}

static unsigned char projModule_ELS_language_icon(projModulePtr &module)
{
    fileLanguage lang = (fileLanguage)module->language();
    unsigned char icon = file_language_icon(lang);
    if (lang == FILE_LANGUAGE_ELS) {
        symbolPtr sym = module;
        symbolPtr xsym = sym.get_xrefSymbol();
        if (xsym.isnotnull()) {
            icon = file_ELS_language_icon(xsym);
	}
    }
    return icon;
}
	    
static unsigned char app_icon_for_prefix (appPtr app_head)
{
   Initialize(app_icon_for_prefix);

    if (!app_head)
	return PIX_DOC_UNKNOWN;

    switch (app_head->get_type()) {
      case App_SMT:
        { 
	projModule* proj = app_head->get_module();
	if (proj) 
	    return projModule_ELS_language_icon (proj);
	else {
	    return file_language_icon ((fileLanguage)app_head->language);
	}
        }
        break;
	
      case App_STE:         return PIX_DOC_STD;
      case App_RTL:         return PIX_RTL;
      case App_UFILE:       return PIX_DOC_UNKNOWN;
      default:              return PIX_DOC_UNKNOWN;
    }
}

unsigned char dd_icon (ddKind kind);

const unsigned char* get_prefix (symbolPtr sym)
{
    Initialize(appTree_icons);
    
    static unsigned char prefix[16];
    
    prefix[0] = prefix[1] = prefix[2] = 0;

#ifndef _WIN32
    extern int is_aseticons_available();
    if (!is_aseticons_available()) return prefix;
#endif

    ddKind k = sym.get_kind();
    switch (k)
    {
      case DD_EXTGROUP:     prefix[0] = PIX_GROUP; break;
      case DD_EXTGROUP_TMP: prefix[0] = PIX_SCRATCHGROUP; break;
      default:              break;
    }

    if (k == DD_BUGS) {
      prefix[0] = dd_icon(DD_BUGS);
      return prefix;
    }
    if (sym.is_instance()) {
      prefix[0] = dd_icon(DD_INSTANCE);
      return prefix;
    }
    if (sym.is_ast() || sym.is_dataCell()) {
      prefix[0] = dd_icon(DD_INSTANCE);
      return prefix;
    }
    
    if (!(sym.is_xrefSymbol() || sym.relationalp() && (is_appTree(sym) || is_RTL(sym))))
	return prefix;
    
    if (sym.is_xrefSymbol()) {
	fsymbolPtr  xe = (symbolPtr )sym.get_xrefSymbol();
        int writable = 0;
        
        ddKind kind = xe.get_kind();

	if(kind == DD_MODULE) {
            prefix[0] = file_ELS_language_icon (xe);
	    prefix[0]++;      // in scan mode, all file icons are dark

	    if (customize::rtl_file_stats()) {
                char const *ln  = sym.get_name();
                writable = projHeader::is_writable(ln);
                prefix[1] = PIX_MODULE;
                if (writable) prefix[1] += 2;
                prefix[1] += 4;
	    } else {
                prefix[1] = PIX_MODULE;
                prefix[1] += 8;
            }
	} else {
	    prefix[0] = dd_icon(kind);
        }
    } else if (sym.relationalp()) {
	RelationalPtr obj = RelationalPtr(sym);
	if (obj && is_app(obj)) {
	    prefix[0] = app_icon_for_prefix (appPtr(obj));
	} else if (obj && is_appTree(obj)) {
	    appTreePtr app_node = appTreePtr(obj);

	    if(is_projModule(app_node))
	    {
		appPtr app_ptr;
		
		projModulePtr module = projModulePtr(app_node);
		app_ptr = module->find_app();
		
		prefix[0] = projModule_ELS_language_icon (module);
		if(app_ptr) prefix[0]++;
		
		genString pn;
		module->get_phys_filename(pn);

	        if (customize::rtl_file_stats()) {
		    prefix[1] = PIX_MODULE;
		    if (global_cache.access (pn, W_OK) == 0) 
		        prefix[1] += 2;
		    if (app_ptr && app_ptr->is_src_modified())
		        prefix[1] += 4;
		    if (app_ptr && (app_ptr->is_imported() || 
		        app_ptr->is_modified()))
		        prefix[1] += 1;
	        } else {
		    prefix[1] = PIX_MODULE;
                    prefix[1] += 8;
		    if (app_ptr && app_ptr->is_src_modified())
		        prefix[1] += 2;
		    if (app_ptr && (app_ptr->is_imported() || 
		        app_ptr->is_modified()))
		        prefix[1] += 1;

		}
	

		return prefix;
	    }
	    else if(is_projNode(app_node))   
	    {
		projNodePtr proj = projNodePtr(app_node);
		
		if(proj == projNode::get_current_proj())
		    prefix[0] = PIX_FOLDER;
		else
		    prefix[0] = PIX_FOLDER + 1;

		return prefix;
	    }
	    else if(is_ddElement(app_node) || is_smtTree(app_node))
	    {
		ddElementPtr dd = is_ddElement(app_node)?ddElementPtr(app_node):
		smt_get_dd(checked_cast(smtTree, app_node));
		if (dd == 0) {
		    prefix[0] = app_icon_for_prefix(appPtr(appTreePtr(app_node)->get_header()));

		    return prefix;
		}
		prefix[0] = dd_icon(dd->get_kind());
		
		if(dd->get_kind() == DD_PROJECT)
                // This newer code handles home projects that are not at root
		// level, and shows the open icon for all subprojects of such
		{
		    prefix[0] = PIX_FOLDER+1;

		    ddElementPtr dd2 = dd->get_definition();
		    projNode * thisproj = checked_cast(projNode,
						       get_relation(def_assocnode_of_dd, dd2));
		    
		    if(thisproj){
			projNode *temp = thisproj;
			if ( thisproj == projNode::get_current_proj()) {
			    prefix[0]--;
			    return prefix;
			} else while (temp = checked_cast(projNode,
							get_relation(parentProject_of_childProject, temp))) {
			    if ( temp == projNode::get_current_proj()) {
				prefix[0]--;
				return prefix;
			    }
			}
		    }
		    return prefix;
		} else
		    return prefix;
	    } else if(is_InstanceRep(app_node)){
		InstanceRep *rep = (InstanceRep *)app_node;
		prefix[0] = rep->get_icon();
	    } else {
		prefix[0] = app_icon_for_prefix(appPtr(appTreePtr(app_node)->get_header()));
	    }
	} else
	    prefix[0] = PIX_XUNKNOWN;
    }
    
    return prefix;
}

void get_sym_name_for_symbol(symbolPtr sym,
			     const unsigned char *&sym_pref,
			     const char *&sym_name)
{
    Initialize(get_sym_name_for_symbol);
 
    if (sym.relationalp()) {
	Relational *obj = sym;

        if(is_InstanceRep(obj)){
            InstanceRep *rep = (InstanceRep *)obj;
            if(!rep->use_own_icon() || !rep->use_own_name()){
                symbolPtr s = rep->get_associated_symbol();
                get_sym_name_for_symbol(s, sym_pref, sym_name);
            }
            if(rep->use_own_icon()){
                sym_pref = get_prefix(sym);
            }
            if(rep->use_own_name()){
                sym_name = rep->get_name();
            }
        } else{
            sym_name = obj->get_name();
            sym_pref = get_prefix(sym);
        }

    } else {
	// get the icons, which are just funny characters that display in front of the string
	sym_pref       = get_prefix(sym);
	if (sym.is_instance()) {
            sym_name       = Metric::get_instance_info(sym);
        } else if (sym.is_ast() || sym.is_dataCell()) {
	    sym_name = astNode_get_src_info(sym);
	} else {
            symbolPtr xref = sym.get_xrefSymbol();
            sym_name       = xref.get_name();
	}
    }
    // steDocument is a special case, where the logical filename is stored in get_filename()
    //     In this case, get_name() just returns the last node of the path string
    // Possibly all apps should use get_filename(), but we cannot afford to take the chance of changing anything for which the impact cannot 
    // be analyzed
}

// In some circumstance, someone may set current_name (from ldr/src) so
// that they get the result they want from sym_get_symname()

char * current_name = 0;

const char * sym_get_symname(symbolPtr & sym) {
    Initialize(sym_get_symname);

    if (current_name)
      return current_name;
    else {
	const unsigned char * pref;
	const char * name;
	get_sym_name_for_symbol(sym, pref, name);
	
	return (char*)name;
    }
}

const char * sym_get_form_str(symbolPtr & sym) {
    Initialize(sym_get_formstr);

    return "";
}

int sym_get_line(symbolPtr & sym) {
    Initialize(sym_line);
    
    int sym_line = ATT_line(sym);
    return sym_line;
}

const char * ATT_filename( symbolPtr& sym);
const char * sym_get_filename(symbolPtr & sym) {
    Initialize(sym_get_filename);

    const char * sym_filename;
    sym_filename = ATT_filename(sym);
    if(sym_filename == NULL || sym_filename[0] == '\0')
	sym_filename = "<noname>";
    return sym_filename;
}
   
const char * sym_get_extension(symbolPtr & sym) {
    Initialize(sym_get_extension);

    const char * name = sym_get_filename(sym);
    const char *ptr;
    if (ptr = strchr (name, '.'))
      return ptr;
    else
      return name;
}


static iconType ddKind_icon(ddKind kind)
//
// Convert ddKind into iconType.
//
{
    switch(kind)
    {
      case DD_TYPEDEF:   return icon_XREF_TYPEDEF;
      case DD_SEMTYPE:   return icon_XREF_SEMTYPE;
      case DD_MACRO:     return icon_XREF_MACRO;
      case DD_FIELD:     return icon_XREF_FIELD;
      case DD_VAR_DECL:  return icon_XREF_VAR;
      case DD_FUNC_DECL: return icon_XREF_FUNC;
      case DD_ENUM:      return icon_XREF_ENUM;
      case DD_ENUM_VAL:  return icon_XREF_ENUM_VAL;
      case DD_CLASS:     return icon_XREF_CLASS;
      case DD_UNION:     return icon_XREF_UNION;
      case DD_SUBSYSTEM: return icon_XREF_SUBSYSTEM;
      case DD_MODULE:    return icon_XREF_UNKNOWN;
      case DD_PROJECT:   return icon_XREF_PROJECT;
    }
    return icon_XREF_DD;
}


static iconType fileLanguage_icon(fileLanguage lang)
//
// Convert fileLanguage into document-style iconType.
//
{
    switch(lang)
    {
      case FILE_LANGUAGE_C:        return icon_XREF_C;
      case FILE_LANGUAGE_CPP:      return icon_XREF_CPP;
      case FILE_LANGUAGE_RAW:      return icon_XREF_UNKNOWN;
      case FILE_LANGUAGE_EXT:	   return icon_XREF_XDOC;
      case FILE_LANGUAGE_MAKEFILE: return icon_XREF_MAKEFILE;
      case FILE_LANGUAGE_UNKNOWN:  return icon_XREF_UNKNOWN;

      case FILE_LANGUAGE_EXT_I:  //  Peter:  These are not used, but causing warning.
      case FILE_LANGUAGE_EXT_X:  //          Same here.
      case FILE_LANGUAGE_FORTRAN:
      case FILE_LANGUAGE_COBOL:
      case FILE_LANGUAGE_LAST:     return icon_XREF_DD;
      case FILE_LANGUAGE_ESQL_C:   return icon_XREF_ESQL_C;
      case FILE_LANGUAGE_ESQL_CPP: return icon_XREF_ESQL_CPP;
      case FILE_LANGUAGE_ELS:      return icon_XREF_ESQL_C;
    }
    return icon_XREF_DD;
}


static iconType app_icon_for_view(appPtr app_head)
//
// Convert app_head->get_type() into iconType.
//
{
    if(app_head)
    {
	switch(app_head->get_type())
	{
	  case App_SMT:		{
				projModule* proj = app_head->get_module();
	                        if (proj)
				  return fileLanguage_icon((fileLanguage) 
							   proj->language());
				return fileLanguage_icon((fileLanguage) 
							 app_head->language);
				}
	  case App_STE:         return iconType(icon_XREF_STE + 1);
	  case App_RTL:         return icon_RTL;
	  case App_SUBSYS:      return iconType(icon_XREF_SUBSYSTEM + 1);
	  case App_SCRAPBOOK:   return icon_SCRAPBOOK;
	  case App_UFILE:       return icon_UFILE;
	  case App_DGRAPH:	return icon_RTL;
	}
    }
    return icon_XREF_UNKNOWN;
}


static void appTree_icons(appTreePtr app_node, char* array)
//
// Given an appTreePtr, store an appropriate icon string in icons.
//
{    
    Initialize(appTree_icons);

    if(is_projModule(app_node))
    {
        appPtr app_ptr;

	projModulePtr module = projModulePtr(app_node);
        app_ptr = module->find_app();

	*array = fileLanguage_icon(module->language());
	if(app_ptr) *array += 1;

	int flags = icon_MODULE_FLAGS;
	genString pn;
	module->get_phys_filename(pn);
	flags += ICON_LOADED_F;
	if (global_cache.access (pn, W_OK) == 0)  flags += ICON_WRITABLE_F;
	if (app_ptr && app_ptr->is_src_modified()) flags += ICON_MODIFIED_F;
	if (app_ptr && (app_ptr->is_imported() || app_ptr->is_modified()))
	    flags += ICON_IMPORTED_F;
	array[1] = flags;
	

    }
    else if(is_projNode(app_node))   
    {
	projNodePtr proj = projNodePtr(app_node);

	if(proj == projNode::get_current_proj())
	    *array = icon_WRITABLE_PROJECT;
	else
	    *array = icon_XREF_PROJECT + 1;
    }
    else if(is_ddElement(app_node) || is_smtTree(app_node))
    {
	ddElementPtr dd = is_ddElement(app_node)?ddElementPtr(app_node):
	smt_get_dd(checked_cast(smtTree, app_node));
	if (dd == 0) {
	    *array = app_icon_for_view(appPtr(appTreePtr(app_node)->get_header()));
	    return;
	}
        *array = ddKind_icon(dd->get_kind());
	if(dd->get_kind() == DD_PROJECT)
	{
	    ddElementPtr dd2 = dd->get_definition();
	    projNode * thisproj = checked_cast(projNode,
		     get_relation(def_assocnode_of_dd, dd2));
  
	    if(thisproj){
		projNode *temp = thisproj;
		if ( thisproj == projNode::get_current_proj())
                      *array = icon_WRITABLE_PROJECT;
		else while (temp = checked_cast(projNode,
			     get_relation(parentProject_of_childProject, temp))) {
		    if ( temp == projNode::get_current_proj())
                      *array = icon_WRITABLE_PROJECT;
	        }
	    }
	}
	if((*array != icon_WRITABLE_PROJECT)  &&  dd->get_loaded_definition())
	    *array += 1;
    }
    else
    {
	*array = app_icon_for_view(appPtr(appTreePtr(app_node)->get_header()));
    }
}


//-----------------------------------------
// view_icons
// Given a symbolPtr, returns the appropriate string of icons.
// Caller should zero out icons array before passing it in!
// --------------------------------------------

extern "C" void view_icons(symbolPtr symbol, char* array)
{    
    Initialize(view_icons);

    if(!array)
	return;

    *array = icon_BLANK;		// default icon is blank.

    if(symbol.is_xrefSymbol())
    {
        fsymbolPtr  xe = (symbolPtr )symbol.get_xrefSymbol();
	int writable = 0;
	if(xe->get_kind() == DD_MODULE)
	{
	    *array = fileLanguage_icon((fileLanguage) xe.get_language());
	    *array += (xe.is_loaded() != 0);
	    array[1] = icon_BLANK;
	    char const *ln  = symbol.get_name();
	    writable = projHeader::is_writable(ln);
            array[1] = icon_MODULE_FLAGS;	    if (writable)
	    if (writable) array[1] += ICON_WRITABLE_F;
	    array[1] += ICON_LOADED_F;
	}
	else
	{
	    *array = ddKind_icon(xe->get_kind());
	    *array += 1;	// Always appear "loaded"
	}
    }
    else if(symbol.relationalp())
    {
	RelationalPtr obj = RelationalPtr(symbol);

	if(is_app(obj))
	    *array = app_icon_for_view(appPtr(obj));

	else if(is_appTree(obj))
	    appTree_icons(appTreePtr(obj), array);
    }
}

