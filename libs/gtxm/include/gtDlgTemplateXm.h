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
#ifndef _gtDlgTemplateXm_h
#define _gtDlgTemplateXm_h

// gtDlgTemplateXm.h
//------------------------------------------
// synopsis:
// Class definitions and headers for XMotif implentation of gtDialogTemplate
//------------------------------------------

#include <gtDlgTemplate.h>


class gtDialogTemplateXm : public gtDialogTemplate
{
  public:
    gtDialogTemplateXm(gtBase* parent, const char* name, const char* title,
		       double m_height, double m_width);
    ~gtDialogTemplateXm();

    void add_default_buttons(
	const char* OK_label, gtPushCB OK_callback, void* OK_data,
	const char* apply_label, gtPushCB apply_callback, void* apply_data,
	const char* cancel_label, gtPushCB cancel_callback, void* cancel_data,
	const char* help_label, gtPushCB help_callback, void* help_data);

    int add_button(const char* name, const char* label,
		   gtPushCB callback, void* data);

    void add_label(const char *name, const char *txt);
    gtBase* container();
    gtPushButton* button(unsigned int n) {
	return (n < num_buttons ? buttons[n] : 0); }
    gtPushButton* button(const char* nm);
    void title(const char* title_string);
    char* title();
    void default_button(gtPushButton*);
    gtPushButton* default_button();
    void resize_policy(gtResizeOption);

    void add_default_buttons(
	gtPushCB OK_callback, void* OK_data,
	gtPushCB apply_callback, void* apply_data,
	gtPushCB cancel_callback, void* cancel_data);

    void add_help_button();

    void cancel_button(gtPushButton*);
    gtPushButton* cancel_button();

    void show_button(unsigned int, int flag);

  private:
    gtForm*		overall_form;
    gtForm*		user_form;
    gtSeparator*	separ;
    gtPushButton**	buttons;     
    unsigned int	num_buttons;
};


/*
   START-LOG-------------------------------------------

// $Log: gtDlgTemplateXm.h  $
// Revision 1.2 1995/05/17 15:13:33EDT so 
// Bug track: 9903
// fix bug 9903
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Add dtor.
 *
 * Revision 1.2.1.3  1993/01/13  23:46:50  glenn
 * Add show_button.
 *
// Revision 1.16  92/09/23  11:00:42  glenn
// Add cancel_button set and get.
// 
// Revision 1.15  92/09/21  00:02:05  glenn
// Change order of virtual functions.
// 
// Revision 1.14  92/09/20  21:05:27  glenn
// Add add_default_buttons without labels or help specified.
// Add add_help_button.
// 
// Revision 1.13  92/09/11  17:06:08  glenn
// Remove manage.
// 
// Revision 1.12  92/09/11  00:45:31  glenn
// Remove excess include files.
// Remove sh, butt_form.
// Remove bring_to_top.
// 
// Revision 1.11  92/08/15  09:03:59  rfermier
// added resize_option
// 
// Revision 1.10  92/08/06  18:10:51  rfermier
// hierarchy change, default_button()
// 
// Revision 1.9  92/07/31  13:32:12  rfermier
// fixed const char * bug in add_default_buttons
// 
// Revision 1.8  92/07/30  23:55:48  rfermier
// GT method name changes
// 
// Revision 1.7  92/07/29  15:52:58  rfermier
// added char *
// 
// Revision 1.6  92/07/23  17:31:58  rfermier
// realization fix, added default_button
// 
// Revision 1.5  92/07/22  13:46:54  rfermier
// added title function
// 
// Revision 1.4  92/07/16  13:55:32  rfermier
// added title setting function
// 
// Revision 1.3  92/07/09  18:03:16  rfermier
// *** empty log message ***
// 
// Revision 1.2  92/07/09  11:06:45  rfermier
// add margin; fix button spacing
// 
// Revision 1.1  92/07/08  20:40:53  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtDlgTemplateXm_h
