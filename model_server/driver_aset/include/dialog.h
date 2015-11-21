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
#if 0 //#ifndef _dialog_h
#define _dialog_h

// dialog.h
//------------------------------------------
// synopsis:
// Abstract base class of all dialogs.
//
// description:
// A dialog contains one or more views with a
// common insertion point.  It also contains
// some other bookkeeping information.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#ifndef  genArr_h
#include "genArr.h"
#endif
#ifndef  _genString_h
#include "genString.h"
#endif
#ifndef  _objRelation_h
#include "objRelation.h"
#endif
#ifndef  _objOper_h
#include "objOper.h"
#endif
#ifndef  _objCollection_h
#include "objCollection.h"
#endif
#ifndef   ARRAYOB_H
#include "ArrayOb.h"
#endif
#ifndef  _representation_h
#include "representation.h"
#endif
#ifndef  _dialogType_h
#include "dialogType.h"
#endif

//#include "ldrRTLDisplayType.h"


// classes referenced

RelClass(app);
RelClass(appTree);
RelClass(ldr);
RelClass(ldrTree);
RelClass(view);
RelClass(viewTree);

class genString;

class viewSelection;
class OperPoint;
class ldrHeader;

struct dialogView
{
    viewPtr view;
    void*   window;
    void*   horz_scroll;
    void*   vert_scroll;
    void*   panner;
    
    void zero()
    { view = NULL; window = horz_scroll = vert_scroll = panner = NULL; }
};

genArr(dialogView);

//
// dialog
//
class dialog : public Relational
{
  protected:
    dialogType diag_type;		// SPD, STE, etc.
    genString  title;			// Title of window
    void*      dialog_swidget;		// Swidget of dialog window (maybe)
    void       (*detach_hook)(void*,int); // function for detach_view to call
    void*      detach_data;		// data to pass to detach_hook

    viewPtr    current_view;		// Pointer to current view
    char*      current_text;		// Current text buffer
    OperPoint* oper_pt;			// Current insertion point

    int        num_views;		// Number of views allocated
    dialogViewArr view_array;
    
    int max_view_num() { return view_array.size(); }

    viewPtr& get_view(int i)        { return view_array[i]->view; }
    void*&   get_window(int i)      { return view_array[i]->window; }
    void*&   get_horz_scroll(int i) { return view_array[i]->horz_scroll; }
    void*&   get_vert_scroll(int i) { return view_array[i]->vert_scroll; }
    void*&   get_panner(int i)      { return view_array[i]->panner; }

  public:
    define_relational(dialog, Relational);
    copy_member(dialog);

    static view* open_view (ldr*, repType, int level);

    static view* open_view(appTreePtr, repType, int level = 0, int = 0,
			     ldrRTLDisplayType = rtlDisplayUnknown);

    static ldrHeader* layout(appPtr, appTreePtr, repType, int, int,
			     ldrRTLDisplayType );

    dialog(char* title = (char *)"");
    dialog(const dialog&); 
    ~dialog();

    char* get_title() { return title; }
    char* get_view_name(int view_num);

    void set_title(char *title);

    void open_view(int view_num, appPtr, appTreePtr, repType,
		   int sort_order = 0, ldrRTLDisplayType = rtlDisplayUnknown);
    void set_view(int view_num, viewPtr);

    void close_view (int view_num, int delete_view = 1); // closes the view
    void close_view (viewPtr view, int delete_view = 1);

    void detach_view (viewPtr view);	// detaches view when it is deleted

    void  create_window(int view_num, void* win);
    void  close_window (int view_num);
    void  resize_window(void* win);

    void  change_view (void* win, int view_num, repType new_type,
		       appTreePtr subroot);
    void  change_view (void* win, int view_num, repType new_type);
    void  change_view (int view_num, appPtr header, appTreePtr subroot,
		       repType new_type);

    void  regenerate_view(viewPtr);

    // View iterator functions:
    int first_view() { return next_view(-1); }
    int next_view(int view_num);
    int empty_view();

    void  refresh();
    void  clear();

    boolean    contains_app(appPtr);

    viewPtr    get_current_view();
    void       set_current_view(viewPtr);
    boolean    contains_view(viewPtr);

    appPtr     get_current_app();

    OperPoint* get_current_pt();
    void       set_current_pt(OperPoint* pt);

    OperPoint* put_selection(viewSelection&);

    virtual void send_string( ostream& stream ) const;
    void         set_string(char*);

    void	manipulate_perspective( int view_num, int operation,
					int i1, int i2, int i3, int i4 );

    int         get_slider_style(int view_num);
    void        set_slider_style(int view_num, int style);

    void        send_view_to_ui (int view_num, void *data);
    viewPtr     get_view_ptr (int view_num);
    int         get_view_num (viewPtr);
    void*	window_of_view(viewPtr);

    void        register_scrollbars (int view_num, void* horz, void* vert);
    void        unregister_scrollbars(int view_num);

    void        register_panner (int view_num, void* panner);
    void        unregister_panner (int view_num);

    void        set_horiz_scrollbar_value (int view_num, int val);
    void        set_vert_scrollbar_value (int view_num, int val);

    void        set_horiz_slider_size (int view_num, int val);
    void        set_vert_slider_size (int view_num, int val);

    void	set_vert_scrollbar_range (int view_num, int min, int max);

    void set_detach_hook(void* data, void (*hook)(void*, int));
};
generate_descriptor(dialog,Relational);

inline viewPtr    dialog::get_current_view() { return current_view; }

inline void       dialog::set_current_view(viewPtr vw) { current_view = vw; }

inline OperPoint* dialog::get_current_pt() { return oper_pt; }

inline void       dialog::set_current_pt(OperPoint* pt) { oper_pt = pt; }

inline void       dialog::set_string(char* str)
{
    if (current_text != (char*)NULL) {
        // delete (current_text);
    }
    current_text = (char*) str;
}

//
// C functions
//
extern "C"
{
    char* dialog_name(void* dlg);
    void  dialog_set_name(void* dlg, char *name);
    char* dialog_view_name(void* dlg, int view_num);
    int   dialog_untitled(void* dlg);
    void  dialog_create_window(void* dlg, void* win);
    void  dialog_create_window_instance(int view_num, void* dlg, void* win);
    void  dialog_resize_window(void* dlg, void* win);
    void  dialog_view_change(void* dlg, void* win, char* name, repType type);
    void  dialog_change_view_type(void* dlg, void* win, repType type);
    void  dialog_manipulate_perspective(
	void* dlg, int view_num, int op, int i1, int i2, int i3, int i4 );
    void  dialog_open_view(
	void* dlg, int view_num, appTree *subroot, repType view_type);
    void  dialog_close_view(void* dlg, int view_num);
    void  dialog_send_view_to_ui(void* dlg, int view_num, void* data);
    int   dialog_get_view_type(void* dlg, int view_num);
    void* dialog_get_view_ptr(void* dlg, int view_num);
    void  dialog_register_scrollbars(
	void* dlg, int view_num, void* hscroll, void* vscroll);
    int   dialog_get_slider_style(void* dlg, int view_num);
    void  dialog_set_slider_style(void* dlg, int view_num, int style);
};
/*
   START-LOG-------------------------------------------

   $Log: dialog.h  $
   Revision 1.3 2000/07/07 08:08:22EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.3  1992/11/05  22:27:43  smit
 * unregister scrollbars when not necessary.
 *
 * Revision 1.2.1.2  1992/10/09  18:52:54  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif // _dialog_h
