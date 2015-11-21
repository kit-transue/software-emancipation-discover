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
#include "general.h" // lvku: test of Quick Merge
#include "genError.h"
#include "tcl.h"
#include "Interpreter.h"
#include "api_internal.h"
#include "genString.h"
#include "browserShell.h"
#include "viewerShell.h"
#include "projectBrowser.h"
#include "SetsUI.h"
#include "top_widgets.h"
#include "gtRTL.h"
#include "ste_interface.h"
#include "NewPrompt.h"
#include "cliUI.h"
#include "gtStringEd.h"
#include "gtOptionMenu.h"
#include "gtAppShellXm.h"
#include "gtNoParent.h"
#include "autosubsys.h"
#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "messages.h"
#include "Question.h"
#include "view_creation.h"
#include "edtRawSelections.h"
#include "driver.h"
#include "cLibraryFunctions.h"
#include "BrowserController.h"
#include "LanguageController.h"
#include "viewerShell.h"
#include "miniBrowser.h"
#include "closure.h"
#include "charstream.h"
#include "viewList.h"
#include "machdep.h"
#include "msg.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */

init_relational(cliQueryData,objDictionary);
init_relational(cliMenu,objDictionary);
init_relational(cliMenuItem, objDictionary);
init_rel_or_ptr(browserQueryMenu,browserQueryMenu,0,cliMenu,cliMenu,0);
init_rel_or_ptr(cliMenuItem, cliMenuItem, 0, cliQueryData, cliQueryData, 0);

genString instance_click_script = "";
genString instance_title        = "";

Interpreter*  GetActiveInterpreter();
void SetActiveInterpreter(Interpreter*);
extern void (*ui_change_scope_hook)(char*);
extern void (*ui_change_mode_hook)(char*);


void	cliQueryData::print(ostream& ostr, int ) const
{
  ostr << get_name() << ' ' << (char*)query_command << ' ' << selection_opt
       << result_opt;
}
void	cliMenu::print(ostream& ostr, int ) const
{
  int sz = items.size();
  ostr << get_name() << ' ' << sz;
  if(sz) node_prt(&items);
}
void    cliMenuItem::print(ostream &ostr, int) const
{
  ostr << get_name() << ' ' << "mtype=" << (int)mtype << ' ' << "menu=" << hex << menu << endl;
}

static cliMenuArr* cli_init_menus()
{
   cliMenuArr*arr = new cliMenuArr;

   arr->insert_last(new cliMenu("Access"));

   return arr;
}

static symbolArr& get_contents(gtRTL* gtrtl)
{
  RTL* rtl_head = gtrtl->RTL();
  RTLNode*node = RTLNodePtr(rtl_head->get_root());
  return node->rtl_contents();
}
cliMenuArr *cli_menus = cli_init_menus();
void cliQueryCallback(gtPushButton*, gtEvent*, void* data, gtReason)
{
  Initialize(QueryCallback);
 
#ifndef NEW_UI   
  browserShell * bs = ((browserQueryData *)data)->menu->browser;
  projectBrowser*pb = bs->get_project_browser();
  cliQueryData *q = ((browserQueryData *)data)->data;
  symbolArr selection;
  int scan_mode = 1;
  if(strcmp(pb->mode(), "scan"))
    scan_mode = 0;

  int result_opt = q->result_opt;
  int selection_opt = q->selection_opt;
  if(selection_opt != 4){  // -none
    if(scan_mode == 0) {
      selection = pb->hier_selection;
      if(!result_opt)
	result_opt = 1;
    } else if(selection_opt==1){  //scope
      objArr&scope = pb->get_scope()->domain;
      selection = scope;
    } else {
      int result_size = pb->result_selection.size();
      int domain_size = pb->domain_selection.size();
      if(selection_opt == 0 || selection_opt == 5){  //browser selection
	selection.insert_last(pb->domain_selection);
	selection.insert_last(pb->result_selection);
      } else {  // filter
	if(result_size)
	  selection.insert_last(get_contents(pb->result_list));
	if(domain_size)
	  selection.insert_last(get_contents(pb->domain_list));
      }
      if(result_size>0 && domain_size>0)
	selection.usort();
      if(!result_opt)
	result_opt = domain_size < result_size ? 1 : 2;
    }
    if(selection.size() == 0 && selection_opt != 5) {
      browserShell::display_assist_line("Nothing selected", 1);
      return;
    }
  }

  push_busy_cursor();
  Interpreter* interp = q->interpreter;

  Interpreter *prev_interpreter = GetActiveInterpreter();
  SetActiveInterpreter(interp);

  release_sigchild();
  symbolFormat* defaultSymFormat = interp->current_print_format;
  symbolFormat* att_format = NULL;
  if(result_opt == 3) { // if a command requires to keep current printformat - set it up
	viewListHeaderPtr pVh = (viewListHeaderPtr)pb->domain_list->dial()->get_view_ptr(0);
	if(pb->domain_selection.size() == 0) pVh=(viewListHeaderPtr)pb->result_list->dial()->get_view_ptr(0);
	const char* format_spec = pVh->format_spec();
	if(!format_spec || !*format_spec) {
		symbolAttribute * att = symbolAttribute::get_by_name("on");
		objArr attrs;
		attrs.insert_last(att);
		att_format = new symbolFormat("SortedListFormat", attrs, "%s");
	} else {
		objArr attrs;
		genString form = "";
		while(*format_spec)
			if(*format_spec == '%') {
				format_spec++;
				form += '%';
				while(isdigit(*format_spec) || (*format_spec == '-')) form += *format_spec++;
				form += 's';
				genString name = "";
				while (*format_spec && *format_spec != ',' && !isspace(*format_spec) && *format_spec != '%')
					name += *format_spec++;
				if (*format_spec == ',') format_spec ++;
				symbolAttribute * att = symbolAttribute::get_by_name((char*)name);
				attrs.insert_last(att);
			} else 
				form += *format_spec++;
			att_format = new symbolFormat("ldrListFormat", attrs, form);
	}
	interp->current_print_format = att_format;
  }
  interp->ExecuteQuery(q, selection);
  interp->current_print_format = defaultSymFormat; // restore symbol format
  if(result_opt == 3) {
	if(att_format != NULL) delete att_format;
	result_opt = 1;
  }
  restore_sigchild();

  SetActiveInterpreter(prev_interpreter);

  if(!scan_mode)
    pb->scan_mode();
  
  interp->AnswerQuery(pb, result_opt);
  pop_cursor();
#else
    printf ("QueryCallback(gtPushButton*, gtEvent*, void*, gtReason) called but not defined\n");
#endif
}

void Interpreter::AnswerQuery(projectBrowser*pb, int result_opt){
    if(this->code == TCL_OK){
	DI_object result = this->DecodeDIResult();
	if(result != NULL){
	    symbolArr& res_arr = *get_DI_symarr(result);
	    if(res_arr.size()==0)
		browserShell::display_assist_line("Empty result", 1);
	    else if(result_opt==1)
		pb->insert_array_in_domain(res_arr);
	    else
		pb->insert_array_in_results(res_arr);
	} else {
	    browserShell::display_assist_line(Tcl_GetStringResult(interp), 1);
	}
    } else {
	char *err_info = Tcl_GetVar(this->interp, (char *)"errorInfo", TCL_GLOBAL_ONLY);
	if(err_info)
	    browserShell::display_assist_line(err_info, 1);
	//printf(err_info);
    }
}

int cli_eval_query(const char*cmd, symbolArr&selection, symbolArr&answers)
{
    return
	interpreter_instance->EvalQuery(cmd, selection, answers);
}

int Interpreter::EvalQuery(const char*cmd, symbolArr&selection, symbolArr&answers)
{
    cliQueryData query("");
    query.query_command = cmd;
    query.selection_opt = 0;
    query.interpreter = this;
    release_sigchild();
    ExecuteQuery(&query, selection);
    restore_sigchild();
    answers.removeAll();
    if(code == TCL_OK){
	DI_object result = DecodeDIResult();
	if(result != NULL){
	    symbolArr& res_arr = *get_DI_symarr(result);
	    answers.insert_last(res_arr);
	}
    }
    return code;
}

void Interpreter::ExecuteQuery(cliQueryData *q, symbolArr&selection)
{
    Initialize(Interpreter::ExecuteQuery);

#ifndef NEW_UI
  if(selection.size() > 0 || q->selection_opt==5 ){
      call_cli_callback(this, (const char *)q->query_command.str(), 
			&selection, (symbolArr *)NULL);
  } else {    
    EvalCmd((char*)q->query_command);
    ;
  }
#else
    printf ("Interpreter::ExecuteQuery(void *) called but not defined\n");
#endif
}

static int add_query_item(const char*mtag, const char*tag_, const char*cmd,
     int sel_opt, int res_opt, cliMenuItemCode mtype, Interpreter *i)
{
    cliMenu * menu = (cliMenu*) obj_search_by_name(mtag, *cli_menus);
    if(!menu)
       return 1;

    if(!cmd)
      cmd = tag_;

    const char *tag;
    cliQueryData *q = NULL;
    genString str;
    
    if (mtype == MENU_ITEM_SEPARATOR) {
      //
      // Keep cycling through tag names until we find
      // a unique one:
      //
      int i = 0;
      do {
        str.printf("sep%d", i++);
      } while ((i < INT_MAX) && (obj_search_by_name((const char *)str, menu->items)));
      tag = (const char *)str;
    } 
    else {
      tag = tag_;
      q  = (cliQueryData*) obj_search_by_name(tag, menu->items);

    }

    if(!q){
      q = new cliQueryData(tag);

      cliMenuItem *mitem = new cliMenuItem(tag, mtype, menu);
      cliQueryData_put_cliMenuItem(q, mitem);

      menu->items.insert_last(q);
      browserShell::add_query_submenu(menu, menu->items.size()-1);
    }
    if(interpreter_instance)
	q->interpreter = interpreter_instance;
    else
	q->interpreter = i;
    q->query_command = cmd;
    q->selection_opt = sel_opt<0? menu->selection_opt : sel_opt;
    q->result_opt = res_opt<0? menu->result_opt : res_opt;

    return 0;
}

// Wrapper function to add menu item:
//
static int add_query_item(const char*mtag, const char*tag, const char*cmd,
     int sel_opt, int res_opt,  Interpreter *i)
{
  return add_query_item(mtag, tag, cmd, sel_opt, res_opt, MENU_ITEM_ACTION, i);
}

static int add_query_separator(const char *mtag, cliMenuItemCode mtype, Interpreter *i)
{
  int sel_opt = 0;
  int res_opt = 0;

  return add_query_item(mtag, NULL, NULL, sel_opt, res_opt, mtype, i);
}

int add_basic_item(const char*name, int argc, char**argv, Interpreter * i)
{
  int sel_opt = -1;
  int res_opt = -1;

  const char * item = 0, * command = 0;
  int narg =
     parse_menu_options(argc-1, argv+1, sel_opt, res_opt, &item, &command, 0);

  return
      add_query_item(name, item, command, sel_opt, res_opt, i);
}

int cli_add_query_item(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  int sel_opt = -1;
  int res_opt = -1;

  typedef const char* chp;
  chp mtag = 0, item = 0, command = 0;
  int narg =
     parse_menu_options(argc-1, argv+1, sel_opt, res_opt, &mtag, &item, &command);

  return
   add_query_item(argv[1], argv[2], argv[3], sel_opt, res_opt, (Interpreter*)cd);
}

int cli_add_query_separator(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  int sel_opt = 0;
  int res_opt = 0;
  cliMenuItemCode mtype = MENU_ITEM_SEPARATOR;

  typedef const char* chp;
  chp mtag = 0, item = 0, command = 0;
  int narg =
     parse_menu_options(argc-1, argv+1, sel_opt, res_opt, &mtag, &item, &command);

  return
   add_query_separator(argv[1], mtype, (Interpreter*)cd);
}

static smtTree* symbol_get_def_smt(symbolPtr&sym)
{
 Initialize(symbol_get_def_smt);
  smtTree*smt = NULL;
  ddKind kind = sym.get_kind();
  if(kind==DD_PROJECT || kind==DD_SCOPE)
    return NULL;

  if(kind==DD_MODULE) {
    app* h = sym.get_def_app();
    if(h)
      smt = checked_cast(smtTree,h->get_root());
  } else {
    ddElement* dd = sym.get_def_dd();
    if(dd)
      smt = (smtTree*)get_relation(def_smt_of_dd, dd);
  }

  return smt;
}



#if 0
// does not work
void dd_do_analysis(symbolArr* changed, symbolArr& excluded, projNode*);

static int src_dependencyCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  
  projNode*home = projNode::get_current_proj();
  symbolArr modified; 
  symbolArr anafiles;
  int sz = arr.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = arr[ii];
    ddKind kind = sym.get_kind();
    if(kind== DD_MODULE) {
      symbolArr mod_arr;
      sym->get_link(is_defining, mod_arr);
      modified.insert_last(mod_arr);
      anafiles.insert_last(sym);
    } else {
      symbolPtr def = sym->get_def_file();
      modified.insert_last(sym);
      anafiles.insert_last(def);
    }
  }

  anafiles.usort();
  modified.usort();

  int old_size = modified.size();
  dd_do_analysis(&modified, anafiles, home);
  int new_size = modified.size();

  if(new_size > old_size) {
    DI_object res;
    DI_object_create(&res);
    symbolArr& res_arr = *get_DI_symarr(res);
    for(int ii = old_size; ii<new_size; ++ii)
      res_arr.insert_last(modified[ii]);    
    i->SetDIResult(res);
  }
  
  return TCL_OK;
}
#endif


static void gui_add_menu(cliMenu *menu, cliMenu *pmenu)
{
  if (pmenu) {
    // The menu is a sub-menu (cascading menu item):
    //
    char *tag = menu->get_name();

    cliQueryData *q = (cliQueryData*) obj_search_by_name(tag, pmenu->items);

    if (!q) {
      q = new cliQueryData(tag);

      // The following are meaningless when applied
      // to cascading sub-menus:
      //
      q->interpreter   = NULL;
      q->query_command = NULL;
      q->selection_opt = 0;
      q->result_opt = 0;

      // This distinguishes us from normal menu items
      // that execute user-defined callbacks:
      //
      cliMenuItem *mitem = new cliMenuItem(tag, MENU_ITEM_SUBMENU, menu);
      cliQueryData_put_cliMenuItem(q, mitem);

      pmenu->items.insert_last(q);
      browserShell::add_query_submenu(pmenu, pmenu->items.size() - 1);
    }
  }
  else
    browserShell::add_query_menu(menu);
}

bool valid_menu_hierarchy(const char *mtag)
{
  bool ret_val = false;

  // A menu hierarchy is valid if and only if
  // the first character is not the delimiter:
  //
  if (mtag)
    if (*mtag != '.')
      ret_val = true;
  
  return ret_val;
}

void menu_child_name(const char *name, genString& child)
{
  child = (char *)NULL;

  const char *ptr = strrchr(name, '.');

  if (ptr) {
    const char *tail = (const char *)(name + strlen(name) - 1);
    if (ptr == tail) {
      // Special case: The last delim char is in fact
      // the last char of the name.

      // Go backwards until we hit a non-delim char:
      while (*ptr == '.')
	--ptr;

      // Now go backwards until we hit a delim char:
      while ((ptr >= name + 1) && (*(ptr - 1) != '.'))
	--ptr;
    }
    else
      ++ptr;
  }
  else
    ptr = name;

  child = ptr;
}

void menu_parent_name(const char *name, genString& parent)
{
  parent = (char *)NULL;

  genString child;
  menu_child_name(name, child);
    
  int index = strlen(name) - strlen(child);
  for (int i = 0; i < index - 1; i++)
    parent += name[i];
}

static cliMenu*cli_find_menu_internal(const char*mtag, int sel_opt, int res_opt)
{
  cliMenu * menu = (cliMenu*) obj_search_by_name(mtag, *cli_menus);
  if(!menu) {
    genString ptag;
    cliMenu *pmenu = NULL;

    menu_parent_name(mtag, ptag);

    if (ptag.not_null())
      pmenu = cli_find_menu_internal(ptag, 0, 0);  // Recurse here (create the whole hierarchy if need be).

    menu = new cliMenu(mtag,sel_opt,res_opt);
    cli_menus->insert_last(menu);
    gui_add_menu(menu, pmenu);
  }

  return menu;
}

static cliMenu *cli_find_menu(const char *mtag, int sel_opt, int res_opt)
{
  if (valid_menu_hierarchy(mtag))
    return cli_find_menu_internal(mtag, sel_opt, res_opt);
  else
    return NULL;
}

int cli_add_query_menu(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int sel_opt = 0;
  int res_opt = 0;

  const char* mtag = 0;
  int narg =
     parse_menu_options(argc-1, argv+1, sel_opt, res_opt, &mtag, 0, 0);

  if(narg != 1)
    return 1;

  Interpreter *i = (Interpreter *)cd;

  if (cli_find_menu(mtag, sel_opt, res_opt))
    return TCL_OK;
  else {
    Tcl_SetResult(i->interp, (char *)"Invalid menu hierarchy.", TCL_VOLATILE);
    return TCL_ERROR;
  }
}

int Interpreter::AddQueryCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{

#ifndef NEW_UI

    Interpreter *i   = (Interpreter *)cd;

    add_basic_item("Query", argc, argv, i);

    Tcl_SetResult(i->interp, argv[1], TCL_VOLATILE);

#else
    printf ("Interpreter::AddQueryCmd(ClientData, Tcl_Interp *, int, char **) called but not defined\n");
#endif

    return TCL_OK;
}

int Interpreter::AddActionCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{

#ifndef NEW_UI

    Interpreter *i   = (Interpreter *)cd;

    add_basic_item("Action", argc, argv, i);

    Tcl_SetResult(i->interp, argv[1], TCL_VOLATILE);
#else
    printf ("Interpreter::AddActionCmd(ClientData, Tcl_Interp *, int, char **) called but not defined\n");
#endif

    return TCL_OK;
}
int AddFilterCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{

#ifndef NEW_UI
    CHECK_ARG_NUM("add_filter", 2);
    Interpreter *i   = (Interpreter *)cd;

    cli_find_menu("Filter", 2, 0);
    add_basic_item("Filter", argc, argv, i);

    Tcl_SetResult(i->interp, argv[1], TCL_VOLATILE);
#else
    printf ("Interpreter::AddFilterCmd(ClientData, Tcl_Interp *, int, char **) called but not defined\n");
#endif

    return TCL_OK;
}

int AddCategoryCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{

#ifndef NEW_UI

    Interpreter *i   = (Interpreter *)cd;

    add_basic_item("Category", argc, argv, i);

    Tcl_SetResult(i->interp, argv[1], TCL_VOLATILE);
#else
    printf ("Interpreter::AddActionCmd(ClientData, Tcl_Interp *, int, char **) called but not defined\n");
#endif

    return TCL_OK;
}

symbolArr* GetSelection();

int Interpreter::SelectionCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("selection", 0);
    Interpreter *i  = (Interpreter *)cd;
    symbolArr selection;
    browserShell::get_all_browser_selections(selection, 1);
    DI_object result;
    DI_object_create(&result);
    symbolArr *result_arr = get_DI_symarr(result);
    result_arr->insert_last(selection);
    i->SetDIResult(result);

#else
    printf ("Interpreter::SelectionCmd(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}
extern int gt_process_events();
int Interpreter::EmacsCommandCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("emacs_command", 1);
    if(argc < 2){
	Tcl_AppendResult(interp, "wrong number of arguments", NULL);
	return TCL_ERROR;
    }
    genString cmd = argv[1];
    viewerShell* vs = viewerShell::get_current_vs(1);
    if(vs)
      vs->bring_to_top();
    else {
      vs = viewerShell::get_current_vs(-1);
      gt_process_events();
    }
    ste_epoch_evaluate(cmd);
#else
    printf ("Interpreter::EmacsCommandCmd(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}

static void dummy_callback(void *)
{
}

extern "C" void cmd_do_flush();

int Interpreter::SetResultCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("set_result", 1);
    Interpreter *i  = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;
#if 0
    ostream& os = i->GetOutputStream();
    os << "This command will be removed in future versions of DISCOVER\n";
    os << "Please use <browser set results> instead\n";
#endif

    SetResults(res);

#else
    printf ("Interpreter::SetResultCmd(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}

int Interpreter::SetElementsCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("set_elements", 1);
    Interpreter *i  = (Interpreter *)cd;
    symbolArr res;
    if(i->ParseArguments(1, argc, argv, res) != 0)
	return TCL_ERROR;

#if 0
    ostream& os = i->GetOutputStream();
    os << "This command will be removed in future versions of DISCOVER\n";
    os << "Please use <browser set elements> instead\n";
#endif

    SetElements(res);

#else
    printf ("Interpreter::SetElementsCmd(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}

int Interpreter::GetSelectionCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("get_selection", 0);
#else
    printf ("Interpreter::GetSelectionCmd(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}

view* view_create(symbolPtr sym, repType rep_type, int enforce_rep_type);
static int open_viewCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Interpreter *i = (Interpreter *)cd;
  symbolArr arr;
  int ret;
  repType type = Rep_UNKNOWN;

  if (argc >= 3)
    {
      ret = i->ParseArguments(2, argc, argv, arr);
      int arglen = strlen(argv[1]);
      if (strncmp(argv[1], "-outline", arglen) == 0) type = Rep_VertHierarchy;
      else if (strncmp(argv[1], "-flowchart", arglen) == 0) type = Rep_FlowChart;
      else if (strncmp(argv[1], "-dependency", arglen) == 0) type = Rep_Tree;
      else if (strncmp(argv[1], "-inheritance", arglen) == 0) type = Rep_OODT_Inheritance;
      else if (strncmp(argv[1], "-relations", arglen) == 0) type = Rep_OODT_Relations;
      else if (strncmp(argv[1], "-datachart", arglen) == 0) type = Rep_DataChart;
      else if (strncmp(argv[1], "-subsystems", arglen) == 0) type = Rep_SubsystemMap;
    }      
  else if (argc == 2)
    ret = i->ParseArguments(1, argc, argv, arr);
  else
    return TCL_ERROR;

  if(ret != 0)
    return TCL_ERROR;

  int sz = arr.size();

  for(int ii = 0; ii<sz; ++ii)
    view_create(arr[ii], type, 0);

  view_create_flush();

  return TCL_OK;
}


static int SetInstanceClickCBCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    
    if(argc != 2){
	Tcl_SetResult(i->interp, (char *)"set_instance_click requires script as an argument", TCL_VOLATILE);
	return TCL_ERROR;
    }
    instance_click_script = argv[1];
    Tcl_SetResult(i->interp, (char *)"", TCL_VOLATILE);
    return TCL_OK;
}

static int SetInstanceTitleCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    
    if(argc != 2){
	Tcl_SetResult(i->interp, (char *)"set_instance_title requires an argument", TCL_VOLATILE);
	return TCL_ERROR;
    }
    instance_title = argv[1];
    Tcl_SetResult(i->interp, (char *)"", TCL_VOLATILE);
    return TCL_OK;
}

/* Function called from minibrowser when it is trying to display instances of "selection" */
int cli_instance_hook(symbolPtr& selection, symbolArr& domain_selection, symbolArr& cli_results, 
		      genString& cli_title, genString& cli_click_cb)
{
    Tcl_CmdInfo instance_hook_info;

    if(!interpreter_instance)
	return 0;
    if(Tcl_GetCommandInfo(interpreter_instance->interp, (char *)"instance_hook", &instance_hook_info) == 0)
	return 0;
    DI_object di_selection;
    DI_object_create(&di_selection);
    symbolArr *selection_arr = get_DI_symarr(di_selection);
    DI_object di_domain;
    DI_object_create(&di_domain);
    symbolArr *domain_arr = get_DI_symarr(di_domain);
    genString cmd;
    cmd.printf("instance_hook _DI_%d _DI_%d", di_selection, di_domain);
    interpreter_instance->ReferenceObject(di_selection);
    interpreter_instance->ReferenceObject(di_domain);
    selection_arr->insert_last(selection);
    domain_arr->insert_last(domain_selection);
    instance_click_script = "";
    instance_title        = "";
    int result   = interpreter_instance->EvalCmd((char *)cmd) == TCL_OK;
    if(result) {
	DI_object di_result = interpreter_instance->DecodeDIResult();
	if(di_result == NULL)
	    result = 0;
	else {
	    symbolArr *res_arr = get_DI_symarr(di_result);
	    cli_results.insert_last(*res_arr);
	}
	cli_title    = instance_title;
	cli_click_cb = instance_click_script;
    } else {
	msg("Following error(s) returned from instance_hook:\n$1") << Tcl_GetStringResult(interpreter_instance->interp) << eom;
    }
    interpreter_instance->DeleteDIObject(di_domain);
    interpreter_instance->DeleteDIObject(di_selection);
    return result;
}

void browserShell::change_scope_txt(char*string)
{
    Initialize(browserShell::change_scope_txt);

    for(int ii = 0; ii < instance_array.size(); ++ii) {
	browserShell* bs = *instance_array[ii];
	projectBrowser* pb = bs->project_browser;
	if(!pb) continue;
	gtStringEditor* se = pb->scope_ed;
	if(!se) continue;
	se->text(string);
    }
}

void browserShell::change_scope_mode(char*string)
{
    Initialize(browserShell::change_scope_mode);

    for(int ii = 0; ii < instance_array.size(); ++ii) {
	browserShell* bs = *instance_array[ii];
	projectBrowser* pb = bs->project_browser;
	if(!pb) continue;
	gtOptionMenu* sm = pb->scope_mode_menu;
	if(!sm) continue;
	sm->menu_history(string);
    }
}

int getTmpGroupNameCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if(argc > 1){
	Tcl_AppendResult(interp, argv[0], " does not take any argument(s)", NULL);
	return TCL_ERROR;
    }
    char *name = SetsUI::GetGroupName();
    Tcl_SetResult(interp, name, TCL_VOLATILE);
    return TCL_OK;
}

int updateTmpGroupNameCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if(argc > 1){
	Tcl_AppendResult(interp, argv[0], " does not take any argument(s)", NULL);
	return TCL_ERROR;
    }
    SetsUI::UpdateGroupName();
    return TCL_OK;
}

int ViewFileCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  if (argc > 1) {
    for (int ii = 1; ii < argc; ii++) {
      view_create(argv[ii]);
      view_create_flush();    
    }
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s file1 [ file2 file3 ... ]", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

extern Obj *get_epoch_array(int);
extern steDocumentPtr get_relation_view_document(steViewPtr);

int EdtGetSelCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Initialize(EdtGetSelCmd);

  int ret = TCL_OK;
  
  Tcl_DString list;
  Tcl_DStringInit(&list);
  Tcl_DString item;
  genString word;

  genArrOf(EdtRawSelection) sel;
  edtGetAllSelections(sel);
  int size = sel.size();
  for (int ii = 0; ii < size; ii++) {

    EdtRawSelection *el = sel[ii];
    int start = -1;
    int end = -1;
    genString name = "";

    if (el) {
      start = el->offset;
      end = start + el->length - 1;
      steBufferPtr b = checked_cast(steBuffer, get_epoch_array(el->buffer_id));
      if (b) {
        steViewPtr v = checked_cast(steView, get_relation(buffer_view, b));
	if (v) {
          appPtr a = get_relation_view_document(v);
          if (a) {
	    projModule *m = a->get_module();
            if (m) {
	      name = m->get_name();
	    }
	  }
        }
      }
    }

    Tcl_DStringInit(&item);
    word.printf("%d", start);
    Tcl_DStringAppendElement(&item, (char *)word);
    word.printf("%d", end);
    Tcl_DStringAppendElement(&item, (char *)word);
    Tcl_DStringAppendElement(&item, (char *)name);
    Tcl_DStringAppendElement(&list, (char *)Tcl_DStringValue(&item));
    Tcl_DStringFree(&item);
  }

  Interpreter *i = (Interpreter *)cd;
  Tcl_SetResult(i->interp, (char *)Tcl_DStringValue(&list), TCL_VOLATILE);
  Tcl_DStringFree(&list);

  return ret;
}

static projectBrowser *get_one_pb()
{
  projectBrowser *ret = NULL;

  genArrOf(browserShellPtr) *bsarr = browserShell::get_browsers_list();
  if (bsarr) {
    if ((*bsarr)[0]) {
      browserShell *bs = *((*bsarr)[0]);
      if (bs) {
        ret = bs->get_project_browser();
      }
    }
  }

  return ret;
}

int GetScopeCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;
 
  if (argc == 1) {
    symbolArr scope;
    projectBrowser *pb = get_one_pb();
    if (pb) {
      pb->scan_mode();
      objArr &_scope = pb->get_scope()->domain;
      scope = _scope;
    }
    DI_object di;
    DI_object_create(&di);
    symbolArr *res = get_DI_symarr(di);
    res->insert_last(scope);
    i->SetDIResult(di);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("usage: %s", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

int GetClosureCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;
 
  if (argc == 1) {
    bool closure = get_closure_state();

    if (closure)
      Tcl_SetResult(i->interp, (char *)"1", TCL_VOLATILE);
    else
      Tcl_SetResult(i->interp, (char *)"0", TCL_VOLATILE);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("usage: %s", argv[0]);
    Tcl_AppendResult(i->interp, (char *)usage, NULL);
  }

  return ret;
}

int SetsManagerCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int retval = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  bool refresh = false;
  char *title = NULL;
  bool getsel = false;

  if (argc == 1)
    retval = TCL_OK;
  else
    for (int ii = 1; ii < argc; ii++) {
      if (!strcmp(argv[ii], "-refresh")) {
        if (!refresh && !title) {              // -refresh and -title are mutually exclusive
          refresh = true;
	  retval = TCL_OK;
        }
        else { retval = TCL_ERROR; break; }
      }
      else if (!strcmp(argv[ii], "-title")) {
        if (!title && !refresh) {
	  if (ii < argc - 1) {
            title = argv[++ii];                // Increment counter
	    retval = TCL_OK;
          }
          else { retval = TCL_ERROR; break; }
        }      
        else { retval = TCL_ERROR; break; }
      }
      else if (!strcmp(argv[ii], "-getsel")) {
        if (!getsel) {
          getsel = true;
          retval = TCL_OK;
        }
	else
	  { retval = TCL_ERROR; break; }
      }
      else { retval = TCL_ERROR; break; }
    }

  if (retval == TCL_OK) {
    if (refresh)
      SetsUI::Refresh();
    else if (!getsel)
      SetsUI::Invoke(title);

    if (getsel) {
      TclList list;
      SetsUI::ListSelected(list);
      Tcl_SetResult(i->interp, (char *)list, TCL_VOLATILE);
    }
    else
      Tcl_SetResult(i->interp, (char *)"", TCL_VOLATILE);
  }
  else {
    genString usage;
    usage += "Usage: ";
    usage += argv[0];
    usage += " -refresh\n";
    usage += "       ";
    usage += argv[0];
    usage += " [ -title <title> ]\n";
    usage += "       ";
    usage += argv[0];
    usage += " -getsel";
    Tcl_SetResult(i->interp, usage, TCL_VOLATILE);
  }

  return retval;
}


int execute_put_start_script(symbolArr& symbols, genString& bugtext, genString text);
static int call_put_start_scriptCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;

    if(argc < 4){
	Tcl_SetResult(i->interp, (char *)"call_put_start_script: wrong number of arguments", TCL_VOLATILE);
	return TCL_ERROR;
    }
    genString bugtext = argv[1];
    genString text    = argv[2];
    symbolArr arr;
    if(i->ParseArguments(3, argc, argv, arr) != 0)
	return TCL_ERROR;
    int result = execute_put_start_script(arr, bugtext, text);
    char res[10];
    sprintf(res, "%d", result);
    Tcl_SetResult(i->interp, res, TCL_VOLATILE);
    return TCL_OK;
}

static int get_put_start_scriptCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Interpreter *i = (Interpreter *)cd;
    
    if(argc != 1){
	Tcl_SetResult(i->interp, (char *)"get_put_start_script does not requires any argument(s)", TCL_VOLATILE);
	return TCL_ERROR;
    }
    genString script_name = customize::configurator_put();
    Tcl_SetResult(i->interp, (char *)script_name, TCL_VOLATILE);
    return TCL_OK;
}

int cli_call_put_start(symbolArr& modules, genString& bugtext, genString text)
{
    Tcl_CmdInfo put_start_info;

    if(!interpreter_instance)
	return 1;
    if(Tcl_GetCommandInfo(interpreter_instance->interp, (char *)"cli_put_start", &put_start_info) == 0)
	return 1;
    DI_object di_modules;
    DI_object_create(&di_modules);
    symbolArr *modules_arr = get_DI_symarr(di_modules);
    genString cmd;
    char *bug_txt = (char *)"";
    if(bugtext.length() != 0)
	bug_txt = bugtext;
    char *comment_txt = (char *)"";
    if(text.length() != 0)
	comment_txt = text;
    Tcl_DString btxt;
    Tcl_DStringInit(&btxt);
    Tcl_DString ctxt;
    Tcl_DStringInit(&ctxt);
    Tcl_DStringAppendElement(&btxt, bug_txt);
    Tcl_DStringAppendElement(&ctxt, comment_txt);
    cmd.printf("cli_put_start _DI_%d {%s} {%s}", GET_DI_SET_INDEX(di_modules), (char *)Tcl_DStringValue(&btxt), (char *)Tcl_DStringValue(&ctxt));
    Tcl_DStringFree(&btxt);
    Tcl_DStringFree(&ctxt);
    interpreter_instance->ReferenceObject(di_modules);
    modules_arr->insert_last(modules);

    int result = (interpreter_instance->EvalCmd((char *)cmd) == TCL_OK);
    if(result) {
        genString answer;
        interpreter_instance->GetResult(answer);
        /* Trying to maintain compatibility here -- if the
         * return string looks like a number, use it as
         * the result code; otherwise, leave the result
         * code as 1. (Before, only if the script returned
         * 0 would we count it as a failure; now, we need
         * to support multiple error states.)
         */
        char *cp = (char *)answer.str();
        if (cp && *cp && (isdigit(*cp) || isdigit(cp[1])))
          result = OSapi_atoi(cp);
#if 0
	if(interpreter_instance->interp->result[0] == '0' && interpreter_instance->interp->result[1] == 0)
	    result = 0;
#endif
    } else {
	msg("cli_put_start script returned following error:\n$1") << Tcl_GetStringResult(interpreter_instance->interp) << eom;
    }
    interpreter_instance->DeleteDIObject(di_modules);
    return result;
}

static int statusCmd(ClientData cd, Tcl_Interp *, int argc, char *argv[]);
extern void (*gmgr_refresh_hook)();

extern Display* UxDisplay;
int RefreshUICmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    Widget w = gtApplicationShellXm::find_existing();
    XSync(UxDisplay, 0);
    XmUpdateDisplay(w);
    XSync(UxDisplay, 0);
    gtNoParent* shell = gtNoParent::app_shell();
    XmUpdateDisplay(shell->rep()->widget());
    return TCL_OK;
}

static int pushBusyCursorCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    push_busy_cursor();
    return TCL_OK;
}

static int popCursorCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    pop_cursor();
    return TCL_OK;
}

static int add_commands()
{
    gmgr_refresh_hook = SetsUI::Refresh;
    ui_change_scope_hook = browserShell::change_scope_txt;
    ui_change_mode_hook = browserShell::change_scope_mode;

    new cliCommandInfo ("add_menu", cli_add_query_menu);
    new cliCommandInfo ("add_item", cli_add_query_item);
    new cliCommandInfo ("add_separator", cli_add_query_separator);
    
    new cliCommandInfo("push_busy_cursor", pushBusyCursorCmd);
    new cliCommandInfo("pop_busy_cursor", popCursorCmd);

    new cliCommandInfo("status", statusCmd);
    new cliCommandInfo("get_tmp_group_name", getTmpGroupNameCmd);
    new cliCommandInfo("update_tmp_group_name", updateTmpGroupNameCmd);
    new cliCommandInfo("set_elements", Interpreter::SetElementsCmd);
    new cliCommandInfo("set_result", Interpreter::SetResultCmd);
    new cliCommandInfo("get_selection", Interpreter::SelectionCmd);
    new cliCommandInfo("emacs_command", Interpreter::EmacsCommandCmd);
    new cliCommandInfo("status_bar_msg", Interpreter::MessageCmd);
    new cliCommandInfo("open_view", open_viewCmd);
    new cliCommandInfo("view_file", ViewFileCmd);
    new cliCommandInfo("edt_get_sel", EdtGetSelCmd);
    new cliCommandInfo("set_instance_click", SetInstanceClickCBCmd);
    new cliCommandInfo("set_instance_title", SetInstanceTitleCmd);
    new cliCommandInfo ("smgr", SetsManagerCmd);
    new cliCommandInfo ("get_scope", GetScopeCmd);
    new cliCommandInfo ("get_closure", GetClosureCmd);
    new cliCommandInfo("get_put_start_script", get_put_start_scriptCmd);
    new cliCommandInfo("call_put_start_script", call_put_start_scriptCmd);
    new cliCommandInfo("browser", BrowserCmd);
    new cliCommandInfo("refresh_ui", RefreshUICmd);
    return 0;
}


static int dummy = add_commands();

void ddbuild_hook(smtHeader * head)
{
}

extern "C" void ping_paraset ();
#include <driver.h>
static void api_process_events()
{
//  ping_paraset();
//  gt_process_events();
  driver_instance->flush_events();
}

int Interpreter::MessageCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifndef NEW_UI
    CHECK_ARG_NUM("status_bar_msg", 1);
    if (argc > 1) {
	browserShell::display_assist_line(argv[1], 1);
	viewerShell::set_status_line(argv[1]);
	api_process_events();
    }
#else
    printf ("Interpreter::Message(ClientData, Tcl_Interp *, int, char**) called but not defined\n");
#endif
    return TCL_OK;
}

#include <ParaCancel.h>
#include <StatusDialog.h>
static StatusDialog* status_dialog;

static int status_dialog_delete()
{
  int retval = 0;
  if(status_dialog){
    delete status_dialog;
    status_dialog = 0;
    retval = 1;
  }
  return retval;
}

static int status_dialog_start(const char* title, const char* msg, int items, bool OK)
{
  if(status_dialog)
    delete status_dialog;

  status_dialog     = new StatusDialog;
  int ok = status_dialog->popup((char*)title, (char*)msg, items, OK);

  if (!ok){
    delete status_dialog;
    status_dialog = 0;
  }
  return ok;
} 

static int status_dialog_query(const char* msg, int cur_count)
{
  if(!status_dialog)
    return 1;

  int retval = status_dialog->is_cancelled(cur_count, (char*)msg);
  if(retval){
    delete status_dialog;
    status_dialog = 0;
  } else {
    if(ParaCancel::canceller){
//      cout << "canceller is alive" << endl;
//      delete ParaCancel::canceller;
//      ParaCancel::canceller = 0;
    }
    status_dialog->reestablish_canceller();
    api_process_events();
  }
  return retval;
}

#define isequal(x) (strcmp(name,quote(x)) == 0)
static int statusCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  int retval = TCL_OK;
  Interpreter *i = (Interpreter *)cd;
  ostream&os = i->GetOutputStream();
  if(argc <= 1){
    os << "status subcommands: start query end" << endl;
    return retval;
  }
  const char*name =  argv[1];
  if(isequal(end)){
    int res = status_dialog_delete();
    if(argc != 2){
      os << "status end: no arguments allowed" << endl;
      retval = TCL_ERROR;
    }
    return retval;
  } 

  bool isstart;
  bool OK = false;
  int argind = 2;
  const char* title = "Status Dialog";
  if(isequal(start)){
    if(argc == 2) {
      os << "usage: status start [-ok] [-title <title>] message no_items" << endl;
      return TCL_OK;
    }
    isstart = true;
    // collect optional args
    for(; argind < argc; ++argind){
      name = argv[argind];
      if(isequal(-ok)){
	OK = true;
      } else if(isequal(-title)){
	++ argind;
	title = argv[argind];
      } else {
	break;
      }
    }
  } else if(isequal(query)) {
    if(argc == 2) {
      os << "usage: status query message count" << endl;
      return TCL_OK;
    }
    isstart = false;
  } else {
    os << "status " << name << ": wrong subcommand. Subcommands: start query end" << endl;
    return TCL_ERROR;
  }

  if(argind + 2 > argc){
    os << "status: too few arguments" << endl;
    return TCL_ERROR;
  } else if(argind + 2 < argc){
    os << "status: too many arguments" << endl;
    return TCL_ERROR;
  }
  const char* msg = argv[argind];
  const char* str_val = argv[argind+1];
  char*ptr=NULL;
  int count = OSapi_strtol(str_val, &ptr, 10);
  if(ptr[0] != '\0'){
    os << str_val << ": must be an integer" << endl;  
    return TCL_ERROR;
  }
  int status = 0;

  if(isstart){
    status = status_dialog_start(title, msg, count, OK);
  } else {
    status = status_dialog_query(msg, count);
  } 

  char buf[6];
  sprintf(buf, "%d", status);
  Tcl_SetResult(i->interp, buf, TCL_VOLATILE);
  return retval;
}

void AddCategory(const char* name, const char* command)
{
  BrowserController* pBController = LanguageController::GetBrowserController();
  pBController->add_category(name, command);

  genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
  for(int ii = 0; ii < instances->size(); ii++)
    {
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb)
	{
	  pb->update_categories();
	}
    }
}  

void AddQuery(const char* name, const char* command)
{
  BrowserController* pBController = LanguageController::GetBrowserController();
  pBController->add_query(name, command);

  genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
  for(int ii = 0; ii < instances->size(); ii++)
    {
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb)
	{
	  pb->update_ask();
	}
    }
}  

void RemoveCategory(const char* name)
{
  BrowserController* pBController = LanguageController::GetBrowserController();
  pBController->remove_category(name);

  genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
  for(int ii = 0; ii < instances->size(); ii++)
    {
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb)
	{
	  pb->update_categories();
	}
    }
}

void RemoveQuery(const char* name)
{
  BrowserController* pBController = LanguageController::GetBrowserController();
  pBController->remove_query(name);

  genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
  for(int ii = 0; ii < instances->size(); ii++)
    {
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb)
	{
	  pb->update_ask();
	}
    }
}

static gtRTL* g_NextRtl = NULL;

void SetNextRtl(gtRTL* rtl)
{
  g_NextRtl = rtl;
}

gtRTL* GetNextRtl()
{
  return g_NextRtl;
}

void SetNextRtl(const symbolArr& syms)
{
  gtRTL* rtl = GetNextRtl();

  if (rtl != NULL)
    {
      browser_set_rtl(rtl, (symbolArr&)syms);
    } 
  else // for miniBrowser
    {
      viewerShell* vs = viewerShell::get_current_vs();
      if (vs)
	{
	  miniBrowser* mb = vs->get_mini_browser();
	  if (mb)
	    {
	      Interpreter *i  = GetActiveInterpreter();
	      symbolArr dummy;
	      i->EvalQuery("set_mini", (symbolArr&)syms, dummy);
	    }
	}
    }

  cmd_do_flush();
  pset_send_own_callback (dummy_callback, (void *)NULL);  
}

void SetResults(const symbolArr& syms)
{
    genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
    for(int ii = 0; ii < instances->size(); ii++){
	browserShell *bs   = *((*instances)[ii]);
	projectBrowser *pb = bs->get_project_browser();
	if(pb)
	    pb->insert_array_in_results((symbolArr&)syms);
    }
    cmd_do_flush();
    pset_send_own_callback (dummy_callback, (void *)NULL);
}

void BrowserSort(int argc, char** argv, Interpreter* i)
{
    ostream& os = i->GetOutputStream();  
    if(argc < 4){
      os << "Usage: browser sort ( elements | results ) [ [-a | -d] <attribute> ] ... \n";
      return;
    }
    
    
    boolean to_results = false;
    if (strcmp(argv[2],"results") == 0) {
      to_results = true;
    } else if (strcmp(argv[2],"elements") != 0) {
      os << "argument to \"browser sort\"must be \"elements\" or \"results\"\n";
      return;
    }
    
    
    static ocharstream sort_list;
    sort_list.reset();
    sort_list << '#';
    char nextOrder = 'f';
    int ii;
    for(ii = 3; ii < argc; ii++){
	if (strcmp(argv[ii],"-d") == 0) {
	    nextOrder = 'r';
	} 
	else 
	    if (strcmp(argv[ii],"-a") == 0) {
		nextOrder = 'f';
	    } else {
		symbolAttribute *att = symbolAttribute::get_by_name(argv[ii]);
		if(att == NULL){
		    os << "attribute '" << argv[ii] << "' does not exist\n";
		    return;
		}
		sort_list << '%' << nextOrder << argv[ii];
	    };
    };
    sort_list << ends;

    genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
    for(ii = 0; ii < instances->size(); ii++){
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb) {
	gtRTL* r = NULL;
	if (to_results)
	  r = pb->result_list;
	else 
	  r = pb->domain_list;
	if (r) {
	  r->set_sort(sort_list.ptr());
	  viewListHeaderPtr vh = (viewListHeaderPtr)r->dial()->get_view_ptr(0);
	  vh->regenerate_view();
	}
      }
    }
    cmd_do_flush();
    pset_send_own_callback (dummy_callback, (void *)NULL);
    return;
}

void BrowserFormat(int argc, char** argv, Interpreter* i) {

    ostream& os = i->GetOutputStream();  
    if(argc < 4){
	os << "Usage: browser format <format string> [ <attribute> ] ... \n";
	return;
    }

    
    boolean to_results = false;
    if (strcmp(argv[2],"results") == 0) {
      to_results = true;
    } else if (strcmp(argv[2],"elements") != 0) {
      os << "argument to \"browser format\" must be \"elements\" or \"results\"\n";
      return;
    }
      
    char* format = argv[3];

    static ocharstream format_list;
    format_list.reset();
    ////
    int n = 0;
    char ch;
    
    int no_attr = argc - 4;
    for(char * frmt = (char*)format; (ch=*frmt); ++frmt) {
      if (ch != '%'){
	format_list << ch;
	continue;
      }
      ++frmt;
      ch = *frmt;
      if ((ch != 's') && (ch != 'S')) {
	format_list << ch;
	continue;  // loop
      }
      
      if(n >= no_attr)
	continue;
      
      symbolAttribute *att = symbolAttribute::get_by_name(argv[n+4]);
      if(att == NULL){
	os << "attribute '" << argv[n+4] << "' does not exist\n";
	return;
      }

      format_list << '%' << argv[n+4] << ',';
      n++;
    }
    format_list << ends;
    ///////////
      
    genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
    for(int ii = 0; ii < instances->size(); ii++){
      browserShell *bs   = *((*instances)[ii]);
      projectBrowser *pb = bs->get_project_browser();
      if(pb) {
	gtRTL* r = NULL;
	if (to_results)
	  r = pb->result_list;
	else 
	  r = pb->domain_list;
	if (r) { 
	  r->set_format(format_list.ptr());
	  viewListHeaderPtr vh = (viewListHeaderPtr)r->dial()->get_view_ptr(0);
	  vh->regenerate_view();
	}
      }
    }
    cmd_do_flush();
    pset_send_own_callback (dummy_callback, (void *)NULL);
    return;
}


void SetElements(const symbolArr& syms)
{
    genArrOf(browserShellPtr)* instances = browserShell::get_browsers_list();
    for(int ii = 0; ii < instances->size(); ii++){
	browserShell *bs   = *((*instances)[ii]);
	projectBrowser *pb = bs->get_project_browser();
	if(pb)
	    pb->insert_array_in_domain((symbolArr&)syms);
    }
    cmd_do_flush();
    pset_send_own_callback (dummy_callback, (void *)NULL);
}

static void BrowserAdd(int argc, char** argv, Interpreter* i)
{
  ostream& os = i->GetOutputStream();

  if (argc < 3)
    {
      os << "The following subcommands are available:\n";
      os << "  browser add category <name> <command>\n";
      os << "  browser add query <name> <command>\n";
      return;
    }

  if (argc < 4)
    {
      os << "A name must be specified\n";
    }
  else
    {
      const char* name = argv[3];
      const char* cmd;
      if (argc <= 4)
	cmd = name;
      else
	cmd = argv[4];
      
      if (strcmp(argv[2], "category") == 0)
	{
	  AddCategory(name, cmd);
	}
      else if (strcmp(argv[2], "query") == 0)
	{
	  AddQuery(name, cmd);
	}
      else
	{
	  os << "browser add: " << argv[2] << " -- unknown subcommand\n";
	}
    }
}

static void BrowserRemove(int argc, char** argv, Interpreter* i)
{
  ostream& os = i->GetOutputStream();

  if (argc < 3)
    {
      os << "The following subcommands are available:\n";
      os << "  browser remove category <name>\n";
      os << "  browser remove query <name>\n";
      return;
    }

  if (argc < 4)
    {
      os << "A name must be specified\n";
    }
  else
    {
      const char* name = argv[3];
      if (strcmp(argv[2], "category") == 0)
	{
	  RemoveCategory(name);
	}
      else if (strcmp(argv[2], "query") == 0)
	{
	  RemoveQuery(name);
	}
      else
	{
	  os << "browser remove: " << argv[2] << " -- unknown subcommand\n";
	}
    }
}

static void BrowserSet(int argc, char** argv, Interpreter* i)
{
  ostream& os = i->GetOutputStream();

  if (argc < 3)
    {
      os << "The following subcommands are available:\n";
      os << "  browser set elements <symbol set>\n";
      os << "  browser set results <symbol set>\n";
      os << "  browser set next <symbol set>\n";
      return;
    }

  if (argc < 4)
    {
      os << "A symbol set must be specified\n";
    }
  else
    {
      symbolArr res;
      if(i->ParseArguments(3, argc, argv, res) != 0)
	{
	  os << "An Error occured while processing symbol set\n";
	}
      else  if (strcmp(argv[2], "elements") == 0)
	{
	  SetElements(res);
	}
      else if (strcmp(argv[2], "results") == 0)
	{
	  SetResults(res);
	}
      else if (strcmp(argv[2], "next") == 0)
	{
	  SetNextRtl(res);
	}
      else
	{
	  os << "browser set: " << argv[2] << " -- unknown subcommand\n";
	}
    }
}

int BrowserCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  Interpreter* intr = (Interpreter*) cd;
  ostream& os = intr->GetOutputStream();

  
  if (argc < 2)
    {
      os << "The following subcommands are available:\n";
      os << "  browser add category <name> <command>\n";
      os << "  browser add query <name> <command>\n";
      os << "  browser remove category <name>\n";
      os << "  browser remove query <name>\n";
      os << "  browser set elements <symbol set>\n";
      os << "  browser set results <symbol set>\n";
      os << "  browser set next <symbol set>\n";                    
    }
  else if (strcmp(argv[1], "add") == 0)
    {
      BrowserAdd(argc, argv, intr);
    }
  else if (strcmp(argv[1], "remove") == 0)
    {
      BrowserRemove(argc, argv, intr);
    }
  else if (strcmp(argv[1], "set") == 0)
    {
      BrowserSet(argc, argv, intr);
    }
  else if (strcmp(argv[1], "sort") == 0)
    {
      BrowserSort(argc, argv, intr);
    }
  else if (strcmp(argv[1], "format") == 0)
    {
      BrowserFormat(argc, argv, intr);
    }
  else
    {
      os << "browser: " << argv[1] << " -- unknown subcommand\n";
    }
  return TCL_OK;
}

