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
//**********
//
// clipboard.cxx - Gala application TCL interpreter functions.
//
//**********

#include <vport.h>
#include vdialogHEADER
#include vdragHEADER
#include vcontainerHEADER
#include vstdioHEADER
#include vstdlibHEADER
#include vapplicationHEADER
#include vliststrHEADER
#include vcolorchsrHEADER
#include vcolorHEADER
#include vresourceHEADER
#include vmenubarHEADER
#include vmenuHEADER
#include vtextitemHEADER

#include <ggeneric.h>
#include <gprefs.h>
#include <ginterp.h>
#include <gview.h>
#include <gviewer.h>
#include <grtlClient.h>
#include <gpaneObjects.h>
#include <gprefs.h>
#include <gapl_menu.h>
#include <dis_confirm.h>
#include <glayer.h>
#include "shelltextitem.h"


// -- CLIPBOARD ----------------

#include <gclipboard.h>

gxClipBoard *
gxClipBoard::app_clipboard = (gxClipBoard *)0;

vobjectAttributed *
gxClipBoard::GetClipSelection(vdialogItemList * item_list)
{
    vobjectAttributed * ret_val = (vobjectAttributed *)0;

    if (item_list) {
	int size = item_list->GetCount();
	for (int i=0; i<size && !ret_val; i++) {
	    vobjectAttributed * obj = item_list->GetAt(i);
	    if (obj) {
		if (obj->IsKindOf(&vnotebook::Kind))
		    ret_val = GetClipSelection(vnotebook::CastDown(obj)->GetItemList());
		else if (obj->IsKindOf(&vcontainer::Kind))
		    ret_val = GetClipSelection(vcontainer::CastDown(obj)->GetItemList());
		else if (obj->IsKindOf(&vlistitem::Kind) || obj->IsKindOf(&vtextitem::Kind)) {
		    // -- get only visible ones
		    vdialogItem * item = vdialogItem::CastDown(obj);
		    if (item->IsVisible() && item->IsFocused())
			ret_val = obj;
		}
	    }
	}
    }

    return ret_val;
}

int
gxClipBoard::Cut(vobjectAttributed * item)
{
  int ret_val = -1;

  if (!app_clipboard)
    app_clipboard = new gxClipBoard(vTRUE);
  if (app_clipboard!=0 && item!=0) 
  {
    //disallow 'Cut' for shellTextItem
    if (item->IsKindOf(&vtextitem::Kind) )
    {
      if( item->IsKindOf(&shellTextItem::Kind) )
        ret_val = app_clipboard->Copy(vtextitem::CastDown(item));
      else
        ret_val = app_clipboard->Cut(vtextitem::CastDown(item));
    }
  }
  return ret_val;
}

int
gxClipBoard::Copy(vobjectAttributed * item)
{
    int ret_val = -1;

    if (!app_clipboard)
	app_clipboard = new gxClipBoard(vTRUE);
    if (app_clipboard!=0 && item!=0) {
	if (item->IsKindOf(&vtextitem::Kind))
	    ret_val = app_clipboard->Copy(vtextitem::CastDown(item));
	else if (item->IsKindOf(&rtlClient::Kind))
	    ret_val = app_clipboard->Copy(rtlClient::CastDown(item));
	else if (item->IsKindOf(&vlistitem::Kind))
	    ret_val = app_clipboard->Copy(vlistitem::CastDown(item));
    }

    return ret_val;
}

int
gxClipBoard::Paste(vobjectAttributed * item)
{
    int ret_val = -1;

    if (!app_clipboard)
	app_clipboard = new gxClipBoard(vTRUE);
    if (app_clipboard!=0 && item!=0) {
	if (item->IsKindOf(&vtextitem::Kind))
	    ret_val = app_clipboard->Paste(vtextitem::CastDown(item));
	else if (item->IsKindOf(&rtlClient::Kind))
	    ret_val = app_clipboard->Paste(rtlClient::CastDown(item));
    }

    return ret_val;
}

int
gxClipBoard::Destroy()
{
    int ret_val = -1;

    if (app_clipboard)
	delete app_clipboard;

    return ret_val;
}

void
gxClipBoard::ClipRecvCB(vclipboard * clip_obj, vscrap * data)
{
    if (app_clipboard) {
	if (app_clipboard->clip_ctrl == clip_obj)
	    app_clipboard->clip_data = data;
    }
}

gxClipBoard::gxClipBoard(vbool global)
{
    clip_data     = (vscrap *)0;
    clip_data_rtl = new rtlClient();

    if (global)
	clip_ctrl = vclipboard::CreateGlobalClipboard();
    else
	clip_ctrl = vclipboard::CreateSelectionClipboard();

    if (clip_ctrl)
	clip_ctrl->SetReceiveProc(&gxClipBoard::ClipRecvCB);
}

gxClipBoard::~gxClipBoard()
{
    if (clip_data)
	delete clip_data;
    if (clip_data_rtl)
	delete clip_data_rtl;
    if (clip_ctrl)
	delete clip_ctrl;
}

int
gxClipBoard::ReadClipData()
{
    int ret_val = -1;

    if (app_clipboard) {
	if (clip_ctrl->HasContents()) {
	    clip_ctrl->RequestContents();
	    if (clip_data)
		ret_val = 0;
	}
    }

    return ret_val;
}

int
gxClipBoard::Cut(vtextitem * item)
{
    int ret_val = -1;
    if (item) 
    {
      vdict dict(vnameHash);
      item->GetAllAttributes( &dict );
      item->IssueSelector( vdialogITEM_SELECT_CUT, &dict );
      ret_val = 0;
    }  
    return ret_val;
}

#if 0
int
gxClipBoard::Cut(vlistitem * item)
{
    int ret_val = -1;

    if (item) {
	int selndx = item->GetSelectedItem();
	if (selndx >= 0) {
	    if (Copy(item) >= 0) {
		// Modifiable() ?
		item->RemoveItem(selndx);
		ret_val = 0;
	    }
	}
    }

    return ret_val;
}
#endif

int
gxClipBoard::Copy(vtextitem * item)
{
    int ret_val = -1;
    if (item) 
    {
      vdict dict(vnameHash);
      item->GetAllAttributes( &dict );
      item->IssueSelector( vdialogITEM_SELECT_COPY, &dict );
      ret_val = 0;
    }  
    return ret_val;
}

int
gxClipBoard::Copy(rtlClient * item)
{
    int ret_val = -1;

    if (item) {
	int selndx = item->GetSelectedItem();
	if (selndx >= 0) {
	    if (clip_data_rtl)
		item->EditCopy(clip_data_rtl);
	    ret_val = Copy((vlistitem *)item);
	}
    }

    return ret_val;
}

int
gxClipBoard::Copy(vlistitem * item)
{
    int ret_val = -1;

    if (item) {
	vlistSelection * sel = item->GetSelection();
	if (sel) {
	    int length = 0;

	    // -- compute length of the output
	    if (length <= 0) {
		vlistIterator iter;
		iter.StartWithSelection(sel);
		while (iter.Next()) {
		    int ndx = iter.GetRow();
		    if (ndx >= 0) {
			const vchar * rowval = item->GetItemValue(ndx);
			if (rowval)
			    length += vcharLength(item->GetItemValue(ndx));
			length += 4; // -- number of characters to append
		    }
		}
		iter.Finish();
	    }
	    // -- output strings
	    if (length > 0) {
		int    outndx = 0;
		vstr * output = vstrCreateSized(length + 1);
		if (output) {
		    vlistIterator iter;
		    iter.StartWithSelection(sel);
		    while (iter.Next()) {
			int ndx = iter.GetRow();
			if (ndx >= 0) {
			    const vchar * rowval = item->GetItemValue(ndx);
		            if (rowval) {
				int rowlen = vcharLength(rowval);
				if (rowlen > 0) {
				    vcharSizedCopy(rowval, rowlen, &output[outndx]);
				    outndx += rowlen;
				}
#ifdef _WIN32
				output[outndx] = '\r'; outndx++;
#endif
				output[outndx] = '\n'; outndx++;
			    }
			}
		    }
		    iter.Finish();
		    output[outndx] = '\0';
		    // -- remove any special characters that we may find in lists
		    for (int outclp=0; outclp<outndx; outclp++) {
			if (output[outclp]<' ' && output[outclp]!='\r' && output[outclp]!='\n')
			    output[outclp] = ' ';
		    }
			
		    // -- clip the string
		    vscrap * clip = vscrapFromString(output);
		    if (clip) {
			clip_ctrl->SetContents(clip);
			if (clip_ctrl->HasContents())
			    ret_val = 0;
		    }
		    vstrDestroy(output);
		}
	    }
	}
    }

    return ret_val;
}

int
gxClipBoard::Paste(vtextitem * item)
{
    int ret_val = -1;
    if (item) 
    {
      vdict dict(vnameHash);
      item->GetAllAttributes( &dict );
      item->IssueSelector( vdialogITEM_SELECT_PASTE, &dict );
      ret_val = 0;
    }  
    return ret_val;

}

int
gxClipBoard::Paste(rtlClient * item)
{
    int ret_val = -1;

    if (item) {
	if (clip_data_rtl)
	    clip_data_rtl->EditPaste(item);
	ret_val = 0;
    }

    return ret_val;
}

#if 0
int
gxClipBoard::Paste(vlistitem * item)
{
    int ret_val = -1;

    if (item) {
	int selndx = item->GetSelectedItem();
	// -- Modifiable() ?
	if (selndx >= 0) {
	    vtextitem * txt = new vtextitem();
	    if (txt) {
		if (Paste(txt) >= 0) {
		    vstr * inp = txt->GetTextAsString();
		    if (inp) {
			item->Insert(selndx, inp);
			vstrDestroy(inp);
			ret_val = 0;
		    }
		}
		delete txt;
	    }
	}
    }

    return ret_val;
}
#endif

// -- TCL Interface

int
dis_clipboard (ClientData, Tcl_Interp*, int argc, char* argv[])
{
    if (argc > 1) {
	View *   vw  = (View *)0;
	Viewer * vwr = (Viewer *)0;

	Viewer::GetExecutingView(vwr, vw);
	if (vw!=0 && vwr!=0) {
	    vcontainer * vcnt = vw->GetPane();
	    if (vcnt) {
		vobjectAttributed * obj = gxClipBoard::GetClipSelection(vcnt->GetItemList());
		if (obj) {
		    if (!strcmp(argv[1], "-copy"))
			(void) gxClipBoard::Copy(obj);
		    else if (!strcmp(argv[1], "-paste"))
			(void) gxClipBoard::Paste(obj);
		    else if (!strcmp(argv[1], "-cut"))
			(void) gxClipBoard::Cut(obj);
		}
	    }
	}
    }
 
    return TCL_OK;
}
