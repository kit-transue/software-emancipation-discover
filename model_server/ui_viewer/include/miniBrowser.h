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
#ifndef _miniBrowser_h
#define _miniBrowser_h

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _representation_h
#include <representation.h>
#endif

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef _Relational_h
#include <Relational.h>
#endif

#ifndef _gtOptionMenu_h
#include <gtOptionMenu.h>
#endif

#ifndef _symbolScope_h
#include <symbolScope.h>
#endif

#ifndef _genMask_h
#include <genMask.h>
#endif

RelClass(miniBrowserAPI);
class miniBrowser;
class InstanceController;

class BrowserController;
class genMask;

class miniBrowserRTL_API : public Relational {
  public:
    miniBrowserRTL_API();
    virtual ~miniBrowserRTL_API() {};
    // Function get called when selecting in rtl
    virtual void selection_callback(RTL* rtl, symbolArr& selected, miniBrowser *); 
    // Function get called when selection in rtl is made
    // should return 0 if there are no need in default symbol action processing
    // non 0 otherwise
    virtual int  action_callback(RTL* rtl, symbolArr& selected, miniBrowser *); 
    // Function gets called when minibrowser decides to delete this rtl (out of history)
    // Should return 0 if history manager should not delete this rtl
    virtual int  rtl_dying(RTL *rtl);
    // Function gets called when rtl goes out of history list & not dispayed anywhere
    // in minibrowser
    virtual void rtl_out_of_scope(RTL *rtl);
    // Function called from GTRTL to get sort specifications
    // If it return NULL GTRTL will use filter specifications
    virtual char *get_sort_spec(RTL *rtl);
    // Function should return non 0 if needs set of special buttons (vcr controls)
    // under the list (buttonst are next, prev, current)
    virtual int  need_vcr_control(RTL *rtl);
    // Function called to let api create it\'s own buttons
    // for list control form. (Should use own add_button method)
    // parent should be used as a parent for buttons
    virtual void make_own_buttons(gtBase *parent, gtRTL *gt_rtl, RTL *rtl);
    // Call this function from make_own_buttons to add custom button
    // to the list control form (list supports up to 7 buttons)
    virtual void add_button(gtPrimitive *);
    // Returns control form which might be used as a parent from custom buttons
    virtual gtBase *get_button_control(void);
    // Function called to set list (for internal use only)
    virtual void set_list(gtList *);
  private:
    gtList *list;
};

defrel_one_to_many(miniBrowserRTL_API,APIhandler,RTL,rtl);

typedef void (*FormCleanupCallbackProc)(miniBrowser *mb, void *data);

class miniBrowser
{
  public:
    miniBrowser(void *parent_menu, void *parent_icon_place, void *parent); /* void* - Widgets (using void * because do not 
						                              want to include intrisincs.h 
						                            */
    ~miniBrowser();
    static void clear_selections(void);
    static void fill_selected_nodes(symbolArr& array);
    void   browse(RTLPtr rtl_head, int position = 1);    // Browse list
    void   browse(appTree *app_node, int position = 1);  // Browse list, and removes the list when it goes away
    void   browse_vieweable(appTree *app_node, int position = 1);  // Browse list, and removes the list when it goes away
    void   switch_window(void);
    int    get_next_window(void);  // returns 0 - top window, 1 - bottom window
    void   remove(RTL *rtl);
    void   remove_from_history(RTLPtr rtl);
    gtBase *get_paned_window();
    gtBase *get_main_form();
    void   manage_into_paned_form(gtForm *form, int preffered_height = 0);
    void   manage_into_main_form(gtForm *form);
    void   visible(int flag);
    int    visible(void);
    void   closure(bool enabled);
    void   set_output_window(int index);
    void   cleanup_lists();
    // Unmanages list from form.  list == 0 - top list list != 0 bottom list
    // Create new form for elememt placing and return
    // FormCleanupCallbackProc will be called if minibrowser closes that form
    gtForm *unmanage_list(int list);
    void   set_list_cleanup_form(FormCleanupCallbackProc proc, void *data);
    void   *get_cleanup_form_data(void);
    // Restores unmanaged list
    int    restore_list();
    void   refresh(void);

    static void list1_select(gtList*, gtEventPtr e, void* mb, gtReason);
    static void list2_select(gtList*, gtEventPtr e, void* mb, gtReason);
    static void list1_action(gtList*, gtEventPtr e, void* mb, gtReason);
    static void list2_action(gtList*, gtEventPtr e, void* mb, gtReason);

  private:
    gtForm          *mainForm;
    gtRTL           *selected_list;
    gtRTL           *unmanaged_list;
    gtRTL           *list1;
    gtRTL           *list2;
    gtForm          *list1Form;
    gtForm          *list2Form;
    gtForm          *new_form;
    gtPanedWindow   *paned;
    gtPushButton    *on_button;
    gtPushButton    *off_button;
    gtToggleButton* closure_button;
    gtCascadeButton *history_button;
    gtCascadeButton *group_button;
    
    gtCascadeButton *query_menu;

    symbolArr       selected_symbols;
    symbolArr       list1_selection;
    symbolArr       list2_selection;
    objArr          history_list;
    symbolScope     scope;
    int             displayed;
    void            *parent_viewershell; /* Widget */
    RTL             *list1_prev_rtl;
    RTL             *list2_prev_rtl;
    FormCleanupCallbackProc form_cleanup_callback;
    void                    *cleanup_callback_data;

    void   title1(char *txt);
    void   title2(char *txt);
    void   rtl_select(int extend);
    void   list_action(void);
    void   open_module(symbolPtr sym);
    void   clear_selection();
    void   add_rtl_to_history(RTLPtr rtl, int position = 1, int rtl_num = 0);
    void   make_list2_visible(void);
    void   init_search_path(symbolArr& selection);
    void   write_list(gtRTL *list);
    void   report_out_of_scope(RTLPtr rtl);
//  static void list1_select(gtList*, gtEventPtr e, void* mb, gtReason); // made public
//  static void list2_select(gtList*, gtEventPtr e, void* mb, gtReason);
//  static void list1_action(gtList*, gtEventPtr e, void* mb, gtReason);
//  static void list2_action(gtList*, gtEventPtr e, void* mb, gtReason);
    static void PopupBrowser(gtCascadeButton* b, gtEvent*, void* mb, gtReason);
    static void ToggleBrowser(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void QueryButton(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void SlideButton(gtArrowButton* b, gtEvent*, void* mb, gtReason);
    // static void CategoriesButton(gtPushButton* b, gtEvent*, void* mb, gtReason);

    static void GroupButton(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void GroupManagerButton(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void SaveList1_CB(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void SaveList2_CB(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void history_CB(gtPushButton* b, gtEvent*, void* mb, gtReason);
    static void closure_CB(gtToggleButton*, gtEvent*, void*, gtReason);

    static void update_query_menu_CB(void* data, caddr_t, gtEventPtr);

    void update_query_menu();
    void remove_queries();
    int num_queries;
    genMask old_selection;

    BrowserController* pBController;
};

typedef miniBrowser* miniBrowserPtr;

void cleanup_miniBrowsers();

#endif






