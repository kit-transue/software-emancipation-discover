service DISmain


!#ifdef SERVER
!#include <cLibraryFunctions.h>
!#include <stdlib.h>
!#include <fstream.h>
!
!#include "../../clients/DIS_main/include/prefs.h"
!#include "customize.h"
!#include "pref_registry.h"
!#include "proj.h"
!#include "driver.h"
!#include "symbolPtr.h"
!#include "symbolScope.h"
!#include "ParaCancel.h"
!#include "steView.h"
!#include "dialog.h"
!#include "symbolScope.h"
!#include "closure.h"
!#include "BrowserController.h"
!#include "LanguageController.h"
!#include "BrowserData.h"
!
!#include "dis_view_create.h"
!#include "SelectionIterator.h"
!#include "ginterp.h"
!#include "gRTListServer.h"
!#include "gLayerServer.h"
!#include "gviewerShell.h"
!#include "gviewerMenuBar.h"
!#include "gviewerPopupMenuList.h"
!#include "representation.h"
!#include "symbolArr.h"
!#include "objOper.h"
!#include "partition.h"
!#include "messages.h"
!#include "iQuestion.h"
!#include "dis_view_create.h"
!#include "StatusDialog.h"
!#include "genError.h"
!#include "smt.h"
!#include "objRawApp.h"
!#include "db.h"
!#include "../DIS_ui/interface.h"
!#include "messages.h"
!#include "errorBrowser.h"
!#include "xref_queries.h"
!
!#include "symbolArr.h"
!#include "objOper.h"
!#include "partition.h"
!#include "smt.h"
!#include "objRawApp.h"
!#include "db.h"
!#include "viewTypes.h"
!#include "../../clients/DIS_main/include/projscope.h"
!#include "Interpreter.h"
!#include "statistics.h"
!#include "metric.h"
!#include "charstream.h"
!#include "Interpreter.h"
!#include "assocbridge.h"
!
!
!int dis_select (char *f_name, int lin, int off, int tab, const char *tok, int add);
!int dis_cm_get(int);
!
!int dis_cm_event_internal(const char* event, const char *fname);
!int dis_ide_command_event(int argc, char *argv[]);
!
!int dis_create_view_of_def (symbolPtr sym);
!void getSelection (int id, symbolArr &symbols);
!
!void get_all_uses(symbolArr& entities, symbolArr& result);
!void Part_Check_Conflicts_UI(symbolArr& array, symbolArr& to_be_declared, symbolArr& to_be_edited);
!void do_move_entities_ask_file_new(symbolArr& array, genString filename, int filetype, symbolArr& to_be_declared, symbolArr& to_be_edited, symbolArr& file_to_fetch);
!void update_selection(symbolArr& modules);
!void unload_selection(symbolArr& modules);
!void preload_selection(symbolArr& modules);
!void setup_home_from_selection(symbolArr& projects);
!extern "C" void aset_driver_exit( int );
!int cli_process_input(char *, ostream&, Interpreter*, int do_propmt_and_print);
!void newui_handle_menuitem(int argc, char **argv);
!void GetSavedResult( Interpreter *, symbolArr & );
!void newui_execute_query(symbolArr& results, const symbolArr& selection, const char* cmd_to_eval);
!
!//-----------------------------------------------------------------------------
!
!
!extern "C" char build_date[];
!
!extern "C" void galaxyStartTimer (void);
!extern "C" void galaxyStopTimer (void);
!extern void open_view (viewtype, symbolArr&);
!extern void ste_set_current_focus (steView*, steScreen*);
!extern bool filename_translate(char* name, bool to_logical,genString& translated_name);
!extern void delete_scratch_apps();
!extern StatInfo* StatisticsList[NUM_OF_STATS];
!extern char* PrefsFileName;
!extern Interpreter *interpreter_instance;
!extern void getAllFilters(genString&);
!extern int getFilterPhyname(char *, char *, genString&);
!
!class galaxyBoost {
!    public:
!	galaxyBoost (void) { galaxyStopTimer (); }
!	~galaxyBoost (void) { galaxyStartTimer (); }
!};
!
!//-----------------------------------------------------------------------------
!
!
!//-----------------------------------------------------------------------------
!
!int gala_current_layer;
!steScreen* gala_current_screen;
!dialog* gala_current_screen_dialog;
!int gala_current_screen_layer;
!int gala_current_viewer;
!int gala_current_view;
!int gala_current_window;
!
!//----------------------------------------------------------------------------
!BrowserController* GetBrowserController()
!{
!	return LanguageController::GetBrowserController();
!}
!//-----------------------------------------------------------------------------
!
!static void initSearchPath (void)
!{
!    char* pathlist = "/";
!    projList::domain_reset ();
!    projList::search_list->parse_names_init (pathlist);
!    projList::search_list->put_old_pathlist ((const char*) 0);
!    projList::search_list->domain_string ();
!}
!
!
! extern int dis_focus_node(Relational *);
! extern int dis_focus_instance(symbolPtr);
!
!//-----------------------------------------------------------------------------
!
!static void setProjScope (char *searchList)
!{
!    static genString oldPathList = "";
!
!    projList::domain_reset ();
!    projList::search_list->parse_names_init (searchList);
!    projList::search_list->put_old_pathlist (oldPathList.str());
!    projList::search_list->domain_string ();
!
!    oldPathList = searchList;
!}
!
!//-----------------------------------------------------------------------------
!
!extern Application* DISui_app;
!
!static void get_syms_for_metric_analysis(
!          	symbolArr & selected, symbolArr & domain, 
!		bool need[MI_num], int recursive) 
!{
!    Initialize(do_batch_metric_analysis);
!    
!    domain.removeAll();
!    symbolPtr sym;
!    ForEachS(sym, selected) {
!	switch (sym.get_kind()) {
!	  case DD_FUNC_DECL:
!	    if (need[MI_func]) domain.insert_last(sym);
!	    break;
!	  case DD_CLASS:
!	    if (need[MI_class]) domain.insert_last(sym);
!	    break;
!	  case DD_MODULE:
!	    if (recursive) {
!		symbolArr links;
!		sym->get_link((linkType)is_defining, links);
!		symbolPtr link;
!		ForEachS(link, links) {
!		    ddKind kind = link.get_kind();
!		    if (kind == DD_FUNC_DECL && need[MI_func] || 
!			kind == DD_CLASS && need[MI_class])
!		      domain.insert_last(link);
!		}
!	    }
!	    if (need[MI_file]) domain.insert_last(sym);
!	    break;
!	}
!    }
!    domain.remove_dup_syms();
!}
!#endif

@ int dis_connect_app (vstr *name)
{
    Initialize (dis_connect_DISui);
    galaxyBoost boost;

    if (strcmp((char*)name, "DISui") == 0) {
        if (DISui_app) {
	    fprintf (stderr, "Disconnecting from \"%s\"\n", (char*) name);
	    delete DISui_app;
        }
        {
	    const char* group = Application::getGroupName();
            DISui_app = new Application;
            if (DISui_app->connect (group, "DISui")) {
	        fprintf (stderr, "Cannot connect to service \"%s:DISui\"\n", group);
            } else {
                fprintf (stderr, "%s:DISmain connected to %s:DISui.\n", group, group);
	    }
        }

    }	
    return OSapi_getpid();
}

@ int dis_rtl_create (int a0)
{
    Initialize (dis_rtl_create);
    galaxyBoost boost;

    RTListServer* server = new RTListServer (a0);

    return server->id();
}

@ async dis_rtl_set_caller (int id, vstr* caller)
{
    RTListServer* server = RTListServer::find (id);
    if (!server) return;

    server->set_caller (caller);
}

@ async dis_rtl_init (int id, vstr* spec)
{
    Initialize (dis_rtl_init);
    galaxyBoost boost;

	if (!interpreter_instance)
		return;
    //----------
    // find list
    //----------

    RTListServer* server = RTListServer::find (id);
    if (!server)
	return;

    //-----------
    // clear list
    //-----------

    server->clear ();

	symbolArr result;
	symbolArr	subproj_arr;
	projNode	*pr;

	int i;
	for (i = 0; pr = projList::search_list->get_scan (i); i++)
	    subproj_arr.insert_last (pr);

	//----------------------
    // parse selection range
    //----------------------

	ParaCancel::set();

	genArrCharPtr cmd_arr;
	i = -1;
	//break out of the loop when next selected category name is empty	
	while (true)
	{
		//get next selected Category
		genString cmd;
		cmd.printf("lindex {%s} %d", spec, ++i);
		interpreter_instance->EvalCmd(cmd);
		genString CategoryName = Tcl_GetStringResult(interpreter_instance->interp);
		
		if (CategoryName.is_null() || CategoryName.length() == 0)
			break;
	
		CategoryName.trim();
		const char* command = GetBrowserController()->get_category_command(CategoryName);
		if (command && strlen(command) > 0)
		{
			cmd_arr.append((char**)&command);
		}		
	}

	int size = cmd_arr.size();
	genString* cmds = new genString[size];
	genString defines;
	ddSelector defines_dd;

	GetBrowserController()->process_commands(cmd_arr, cmds, defines, defines_dd);
	
	symbolArr temp;
	
	if (defines.not_null())
	{
		newui_execute_query(temp, subproj_arr, defines);
		result.insert_last(temp);
	}
	for (i = 0; i < size; ++i)
	{
		genString cmd = cmds[i];
		if (cmd.is_null())
			break;
		newui_execute_query(temp, subproj_arr, cmd);
		result.insert_last(temp);
	}
	
	delete [] cmds;

	ParaCancel::reset();
	
	result.remove_dup_syms();
	server->insert (result);
    
    return;
}

@async dis_rtl_clear (int id)
{
    Initialize (dis_rtl_clear);
    galaxyBoost boost;

    RTListServer* list = RTListServer::find (id);
    if (!list)
	return;

    list->clear ();

    return;
}

@ async dis_rtl_select (int id, vstr* spec)
{
    Initialize (dis_rtl_select);
    galaxyBoost boost;

    //----------
    // find list
    //----------

    RTListServer* server = RTListServer::find (id);
    if (!server)
	return;

    //--------------
    // set selection
    //--------------

	genMask selMask;
	SelectionIterator catSel((char*)spec);
	while (catSel.Next() ) 
		selMask += catSel.GetNext();

    if (!server->setSel (selMask) )
	return;

    return;
}

@ async dis_rtl_query_links (int src_id, vstr* spec, int trg_id)
{
    Initialize (dis_rtl_query);
    galaxyBoost boost;

    //-----------
    // find lists
    //-----------

	if (!interpreter_instance)
	return;

    RTListServer* src = RTListServer::find (src_id);
    if (!src)
	return;

    RTListServer* trg = RTListServer::find (trg_id);
    if (!trg)
	return;

    ParaCancel::set();
    symbolArr sel;
    src->getSelArr (sel);

    trg->clear ();

	symbolArr result;
	
	genArrCharPtr cmd_arr;
	int i = -1;
	while(true)
	{
		//get next selected Query
		genString cmd;
		cmd.printf("lindex {%s} %d", spec, ++i);
		interpreter_instance->EvalCmd(cmd);
		genString QueryName = Tcl_GetStringResult(interpreter_instance->interp);
		
		if (QueryName.is_null() || QueryName.length() == 0)
			break;

		QueryName.trim();
		const char* command = GetBrowserController()->get_query_command(QueryName);
		if (command && strlen(command) > 0)
		{
			cmd_arr.append((char**)&command);
		}
	}

	int size = cmd_arr.size();
	genString* cmds = new genString[size];
	genString defines;
	ddSelector defines_dd;

	GetBrowserController()->process_commands(cmd_arr, cmds, defines, defines_dd);
	
	symbolArr temp;
	genString cmd;

	if (defines.not_null())
	{
		GetBrowserController()->handle_closure(defines, cmd);
		newui_execute_query(temp, sel, cmd);
		result.insert_last(temp);
	}
	for (i = 0; i < size; ++i)
	{
		if (cmds[i].is_null())
			break;

		GetBrowserController()->handle_closure(cmds[i], cmd);
		newui_execute_query(temp, sel, cmd);
		result.insert_last(temp);
	}
	
	delete [] cmds;

	result.remove_dup_syms();
     
	trg->insert(result);

    ParaCancel::reset();

    return;
}

@ async dis_rtl_query_proj (int src_id, int pos, int trg_id)
{
    Initialize (dis_rtl_query_proj);
    galaxyBoost boost;

    //-----------
    // find lists
    //-----------

    RTListServer* src = RTListServer::find (src_id);
    if (!src)
        return;
 
    RTListServer* trg = RTListServer::find (trg_id);
    if (!trg)
        return;

    trg->queryProjects (src, pos);

    return;
}

@ async dis_rtl_query_top_proj (int trg_id)
{
    Initialize (dis_rtl_query_top_proj);
    galaxyBoost boost;

    //----------
    // find list
    //----------

    RTListServer* trg = RTListServer::find (trg_id);
    if (!trg)
        return;
 
    trg->queryProjects ();

    return;
}

@ async dis_rtl_query_parent_proj (int trg_id)
{
    Initialize (dis_rtl_query_parent_proj);
    galaxyBoost boost;
    //----------
    // find list
    //----------

    RTListServer* trg = RTListServer::find (trg_id);
    if (!trg)
        return;

    trg->queryParentProject ();

    return;
}

@ vstr* dis_rtl_get_title (int id)
{
    Initialize (dis_rtl_get_title);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server)
	return (vstr*) server->getTitle ();

    return 0;
}

@ vstr* dis_rtl_get_item (int id, int idx)
{
    Initialize (dis_rtl_get_item);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server)
        return (vstr*) server->getItem (idx);

    return 0;
}

@ int dis_rtl_get_unfilteredSize (int id)
{
    Initialize (dis_rtl_get_unfilteredSize);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server) return server->get_unfilteredSize();
    else return (0);
}

@ int dis_rtl_isFiltered (int id)
{
    Initialize (dis_rtl_isFiltered);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server) return server->isFiltered();
    else return (0);
}

@@ gc_put_access_result_into_rtl
{
    void GetSavedResult(symbolArr& arr);

    int id             = atoi(argv[1]);
    RTListServer *list = RTListServer::find(id);
    if(!list) return TCL_OK;
    symbolArr res_arr;
    GetSavedResult(res_arr);
    list->clear();
    list->insert( res_arr );

    return TCL_OK;
}

@@ gc_put_access_value_into_rtl
{
    void GetAccessValue(char *val, symbolArr& arr);

    int id             = atoi(argv[1]);
    char *val          = argv[2];
    RTListServer *list = RTListServer::find(id);
    if(!list) return TCL_OK;
    symbolArr res_arr;
    GetAccessValue(val, res_arr);
    list->clear();
    list->insert( res_arr );

    return TCL_OK;
}

@@ gc_rtl_propagate
{
    int id             = atoi(argv[1]);
    RTListServer *list = RTListServer::find(id);
    if(!list) return TCL_OK;
    list->_propagate();
    return TCL_OK;
}

@ int dis_rtl_get_size (int id)
{
    Initialize (dis_rtl_get_size);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server)
        return server->getSize ();

    return 0;
}

@ int dis_rtl_get_type (int id)
{
    Initialize (dis_rtl_get_type);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server)
        return server->getType ();

    return rtlUnknown;
}

@ int dis_rtl_destroy (int id)
{
    Initialize (dis_rtl_destroy);
    galaxyBoost boost;

    RTListServer* server = RTListServer::find (id);
    if (server) {
        delete server;
        return 0;
    }

    return 1;
}

@ int dis_rtl_edit_copy(int source, int target)
{
    Initialize (dis_rtl_paste);
    galaxyBoost boost;

    int ret_val = 1;

    RTListServer * source_rtl = RTListServer::find(source);
    RTListServer * target_rtl = RTListServer::find(target);
    if (source_rtl!=0 && target_rtl!=0) {
	symbolArr source_sel;
	source_rtl->getSelArr(source_sel);
	if (source_sel.size() > 0) {
	    target_rtl->clear();
	    target_rtl->insert(source_sel);
	    ret_val = 0;
	}
    }

    return ret_val;
}

@@ dis_rtl_copy_selected
{	
    if(argc != 3){
	Tcl_AppendResult(interp, argv[0], ": wrong number of arguments. Should be: source_rtl_id target_rtl_id", NULL);
	return TCL_ERROR;
    }
    int src = atoi(argv[1]);
    int trg = atoi(argv[2]);
    dis_rtl_edit_copy(src, trg);
    return TCL_OK;
}

@ int dis_rtl_add_to_exist_xref(int source_id, int target_id)
{
    Initialize (dis_rtl_add_to_exist_xref);

    int ret_val = 0;

    RTListServer * source_rtl = RTListServer::find(source_id);
    RTListServer * target_rtl = RTListServer::find(target_id);
    if (source_rtl!=0 && target_rtl!=0) 
    {
	    symbolArr source_sel;
	    source_rtl->getSelArr(source_sel);
    
        symbolArr tobe_inserted;
        //get selections from a global stack
        Obj *pObj;
	    OperPoint *pPoint;
	    ForEach( pObj, driver_instance->sel_list.oper_pts )
	    {
		    if( pObj )
		    {
			    pPoint = (OperPoint *)pObj;
			    symbolPtr sym = pPoint->node.get_xrefSymbol();
			    if( !sym.isnull() && sym.get_kind() != DD_SOFT_ASSOC  )
				    tobe_inserted.insert_last( sym );
		    }	
	    }
    
        symbolPtr x_sym;
        //get only xref symbols from selection
        ForEachS(x_sym, source_sel)
        {
            if( x_sym.is_xrefSymbol() && x_sym.get_kind() != DD_SOFT_ASSOC )
                tobe_inserted.insert_last( x_sym );
        }

	    symbolArr target ( target_rtl->getArr() );
        if (tobe_inserted.size() > 0) {
            ForEachS(x_sym, tobe_inserted) 
            {
                if (!target.includes(x_sym))
                {
                    ret_val++;
                    target.insert_last(x_sym);
                }
            }

	        if( ret_val > 0 )
            {
                target_rtl->clear();
	            target_rtl->insert(target);
            }
	    }
    }

    return ret_val;
}

@@ dis_rtl_add_selected_xref
{	
    if(argc != 3){
	Tcl_AppendResult(interp, argv[0], ": wrong number of arguments. Should be: source_rtl_id target_rtl_id", NULL);
	return TCL_ERROR;
    }
    int src = atoi(argv[1]);
    int trg = atoi(argv[2]);
    dis_rtl_add_to_exist_xref(src, trg);
    return TCL_OK;
}

@@ dis_assocbridge_init
{
    if(argc != 3){
	Tcl_AppendResult(interp, argv[0], ": wrong number of arguments. Should be: source_rtl_id target_rtl_id", NULL);
	return TCL_ERROR;
    }
    int rtl1 = atoi(argv[1]);
    int rtl2 = atoi(argv[2]);
    assocBridge::init( rtl1, rtl2 );
    return TCL_OK;
}

@@ dis_assocbridge_destroy
{
    assocBridge::destroy();
    return TCL_OK;
}

@@ dis_assocbridge_deleteassoc
{
    char result[11];
    int nRet = assocBridge::deleteAssoc();
    sprintf( result, "%d", nRet );
    Tcl_SetResult (interp, result, TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_assocbridge_add_extsel
{
    char result[11];
    int nRet = assocBridge::addExtSel();
    sprintf( result, "%d", nRet );
    Tcl_SetResult (interp, result, TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_assocbridge_apply
{
    char result[11];
    const char *pszName = NULL;
    if( argc > 1 )
        pszName = argv[1];
    int nRet = assocBridge::apply(pszName);
    sprintf( result, "%d", nRet );
    Tcl_SetResult (interp, result, TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_assocbridge_is_name_modif
{
    char result[11];
    int nRet = assocBridge::isNameModifiable();
    sprintf( result, "%d", nRet );
    Tcl_SetResult (interp, result, TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_assocbridge_get_name
{
    Tcl_SetResult (interp, (char *)assocBridge::getAssocName(), TCL_VOLATILE);
    return TCL_OK;
}


@ int dis_rtl_add_to_exist(int source_id, int target_id)
{
    Initialize (dis_rtl_add_to_exist);

    int ret_val = 0;

    RTListServer * source_rtl = RTListServer::find(source_id);
    RTListServer * target_rtl = RTListServer::find(target_id);
    if (source_rtl!=0 && target_rtl!=0) {
	symbolArr source_sel;
	source_rtl->getSelArr(source_sel);
	symbolArr target ( target_rtl->getArr() );
    if (source_sel.size() > 0) {
        symbolPtr x_sym;
        ForEachS(x_sym, source_sel) 
        {
            if (!target.includes(x_sym))
            {
                ret_val++;
                target.insert_last(x_sym);
            }
        }

	    if( ret_val > 0 )
        {
            target_rtl->clear();
	        target_rtl->insert(target);
        }
	}
    }

    return ret_val;
}

@@ dis_rtl_add_selected
{	
    if(argc != 3){
	Tcl_AppendResult(interp, argv[0], ": wrong number of arguments. Should be: source_rtl_id target_rtl_id", NULL);
	return TCL_ERROR;
    }
    int src = atoi(argv[1]);
    int trg = atoi(argv[2]);
    dis_rtl_add_to_exist(src, trg);
    return TCL_OK;
}

@ int dis_rtl_remove_selected(int source_id)
{
    Initialize (dis_rtl_remove_selected);

    int ret_val = 0;

    RTListServer * source_rtl = RTListServer::find(source_id);
    if (source_rtl!=0) {
	symbolArr source_sel;
	source_rtl->getSelArr(source_sel);
	symbolArr &source = source_rtl->getArr();
    symbolArr result;
    if (source_sel.size() > 0) {
        symbolPtr x_sym;
        ForEachS(x_sym, source) 
        {
            if (!source_sel.includes(x_sym))
                result.insert_last(x_sym);
            else
                ret_val++;
        }

        source_rtl->clear();
        
        if (result.size() > 0)
	        source_rtl->insert(result);
        else
            source_rtl->_propagate();
	}
    }

    return ret_val;
}


@@ dis_rtl_delete_selected
{	
    if(argc != 2){
	Tcl_AppendResult(interp, argv[0], ": wrong number of arguments. Should be: rtl_id", NULL);
	return TCL_ERROR;
    }
    int src = atoi(argv[1]);
    dis_rtl_remove_selected(src);
    return TCL_OK;
}

@ int dis_rtl_edit_paste(int source, int target)
{
    Initialize (dis_rtl_paste);
    galaxyBoost boost;

    int ret_val = 1;

    RTListServer * source_rtl = RTListServer::find(source);
    RTListServer * target_rtl = RTListServer::find(target);
    if (source_rtl!=0 && target_rtl!=0) {
	symbolArr & source_arr = source_rtl->getArr();
	if (source_arr.size() > 0) {
	    target_rtl->clear();
	    target_rtl->insert(source_arr);
	    ret_val = 0;
	}
    }

    return ret_val;
}

@ int dis_layer_create( int a0, int a1, int a2 )
{
    galaxyBoost boost;
    LayerServer* layers = new LayerServer (a0, a1, a2);

    return 1;
}

@ int dis_layer_destroy( int a0 )
{
    galaxyBoost boost;
    LayerServer* server = LayerServer::find_layer_server (a0);
    if (server) {
        delete server;
        return 0;
    }
    return 1;
}

@ async dis_set_layer( int a0 )
{
    Initialize (dis_view_open);
    galaxyBoost boost;
    
    ste_set_current_focus (NULL, NULL);
    LayerServer* server = LayerServer::find_layer_server (a0);
    if (server) {
        gala_current_layer = server->id();
        if (server->window()) gala_current_window = server->window();
    	if (server->screen()) {
            gala_current_screen_layer = server->id();
	    gala_current_screen_dialog = server->dlg();
            gala_current_screen = server->screen();
	}
    }
}

@ int dis_open_selection (int rtl_id, int rep_type  )
{
    Initialize (dis_view_open);
    galaxyBoost boost;
    
    RTListServer* server = RTListServer::find (rtl_id);
  
    if (!server) 
	return 1;
  
    symbolArr symbols;
    server->getSelArr (symbols);

    symbolPtr el;
    symbolArr files_to_open;
    ForEachS(el, symbols) {
        dis_view_create(el, (repType) rep_type, 0);
    }

    return 0;
}

@ vstr* server_eval (vstr* server, vstr* str)
{
    Application* app = Application::findApplication ((vchar*)server);
    vstr* result = NULL;
    if (app) {
        int e = app->eval((vchar*)str);
        result = app->getEvalResult();
    }

    return result;
}

@ async server_eval_async (vstr* server, vstr* str)
{
    Application* app = Application::findApplication ((vchar*)server);
    vstr* result = NULL;
    if (app) {
        app->eval((vchar*)str);
    }

    return;
}


@ async dis_show_file (vstr* name)
{
    dis_view_create ((char*)name);
    printf ("Showing %s\n", (char*) name);
}

@ int dis_shutdown_server()
{

	int answer = 1;
	// Get the list of modified objects.
	objArr* modified = app::get_modified_headers();

	if (modified->size() > 0) {
		int myAnswer = dis_confirm_save_objects(*modified);
	}

	if (answer > 0) {
		delete_scratch_apps();
		projHeader::save_all_projects();
	}

	// We can tell the client to go ahead and exit. There is a watchdog
	// timer that will notice that the client is no longer communicating
	// and quit the server after all reasonable processing has completed.

	return (answer);
}

@ int dis_viewer_selection_exists()
{
	return driver_instance->sel_list.oper_pts.size();
}

@ async dis_insert_selection_into_rtl( int rtl_id )
{
	RTListServer* server = RTListServer::find (rtl_id);
	if( server )
	{
		Obj *pObj;
		OperPoint *pPoint;
		server->clear();
    server->_propagate();
		int fInserted = 0;
		ForEach( pObj, driver_instance->sel_list.oper_pts )
		{
			if( pObj )
			{
				pPoint = (OperPoint *)pObj;
				symbolPtr sym = pPoint->node.get_xrefSymbol();
				if( !sym.isnull() )
				{
					server->insert( sym );
					fInserted = 1;
				}
			}	
		}
		if( !fInserted )
		{
      rcall_dis_DISui_eval_async (DISui_app, 
						(vstr *)"dis_prompt OKDialog DISCOVER [concat Current selection doesn't contain any symbols]");
		}
	}
}

@ async dis_open_selected_in_view()
{
	OperPoint *pPoint = driver_instance->sel_list.top_pt();
	if( pPoint )
	{
		symbolPtr sym = pPoint->node;
		sym = sym.get_xrefSymbol();
		if( !sym.isnull() )
			dis_create_view_of_def(sym);
		else
      rcall_dis_DISui_eval_async (DISui_app, 
				(vstr *)"dis_Message [concat Current selection doesn't contain any symbols]");
	}
	else
    rcall_dis_DISui_eval_async (DISui_app, 
			(vstr *)"dis_Message [concat There is no symbol selected to open definition for]");

}

@ int dis_create_package_input(int fUseScope, int rtl_id, char *pszScope, char *pszFile)
{
  int nRet = 0;
  symbolArr funcs;
  if( !fUseScope )
  {
    RTListServer* server = RTListServer::find( rtl_id );
    if( server )
      server->getSelArr( funcs );
  }
  else
  {
    genString cmd;
    cmd.printf("set_result [defines -func %s]", pszScope);
    if( interpreter_instance )
    {
      int nCode = interpreter_instance->EvalCmd((char *)cmd.str());
      if( nCode == TCL_OK )
        GetSavedResult( interpreter_instance, funcs );
    } 
  }
  FILE *out_file = OSapi_fopen(pszFile, "w" );
  if( out_file )
  {
    symbolPtr func_ptr;
    char kind_name[4];
    ForEachS(func_ptr, funcs)
    {
      nRet++;
      symbolPtr mod = func_ptr->get_def_file();
      OSapi_strncpy( kind_name, ddKind_name(func_ptr.get_kind()) + 3, 3 );
      kind_name[3] = '\0'; 
      if (mod.isnull())
        OSapi_fprintf(out_file,TXT("  %s: %s [ ]\n"),
          kind_name, func_ptr.get_name());
      else
        OSapi_fprintf(out_file,TXT("  %s: %s [%s]\n"),
          kind_name, func_ptr.get_name(), mod.get_name());
    }
    OSapi_fclose( out_file );
  } 
  return nRet;
}

@@ dis_view
{
  LayerServer* server =  LayerServer::find_layer_server (gala_current_layer);
    if (server) 
	{
    	view* v = LayerServer::get_current_view_of_screen(server->screen());
		symbolArr array;

		int rtl_id = -1;
		if (argv[2] && strcmp (argv[2], "-rtl") == 0) 
		{
			rtl_id = atoi(argv[3]);
		
		   RTListServer* rtl_server = RTListServer::find(rtl_id);
			if (rtl_server) 
			{
				rtl_server->getSelArr (array);
			}
		} 
		else if (v) 
		{
			driver_instance->update_selection(v, 1);
			driver_instance->fill_array_with_selected_appnodes(NULL, array, 1);
		}	 

		int selected = array.size();
	
		if (strcmp(argv[1], "-assistline") == 0) 
		{
			if(selected) 
			{
				symbolPtr sym = array[selected-1];
				genString result;
				char *selectedname = sym.get_name();
				Tcl_SetResult (interp, (char *)selectedname, TCL_VOLATILE);
			}
		}
		else if (selected)
		{
			for(int i = 0; i < selected; i++) 
			{
				symbolPtr sym = array[i];
			
				//try user defined action first
				int kind = sym.get_kind();
				const char* cmd = LanguageController::GetBrowserData()->get_action_cmd(kind);
				if (cmd && (strcmp(argv[1], "-definition") == 0 || strcmp(argv[1], "-text") == 0))
				{
					if (strlen(cmd) > 0)
					{
						symbolArr sel, dummy;
						sel.insert_last(sym);
						newui_execute_query(dummy, sel, cmd);
					}
				}
				else 
				{
					dis_open_view(sym, argv[1]);
				}
			}
		}
		else if (v) 
		{
			OperPoint* pt = v->insertion_point();
			if(pt  &&  !pt->node.isnull())
			{
				symbolPtr sym = pt->node;
				dis_create_view_of_def(sym);
			}
			delete pt;
		}
	}
    return TCL_OK;
}

@@ dis_get_token_info
{
    //$ideHandle $tab $col $sel $fname $line"

    if (argc != 7) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

// The last argument means "add selection to selection list" (if it is not zero)
    return dis_select (argv[5], atoi(argv[6]), atoi(argv[3]), atoi(argv[2]), argv[4], 0);
}

@@ dis_cm_event
{
	//$ideHandle $eventName $fname_list
    if (argc != 4) {
	Tcl_SetResult (interp, "wrong number of arguments", TCL_STATIC);
	return TCL_ERROR;
    }

	return dis_cm_event_internal (argv[2], argv[3]);
}

@@ dis_IDE_command_event
{
    //$ideHandle $back $wv $cmd $arg 
    return dis_ide_command_event(argc, argv ); 
}
 

@@ dis_pref_get_variable
{
    // {-string, -int, -bool} $KeyName
    if(strcmp(argv[1], "-string") == 0) {
        char* pref = customize::getStrPref(argv[2], false);
        Tcl_SetResult (interp, pref, TCL_VOLATILE);

    } else if(strcmp(argv[1], "-int") == 0) {
        const int pref = customize::getIntPref(argv[2]);
	genString tmp;
	tmp.printf("%d", pref);
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    } else {
        Tcl_SetResult (interp, "invalid parameter", TCL_STATIC);
        return TCL_ERROR;
    }

    return TCL_OK;
}

        
@@ dis_pref_put_variable
{
    // {-string, -int, -bool} $KeyName $Value
   if(strcmp(argv[1], "-string") == 0) {
      genString value = argv[3];
      customize::putPref(argv[2], value, false);

   } else if(strcmp(argv[1], "-int") == 0) {
      int value = atoi(argv[3]);
      customize::putPref(argv[2], value);

   } else {
      Tcl_SetResult (interp, "invalid parameter", TCL_STATIC);
   }
       
   return TCL_OK;
}

@@ dis_pref_get_folder
{
   if(argc != 4) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_get_folder requires 3 args instead of %d",
		argc-1);
   } else if (strcmp(argv[1], "-getList") == 0) {
       int viewId = atoi(argv[2]);
       vchar* FolderName = (vchar*) argv[3];
       GetFolder(viewId, FolderName);
 
    } else {
        dis_message(NULL, MSG_ERROR,
		"Preference folder '%s' not found.", argv[1]);
    }
 
    return TCL_OK;
}

@@ dis_pref_get_values
{
   if(argc != 4) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_get_values requires 3 args instead of %d",
		argc-1);
   } else {
       char* SelectedName = argv[1];
       int viewId = atoi(argv[2]);
       vchar* Valuelist = (vchar*) argv[3];
       GetValues(SelectedName, viewId, Valuelist);
   } 
 
    return TCL_OK;
}

@@ dis_pref_get_engname
{
   char ename[1024];
   if(argc != 5) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_get_engname requires 4 args instead of %d",
		argc-1);
   } else {
       char *SelectedName = argv[1];
       GetEname(SelectedName, ename);
       genString command;
       command.printf ("dis_setvar %s %s %s {%s}", argv[2], argv[3],
                          argv[4], ename);
       rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());

//       Tcl_SetResult (interp, ename, TCL_VOLATILE);
   }
   return TCL_OK;
}

@@ dis_pref_get_type
{
   char ename[1024];
   int type;
   if(argc != 6) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_get_type requires 5 args instead of %d",
		argc-1);
   } else {
       char *Foldname = argv[1];
       char *SelectedName = argv[2];
       GetEname(SelectedName, ename);
       type = GetType(Foldname, ename);
       char typestr[10];
       sprintf(typestr, "%d", type);
       genString command;
       command.printf ("dis_setvar %s %s %s {%s}", argv[3], argv[4], argv[5], typestr);
       rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());

//       sprintf(interp->result, "%d", type);
   }
   return TCL_OK;
}

@@ dis_pref_get_selectedvalue
{
   char retval[1024];
   if(argc != 5) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_get_selectedvalue requires 4 args instead of %d",
		argc-1);
   } else {
       char *SelectedName = argv[1];
       GetSelectedValue(SelectedName, retval);
       genString command;
       command.printf ("dis_setvar %s %s %s {%s}", argv[2], argv[3],
                           argv[4], retval);
       rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
 
//       Tcl_SetResult (interp, retval, TCL_VOLATILE);
   }
   return TCL_OK;
}

@@ dis_pref_change_prefvalue
{
   // parameters: { -string, -int } $folderengname $prefengname $value

   char keyname[1024];
   if(argc != 5) {
       dis_message(NULL, MSG_ERROR,
		"dis_pref_change_prefvalue requires 4 args instead of %d",
		argc-1);
   } else {
       char *engname = argv[3];
       GetKeyname(argv[2], engname, keyname);

       if((strcmp(argv[1], "-string") == 0) && 
          (strcmp(keyname, "DIS_misc.MiniBrowserHistory") != 0)) {
          genString value = argv[4];
          customize::putPref(keyname, value, false);
 
       } else {  
          int value = atoi(argv[4]);
          customize::putPref(keyname, value);
       }
   } 
   return TCL_OK;
}

@@ dis_StatusDialogOK
{
#ifdef NEW_UI
    int dialogId = atoi (argv[1]);
    if (!dialogId) return TCL_ERROR;
 
    StatusDialog* sd = StatusDialog::FindDialog (dialogId);
    if (!sd) return TCL_ERROR;
 
    sd->ok_cb();
#endif
    return TCL_OK;
}
 
@@ dis_StatusDialogCancel
{
#ifdef NEW_UI
    int dialogId = atoi (argv[1]);
    if (!dialogId) return TCL_ERROR;
 
    StatusDialog* sd = StatusDialog::FindDialog (dialogId);
    if (!sd) return TCL_ERROR;
 
    sd->cancel_cb();
#endif
    return TCL_OK;
}
 
!#include "gglobalFuncs.h"
!#undef _view_h
!#define Window Window_
!#define Display Display_
!#include "gview.h"
!#include "gviewer.h"

@@ dis_setProjectScopeFromRTL
{
    if (argc < 4)
	return TCL_ERROR;
    RTListServer* server = RTListServer::find (atoi (argv[3]));
    if (!server) return TCL_ERROR;
 
    symbolArr symbols;
    server->getSelArr (symbols);
 
    genString selString;
    symbolPtr sym;
 
 
    ForEachS (sym, symbols) {
        if (sym.isnotnull() && sym.get_kind() == DD_PROJECT) {
            char * nm = sym.get_name();
            if (nm) {
               selString = nm;
               break;
            }
        }
    }
 
    if (!selString.length()) return TCL_OK;
 
    genString command;
    command.printf ("dis_setvar %s %s {%s}", argv[1], argv[2], selString.str());

    if (server->get_caller() == 1) {
	Viewer* vr;
	View *v;	
	Viewer::GetExecutingView (vr, v);
	if (vr && v) {
	    Tcl_Interp *interp = v->interp();
	    gala_eval (interp, (char *)command.str());
	}
    }

    else {
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
    }
 
    return TCL_OK;
}
 
@@ dis_setProjectScope
{
    if (argc < 4)
	return TCL_ERROR;

    setProjScope (argv[1]);

    if(!argv[2] || atoi(argv[2])){
       return TCL_OK;
    }

    RTListServer* server = RTListServer::find (atoi (argv[3]));
    if (!server) return TCL_ERROR;

    // This needs to change in next release. Now the project browser
    // only shows the contents of the first one if multiple projects
    // are input.
/*
    char *ptr = argv[1];
    while ( ptr[0]!='\0' && !(isspace(ptr[0])) )
	ptr++;
    *ptr = '\0';
*/
    if(!strcmp(argv[1], "/")) {
        server->queryProjects ();
        return TCL_OK;
    }

    int ret;
    server->queryProjects(argv[1], &ret);

    if(!ret)
        dis_message(NULL, MSG_ERROR,
		"Could not find project '%s'", argv[1]);

    return TCL_OK;
}
 
@@ dis_setClosure
{
    int status = TCL_OK;
    if (argc != 2) {
	dis_message(NULL, MSG_ERROR, 
		"dis_setClosure requires 1 arg, not %d",
		argc-1);
        status = TCL_ERROR;
    } else {
        int closure = atoi(argv[1]);
        set_closure_state(closure);
    }
    return status;
}
 
@@ dis_getClosure
{
    int status = TCL_OK;
    if (argc != 3) {
        printf("Usage:dis_getClosure varId varName\n");
	dis_message(NULL, MSG_ERROR, 
		"dis_getClosure requires 2 args instead of %d",
		argc-1);
        status = TCL_ERROR;
    } else {
        int closure = get_closure_state();
        genString cmd;
        cmd.printf("dis_setvar %s %s {%d}", argv[1], argv[2], closure);
        rcall_dis_DISui_eval_async(DISui_app, (vstr *)cmd.str() );
    }
    return status;
}


@@ dis_SetStatusInfo
{
   RTListServer* server = RTListServer::find (atoi (argv[1]));
   if (!server) return TCL_ERROR;  

   symbolArr symbols;
   server->getSelArr (symbols);
 
   int selected = symbols.size();

   if(selected) {
      symbolPtr sym = symbols[selected-1];
      genString info_msg;
      if(sym.without_xrefp())
		 get_display_string_from_symbol(sym, info_msg);
      else {
         fsymbolPtr xsym = sym.get_xrefSymbol();
         if (xsym.xrisnotnull())
			get_display_string_from_symbol(xsym, info_msg);
      }
      if (info_msg.length()) {
          genString command;
          command.printf ("dis_Message {%s}", info_msg.str());
          rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
      }
   }

   return TCL_OK;
}

@ int dis_rtl_processKey (int id, vstr* thekey, int mod, int startat)
{
    Initialize (dis_rtl_processKey);
    galaxyBoost boost;
    
    RTListServer* server = RTListServer::find (id);
    if (!server)
        return -1;
   
    symbolArr symbols = server->getArr();
    symbols.sort();
    symbolPtr sym;

    int size = symbols.size();
    if(size <= 0)  return -1;

    char key = (char)*thekey;

    static int current = 0;
    static char buffer[1024] = "";
    
    static char searchmode = ' ';

    if(mod == 0) {
       if(startat >=0 ) {
          strcpy(buffer, "");
          searchmode = ' ';
          current = startat;
       }

       if(strcmp(buffer,"")==0)
          strcpy(buffer, (char *)thekey);
       else
          strcat(buffer, (char *)thekey);

       if(searchmode == '>') {
          current = size-1;
          for(int i = size-1; i >= 0; i--) {
             sym = symbols[i];
             if (sym.isnotnull()) {
                char * nm = sym.get_name();
                if (strstr(nm, buffer)) {
                   return current;
                }
             }
             current--;
          }  
       } else {
          if(searchmode == '<')
             current = 0;
          
           for(int i = current; i < size; i++) {
             sym = symbols[i];
             if (sym.isnotnull()) {
                char * nm = sym.get_name();
                if (strstr(nm, buffer)) {
                   return current;
                }
             }
             current++;
          }
       } 
    } else if(mod == 1) {              // CTRL is pressed

       if((key == 's') || (key == 'S')) {        // search forward
          for(int i = current+1; i < size; i++) {
             current++;
             sym = symbols[i];
             if (sym.isnotnull()) {
                char * nm = sym.get_name();
                if (strstr(nm, buffer)) 
                   return current;
             }
          }
       } 
       if((key == 'r') || (key == 'R')) {       // search backward
          for(int i = current-1; i >= 0; i--) {
             current--;
             sym = symbols[i];
             if (sym.isnotnull()) {
                char * nm = sym.get_name();
                if (strstr(nm, buffer)) 
                   return current;
             }
          }
       } 
    } else if(mod ==2) {             // ALT + SHIFT + '<' or '>'
       if(key == '<') {
          searchmode = '<';
          strcpy(buffer, "");
          return 0;
       }
       if(key == '>') {
          searchmode = '>';
          strcpy(buffer, "");
          return size-1;
       }
    }     

    return -1;   
}


@@ dis_partition
{
    char *cond = argv[1];

    if((strcmp(cond, "-physical")==0)||(strcmp(cond,"-logical")==0)) {
        int to_logical;
        if(strcmp(cond, "-physical")==0)
          to_logical = 0;
        else
          to_logical = 1;
        genString target_name;
        filename_translate(argv[2], to_logical, target_name);
        genString command;
        command.printf ("dis_setvar %s %s %s {%s}", argv[3], argv[4],
                          argv[5], target_name.str());
        rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
        return TCL_OK;
    }

    RTListServer* EntitiesServer = RTListServer::find(atoi(argv[2]));

    if(!EntitiesServer)
       return TCL_ERROR;

    symbolArr entities;
    EntitiesServer->getSelArr(entities);
    int selected = entities.size(); 

    if(strcmp(cond, "-clear")==0) {
        symbolArr  result;
        EntitiesServer->clear();
        EntitiesServer->insert(result);
    } else if(strcmp(cond, "-related")==0) {
        RTListServer* ResultsServer = RTListServer::find(atoi(argv[3]));
        ResultsServer->clear();

        symbolArr  results;
        get_all_uses(entities, results);
        ResultsServer->insert(results);
    } else if((strcmp(cond, "-check")==0) || (strcmp(cond, "-move")==0)) {
        RTListServer* NeedEditServer = RTListServer::find(atoi(argv[3]));
        RTListServer* NeedDeclServer = RTListServer::find(atoi(argv[4]));
        NeedDeclServer->clear();
        NeedEditServer->clear();

        symbolArr need_decl, need_edit;

        if(strcmp(cond, "-check")==0) {
           Part_Check_Conflicts_UI(entities, need_decl, need_edit);
        } else if(strcmp(cond, "-move")==0) {
           genString filenm = argv[5];
           int filetype = atoi(argv[6]);
           RTListServer* FilesNeedFetchServer = RTListServer::find(atoi(argv[7]));
           FilesNeedFetchServer->clear();

           symbolArr files_to_fetch;
           do_move_entities_ask_file_new(entities, filenm, filetype, need_decl, need_edit, files_to_fetch);
           if(files_to_fetch.size() != 0) {
              FilesNeedFetchServer->insert(files_to_fetch);
              genString command;
              command.printf ("dis_Message [concat %s]", "Files are not checkout, please checkout first.");
              rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
              return TCL_OK;
           }
        }

        NeedDeclServer->insert(need_decl);
        NeedEditServer->insert(need_edit);
    }

    return TCL_OK;
}

@@ dis_SimplifyIncludes
{
   RTListServer* server = RTListServer::find (atoi (argv[1]));
   if (!server) return TCL_ERROR;  

   symbolArr symbols;
   server->getSelArr (symbols);

   dis_message(NULL, MSG_INFORM, "Simplify/sw started ...");
   simplify_headers (symbols);   
   dis_message(NULL, MSG_INFORM, "Simplify/sw completed.");
   
   return TCL_OK;
}

@@ dis_ModuleSave
{
   int success = 0;

   symbolArr symbols;
   getSelection (atoi(argv[1]), symbols);

   objArr app_array(symbols);

   for (int i = 0; i < app_array.size(); i++) {
      appPtr ap = NULL;
      Relational *ob = (Relational *)app_array[i];
      if (!ob) continue;

      if(is_projModule(ob)) {
          projModule *pj = (projModule *)ob;
          genString fn;
          pj->get_phys_filename (fn);
          ap = app::get_header(App_RAW, fn);
          if (!ap) ap = pj->get_app();
      }

      else if(is_app(ob))   ap = (appPtr)ob;

      if(ap)
	::db_save(ap, NULL);
   }
   return TCL_OK;
}


@@ dis_moduleUpdate
{
   int success = 0;

   symbolArr symbols;
   getSelection (atoi(argv[1]), symbols);

   update_selection (symbols);
   return TCL_OK;
}


@@ dis_moduleLoad
{
  symbolArr symbols;
  getSelection (atoi(argv[1]), symbols);

// Need to reimplement the function get_module_from_view that is found in viewerShell.h.C and call it
// here.

  preload_selection (symbols); 
  return TCL_OK;
}


@@ dis_moduleUnload
{
  symbolArr symbols;
  getSelection (atoi(argv[1]), symbols);

// Need to reimplement the function get_module_from_view that is found in viewerShell.h.C and call it
// here.

  unload_selection (symbols);
  return TCL_OK;
}


@@ dis_set_homeproj
{
   RTListServer* server = RTListServer::find (atoi (argv[1]));
   if (!server) return TCL_ERROR;

   symbolArr sels;
   server->getSelArr (sels);

   setup_home_from_selection(sels);

   if(sels.size() == 1) {
      symbolPtr sym = sels[0];
      projNode* proj = projNode::find_project(sym.get_name());
      proj->refresh_projects();
   }

   return TCL_OK;
}     


@@ dis_projscope_startup
{
   RTListServer* visible = RTListServer::find (atoi (argv[1]));
   RTListServer* hidden = RTListServer::find (atoi (argv[2]));
   if (!visible || !hidden) return TCL_ERROR;

   symbolArr visibleprojs, hiddenprojs;
   load_projlists(visibleprojs, hiddenprojs);

   visible->clear();
   visible->insert(visibleprojs);
   hidden->clear();
   hidden->insert(hiddenprojs);

   return TCL_OK;
}

@@ dis_projscope_transfer
{
   int allflag = atoi(argv[1]);

   RTListServer* source = RTListServer::find (atoi (argv[2]));
   RTListServer* target = RTListServer::find (atoi (argv[3]));
   if (!source || !target) return TCL_ERROR;

   symbolArr s_projs, t_projs, sel_projs;

   s_projs = source->getArr();
   if(!s_projs.size())
       return TCL_OK;

   if (allflag) {
       sel_projs = source->getArr();
       s_projs.removeAll();
   } else {
       source->getSelArr(sel_projs);
       if(!sel_projs.size()) {
           dis_message(NULL, MSG_WARN,
		"No projects were selected.");
           return TCL_OK;
       }
       symbolArr s_copy;
       s_copy = source->getArr();
       symbolPtr  s1, s2;
       ForEachS(s1, s_copy) {
          ForEachS(s2, sel_projs) {
	     if(!strcmp(s1.get_name(), s2.get_name())) {
   		s_projs.remove(s1);
		break;
	     }
          }
       }
   }

       
   t_projs = target->getArr();
   symbolPtr  sym;
   ForEachS(sym, sel_projs) {
      if(!t_projs.includes(sym))
         t_projs.insert_last(sym);
   }
   	       
   source->clear();
   source->insert(s_projs);
   target->clear();
   target->insert(t_projs);

   return TCL_OK;
}


@@ dis_projscope_ok
{
   RTListServer* visible = RTListServer::find (atoi (argv[1]));
   if (!visible) return TCL_ERROR;

   symbolArr visible_projs;

   visible_projs = visible->getArr();
   unload_projlists(visible_projs);

   symbolArr allprojs;
   projList::search_list->get_search_list(allprojs);
   symbolPtr sym = allprojs[0];
   projNode* proj = projNode::find_project(sym.get_name());
   proj->refresh_projects();

   return TCL_OK;
}

@@ dis_version_info
{
    if (interp) {
	Tcl_SetResult(interp, (char *)build_date, TCL_VOLATILE);
    }

    return TCL_OK;
}

@@ dis_run_stats_report
{
  dis_message(NULL, MSG_INFORM, "Summary/sw Report started .....");
  Interpreter* cli_interp = Interpreter::Create();
  if( cli_interp ) //use global Access interpreter unstance
  {
    genString fname = "..";
    char *home = OSapi_getenv("PSETHOME");
    if( home )
      fname  = home;
    fname += "/lib/discover.summary.dis";
    if( argc > NUM_OF_STATS )
    {
      statistics_list_init();
      for (int i = 0; i < NUM_OF_STATS; i++) 
      {
		    if (StatisticsList[i]->type != OPTION_STAT) 
          Tcl_SetVar( cli_interp->interp, StatisticsList[i]->string, argv[i+1], TCL_GLOBAL_ONLY );
		    else 
        {
			    switch (*(argv[i+1])) 
          {
			      case 'A' :
              Tcl_SetVar( cli_interp->interp, StatisticsList[i]->string, "STAT_ASCII_FILE", TCL_GLOBAL_ONLY );
				    break;
			      case 'H' :
              Tcl_SetVar( cli_interp->interp, StatisticsList[i]->string, "STAT_HTML_FILE", TCL_GLOBAL_ONLY );
				    break;
			      case 'R' :
              Tcl_SetVar( cli_interp->interp, StatisticsList[i]->string, "STAT_RTF_FILE", TCL_GLOBAL_ONLY );
				    break;
			      default :
              Tcl_SetVar( cli_interp->interp, StatisticsList[i]->string, "STAT_PRINTER", TCL_GLOBAL_ONLY );
			    }
		    }
	    }
	  cli_interp->SetOutputStream(cerr);
      int ret_code = cli_interp->EvalFile(OSPATH(fname));
      if (ret_code == TCL_OK) 
        dis_message(NULL, MSG_INFORM, "Summary/sw Report completed.");
      else 
        dis_message(NULL, MSG_ERROR, "Summary/sw Report could not be generated.");

      //Clean up variables we just added (just in case if we change this to use global interp)
      for (i = 0; i < NUM_OF_STATS; i++)
        Tcl_UnsetVar( cli_interp->interp, StatisticsList[i]->string, TCL_GLOBAL_ONLY );
    }
    else
      dis_message(NULL, MSG_ERROR, "Summary/sw Report could not be generated. Not enough arguments.");

    
    //Memory Leak!!!!!!!!!!!!!
    //We have to delete cli_interp instance here.
    //When Interpreter's destructor is implemented, uncomment the following line:
    //delete cli_interp;
  
  }
  else
  {
    dis_message(NULL, MSG_ERROR, "Summary/sw Report could not be generated.");
    dis_message(NULL, MSG_ERROR, "Could not create Access/sw Interpreter instance.");
  }
  return TCL_OK;
}

@@ dis_open_view 
{

    symbolArr symbols;
    getSelection (atoi (argv[1]), symbols);

    viewtype vt;
    if(strcmp(argv[2], "Text") == 0) {
    	OperPoint *pPoint = driver_instance->sel_list.top_pt();
	    if( pPoint )
	    {
		    symbolPtr sym = pPoint->node;
		    sym           = sym.get_xrefSymbol();
		    if( !sym.isnull() )
			    dis_create_view_of_def(sym);
		    else
          rcall_dis_DISui_eval_async (DISui_app, 
				    (vstr *)"dis_Message [concat Current selection doesn't contain any symbols]");
	    }
	    else
        rcall_dis_DISui_eval_async (DISui_app, 
			    (vstr *)"dis_Message [concat There is no symbol selected to open definition for]");
      return TCL_OK;
    }
    if (strcmp (argv[2], "Calltree") == 0) 		vt = viewCallTree;
    else if (strcmp (argv[2], "Flowchart") == 0) 	vt = viewFlowchart;
    else if (strcmp (argv[2], "Outline") == 0)		vt = viewOutline;
    else if (strcmp (argv[2], "Inheritance") == 0)	vt = viewInheritance;
    else if (strcmp (argv[2], "Datachart") == 0) 	vt = viewDatachart;
    else if (strcmp (argv[2], "Graph") == 0) 		vt = viewGraph;
    else if (strcmp (argv[2], "Relations") == 0) 	vt = viewERD;
    else if (strcmp (argv[2], "Subsystem") == 0) 	vt = viewSubsysBrowser;

    open_view (vt, symbols);

    return TCL_OK;
}


@@ dis_metrics_get_num_metrics
{
    genString tmp;
    tmp.printf("%d", NUM_OF_METRICS);
    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    return TCL_OK;
}

@@ dis_metrics_get_metric_name
{
    if (argc == 2) {
        int i = atoi (argv[1]);
	if ((i>=0) || (i<NUM_OF_METRICS)) {
	    Tcl_SetResult(interp, (char *)Metric::definitions[i].name, TCL_VOLATILE);
            return TCL_OK;
	}
    }
    return TCL_ERROR;
}

@@ dis_metrics_get_metric
{
    if (argc == 2) {
        for (int i=0; i<NUM_OF_METRICS; i++) {
	    if (strcmp (argv[1], Metric::definitions[i].name) == 0) {
	       genString tmp;
       	       tmp.printf("%d", i);
               Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
	       return TCL_OK;
	    }
	}
    }
    return TCL_ERROR;
}

@@ dis_metrics_query
{
    if (argc != 9)
		return TCL_ERROR;
	
    RTListServer* selectionRTL = RTListServer::find (atoi(argv[6]));
    RTListServer* resultRTL = RTListServer::find (atoi(argv[7]));
	
    if (!selectionRTL && !resultRTL)
		return TCL_ERROR;
	
    resultRTL->clear();
	
    symbolArr selected;
    selected.insert_last (selectionRTL->getArr());
	
    if (selected.empty()) {
		return TCL_ERROR;
    }
	
    int type = Metric::definitions[atoi(argv[2])].type;
    bool need[MI_num];
	
    need[MI_file] = Metric::need_to_compute_def(MI_file, type);
    need[MI_func] = Metric::need_to_compute_def(MI_func, type);
    need[MI_class] = Metric::need_to_compute_def(MI_class, type);
	
    int recursive = atoi(argv[3]);
	
    if ( ! (need[MI_file] || need[MI_func] || need[MI_class])) {
        dis_message (NULL, MSG_INFORM, "All Entity Types Excluded");
		return TCL_ERROR;
    }
	
    genString lowstr = argv[4];
    genString highstr = argv[5];
    lowstr += "-1";
    highstr += "-1";
    int low = atoi(lowstr);
    int high = atoi(highstr);
    double low_f = atof(lowstr);
    double high_f = atof(highstr);
    
    if (low < 0 && high < 0) {
		dis_message (NULL, MSG_INFORM, "Limits Not Specified");
		return TCL_ERROR;
    }
	
    if (low >= 0 && high >= 0 && low >= high) {
		dis_message (NULL, MSG_INFORM, "Limits Inconsistent");
		return TCL_ERROR;
    }

    needLoad temp_needLoad(false);   // When computing a metric, don't load files.
	
    symbolArr domain;
    get_syms_for_metric_analysis(selected, domain, need, recursive);
    if (domain.empty()) {
		dis_message (NULL, MSG_INFORM, "No Entities of Selected Types");
		return TCL_ERROR;
    }
	
    symbolArr result;
    int no_ent = domain.size();
    
    genString txt;
    txt.printf(TXT("Starting analysis of %d entities."), no_ent);
	dis_message (NULL, MSG_INFORM, (char*)txt);
    
	int       mod    = no_ent < 100 ? 1 : no_ent / 100;
	int       cnt    = 0;
    bool      is_int = Metric::def_array[type]->type_is_int;
    symbolPtr dsym;
	
    ForEachS(dsym, domain) {
		if (ParaCancel::is_cancelled()) {
            dis_message (NULL, MSG_INFORM, "Cancelled.");
			return TCL_ERROR;
		}
		if (is_int) {
			int val;
			Metric::get_metric((metricType)type, dsym, &val);
			if (val >= 0 && (low < 0 || val > low) && (high < 0 || val < high))
				result.insert_last(dsym);
		} else {
			double val;
			Metric::get_metric((metricType)type, dsym, &val);
			if (val >= 0 && (low_f < 0 || val > low_f) && (high_f < 0 || val < high_f))
				result.insert_last(dsym);
		}
		cnt++;

		if ( mod == 1 || cnt % mod == 0 || cnt == 0 || cnt == no_ent )
		{
			txt.printf(TXT("%d%%%% complete."), (cnt * 100) / no_ent);
			dis_message (NULL, MSG_INFORM, (char*)txt);
		}
		vevent::FlushProcessing();
    }
	
    resultRTL->insert (result);
    	
    //Update RTL format to show the criteria
    
    genString sort, format, show, hide;
    genString metric_filter = "met_";
    metric_filter += Metric::definitions[atoi(argv[2])].abbrev;
    
	resultRTL->getFilter( sort, format, show, hide );

    if( !strstr( format, metric_filter ) )
    {
		if( format.is_null() || !format.length() )
			format = "%on,";
		format += " %";
		format += metric_filter;
		format += ',';

		resultRTL->setFilter( (char  *)sort.str(), (char *)format.str(), (char *)show.str(), (char *)hide.str() );
    }
    
    return TCL_OK;
}

@@ dis_metrics_init
{
    RTListServer* l1 = RTListServer::find (atoi(argv[1]));
    RTListServer* l2 = RTListServer::find (atoi(argv[2]));
 
    if (l1 && l2) {
        symbolArr sel;
        l2->getSelArr(sel);

        l1->clear();
        l1->insert (sel);
 
        return TCL_OK;
    }

    return TCL_ERROR;
}


@@ dis_parser_get_error_filename
{
    const char* filename = get_parser_error_filename();
    Tcl_SetResult (interp, (char*) filename, TCL_VOLATILE);
    return TCL_OK;
}

@ async cli_process_shell_request(char* in, int id, char* tag)
{
  static   ocharstream result;
  cli_process_input(in, result, NULL, 1);
  result << ends;
  char * ostring = result.ptr();
  Application *appl = Application::findApplication("DISui");
  if( appl )
    rcall_access_shell_set_result (appl, id, tag, ostring);
  result.reset();
}

@ int dis_access_eval (char *cmd, output char *&out)
{
  out = NULL;
  int nRes = TCL_ERROR;
  if( interpreter_instance )
  {
    if (Application::traceMode)
      printf ("Evaluating command : %s\n", cmd);
    nRes = interpreter_instance->EvalCmd( cmd );
    out  = Tcl_GetStringResult(interpreter_instance->interp);
  }
  /* NOTE: because demarshall__charptr uses strdup, we have to free the memory (YUCK!!!!)
	 This really needs to be fixed in galgen, but until then...
	 */
	free(cmd);
	cmd=NULL;
  return nRes;
}

@ async dis_access_eval_async(char *str)
{
  if( interpreter_instance )
    {
      if (Application::traceMode)
	printf ("Evaluating command : %s\n", str);
      interpreter_instance->EvalCmd(str);
    }
	/*SEE previous comment*/
	free(str);
	str=NULL;
}

@@ dis_get_filterlist
{
    if(argc != 1)
       return TCL_ERROR;

    genString filterlist;
    getAllFilters(filterlist);

    Tcl_SetResult (interp, (char*) filterlist.str(), TCL_VOLATILE);
    return TCL_OK;
}


@@ dis_get_filtername
{
    if(argc != 3 || (OSapi_strcmp(argv[1], "-save")!=0 && OSapi_strcmp(argv[1], "-load")!=0))
       return TCL_ERROR;

    char type[1024];
    if(strcmp(argv[1], "-load")==0) {
       if(strcmp(argv[2], "Default")==0) {
          char filename[1024];
          char *path = OSapi_getenv("PSETHOME");
          if(!path)
             return TCL_ERROR;
          strcpy(filename, path);
          strcat(filename, "\\lib\\Default.flt");
          Tcl_SetResult (interp, filename, TCL_VOLATILE);
          return TCL_OK;
       } else
          OSapi_strcpy(type, "load");
    } else
       OSapi_strcpy(type, "save");

    genString physicalname;
    int ret = getFilterPhyname(argv[2], type, physicalname);
    if(ret == -1)
       return TCL_ERROR;

    Tcl_SetResult (interp, (char *)physicalname.str(), TCL_VOLATILE);
    return TCL_OK;
}


@@ dis_access_menu_eval
{
  newui_handle_menuitem(argc - 1, argv + 1);
  return TCL_OK;
}

@@ dis_rtl_nsel
{
  // Returns the number of items selected in a given
  // RTL (-1 means the RTL couldn't be found):

  int ret = TCL_ERROR;

  if (argc == 2) {
    int nsel = -1;
    int id = atoi(argv[1]);
    RTListServer *rtl = RTListServer::find(id);
    if (rtl) {
      symbolArr arr(0);
      rtl->getSelArr(arr);    
      nsel = arr.size();
    }
    genString result;
    result.printf("%d", nsel);
    Tcl_SetResult(interp, (char *)result, TCL_VOLATILE);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s <sid>", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}


@@ dis_browser_get
{
	if (argc > 1 ) {
		const char* what_to_get = argv[1];
		if (strcmp(what_to_get, "-categoriesList") == 0) {
			genString list;
			GetBrowserController()->get_categories(list);
			Tcl_SetResult(interp, list, TCL_VOLATILE);
		} else if (strcmp(what_to_get, "-askList") == 0) {
			genString list;
			GetBrowserController()->get_all_queries(list);
			Tcl_SetResult(interp, list, TCL_VOLATILE);
		} else if (strcmp(what_to_get, "-askListForRTL") == 0) {
			if (argc == 3 ) {
				const char* rtl = argv[2];
				int rtlId = OSapi_atoi(rtl);
				RTListServer* elementsRTL = RTListServer::find(rtlId);					
				if (elementsRTL) {
					symbolArr syms;
					elementsRTL->getSelArr(syms);
					genString list;
					GetBrowserController()->get_queries((const symbolArr&) syms, list);
					Tcl_SetResult(interp, list, TCL_VOLATILE);
				}
			} else {
				dis_message(NULL, MSG_ERROR,
					"'%s' requires an RTL Id.",
					what_to_get);
			}
		} else if (strcmp(what_to_get, "-askListForCategories") == 0) {
			if (argc >= 3 ) {
				// parse selection range
				genMask ddsel;
				
				//break out of the loop when next selected category name is empty	
				for (int i = 2; i < argc; i++)				{
					//get next selected Category
					genString CategoryName = argv[i];
					if (CategoryName.is_null())
						continue;
			
					CategoryName.trim();
					int kind = GetBrowserController()->get_category_kind(CategoryName);
					if (kind >= 0)
						ddsel += kind;
				}
				
				if (ddsel.empty() ) {
					genString list;
					GetBrowserController()->get_all_queries(list);
					Tcl_SetResult(interp, list, TCL_VOLATILE);
				} else {
					genString list;
					GetBrowserController()->get_queries((const genMask&)ddsel, list);
					Tcl_SetResult(interp, list, TCL_VOLATILE);
				}
			} else {
				dis_message(NULL, MSG_ERROR,
					"'%s' requires an RTL Id.",
					what_to_get);
			}
		} else {
			dis_message(NULL, MSG_ERROR,
				"'%s' is an unknown dis_browser_get option.",
				what_to_get);
		}
	} else {
		dis_message(NULL, MSG_ERROR,
			"dis_browser_get requires at least one arg instead of %d", argc);
	}
  return TCL_OK;
}

@int dis_is_module(int id, int pos)
{
    int fRet = 0;

    RTListServer* server = RTListServer::find (id);
    if( server )
    {
        symbolPtr sym = server->getSortedEntry (pos);
        if( sym.get_kind() == DD_MODULE )
            fRet = 1;
    }
    return fRet;
}
