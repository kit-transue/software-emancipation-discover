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

#include <vport.h>
#include vdialogHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vspringdomHEADER
#include vlistHEADER
#include vliststrHEADER
#include vlistitemHEADER
#include vcontainerHEADER
#include vimageHEADER
#include vnumHEADER

#ifdef XWIN32
extern "C" char * strdup (const char *);
#endif 

#include "ggeneric.h"
#include "gglobalFuncs.h"
#include "gArray.h"
#include "gString.h"
#include "ginterp.h"
#include "gCenteredLabel.h"
#include "grtlClient.h"
#include "gpaneObjects.h"
#include "glayer.h"
#include "gviewer.h"
#include "gview.h"

#include <../../DIS_main/interface.h>

int waiting = 0;
static int selecting = 0;

void show_interpreter_wait(int);

//-----------------------------------------------------------------------------


typedef vimage* iconPtr;
gArray (iconPtr);

//-----------------------------------------------------------------------------

vkindTYPED_FULL_DEFINITION (rtlClient, gdListItem, "rtlClient");
vkindTYPED_FULL_DEFINITION (clientButton, vbutton, "clientButton");

int	rtlClient::serial_ = 0;
rtlClient* rtlClient::list_;

static const int	rtlIconWidth	= 16;
static const int	rtlIconHeight	= 16;
static const int	rtlMaxIcons	= 4;

static gArrayOf (iconPtr) rtlIcon;

static const char* const iconName[] = {
					"Unknown",
					"Folder",
					"HomeFolder",
					"List",
					"Document",
					"CSource",
					"C++Source",
                                        "Function",
                                        "Macro",
                                        "Struct",
                                        "Subsys",
                                        "Union",
                                        "Typedef",
                                        "Enum",
                                        "Variable",
                                        "Member",
					"Group",
					"ScratchGroup",
					"Write15",
					"Write16",
					"Write17",
					"Write18",
					"Write19",
					"Write20",
					"Write21",
					"Write22",
					"Write23",
					"Write24",
					"Write25",
					"Write26",
					"Instance",
					"String",
					"Local",
					"SQL_C",
					"SQL_CPlusPlus",
					"Defect",
					"JavaSource",
					"Interface",
					"Package"
				      };


//-----------------------------------------------------------------------------

static void processRange (vlist* list, vlistSelection* sel, char* str)
{
    if (!str)
	return;

    int	high, low = atoi (str);
    char	*pc = strchr (str, '-');

    if (pc) {
	high = atoi (pc + 1);
	if (high < low) {
	    int tmp = low;
	    low = high;
	    high = tmp;
	}
    }
    else high = low;

    for (int i = low; i <= high; i++)
	list->SelectRow (sel, i);
}

//-----------------------------------------------------------------------------

static void processSpec (vlist* list, vlistSelection* sel, const vchar* spec)
{
    char c = *spec;
    while (isspace (c)) c = *++spec;
    if (*spec == '\0')
	return;

    char* sp = strdup ((char*)spec);
    if (!sp)
	return;

    for (char *pc2, *pc = sp; pc; pc = pc2 + 1) {
	if (pc2 = strchr (pc, ',')) {
	    *pc2 = '\0';
	    processRange (list, sel, pc);
	}
	else {
	    processRange (list, sel, pc);
	    break;
	}
    }

    free (sp);
    return;
}

//-----------------------------------------------------------------------------

inline void* iconKey (int idx)
{
    gInit (iconKey);

    return (void*) (idx + 1);
}

//-----------------------------------------------------------------------------

vimage* iconImage (vchar ic)
{
    gInit (iconImage);

    // Note - 'iconUnknown' is the first enumerated value.
    int idx = (int) ic - iconUnknown - 1;

    int rtlSize = rtlIcon.size();

    if ((idx >= 0) && (idx < rtlSize))
	return rtlIcon[idx];

    return 0;
}

//-----------------------------------------------------------------------------

inline vchar iconChar (iconKind kind)
{
    gInit (iconChar);

    return vcharFromASCII ((int) kind + 1);
}

//-----------------------------------------------------------------------------

static unsigned long iconDictHashFunc (const void* p)
{
    gInit (iconDictHashFunc);

    return (unsigned long) p;
}

//-----------------------------------------------------------------------------

static unsigned extractIcons (vstr* str, vchar* ib)
{
    gInit (extractIcons);

    if (ib) {
	ib[0] = iconChar (iconUnknown);
	ib[1] = vcharNULL;
    }

    if (!str)
	return 1;

    unsigned	num = 0;
    int		i = vcharLength (str);

    if (i > 0) {

	i--;
        while ((i >= 0) && (str[i] > 0x7F) && (num < rtlMaxIcons))
	    i--, num++;

	if (num > 0) {
	    i++;
	    if (ib)
                vcharCopyFast (&str[i], ib);
	    str[i] = vcharNULL;
	}
    }

    if (num == 0) {
	if (ib) {
	    ib[0] = iconChar (iconUnknown);
	    ib[1] = vcharNULL;
	}
	num = 1;
    }

    return num;
}

//-----------------------------------------------------------------------------

void upButtonObserveProc (vdialogItem* di, vevent* ev)
{
    gInit (upButtonObserveProc);

    if (!di || !ev)
	return;

    clientButton* b = clientButton::CastDown (di);
    if (!b)
	return;

    rtlClient* lc = b->getParent ();
    if (!lc)
	return;

    Tcl_Interp*		interp = 0;
    Layer*		lp = lc->getLayer ();

    if (lp)
	interp = lp->getInterp ();

    if (ev->GetType() == veventBUTTON_UP) {
	if (interp) {
	    gString cmd;
	    cmd.sprintf_scribed (
			vcharScribeLiteral("rtl_query_parent_project %d"),
                        lc->getId() );
	    gala_eval (interp, cmd);
	}
	else lc->queryParentProject ();
    }
}

//-----------------------------------------------------------------------------

int rtlQueryParentProjectCmd (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    gInit (rtlQueryParentProjectCmd);

    if (argc != 2)
	return TCL_ERROR;

    int		id = atoi (argv[1]);
    rtlClient*	lc = rtlClient::find (id);

    if (!lc)
	return TCL_ERROR;

    lc->queryParentProject ();

    return TCL_OK;
}

void rtlClient::Open()
{
    gdListItem::Open();
}

//-----------------------------------------------------------------------------

rtlClient* rtlClient::find (int id)
{
    gInit (rtlClient::find);

    for (rtlClient* r = list_; r; r = r->next_)
	if (r->getId() == id)
	    return r;

    return 0;
}

//-----------------------------------------------------------------------------

void rtlClient::addCommands (Tcl_Interp* interp)
{
    gInit (rtlClient::addCommands);

    Tcl_CreateCommand (interp, "rtl_query_parent_project",
					rtlQueryParentProjectCmd, NULL, NULL);
}

//-----------------------------------------------------------------------------

const vchar* rtlClient::getItem (int idx)
{
    gInit (rtlClient::getItem);

    if ((idx < 0) || (idx >= GetItemCount()))
	return vcharGetEmptyString ();

    const vchar* val = GetItemValue (idx);

    if (!val)
	SetItemValue (val = vcharGetEmptyString(), idx);

    if (!vcharLength (val)) {

	update_called = 0;
        vstr* str     = rcall_dis_rtl_get_item (Application::findApplication("DISmain"), 
						serverId_, idx);
	if(update_called){
	    Inval (vwindowINVAL_IMMEDIATE);
	    return (vchar *)" ";
	}
        if (str) {

	    vchar* ib = (vchar*) iconDict_->GetElem (iconKey (idx));
	    if (!ib) {
		ib = new vchar [rtlMaxIcons + 1];
		iconDict_->SetElem (iconKey (idx), ib);
	    }

	    unsigned	num_icons = extractIcons (str, ib);
	    vfont*	font = DetermineFont ();

	    int		width = font->StringWidthX (str) + 6;
 
	    width += num_icons * (rtlIconWidth + 1) + 1;
	    if (contentWidth_ < width)
		if ((contentWidth_ = width) > interiorWidth_)
	            SetColumnWidth (vlistviewALL_COLUMNS, contentWidth_);

	    SetItemValueScribed (vstrScribe (str), idx);
	    val = GetItemValue (idx);
	}
    }

    return val;
}

//-----------------------------------------------------------------------------

void rtlClient::refresh_ (void)
{
    gInit (rtlClient::refresh_);

    if (inUpdate_)
	Inval (vwindowINVAL_IMMEDIATE);
}

//-----------------------------------------------------------------------------

void rtlClient::set_caller (vchar *caller)
{
    rcall_dis_rtl_set_caller (Application::findApplication("DISmain"), serverId_, caller);
}

//-----------------------------------------------------------------------------

void rtlClient::select (const vchar* spec)
{
    gInit (rtlClient::select);

    vlist* list = GetList ();
    if (!list)
	return;

    vlistSelection* sel = GetSelection ();
    if (!sel)
	return;

    list->UnselectAll (sel);

    processSpec (list, sel, spec);

    refresh_();
}

//-----------------------------------------------------------------------------

void rtlClient::select (int spec)
{
    gInit (rtlClient::select);

    vlist* list = GetList ();
    if (!list)
	return;

    vlistSelection* sel = GetSelection ();
    if (!sel)
	return;

    list->UnselectAll (sel);

    list->SelectRow(sel, spec);

    refresh_();
}

//-----------------------------------------------------------------------------
void rtlClient::unselect (void)
{
    gInit (rtlClient::unselect);

    vlist* list = GetList ();
    if (!list)
	return;

    vlistSelection* sel = GetSelection ();
    if (!sel)
	return;

    list->UnselectAll (sel);
}

//-----------------------------------------------------------------------------

void rtlClient::unselectLocal_ (void)
{
    gInit (rtlClient::unselectLocal_);

    localOnly_ = 1;
    unselect ();
    localOnly_ = 0;
}

//-----------------------------------------------------------------------------

void rtlClient::DrawCellContents (int row, int, int, vrect *drawrect)
{
    gInit (rtlClient::DrawCellContents);

    show_interpreter_wait(-1);

    vchar dib[2];
    dib[0] = iconChar (iconUnknown);
    dib[1] = vcharNULL;

    if (row >= GetItemCount()) {
        show_interpreter_wait(-2);
	return;
    }

    const vchar*	txt = getItem(row);
    vchar* 		ib = (vchar*) iconDict_->GetElem (iconKey (row));
    vfont*		fnt = vdraw::GetFont ();

    int			x = drawrect->x + 3;
    int			y = drawrect->y + 3;
    unsigned		ni = 1;
    vimage*		image;

    if (!ib || ((ni = vcharLength (ib)) < 1)) {
	ib = dib;
	ni = 1;
    }
    
    vdraw::GSave ();

    vdraw::MoveTo (x + ni * (rtlIconWidth + 1) + 1, y + fnt->Descent());
    vdraw::Show (txt);

    for (int idx = 0; ib[idx]; idx++) {
	if (image = iconImage (ib[idx])) {
            vdraw::MoveTo (x + idx * (rtlIconWidth + 1), y);
            vdraw::ImageCompositeIdent (image);
	}
    }

    vdraw::GRestore ();

    show_interpreter_wait(-2);
}

//-----------------------------------------------------------------------------

int rtlClient::HandleButtonDown (vevent* ev)
{
    gInit (rtlClient::HandleButtonDown);

    lastModifiers_ = ev->GetKeyModifiers ();
    lastModifiers_ &= ~vkeyMODIFIER_BUTTON;

    flag_ = 0;

    return gdListItem::HandleButtonDown (ev);
}

//-----------------------------------------------------------------------------

int rtlClient::HandleKeyDown(vevent* ev)
{
    vkeyStroke stroke = ev->GetKeyStroke();
	vkey Key = vkeyGetStrokeKey(stroke);
    vchar theKey = vkeyToChar(Key);
    vkeyModifiers theMod = vkeyGetStrokeModifiers(stroke);

    static int alt_escape = 0;

    if((Key == vkeyESCAPE) || (theMod == vkeyMODIFIER_KEY_ALT))
       alt_escape = 1;

    if ( (theMod == vkeyMODIFIER_KEY_COMMAND) && ( theKey == '/')  ) {
        vlistSelection* sel = GetSelection ();
        if (sel) GetList()->SelectAll(sel);
        alt_escape = 0;
        return vTRUE;

    } else if ( (theMod == vkeyMODIFIER_KEY_COMMAND) && (theKey == '\\') ) {
        vlistSelection* sel = GetSelection ();
        if (sel) GetList()->UnselectAll(sel);
        alt_escape = 0;
        return vTRUE;

    } else if (   ((theKey >= 'a') && (theKey <= 'z')) ||
                  ((theKey >= 'A') && (theKey <= 'Z')) ||
                  ((theKey >= '0') && (theKey <= '9')) ||
                  (theKey == '_') || (theKey == '-') || (theKey == '/') ||
                  (theKey == ':') || (theKey == '.') || (theKey == '%') ||
                  (theKey == '&') || (theKey == ',') || (theKey == '<') ||
                  (theKey == '>') ) {
          int mode;
          if(theMod == vkeyMODIFIER_KEY_COMMAND) 
             mode = 1;
          else if(((theKey == '<') || (theKey == '>')) && alt_escape) 
             mode = 2;
          else
             mode = 0;
          alt_escape = 0;
          char keyname[2];
          keyname[0] = (char) theKey;
          keyname[1] = '\0';
          int spec = rcall_dis_rtl_processKey (Application::findApplication("DISmain"), 
		       serverId_, (vstr*) keyname, mode, startat_);
          if(spec >= 0) {
             select(spec);
             flag_ = 1;
             ObserveSelect(vlistviewSELECTION_HAS_CHANGED);
             flag_ = 0;
             ScrollSelectionIntoView();
          }
          return vTRUE;
    } else 
		return gdListItem::HandleKeyDown (ev);
}

//-----------------------------------------------------------------------------

void rtlClient::ObserveSelect (int state)
{
    gInit (rtlClient::ObserveSelect);


    if (!selecting && state == vlistviewSELECTION_HAS_CHANGED) {

	Tcl_Interp*		intr = 0;

	Layer* layer = getLayer ();
	if (layer)
	    intr = layer->getInterp ();
        else {
	    GDialog* dialog = GDialog::CastDown(DetermineDialog());
	    if (dialog)
	        intr = dialog_->GetInterpreter();
	    else {	
	        Viewer* vr;
	        View* v;
                Viewer::GetExecutingView (vr, v);
		if (v)
		    intr = v->interp();
	    }
	}

	vlistIterator iter;
	vlistSelection* sel = GetSelection();
	iter.StartWithSelectionRange (sel);

	int			first = 1, total = 0;
	gString			str;

	while (iter.Next()) {
            if(!flag_) {      // for handling keydown
               startat_ = iter.GetRow();
            } else 
               startat_ =  -1;

	    int	rows = iter.GetRangeRowCount ();
	    if (rows < 1)
		continue;

	    total += rows;

	    if (localOnly_)
		continue;

	    if (!first)
		str += ',';
	    else first = 0;

	    vrectLong	my_rect;
	    gString	segment;

	    iter.GetRange (&my_rect);
	    if (rows == 1)
		segment.sprintf_scribed (vcharScribeLiteral("%d"), my_rect.y);
	    else
		segment.sprintf_scribed (vcharScribeLiteral("%d-%d"),
				    my_rect.y, my_rect.y + my_rect.h - 1);
	    str += segment;
	}

	iter.Finish ();

	if (filter_) {
		int unfilteredSize = rcall_dis_rtl_get_unfilteredSize (Application::findApplication("DISmain"), serverId_);
		int listSize = rcall_dis_rtl_get_size (Application::findApplication("DISmain"), serverId_);
		filter_->SetDisplay (total, unfilteredSize == listSize ? -1 : GetItemCount(), unfilteredSize);
	}

	if (intr) {
	    Tcl_SetVar (intr, "Selection", str, TCL_GLOBAL_ONLY);
	}

	if (!localOnly_) { 
	    rcall_dis_rtl_select (Application::findApplication("DISmain"), 
			       serverId_, (vstr*)(vchar*)str);

            if (intr) {
                // Set global RTL and SeverRTL vars.
                gString rtl;
                rtl.sprintf((vchar*)"%d", id_);
                Tcl_SetVar(intr, "GLOBAL_LastRTL", (char*)(vchar*)rtl,
                           TCL_GLOBAL_ONLY);
                rtl.sprintf((vchar*)"%d", serverId_);
                Tcl_SetVar(intr, "GLOBAL_LastServerRTL", (char*)(vchar*)rtl,
                           TCL_GLOBAL_ONLY);
            }
	}
    }
    gdListItem::ObserveSelect(state);
}

//-----------------------------------------------------------------------------

void rtlClient::queryCategories (const vchar* which)
{
    gInit (rtlClient::queryCategories);

    if (!which)
	return;

    rcall_dis_rtl_init (Application::findApplication("DISmain"), 
						     serverId_, (vstr*) which);
}

//-----------------------------------------------------------------------------

int rtlClient::queryProjects (rtlClient* src)
{
    gInit (rtlClient::queryProjects);
    int is_module = 0;

    if (!src) {
	rcall_dis_rtl_query_top_proj (Application::findApplication("DISmain"), serverId_);
    }
    else {

	int		r, row = -1;
	vlistIterator	iter;

	iter.StartWithSelectionRange (src->GetSelection());
	while (iter.Next())
	    if (((r = iter.GetRow()) < row) || (row < 0))
		row = r;
	iter.Finish ();

	if (row >= 0) {	
	    Application *app = Application::findApplication("DISmain"); 
	    if( app )
	    {
		is_module = rcall_dis_is_module (app,
				      src->serverId_, row );
		if( !is_module )
		    rcall_dis_rtl_query_proj (app,
				      src->serverId_, row, serverId_);
	    }
	}
    }
    return is_module;
}

//-----------------------------------------------------------------------------

void rtlClient::queryParentProject (void)
{
    gInit (rtlClient::queryParentProject);

    rcall_dis_rtl_query_parent_proj (Application::findApplication("DISmain"), serverId_);
}

//-----------------------------------------------------------------------------

void rtlClient::queryLinks (rtlClient* src, const vchar* which)
{
    gInit (rtlClient::queryLinks);

    if (!src || !which)
	return;

    rcall_dis_rtl_query_links (Application::findApplication("DISmain"), 
			 src->serverId_, (vstr*) which, serverId_);
}

//-----------------------------------------------------------------------------

void rtlClient::queryLink (rtlClient* src, const vchar* which)
{
    gInit (rtlClient::query);

    if (!src || !which)
	return;

    rcall_dis_rtl_query_links (Application::findApplication("DISmain"), 
			 src->serverId_, (vstr*) which, serverId_);
    
}

//-----------------------------------------------------------------------------

void rtlClient::openItem (void)
{
    gInit (rtlClient::openItem);

    rcall_dis_open_selection (Application::findApplication("DISmain"), serverId_, 0);
 
}

//-----------------------------------------------------------------------------

// saves selection for single update call
void rtlClient::preserveSelection (void)
{
    gInit (rtlClient::preserveSelection);

    preserve_selection = 1;
}

//-----------------------------------------------------------------------------

int rtlClient::EditCopy(rtlClient * item)
{
    gInit (rtlClient::EditCopy);

    int ret_val = -1;

    if (item) {
	rcall_dis_rtl_edit_copy(Application::findApplication("DISmain"), serverId_, item->serverId_);
	ret_val = 0;
    }

    return ret_val;
}

//-----------------------------------------------------------------------------

int rtlClient::EditPaste(rtlClient * item)
{
    gInit (rtlClient::EditPaste);

    int ret_val = -1;

    if (item) {
	rcall_dis_rtl_edit_paste(Application::findApplication("DISmain"), serverId_, item->serverId_);
	ret_val = 0;
    }

    return ret_val;
}

//-----------------------------------------------------------------------------

void rtlClient::clear (void)
{
    gInit (rtlClient::clear);

    rcall_dis_rtl_clear (Application::findApplication("DISmain"), serverId_);
}

//-----------------------------------------------------------------------------

void rtlClient::update (void)
{
    gInit (rtlClient::update);


    show_interpreter_wait (-1);

    inUpdate_ = vTRUE;


    //-----------------------------------------------------
    // unselect client; server should already 

    if(preserve_selection)
      preserve_selection = 0;
    else
      unselectLocal_ ();

    //-----------------
    // adjust list size
    //-----------------

    int size = rcall_dis_rtl_get_size (Application::findApplication("DISmain"), serverId_);
    if (size != GetItemCount()) {
        SetItemCount (size);
		if (filter_) {
			int unfilteredSize = rcall_dis_rtl_get_unfilteredSize (Application::findApplication("DISmain"), serverId_);
			filter_->SetDisplay (0, (unfilteredSize == size) ? -1 : size, unfilteredSize);
		}
    }

    //---------------------------------
    // reset all items to empty strings
    //---------------------------------

    for (int count = GetItemCount(), idx = 0; idx < count; idx++)
        SetItemValue (vcharGetEmptyString(), idx);

    contentWidth_ = 0;

    //---------------------------------
    // adjust visuals according to type
    //---------------------------------

    int			type = rcall_dis_rtl_get_type (Application::findApplication("DISmain"), serverId_);
    clientButton*	b;

    deleteAllButtons ();
    switch (type) {
	case rtlProject:
	    b = addTextButtonScribed (vcharScribeLiteral("Up"));
	    if (b)
		b->SetObserveDialogItemProc (upButtonObserveProc);
	    break;
	default:
	    break;
    }

    //----------------------------
    // set list title if necessary
    //----------------------------

    if (label_) {
	vstr* my_title = rcall_dis_rtl_get_title (Application::findApplication("DISmain"), serverId_);
        if (my_title)
            label_->SetTitleScribed (vstrScribe (my_title));
    } else if (centeredLabel_) {
	vstr* my_title = rcall_dis_rtl_get_title (Application::findApplication("DISmain"), serverId_);
        if (my_title)
            centeredLabel_->SetTitleScribed (vstrScribe (my_title));
    }

    //---------------------
    // invalidate list view
    //---------------------

    //refresh_ ();
    inUpdate_ = vFALSE;

    update_called = 1;
    show_interpreter_wait(-2);
}

//-----------------------------------------------------------------------------

void rtlClient::SetRect (const vrect* my_rect)
{
    gInit (rtlClient::SetRect);

    short	si = GetDefaultSelectionInset ();
    int		junk, my_left, my_right;

    GetDecorationSizes (&junk, &my_left, &junk, &my_right);
    interiorWidth_ = my_rect->GetWidth() - my_left - my_right - 2 * si;

    if (contentWidth_ < interiorWidth_)
	SetColumnWidth (vlistviewALL_COLUMNS, interiorWidth_);
    else SetColumnWidth (vlistviewALL_COLUMNS, contentWidth_);

    gdListItem::SetRect (my_rect);
}

//-----------------------------------------------------------------------------

void rtlClient::Init (void)
{
    gInit (rtlClient::Init);

    gdListItem::Init ();
    doInit ();
}

//-----------------------------------------------------------------------------

void rtlClient::LoadInit (vresource res)
{
    gInit (rtlClient::LoadInit);

    gdListItem::LoadInit (res);
    doInit ();
}

//-----------------------------------------------------------------------------

void rtlClient::doInit (void)
{
    gInit (rtlClient::doInit);

    //-------------------------
    // initialize local members
    //-------------------------

    id_			= serial_++;
    layer_		= 0;
    label_		= 0;
    centeredLabel_	= 0;
    sdObj_		= 0;
    parent_		= 0;
    filter_		= 0;
    inUpdate_		= vFALSE;
    springDomain_	= 0;
    contentWidth_	= 0;
    interiorWidth_	= 0;
    preserve_selection  = 0;
    lastModifiers_	= vkeyNULL_MODIFIERS;

    //-----------------------
    // create icon dictionary
    //-----------------------

    iconDict_		= new vdict (iconDictHashFunc);

    //-------------------
    // add to global list
    //-------------------

    next_ 		= list_;
    list_		= this;

    //--------------------------------------
    // create corresponding object on server
    //--------------------------------------

    serverId_ = rcall_dis_rtl_create (Application::findApplication("DISmain"), id_);

    //----------------------
    // determine item height
    //----------------------

    vfont*	font = DetermineFont ();

    int		ht = font->Height ();

    if (ht < rtlIconHeight)
	ht = rtlIconHeight;

    SetItemHeight (ht + 6);

    //-----------------------------
    // miscellaneous initialization
    //-----------------------------

    SetSelectMethod (vlistFREE_SELECTION);
    SetAutowidth (vFALSE);

    //---------------------------
    // make sure icons are loaded
    //---------------------------

    if (rtlIcon.size() < 1) {
        int numNames = sizeof(iconName)/sizeof(char*);
        // The unknown icon is always the first icon.
        int numKinds = iconNumKinds - iconUnknown;
        if (numNames != numKinds) 
printf("WARNING! There are %d icon names and %d icons!\n", numNames, numKinds);
//        dis_message((vchar*)"WARNING! There are %d icon names and %d icons!",
//		numNames, numKinds);

	vresource dictRes;
        vresource iconRes;
	if (getResource("rtlIcons", &dictRes) ) {
            for (int i = 0; i < numNames; i++)
		if (iconName[i]) {
                    if (getResource(dictRes, iconName[i], &iconRes) )
                        rtlIcon.append(new vimage(iconRes) );
		} else
			printf("Icon #%d has a NULL Name.\n", i);
        }
    }

    // enable focus-ability to ensure that we get HandleKeyDown events for select all
    // and searches
    SetFocusable(vTRUE);
}

//-----------------------------------------------------------------------------

void rtlClient::postLoadInit (Layer* lp)
{
    gInit (rtlClient::postLoadInit);

    //-------------
    // assign layer
    //-------------

    layer_ = lp;

    //---------------------
    // load command strings
    //---------------------

    const vname* my_tag = GetTag ();
    if (!my_tag)
	my_tag = vnameInternGlobalLiteral("UnknownList");

    vresource	dictRes;
    if (getResource("rtlCommands", &dictRes) ) {
        vresource   listRes;
        if (getResource(dictRes, my_tag, &listRes) ) {
            vliststr *   list = new vliststr(listRes);
            selCmd_.put_value_scribed (list->ScribeCellValue (0, 0));
            actCmd_.put_value_scribed (list->ScribeCellValue (1, 0));
            delete list;
        }
    }

    //------------------------
    // find associated objects
    //------------------------

    vdialogItemList* dil = GetItemListIn ();
    if (!dil) return;

    vcontainer* c = parent_ = vcontainer::CastDown (dil->GetOwner());
    if (!c) return;

    const vname* labelName = vnameInternGlobalLiteral("rtlLabel");
    label_ = gdLabel::CastDown(c->FindItem(labelName) );
    if (!label_) 
        centeredLabel_ = gCenteredLabel::CastDown(c->FindItem(labelName) );

    if (filter_) filter_->SetDisplay (0, -1, 0);

    //------------------------------------
    // resolve spring solution into domain
    //------------------------------------

    vspringSolution* sol = c->GetSolution ();
    if (!sol)
	return;

    vspringdom* sd = springDomain_ = vspringdom::Decompile (sol);
    if (!sd)
	return;

    int			count = c->GetItemCount ();
    int			idx;
    vdialogItem*	itm;
    vspringdomObject*	obj;

    for (idx = 0, obj = (vspringdomObject*) sd->GetTopObject ();
	 obj && (idx < count);
	 idx++, obj = (vspringdomObject*) sd->GetNextObjectBehind (obj)) {
	if ((itm = c->GetItemAt (idx)) == filter_)
	    sdObjArray_.append (obj);
	else if (itm == this)
	    sdObj_ = obj;
	sd->SetObjectData (vspringdomObjectToSet (obj), itm);
    }
}

//-----------------------------------------------------------------------------

void rtlClient::setFilterButton (vchar *filterName) {

    vdialogItemList* dil = GetItemListIn ();
    if (!dil) return;

    vcontainer* c = parent_ = vcontainer::CastDown (dil->GetOwner());
    if (!c) return;

    filter_ = filterButton::CastDown(c->FindItem(
                            vnameInternGlobal(filterName)));
    if (filter_) {
    	filter_->SetrtlClinet (id_);
    }

    vstr *filterResetName = vstrCopyMulti(filterName, (vchar *)"Reset", NULL, NULL);
    filterResetButton* filter_reset = filterResetButton::CastDown(c->FindItem(
                                      vnameInternGlobal(filterResetName)));
    vstrDestroy (filterResetName);
    if (filter_reset) {
        filter_reset->SetrtlClient (id_);
    }
}

//-----------------------------------------------------------------------------

clientButton* rtlClient::addTextButtonScribed (vscribe* scr)
{
    gInit (rtlClient::addTextButtonScribed);

    clientButton* b = new clientButton;

    b->setParent (this);

    if (scr)
        b->SetTitleScribed (scr);

    if (filter_)
	b->SetFont (filter_->GetFont());

    clientButton* r = addButton (b);
    if (!r)
	delete b;

    return r;
}

//-----------------------------------------------------------------------------

clientButton* rtlClient::addButton (clientButton* b)
{
    gInit (rtlClient::addButton);

    vspringdomConnection*	c;
    vspringdom*			sd = springDomain_;
    int				sz = sdObjArray_.size ();

    if (!b || !parent_ || !sd || !sdObj_ || (sz < 1))
	return 0;

    parent_->AppendItem (b);
    parent_->SetItemIndex (b, 0);

    vspringdomObject* obj = (vspringdomObject*) sd->AddObject ();
    sd->SetObjectData (vspringdomObjectToSet (obj), b);

    vspringdomObject* border = sd->GetBorder ();
    vspringdomObject* prev = sdObjArray_[sz - 1];

    c = sd->GetObjectConnection (obj, vspringdomTOP_OUTSIDE);
    sd->ConnectToObjectEdge (c, sdObj_, vspringdomBOTTOM);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vTRUE);
    sd->SetConnectionLocked (c, vTRUE);

    c = sd->GetObjectConnection (obj, vspringdomBOTTOM_OUTSIDE);
    sd->ConnectToObjectEdge (c, border, vspringdomBOTTOM);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomLEFT_OUTSIDE);
    sd->ConnectToObjectEdge (c, border, vspringdomLEFT);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomRIGHT_OUTSIDE);
    sd->ConnectToObjectEdge (c, prev, vspringdomLEFT);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomTOP_INSIDE);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomBOTTOM_INSIDE);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomLEFT_INSIDE);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (obj, vspringdomRIGHT_INSIDE);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    c = sd->GetObjectConnection (prev, vspringdomLEFT_OUTSIDE);
    sd->ConnectToObjectEdge (c, obj, vspringdomRIGHT);
    sd->SetConnectionFixedBase (c, 0);
    sd->UseConnectionNaturalBase (c, vFALSE);
    sd->SetConnectionStretches (c, vFALSE);
    sd->SetConnectionLocked (c, vFALSE);

    sdObjArray_.append (obj);

    parent_->SetSolution (sd->Compile());

    return b;
}

//-----------------------------------------------------------------------------

void rtlClient::removeButton (clientButton* b, int destroy)
{
    gInit (rtlClient::removeButton);
    
    vspringdomConnection*	c;
    vspringdom*			sd = springDomain_;
    int				sz = sdObjArray_.size ();

    if (!b || !parent_ || !sd || !sdObj_ || (sz < 2))
	return;

    if (b->GetItemListIn() != parent_->GetItemList())
	return;

    vspringdomObject*		obj;
    vspringdomObject*		my_left;
    vspringdomObject*		my_right;
    vspringdomObject*		border = sd->GetBorder ();

    for (int i = 1; i < sz; i++) {

	if (!(obj = sdObjArray_[i]))
	    continue;
	if (((clientButton*) sd->GetObjectData (obj)) != b)
	    continue;

	my_right = sdObjArray_[i - 1];
	my_left = (i < (sz - 1)) ? sdObjArray_[i + 1] : border;

	if (my_right) {
	    c = sd->GetObjectConnection (my_right, vspringdomLEFT_OUTSIDE);
	    if (!my_left || (my_left == border))
	        sd->ConnectToObjectEdge (c, border, vspringdomLEFT);
	    else sd->ConnectToObjectEdge (c, my_left, vspringdomRIGHT);
	}

	if (my_left && (my_left != border)) {
	    c = sd->GetObjectConnection (my_left, vspringdomRIGHT_OUTSIDE);
	    if (!my_right || (my_right == border))
		sd->ConnectToObjectEdge (c, border, vspringdomRIGHT);
	    else sd->ConnectToObjectEdge (c, my_right, vspringdomLEFT);
	}

	sdObjArray_.remove (i);
	sd->RemoveObjects (vspringdomObjectToSet (obj));

	parent_->DeleteItem (b);
	if (destroy)
	    b->DeleteLater ();

        parent_->SetSolution (sd->Compile());
        parent_->Arrange (parent_->GetRect());

	return;
    }
}

//-----------------------------------------------------------------------------

void rtlClient::deleteAllButtons (void)
{
    gInit (rtlClient::deleteAllButtons);

    vspringdom*	sd = springDomain_;
    int		sz = sdObjArray_.size ();

    if (!parent_ || !sd || !sdObj_ || (sz < 2))
	return;

    vspringdomObject*	obj;
    clientButton*	b;

    for (int i = 0; i < sz; i++) {
	if (!(obj = sdObjArray_[i]))
	    continue;
	if (!(b = (clientButton*) sd->GetObjectData (obj)))
	    continue;
	removeButton (b);
    }
}

//-----------------------------------------------------------------------------

void rtlClient::Destroy (void)
{
    gInit (rtlClient::Destroy);

    vdictIterator iter;

    iter.Start (iconDict_);
    while (iter.Next())
	delete [] (vchar*) iter.GetValue ();
    iter.Finish ();

    iconDict_->Destroy();

    // Delete rtlIcon vimage members.
    if (rtlIcon.size() ) {
        for (int i=0; i++; i<rtlIcon.size())
            delete rtlIcon[i];
    }

    // Remove ourself from the list of rtls
    rtlClient* prev = NULL;
    for (rtlClient* r = list_; r; r = r->next_) {
	if (r->getId() == id_)
	    break;
	else
	    prev = r;
    }
    if (r) {
	if (prev)
	    prev->next_ = next_;
	else
	    list_ = next_;
    }

    

    gdListItem::Destroy();
}

//-----------------------------------------------------------------------------

void clientButton::Init (void)
{
    gInit (clientButton::Init);

    vbutton::Init ();
    parent_ = 0;
}
