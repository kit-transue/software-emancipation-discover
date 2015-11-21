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
#ifndef _gtListDialog_h
#define _gtListDialog_h

// gtListDialog.h
//------------------------------------------
// synopsis:
// ...
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#include <gtBase.h>
#include <gtDlgTemplate.h>

struct gtListDialogConnector
{
    gtPushButton* pb;
    void  (*func)(void *);
    void* data;
    int   counter;
};

class gtListDialog
{
  public:
    static gtListDialog *create(
	gtBase *p, const char *name, const char *dlg_title,
	const char *list_title, gtListStyle sty,
	const char **entries, int num_entries)
    { return new gtListDialog(p, name, dlg_title, list_title, sty,
			      entries, num_entries); }
    ~gtListDialog();

    void connect_to_button(int n, void (*joint_func)(void *),
			   void *client_data);
    void connect_to_button(gtPushButton *, void (*joint_func)(void *),
			   void *client_data);

    gtList* gt_list() { return l; }
    gtDialogTemplate* gt_template() { return tmp; }

    int add_button(const char *name, const char *label, gtPushCB callback,
		   void *client_data)
    {	return tmp->add_button(name, label, callback, client_data); }
    void add_help_button() { tmp->add_help_button(); }
    void add_default_buttons(
	const char *OK_label, gtPushCB OK_callback, void *OK_client_data,
	const char *apply_label, gtPushCB apply_callback, void *apply_client_data,
	const char *cancel_label, gtPushCB cancel_callback, void *cancel_client_data,
	const char *help_label, gtPushCB help_callback, void *help_client_data)
    { tmp->add_default_buttons(OK_label, OK_callback, OK_client_data,
			       apply_label, apply_callback, apply_client_data,
			       cancel_label, cancel_callback, cancel_client_data,
			       help_label, help_callback, help_client_data); }
    void popup(int mod) { tmp->popup(mod); }
    void popdown() { tmp->popdown(); }
    gtPushButton *button(int n) { return tmp->button(n); }
    gtPushButton *button(const char *n) { return tmp->button(n); }
    void default_button(gtPushButton *b) { tmp->default_button(b); }
    void title(const char *title_string) { tmp->title(title_string); }
    char *title() { return tmp->title(); }
    void bring_to_top() { tmp->bring_to_top(); }

    static void button_connector(gtPushButton *, gtEventPtr, void *, gtReason);
    static void list_connector(gtList *, gtEventPtr, void *, gtReason);
    void do_connect(gtPushButton *);
    void do_connect();
    
  protected:
    gtListDialog(gtBase *, const char*, const char*,
		 const char*, gtListStyle, const char**, int);

  private:
    gtDialogTemplate* tmp;
    gtList* l;
    int num_connections;
    gtListDialogConnector* connections;
};

#endif // _gtListDialog_h

/*
   START-LOG-------------------------------------------

 $Log: gtListDialog.h  $
 Revision 1.1 1993/07/28 19:47:44EDT builder 
 made from unix file
 * Revision 1.2.1.3  1992/10/21  18:36:10  jon
 * Added add_help_button()
 *
 * Revision 1.2.1.2  92/10/09  18:10:16  jon
 * RCS history fixup
 * 
 * Revision 1.2.1.1  92/10/07  20:33:52  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:50  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:51  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:21  smit
 * Initial revision
 * 
// Revision 1.6  92/09/11  11:12:10  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  08:17:51  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.4  92/09/11  04:07:27  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/08/06  18:21:55  rfermier
// added destructor.
// 
// Revision 1.2  92/08/04  16:37:59  rfermier
// Triggered by default_callback rather than select_callback, added style argument
// 
// Revision 1.1  92/08/04  16:05:45  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
