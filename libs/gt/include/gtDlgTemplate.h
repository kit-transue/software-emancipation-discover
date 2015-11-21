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
#ifndef _gtDlgTemplate_h
#define _gtDlgTemplate_h

// gtDlgTemplate.h
//------------------------------------------
// synopsis:
// generic DialogTemplate manager (gtDialogTemplate)
//
//------------------------------------------

#ifndef _gtDialog_h
#include <gtDialog.h>
#endif


//Guy:
//add a flag to signify if help should be available for dialog boxes or not.  (before a pdf is loaded, then
// help can not be used.  The above functions set the flag accordingly.

extern "C" {
    void gtDialogTemplate_enable_help();
    void gtDialogTemplate_disable_help();
}
//end Guy\'s changes

class gtDialogTemplate : public gtDialog
{
  public:
    // Creates a DialogTemplate suitable for making basic dialogs.  If the 
    // m_height and m_width arguments are passed to the function, then there
    // will be a margin of those dimensions (in mm) around the outside of the 
    // gtForm which is returned.  If unspecified, they default to 3mm.
    // Pre: It must have a valid gt object parent passed to it.
    // Post: The returned pointer points to a valid gtDialogTemplate
    // specific to the chosen toolkit.
    static gtDialogTemplate* create(
	gtBase* parent, const char* name, const char* title);

    static gtDialogTemplate* create(
	gtBase* parent, const char* name, const char* title,
	double margin_height, double margin_width);

    ~gtDialogTemplate();

    // Add the basic "big 4" dialog buttons.  If a label is specified, it
    // will be used, but if NULL is passed as the label then the default
    // label is used.  This is recommended in general, because if you want
    // different buttons you really ought to just use add_button several
    // times.  If NULL is passed as a callback, the button will be grayed out.
    //
    // NOTE: Use of this function is deprecated.
    //       (see next function with same name).
    //
    virtual void add_default_buttons(
	const char* OK_label, gtPushCB OK_callback, void* OK_data,
	const char* apply_label, gtPushCB apply_callback, void* apply_data,
	const char* cancel_label, gtPushCB cancel_callback, void* cancel_data,
	const char* help_label, gtPushCB help_callback, void* help_data) = 0;

    // Add a button to go into the button area at the bottom of the template.
    virtual int add_button(const char* name, const char* label,
			   gtPushCB callback, void* client_data) = 0;
    virtual void add_label(const char *name, const char *txt) = 0;
    // Returns the nth pushbutton.
    virtual gtPushButton* button(unsigned int n) = 0;

    // Returns the pushbutton whose name is nm
    virtual gtPushButton* button(const char* nm) = 0;

    // Sets or retrieves the title of the dialog to title_string
    virtual void title(const char* title_string) = 0;
    virtual char* title() = 0;

    // Specify which button should perform the dialogs default action.
    virtual void default_button(gtPushButton*) = 0;

    // Return the default button.
    virtual gtPushButton* default_button() = 0;

    // Specify the resizing policy.
    virtual void resize_policy(gtResizeOption) = 0;

    // This function is preferred to the preceeding function of the same name.
    // Labels are not specified, and a help button is added automatically.
    virtual void add_default_buttons(
	gtPushCB OK_callback, void* OK_data,
	gtPushCB apply_callback, void* apply_data,
	gtPushCB cancel_callback, void* cancel_data) = 0;

    // Add a help button with appropriate callback.
    virtual void add_help_button() = 0;

    // Specify which button should cancel the dialog.
    virtual void cancel_button(gtPushButton*) = 0;

    // Return the cancel button
    virtual gtPushButton* cancel_button() = 0;

    // Show or hide a button at the bottom of the template.
    virtual void show_button(unsigned int, int visible = 1) = 0;

    void help_context_name(const char *);

  protected:
    gtDialogTemplate();
};


/*
   START-LOG-------------------------------------------

 $Log: gtDlgTemplate.h  $
 Revision 1.4 1995/11/03 13:55:35EST ljb 
 Add function help_context_name (bug fix 11159).
 * Revision 1.2.1.4  1994/08/01  19:15:04  farber
 * Bug track: 7911
 * disable help before pdf is loaded
 *
 * Revision 1.2.1.3  1993/01/13  23:46:30  glenn
 * Add show_button
 *
 * Revision 1.2.1.2  1992/10/09  18:10:01  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:20  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:19  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:49  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:19  smit
 * Initial revision
 * 
// Revision 1.18  92/09/23  11:01:00  glenn
// Add cancel_button set and get.
// 
// Revision 1.17  92/09/20  21:04:25  glenn
// Add add_default_buttons without labels or help specified.
// Add add_help_button.
// 
// Revision 1.16  92/09/11  11:11:58  builder
// Make destructor public.
// 
// Revision 1.15  92/09/11  04:40:30  builder
// Make dtor public.
// 
// Revision 1.14  92/09/11  04:07:17  builder
// Add ctor and dtor.
// 
// Revision 1.13  92/09/11  00:38:20  glenn
// Remove pure virtual bring_to_top (now in gtShell).
// 
// Revision 1.12  92/08/15  10:21:05  rfermier
// Fixed pure virtual vtable problem
// 
// Revision 1.11  92/08/15  09:04:13  rfermier
// added resize_option
// 
// Revision 1.10  92/08/06  18:11:04  rfermier
// hierarchy change, default_button()
// 
// Revision 1.9  92/07/30  23:44:36  glenn
// GT method name changes
// 
// Revision 1.8  92/07/29  15:53:47  rfermier
// change to const char *
// 
// Revision 1.7  92/07/23  17:32:28  rfermier
// realization fix, added default_button
// 
// Revision 1.6  92/07/22  13:47:06  rfermier
// added title function
// 
// Revision 1.5  92/07/17  12:19:17  rfermier
// Changed description of modality
// 
// Revision 1.4  92/07/16  13:55:38  rfermier
// added title setting function
// 
// Revision 1.3  92/07/09  18:03:26  rfermier
// *** empty log message ***
// 
// Revision 1.2  92/07/09  11:07:11  rfermier
// more documentation, add margin
// 
// Revision 1.1  92/07/08  20:38:53  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/

#endif // _gtDlgTemplate_h
