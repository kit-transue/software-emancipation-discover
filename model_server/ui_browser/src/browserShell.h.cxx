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
// browserShell.h.C 
//------------------------------------------
// synopsis:
// Generic container for file and project browsers
//
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <x11_intrinsic.h>

#include <genArr.h>

#include <gtTopShell.h>
#include <gtSepar.h>
#include <gtTopShellXm.h>
#include <gtMainWindow.h>
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtOptionMenu.h>
#include <gtStringEd.h>
#include <gtFrame.h>
#include <gtMenuBar.h>
#include <gtTogB.h>
#include <gtRadioBox.h>
#include <gtTextEd.h>
#include <gtCascadeB.h>
#include <gtLabel.h>
#include <gtPanedWindow.h>
#include <gtList.h>
#include <gtPushButton.h>
#include <top_widgets.h>
#include <messages.h>
#include <genString.h>
#include <genError.h>
#include <customize.h>

#include <symbolArr.h>
#include <objOper.h>
#include <RTL.h>
#include <gtRTL.h>
#include <viewNode.h>

#include <proj.h>

#include <fileCache.h>

#include <browserShell.h>

#include <view_creation.h>

#include <help.h>
#include <cmd.h>

#include <partition.h>

#include <cliUI.h>
#include <viewerShell.h>
#include <SetsUI.h>
#include <mpb.h>
#include <xxinterface.h>
#include <QualityCockpitUI.h>

#undef index 

RelClass(RTL);

// EXTERNAL FUNCTIONS

extern void delete_choose_home();

// VARIABLE DEFINITIONS

const char* const browserShell::LAYOUT_FORMAT_VERSION = "1.0";
const char* const browserShell::DEFAULT_LAYOUT_FILE = ".psetlayout";

genArrOf(browserShellPtr) browserShell::instance_array;

// FUNCTION DEFINITIONS

extern "C" void browserShell_load()
{
    browserShell::startup();
}

void browserShell::startup()
{
    browser_new_browser_window(NULL, NULL, 0, gtActivate);
}

void browserShell::obtain_all_projects(symbolArr &pn, int &top_proj)
{
    Initialize(browserShell::obtain_all_projects);
    
    project_browser->obtain_all_projects(pn, top_proj);
}

void browserShell::remember(browserShellPtr bs)
{
    Initialize(browserShell::remember);
    
#ifndef NEW_UI
    instance_array.append(&bs);
#endif
}

void browserShell::forget(browserShellPtr bs)
{
    Initialize(browserShell::forget);
    for(int i = 0; i < instance_array.size(); ++i)
    {
	browserShellPtr instance = *instance_array[i];
	if(instance == bs)
	{
	    instance_array.remove(i);
	    break;
	}
    }
}

projNode *browserShell::cur_projnode()
{
    Initialize(browserShell::cur_projnode);

    RTLPtr rt = project_browser->browsed_project();
    if(rt && is_projHeader(rt))
    {
        projNodePtr pn = checked_cast(projNode, rt->get_root());
        return pn;
    }
    return 0;
}

void browserShell::force_up(int up_level)
{
    Initialize(browserShell::force_up);

    project_browser->force_up(up_level);
}

void browserShell::update_existing_project_node(projNode *dpn)
{
    Initialize(browserShell::update_existing_project_node);

    for(int i = 0; i < instance_array.size(); ++i)
    {
        browserShellPtr bs = *instance_array[i];

        projNode *pn = bs->cur_projnode();
        if (pn)
        {
            int up_level = 1;
            projNode *cur = pn;
            while(cur)
            {
                if (cur == dpn)
                {
                    bs->force_up(up_level);
                    break;
                }
                up_level++;
                if (cur->get_map() == 0)
                    cur = cur->find_parent();
                else
                    break;
            }
        }
    }
}

static int nNumInstances = 0;

#include "CMIntegratorCtrlr.h"

browserShell::browserShell()
: selected_browser(NULL),
  project_browser(NULL),
  project_selection(NULL),
  project_selection_data(NULL),
  manage_button(NULL),
  cascade_button(NULL),
  num_make_menu_entries(0),
  assist_line(NULL)
{
    Initialize(browserShell::browserShell);

    if(m_pcmController==NULL)
	m_pcmController = CCMIntegratorCtrlr::getInstance();
    nNumInstances++;

    // Assign a unique numeric string to this layout.
    for(int i = 1; ; ++i)
    {
	layout_name.printf("%d", i);

	int j;
	for(j = 0; j < instance_array.size(); ++j)
	{
	    browserShellPtr instance = *instance_array[j];
	    if(instance->layout_name == layout_name)
		break;
	}
	if(j >= instance_array.size())
	    break;
    }

    build_interface();
#ifndef NEW_UI
    shell->popup();
    mpb_popup();  //popup main progress bar
#endif
    project_browser->browse_mode();

    remember(this);    
}

browserShell::browserShell(
    gtBase* parent, const char* prompt, projectSelectionCB cb, void* data)
 : selected_browser(NULL),
  project_browser(NULL),
  project_selection(cb),
  project_selection_data(data),
  layout_name("dialog"),
  manage_button(NULL),
  cascade_button(NULL),
  num_make_menu_entries(0),
  assist_line(NULL)
{
    Initialize(browserShell::browserShell);

    if(m_pcmController==NULL)
	m_pcmController = CCMIntegratorCtrlr::getInstance();

    build_dialog(parent, prompt);

    remember(this);
}

browserShell::~browserShell()
{
    Initialize(browserShell::~browserShell);

    forget(this);
#ifndef NEW_UI
    shell->popdown();
#endif

    // groupManager
    int count = instance_array.size();
    while (count > 0) {
	browserShell *bs = *instance_array[count-1];
	if (bs != this) {
	    this->project_browser->reset_group_dialog(bs->project_browser);
	    count = -1;
	}
	count--;
    }

    delete project_browser;
    delete shell;

    // cliQuery
    int no_menus = cli_menus->size();
    for(int ii = 0; ii<no_menus; ++ii){
      cliMenu * menu = (cliMenu*) (*cli_menus)[ii];
      Obj* dset = cliMenu_get_browserQueryMenus(menu);
      Obj*el;
      ForEach(el, *dset){
        browserQueryMenu*bqm = (browserQueryMenu*)el;
        if(bqm->browser == this){
	  delete bqm;
	  break;
	}
      }
    }
}

void browserShell::select(projectBrowser* pb)
{
    Initialize(browserShell::select);

    symbolArr& projects = pb->project_selection();
    symbolArr& modules = pb->module_selection();
    symbolArr& symbols = pb->symbol_selection();

    if(projects.size()  ||  modules.size()  ||  symbols.size())
    {
	selected_browser = pb;
    }
}

void browserShell::default_action()
{
    Initialize(browserShell::default_action);

    push_busy_cursor();

    if(selected_browser && selected_browser == project_browser) {
	symbolArr& projects = project_browser->project_selection();
	symbolArr& symbols = project_browser->symbol_selection();

        if(symbols.size()) {
	    open_views(Rep_UNKNOWN);
	} else if(projects.size()) {
	    project_browser->open(projects[0]);
	}
    }

    pop_cursor();
}

//------------------------------------------
// browserShell::browsed_project()
//------------------------------------------

RTLPtr browserShell::browsed_project()
{
   Initialize(browserShell::browsed_project);

   return project_browser ? project_browser->browsed_project() : NULL;
}

//------------------------------------------
// browserShell::update_domains()
//------------------------------------------

void browserShell::update_domains() {
   Initialize(browserShell::update_domains);

#ifndef NEW_UI
   if (project_browser) {
      char* text = project_browser->gt_string_ed->text();
      projList::domain_reset();
      projList::search_list->parse_names_init(text);
      projList::search_list->domain_string();
      gtFree(text);
   }
#endif
}

//------------------------------------------
// browserShell::disable_apply_button()
//------------------------------------------

void browserShell::disable_apply_button() {
   Initialize(browserShell::disable_apply_button);
#ifndef NEW_UI
   ((gtDialogTemplate *) shell)->show_button(1, 0);
#endif
}


gtMenuBar* browserShell::build_body()
{
    Initialize(build_body);
#ifndef NEW_UI
    gtTopLevelShell* top_level = gtTopLevelShell::create(NULL, "browser");

    genString title_string;
    title_string.printf("ParaSET Browser: %s", layout_name.str());
    top_level->title(title_string);
    top_level->icon_title ("Browser");

    top_level->override_WM_destroy(browserShell::wm_close);

    shell = top_level;

    gtMainWindow* main_window = gtMainWindow::create(shell, "main_window", 0);
    main_window->manage();

    gtPanedWindow* paned = gtPanedWindow::create(main_window, "paned_window");
    paned->attach(gtTop);
    paned->attach(gtLeft);
    paned->attach(gtRight);

    assist_line = gtStringEditor::create(main_window, "assist_line", " ");
    assist_line->read_only();
    assist_line->show_cursor(0);
    assist_line->attach(gtRight);
    assist_line->attach(gtLeft);
    assist_line->attach(gtBottom, NULL, 0);
    assist_line->alignment(gtBeginning);
    assist_line->manage();

    paned->attach(gtBottom, assist_line, 0);
    paned->manage();
    browser_container = paned;

    project_browser = new projectBrowser(this, projNode::get_project_rtl());

    return main_window->menu_bar();
#else
    return NULL;
#endif
}


extern "C" void vs_set_status_line_message(Widget,char*);
void browserShell::display_assist_line( const char *txt, int was_browser)
{
    Initialize(browserShell::display_assist_line);
#ifndef NEW_UI
    if (was_browser) {
      // Loop throught instances to find browser (avoid selection dialogs & so on)
      for(int index = browserShell::instance_array.size() - 1; index >= 0; index--){
	browserShellPtr instance = *browserShell::instance_array[index];
	if(instance->browser_container){
	  instance->assist_line->text(txt);
	}
      }
    } else
	viewerShell::set_status_line(txt);
#endif
}


gtBase* browserShell::container()
{
    Initialize(browserShell::container);

    return browser_container;
}

#define PUSHBUTTON(menu,title,name) \
	gtMenuStandard, quote(menu) "_" quote(name), title, \
	this, browserShell::paste3(menu,_,name)

#define DEADBUTTON(menu,title,name) \
	gtMenuStandard, quote(menu) "_" quote(name), title, \
	this, NULL

#define CASCBUTTON(menu,title,name) \
	gtMenuCascade, quote(menu) "_" quote(name), title, \
	NULL, NULL

#define HELPBUTTON(menu,title,name) \
	gtMenuStandard, quote(menu) "_" quote(name), title, \
	this, gtBase::help_button_callback

#define SEPARATOR \
	gtMenuSeparator, "sep"

#define ENDMENU \
	gtMenuNull

//
// Remove 'funny' characters from a widget name. 
//
static void widget_proper_name(const char *str, genString& name)
{
  Initialize(widget_proper_name);

  int len = strlen(str);

  for (int i = 0; i < len; i++)
    if (isalnum(str[i]))
      name += str[i];
    else
      name += '_';
}

void browserShell::build_query_menus(gtMenuBar* menubar)
{
    Initialize(browserShell::build_query_menus);

    int sz = cli_menus ? cli_menus->size() : 0;
    for(int ii=0; ii<sz; ++ii){
      cliMenu *cli_menu = (cliMenu*) (*cli_menus)[ii];
      build_query_menu(cli_menu);
    }
} 

//
// sschmidt 10/25/1999:  This version of query_button_add_item allows
// the addition of menu items to query menus from C code.  Currently 
// it does not know how to deal with requests to add submenus.  This 
// function is called from browserShell::add_item_to_cliMenu().  
//
static void query_button_add_item (browserQueryMenu *bm,  int first, cliQueryData *data, void *callback)
{
   const char *title = data->get_name();

   browserShell *bs = bm->browser;
   gtCascadeButton *bt = bm->button;

   if(first)
     bt->remove_entry(0);

   browserQueryData *bqd = new browserQueryData;
   bqd->menu = bm;
   bqd->data = data;

   cliMenuItem *mitem = cliQueryData_get_cliMenuItem(data);

   genString widget_name;
   widget_proper_name(title, widget_name);
 
   if (mitem) {
     switch (mitem->mtype)
     {
       case MENU_ITEM_SEPARATOR: bt->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull); break;
       case MENU_ITEM_ACTION:    bt->insert_entries(-1, gtMenuStandard, (char*)widget_name, title, bs, callback, gtMenuNull); break;
       default:                  break;
     }
   }
   else
     // Paranoid redundancy (every cliQueryData item
     // should have an associated cliMenuItem, but if
     // not, perform the default (read: "old") action):
     //
     bt->insert_entries(-1, gtMenuStandard, (char*)widget_name, title, bs, callback, gtMenuNull);
}

void cliQueryCallback(gtPushButton*, gtEvent*, void* data, gtReason);

static void query_button_add_item(browserQueryMenu*bm,  int first, cliQueryData *data)
{
   const char*title = data->get_name();

   browserShell* bs = bm->browser;
   gtCascadeButton*bt = bm->button;

   if(first)
     bt->remove_entry(0);

   browserQueryData*bqd = new browserQueryData;
   bqd->menu = bm;
   bqd->data = data;
   void * callback = (void *)cliQueryCallback;

   cliMenuItem *mitem = cliQueryData_get_cliMenuItem(data);

   genString widget_name;
   widget_proper_name(title, widget_name);
 
   if (mitem) {
     switch (mitem->mtype)
     {
       case MENU_ITEM_SUBMENU:   bs->build_query_menu(mitem->menu, bt); break;
       case MENU_ITEM_SEPARATOR: bt->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull); break;
       case MENU_ITEM_ACTION:    bt->insert_entries(-1, gtMenuStandard, (char*)widget_name, title, bqd, callback, gtMenuNull); break;
       default:                  break;
     }
   }
   else
     // Paranoid redundancy (every cliQueryData item
     // should have an associated cliMenuItem, but if
     // not, perform the default (read: "old") action):
     //
     bt->insert_entries(-1, gtMenuStandard, (char*)widget_name, title, bqd, callback, gtMenuNull);
}

void  browserShell::build_query_menu( cliMenu*cli_menu, gtCascadeButton *mbtn)
{
    const char *name = cli_menu->get_name();

    genString widget_name;
    widget_proper_name(name, widget_name);

    gtMenuBar* menubar = top_menubar;

    genString child;
    menu_child_name(name, child);
    const char *title = child;

    int sz = cli_menu->items.size();

    if (mbtn) {
      // Cascading sub-menu:
      //
      mbtn->insert_entries(-1, gtMenuCascade, (const char *)widget_name, title, NULL, NULL, gtMenuNull);
      this->query_cascade_btn = (gtCascadeButton *)mbtn->button((const char *)widget_name);
    }
    else {
      genString parent;
      menu_parent_name(name, parent);
      // Only worry about top-level menus:
      //
      if (parent.is_null())
        this->query_cascade_btn = gtCascadeButton::create(menubar, (const char *)widget_name, title, NULL, NULL);
      else
	return;
    }
    browserQueryMenu* bqm = new browserQueryMenu;
    bqm->browser = this;
    bqm->button = this->query_cascade_btn;

    cliMenu_put_browserQueryMenu(cli_menu, bqm);

    this->query_cascade_btn->pulldown_menu(	"query_menu",
	DEADBUTTON(browser, "No queries defined", dead),
	ENDMENU);

    this->query_cascade_btn->manage();

    for(int ii=0; ii<sz; ++ii){
        cliQueryData*cqd = (cliQueryData*)cli_menu->items[ii];
        query_button_add_item( bqm, ii==0, cqd); 
    }
}

//
// This function adds a menu item to a cliMenu.  It works for menu
// items which take selections and for those which don't.
//
void browserShell::add_item_to_cliMenu(cliMenu *menu, const char *name, void *callback)
{
    Initialize(browserShell::add_to_cliMenu);

    if(menu) {
	int first = (menu->items.size()==0);
        cliQueryData *cqd = new cliQueryData(name);
	cliMenuItem *mitem = new cliMenuItem(name, MENU_ITEM_ACTION, menu);
	cliQueryData_put_cliMenuItem(cqd, mitem);

	menu->items.insert_last(cqd);

        Obj *dset = cliMenu_get_browserQueryMenus(menu);
        Obj *el;
        ForEach(el, *dset){
            browserQueryMenu *bqm = (browserQueryMenu*)el;
            query_button_add_item (bqm, first, cqd, callback); 
        }
    }
}

void browserShell::add_query_submenu(cliMenu* menu, int ind)
{
    Initialize(browserShell::add_query_submenu);

    cliQueryData* cqd = (cliQueryData*)menu->items[ind];
    Obj* dset = cliMenu_get_browserQueryMenus(menu);
    Obj*el;
    ForEach(el, *dset){
        browserQueryMenu*bqm = (browserQueryMenu*)el;
        query_button_add_item( bqm, ind==0, cqd); 
    }
}

void browserShell::add_query_menu(cliMenu* menu)
{
    Initialize(browser_add_query_menu);

    for(int ii = 0; ii < instance_array.size(); ++ii) {
	browserShell* bs = *instance_array[ii];
	bs->build_query_menu(menu);
    }
}

extern "C" int isUpdateAvailable() {
  static int bUpdateAvailable = -1;
  if(bUpdateAvailable == -1)
    bUpdateAvailable = (customize::getIntPref("DIS_internal.DebugMenu_Enabled") > 0);
  return bUpdateAvailable;
} 

//
// The main entry for the pull down menu of "file".
// Note that some of the entries have callbacks beginning
// with browser_;  these were moved from the Browser menu.
//
void browserShell::build_file_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_file_menu);
#ifndef NEW_UI
    gtCascadeButton* cascade_btn = gtCascadeButton::create(
	menubar, "file", TXT("File"), NULL, NULL);

    if(isUpdateAvailable()) {
      cascade_btn->pulldown_menu(
	   "file_menu",

	   PUSHBUTTON(file,TXT("Save All Files..."),save_all_files),
	   SEPARATOR,
	   PUSHBUTTON(file,TXT("Load"),preload),
	   PUSHBUTTON(file,TXT("Update"),update),
	   PUSHBUTTON(file,TXT("Unload"),unload),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("CM..."),cm),
	   PUSHBUTTON(browser,TXT("Preferences..."),preferences),
	   PUSHBUTTON(browser,TXT("Messages..."),messages),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("New Browser"),new_browser_window),
	   PUSHBUTTON(browser,TXT("Close"),close_window),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("Quit ParaSET"),quit),
	   ENDMENU);
    } else {
      cascade_btn->pulldown_menu(
	   "file_menu",

	   PUSHBUTTON(file,TXT("Save All Files..."),save_all_files),
	   SEPARATOR,
	   PUSHBUTTON(file,TXT("Load"),preload),
	   PUSHBUTTON(file,TXT("Unload"),unload),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("CM..."),cm),
	   PUSHBUTTON(browser,TXT("Preferences..."),preferences),
	   PUSHBUTTON(browser,TXT("Messages..."),messages),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("New Browser"),new_browser_window),
	   PUSHBUTTON(browser,TXT("Close"),close_window),
	   SEPARATOR,
	   PUSHBUTTON(browser,TXT("Quit ParaSET"),quit),
	   ENDMENU);
    }

    cascade_btn->manage();
#endif
}

void browserShell::browser_manage_menus (int sensitivity)
{
    for(int i = 0; i < browserShell::instance_array.size(); ++i){
	browserShellPtr instance = *browserShell::instance_array[i];
	if (instance)
	    instance->browser_manage_menu (sensitivity);
    }
}

void browserShell::browser_manage_menu (int sensitivity)
{
    Initialize(browserShell::browser_manage_menu);
#ifndef NEW_UI
    gtPrimitive* prim;

    if (!manage_button) return;

    prim = manage_button->button ("project_get");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_get2");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_put");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_copy");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_diff");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_lock");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_unlock");
    if (prim) prim->set_sensitive (sensitivity);
    prim = manage_button->button ("project_unget");
    if (prim) prim->set_sensitive (sensitivity);
#endif
}

#ifndef NEW_UI
extern "C" const char* customize_getStrPref_AsIs(const char *);

int getPrefsToolText( char* prefsKey, genString& title, genString& tool, int* outFlag ) 
{
    title = ""; tool = ""; *outFlag = 0;

    char *p, *q, *s;
    const char* sprefs;
    sprefs = customize_getStrPref_AsIs(prefsKey);
    if ( sprefs && *sprefs )
    {
	char* stext= s = new char [ strlen(sprefs) +1];
	if ( stext ) 
	{
	    strcpy( s, sprefs);
	    p= q = strchr(s, ',');
	    *p++ = 0;
	    if ( *s == 0 ) 
		s=prefsKey;
	    else
	    {
		q--;
		while ( q > s && (*q == ' ' || *q == '\t' ) ) q--;
		*(q+1) = 0;
		if ( *s && *s == '"' && *q == '"' )
		{
		   s++; *q=0;
		}
	    }
	    
	    if ( *p == ',' )
	    {
		*outFlag=1;
		p++;
	    }
	    while ( *p == ' ' || *p == '\t' ) p++;
	    q = p+ strlen(p) -1;
	    if ( *p && *p == '"' && *q == '"' )
	    {
	        p++; *q=0;
	    }

	    title = s;
	    tool = p;
	    delete stext;
	    return 1;
	}
    }
    return 0;
}
#endif

//
// the routine to handle the pull down menu for manage in the project browser
//
void browserShell::build_project_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_project_menu);
#ifndef NEW_UI
  if (customize::getIntPref("DIS_internal.OldManageMenu") <= 0) {
    manage_button = gtCascadeButton::create(menubar, "cm", TXT("CM"), NULL, NULL);
    manage_button->pulldown_menu(
	"cm_dropdown",
	SEPARATOR, // for correct removing items in browserShell::buildCMMenu()
	SEPARATOR, //
	ENDMENU);
    manage_button->set_sensitive(0);
    manage_button->manage();
  } else {
    manage_button = gtCascadeButton::create(
	menubar, "project", TXT("Manage"), NULL, NULL);

    manage_button->pulldown_menu(
	"project_menu",

	PUSHBUTTON(project,TXT("Home Project"),home_project),
//	PUSHBUTTON(project,TXT("Project Scope..."),project_scope),
	SEPARATOR,
	PUSHBUTTON(project,TXT("Get"),get),
	CASCBUTTON(project,TXT("More Get/Copy Options"),get2),
	PUSHBUTTON(project,TXT("Put..."),put),
	SEPARATOR,
	PUSHBUTTON(project,TXT("Copy"),copy),
	PUSHBUTTON(project,TXT("Diff"),diff),
	PUSHBUTTON(project,TXT("Lock"),lock),
	PUSHBUTTON(project,TXT("Unlock..."),unlock),
	SEPARATOR,
        PUSHBUTTON(project,TXT("UnGet"),unget),
	ENDMENU);

    genString widget_name;
    int   ntool = 0;
    int   outFlag;
    genString title, tool;
    if ( getPrefsToolText( "ManageTool1", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool1, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool2", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool2, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool3", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool3, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool4", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool4, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool5", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool5, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool6", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool6, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool7", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool7, gtMenuNull);
    }
    if ( getPrefsToolText( "ManageTool8", title, tool, &outFlag ) )
    {
	if ( ntool == 0 )
	    manage_button->insert_entries(-1, gtMenuSeparator, "sep", gtMenuNull );
	setTool(++ntool, title, tool, outFlag);
	widget_proper_name(getToolTitle(ntool), widget_name);
	manage_button->insert_entries(-1, gtMenuStandard, (char *)widget_name, getToolTitle(ntool), this, browserShell::project_Tool8, gtMenuNull);
    }



    browser_manage_menu (projNode::home_is_writeable() && !customize::no_cm());

    manage_button->manage();

    gtCascadeButton* get_button =
	(gtCascadeButton*)manage_button->button("project_get2");

    get_button->pulldown_menu(
	"make_menu",
	gtMenuStandard, "project_get2",  TXT("Get with Options..."),
	this, browserShell::project_get2,
	gtMenuStandard, "project_get_version", TXT("Copy by Version..."),
	this, browserShell::project_get_version,
	gtMenuStandard, "project_get_date", TXT("Copy by Date (after)..."),
	this, browserShell::project_get_date,
        gtMenuNull);
  }
#endif
}

void browserShell::build_view_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_view_menu);
#ifndef NEW_UI
    gtCascadeButton* cascade_btn = gtCascadeButton::create(
	menubar, "view", TXT("View"), NULL, NULL);

    cascade_btn->pulldown_menu(
	"view_menu",
			       
	PUSHBUTTON(view,TXT("New Viewer"),new_viewer),
	SEPARATOR,
	PUSHBUTTON(view,TXT("Text"),text),
	PUSHBUTTON(view,TXT("Flowchart"),flowchart),
	PUSHBUTTON(view,TXT("Outline"),outline),
	PUSHBUTTON(view,TXT("ERD"),relationships),
	PUSHBUTTON(view,TXT("Data Chart"),structures),
	PUSHBUTTON(view,TXT("Inheritance"),inheritance),
	PUSHBUTTON(view,TXT("Call Tree"),call_tree),
	PUSHBUTTON(view,TXT("Graph"),graph),
	ENDMENU);

    cascade_btn->manage();
#endif
}

void browserShell::build_utility_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_utility_menu);

#ifndef NEW_UI
    gtCascadeButton* cascade_btn = gtCascadeButton::create(menubar, "utility", TXT("Tools"), NULL, NULL);

    cascade_btn->pulldown_menu(
	"utility_menu",
	PUSHBUTTON(utility,TXT("Group Manager"),groupManager),
	SEPARATOR,
	PUSHBUTTON(utility,TXT("Impact"),check_impact),
	PUSHBUTTON(utility,TXT("Dormant"),find_dormant_code),
	PUSHBUTTON(utility,TXT("Simplify Headers"),simplify_headers),
	SEPARATOR,
	PUSHBUTTON(utility,TXT("Caliper"),qualityCockpit),
	ENDMENU);
    
    {
      gtPrimitive* prim;
      prim = cascade_btn->button ("utility_check_impact");
      if (prim) prim->set_sensitive(1);
    }

    if (_lf(LIC_DORMANT)) {
      gtPrimitive* prim;
      prim = cascade_btn->button ("utility_find_dormant_code");
      if (prim) prim->set_sensitive(0);
    }

    if (_lf(LIC_SIMPLIFY)) {
      gtPrimitive* prim;
      prim = cascade_btn->button ("utility_simplify_headers");
      if (prim) prim->set_sensitive(0);
    }

    if (_lf(LIC_DELTA)) {
      gtPrimitive* prim;
      prim = cascade_btn->button ("utility_mod_objects");
      if (prim) prim->set_sensitive(0);
    }

    if (_lf(LIC_QAC)) {
      gtPrimitive* prim;
      prim = cascade_btn->button ("utility_qualityCockpit");
      if (prim) prim->set_sensitive(0);
    }

    cascade_btn->manage();
#endif
}

void browserShell::journal_manage_menus ()
{
    for(int i = 0; i < browserShell::instance_array.size(); ++i){
        browserShellPtr instance = *browserShell::instance_array[i];
        if (instance)
            instance->journal_manage_menu ();
    }
}
 
void browserShell::journal_manage_menu ()
{
    Initialize(browserShell::journal_manage_menu);
#ifndef NEW_UI 
    gtPrimitive* prim;
    int on=0;
 
    if (cmd_current_journal_name) 
	on = 0;
    else on = 1;
  
    prim = cascade_button->button ("debug_save_journal");
    prim->set_sensitive(!on);
    prim = cascade_button->button ("debug_new_journal");
    prim->set_sensitive(on);
    prim = cascade_button->button ("debug_execute_journal");
    prim->set_sensitive(on);
    prim = cascade_button->button ("debug_resume_execution");
    prim->set_sensitive(on);
#endif
}


void browserShell::build_debug_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_debug_menu);
#ifndef NEW_UI
    cascade_button = gtCascadeButton::create(
	menubar, "debug", TXT("DEBUG"), NULL, NULL);

    cascade_button->pulldown_menu(
	"debug_menu",

	PUSHBUTTON(debug,TXT("Break"),break),
	PUSHBUTTON(debug,TXT("Eval"),eval),
	PUSHBUTTON(debug,TXT("Print Node"),print_selected_node),
	CASCBUTTON(debug,TXT("Follow"),follow),
	PUSHBUTTON(debug,TXT("Print Tree"),print_selected_tree),
	PUSHBUTTON(debug,TXT("Print Symbol"),print_selected_symbol),
	SEPARATOR,
	PUSHBUTTON(debug,TXT("Start Metering"),start_metering),
	PUSHBUTTON(debug,TXT("Stop Metering"),stop_metering),
	PUSHBUTTON(debug,TXT("Suspend Metering"),suspend_metering),
	PUSHBUTTON(debug,TXT("Resume Metering"),resume_metering),
	SEPARATOR,
	PUSHBUTTON(debug,TXT("New Journal..."),new_journal),
	PUSHBUTTON(debug,TXT("Save Journal"),save_journal),
	PUSHBUTTON(debug,TXT("Resume Execution"),resume_execution),
	PUSHBUTTON(debug,TXT("Execute Journal..."),execute_journal),
	SEPARATOR,
	PUSHBUTTON(debug,TXT("Edit Help Index"),edit_help_index),
	PUSHBUTTON(debug,TXT("Reset Help Index"),reset_help_index),
	PUSHBUTTON(debug,TXT("Toggle Help Pathnames"),toggle_help_pathnames),
	PUSHBUTTON(debug,TXT("Toggle Helper Shells"),toggle_use_helper_shell),
	SEPARATOR,
	PUSHBUTTON(debug,TXT("Clear All Selections"),clear_all_selections),
	SEPARATOR,
	ENDMENU);

    journal_manage_menu();
    cascade_button->manage();
	

    gtCascadeButton* follow_button =
	(gtCascadeButton*)cascade_button->button("debug_follow");

    follow_button->pulldown_menu(
	"follow_menu",

        gtMenuStandard, "debug_follow_0",	TXT(" 0 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_1",	TXT(" 1 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_2",	TXT(" 2 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_3",	TXT(" 3 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_4",	TXT(" 4 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_5",	TXT(" 5 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_6",	TXT(" 6 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_7",	TXT(" 7 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_8",	TXT(" 8 "),
        this, browserShell::debug_follow,
        gtMenuStandard, "debug_follow_9",	TXT(" 9 "),
        this, browserShell::debug_follow,
	gtMenuNull);
#endif
}

void browserShell::build_help_menu(gtMenuBar* menubar)
{
    Initialize(browserShell::build_help_menu);
#ifndef NEW_UI
    gtCascadeButton* cascade_btn = gtCascadeButton::create(
	menubar, "help", TXT("Help"), NULL, NULL);

    menubar->set_help(cascade_btn);

    cascade_btn->pulldown_menu(
	"help_menu",
	HELPBUTTON(help,TXT("Contents"),index),
	SEPARATOR,
	PUSHBUTTON(browser,TXT("About ParaSET..."),about_paraset),
	ENDMENU);

    REG_HELP(cascade_btn->button("help_index"), "Pset.Help.Index");

    cascade_btn->manage();
#endif
}

void browserShell::build_interface()
{
    Initialize(browserShell::build_interface);
#ifndef NEW_UI
    gtMenuBar* menubar = build_body();
    top_menubar = menubar;
    query_cascade_btn = NULL;
    build_file_menu(menubar);
    build_project_menu(menubar);
    build_view_menu(menubar);
    build_utility_menu(menubar);
    buildCMMenu();

    if (customize::getIntPref("DIS_internal.DebugMenu_Enabled") > 0) {
	build_debug_menu(menubar);
    }

    build_query_menus(menubar);

    build_help_menu(menubar);
    delete_choose_home();
#endif
}

//////////////////////////////////////////////////////////////////////////
//                       Browser Selection Dialog			//
//////////////////////////////////////////////////////////////////////////

void browserShell::build_dialog(gtBase* parent, const char* prompt)
{
    Initialize(browserShell::build_dialog);
#ifndef NEW_UI
    const char* browser_name = "selection_browser";
    const char* browser_title = "Selection browser";
    
    gtDialogTemplate* dlg = gtDialogTemplate::create(parent, browser_name, browser_title);
    dlg->add_default_buttons(ok_CB, this, apply_CB, this, cancel_CB, this);
    dlg->default_button(0);
    
    shell = dlg;
    browser_container = dlg;
    
    gtLabel* label;
    
    label = gtLabel::create(browser_container, "prompt", prompt);
    label->manage();
    
    gtFrame* frame;
    
    project_browser = new projectBrowser(this, projNode::get_project_rtl());
    
    assist_line = gtStringEditor::create(browser_container, "assist_line", " ");
    assist_line->editable(0);
    assist_line->set_sensitive(0);
    assist_line->attach(gtRight);
    assist_line->attach(gtLeft);
    assist_line->attach(gtBottom, NULL, 0);
    assist_line->alignment(gtBeginning);
    assist_line->manage();
    
    gtForm::vertStack(label, project_browser->top());
    dlg->popup(1);
    project_browser->top()->attach (gtBottom, assist_line);
#endif
}

void browserShell::ok_CB(gtPushButton*, gtEventPtr, void* bsh_data, gtReason)
{
    Initialize(browserShell::ok_CB);
#ifndef NEW_UI
    browserShell* bsh = (browserShell*)bsh_data;

    if(bsh->project_selection)
    {
        symbolArr& selection = bsh->project_browser->symbol_selection();
        (*bsh->project_selection)(bsh->project_selection_data, selection);
   } else {
        symbolArr arr;
        (*bsh->project_selection)(0, arr);
    }

    delete bsh;
#endif
}

void browserShell::apply_CB(gtPushButton*, gtEventPtr, void* bsh_data, gtReason)
{
    Initialize(browserShell::apply_CB);
#ifndef NEW_UI
    browserShell* bsh = (browserShell*)bsh_data;

    if(bsh->project_selection)
    {
        symbolArr& selection = bsh->project_browser->symbol_selection();
        (*bsh->project_selection)(bsh->project_selection_data, selection);
    }
#endif
}

void browserShell::cancel_CB(gtPushButton*, gtEventPtr, void* bsh_data, gtReason)
{
    Initialize(browserShell::cancel_CB);
#ifndef NEW_UI
    browserShell* bsh = (browserShell*)bsh_data;

    symbolArr arr;
    (*bsh->project_selection)(0, arr);

    delete bsh;
#endif
}

// static function to reset all the browsers (normally only one)
//    this function is equivalent to user pressing RESET on each
//    browser
void browserShell::reset_all()
{
    for (int i=0; i<instance_array.size(); i++) {
	browserShell* bs = *instance_array[i];
	projectBrowser* pb = bs->project_browser;
	pb->reset();
    }
}

// static function to unmap all the browsers
void browserShell::unmap_all()
{
#ifndef NEW_UI
    for (int i=0; i < instance_array.size(); i++) 
    {
	browserShell* bs = (instance_array[i]) ? *instance_array[i] : NULL;
	gtShell *sh = (bs) ? (bs)->top_level() : NULL;
	if (sh) sh->popdown();
    }
#endif
}

// function to fill in the selected items in all browsers
// (for adding and removing groups elements)
void browserShell::toAllBrowsers(projectBrowserMember fp, groupHandle *handle)
{
    int i, cont_scan = 1;

    for (i=0; i<instance_array.size() && cont_scan; i++) {
	browserShell* bs = *instance_array[i];
	projectBrowser* pb = bs->project_browser;
	cont_scan = (pb->*fp)(handle);
    }
}

//------------------------------------------
// browserShell::set_closure_state [static]
//------------------------------------------

void browserShell::set_closure_state(bool closure_enabled) {
   Initialize(browserShell::set_closure_state);

   for (int i = 0; i < instance_array.size(); i++) {
      browserShell* bs = *instance_array[i];
      if (bs) {
	 projectBrowser* pb = bs->project_browser;
	 if (pb) {
	    pb->closure(closure_enabled);
	 }
      }
   }
}

//----------------------------------------------------------------------------

//
// get selections from all browsers
//

void browserShell::get_all_browser_selections (symbolArr& arr, bool include_projects)
{
    Initialize (browserShell::get_all_browser_selections);

    browserShell*	bs;
    projectBrowser*	pb;

    for (int i = 0; i < instance_array.size (); i++) {
	bs = *instance_array[i];
	pb = bs->project_browser;
	arr.insert_last (pb->symbol_selection ());
	if (include_projects) {
	   arr.insert_last(pb->project_selection());
	}
    }
}

//----------------------------------------------------------------------------

//
// clear selections in all browsers
//

void browserShell::clear_all_browser_selections (browserShell* keep)
{
    Initialize (browserShell::clear_all_browser_selections);

    browserShell*	bs;
    projectBrowser*	pb;

    for (int i = 0; i < instance_array.size (); i++) {
	bs = *instance_array[i];
	if (bs != keep) {
	    pb = bs->project_browser;
	    pb->deselect ();
	}
    }
}

//----------------------------------------------------------------------------


void browserShell::utility_groupManager(gtPushButton*, gtEventPtr, void*, gtReason)
{
    Initialize(browserShell::utility_groupManager);
    SetsUI::Invoke(NULL);
}

void browserShell::utility_qualityCockpit(gtPushButton*, gtEventPtr, void *bshell, gtReason)
{
  Initialize(browserShell::utility_qualityCockpit);
  
  browserShell *browser = (browserShell *)bshell;
  
  if ( browser && browser->project_browser )
    {
      // force browser into scan mode or we cannot pick up the appropriate current project
      browser->project_browser->scan_mode();
      QualityCockpitUI::Invoke(NULL);
    }
}

void browserShell::utility_temp_null_function(gtPushButton*, gtEventPtr, void*, gtReason)
{
}
