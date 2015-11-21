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
// gtDlgTemplateXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the DialogTemplate
// class (gtDialogTemplateXm)
//------------------------------------------

// INCLUDE FILES

#include <psetmem.h>

#include <gtBaseXm.h>
#include <gtDlgTemplateXm.h>

#include <Xm/Form.h>

#include <gtNoParent.h>
#include <gtPushButton.h>
#include <gtSepar.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <motifString.h>
#include <systemMessages.h>

  //Guy: the next variable keeps track if help is enabled for dialgboxes or not
unsigned int  gtDialogTemplate_Help_available_flag;

void clear_modal_dialog();

// FUNCTION DEFINITIONS

gtDialogTemplate* gtDialogTemplate::create(gtBase *parent, const char *name,
					   const char *title)
{
    const double DEFAULT_MARGIN = 10.0;

    return new gtDialogTemplateXm(
	parent, name, title, DEFAULT_MARGIN, DEFAULT_MARGIN);
}


gtDialogTemplate* gtDialogTemplate::create(
    gtBase* parent, const char* name, const char* title,
    double m_height, double m_width)
{
    return new gtDialogTemplateXm(parent, name, title, m_height, m_width);
}


gtDialogTemplateXm::gtDialogTemplateXm(
    gtBase* parent, const char* name, const char* title,
    double m_height, double m_width)
: num_buttons(0),
  buttons(NULL)
{
    if(parent == NULL)
	parent = gtNoParent::app_shell();

    motifString xm_title(title ? (char*)gettext(title) : "");

    Arg	args[3];
    int n = 0;
    XtSetArg(args[0], XmNautoUnmanage, False); n++;
    XtSetArg(args[1], XmNresizePolicy, XmRESIZE_NONE); n++;
    if(title)
    {
	XtSetArg(args[2], XmNdialogTitle, (XmString)xm_title); n++;
    }

    Widget dialog_form = XmCreateFormDialog(
	parent->container()->rep()->widget(), (char*)name, args, n);

    r->widget(XtParent(dialog_form));

    overall_form = gtForm::create((gtBase *)NULL, "dialog_form");
    overall_form->release_rep();
    overall_form->attach_rep(new gtRep(dialog_form));

    separ = gtSeparator::create(overall_form, "separator", gtHORZ);
    separ->attach(gtLeft);
    separ->attach(gtRight);
    separ->attach(gtBottom, NULL, 65);
    separ->manage();

    user_form = gtForm::create(overall_form, "form");
    user_form->attach(gtLeft,   NULL,  (int)m_width);
    user_form->attach(gtRight,  NULL,  (int)m_width);
    user_form->attach(gtTop,    NULL,  (int)m_height);
    user_form->attach(gtBottom, separ, (int)m_height);
    user_form->manage();

    destroy_init();
}

gtDialogTemplateXm::~gtDialogTemplateXm()
{
    clear_modal_dialog();

    if(buttons)
	free((char *)buttons);
}


gtBase* gtDialogTemplateXm::container()
{
    return user_form;
}


static void attach_buttons(gtPushButton** buttons, unsigned int num_buttons)
{
    int num_managed = 0;
    int i;
    for(i = 0; i < num_buttons; ++i)
	if(buttons[i]->is_managed())
	    num_managed++;

    const double margin_over_width = 0.1;

    const double width =
	100.0 /
	(margin_over_width * (num_managed + 1) + num_managed);

    const double margin = width * margin_over_width;

    int n = 0;
    for(i = 0; i < num_buttons; i++)
    {
	if(buttons[i]->is_managed())
	{
	    n++;
	    buttons[i]->unmanage();
	    buttons[i]->attach_pos(
		gtLeft, int((n * margin) + ((n - 1) * width) + 0.5));
	    buttons[i]->attach_pos(
		gtRight, int((n * margin) + (n * width) + 0.5));
	    buttons[i]->manage();
	}
    }
}


int gtDialogTemplateXm::add_button(const char *name, const char *label,
				   gtPushCB callback, void *callback_data)	
{
    gtPushButton* pb = gtPushButton::create(
	overall_form, (char*)name, (char*)label, callback, callback_data);
    if(callback == NULL)
	pb->set_sensitive(False);
    pb->attach(gtTop, separ, 4);
    pb->shadow_thickness(1);

    if(num_buttons++ == 0)
	buttons = (gtPushButton **)psetmalloc(sizeof(gtPushButton*));
    else
	buttons = (gtPushButton **)realloc(
	    (char *)buttons, num_buttons * sizeof(gtPushButton*));
    buttons[num_buttons - 1] = pb;

    if(num_buttons == 1)
	default_button(pb);
    if(strcmp(name, "cancel") == 0)
	cancel_button(pb);

    pb->manage();
    attach_buttons(buttons, num_buttons);

    return num_buttons - 1;
}

void gtDialogTemplateXm::add_label(const char *name, const char *txt)
{
    gtLabel *label = gtLabel::create(overall_form, (char *)name, (char *)txt);
    label->attach(gtTop, separ, 20);
    label->attach(gtLeft, NULL, 10);
    label->attach(gtRight, NULL, 10);
    label->alignment(gtCenter);
    label->manage();
}


void gtDialogTemplateXm::show_button(unsigned int which, int flag)
{
    if(which < num_buttons)
    {
	if(flag)
	    buttons[which]->manage();
	else
	    buttons[which]->unmanage();

	attach_buttons(buttons, num_buttons);
    }
}

void gtDialogTemplateXm::add_default_buttons(
    const char *OK_label, gtPushCB OK_callback, void *OK_client_data,
    const char *apply_label, gtPushCB apply_callback, void *apply_client_data,
    const char *cancel_label, gtPushCB cancel_callback, void *cancel_client_data,
    const char *help_label, gtPushCB help_callback, void *help_client_data)
{
    add_button("ok", (OK_label ? OK_label : TXT("OK")), OK_callback,
	       OK_client_data);
    add_button("apply", (apply_label ? apply_label : TXT("Apply")),
	       apply_callback, apply_client_data);
    add_button("cancel", (cancel_label ? cancel_label : TXT("Cancel")),
	       cancel_callback, cancel_client_data);

    //Guy: check if help is available, and set the sensitivity acordingly
    if(help_callback)
    {
	unsigned int nHelpButton = add_button("help", (help_label ? help_label : TXT("Help")),
		   help_callback, help_client_data);
	// we do not have a context sensitive help for now
	show_button(nHelpButton,0);
/*
	if (!gtDialogTemplate_Help_available_flag)
	{
	    gtPushButton* tempButtonP = button("help");
	    if (tempButtonP != NULL)
		tempButtonP->set_sensitive(False);
	}
*/
    }
    else
	add_help_button();
}


void gtDialogTemplateXm::add_default_buttons(
    gtPushCB OK_callback, void* OK_data,
    gtPushCB apply_callback, void* apply_data,
    gtPushCB cancel_callback, void* cancel_data)
{
    add_button("ok", TXT("OK"), OK_callback, OK_data);
    add_button("apply", TXT("Apply"), apply_callback, apply_data);
    add_button("cancel", TXT("Cancel"), cancel_callback, cancel_data);
    add_help_button();
}

void gtDialogTemplateXm::add_help_button()
{
    unsigned int nHelpButton = add_button("help", TXT("Help"), gtBase::help_button_callback, NULL);
    // we do not have a context sensitive help for now
    show_button(nHelpButton,0);
    
/*
    //Guy: check if help is available. If not, set the help button insensitive

    if (!gtDialogTemplate_Help_available_flag)
    {
	gtPushButton* tempButtonP = button("help");
	if (tempButtonP != NULL)
	    tempButtonP->set_sensitive(False);
    }
*/
}

gtPushButton* gtDialogTemplateXm::button(const char *nm)
{
    for (int i = 0; i < num_buttons; i++)
	if(strcmp(buttons[i]->name(), nm) == 0)
	    return buttons[i];

    return NULL;
}

void gtDialogTemplateXm::title(const char *title)
{
    if(title)
    {
	motifString xm_title((char*)gettext(title));

	Arg args[1];
	XtSetArg(args[0], XmNdialogTitle, (XmString)xm_title);
	XtSetValues(overall_form->rep()->widget(), args, 1);
    }
}    

char* gtDialogTemplateXm::title() // NOT IMPLEMENTED
{
    return NULL;
}    
    
void gtDialogTemplateXm::default_button(gtPushButton *p)
{
    Arg	args[1];

    // remove old default appearance
    gtPushButton *pb = default_button();
    if(pb) {
	XtSetArg(args[0], XmNshowAsDefault, 0);
	XtSetValues(pb->rep()->widget(), args, 1); 
    }

    // set new resources
    if(p) {
	XtSetArg(args[0], XmNshowAsDefault, 1);
	XtSetValues(p->rep()->widget(), args, 1); 
	XtSetArg(args[0], XmNdefaultButton, p->rep()->widget());
    }
    else
	XtSetArg(args[0], XmNdefaultButton, NULL);

    XtSetValues(overall_form->rep()->widget(), args, 1);
}    

gtPushButton* gtDialogTemplateXm::default_button()
{
    Arg	args[1];
    Widget w;
    XtSetArg(args[0], XmNdefaultButton, &w);

    XtGetValues(overall_form->rep()->widget(), args, 1);

    for (int i = 0; i < num_buttons; i++)
	if (w = buttons[i]->rep()->widget())
	    return buttons[i];

    return NULL;
}

void gtDialogTemplateXm::cancel_button(gtPushButton *p)
{
    Arg	args[1];
    XtSetArg(args[0], XmNcancelButton, p ? p->rep()->widget() : NULL);
    XtSetValues(overall_form->rep()->widget(), args, 1);
}    

gtPushButton* gtDialogTemplateXm::cancel_button()
{
    Arg	args[1];
    Widget w;
    XtSetArg(args[0], XmNcancelButton, &w);

    XtGetValues(overall_form->rep()->widget(), args, 1);

    for (int i = 0; i < num_buttons; i++)
	if (w = buttons[i]->rep()->widget())
	    return buttons[i];

    return NULL;
}

void gtDialogTemplateXm::resize_policy(gtResizeOption opt)
{
    Arg	args[1];

    if (opt == gtResizeNone)
	XtSetArg(args[0], XmNresizePolicy, XmRESIZE_NONE);
    else if (opt == gtResizeGrow)
	XtSetArg(args[0], XmNresizePolicy, XmRESIZE_GROW);
    else if (opt == gtResizeAny)
	XtSetArg(args[0], XmNresizePolicy, XmRESIZE_ANY);
    else
	return;

    XtSetValues(overall_form->rep()->widget(), args, 1);
}


//Guy:
//I am defining the next two functions to set the flag: gtDialogTemplate_Help_available_flag "true" or false" 
//respectively.  This flag signifies whether or not help is available for dialog boxes. (i.e. before the 
//pdf is loaded, no help can be used


void gtDialogTemplate_enable_help(){
   gtDialogTemplate_Help_available_flag = 1 ;
}

void gtDialogTemplate_disable_help(){
    gtDialogTemplate_Help_available_flag = 0;
}

//end Guy\'s changes

/*
   START-LOG-------------------------------------------

   $Log: gtDlgTemplateXm.h.C  $
   Revision 1.8 2003/06/03 13:35:36EDT Dmitry Ryachovsky (dmitry) 
   
   Revision 1.7 2000/07/07 08:10:33EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.6  1994/08/01  19:15:28  farber
Bug track: 7911
disable help before pdf is loaded

Revision 1.2.1.5  1994/01/13  02:47:24  kws
Use psetmalloc

Revision 1.2.1.4  1993/01/26  04:32:20  glenn
Add dtor.

Revision 1.2.1.3  1993/01/13  23:45:22  glenn
Add attach_buttons to do horizontal layout of buttons in add_button
and show_button.
Add show_button.

Revision 1.2.1.2  1992/10/09  19:03:29  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
