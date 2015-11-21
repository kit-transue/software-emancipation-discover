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
// view_creation_C
//------------------------------------------
// SYNOPSIS:
//
// Global utilities for creating and displaying views.
//
// ENTRY POINTS:
//
// void    view_set_target_viewer(viewer*);
// void    view_unset_target_viewer(viewer*);
// viewer* view_target_viewer()
// viewPtr view_create(RelationalPtr, repType = Rep_UNKNOWN, int enforce = 0);
// viewPtr view_create(const symbolArr&, repType);
// viewPtr view_create(const char *filenm);
// void    view_create_flush();
//------------------------------------------

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <assert.h>
#include <msg.h>
#include <objOper.h>

#include "representation.h"

#include <viewerShell.h>
#include <systemMessages.h>
#include <top_widgets.h>
#include <view_creation.h>

#include <objArr.h>
#include <objRawApp.h>
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <smt.h>
#include <smtRegion.h>

#include <dialog.h>

#include <ldrHeader.h>
#include <ldrERDHierarchy.h>
#include <ldrDCHierarchy.h>
#include <ldrSubsysMapHierarchy.h>

#include <appDGraph.h>

#include <xref.h>
#include <fileCache.h>
#include <sys/param.h>
#include <ste_send_message.h>
#include <ste_interface.h>
#include <machdep.h>
#include <steHeader.h>
#include <path.h>

#include <miniBrowser.h>
#include <ste_create_region.h>

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#ifndef _groupTree_h
#include <groupTree.h>
#endif

#include <HelpUI.h>
#include <gtPushButton.h>
#include <dfa_db.h>

// EXTERNAL FUNCTIONS
//extern app *proj_path_find_app (const char *fname, int no_raw = 0);
extern void proj_path_report_app (app *, projNode *prj = NULL);
//extern app *proj_path_convert_app (genString& path, genString& real_path, projNode *prj = NULL);

extern int multiple_psets;
extern appPtr proj_get_raw_app(const char* f_name);
int astNode_start(astNode*);
int astNode_length(astNode*);
//void errorBrowser_collect_errors_from_app (smtHeader*);
ldrHeader* dialog_layout(symbolPtr, repType);
smtTreePtr dd_get_smt(ddElementPtr);
smtTree* dd_get_smt_from_line(ddElement* ddel);
//smtTreePtr dd_get_any_smt(ddElementPtr);
appTreePtr dd_get_loaded_def_or_file(smtTreePtr);
appTreePtr dd_get_loaded_definition (symbolPtr);

int is_epoch_reporting();
//int paraGPI_open_ext_file(char *cname);
//int proj_no_delayed_loads();

void view_flush_on_top();
extern viewPtr ct_viewGraHeader(ldrPtr, repType);

int get_file_fn(const symbolPtr& symbol, genString& gs);
//void view_select (viewPtr v);
int dataCell_get_astNode(const symbolPtr &datasym, symbolPtr &symbol);

// CONSTANTS

enum { MAX_VIEW_NAME_LENGTH = 50 };


// VARIABLE DEFINITIONS

static int do_one_flush = 0; // Used from OpenFromBrowser, to always bring viewer on top.
static int do_one_flush_now = 0; // Used from end of graphis loop

// Each entry in rep_table describes a valid combination of appType and
// repType.  The first occurance of an appType provides the default repType.

static const struct repTypeTable
{
    appType app_type;
    repType rep_type;
}
rep_table[] =
{
   { App_STE,		Rep_TextText		},
   { App_STE,		Rep_TextDiagram		},
   { App_STE,		Rep_TextFullDiagram	},

   { App_SMT,		Rep_SmtText		},
   { App_SMT,		Rep_VertHierarchy	},
   { App_SMT,		Rep_FlowChart		},
   { App_SMT,		Rep_Tree		},

   { App_DD,		Rep_OODT_Inheritance	},
   { App_DD,		Rep_OODT_Relations	},
   { App_DD,		Rep_OODT_Scope		},

   { App_SUBSYS,	Rep_OODT_Inheritance	},

   { App_SCRAPBOOK,	Rep_Grid		},

   { App_DGRAPH,	Rep_DGraph		},

   { App_RAW,		Rep_RawText		},
};

// Number of entries in rep_table.
static const int rep_table_size = sizeof(rep_table) / sizeof(repTypeTable);

static viewPtr			last_created_view;

// viewer used by view_create_flush().
static viewer*			_target_viewer;


// FUNCTION DEFINITIONS

#ifndef _ASET_OPTIMIZE
typedef struct
{
    WidgetClass     superclass;		/* pointer to superclass ClassRec   */
    String          class_name;		/* widget resource class name       */
}
*StartOfWidgetClassPtr;

void print_widget_path(Widget w)
{
    Widget p = XtParent(w);
    if(p)
    {
	print_widget_path(p);
	putc(' ', stderr);
	putc('-', stderr);
	putc('>', stderr);
	putc(' ', stderr);
    }
    fputs(StartOfWidgetClassPtr(XtClass(w))->class_name, stderr);
    putc(' ', stderr);
    putc('"', stderr);
    fputs(XtName(w), stderr);
    putc('"', stderr);
}
#endif


viewer* view_set_target_viewer(viewer* vwr)
//
// Specify a viewer that should be used to display new views.
// Any previously created view will be flushed if necessary.
// Return previous target viewer.
//
{
    Initialize(view_set_target_viewer);

    viewer* prev_target_viewer = _target_viewer;

    if(_target_viewer != vwr)
    {
	view_create_flush();
        if (vwr)
	    _target_viewer = vwr;
    }

    return prev_target_viewer;
}


void view_unset_target_viewer(viewer* vwr)
//
// Specify a viewer that should NOT be used to display new views.
//
{
    Initialize(view_unset_target_viewer);

    if(_target_viewer == vwr)
	_target_viewer = NULL;
}



viewer* view_target_viewer()
//
// Return the viewer where new views will be created.
//
{
    Initialize(view_target_viewer);
    
    viewer* pv = viewer::preferred_viewer();
    if(pv && !is_epoch_reporting())
	return pv;

    if(_target_viewer == NULL)
	_target_viewer = viewerShell::find_target_viewer();

    return _target_viewer;
}

static appTreePtr dd_get_smt_or_smt_root(ddElement *dd)
{
  Initialize(dd_get_smt_or_smt_root);

  if (!dd) return NULL;
  appTreePtr at = dd_get_smt(dd);
  if (!at) {
    at = dd_get_smt_from_line(dd);
    if (!at) {
      at = checked_cast(appTree, get_relation(ref_file_of_dd, dd));
      if (!at) {
	appPtr ah = dd->get_main_header();
	if (ah && is_smtHeader(ah))
	  at = checked_cast(appTree, ah->get_root());
      }
    }
  }
  return at;
}

static appTreePtr view_prefilter(appTreePtr app_node, repType rep_type)
//
// Pre-filter the some types of app-nodes to convert them to a
// more appropriate type of object for viewing.
//
{
    Initialize(view_prefilter);

    if(app_node == NULL)
	return NULL;

    if(is_projModule(app_node))
    {
	appPtr app_head = projModulePtr(app_node)->load_module();
	if(app_head)
	    app_node = checked_cast(appTree, app_head->get_root());
    }
    else if(is_ddElement(app_node))
    {
	ddElementPtr dd_elem = ddElementPtr(app_node);
	if (dd_has_smt_direct(dd_elem->get_kind()))
	{
    	    ddKind k;
	    if (dd_elem->get_kind() == DD_FUNC_DECL)
	      if (dd_elem->is_pure_virtual())
	          if ((rep_type == Rep_VertHierarchy) || (rep_type == Rep_FlowChart))
		      if (!msg_question (TXT("You have selected a pure ")
					 TXT("virtual member function.\n")
				         TXT("The view will be of the ")
					 TXT("corresponding class ")
					 TXT("definition.")))
		          return NULL;
	    if (((k = dd_elem->get_kind()) == DD_CLASS || k == DD_INTERFACE) && 
		rep_type != Rep_VertHierarchy && rep_type != Rep_SmtText &&
		rep_type != Rep_FlowChart && rep_type != Rep_UNKNOWN)
		return app_node;
		
	    app_node = dd_get_smt_or_smt_root(dd_elem);
	}
	else if (dd_has_smt_via_main_header(dd_elem->get_kind()))
	    app_node = checked_cast(appTree, (dd_elem->get_main_header())->get_root());
	else if (dd_elem->get_kind() == DD_SUBSYSTEM)
	    app_node = dd_get_def_assocnode(dd_elem);

    }
    else if (is_smtTree(app_node) && rep_type == Rep_OODT_Inheritance)
       app_node = smt_get_dd(smtTreePtr(app_node));

    return app_node;
}


static repType default_rep_type(appType app_type)
//
// Derive the default rep-type from rep_table.
//
{
    Initialize(default_rep_type);

    for(int i = 0; i < rep_table_size; ++i)
    {
	if(rep_table[i].app_type == app_type)
	    return rep_table[i].rep_type;
    }
    return Rep_UNKNOWN;
}


static int invalid_rep_type(appType app_type, repType rep_type)
//
// Return true if the specified app/rep-type pair is valid.
//
{
    Initialize(invalid_rep_type);

    for(int i = 0; i < rep_table_size; ++i)
    {
	if(rep_table[i].app_type == app_type  &&
	   rep_table[i].rep_type == rep_type)
	    return 0;
    }
    return 1;
}

//boris: returns 1 and sets the rep_type to available value. Otherwise, return 0.
//       if   enforce_rep_type == 1  ,  than check only the specified rep_type (ignore the default)
//
static int check_view_rep_type (appType app_type, repType& rep_type, int enforce_rep_type)
{
    Initialize(check_view_rep_type);
    int res  = 0;

    //default type for the app_type
    repType def_type = default_rep_type(app_type);

    if(rep_type < 0)		// Look for default rep-type.
    {
	if(def_type >= 0)
	{
	    rep_type = def_type;
	    res = 1;
	}
    }
    else if(invalid_rep_type(app_type, rep_type))
    {
	if( !(enforce_rep_type  ||  def_type < 0) )
	{
	    rep_type = def_type;
	    res = 1;
	}
    } else
	res = 1;
    
    return res;
}

static void view_final_prep(viewPtr view_head)
//
// Associate the view-header with a dialog in a viewerShell.
//
{
    Initialize(view_final_prep);

    if(view_head)
    {
	viewerShell* vsh = view_target_viewer()->get_shell();
	view_flush_on_top();

	// Insert view-header into dialog if it is not already there.
	dialogPtr dlg = vsh->getDialog();
	if(!dlg->contains_view(view_head))
	    dlg->set_view(dlg->empty_view(), view_head);

	// Update the list-of-views option menu.
	vsh->add_view_to_menu(view_head);

	// signal that the view should behave like a newly created view
	// (used in steView::open_window)
	if(is_steView(view_head))
	   steView::new_view_flag = 1;

	// Remember most recently created view.
	last_created_view = view_head;
    }
}


static viewPtr view_create_internal(
    appPtr app_head, appTreePtr app_node, repType rep_type)
//
// Do the real work of creating a view-header for app_node.
// The view is not displayed anywhere.
//
{
    Initialize(view_create_internal(appPtr,appTreePtr,repType));

    push_busy_cursor();

    viewPtr view_head = NULL;

    app_node              = app_node->get_world();
    
    ldrPtr ldr_head =
        dialog::layout(app_head, app_node, rep_type, 0, 0, rtlDisplayUnknown);
    if(ldr_head)
    {
        // Attempt to re-use an existing view.
        viewerShell* vsh = view_target_viewer()->get_shell();
        view_head = vsh->find_view_for_viewer(ldr_head, view_target_viewer());
        
        if(view_head == NULL)	// No existing view available, make a new one.
    	view_head = dialog::open_view(ldr_head, rep_type, 0);
        
        view_final_prep(view_head);
    }
    pop_cursor();

    return view_head;
}

static genString epoch_command("(recenter 0)");

static int focus_on_view (view *view_ptr, appTree *node, int highlight, int scroll_to_top)
{
    Initialize(focus_on_view);
    int to_ret = 0;
    if (node && view_ptr && is_steView(view_ptr)) {
	steView *v = checked_cast(steView, view_ptr); 
	steRegion *p;
	if(highlight) {
	    appTree *st = checked_cast(appTree,node->get_first_leaf());
	    p = ste_create_region (v, st, st);
	}

	OperPoint pt(node, FIRST);
	v->insertion_point (&pt);
	
	if(scroll_to_top)
	    ste_epoch_evaluate(epoch_command);

	to_ret = 1;
    }

    return to_ret;
}

int focus_on_node(appTree *node, int highlight, int scroll_point_to_top) 
// open a view with the focus on object node    
// if hightlight is set, highlight node
// if scroll_point_to_top, place the cursor at the top of the screen    
// (cursor in the middle otherwise)    
{ 
  Initialize(focus_on_node);

  if (!node)
      return 0;

  repType tp = Rep_SmtText;
  if (!is_smtTree(node))
      tp = Rep_RawText;

  appPtr h = checked_cast(app, node->get_header());
  appTreePtr t = (h) ? checked_cast(appTree,h->get_root()) : NULL;

  if (!t)
      return 0;

  viewPtr view_ptr = view_create_internal (h, t, tp);
  view_create_flush();

  if (tp == Rep_RawText)
      return 1;

  return focus_on_view (view_ptr, node, highlight, scroll_point_to_top);
}

//------------------------------------------
// open selection stack items SMT definition
int create_view_of_def (symbolPtr sym)
{
    Initialize (create_view_of_def);

    appTree *def = dd_get_loaded_definition (sym);
    int res = focus_on_node (def, 0, 1);

    return res;
}

static viewPtr view_create_internal(symbolPtr app_node, repType rep_type)
//
// Do the real work of creating a view-header for app_node.
// The view is not displayed anywhere.
//
{
    Initialize(view_create_internal(symbolPtr,repType));

    push_busy_cursor();

    viewPtr view_head = NULL;
    
    ldrPtr ldr_head =
        dialog_layout(app_node, rep_type);
    if(ldr_head)
    {
        // Attempt to re-use an existing view.
        viewerShell* vsh = view_target_viewer()->get_shell();
        view_head = vsh->find_view_for_viewer(ldr_head, view_target_viewer());
        
        if(view_head == NULL)	// No existing view available, make a new one.
    	view_head = dialog::open_view(ldr_head, rep_type, 0);
        
        view_final_prep(view_head);
    }

    pop_cursor();

    return view_head;
}

void send_focus_command(void *data)
{
    int offset = (int)data;
    genString cmd;
    cmd.printf("(goto-char %d)", offset);
    ste_epoch_evaluate( cmd );
    cmd.printf("(di-delete-selections)");
    ste_epoch_evaluate( cmd );
    cmd.printf("(di-highlight-word nil)");
    ste_epoch_evaluate( cmd );
}

static void send_focus_command_region(void *data)
{
    astNode* ast = (astNode*)data;
    int offset = 0, size = 0;
    if (ast) {
	offset= astNode_start(ast);
	size = astNode_length(ast);
    }
    genString cmd;
    cmd.printf("(goto-char %d)", offset);
    ste_epoch_evaluate( cmd );
    cmd.printf("(di-delete-selections)");
    ste_epoch_evaluate( cmd );
    if (size)
	cmd.printf("(di-highlight-word %d)", size);
    else
	cmd.printf("(di-highlight-word nil)");
    ste_epoch_evaluate( cmd );
}

void msg_model_is_not_good(repType rep_type, char*filename);

extern int project_name_to_fn(char const *, genString &);

// file_is_not_viewable defined in xref.h.C
extern bool file_is_not_viewable(symbolPtr &file);

static bool is_not_viewable(symbolPtr &file)
{
    if (file_is_not_viewable(file)) {
        symbolPtr xfile = file.get_xrefSymbol();
        static  genString phys_string;
        char *proj_name = xfile.get_name();
        project_name_to_fn(proj_name, phys_string);
        const char *phys_name = phys_string.str();
    	if (phys_name == NULL) {
    	    phys_name = proj_name;
	}
	msg("\"$1\" cannot be viewed as text.", error_sev) << phys_name << eom;
	return true;
    }
    return false;
}

static ddElement *get_dd_from_smt_or_dd(appTree *ra)
{
    Initialize(get_dd_from_smt_or_dd);
    ddElement *rdd = 0;
    if (is_smtTree(ra))
	rdd = smt_get_dd(checked_cast(smtTree, ra));
    else if (is_ddElement(ra))
	rdd = checked_cast(ddElement, ra);
    return rdd;
}

static viewPtr
create_view_instance(symbolPtr sym)
{
	Initialize(create_view_instance);
    	// It is important to do this near the beginning so that 
    	//   we do not load any pset files.    
	symbolPtr src = sym.scope();
	if (src.xrisnull()) { return NULL; }
	int offset = sym.char_offset();
	
	
	// get module and open it without loading pset      
	ddKind kind = src.get_kind();
	symbolPtr mod;
	if (kind != DD_MODULE) {
	    symbolPtr xsym = src.get_xrefSymbol();
	    if(xsym.xrisnull())
		return NULL;
	    mod = xsym->get_def_file();
	} else {
	    mod = src;
	}
	if (mod.isnull() || (mod.get_kind() != DD_MODULE)) 
	    return NULL;
	
	genString fname;
	if (!get_file_fn(mod, fname)) 
	    return NULL;
	
	appPtr app_head = proj_get_raw_app(fname);
	appTree *rt;
	
	if(app_head == NULL || !(rt = checked_cast(appTree,app_head->get_root())))
	{
	    //	  ste_print_message ("ASCII file not found.");
	    return NULL;
	} else {
	    viewPtr v = NULL;
	    if(v = view_create(app_head)) {
		// flush view first, so "goto-char" command will work on this file
		view_create_flush();
		pset_send_own_callback(send_focus_command, (void *)(offset + 1));
		return last_created_view;
	    }
	}
	return NULL; // now we probably should just bite the bullet and load a pset
}

static viewPtr
create_view_ast(symbolPtr sym)
{
    Initialize(create_view_ast);
    assert(sym.is_ast());

      astXref *axr = sym.get_astXref();

      if (axr) {
	symbolPtr mod = axr->get_module();

	if (mod.isnull())  
	  return NULL; 

	astNode *ast = astNode_of_sym(sym);
	appPtr app_head = (app *) mod;
	appTree *rt;
	
	if (app_head == NULL || !(rt = checked_cast(appTree,app_head->get_root()))) {
	  //	  ste_print_message ("ASCII file not found.");
	  return NULL;
	} else {
	  viewPtr v = NULL;

	  if (v = view_create(app_head)) {
	    // flush view first, so "goto-char" command will work on this file
	    view_create_flush();
	    pset_send_own_callback(send_focus_command_region, (void *) ast);
	    return last_created_view;
	  }
	}
	return NULL; // now we probably should just bit the bulet and load a pset
      }
}

viewPtr view_create(symbolPtr sym, repType rep_type, int enforce_rep_type)
//
// Create a view of an object, attempting to preserve the rep_type.
//
// if rep_type is Rep_UNKNOWN and enforce_rep_type != 0,
// open a view on the symbol's file and focus the symbol
// instead of just opening a view of the symbol
// 
{
    Initialize(view_create(symbolPtr,repType,int));

    // First crack: things that have no views:
    if (sym.isnull()) {
	return NULL;
    }

    // Second crack: certain kinds of symbols get no choice of views:
    // (sorry about the blocks, but they make the variables more local)
    switch (sym.get_kind()) {
	case DD_SMT:
	case DD_REGION:
    	    {
		smtTree *smt = GetSmtFromMarker(sym);
		if(smt){
	    	    focus_on_node(smt, 0, 1);
	    	    return last_created_view;
		}
		return NULL;
    	    }
		break;
	case DD_INSTANCE:
		return create_view_instance(sym);
		break;
	case DD_AST:
		return create_view_ast(sym);
		break;
	case DD_DATA:
    	    {
		// convert to an ast node and try again with that, eh?
		// this is a little strange: below we do conversions
		//   without recursion: we're happy to change sym.
      		symbolPtr node;
      		dataCell_get_astNode(sym, node);
      		return view_create(node, rep_type, enforce_rep_type);
    	    }
		break;
    }

    // Deal with pset entities.  Two possibilities:
    if (sym.relationalp()) {
	Relational * rel = sym;
	if (rel) {
	    if (is_appDGraph(rel) && rep_type==Rep_DGraph) {
		// first (Third crack): it's a magic DGraph set pair
		appDGraph * gh = checked_cast(appDGraph,rel);
		return view_create_internal((app *)gh, (appTree *)(gh->get_root()), rep_type);
	    }
	    else if (is_dd_or_xref_node(rel)) {
		// second: it's a pset entity.  Set sym to corresponding xref 
		dd_or_xref_node * nd = (dd_or_xref_node *)rel;
		sym = nd->get_xrefSymbol();
	    }
	}
    }

    symbolPtr xsym = sym.get_xrefSymbol();

    int is_text_view = 0;

    // Yet another hack...  Force all calls to open SmtText to open
    // the file the same as mini-brower.
    if(rep_type==Rep_SmtText)
	rep_type=Rep_UNKNOWN;
	
    // Fourth crack: it's really a file.  Open it.
    if ( xsym.get_kind() == DD_MODULE) {
	switch (rep_type) {
	  case Rep_SmtText :
	    is_text_view = 1;
	    break;
	  case Rep_UNKNOWN :
	    {
		int lang = sym.get_language();
	    	if (lang == FILE_LANGUAGE_C ||
		    lang == FILE_LANGUAGE_CPP ||
		    lang == FILE_LANGUAGE_ESQL_C ||
		    lang == FILE_LANGUAGE_ESQL_CPP ||
		    lang == FILE_LANGUAGE_ELS)
			is_text_view = 1;
	    	break;
	    }
	    default :
	    break;
	}
    	if (is_text_view) {
    	    if (is_not_viewable(xsym)) {
    	    	return NULL;
	    }
    	}
    }

    // Fifth crack: do the right thing depending on view:
    switch (rep_type) {
	case Rep_MultiConnection:
	case Rep_OODT_Relations:
	case Rep_OODT_Scope:
		return view_create_internal (sym, rep_type);
		break;  // not reached
    	case Rep_Tree:
    	    {
      		ddElement *new_dd = 0;
      		if (xsym.xrisnull() && sym.relationalp()) {
			//sudha:11/97 To open call tree from flow chart
			new_dd = get_dd_from_smt_or_dd(sym);
			Relational *obj = sym;
			if (!new_dd && is_smtTree(obj)) {
	  		    smtTreePtr smt = checked_cast(smtTree, obj);
	  		    smt = checked_cast(smtTree, smt->get_parent());
          		    if (smt) {
	    		        new_dd = get_dd_from_smt_or_dd(smt);
			    }
			}
      		}    
      		if (new_dd)
		    return view_create_internal(new_dd, rep_type);     
      		else
		    return view_create_internal(sym, rep_type);
    	    }
		break;	// not reached
        case Rep_ERD:
    	case Rep_DataChart:
	    {
		// require collections: convert sym into arr
		symbolArr arr;
		arr.insert_last(sym);
		return view_create(arr, rep_type);
	    }
		break;	// not reached
    	case Rep_OODT_Inheritance:
	    {
    		ddKind k;
    		if( xsym.isnotnull() &&
       			((k = xsym.get_kind()) == DD_CLASS || k == DD_INTERFACE))
    		{
       		    return view_create_internal(xsym, rep_type);
    		}
	    }
		break;	// don't know what continuing means.
    	case Rep_FlowChart:
	    {
    		if( xsym.isnotnull() &&
       			(xsym.get_kind() != DD_FUNC_DECL))
    		{
		    msg("Only functions may be displayed in FlowCharts.", error_sev) << eom;
       		    return NULL;
    		}
	    }
		break;	// type check OK
	default:
		// lots of stuff left
		break;
    } // end case rep_type


    // special case to deal w/project objects
    // 010501 kit transue: I think the sym.relationalp() is always false,
    // because of the conversion from psets around Crack Three above
    if( xsym.isnotnull() &&
	xsym.get_kind() == DD_PROJECT
	&& ! (sym.relationalp() && is_RTL((RelationalPtr) sym)))
    {
	appPtr project = sym.get_def_app();
	if(project) {
	    if(is_projHeader(project)) {
		projNode *pn = checked_cast(projNode,project->get_root());
		pn->refresh();
	    }
	    return view_create(project, rep_type, enforce_rep_type);
	}
    }


    // if symbol isn't loaded, load if possible...
    if (sym.is_xrefSymbol() && !sym.is_loaded()) {
	ddKind sym_kind = sym.get_kind();
	symbolPtr def_file = sym;
	if (sym_kind != DD_MODULE)
	    def_file =sym->get_def_file();
	
	// ...return nothing if impossible:
        if (def_file.isnull()){
            msg("Could not find definition of \"$1\".", error_sev) << sym.get_name() << eom;
	    return NULL;
        }
        if (is_not_viewable(def_file)) {
	    return NULL;
    	}

	projNode *pr = projHeader::get_pmod_project_from_ln(def_file.get_name());
	genString pn;

	if (pr) {  //boris: fixing bug11340. Opening non existing file ( 4 = R_OK )
	    pr->ln_to_fn(def_file.get_name(), pn);
	    if ( !pn.length() || global_cache.access ((const char *)pn, 4) != 0 )
		pr = NULL;
	}
	
	if (pr) {
	    bool edt_view = (is_text_view || rep_type == Rep_UNKNOWN);
	    if (sym_kind == DD_MODULE && edt_view)
		return view_create ((const char *)pn);

	    view *v      = NULL;
	    app *h       = NULL;
	    appTree *def = NULL;
	    
	    def = dd_get_loaded_definition (sym);
	    if (def)
		h = def->get_header();
	    else {
		h = pr->restore_file_wo_module(def_file.get_name());
		if (h) {
		    proj_path_report_app (h, pr);
		    def = checked_cast(appTree,h->get_root());
		}
	    }

	    if (def && h) {
		appTree *rt = checked_cast(appTree,h->get_root());
		int res = check_view_rep_type ((appType)h->get_type(), rep_type, enforce_rep_type);
		if (res == 1) {
		    if (edt_view)
			v = view_create_internal (h, rt, rep_type);
		    else
			v = view_create_internal (h, def, rep_type);

		    if (v && edt_view && rep_type == Rep_SmtText)
			(void) focus_on_view (v, def, 0, 1);
		}
	    }

	    if (!v) {
		msg_model_is_not_good (rep_type, (char *)pn);
		if (edt_view)
		    v = view_create ((const char *)pn);
	    }

	    return v;
	}
    }


    RelationalPtr obj = sym;	 // last use of sym; attention changes to obj
    if(obj == NULL)
    {
	    // conversion failed; try and open the file?
	    ddKind d_kind = sym.get_kind ();

	    if (d_kind == DD_MODULE)
		return view_create(sym.get_name());

	    msg("Could not load definition of \"$1\".", error_sev) << sym.get_name() << eom;
	    return NULL;
    }


    // Look for a few special types of objects, and open them immediately.
    // Bail out if a suitable app_node is not found.
    if (is_projModule(obj))
    {
	projModule* module = (projModule*) obj;
	genString pn;
	module->get_phys_filename (pn);
	
	if(is_text_view && !multiple_psets) {
            return view_create ((const char *)pn);
	}

        appPtr appptr = module->restore_module();
        if (!appptr) {
            return view_create ((const char *)pn);
	} else {
	    obj = appptr;
	}
    }
 

    // attention now changes from obj to app_node
    appTreePtr app_node = NULL;

    if(is_app(obj))
    {
	appPtr app_head = appPtr(obj);
	app_node = checked_cast(appTree, app_head->get_root());
    }
    else if (is_appTree(obj))
    {
        app_node = appTreePtr(obj);
      
	//
	// For the time being we are going to treat Open Selection the
	// same as Open Definition for graphical views!
	//
	// So here we enter the code for Opening the definition of the
	// selected appPtr if it is a dd_or_xref_node
	//
	
	if (app_node && is_smtTree(app_node) ){
	    appTreePtr def = dd_get_loaded_def_or_file(smtTreePtr(app_node));
	    if(def && (def != appTreePtr(app_node)))
	    {
	        if(rep_type == Rep_UNKNOWN  && enforce_rep_type) 
		{
		    // You want the location the the definition
		    // not the selected app_node...
		    if(focus_on_node(def, 0, 1))
		        return last_created_view;
		}
		return view_create(def, rep_type);
	    }
	}
    }
   
    app_node = view_prefilter(app_node, rep_type);
    if(app_node == NULL)
	return NULL;

    // Get the app-header.
    appPtr app_head = checked_cast(app, app_node->get_header());
    Assert(app_head);

    projModule *app_head_module = app_head->get_module();
    if (app_head_module != NULL) {
        symbolPtr file = app_head_module->get_def_file_from_proj();
    	if (is_not_viewable(file)) {
	    return NULL;
	}
    }

    // Give smt-headers a chance to initialize themselves for viewing.

    if(is_smtHeader(app_head))
    {
	// Make SMT tree for header (by importing or restoring).

	smtHeaderPtr smt_head = smtHeaderPtr(app_head);
	if(!smt_head->parsed)
        {
            projModule *module = projModule::projModule_of_app(app_head);
            if (module)
            {
                app_head = module->load_module();
                app_node = checked_cast(appTree, app_head->get_root());
            }
        }
    }

    if(rep_type == Rep_UNKNOWN  && enforce_rep_type) {
      if(focus_on_node(app_node, 0, 1))
	  return last_created_view;
    }

    // Find a suitable rep-type, or bail out.
    if ( check_view_rep_type((appType)app_head->get_type(), rep_type, enforce_rep_type) == 0 ) {
	gtPushButton::next_help_context("Browser.View.Help.Module");
	msg("$1 is not a viewable module in the selected mode", error_sev) << (char *)app_head->get_name() << eom;
	return NULL;
    }

    view *v_ptr = NULL;
    if (app_head && is_smtHeader(app_head) &&
	(rep_type == Rep_SmtText || rep_type == Rep_UNKNOWN)) {
	appTree *focus_node = app_node;
	appTree *rt = checked_cast(appTree,app_head->get_root());
	v_ptr = view_create_internal (app_head, rt, rep_type);
	int res = focus_on_view (v_ptr, focus_node, 0, 1);
    } else
	v_ptr = view_create_internal(app_head, app_node, rep_type);

    return v_ptr;
}


viewPtr view_create(const symbolArr& array, repType rep_type)
//
// Return a view-header for the objects in array.
//
{
    Initialize(view_create(symbolArr&,repType));

    // This array will contain the objects used to construct the ldr.
    symbolArr view_contents;

    // This string will contain the name of the view, based on the arguments.
    genString view_name;

    // Insert appropriate prefix for view-name, and reject bad rep-types.
    switch(rep_type)
    {
      case Rep_ERD:
	view_name = gettext(TXT("ERD{"));
	break;

      case Rep_DataChart:
	view_name = gettext(TXT("Data Chart{"));
	break;

      default:			// Avoid unsupported rep-types.
	msg("Unsupported view type", error_sev) << eom;
	return NULL;
    }

    // Pre-filter objects in array and construct name of view.

    int need_comma = 0;

    symbolPtr elem;
    ForEachS(elem,array)
    {
	appTreePtr obj;
	symbolPtr elem_xref = elem.get_xrefSymbol();

	if (elem_xref.xrisnull()) {
            char *nm = elem.get_name();
	    if (nm != NULL)
		msg("Cannot add \"$1\" to view: wrong kind of entity.", error_sev) << nm << eom;
            continue;
        }

	// Find the name of the object to use in the name of the view.
	// Leaving component_name empty means ignore the object.

	const char* component_name = NULL;
	switch(rep_type)
	{
	  case Rep_ERD:
	  case Rep_DataChart:
	    //if the component is a function call, check if it is a call to 
	    //constructor or destructor. If so, get the corresponding class
	    //element and use that - ie., selecting constr or destr calls for
	    //building ERD or DC should be the same as selecting the class.
	    if(elem_xref.get_kind() == DD_FUNC_DECL){
                if(elem_xref->get_attribute(CONS_DES,1)){
 
                    symbolPtr new_elem = elem_xref->get_parent_of();
		    if (new_elem.xrisnotnull())
		      view_contents.insert_last(new_elem);
                    component_name = NULL;
                }
 
            }else
               component_name = elem_xref.get_name();
	    break;

	}

	// If component_name was not set, then the object is
	// inappropriate and should be ignored.

	if(component_name)
	{
	    if(need_comma)
		view_name += ",";
	    else
		need_comma = 1;
	    view_name += component_name;

	    view_contents.insert_last(elem);
	}
    }
    view_name += "}";

    // Truncate view_name if it is too long.
    if(view_name.length() > MAX_VIEW_NAME_LENGTH)
	strcpy((char*)view_name + MAX_VIEW_NAME_LENGTH - 4, "...}");

    // Create the ldr and view,
    // assuming something made it through the pre-filtering.

    ldrHeaderPtr ldr_head = NULL;
    viewPtr view_head = NULL;

    if (!view_contents.empty() || rep_type == Rep_ERD ||
	rep_type == Rep_DataChart)
    {
	push_busy_cursor();

	switch(rep_type)
	{
	  case Rep_ERD:
	    ldr_head = db_new(ldrERDHierarchy, (view_contents));
	    view_head = ct_viewGraHeader(ldr_head, Rep_ERD);
	    break;

	  case Rep_DataChart:
	    ldr_head = db_new(ldrDCHierarchy, (view_contents));
	    view_head = ct_viewGraHeader(ldr_head, Rep_DataChart);
	    break;

	  default:
	    break;
	}
	if(ldr_head && view_head)
	{
	    put_relation(view_of_ldr, ldr_head, view_head);
	    view_head->set_name(view_name);
	    view_final_prep(view_head);
	}
	pop_cursor();
    }

    return view_head;
}

void view_flush_on_top()
{
    do_one_flush = 1;
}

static int flushing = 1;

void view_flushing(int state)
{  
  flushing = state;
}

void view_create_flush_finish()
{
    do_one_flush_now = 1;
}

void view_create_flush()
//
// Make the most recently created view visible in the target viewer.
//
{
    Initialize(view_create_flush);

    if(last_created_view)
    {
	push_busy_cursor();
  
	viewer* vwr      = view_target_viewer();
	viewPtr old_view = vwr->get_view_ptr();

	if (old_view != last_created_view)
	    vwr->add_view_to_history();
	vwr->change_view(last_created_view);
	

	viewerShell* vsh = vwr->get_shell();

//	if (last_created_view && (_target_viewer != vwr)) 
	if (last_created_view && flushing)
	    vsh->bring_to_top();

	if (flushing) vsh->map();

	// Reset last_created_view BEFORE calling viewerShell::focused_viewer().
	last_created_view = NULL;

	viewerShell::focused_viewer(vwr);

	pop_cursor();
    }
    if(do_one_flush && do_one_flush_now) {
	viewerShell *vsh = viewerShell::get_current_vs(1); // Don\'t create if not exists.
	if(vsh)
	    vsh->bring_to_top();
	do_one_flush = 0;
	do_one_flush_now = 0;
    }
}


int view_displays_app (viewPtr view_head, appPtr app_head, appTreePtr root)
{
    Initialize(view_displays_app);

    if(!(view_head  &&  app_head))
	return 0;

    ldrPtr ldr_head = view_get_ldr(view_head);
    app *  view_app = (ldr_head) ? ldr_get_app(ldr_head) : NULL;
    
    if (!view_app || view_app != app_head)
	return 0;
    
    if(root == NULL)
	root = checked_cast(appTree,app_head->get_root());

    if(ldr_head)
    {
	ldrTreePtr ldr_root = checked_cast(ldrTree,ldr_head->get_root());
	appTreePtr app_root = ldr_root ? ldr_root->get_appTree() : NULL;
	if (app_root && (app_root == root || root->subtree_of(app_root)))
	    return 1;
    }
    return 0;
}

appPtr get_create_app_by_name (const char* f_name);

viewPtr view_create(appPtr app_head)
{
    Initialize(view_create);
 
    viewPtr view_head = view_target_viewer()->get_view_ptr();

    if(view_displays_app(view_head, app_head))
    {
        if ( is_steView (view_head) ) {
           ste_set_current_focus (checked_cast(steView,view_head));
	   ste_print_message("File is in a target viewer.");

	   last_created_view = NULL;
	   return view_head;
       }
    }


    switch(app_head->get_type())
    {
      case App_STE:
	return view_create(app_head->get_root(), Rep_TextText);

      case App_SMT:
	return view_create(app_head->get_root(), Rep_SmtText);

      case App_RTL:      // Simple text view of an rtl file
      case App_RAW:
	return view_create_internal(app_head,
	    checked_cast(appTree,app_head->get_root()), Rep_RawText);
   }

    return NULL;
}


viewPtr view_create(const char* filename)
//
// Returns existing or new view-header that shows in a current viewer
// existing or new app-header with name "filename"
//
{
    Initialize(view_create(char*));

    if(!(filename  &&  *filename))
	return NULL;

    viewPtr v = NULL;
    appPtr app_head = get_create_app_by_name(filename);

    if(app_head == NULL)
	ste_print_message ("ASCII file not found.");
    else
	v = view_create(app_head);

    return v;
}

viewPtr view_create_noselect (app *app_head)
{
    Initialize(view_create_noselect_appPtr);
    appTreePtr rt = NULL;
    if(app_head == NULL || !(rt = checked_cast(appTree,app_head->get_root())))
    {
	return NULL;
    }

    repType tp;
    switch(app_head->get_type())
    {
      case App_STE: tp = Rep_TextText; break;
      case App_SMT: tp = Rep_SmtText;  break;
      case App_RAW: tp = Rep_RawText;  break;
      default:
	ste_print_message ("Failed to create buffer");
	return NULL;
    }
    
    viewPtr view_head = NULL;
    ldrPtr ldr_head = 
	dialog::layout(app_head, rt, tp, 0, 0, rtlDisplayUnknown);
    if (ldr_head)
    {
	// Attempt to re-use an existing view.
	Obj *el, *v_set = ldr_get_views(ldr_head);
	ForEach(el, *v_set) {
	    RelationalPtr vvv = (RelationalPtr)el;
	    view_head = checked_cast(view,vvv);
	    break;
	}

	if (view_head == NULL)	// No existing view available, make a new one.
	    view_head = dialog::open_view(ldr_head, tp, 0);
    }

    view_final_prep (view_head);
    last_created_view = NULL;
    return view_head;
}


viewPtr view_create_noselect(const char* filename)
//
// Returns existing or new view-header and does not select it

//
{
    Initialize(view_create_noselect_filename);

    if(!(filename  &&  *filename))
	return NULL;

    appPtr app_head = proj_get_raw_app(filename);
    appTree *rt;

    if(app_head == NULL || !(rt = checked_cast(appTree,app_head->get_root())))
    {
	ste_print_message ("ASCII file not found.");
	return NULL;
    }

    viewPtr view_head = view_create_noselect (app_head);
    return view_head;
}

void view_select (viewPtr v) {
    Initialize(view_select);
    viewer *vwr = view_target_viewer ();
    if (vwr) {
	push_busy_cursor();
	if (vwr->get_view_ptr() != v) {
	    vwr->change_view(v);
	    viewerShell* vsh = vwr->get_shell();
	    vsh->map();
	}
	last_created_view = NULL;
	viewerShell::focused_viewer(vwr);
	pop_cursor();
    }
}

int has_view(app *h)
{
    Initialize(has_view);

    Obj *ldr_set = app_get_ldrs(h);
    Obj *cur_ldr, *view_set, *cur_view;

    ForEach(cur_ldr,*ldr_set){
	ldrPtr ldrh = checked_cast(ldr,cur_ldr);
	view_set = ldr_get_views(ldrh);
	ForEach(cur_view,*view_set){
            return 1;
        }
    }

    return 0;
}

/*
   START-LOG-------------------------------------------

   $Log: view_creation.C  $
   Revision 1.106 2002/02/20 17:01:52EST ktrans 
   DISCOVER debranding

   END-LOG---------------------------------------------
*/





