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
// projectQuery.C
//-----------------------------------------
// synopsis:
// XREF Query subwindow in projectBrowser
//------------------------------------------

// INCLUDE FILES

#include <messages.h>

#include <cLibraryFunctions.h>
#define _xref_h
#define _dd_forward_h
#include <_Xref.h>
#include <xrefSymbol.h>

#include <db.h>

#include <gtTopShell.h>
#include <gtPrim.h>
#include <gtForm.h>
#include <gtPushButton.h>
#include <gtRTL.h>

#include <projectBrowser.h>

#include <gtBaseXm.h>

#include <top_widgets.h>
#include <view_creation.h>

#include <stePostScript.h>
#include <path.h>

#include <xref_queries.h>
#include <viewerShell.h>
#include <steHeader.h>
#include <gtTogB.h>
#include <fileCache.h>
#include <viewNode.h>
#include <viewGraHeader.h>

#ifndef _objRelation_h
#include <objRelation.h>
#endif

#ifndef __viewGraHeader_h
#include <viewGraHeader.h>
#endif

#ifndef _gtBase_h
#include <gtBase.h>
#endif

#ifndef _gtTogB_h
#include <gtTogB.h>
#endif

#ifndef _genError_h
#include <genError.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _browserRTL_h
#include <browserRTL.h>
#endif

#ifdef TXT
#undef TXT
#endif

#define TXT(a) a

//parameters for Print and Save options
#define  TOP_MARGIN       54.0
#define  BOTTOM_MARGIN    54.0
#define  LEFT_MARGIN      54.0
#define  RIGHT_MARGIN     54.0
#define  PARAG_LEADING    3.0
#define  ENTRY_FONT       "Palatino-Roman"
#define  TEXT_SCALE       11.0
#define  TEXT_SCALE1      9.0
#define  KIND_FONT        "Courier"
#define  DEFAULT_HEADER   "Selected Objects"
#define  DEFAULT_RTL_NAME "Selected"

static int does_file_exist(char *name)
{
    int ret_val = 0;

    if (name) {
	FILE *fp = OSapi_fopen(name, "r");
	if (fp) {
	    OSapi_fclose(fp);
	    ret_val = 1;
	}
    }

    return ret_val;
}

static void get_group_directory(genString &dirname)
// return the directory name that contains the group
//
// the name will be returned by dirname
{
    dirname = (char *)0;
    
    projNode *pn = projNode::get_home_proj();
    if(pn->is_script()){
	genString name;
	   
	name.printf("%s/%s", pn->get_ln(), ".rtl/tmp_grp.rtl");
	pn->ln_to_fn_imp(name, dirname, FILE_TYPE, 0, 1);
	if(dirname.length()){
	    char *p = strrchr((char *)dirname, '/');
	    if(p)
		*p = 0;
	}
    } else {
        symbolArr sym;
	pn->search_nodes_by_name(".rtl", sym);
    
	symbolPtr node;
	if (sym.size()) // remove compilation warning
	{
	    node = sym[0];
	    pn->ln_to_fn_imp(node.get_name(), dirname, DIR_TYPE);
	}
    }
}

static int
get_group_filename(char * group_name, genString & filename)
//
//  returns filename of the specified group (if the one exists)
//
{
    int ret_val = 0;
    
    if (group_name) {
	genString dirname;

	get_group_directory(dirname);
	if (!dirname.is_null()) {
	    filename.printf("%s/%s.rtl", (char *)dirname, group_name);
	    ret_val=does_file_exist((char *)filename);
	}
    }
    if (!ret_val)
	filename = (char *)0;

    return ret_val;
}

/*static */
boolean find_action(int& action, int& ind, const char* label, actionSet actions[], int actions_size)
//
// Use a label to find out the associated action enum. -jef
//
{
    int i;
    boolean found = NO;
    
    if (label){
      for (i=0; i<actions_size; i++){
	if (strcmp(label, actions[i].label) == 0) {
	  action = actions[i].action;
	  found = YES;
	  ind = i;
	  break;
	}
      }
    }
    return found;
}

// -- some backwards compatibility
 
extern "C" void fill_selected_objects (symbolArr *selection);

extern    char* ddKind_external_name(ddKind);

// Set up the format for PostScript printing
static void ps_initial_setup(stePostScript *ps)
{
//set margins
    ps->set_lmargin(LEFT_MARGIN);
    ps->set_tmargin(TOP_MARGIN);
    ps->set_bmargin(BOTTOM_MARGIN);
    ps->set_rmargin(RIGHT_MARGIN);

    ps->set_justified(false);  //not right-justified   
    ps->do_page_header = 1;    //there will be a header 
    ps->raw_flag_value(false); 
    ps->set_interparagraph_leading(PARAG_LEADING);
    ps->new_page_flag = 1;
}

int print_symbolArr(symbolArr &selected, FILE *psfp, char *title, int physical)
// prints the contents of a symbolArr
// into the (already open) pipe
// returns the number of pages produced
{    
    if(! psfp) return 0;
    
       genString entry, gfile, obj_kind, header;

       stePostScript *ps = new stePostScript(psfp);
       ps_initial_setup(ps);  

//header: name / time
       time_t tm;
       OSapi_time(&tm);
       header.printf("%s / %s", title, OSapi_ctime(&tm));
       ps->head_str((char *)header);

       symbolPtr el;
       ForEachS(el, selected){
         symbolPtr  xref = el.get_xrefSymbol();
         if (! xref.isnull()) {
//get the kind of object
             ddKind kind = el.get_kind();
             obj_kind = ddKind_external_name(kind);

//get the name of object
             char *nm = xref.get_name();
             entry.printf(" %s", nm);

//where entity is defined
             if (kind != DD_PROJECT  &&  kind != DD_MODULE) {
                 char *def_file;
                 symbolPtr file = el->get_def_file();
                 if (! file.isnull()) {
                     def_file = file.get_name();
                     gfile = " ==> ";
		     if (physical) {
		       int i = 0;
		       projNode *p;
		       genString fn;
		       for (p=projList::search_list->get_proj(i); p;i++, 
			  p=projList::search_list->get_proj(i)) {
			 p->ln_to_fn (def_file, fn);
			 if ((fn.length() > 0) && (global_cache.access ((char *)fn, R_OK) == 0)) {
			   gfile += fn;
			   break;
			 }
		       }
		       if (!p) gfile += def_file;
		     } else
		       gfile += def_file;
		   }
	       }
              gfile += '\n';
 
              ps->set_font(KIND_FONT, TEXT_SCALE1);
              ps->put((char *)obj_kind, strlen((char *)obj_kind));
              ps->set_font(ENTRY_FONT, TEXT_SCALE);
              ps->put((char *)entry, strlen((char *)entry));
              ps->set_font(ENTRY_FONT, TEXT_SCALE1);
              ps->put((char *)gfile, strlen((char *)gfile));

              ps->end_paragraph(); 
           }
       }	
       ps->end_page();
  }

int print_symbolArr(symbolArr &selected, FILE *psfp, char *title)
{
    return print_symbolArr(selected, psfp, title, 0);
}

int save_array_into_file(symbolArr &array, FILE *svfp)
{
    Initialize(save_array_into_file);
    
    symbolPtr el;
    ForEachS(el, array){
	symbolPtr  xref = el.get_xrefSymbol();
	if (!xref.isnull()) {
	    genString entry;
            //get the kind of object
	    ddKind kind        = el.get_kind();
	    genString obj_kind = ddKind_external_name(kind);
            //get the name of object
	    char *nm = xref.get_name();
	    entry.printf("%s", nm);
            //where entity is defined
	    if (kind != DD_PROJECT  &&  kind != DD_MODULE) {
		char *def_file;
		symbolPtr file = el->get_def_file();
		if (! file.isnull()) {
		    def_file = file.get_name();
		    entry += "|";
		    entry += def_file;
		}
	    }
	    entry += '\n';
	    OSapi_fprintf(svfp, "%s %s", (char *)obj_kind, (char *)entry);
	} else { // xref is null
	    ddKind kind = el.get_kind();
	    if(kind == DD_PROJECT || kind == DD_MODULE){
		genString obj_kind = ddKind_external_name(kind);
		//get the name of object
		genString entry;
		char *nm = el.get_name();
		entry.printf("%s\n", nm);
		OSapi_fprintf(svfp, "%s %s", (char *)obj_kind, (char *)entry);
	    }
	}
    }
    return 1;
}
