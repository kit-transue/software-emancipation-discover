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
// gtPrimXm
//------------------------------------------
// synopsis:
// XMotif implementation of Primitive GT objects
//
//------------------------------------------

// INCLUDE FILES

#include <malloc.h>

#include <gtBaseXm.h>
#include <gtPrimXm.h>

#include <X11/keysymdef.h>


// FUNCTION DEFINITIONS

void gtPrimitive::unmanage()
{
    XtUnmanageChild(r->widget());
}

void gtPrimitive::remanage()
{
    unmanage();
    manage();
}


static void attachment_arg(
    Arg* args, int& n,
    gtCardinalDir dir, gtBase* attachee, int offset, int opposite)
{
    int attach_style;

    if(opposite)
	if(attachee)
	    attach_style = XmATTACH_OPPOSITE_WIDGET;
	else
	    attach_style = XmATTACH_OPPOSITE_FORM;
    else
	if(attachee)
	    attach_style = XmATTACH_WIDGET;
	else
	    attach_style = XmATTACH_FORM;

    char* attach_side = NULL;
    char* attach_widget = NULL;
    char* attach_offset = NULL;
    double ppmm = 0.0;

    switch(dir)
    {
      case gtDirUp:
	attach_side   = XmNtopAttachment;
	attach_widget = XmNtopWidget;
	attach_offset = XmNtopOffset;
	ppmm = gtBase::ppmm_height;
	break;

      case gtDirDown:
	attach_side   = XmNbottomAttachment;
	attach_widget = XmNbottomWidget;
	attach_offset = XmNbottomOffset;
	ppmm = gtBase::ppmm_height;
	break;

      case gtDirLeft:
	attach_side   = XmNleftAttachment;
	attach_widget = XmNleftWidget;
	attach_offset = XmNleftOffset;
	ppmm = gtBase::ppmm_width;
	break;

      case gtDirRight:
	attach_side   = XmNrightAttachment;
	attach_widget = XmNrightWidget;
	attach_offset = XmNrightOffset;
	ppmm = gtBase::ppmm_width;
	break;
    }

    int offset_pixels = (offset ? (int)((offset * .254 * ppmm) + 0.5) : 0);

    XtSetArg(args[n], attach_side, attach_style); n++;
    XtSetArg(args[n], attach_offset, offset_pixels); n++;
    if(attachee) {
	XtSetArg(args[n], attach_widget, attachee->rep()->widget()); n++;
    }
}


static void detachment_arg(Arg* args, int& n, gtCardinalDir dir)
{
    char* attach_side = NULL;
    switch(dir)
    {
      case gtDirUp:
	attach_side   = XmNtopAttachment;
	break;

      case gtDirDown:
	attach_side   = XmNbottomAttachment;
	break;

      case gtDirLeft:
	attach_side   = XmNleftAttachment;
	break;

      case gtDirRight:
	attach_side   = XmNrightAttachment;
	break;
    }
    XtSetArg(args[n], attach_side, XmATTACH_NONE); n++;
}


void gtPrimitive::attach(
    gtCardinalDir dir, gtBase *attachee, int offset, int opposite)
//
// Attach one side.
//
{
    int n = 0;
    Arg args[3];
    attachment_arg(args, n, dir, attachee, offset, opposite);

    XtSetValues(r->widget(), args, n);
}


void gtPrimitive::attach_tblr(gtBase* T, gtBase* B, gtBase* L, gtBase* R)
//
// Attach all sides of an object with offsets of zero.
// Passing "this" indicates no attachment in the given direction.
//
{
    int n = 0;
    Arg	args[12];

    if(T != this)
	attachment_arg(args, n, gtTop, T, 0, 0);
    else
	detachment_arg(args, n, gtTop);

    if(B != this)
	attachment_arg(args, n, gtBottom, B, 0, 0);
    else
	detachment_arg(args, n, gtBottom);

    if(L != this)
	attachment_arg(args, n, gtLeft, L, 0, 0);
    else
	detachment_arg(args, n, gtLeft);

    if(R != this)
	attachment_arg(args, n, gtRight, R, 0, 0);
    else
	detachment_arg(args, n, gtRight);

    XtSetValues(r->widget(), args, n);
}


void gtPrimitive::attach_tb(gtBase* T, gtBase* B)
//
// Attach top and bottom sides of an object with offsets of zero.
// Passing "this" indicates no attachment in the given direction.
//
{
    int n = 0;
    Arg	args[12];

    attachment_arg(args, n, gtLeft, NULL, 0, 0);

    attachment_arg(args, n, gtRight, NULL, 0, 0);

    if(T != this)
	attachment_arg(args, n, gtTop, T, 0, 0);
    else
	detachment_arg(args, n, gtTop);

    if(B != this)
	attachment_arg(args, n, gtBottom, B, 0, 0);
    else
	detachment_arg(args, n, gtBottom);

    XtSetValues(r->widget(), args, n);
}


void gtPrimitive::attach_lr(gtBase* L, gtBase* R)
//
// Attach left and right sides of an object with offsets of zero.
// Passing "this" indicates no attachment in the given direction.
//
{
    int n = 0;
    Arg	args[12];

    attachment_arg(args, n, gtTop, NULL, 0, 0);

    attachment_arg(args, n, gtBottom, NULL, 0, 0);

    if(L != this)
	attachment_arg(args, n, gtLeft, L, 0, 0);
    else
	detachment_arg(args, n, gtLeft);

    if(R != this)
	attachment_arg(args, n, gtRight, R, 0, 0);
    else
	detachment_arg(args, n, gtRight);

    XtSetValues(r->widget(), args, n);
}


void gtPrimitive::detach(gtCardinalDir dir)
//
// Detach the specified side of the object.
//
{
    int n = 0;
    Arg args[1];
    detachment_arg(args, n, dir);

    XtSetValues(r->widget(), args, n);
}

void gtPrimitive::attach_pos(gtCardinalDir dir, int position)
{
    Arg args[2];
    switch (dir)
    {
      case gtDirUp:
	XtSetArg(args[0], XmNtopAttachment, XmATTACH_POSITION);
	XtSetArg(args[1], XmNtopPosition, position);
	break;
      case gtDirDown:
	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_POSITION); 
	XtSetArg(args[1], XmNbottomPosition, position);
	break;
      case gtDirLeft:
	XtSetArg(args[0], XmNleftAttachment, XmATTACH_POSITION); 
	XtSetArg(args[1], XmNleftPosition, position);
	break;
      case gtDirRight:
	XtSetArg(args[0], XmNrightAttachment, XmATTACH_POSITION);
	XtSetArg(args[1], XmNrightPosition, position);
	break;
    }
    XtSetValues(r->widget(), args, 2);
}

void gtPrimitive::attach_offset(gtCardinalDir dir, int off)
{
    Arg args[1];
    switch (dir)
    {
      case gtDirUp:
	XtSetArg(args[0], XmNtopOffset, hund_to_pix_vert(off));
	break;

      case gtDirDown:
	XtSetArg(args[0], XmNbottomOffset, hund_to_pix_vert(off));
	break;

      case gtDirLeft:
	XtSetArg(args[0], XmNleftOffset, hund_to_pix_horiz(off));
	break;

      case gtDirRight:
	XtSetArg(args[0], XmNrightOffset, hund_to_pix_horiz(off));
	break;
    }
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::min_max_height(int min, int max)
{
    int n = 0;
    Arg args[2];

    if(min < 0)
    {
	XtSetArg(args[n], XmNpaneMinimum, 1); ++n;
    }
    else if(min > 0)
    {
	XtSetArg(args[n], XmNpaneMinimum, hund_to_pix_vert(min)); ++n;
    }
    if(max < 0)
    {
	XtSetArg(args[n], XmNpaneMaximum, 0x7FFF); ++n;
    }
    else if(max > 0)
    {
	XtSetArg(args[n], XmNpaneMaximum, hund_to_pix_vert(max)); ++n;
    }
    XtSetValues(r->widget(), args, n);
}

void gtPrimitive::construct()
{}    

void gtPrimitive::set_sensitive(int sens)
{
    sens = (sens != 0);		// Coerce to canonical boolean value.
    if(XtIsSensitive(r->widget()) != sens)
	XtSetSensitive(r->widget(), sens);
}

void gtPrimitive::accelerator(const char *acc, const char *acc_text)
{
    XmString 	str;
    Arg		args[2];
    int		n = 0;

    if (acc != NULL) {
	XtSetArg(args[n], XmNaccelerator, acc); n++; }
    if (acc_text != NULL) {
	str = XmStringCreateSimple((char *)gettext(acc_text));
	XtSetArg(args[n], XmNacceleratorText, str);
	n++; 
    }
    XtSetValues(r->widget(), args, n);
    if (acc_text !=NULL)
	XmStringFree(str);
}

void gtPrimitive::mnemonic(const char *mne)
{
    XtVaSetValues(r->widget(), XtVaTypedArg, XmNmnemonic, XmRString,
		  mne, strlen(mne), NULL);
}
    
void gtPrimitive::add_key_navig_group()
{
    Arg args[1];
    XtSetArg(args[0], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP);
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::rem_key_navig_group()
{
    Arg args[1];
    XtSetArg(args[0], XmNnavigationType, XmNONE);
    XtSetValues(r->widget(), args, 1);
}	

void* gtPrimitive::userData()
{
    void* ans;

    Arg args[1];
    XtSetArg(args[0], XmNuserData, &ans);
    XtGetValues(r->widget(), args, 1);

    return ans;
}

void gtPrimitive::userData(void* data)
{
    Arg args[1];
    XtSetArg(args[0], XmNuserData, data);
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::manage_list(gtPrimitive **plist, int num_elems)
{
    Widget *wlist;
    int i;

    wlist = (Widget *)calloc(num_elems, sizeof(Widget));
    for (i=0; i<num_elems; i++) {
	wlist[i] = (plist[i])->rep()->widget();
    }
    XtManageChildren(wlist, num_elems);
}

void gtPrimitive::unmanage_list(gtPrimitive **plist, int num_elems)
{
    Widget *wlist;
    int 	i;

    wlist = (Widget *)calloc(num_elems, sizeof(Widget));
    for (i=0; i<num_elems; i++) {
	wlist[i] = plist[i]->rep()->widget();
    }
    XtUnmanageChildren(wlist, num_elems);
}

void gtPrimitive::install_accelerator(gtPrimitive *targ)
{
    Widget w1, w2;
    if (container())
	w1 = container()->rep()->widget();
    else	
	w1 = rep()->widget();
    if (targ->container())
	w2 = targ->container()->rep()->widget();
    else	
	w2 = targ->rep()->widget();
   XtInstallAccelerators(w1, w2);
}

void gtPrimitive::translations(void *tran)
{
    Arg	args[1];
    XtSetArg(args[0], XmNtranslations, (XtTranslations)tran);
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::background(unsigned long c)
{
    Arg args[1];
    XtSetArg(args[0], XmNbackground, c);
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::background(const char *c)
{
    XtVaSetValues(r->widget(), XtVaTypedArg, XmNbackground, XmRString, c,
		  strlen(c), NULL);
}

unsigned long gtPrimitive::background()
{
    unsigned long ans;
    Arg args[1];	
    XtSetArg(args[0], XmNbackground, &ans);
    XtGetValues(r->widget(), args, 1);
    return ans;
}

void gtPrimitive::foreground(unsigned long c)
{
    Arg args[1];
    XtSetArg(args[0], XmNforeground, c);
    XtSetValues(r->widget(), args, 1);
}

void gtPrimitive::foreground(const char *c)
{
    XtVaSetValues(r->widget(), XtVaTypedArg, XmNforeground, XmRString, c,
		  strlen(c), NULL);
}

unsigned long gtPrimitive::foreground()
{
    unsigned long ans;
    Arg args[1];	
    XtSetArg(args[0], XmNforeground, &ans);
    XtGetValues(r->widget(), args, 1);
    return ans;
}

void gtPrimitive::font(const char *font_name)
{
    XtVaSetValues(r->widget(), XtVaTypedArg, XmNfontList, XmRString,
		  font_name, strlen(font_name), NULL);
}


void gtPrimitive::alignment(gtAlign a)
{
    int xm_alignment = 0;
    switch(a)
    {
      case gtBeginning:	xm_alignment = XmALIGNMENT_BEGINNING;	break;
      case gtCenter:	xm_alignment = XmALIGNMENT_CENTER;	break;
      case gtEnd:	xm_alignment = XmALIGNMENT_END;		break;
    }
    Arg args[1];
    XtSetArg(args[0], XmNalignment, xm_alignment);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtPrimXm.h.C  $
   Revision 1.4 2000/07/07 08:10:45EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.13  1994/06/23  18:24:19  builder
sun5 port

Revision 1.2.1.12  1994/03/29  01:20:34  builder
Port

Revision 1.2.1.11  1994/02/23  22:14:52  builder
Port

Revision 1.2.1.9  1993/10/04  14:03:01  kws
Port

Revision 1.2.1.8  1993/10/01  17:31:22  kws
Port

Revision 1.2.1.7  1993/02/08  23:41:48  glenn
Check sensitivity before (re)setting it in set_sensitive.

Revision 1.2.1.6  1993/01/27  15:19:17  glenn
Use XtSetSensitive in set_sensitive instead of XmNsensitive, which does
not seem to deal with children of composite widgets.

Revision 1.2.1.5  1993/01/26  04:52:04  glenn
Remove trigger_callbacks, window, name.

Revision 1.2.1.4  1993/01/22  17:14:12  glenn
Add attach_tb, attach_lr, attach_tblr, alignment.
Use attachment_arg, detachment_arg in all attach calls.

Revision 1.2.1.3  1993/01/20  03:38:02  glenn
Use hund_to_pix functions.  Extend meaning of arguments to min_max_height.

Revision 1.2.1.2  1992/10/09  19:03:42  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
