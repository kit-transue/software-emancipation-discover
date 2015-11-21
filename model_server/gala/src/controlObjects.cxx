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
#include vsliderHEADER 
#include vbuttonHEADER 
#include vcontrolHEADER
#include vtextitemHEADER
#include vtextHEADER
#include vcontainerHEADER
#include vcomboHEADER
#include vcomboboxHEADER
#include vlistitemHEADER
#include vnotebookHEADER
#include vstdlibHEADER
#include vspringdomHEADER
#include vdateHEADER
#include veventHEADER
#include vnameHEADER
#include vimageHEADER
#include vdictHEADER
#include vrectHEADER

#include <Application.h>
#include <gcontrolObjects.h>
#include <ggenDialog.h>
#include <ginterp.h>
#include <gString.h>
#include <gglobalFuncs.h>
#include <gviewer.h>
#include <gview.h>
#include <gbar.h>
#include <grtlClient.h>
#include <ggenDialog.h>
#include <gdraw.h>
#include <gString.h>
#include <TextFile.h>
#include <scanner.h>
#include <gSpreadSheet.h>

#include <cLibraryFunctions.h>
#include <machdep.h>



vkindTYPED_FULL_DEFINITION(ListViewPopupMenu, vmenu, "ListViewPopupMenu");
vkindTYPED_FULL_DEFINITION(ListViewPopupMenuItem, vmenuItem, "ListViewPopupMenuItem");
vkindTYPED_FULL_DEFINITION(ListViewTextItem, vtextitem, "ListViewTextItem");
vkindTYPED_FULL_DEFINITION(ListViewItemList, vdialogItemList, "ListViewItemList");

vkindTYPED_FULL_DEFINITION(springContainer, vcontainer, "springContainer");
vkindTYPED_FULL_DEFINITION(gdButton, vbutton, "gdButton");
vkindTYPED_FULL_DEFINITION(gdOptionMenu, vcontrolOptionMenu, "gdOptionMenu");
vkindTYPED_FULL_DEFINITION(gdListItem, vlistitem, "gdListItem");
vkindTYPED_FULL_DEFINITION(gdIconListItem, gdListItem, "gdIconListItem");
vkindTYPED_FULL_DEFINITION(gdListView, vlistview, "gdListView");
vkindTYPED_FULL_DEFINITION(gdNumberSpinner, vspinner, "gdNumberSpinner");

vkindTYPED_FULL_DEFINITION(gdTextSpinner, vspinnerText, "gdTextSpinner");
vkindTYPED_FULL_DEFINITION(gdNotebookPage, vnotebookPage, "gdNotebookPage");
vkindTYPED_FULL_DEFINITION(gdNotebook, vnotebook, "gdNotebook");
vkindTYPED_FULL_DEFINITION(gdProgressItem, ProgressItem, "gdProgressItem");
vkindTYPED_FULL_DEFINITION(gdTextObserver, vtextObserver, "gdTextObserver");
vkindTYPED_FULL_DEFINITION(gdTextEditor, vtextview, "gdTextEditor");
vkindTYPED_FULL_DEFINITION(gdTextItem, vtextitem, "gdTextItem");
vkindTYPED_FULL_DEFINITION(gdQueryTextItem, vtextitem, "gdQueryTextItem");
vkindTYPED_FULL_DEFINITION(gdExclusiveGroup, vcontainerExclusiveGroup, "gdExclusiveGroup");
vkindTYPED_FULL_DEFINITION(gdToggle, vcontrolToggle, "gdToggle");
vkindTYPED_FULL_DEFINITION(gdComboBox, vcombo, "gdComboBox");
vkindTYPED_FULL_DEFINITION(gdPopDownComboBox, vcomboPopdown, "gdPopDownComboBox");
vkindTYPED_FULL_DEFINITION(gdLabel, vdialogLabelItem, "gdLabel");
vkindTYPED_FULL_DEFINITION(gdRuler, vruler, "gdRuler");
vkindTYPED_FULL_DEFINITION(gdSlider, vslider, "gdSlider");
vkindTYPED_FULL_DEFINITION(gdSash, vdialogLineItem, "gdSash");
vkindTYPED_FULL_DEFINITION(gdSashDrag, vdrag, "gdSashDrag");
vkindTYPED_FULL_DEFINITION(gdMultiLineLabel, vtextitem, "gdMultiLineLabel");


#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#undef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))

const int BORDER_THICKNESS = 2;


void show_interpreter_wait(int);
static vspringdomObject *SpingDomObjFromItem( vspringdom *sd, vdialogItem *item );
static vdialogItem *ItemFromSpingDomObj( vcontainer *c, vspringdom *sd, vspringdomObject *obj );

int gdSash::_minWidth;
int gdSash::_minHeight;
vspringdomConnection *gdSash::_target;
static int key_pressed;


/************* gdWidget **********************/

void gdWidget::SetTclCmd(vchar* execCommand)
{
    execCommand_ = new vchar [vcharLength(execCommand)+1];
    vcharCopy((const vchar*)execCommand, execCommand_);
}

// SetInterpVariable - Sets the TCL variable to the given value.
void gdWidget::SetInterpVariable (const vchar* variable, const vchar* value)
{
    if (variable && value) 
	{
		char * tclVar = (char *) vcharExportAlloc(vcharScribeSystemExport(variable));
        char * tclValue =  (char *) vcharExportAlloc(vcharScribeSystemExport(value));
		Tcl_SetVar(interp(), tclVar, tclValue, TCL_GLOBAL_ONLY);
        vcharExportFree(tclVar);
        vcharExportFree(tclValue);
    }
}

// GetInterpVariable - Returns TCL value of the given variable.
vchar* gdWidget::GetInterpVariable (vchar* variable)
{
	vchar *  value = NULL;
	
    if (variable)
        value = (vchar *) Tcl_GetVar(interp(), (char *) variable,
		TCL_GLOBAL_ONLY);
    return value;
}


void gdWidget::InitgdWidget(vchar* tagName, vdialogItem* item)
{
    item_ = item;
	
    Viewer* vr;
    View *view_;
    Viewer::GetExecutingView (vr, view_);
    if ((dialog_||view_) && (tagName))  {
		pane_ = view_ ? view_->GetPane() : NULL;
		if (interp_trace) printf ("%08x [%s]\n", interp(), (char*)tagName);
        // Remove previous trace to prevent double actions.
		Tcl_UntraceVar(interp(), (char*)tagName, TCL_TRACE_WRITES,
			gdWidget::UpdateVariable, (ClientData) this);
        // Add a trace/callback when this variable is changed.
		Tcl_TraceVar(interp(), (char*)tagName, TCL_TRACE_WRITES,
			gdWidget::UpdateVariable, (ClientData) this);
		insideUpdate_ = vFALSE;
    }
}

char* gdWidget::UpdateVariable(ClientData cd, Tcl_Interp*, char *, char *, int)
{
    show_interpreter_wait (1);
    gdWidget* self = (gdWidget*)cd;
    if (!self->insideUpdate_) {
        if (self->item_->IsKindOf (&rtlClient::Kind)) {
            gdListItem* li     = gdListItem::CastDown(self->item_);
            Tcl_Interp* interp = self->interp(); 
            gString cmd;
            cmd.sprintf((vchar*)"lindex $%s 0", li->GetTag() );
            gala_eval(interp, (char*)(vchar*)cmd);
            cmd = (vchar*)Tcl_GetStringResult(interp);
            gala_eval(interp, (char*)(vchar*)cmd);
            int count = atoi(Tcl_GetStringResult(interp));
			
            // scroll the list to the top when changing the value 
            // if the size of the list is less then what will
            //  fit in the list.  This is to fix a galaxy bug. 
            int topRow, leftColumn, numRows, numColumns;
            li->GetVisibleRange (&topRow, &leftColumn, &numRows, &numColumns);
            if (numRows >= count) li->ScrollAbsolute (0, 0);
            li->SetItemCount (count);
            li->InvalView();
			
        } else if ( (self->item_->IsKindOf (&gdListItem::Kind)) || (self->item_->IsKindOf(&gdIconListItem::Kind ))) {
            gdListItem* li     = gdListItem::CastDown(self->item_);
            int row = li->GetSelectedItem();
            while (row != vlistitemNO_CELLS_SELECTED) {
                li->UnselectItem(row);
                int nextRow = li->GetSelectedItem();
                if (nextRow == row) 
                    row = vlistitemNO_CELLS_SELECTED;
                else
                    row = nextRow;
            }
            Tcl_Interp* interp = self->interp(); 
            gString cmd;
            cmd.sprintf((vchar*)"lindex $%s 0", li->GetTag() );
            gala_eval(interp, (char*)(vchar*)cmd);
            cmd = (vchar*)Tcl_GetStringResult(interp);
            gala_eval(interp, (char*)(vchar*)cmd);
            int count = atoi(Tcl_GetStringResult(interp));
			
            // scroll the list to the top when changing the value 
            // if the size of the list is less then what will
            // fit in the list.  This is to fix a galaxy bug. 
            int topRow, leftColumn, numRows, numColumns;
            li->GetVisibleRange (&topRow, &leftColumn, &numRows, &numColumns);
            if (numRows >= count) li->ScrollAbsolute (0, 0);
            li->SetItemCount (count);
            li->InvalView();
        } else if (self->item_->IsKindOf (&gdListView::Kind)) {
            gdListView* li     = gdListView::CastDown(self->item_);
			
			//            int row = li->GetSelectedItem();
			//            while (row != vlistitemNO_CELLS_SELECTED) {
			//                li->UnselectItem(row);
			//                int nextRow = li->GetSelectedItem();
			//                if (nextRow == row) 
			//                    row = vlistitemNO_CELLS_SELECTED;
			//                else
			//                    row = nextRow;
			//            }
			
            Tcl_Interp* interp = self->interp(); 
            gString cmd;
            cmd.sprintf((vchar*)"lindex $%s 0", li->GetTag() );
            gala_eval(interp, (char*)(vchar*)cmd);
            cmd = (vchar*)Tcl_GetStringResult(interp);
            gala_eval(interp, (char*)(vchar*)cmd);
            int count = atoi(Tcl_GetStringResult(interp));
			
            // scroll the list to the top when changing the value 
            // if the size of the list is less then what will
            // fit in the list.  This is to fix a galaxy bug. 
			
            int topRow, leftColumn, numRows, numColumns;
            li->GetVisibleRange (&topRow, &leftColumn, &numRows, &numColumns);
            if (numRows >= count) li->ScrollAbsolute (0, 0);
			
			vlist *l = li->GetList();
            if (l) {
				l->SetColumnCount (li->getColumns());
				l->SetRowCount (count);
			}
			
            li->InvalView();
        } else if (self->item_->IsKindOf (&gdSpreadSheet::Kind)) {
            gdSpreadSheet* li     = gdSpreadSheet::CastDown(self->item_);
			
            Tcl_Interp* interp = self->interp(); 
            gString cmd;
            cmd.sprintf((vchar*)"lindex $%s 0", li->GetTag() );
            gala_eval(interp, (char*)(vchar*)cmd);
            cmd = (vchar*)Tcl_GetStringResult(interp);
            gala_eval(interp, (char*)(vchar*)cmd);
            int count = atoi(Tcl_GetStringResult(interp));
			
            // scroll the list to the top when changing the value 
            // if the size of the list is less then what will
            // fit in the list.  This is to fix a galaxy bug. 
			
            int topRow, leftColumn, numRows, numColumns;
            li->GetVisibleRange (&topRow, &leftColumn, &numRows, &numColumns);
            if (numRows >= count) li->ScrollAbsolute (0, 0);
			
			//	    vlist *l = li->GetList();
			//            if (l) {
			//		l->SetColumnCount (li->getColumns());
			//		l->SetRowCount (count);
			//	    }
			
            li->InvalView();
		} else if (self->item_->IsKindOf (&gdTextItem::Kind)) {
			gdTextItem* ti = gdTextItem::CastDown(self->item_);
			
			self->insideUpdate_ = vTRUE;
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			
			// Multiline text windows should not handle update.
			
			if (ti->IsOneLine()) {
				self->UpdateDisplay();
			}
			self->insideUpdate_ = vFALSE;
			ti->item_has_changed = vFALSE;
			
		} else if (self->item_->IsKindOf (&gdProgressItem::Kind)) {
			self->insideUpdate_ = vTRUE;
			gdProgressItem* pri = gdProgressItem::CastDown(self->item_);
			char* newValue = NULL;
			if (pri) {
				newValue = Tcl_GetVar (self->interp(), (char *)pri->GetTag(), TCL_GLOBAL_ONLY);
				unsigned int max, count, interval;
				int numItems = sscanf (newValue, "%d %d %d", &max, &count, &interval);
				pri->MoveTo (count);
				pri->SetNumberSteps (max);
				if (count == 0) {
					if (numItems == 3) pri->SetInterval ((double)interval);
					else pri->SetInterval ((double)120);
				}
				else {
					pri->SetInterval ((double)pri->ElapsedTime() / count);
				}
			}
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			self->insideUpdate_ = vFALSE;
		} else if (self->item_->IsKindOf (&gdLabel::Kind)) {
			self->insideUpdate_ = vTRUE;
			gdLabel* l = gdLabel::CastDown(self->item_);
			if (l) {
				l->UpdateDisplay();
			}
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			self->insideUpdate_ = vFALSE;
		} else if (self->item_->IsKindOf (&gdMultiLineLabel::Kind)) { 
			gdMultiLineLabel* l = gdMultiLineLabel::CastDown(self->item_);
			if (l)
				l->UpdateDisplay();
		} else if (self->item_->IsKindOf (&gdPopDownComboBox::Kind)) {
			self->insideUpdate_ = vTRUE;
			gdPopDownComboBox* pd = gdPopDownComboBox::CastDown(self->item_);
			if (pd) {
				pd->UpdateDisplay();
			}
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			self->insideUpdate_ = vFALSE;
		} else if (self->item_->IsKindOf (&gdComboBox::Kind)) {
			self->insideUpdate_ = vTRUE;
			gdComboBox* cb = gdComboBox::CastDown(self->item_);
			if (cb) {
				cb->UpdateDisplay();
			}
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			self->insideUpdate_ = vFALSE;
		} else if (self->item_->IsKindOf (&gdExclusiveGroup::Kind)) {
			self->insideUpdate_  = vTRUE;
			gdExclusiveGroup* ex = gdExclusiveGroup::CastDown(self->item_);
			if (ex) ex->UpdateDisplay();
			if (self->execCommand_) gala_eval(self->interp(), (char*)self->execCommand_);
			
			self->insideUpdate_ = vFALSE;
		} else if (self->item_->IsKindOf(&gdNumberSpinner::Kind)) {
			self->insideUpdate_=vTRUE;
			gdNumberSpinner * ns=gdNumberSpinner::CastDown(self->item_);
			if (ns) ns->UpdateDisplay();
			if (self->execCommand_) gala_eval(self->interp(), (char *) self->execCommand_);
			self->insideUpdate_=vFALSE;
		} else {
			self->insideUpdate_ = vTRUE;
			if (self->execCommand_)
				gala_eval(self->interp(), (char*)self->execCommand_);
			self->UpdateDisplay();
			self->insideUpdate_ = vFALSE;
		}
}

show_interpreter_wait(0);
return NULL;
}

/************* gdButton **********************/


void gdButton::Open()
{
    vbutton::Open();
	
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}



int gdButton::HandleButtonUp(vevent *event)
{
	//    SetInterpVariable((vchar*)GetTag(), (vchar*)"");
    return vbutton::HandleButtonUp(event);
}

int gdButton::HandleKeyDown(vevent *event)
{
	return vbutton::HandleKeyDown(event);
}


void gdButton::ObserveDialogItem(vevent *event)
{
    // When a button 'blinks', the binding will be null.
    // We may want to treat the Blink as a Select event.
	
    const vchar* binding = event->GetBinding();
    if (binding && !vcharCompare(binding, (vchar*)"Select") ) {
		SetInterpVariable((vchar*)GetTag(), (vchar*)"");
    } 
	
    else if(!(char *)binding) {  	// triggered by keystrokes
		SetInterpVariable((vchar*)GetTag(), (vchar*)"");
    }
}

/************* gdSash **********************/
void gdSash::Open()
{
    vdialogLineItem::Open();
	
    _prev = _next = 0;
    _left = _right = 0;
	
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
	
	
    const vrect* r = GetRect();
    vresource res;
    if (r->GetHeight() > r->GetWidth()) {
		getResource ("Cursors:HorzPaneResize", &res);
        cursor = new vcursor (res);
    } else {
        getResource ("Cursors:VertPaneResize", &res);
        cursor = new vcursor (res);
    }
    cursor->SetHot(8,8);
}
void gdSash::Activate(void) {
	vdialogLineItem::Activate();
	SetCursor(cursor);
}

void gdSash::Deactivate(void) {
	SetCursor(NULL);
	vdialogLineItem::Deactivate();
}

int  gdSash::HandleButtonDown(vevent* event)
{	
    if( event->GetButton() == 0 )//only for left button
    {
		const vrect* r = GetRect();
		
		
		// Create drag and initialize
		gdSashDrag *drag = new gdSashDrag;
		
		drag->SetSash(this, event);
		vdialogItemList* dil = GetItemListIn ();
		
		if (r->GetHeight() > r->GetWidth()) {
			drag->SetMaxSize(5, r->GetHeight());
			int y;
			vwindow::GetRoot()->GetPointer( &m_nOldPos, &y );
			drag->SetOffset(r->GetWidth()/2, event->GetY());
		} else {
			drag->SetMaxSize(r->GetWidth(),5);
			int x;
			vwindow::GetRoot()->GetPointer( &x, &m_nOldPos );
			drag->SetOffset(event->GetX(), r->GetHeight()/2);
		}
		
		drag->Start(event);
    }
	
    return 0; 
}

void gdSash::SetPrev (gdSash *prev) {
    _prev = prev;
}

void gdSash::SetNext (gdSash *next) {
	_next = next;
}

void gdSash::SetLeft (vdialogItem *left) {
	_left = Pane::CastDown(left);
}

void gdSash::SetRight (vdialogItem *right) {
	_right = Pane::CastDown(right);
}

void gdSash::Draw()
{
	const vrect* r = GetRect();
    vdraw::GSave ();
    DrawDevice();
	
    if (r->GetHeight() > r->GetWidth()) {
        Draw3DTopLeft(r, BORDER_THICKNESS,
            DetermineColor(vdrawCOLOR_SHADOW_TOP));
        Draw3DBottomRight(r, BORDER_THICKNESS,
            DetermineColor(vdrawCOLOR_SHADOW_BOTTOM));
        Draw3DCenter(r, BORDER_THICKNESS, 
			DetermineColor(vdrawCOLOR_BACKGROUND));
    } else {
        Draw3DTopLeft(r, BORDER_THICKNESS,
            DetermineColor(vdrawCOLOR_SHADOW_TOP));
        Draw3DBottomRight(r, BORDER_THICKNESS,
            DetermineColor(vdrawCOLOR_SHADOW_BOTTOM));
        Draw3DCenter(r, BORDER_THICKNESS, 
			DetermineColor(vdrawCOLOR_BACKGROUND));
    }
	
    vdraw::GRestore();
}

void gdSash::MoveLeft (vcontainer *c, vspringdom *sd, int x, int nTotal) {
	
	// See if we can move left and still maintain the minimum width.
	
	int leftWidth = _left->GetWidth();
	int amountToMove = leftWidth - x < _minWidth ? leftWidth - _minWidth : x;
	
	//Get vspringdomObject for the pane to the left
	vspringdomObject* springObj = SpingDomObjFromItem(sd, _left); 
	vspringdomConnection *tempConnection;
	if( springObj )
	{
		tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, leftWidth - amountToMove);
		_left->m_Ratio = float(leftWidth - amountToMove)/float(nTotal);
		
	}
	int nWidth;
	springObj = SpingDomObjFromItem(sd, _right);
	if( springObj )
	{
		nWidth = _right->GetWidth();
		tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, nWidth + amountToMove);
		_right->m_Ratio = float(nWidth + amountToMove)/float(nTotal);
	}
	
	springObj = (vspringdomObject *)sd->GetTopObject ();
	int count = c->GetItemCount ();
	
	for (int idx = 0; springObj && (idx < count);
    idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
	{
		vdialogItem* itm = c->GetItemAt (idx);
		
		Pane *pane  = Pane::CastDown( itm );
		//Only if dialog item is a Pane
		if( pane && pane != _left && pane !=_right )
		{
			nWidth = pane->GetWidth();
			tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
			sd->SetConnectionFixedBase (tempConnection, nWidth);
			pane->m_Ratio = float(nWidth)/float(nTotal);
		}
	}
}


void gdSash::MoveRight (vcontainer *c, vspringdom *sd, int x, int nTotal) {
	
	int rightWidth = _right->GetWidth();
	int amountToMove = rightWidth - x < _minWidth ? rightWidth - _minWidth : x;
	
	//Get vspringdomObject for the pane to the left
	vspringdomObject* springObj = SpingDomObjFromItem(sd, _right); 
	vspringdomConnection *tempConnection;
	if( springObj )
	{
		tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, rightWidth - amountToMove);
		_right->m_Ratio = float(rightWidth - amountToMove)/float(nTotal);
	}
	int nWidth;
	springObj = SpingDomObjFromItem(sd, _left);
	if( springObj )
	{
		nWidth = _left->GetWidth();
		tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, nWidth + amountToMove);
		_left->m_Ratio = float(nWidth + amountToMove)/float(nTotal);
	}
	
	springObj = (vspringdomObject *)sd->GetTopObject ();
	int count = c->GetItemCount ();
	
	for (int idx = 0; springObj && (idx < count);
    idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
	{
		vdialogItem* itm = c->GetItemAt (idx);
		
		Pane *pane  = Pane::CastDown( itm );
		//Only if dialog item is a Pane
		if( pane && pane != _left && pane !=_right )
		{
			nWidth = pane->GetWidth();
			tempConnection = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
			sd->SetConnectionFixedBase (tempConnection, nWidth);
			pane->m_Ratio = float(nWidth)/float(nTotal);
		}
	}
}


void gdSash::MoveDown (vcontainer *c, vspringdom *sd, int y, int nTotal) {
	
	// See if we can move down and still maintain the minimum height.
	
	int downHeight = _left->GetHeight();
	int amountToMove = downHeight - y < _minHeight ? downHeight - _minHeight : y;
	
	//Get vspringdomObject for the pane below
	vspringdomObject* springObj = SpingDomObjFromItem(sd, _left); 
	vspringdomConnection *tempConnection;
	if( springObj )
	{
		tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, downHeight - amountToMove);
		_left->m_Ratio = float(downHeight - amountToMove)/float(nTotal);
		
	}
	int nHeight;
	springObj = SpingDomObjFromItem(sd, _right);
	if( springObj )
	{
		nHeight = _right->GetHeight();
		tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, nHeight + amountToMove);
		_right->m_Ratio = float(nHeight + amountToMove)/float(nTotal);
	}
	
	springObj = (vspringdomObject *)sd->GetTopObject ();
	int count = c->GetItemCount ();
	
	for (int idx = 0; springObj && (idx < count);
    idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
	{
		vdialogItem* itm = c->GetItemAt (idx);
		
		Pane *pane  = Pane::CastDown( itm );
		//Only if dialog item is a Pane
		if( pane && pane != _left && pane !=_right )
		{
			nHeight = pane->GetHeight();
			tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
			sd->SetConnectionFixedBase (tempConnection, nHeight);
			pane->m_Ratio = float(nHeight)/float(nTotal);
		}
	}
}

void gdSash::MoveUp (vcontainer *c, vspringdom *sd, int y, int nTotal) 
{
	
	int topHeight = _right->GetHeight();
	int amountToMove = topHeight - y < _minHeight ? topHeight - _minHeight : y;
	
	//Get vspringdomObject for the pane to the left
	vspringdomObject* springObj = SpingDomObjFromItem(sd, _right); 
	vspringdomConnection *tempConnection;
	if( springObj )
	{
		tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, topHeight - amountToMove);
		_right->m_Ratio = float(topHeight - amountToMove)/float(nTotal);
	}
	int nHeight;
	springObj = SpingDomObjFromItem(sd, _left);
	if( springObj )
	{
		nHeight = _left->GetHeight();
		tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
		sd->SetConnectionFixedBase (tempConnection, nHeight + amountToMove);
		_left->m_Ratio = float(nHeight + amountToMove)/float(nTotal);
	}
	
	springObj = (vspringdomObject *)sd->GetTopObject ();
	int count = c->GetItemCount ();
	
	for (int idx = 0; springObj && (idx < count);
    idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
	{
		vdialogItem* itm = c->GetItemAt (idx);
		
		Pane *pane  = Pane::CastDown( itm );
		//Only if dialog item is a Pane
		if( pane && pane != _left && pane !=_right )
		{
			nHeight = pane->GetHeight();
			tempConnection = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
			sd->SetConnectionFixedBase (tempConnection, nHeight);
			pane->m_Ratio = float(nHeight)/float(nTotal);
		}
	}
}

void gdSash::MoveToX (int x) 
{
	
	// Get the container, the solution, and the spring domain.
	// See if we moving left or right
	int oldX = m_nOldPos;
	int movingLeft = 0;
	if (x < oldX) movingLeft = 1;
	vdialogItemList* dil = GetItemListIn ();
	if ( dil )
	{
		springContainer *c = springContainer::CastDown (dil->GetOwner());
		if ( c )
		{
			if( c->GetContainerType() == CONT_UNDEFINED )
				c->InitWithType( CONT_HORIZONTAL );
			int nWidth = c->GetWidth();
			vspringSolution *sol = c->GetSolution ();
			if (sol)
			{
				vspringdom* sd = vspringdom::Decompile (sol);
				if( sd )
				{
					if( !(_left && _right) )
					{
						vdomainObjectSet *objSet;
						vspringdomObject *springObj = SpingDomObjFromItem(sd, this);
						if( springObj )
						{
							objSet = sd->GetObjectsConnectedToEdge( springObj, vspringdomLEFT );
							if( objSet )
							{
								vdomainObjectSetIterator iter;
								iter.Start( objSet );
								while( iter.Next() && !_left )
								{
									vspringdomObject *obj = (vspringdomObject *)iter.GetObject();
									_left = Pane::CastDown(ItemFromSpingDomObj( c, sd, obj ));
								}
								iter.Finish();
								vdomainDestroyObjectSet( objSet );
							}
							objSet = sd->GetObjectsConnectedToEdge( springObj, vspringdomRIGHT );
							if( objSet )
							{
								vdomainObjectSetIterator iter;
								iter.Start( objSet );
								while( iter.Next() && !_right )
								{
									vspringdomObject *obj = (vspringdomObject *)iter.GetObject();
									_right = Pane::CastDown(ItemFromSpingDomObj( c, sd, obj ));
								}
								iter.Finish();
								vdomainDestroyObjectSet( objSet );
							}
						}
					}
					if (_left && _right)
					{
						if( movingLeft )
						{
							_left->GetMinSize (&_minWidth, &_minHeight);
							MoveLeft (c, sd, oldX - x, nWidth - c->GetFixedSize());
						}
						else
						{
							_right->GetMinSize (&_minWidth, &_minHeight);
							MoveRight (c, sd, x - oldX, nWidth - c->GetFixedSize());
						}
						// Recalculate the solution and set it to the container.
						sd->Recalculate();
						c->SetSolution (sd->Compile());
					}
					delete sd;
				}
			}
		}
	}
}

void gdSash::DumpIt (vspringdom *sd) {
	// Print out the connections and the positions of the sashes.
	
    for (gdSash *p = _prev; p; p = p->_prev) {
        printf ("\nObject %d\n", p);
        printf ("  The object has bounds x %d, y %d, width %d, height %d\n", p->GetX(), p->GetY(), p->GetWidth(), p->GetHeight());
        printf ("  The Left Edge\n");
        vspringdomConnection *tempConnection = sd->GetObjectConnection (p->thisObj, vspringdomLEFT_OUTSIDE);
        int edge = sd->GetEdgeConnectedTo (tempConnection);
        printf ("  Connected to edge %d\n", edge);
        vdomainObject *connectedTo = sd->GetObjectConnectedTo (tempConnection);
        printf ("  Connected to object %d\n", connectedTo);
        int tempBase = sd->GetConnectionFixedBase (tempConnection);
        printf ("  The base is set to %d\n", tempBase);
        printf ("  The Right Edge\n");
        tempConnection = sd->GetObjectConnection (p->thisObj, vspringdomRIGHT_OUTSIDE);
        edge = sd->GetEdgeConnectedTo (tempConnection);
        printf ("  Connected to edge %d\n", edge);
        connectedTo = sd->GetObjectConnectedTo (tempConnection);
        printf ("  Connected to object %d\n", connectedTo);
        tempBase = sd->GetConnectionFixedBase (tempConnection);
        printf ("  The base is set to %d\n", tempBase);
    }
	
    for (gdSash *n = this; n; n = n->_next) {
        printf ("Object %d\n", n);
        printf ("  The object has bounds x %d, y %d, width %d, height %d\n", n->GetX(), n->GetY(), n->GetWidth(), n->GetHeight());
        printf ("  The Left Edge\n");
        vspringdomConnection *tempConnection = sd->GetObjectConnection (n->thisObj, vspringdomLEFT_OUTSIDE);
        int edge = sd->GetEdgeConnectedTo (tempConnection);
        printf ("  Connected to edge %d\n", edge);
        vdomainObject *connectedTo = sd->GetObjectConnectedTo (tempConnection);
        printf ("  Connected to object %d\n", connectedTo);
        int tempBase = sd->GetConnectionFixedBase (tempConnection);
        printf ("  The base is set to %d\n", tempBase);
        printf ("  The Right Edge\n");
        tempConnection = sd->GetObjectConnection (n->thisObj, vspringdomRIGHT_OUTSIDE);
        edge = sd->GetEdgeConnectedTo (tempConnection);
        printf ("  Connected to edge %d\n", edge);
        connectedTo = sd->GetObjectConnectedTo (tempConnection);
        printf ("  Connected to object %d\n", connectedTo);
        tempBase = sd->GetConnectionFixedBase (tempConnection);
        printf ("  The base is set to %d\n", tempBase);
    }
}


void gdSash::MoveToY (int y) 
{
	
	// Get the container, the solution, and the spring domain.
	// See if we moving up or down
	int oldY = m_nOldPos;
	int movingDown = 0;
	if (y< oldY)
		movingDown = 1;
	vdialogItemList* dil = GetItemListIn ();
	if ( dil )
	{
		springContainer *c = springContainer::CastDown (dil->GetOwner());
		if ( c )
		{
			if( c->GetContainerType() == CONT_UNDEFINED )
				c->InitWithType( CONT_VERTICAL );
			int nHeight = c->GetHeight();
			vspringSolution *sol = c->GetSolution ();
			if (sol)
			{
				vspringdom* sd = vspringdom::Decompile (sol);
				if( sd )
				{
					if( !(_left && _right) ) // find adjacent panes if they are undefined
						// _left will contain pane below the sash, _right - above it
					{
						vdomainObjectSet *objSet;
						vspringdomObject *springObj = SpingDomObjFromItem(sd, this);
						if( springObj )
						{
							objSet = sd->GetObjectsConnectedToEdge( springObj, vspringdomBOTTOM );
							if( objSet )
							{
								vdomainObjectSetIterator iter;
								iter.Start( objSet );
								while( iter.Next() && !_left )
								{
									vspringdomObject *obj = (vspringdomObject *)iter.GetObject();
									_left = Pane::CastDown(ItemFromSpingDomObj( c, sd, obj ));
								}
								iter.Finish();
								vdomainDestroyObjectSet( objSet );
							}
							objSet = sd->GetObjectsConnectedToEdge( springObj, vspringdomTOP );
							if( objSet )
							{
								vdomainObjectSetIterator iter;
								iter.Start( objSet );
								while( iter.Next() && !_right )
								{
									vspringdomObject *obj = (vspringdomObject *)iter.GetObject();
									_right = Pane::CastDown(ItemFromSpingDomObj( c, sd, obj ));
								}
								iter.Finish();
								vdomainDestroyObjectSet( objSet );
							}
						}
					}
					if (_left && _right)
					{
						if( movingDown )
						{
							_left->GetMinSize (&_minWidth, &_minHeight);
							MoveDown (c, sd, oldY - y, nHeight - c->GetFixedSize());
						}
						else
						{
							_right->GetMinSize (&_minWidth, &_minHeight);
							MoveUp (c, sd, y - oldY, nHeight - c->GetFixedSize());
						}
						// Recalculate the solution and set it to the container.
						sd->Recalculate();
						c->SetSolution (sd->Compile());
					}
					delete sd;
				}
			}
		}
	}
}

void gdSashDrag::SetSash (gdSash* sash, vevent* event)
{
    wid = sash;
    const vrect* r = sash->GetRect();
    horizontal = r->GetHeight() > r->GetWidth();
    vdialogItemList* il = wid->GetItemListIn();
    il->TransformPoint (r->GetX(), r->GetY(), &x_, &y_);
    w_ = NULL;
    w_ok_ = 0;
}

void gdSashDrag::ObserveDrag(vwindow *origin, vwindow *to, vevent *event) 
{
    gInit (gdSashDrag::ObserveDrag);
    int x, y;
	
    if (to == origin) {
        int maxWidth = GetMaxWidth();
        if( horizontal )
		{
			vwindow::GetRoot()->GetPointer( &x, &y );
			wid->MoveToX (x);
		}
		else
		{
			vwindow::GetRoot()->GetPointer( &x, &y );
			wid->MoveToY (y);
		}
    }
	
    DeleteLater(); 
    vdrag::ObserveDrag (origin, to, event);
}

int gdSashDrag::HandleDetail (vwindow* src, vwindow* dst, vevent* event)
{
    if (!w_) w_ = src;
    w_ok_ = (w_ == dst);
	
    vdialogItemList* il = wid->GetItemListIn();
    //if (il) il->TransformEvent(event);
    if (horizontal) {
        SetOffset(wid->GetWidth()/2, (event->GetY() - y_));
		
    } else {
        SetOffset((event->GetX() - x_), wid->GetHeight()/2);
    }
	
	if (w_ok_)
		return vdrag::HandleDetail (src, dst, event);
	else return 0;
}

void gdSashDrag::Draw(vrect *r) 
{
    gInit (SashDragVert::Draw);
	
    if (!w_ok_) return;
	
    vdraw::GSave();
	
    vdraw::SetLineWidth(1);
	
    vdraw::SetColor(wid->DetermineColor(vdrawCOLOR_FOREGROUND));
    if (horizontal) {
		vdraw::MoveTo (r->GetX(), r->GetY());
		vdraw::LineTo (r->GetX(), r->GetY()+r->GetHeight());
		vdraw::Stroke();
		
		vdraw::MoveTo (r->GetX()+4, r->GetY());
   	    vdraw::LineTo (r->GetX()+4, r->GetY()+r->GetHeight());
		vdraw::Stroke();  
    } else {
		vdraw::MoveTo (r->GetX(), r->GetY()+1);
		vdraw::LineTo (r->GetX()+r->GetWidth(), r->GetY()+1);
		vdraw::Stroke();
		
		vdraw::MoveTo (r->GetX(), r->GetY()+5);
   	    vdraw::LineTo (r->GetX()+r->GetWidth(), r->GetY()+5);
		vdraw::Stroke();  
		
    }
	
    vdraw::GRestore(); 
}

/************* gdOptionMenu **********************/


void gdOptionMenu::Open()
{
    vcontrolOptionMenu::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    SetValue(0);
}



void gdOptionMenu::SetValue(int newValue)
{
    vcontrolOptionMenu::SetValue(newValue);
    vmenu* myMenu = GetMenu();
    vmenuItem* myItem = myMenu->GetItemAt(newValue);
    if (!insideUpdate_)
		if (dialog_) dialog_->SetInterpVariable((vchar*)GetTag(), 
		(vchar*)myItem->GetTag());
    InitgdWidget((vchar*)GetTag(), this);
}


void gdOptionMenu::UpdateDisplay()
{
    if (dialog_) {
        vchar* newValue = dialog_->GetInterpVariable((vchar*)GetTag());
        vmenu* myMenu = GetMenu();
		
        int x = 0;
        vmenuItem* nextItem = myMenu->GetItemAt(x);
        while (vcharCompare((vchar*)nextItem->GetTag(), newValue) != 0)  {
			x++;
			if (x > myMenu->GetItemCount())
				printf("Error!  Tagname %s has not been found.\n", newValue);
			nextItem = myMenu->GetItemAt(x);
        }
		
        SetValue(myMenu->GetItemIndex(nextItem));
    }
}


/************* gdListItem **********************/

// Open the list item.
void gdListItem::Open()
{
    // First, allow selections.
    allowSelection_ = vTRUE;
	
    vlistitem::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}


void gdListItem::Init (void)
{
    gInit (gdListItem::Init);
    vlistitem::Init ();
	SetAutowidth (vFALSE);
    colwidth_       = 0;
}


void gdListItem::LoadInit (vresource res)
{
    gInit (gdListItem::LoadInit);
    vlistitem::LoadInit (res);
	SetAutowidth (vFALSE);
    colwidth_       = 0;

}

void gdListItem::SetRect(const vrect *myrect)
{
    vlistitem::SetRect(myrect);
    colwidth_ = 0;
}


// Find out if the list item can handle selecitons.
vbool gdListItem::AllowSelection()
{
    int selectionType = GetSelectMethod();
    return (allowSelection_ && (selectionType != vlistNO_SELECTION) );
}

// Allow or prevent the list item from handling selections.
void gdListItem::AllowSelection(vbool state)
{
    vbool oldState = allowSelection_;
    allowSelection_ = state;
    if ((allowSelection_ == vTRUE)  && (oldState == vFALSE) ) {
        // Make sure we handle any changes made while it was off.
        if (GetSelectMethod() != vlistNO_SELECTION)
            ObserveSelect(vlistviewSELECTION_HAS_CHANGED);
    }
}

// Get the string associated with a list item entry.
vchar* gdListItem::GetCellString(int row)
{
    gInit (gdListItem::GetCellString);
    show_interpreter_wait(-1);
    Tcl_Interp* interp = gdWidget::interp(); 
    // Set the row number.
    char buffer[64];
    sprintf (buffer, "%d", row);
    Tcl_SetVar(interp, "Item", (char*)buffer, TCL_GLOBAL_ONLY);
	
    gString cmd;
    cmd.sprintf((vchar*)"global AskListItems;global Item;global %s;lindex $%s 1", GetTag(), GetTag());
    gala_eval(interp, (char*)(vchar*)cmd);
    cmd = (vchar*)Tcl_GetStringResult(interp);
    gala_eval(interp, (char*)(vchar*)cmd);
    show_interpreter_wait(-2);    
    return((vchar*)Tcl_GetStringResult(interp));
}


//-----------------------------------------------------------------------------
void gdListItem::DrawCellContents (int row, int, int, vrect *drawrect)
{
    gInit (gdListItem::DrawCellContents);
    
    gString	txt = GetCellString(row);
    vfont*	fnt = vdraw::GetFont ();
	
    int		x = drawrect->x + 3;
    int		y = drawrect->y + 3;
	
    vdraw::GSave ();
    vdraw::MoveTo (x, y + fnt->Descent());
    vdraw::Show (txt);
    vdraw::GRestore ();
	
    vfont*      font = DetermineFont ();
	
    vchar *val = (vchar *)txt.str();
    int         width = font->StringWidthX (val) + 3;
	
    const vrect *rt = GetListRect();
	
    int listwidth = rt->w - 2;
	
    int larger;
	
    if(width > listwidth)  larger = width;
    else larger = listwidth;
	
    if(larger > colwidth_) {
		SetColumnWidth (vlistviewALL_COLUMNS, larger);
		colwidth_ = larger;
    }
}


void gdListItem::ObserveSelect(int state)
{
    gInit (gdListItem::ObserveSelect);
	
    if ( (state == vlistitemSELECTION_HAS_CHANGED) && AllowSelection() ) {
        show_interpreter_wait (1);
		
		vlistIterator iter;
		vlistSelection* sel = GetSelection();
		iter.StartWithSelectionRange(sel);
		
        vbool  		first = vTRUE;
        int		total = 0;
        gString		str;
		
		while (iter.Next()) {
            int rows = iter.GetRangeRowCount ();
            if (rows > 0) {
                total += rows;
                if (first)
                    first = vFALSE;
                else
                    str += (vchar) ',';
                vrectLong	my_rect;
                gString	segment;
                iter.GetRange (&my_rect);
                if (rows == 1)
                    segment.sprintf_scribed (vcharScribeLiteral("%d"), my_rect.y);
                else
                    segment.sprintf_scribed(vcharScribeLiteral("%d-%d"),
					my_rect.y, my_rect.y + my_rect.h - 1);
                str += (vchar*) segment;
            }
        }
		
        iter.Finish ();
		
        if (total > 0) {
            gString totalNum;
            totalNum.sprintf_scribed (vcharScribeLiteral("%d"), total);
			
            Tcl_Interp* interp = gdWidget::interp(); 
            Tcl_SetVar(interp,  "Selection", (char*)(vchar*)str,
				TCL_GLOBAL_ONLY);
			
            Tcl_SetVar(interp,  "NumberSelected", (char*)(vchar*)totalNum,
				TCL_GLOBAL_ONLY);
			
            // Get and execute the selection cmd.
            gString cmd;
            if (GetLastPointerClickCount() > 1)
                cmd.sprintf((vchar*)"lindex $%s 3", GetTag() );
            else
                cmd.sprintf((vchar*)"lindex $%s 2", GetTag() );
            gala_eval(interp, (char*)(vchar*)cmd);
            cmd = (vchar*)Tcl_GetStringResult(interp);
            gala_eval(interp, (char*)(vchar*)cmd);
        }
		
        show_interpreter_wait (0);
		vlistitem::ObserveSelect(state);
    }
	
	
    return;
}

int gdListItem::HandleKeyDown(vevent* ev)
{
/* need this function because of Galaxy buggy processing of
	   selections made with arrows and Shift+arrows */
	
    vkeyStroke stroke = ev->GetKeyStroke();
	vkey Key = vkeyGetStrokeKey(stroke);
    vkeyModifiers theMod = vkeyGetStrokeModifiers(stroke);
	
	if ((Key == vkeyUP || Key == vkeyDOWN) && 
		(theMod == 0 || theMod == vkeyMODIFIER_KEY_SHIFT))
	{
		int first = -1;
		int last = -1;
		
		vlistSelection* sel = GetSelection ();
		vlistIterator iter;
		iter.StartWithSelectionRange (sel);
		
		//find the first and last selected items
		while (iter.Next()) 
		{
			vrectLong	my_rect;
			iter.GetRange (&my_rect);
			
			if (my_rect.y < first || first == -1)
				first = my_rect.y;
			if (my_rect.y + my_rect.h - 1 > last)
				last = my_rect.y + my_rect.h - 1;
		}
		iter.Finish ();
		
		int selitem = -1;
		
		//if Up arrow was pressed, select item before first,
		//otherwise, select item after last
		
		if (Key == vkeyUP && first >= 0)
			selitem = first - 1;
		else if (last >= 0)
			selitem = last + 1;
		
		if (selitem >= 0 && selitem < GetItemCount())
		{
			//if Shift key was not pressed, delete previous selection
			if (theMod == 0) 
				GetList()->UnselectAll(sel);
			
			SelectItem(selitem); //add new item to selection
			SetFocus(selitem, 0);
			
			//make sure that selected item is visible
			int bottomrow = GetBottomRow();
			int toprow = GetTopRow();
			if (selitem >= bottomrow)
				Scroll(1, 0);
			else if (selitem <= toprow)
				Scroll(-1, 0);
		}
		
		return vTRUE;
	}
	
	return vlistitem::HandleKeyDown (ev);
}

/*****************gdIconListItem********************/

// protected
const int gdIconListItem::icon_x = 16;
const int gdIconListItem::icon_y = 16;
const int gdIconListItem::icon_space = 3;
int gdIconListItem::nRefCount =0;
vdict * gdIconListItem::iconDict =NULL;
void gdIconListItem::doInit()
{
	if (nRefCount==0)
	{
		iconDict=new vdict(vdictHashGeneric);
		LoadIcons(iconDict);
		nRefCount++;
	}
	else
		nRefCount ++;
}

void gdIconListItem::Init()
{
	gInit(gdIconListItem::Init);
	gdListItem::Init();
	doInit();
}

void gdIconListItem::LoadInit(vresource res)
{
	gInit(gdIconListItem::LoadInit);
	gdListItem::LoadInit(res);
	doInit();
}

//protected static
//returns the number of icons loaded.
int gdIconListItem::LoadIcons(vdict *dict)
{
	int nRet=-1;
	if (dict)
	{
		nRet=0;
		vresource dictRes;
		if (getResource("gdIconListIcons", &dictRes))
		{
			vexWITH_HANDLING {
				
				//have to do something about counting arguments.
				size_t nCount= vresourceCountComponents(dictRes);
				size_t i;
				for (i=0; i<nCount; i++)
				{
					
					const vname * iconName= vnameInternGlobal(vresourceGetNthTag(dictRes, i));
					vresource iconRes=vresourceGet(dictRes, iconName);
					//make new image
					
					vimage * image= new vimage(iconRes);
					dict->SetElem(iconName,image);
					nRet++;
				}
				
				

			}
			vexON_EXCEPTION {}
			vexEND_HANDLING;

		}
	}
	return nRet;
}

void gdIconListItem::Destroy()
{
	nRefCount--;
	if (nRefCount==0 && iconDict)
	{
		vdictIterator iter;
		iter.Start(iconDict);
		while(iter.Next())
		{

			vimage * curimg=(vimage *) iter.GetValue();
			delete curimg;
		}
		
		iconDict->Destroy();
		delete iconDict;
		iconDict=NULL;
	}
}

vimage * gdIconListItem::GetCellIcon(vchar *name)
{
	gInit (gdIconListItem::GetCellIcon);
	vimage * pRet=NULL;
	if (iconDict &&name)
	{
		const vname * myName;
		myName=vnameGetGlobal(name);
		if (!myName||!strcmp((char *)myName, ""))
		{
			myName=vnameInternGlobalLiteral("default");
		}
		if (myName)
		{

		vexWITH_HANDLING {

			pRet=(vimage*)iconDict->GetElem(myName);
		}
		vexON_EXCEPTION{
			myName=vnameInternGlobalLiteral("default");
			pRet=(vimage*) iconDict->GetElem(myName);
		}
		vexEND_HANDLING
		}
	}
	return pRet;
}








vchar * gdIconListItem::GetCellIconName(int row)
{
	gInit (gdIconListItem::GetCellIconName);
    //show_interpreter_wait(-1);
    Tcl_Interp* interp = gdWidget::interp(); 
    // Set the row number --need this because we can't guarantee that getcellstring gets
	// called first (or people might get sloppy)
    char buffer[64];
    sprintf (buffer, "%d", row);
    Tcl_SetVar(interp, "Item", (char*)buffer, TCL_GLOBAL_ONLY);
	
    gString cmd;
    cmd.sprintf((vchar*)"global Item;global %s;lindex $%s 4", GetTag(), GetTag());
    gala_eval(interp, (char*)(vchar*)cmd);
    cmd = (vchar*)Tcl_GetStringResult(interp);
    gala_eval(interp, (char*)(vchar*)cmd);
    //show_interpreter_wait(-2);    
    return((vchar*)Tcl_GetStringResult(interp));
}

void gdIconListItem::DetermineCellSize(int row, int column, int * width, int * height)
{
	gInit (gdIconListItem::DetermineCellSize);
	gdListItem::DetermineCellSize( row, column,  width, height);
	*width=*width+icon_x+icon_space;
	if (*height<icon_y)
		*height=icon_y;
	return;
}


//-----------------------------------------------------------------------------
void gdIconListItem::DrawCellContents (int row, int, int, vrect *drawrect)
{
    gInit (gdIconListItem::DrawCellContents);
    
    gString	txt = GetCellString(row);
    vfont*      font = DetermineFont ();
	vimage * image = GetCellIcon(GetCellIconName(row));
	const int inset=3;
    int		x = drawrect->x + inset;
    int		y = drawrect->y;
	
    vdraw::GSave ();
	if (image)
	{
		vdraw::MoveTo(x,y);
		vdraw::ImageCompositeIdent(image);
	}

	vdraw::MoveTo(x+icon_x+icon_space+1,y+font->Descent()+inset);
	vdraw::Show (txt);
    vdraw::GRestore ();
	
    
    vchar *val = (vchar *)txt.str();
    int         width = font->StringWidthX (val) +inset+icon_x;
	


    const vrect *rt = GetListRect();
	
    int listwidth = rt->w;
	
    int larger;
	
    if(width > listwidth)  larger = width;
    else larger = listwidth;
	
    if(larger > colwidth_) {
		SetColumnWidth (vlistviewALL_COLUMNS, larger);
		colwidth_ = larger;
    }
}

//this should set the row height properly...
void gdIconListItem::SetRowHeight(int row, int height)
{

	gInit(gdIconListItem::SetRowHeight);

	//call the parent
	int nFontHeight= DetermineFont()->Height();
	if (nFontHeight>height)
		height=nFontHeight;

	if (height<icon_y)
		height=icon_y;
	gdListItem::SetRowHeight(row, height);
}




/*************** gdListCheck********************/

//---------
gdListCheck::gdListCheck(gdListView* p) 
{
    static char *on ="SpreadSheetImages:CheckOnImage";
    static char *off ="SpreadSheetImages:CheckOffImage";
	
    parentWindow=p;
    x_shift=1;
    y_shift=1;
    indicator.x=indicator.y=indicator.w=indicator.h=0;
    title="";
    alternate="";
    onImagePath  = on;
    offImagePath = off;
}

gdListCheck::~gdListCheck() 
{
}

// main drawing routine - must be called from the parent control drawing
// routine after the parent drawing. 
void gdListCheck::Draw(void) {
    vrect visRect;
    visRect.Intersect(&clipRect,&windowRect);
    if(!visRect.IsEmpty()) ParentDraw();
}


// parent window drawing function. If you use the correct resource
// paths, the bitmaps from the resources will be displayed.
// if the resources paths is incorrect, red or green rectangle will 
// be displayed.
// the resource path must be in a form tag/tag/tag....
void gdListCheck::ParentDraw(void) {
    vdraw::GSave ();
    parentWindow->DrawDevice();
    vdraw::RectClip(parentWindow->GetListRect()->x,parentWindow->GetListRect()->y,parentWindow->GetListRect()->w,parentWindow->GetListRect()->h);
    vdraw::RectClip(windowRect.x,windowRect.y,windowRect.w,windowRect.h);
    vdraw::SetColor(vcolor::GetBlack());
	
	vimage* imageOn = parentWindow->GetImageFromResource(&onImagePath);
	vimage* imageOff= parentWindow->GetImageFromResource(&offImagePath);
    
	if(value==vTRUE) {
		if(imageOn!=NULL) {
			vrect sr;
			sr.Set(0,0,imageOn->GetWidth(),imageOn->GetHeight());
			vdraw::RectImageComposite(imageOn,&sr,&indicator);
		} else {
			vdraw::SetColor(vcolor::GetBlack());
			vdraw::RectFill(indicator.x+1,indicator.y+1,indicator.w-2,indicator.h-2);
		}
		if(title.length()!=0) {
			vdraw::MoveTo(3+indicator.x+indicator.w+x_shift,
				indicator.y+1);
			vdraw::SetColor(vcolor::GetBlack());
			vdraw::Show(title);
		}
    } else {
		if(imageOff!=NULL) {
			vrect sr;
			sr.Set(0,0,imageOff->GetWidth(),imageOff->GetHeight());
			vdraw::RectImageComposite(imageOff,&sr,&indicator);
		} else {
			vdraw::SetColor(vcolor::GetWhite());
			vdraw::RectFill(indicator.x+1,indicator.y+1,indicator.w-2,indicator.h-2);
		}
		if(title.length()!=0) {
			vdraw::MoveTo(3+indicator.x+indicator.w+x_shift,
				indicator.y+1);
			vdraw::SetColor(vcolor::GetBlack());
			if(alternate.length()!=0)   vdraw::Show(alternate);
			else                vdraw::Show(title);
		}
		
    }
    vdraw::GRestore();
}


// mouse button press handling, switching the checkbox to 
// the opposite state
int gdListCheck::HandleButtonDown(vevent* event) {
	if(callback_str.length()==0) return vFALSE;
	if(indicator.ContainsPoint(event->GetX(),event->GetY())) {
		if(value==vTRUE) value=vFALSE;
		else             value=vTRUE;
		parentWindow->InvalRect(&windowRect,vwindowINVAL_UPDATE);
	}
	return vTRUE;
}

// nothing, just perform vwindow-like function
int gdListCheck::HandleButtonUp(vevent *event) {
	return vFALSE;
}


// checkbox will switch to the opposite state in the case "Confirm"
// button pressed.
int gdListCheck::HandleKeyDown(vevent *event) {
    if(callback_str.length()==0) return vFALSE;
	if((event->binding && strcmp((char *)event->binding, "Confirm") == 0)) {
		if(value==vTRUE) value=vFALSE;
		else             value=vTRUE;
		parentWindow->InvalRect(&windowRect,vwindowINVAL_UPDATE);
		return vTRUE;
	}
	return vFALSE;
}

// nothing, just perform vwindow-like function
int gdListCheck::HandleKeyUp(vevent *event) {
	return vFALSE;
}


// returns indicator rectangle in the base window coordinates
vrect* gdListCheck::GetRect(void) {
	return &windowRect;
}

// determines the complete checkbox rectangle. If the rectangle is
// too small, minimum indicator size will be used (5*5) dots.
// in every other case indicator will be positioned at the left
// of the checkbox rectangle with X and Y offsets determined by
// IndicatorOffsets function.
void gdListCheck::SetRect(vrect* newRect) {
	windowRect=*newRect;
	indicator.x=windowRect.x+x_shift;
	indicator.y=windowRect.y+y_shift;
	
	if(indicator.x>windowRect.x+windowRect.w)
		indicator.x=windowRect.x+windowRect.w;
	
	if(indicator.y>windowRect.y+windowRect.h)
		indicator.y=windowRect.y+windowRect.h;
	
	int d=windowRect.h-2*y_shift;
	if(d<5) d=5;
	indicator.w=indicator.h=d;
	
}

// if indicator is completely outside the clip rectangle there
//  will be no drawing, intersection will be drawn without any
// clipping
void gdListCheck::SetClipRect(const vrect* newRect) {
	clipRect=*newRect;
}


// returns vTRUE or vFALSE corresponding to the checkbox status
int  gdListCheck::GetValue() {
	return value;
}


// value can be vTRUE to make indicator switch to ON position
// and vFALSE to switch to OFF position.
// only vTRUE and vFALSE values are permitted, other values will 
// be ignored.
void gdListCheck::SetValue(int v) {
	if(value!=v) {
		if(v==vTRUE) value=vTRUE;
		if(v==vFALSE) value=vFALSE;
	}
}


void gdListCheck::SetValueAndRedraw(int v) {
	if(value!=v) {
		if(v==vTRUE) value=vTRUE;
		if(v==vFALSE) value=vFALSE;
		parentWindow->InvalRect(&windowRect,vwindowINVAL_UPDATE);
	}
}

// title will be shown at the right side of the indicator rectange
// with the offset determined in IndicatorOffset function
void gdListCheck::SetTitle(const vchar* s) {
	title=s;
}

void gdListCheck::SetAlternateTitle(const vchar* s) {
	alternate = s;
}

// returns the indicator rectangle in base window coordinates.
// (not in checkbox coordinates)
vrect*   gdListCheck::GetIndicator(void) {
	return &indicator;
}

// this function will set indicator bounds in dots.
// the indicator will be placed at the left side of the
// checkbox rectangle with the up and down offsets = yOffset
// and left offset and distance to the text title = xOffset.
// default is 1,1.
void gdListCheck::SetIndicatorOffsets(int xOffset,int yOffset) {
	x_shift=xOffset-1;
	if(x_shift<0) x_shift=0;
	y_shift=yOffset+1;
	if(y_shift<0) y_shift=0;
	indicator.x=windowRect.x+x_shift;
	indicator.y=windowRect.y+y_shift;
	if(indicator.x>windowRect.x+windowRect.w)
		indicator.x=windowRect.x+windowRect.w;
	if(indicator.y>windowRect.y+windowRect.h)
		indicator.y=windowRect.y+windowRect.h;
	
	int d=windowRect.h-2*y_shift;
	if(d<5) d=5;
	indicator.w=indicator.h=d;
}

void gdListCheck::SetImageResources(char* on, char* off) {
	onImagePath = on;
	offImagePath= off;
}

char* gdListCheck::GetCallback(void) {
	return callback_str;
}
void gdListCheck::SetCallback(char* name) {
	callback_str=name;
}

//A number spinner. Don't stop event processing in hte TCL callback, as an infinity of button select messages
// seem to hit the spinner...


void gdNumberSpinner::Open()
{
	vspinner::Open();
	//this stupid hack may solve the annoying bug of the initialization changing the min/max values not succeeding...
	int x=GetValue();
	SetValue(x+1);
	SetValue(x);
	MarkContentDirty();
	dialog_ = GDialog::CastDown(DetermineDialog());
	InitgdWidget((vchar*)GetTag(), this);
	UpdateDisplay();
}


void gdNumberSpinner::ObserveDialogItem(vevent* event)
{
    vspinner::ObserveDialogItem(event);
    if ((!insideUpdate_) && (dialog_ || pane_)) 
	{
		
		vchar buffer[LOCAL_STRING_SIZE];
		sprintf((char*)buffer,"%d",GetValue());
		
		SetInterpVariable((vchar *)GetTag(), buffer);
    }

}

void gdNumberSpinner::SetMax(int x)
{
	vspinner::SetMax(x);
	unsigned int nCur=GetValue();
	if (nCur>(unsigned int) x)
	{
		SetValue(x);
	}
}
void gdNumberSpinner::SetMin(int x)
{
	vspinner::SetMin(x);
	unsigned int nCur=GetValue();
	if (nCur< (unsigned int) x)
		SetValue(x);
}
void gdNumberSpinner::SetValue(int x)
{
	vspinner::SetValue(x);
 if ((!insideUpdate_) && (dialog_ || pane_)) 
	{
	vchar buffer[LOCAL_STRING_SIZE];
	sprintf((char *) buffer, "%d", x);
	SetInterpVariable((vchar *) GetTag(), buffer);
	MarkContentDirty();
	}
}

void gdNumberSpinner::UpdateDisplay()
{
    if (dialog_ || pane_)  
	{
		vchar* newValue = GetInterpVariable((vchar*)GetTag());
		int resultValue;
		if (newValue) 
		{
			resultValue = atoi((const char*)newValue);
			SetValue(resultValue);
			MarkContentDirty();
		}
    }
}

// ******************** gdTextSpinner *************************
// This class was designed to represent Text Spinner in C++.
// The main idea is to embagge TCL callback to notify TCL every time selection changed.

// PUBLIC :

//------------------------------------------------------------------------------------
// Re-definition of Open. We need this to initialize gdWidget and for future
// extentions - to perform some operations while opening this window.
//------------------------------------------------------------------------------------
void gdTextSpinner::Open() {
	vspinnerText::Open();
	dialog_ = GDialog::CastDown(DetermineDialog());
	InitgdWidget((vchar*)GetTag(), this);
	
	char text[201];
	GetText((unsigned char *)text,200);
	m_CurrentText  = text;
	m_CurrentIndex = FindTextIndex((unsigned char *)text);
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// We need this function to catch GALAXY notifications and call OnSelectionChanged
// function to pass new selection to the TCL.
//------------------------------------------------------------------------------------
void gdTextSpinner::ObserveDialogItem(vevent *event) {
	char text[201];
	int  index;
	
	GetText((unsigned char *)text,200);
	index=FindTextIndex((unsigned char *)text);
	if(m_ChangeCallback.length()!=0)
		OnSelectionChanged(index,(unsigned char *) text);
	m_CurrentText=text;
	m_CurrentIndex=index;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// We need this function to embagge any string wich will be passed to TCL in
// OnSelectionChanged to notify new spin selection.
//------------------------------------------------------------------------------------
void gdTextSpinner::SetChangeCallback(char *cb) {
	m_ChangeCallback=cb;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function returns the TCL callback function.
//------------------------------------------------------------------------------------
char* gdTextSpinner::GetChangeCallback(void) {
	return (char *)m_ChangeCallback;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function returns current text spinner index
//------------------------------------------------------------------------------------
int gdTextSpinner::GetCurrentIndex(void) {
	return m_CurrentIndex;
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// This function returns the currently selected text string
//------------------------------------------------------------------------------------
char*  gdTextSpinner::GetCurrentText(void) {
	return (char *) m_CurrentText;
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// This function cleans all text spin list
//------------------------------------------------------------------------------------
void gdTextSpinner::Clean(void) {
	vliststr* list;
	
	list=GetListstr();
	list->RemoveRange(0,list->GetRowCount(),vlistROW);
}
//------------------------------------------------------------------------------------


// PROTECTED :

//------------------------------------------------------------------------------------
// Re-definition of Init. We need this to set empty string in TCL callback vString
// and for future extentions - to initialize all data.
//------------------------------------------------------------------------------------
void gdTextSpinner::Init() {
	vspinnerText::Init();
	CommonInit();
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Re-definition of Init. We need this to set empty string in TCL callback vString
// and for future extentions - to initialize all data.
//------------------------------------------------------------------------------------
void gdTextSpinner::LoadInit(vresource resource) {
	vspinnerText::LoadInit(resource);
	CommonInit();
}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Here all work common to Init and LoadInit must be done.
//------------------------------------------------------------------------------------
void gdTextSpinner::CommonInit() {
	// nothing selected.
	m_CurrentIndex = -1;
	m_CurrentText  = "";
	m_ChangeCallback  = "";
	MarkContentDirty();
	
}
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
// Re-definition of Destroy. Here all initialized memory blocks must be free.
// (From my experience this function will never be called, but in future the 
// situation can change.)
//------------------------------------------------------------------------------------
void gdTextSpinner::Destroy() {
}
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// Called every time spin selection changed. m_CurrentIndex & m_CurrentText 
// contain old selections at this moment, will be updated immidiatly after
// this function exit.This version just call TCL string if it is not empty.
//------------------------------------------------------------------------------------
void gdTextSpinner::OnSelectionChanged(int newIndex, vchar* newText) {
	
	gString cmd;
	cmd.sprintf((vchar*)"%s %d {%s} %d {%s}", GetChangeCallback(), 
		newIndex, newText,
		m_CurrentIndex, m_CurrentText);
	gala_eval(interp(), (char*)(vchar*)cmd);
	
}
//------------------------------------------------------------------------------------



//=======================================================================================


/************* gdListView **********************/

void gdListView::Open() {
	vlistview::Open();
	dialog_ = GDialog::CastDown(DetermineDialog());
	InitgdWidget((vchar*)GetTag(), this);
	vlist *list;
	list=GetList();
	if(list==NULL) {
		list=new vlist;
		SetList (list);
		list->SetSelectMethod (GetSelection(), vlistONE_ONLY_SELECTION);
		list->SetExtendRowSelection (GetSelection(), TRUE);
	}
}

//-----------------------------------------------------------------------------

void gdListView::LoadInit(vresource resource)
{
	vlistview::LoadInit(resource);
	CommonInit();
}

void gdListView::Init()
{
	// initialize inherited data
	vlistview::Init();
	CommonInit();
}

// Initializing local things not performed by vlistview
void gdListView::CommonInit()
{
	internal_list   = NULL;
	change_callback = NULL;                    // sell changing callback
	click_callback  = NULL;                    // mouse clicking callback
	editing         = vFALSE;                  // edit control is not opened on the sell
	
	// this is sell editor
	textitem        = new ListViewTextItem;
	textitem->SetParentView(this);
	
	// will not be scrolled left-right automatically
	textitem->SetAutoScroll(vFALSE);
	// only one line of text will be performed
	textitem->SetOneLine(vTRUE);
	// empty list of dialog elements
	itemlist = new ListViewItemList;
	// this class will be it's owner
	itemlist->SetOwner(this);
	// sell editor will be in itemlist container
	key_pressed=0;
	_ruler=NULL;
	
}

void gdListView::Destroy() {
	ListViewCell* cell;
	printf("Destroy called \n");
	if(textitem!=NULL) {
		delete textitem;
		textitem=NULL;
	}
	if(itemlist!=NULL) {
		delete itemlist;
	}
	vlist* list=GetList();
	if(list!=NULL) {
		for(register i=0;i<list->GetColumnCount();i++) {
			for( register j=0;j<list->GetRowCount();j++) {
				vexWITH_HANDLING {
					cell = (ListViewCell *)list->GetCellValue(j,i);
				}
				vexON_EXCEPTION {
					if (vexGetArgException()) {    // Name not found in Dict.
						vexClear();
						cell = NULL;
					} else    // Handle the expection.
						vexPropagate(vexGetRootClass(), vexNULL);
				} vexEND_HANDLING;
				if(cell!=NULL) {
					if(cell->value != NULL)      free(cell->value);
					if(cell->data  != NULL)      free(cell->data);
					if(cell->check_button!=NULL) delete cell->check_button;
					//delete cell;
				}
			}
		}
	}
#if 0
	if (textitem)
		delete textitem;
	vlist *list = GetList();
	if(list)
		delete list;
	vlistview::Destroy();
#endif
}

void gdListView::Draw() {
	ListViewCell* cell=NULL;
	
	// Draw the listview first!
	if(editing && key_pressed) {
		textitem->Draw();
		key_pressed=0;
		return;
	}
	vlistview::Draw();
	vlist* list=GetList();
	if(list==NULL) return;
	for(register i=0;i<list->GetColumnCount();i++) {
		for( register j=0;j<list->GetRowCount();j++) {
			vexWITH_HANDLING {
				cell = (ListViewCell *)list->GetCellValue(j,i);
			}
			vexON_EXCEPTION {
				if (vexGetArgException()) {    // Name not found in Dict.
					vexClear();
					cell = NULL;
				} else    // Handle the expection.
					vexPropagate(vexGetRootClass(), vexNULL);
			} vexEND_HANDLING;
			if(cell!=NULL && cell->check_button!=NULL) {
				cell->check_button->Draw();
			}
		}
	}
	// and then all visible items from dialog items list
	vdrawGSave();
	Clip();
	itemlist->Draw();
	vdrawGRestore();
}

// set the table position and size
void gdListView::SetRect(const vrect *myrect) {
	ListViewCell* cell=NULL;
	
	if(editing) CloseEditor();
	vlistview::SetRect(myrect);
	vlist* list=GetList();
	if(list==NULL) return;
	if(_ruler!=NULL) {
		vrulerItem* last=_ruler->GetItemAt(_ruler->GetItemCount()-1);
		if(last!=NULL) {
			last->SetWidth(GetListRect()->GetWidth()-last->GetOffset()-list->GetColumnCount());
		}
	}
	for(register i=0;i<list->GetColumnCount();i++) {
		for( register j=0;j<list->GetRowCount();j++) {
			vexWITH_HANDLING {
				cell = (ListViewCell *)list->GetCellValue(j,i);
			}
			vexON_EXCEPTION {
				if (vexGetArgException()) {    // Name not found in Dict.
					vexClear(); 
					cell = NULL;
				} else    // Handle the expection.
					vexPropagate(vexGetRootClass(), vexNULL);
			} vexEND_HANDLING;
			if(cell!=NULL && cell->check_button!=NULL) {
				vrect r;
				const vchar *value;
				GetCellRect(j, i, &r);
				value = (const vchar*)GetCellValue(j,i);
				// set it's position
				if(value && vcharLength(value)!=0)
					r.Inset(&r, 1, 1);
				else {
					r.Inset(&r, 1, 1);
					int d=r.h;
					r.x=r.x+(r.w-d)/2;
					r.w=d;
				}
				cell->check_button->SetClipRect(GetRect());
				cell->check_button->SetRect(&r);
			}
		}
	}
}

// scroll redifinition to add checkbox support
void gdListView::ObserveScroll(int x, int y) {
	ListViewCell* cell=NULL;
	
	if(editing) CloseEditor();
	// vlistview::ObserveScroll(deltaX,deltaY);
	
	vlist* list=GetList();
	if(list==NULL) return;
	for(register i=0;i<list->GetColumnCount();i++) {
		for( register j=0;j<list->GetRowCount();j++) {
			vexWITH_HANDLING {
				cell = (ListViewCell *)list->GetCellValue(j,i);
			}
			vexON_EXCEPTION {
				if (vexGetArgException()) {    // Name not found in Dict.
					vexClear();
					cell = NULL;
				} else    // Handle the expection.
					vexPropagate(vexGetRootClass(), vexNULL);
			} vexEND_HANDLING;
			if(cell!=NULL && cell->check_button!=NULL) {
				vrect r;
				r=*((vrect *)cell->check_button->GetRect());
				r.x+=x;
				r.y+=y;
				cell->check_button->SetRect(&r);
			}
		}
	}
}


void gdListView::SetColumnWidth(int column, int width) {
	ListViewCell* cell=NULL;
	
	if(editing) CloseEditor();
	vlistview::SetColumnWidth(column, width);
	vlist* list=GetList();
	if(list==NULL) return;
	for(register i=0;i<list->GetColumnCount();i++) {
		for( register j=0;j<list->GetRowCount();j++) {
			vexWITH_HANDLING {
				cell = (ListViewCell *)list->GetCellValue(j,i);
			}
			vexON_EXCEPTION {
				if (vexGetArgException()) {    // Name not found in Dict.
					vexClear();
					cell = NULL;
				} else    // Handle the expection.
					vexPropagate(vexGetRootClass(), vexNULL);
			} vexEND_HANDLING;
			if(cell!=NULL && cell->check_button!=NULL) {
				vrect r;
				const vchar *value;
				GetCellRect(j, i, &r);
				value = (const vchar*)GetCellValue(j,i);
				// set it's position
				if(value && vcharLength(value)!=0)
					r.Inset(&r, 1, 1);
				else {
					r.Inset(&r, 1, 1);
					int d=r.h;
					r.x=r.x+(r.w-d)/2;
					r.w=d;
				}
				cell->check_button->SetRect(&r);
			}
		}
	}
}


// drawing specified cell
void gdListView::DrawCellContents (int row, int column, int sel, vrect *drawrect) {
	
	gInit (gdListView::DrawCellContents);
	
	ListViewCell *cell = GetCellInfo(row, column);
	if(cell!=NULL && cell->check_button!=NULL) return;
	if(editing && edit_col==column && edit_row==row) return;
	
	const vchar *txt;
	if(internal_list == NULL) {
		// base processing of the drawcell - specified cell will be drawn 
		// with sel==true in the case the cell is selected and the selected
		// rectangular drawrect
		vlistview::DrawCellContents (row, column, sel, drawrect);
		Tcl_Interp* interp = gdWidget::interp(); 
		//show_interpreter_wait(-1);
		
		// Get the size of the list
		int count = 0;
		
		char buffer[64];
		// TCL variabel Item will be row*columns+column
		sprintf (buffer, "%d", row *columns + column);
		Tcl_SetVar(interp, "Item", (char*)buffer, TCL_GLOBAL_ONLY);
		
		// the following TCL script will be interp
		//   global Item
		//   global <ListTag>
		//   lindex $<ListTag> 1
		// the first item from the list <ListTag>
		gString cmd;
		cmd.sprintf((vchar*)"global Item;global %s;lindex $%s 1", GetTag(), GetTag());
		gala_eval(interp, (char*)(vchar*)cmd);
		// the fist inem from the list <ListTag> will be procecced
		cmd = (vchar*)Tcl_GetStringResult(interp);
		gala_eval(interp, (char*)(vchar*)cmd);
		
		//show_interpreter_wait(-2); 
		txt = (vchar*)Tcl_GetStringResult(interp);
	} else {
		txt = GetCellValue(row, column);
		if(txt == NULL)  txt = (const vchar *)"";
	}
	
	vfont* fnt  = vdraw::GetFont ();
	int	     x  = drawrect->x + 3;
	int	     y  = drawrect->y + 3;
	unsigned ni = 1;
	
	vdraw::GSave ();
	vdraw::MoveTo (x, y + fnt->Descent());
	vdraw::Show (txt);
	vdraw::GRestore ();
}

void gdListView::CreateInternalList() {
	ListViewCell* cell=NULL;
	
	if(editing) CloseEditor();
	// sell editor will be in itemlist container
	CurrentRow=0;
	CurrentColumn=0;
	vlist* list=GetList();
	if(list!=NULL) {
		for(register i=0;i<list->GetColumnCount();i++) {
			for( register j=0;j<list->GetRowCount();j++) {
				vexWITH_HANDLING {
					cell = (ListViewCell *)list->GetCellValue(j,i);
				}
				vexON_EXCEPTION {
					if (vexGetArgException()) {    // Name not found in Dict.
						vexClear();
						cell = NULL;
					} else    // Handle the expection.
						vexPropagate(vexGetRootClass(), vexNULL);
				} vexEND_HANDLING;
				if(cell!=NULL) {
					if(cell->value != NULL)      free(cell->value);
					if(cell->data  != NULL)      free(cell->data);
					if(cell->check_button!=NULL) delete cell->check_button;
					//delete cell;
				}
			}
		}
	}
	internal_list = new vlist;
	SetList(internal_list);
	if (list!=NULL) delete list;
	//list->DeleteLater();
	SetRowHeight(vlistviewALL_ROWS,20);
	internal_list->SetExtendRowSelection (GetSelection(), vFALSE);
	internal_list->SetSelectMethod (GetSelection(), vlistNO_SELECTION);
	internal_list->SetSelectionImmutable(GetSelection(), vFALSE);
	SetDrawingSelection(vFALSE);
}

void gdListView::SetChangeCallback(char *cb)
{
	if(change_callback != NULL)
		free(change_callback);
	change_callback = strdup(cb);
}

void gdListView::SetDblClickCallback(char *cb)
{
	if(click_callback != NULL)
		free(click_callback);
	click_callback = strdup(cb);
}

// defines the cell change callback wich will be called every time
// the current cell changed.
void gdListView::SetCellChangeCallback(char* name) {
	cell_change_callback=name;
}

// returns the cell change callback wich will be called every time
// the current cell changed.
char* gdListView::GetCellChangeCallback(void) {
	return cell_change_callback;
}


// sell popup menu assosiation. sell xy in , callback function address in,
// menu items and it's amount in
void gdListView::SetCellMenu(int row, int col, char *callback, int count, char **items)
{
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL)     return;
	// popup items list cleaning
	cell->popup_items.reset();
	// and filling
	for(int i = 0; i < count; i++){
		PopupMenuItemDescPtr desc = new PopupMenuItemDesc;
		desc->title    = strdup(items[i]);
		desc->callback = strdup(callback); // ?
		cell->popup_items.append(desc); // real menu items filling
	}
}


// embagges the check control to the specified cell and set it's
// initial value. the embagged control vill be removed and memory 
// will be free when dialog item list will be removed
gdListCheck* gdListView::SetCellCheck(int row, int col, char* callback,int state) {
	
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL)     return NULL;
	if(cell->check_button==NULL) {
		cell->read_only=vTRUE; 
		cell->check_button=CreateCheckButton(row,col);
		cell->check_button->SetValue(state);
		cell->check_button->SetCallback(callback);
		return cell->check_button;
	}
	return NULL;
}



// changes the value of the check control embagged into the 
// specified cell. Do nothing if cell is not specified or 
// check control is not embagged into it.
void gdListView::ChangeCellCheck(int row, int col, int value) {
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL) return;
	if(cell->check_button!=NULL)  cell->check_button->SetValue(value);
}

// returns the check value of the cell or -1 if cell is not defined
// or check control is not embagged into cpecified cell.
int gdListView::InspectCellCheck(int row, int col) {
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL) return -1;
	if(cell->check_button!=NULL)  return cell->check_button->GetValue();
	else return -1;
}


// returns the cell element - ListViewCell on the spesific position.
ListViewCell *gdListView::GetCellInfo(int row, int col) {
	ListViewCell *cell = NULL;
	if(!internal_list) return NULL;
	if(internal_list->GetColumnCount() <= col || 
		internal_list->GetRowCount() <= row) return NULL;
	
	vexWITH_HANDLING {
		cell = (ListViewCell *)internal_list->GetCellValue(row, col);
	}
	vexON_EXCEPTION {
		if (vexGetArgException()) {    // Name not found in Dict.
			vexClear();
			cell = NULL;
		} else    // Handle the expection.
			vexPropagate(vexGetRootClass(), vexNULL);
	} vexEND_HANDLING;
	return cell;
}


// set text and data assosiated with the specified cell. 
// Enables or disables cell editing.
void gdListView::SetDataCell(int row, int col, char *value, int read_only, char *mydata) {
	ListViewCell *cell = GetCellInfo(row, col);
	if(!cell){
		cell               = new ListViewCell;
		cell->value        = NULL;
		cell->data         = NULL;
		cell->check_button = NULL;
		internal_list->SetCellValue(row, col, cell);
	}
	if(cell->value != NULL)      free(cell->value);
	if(cell->data  != NULL)      free(cell->data);
	if(cell->check_button!=NULL) {
		delete cell->check_button;
		cell->check_button=NULL;
	}
	cell->value=strdup(value);
	if(mydata != NULL) cell->data = strdup(mydata);
	cell->read_only    = read_only;
	cell->check_button = NULL;
	
}

// removes text and data assosiated with the specified cell. 
// removes the cell from the internal list
void gdListView::RemoveAllRow(int row) {
	register i;
	int mycolumns=internal_list->GetColumnCount();
	
	for(i=0;i<mycolumns;i++) {
		ListViewCell *cell = GetCellInfo(row, i);
		if(cell==NULL) return;
		if(cell->value != NULL) {
			free(cell->value);
			cell->value=NULL;
		}
		if(cell->data  != NULL) {
			free(cell->data);
			cell->data=NULL;
		}
		if(cell->check_button!=NULL) {
			delete cell->check_button;
			cell->check_button=NULL;
		}
	}
	internal_list->RemoveRow(row);
}

// Close editor if any
void gdListView::Flush(void) {
	if(editing) CloseEditor();
}

// returns text string assosiated with the cell
vchar *gdListView::GetCellValue(int row, int col) {
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL)
		return NULL;
	else
		return (vchar *)cell->value;
}


// returns the datapointer assosiated with the cell
vchar *gdListView::GetCellData(int row, int col)
{
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL) return NULL;
	else	         return (vchar *)cell->data;
}

// from the list of items wich was formed in the 
ListViewPopupMenu *gdListView::CreateCellMenu(int row, int col) {
	if(!internal_list) return NULL;
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell == NULL || cell->popup_items.size() == 0) return NULL;
	return BuildPopupMenu(cell->popup_items, row, col);
}

ListViewPopupMenu *gdListView::BuildPopupMenu(gArrayOf (PopupMenuItemDescPtr) &mypopup_items, int r, int c)
{
	ListViewPopupMenu *menu = new ListViewPopupMenu;
	menu->SetListView(this);
	ListViewPopupMenuItem *item;
	for(int i = 0; i < mypopup_items.size(); i++) {
		PopupMenuItemDescPtr desc = mypopup_items[i];
		item                      = new ListViewPopupMenuItem;
		item->SetTitle((vchar *)desc->title);
		menu->AppendItem(item);
	}
	int x, y;
	vwindow *vwd = DetermineWindow();
	menu->SetBackground(vwd->GetBackground());
	vwd->GetPointer(&x, &y);
	vrect rr; //don't call this rect because it generates a warning
	rr.x = x;
	rr.y = y;
	rr.w = 1;
	rr.h = 1;
	menu->SetRowCol(r, c);
	menu->PlaceRight(vwd, &rr);
	return menu;
}

void gdListView::ObserveSelect(int state)
{
	gInit (gdListView::ObserveSelect);
	
	if(internal_list != NULL) {
		vlistview::ObserveSelect(state);
		return;
	}
	show_interpreter_wait (1);
	
	if (state == vlistviewSELECTION_HAS_CHANGED) {
		vbool  first = vTRUE;
		int			total = 0;
		gString			str;
		vlistIterator		iter;
		iter.StartWithSelectionRange (GetSelection());
		while (iter.Next()) {
			int rows = iter.GetRangeRowCount ();
			if (rows < 1)
				continue;
			total += rows;
			if (first)
				first = vFALSE;
			else
				str += (vchar) ',';
			vrectLong	my_rect;
			gString	segment;
			iter.GetRange (&my_rect);
			if (rows == 1)
				segment.sprintf_scribed (vcharScribeLiteral("%d"), my_rect.y);
			else
				segment.sprintf_scribed(vcharScribeLiteral("%d-%d"),
				my_rect.y, my_rect.y + my_rect.h - 1);
			str += (vchar*) segment;
		}
		
		
		
		Tcl_Interp* interp = gdWidget::interp(); 
		Tcl_SetVar(interp,  "Selection", (char*)(vchar*)str, TCL_GLOBAL_ONLY);
		
		
		// Get and execute the selection cmd.
		gString cmd;
		if (GetLastPointerClickCount() > 1)
			cmd.sprintf((vchar*)"lindex $%s 3", GetTag() );
		else
			cmd.sprintf((vchar*)"lindex $%s 2", GetTag() );
		gala_eval(interp, (char*)(vchar*)cmd);
		cmd = (vchar*)Tcl_GetStringResult(interp);
		gala_eval(interp, (char*)(vchar*)cmd);
		
		iter.Finish ();
	}
	
	show_interpreter_wait (0);
	
	vlistview::ObserveSelect(state);
	
	return;
}

void gdListView::Close() {
	ListViewCell* cell=NULL;
	if(editing) CloseEditor();
	vlistview::Close();
}


void gdListView::OnCheck(int row, int col, int new_status) 
{
	char *txt;
	
	ListViewCell *cell = GetCellInfo(row, col);
	if(cell==NULL) return;
	if(cell->check_button==NULL) return;
	if(cell->check_button==NULL) return;
	if(cell->check_button->GetCallback()==NULL) return;
	if(strlen(cell->check_button->GetCallback())==0) return;
	if(new_status == 0) txt = "0";
	else txt = "1";
	gString cmd;
	cmd.sprintf((vchar*)"%s {%s} %d %d", (char *)cell->check_button->GetCallback(), txt, row, col);
	gala_eval(interp(), (char*)(vchar*)cmd);
}

int gdListView::HandleButtonDown(vevent *event) {
	
	
	vpoint        point;
	point.x = event->GetX(); point.y = event->GetY();
	if (!GetRect()->ContainsPoint(point.GetX(), point.GetY())) 
		return vlistview::HandleButtonDown(event);    
	
	
	int r, c;
	MapPointToCell(&point, &r, &c);
	if(r == -1 || c == -1)  return vlistview::HandleButtonDown(event);    
	
	int button = event->GetButton();
	
	if(button == 0) { // right button was pressed...
		
		// if button was pressed on scrollbar controls
		if (HasHorzBar()){
			vcontrol      *scrl;
			scrl = this->hscroll;
			if (scrl && scrl->ContainsPoint(point.x, point.y)) {
				if(editing) CloseEditor();
				return vlistview::HandleButtonDown(event);   
			}
		}
		if (HasVertBar()){
			vcontrol      *scrl;
			scrl = this->vscroll; 
			if (scrl && scrl->ContainsPoint(point.x, point.y)){
				if(editing) CloseEditor();
				return vlistview::HandleButtonDown(event);   
			}
		}
		
		// let TCL now that current cell has changed
		if( CurrentRow!=r || CurrentColumn!= c ) {
			gString cmd;
			cmd.sprintf((vchar*)"%s %d %d %d %d", GetCellChangeCallback(), r, c,CurrentRow,CurrentColumn);
			gala_eval(interp(), (char*)(vchar*)cmd);
			CurrentRow    = r;
			CurrentColumn = c;
		}
		
		// Double-click processing...
		ListViewCell *cell = GetCellInfo(r, c);
		if(!editing && event->GetClickCount() == 2){
			// remove this if if you want to permit double-click actions
			// on the checkboxed cells.
			if(cell!=NULL) {
				if(cell->check_button==NULL) { 
					if(click_callback){
						gString cmd;
						cmd.sprintf((vchar*)"%s %d %d", click_callback, r, c);
						gala_eval(interp(), (char*)(vchar*)cmd);
					}
				} else return cell->check_button->HandleButtonDown(event);
			}
			return vlistview::HandleButtonDown(event);   
		}
		if(cell == NULL) return vlistview::HandleButtonDown(event);
		
		//--------------------------------------------------------------
		// if check button embagged into the cell just pass the mouse
		// click to the check button and select this cell
		if(cell->check_button!=NULL) {
			if(editing)  CloseEditor();
			SetFocus(r,c);
			int res = cell->check_button->HandleButtonDown(event);
			OnCheck(r,c,cell->check_button->GetValue());
		}
		//---------------------------------------------------------------
		
		//---------------------------------------------------------------
		// if there is no menu selected
		if(cell->popup_items.size() == 0) {
			if(editing) { // in the editing mode
				if(cell->read_only) { // read-only cell was selected
					CloseEditor();
					SetFocus(r,c);
					// listview will process this cell by itself
					return vTRUE;   
				}
				// the same cell was selected - editor will process mouse
				// click
				if ( textitem->GetRect()->ContainsPoint(point.x, point.y) ) {
					return textitem->HandleButtonDown(event);
				} else { // different cell with editor was selected
					// editor will be re-created on a new cell
					SetFocus(r,c);
					CloseEditor();
					edit_row = r;
					edit_col = c;
					CreateEditor();
					return vTRUE;
				}
			} else { // editor was not created yet
				// if the cell is read-only let vlistview to process 
				// this message
				SetFocus(r,c);
				if(cell->read_only) return vTRUE;   
				// creating editor at the celected cell
				edit_row = r;
				edit_col = c;
				CreateEditor();
				textitem->Inval(vwindowINVAL_UPDATE);
				return vTRUE;
			}
		} else { // the cell contains menu
			// if we were in the editor cell - close the editor
			if(editing) CloseEditor();
			SetFocus(r,c);
			// wake up popup menu
			ListViewPopupMenu *menu = CreateCellMenu(r, c);
			return menu->Popup(event); 
		}
} else if(button == 2){
	ListViewPopupMenu *menu = CreateCellMenu(r, c);
	if(menu == NULL) {
	       if(GetList()->IsCellSelected(GetSelection(), r, c)) {
			   if(popup_items.size() != 0) {
				   menu = BuildPopupMenu(popup_items, r, c);
				   return menu->Popup(event);
			   }        
		   }
	} else {
		return menu->Popup(event);
	}
} 
return vlistview::HandleButtonDown(event);   
}

int gdListView::HandleButtonUp(vevent *event) {
	vpoint        point;
	int           r,c;
	
	point.x = event->GetX(); 
	point.y = event->GetY();
    if (!GetRect()->ContainsPoint(point.GetX(), point.GetY())) 
		return vlistview::HandleButtonUp(event);    
    MapPointToCell(&point, &r, &c);
    if(r == -1 || c == -1)  
		return vlistview::HandleButtonUp(event);    
	ListViewCell *cell = GetCellInfo(r, c);
	if(cell == NULL) return vlistview::HandleButtonUp(event);
	if(cell->check_button!=NULL) return cell->check_button->HandleButtonUp(event);
	
    int res;
	res=vlistview::HandleButtonUp(event);
	if(editing)  if(textitem->HandleButtonUp(event)==vTRUE) return vTRUE;
	return res;  
}

int gdListView::HandleMotion(vevent *event) {
	vpoint        point;
	int           r,c;
	
	point.x = event->GetX(); 
	point.y = event->GetY();
    if (!GetRect()->ContainsPoint(point.GetX(), point.GetY())) 
		return vlistview::HandleMotion(event);    
    MapPointToCell(&point, &r, &c);
    if(r == -1 || c == -1)  
		return vlistview::HandleMotion(event);    
	ListViewCell *cell = GetCellInfo(r, c);
	if(cell == NULL) return vlistview::HandleMotion(event);
    int res;
	res=vlistview::HandleMotion(event);
    if(editing)  if(textitem->HandleMotion(event)==vTRUE) return vTRUE;
	return res;
}

int gdListView::HandleDrag(vevent *event) {
	vpoint        point;
	int           r,c;
	
	point.x = event->GetX(); 
	point.y = event->GetY();
    if (!GetRect()->ContainsPoint(point.GetX(), point.GetY())) 
		return vlistview::HandleDrag(event);    
    MapPointToCell(&point, &r, &c);
    if(r == -1 || c == -1)  
		return vlistview::HandleDrag(event);    
	ListViewCell *cell = GetCellInfo(r, c);
	if(cell == NULL) return vlistview::HandleDrag(event);
    int res;
    res=vlistview::HandleDrag(event);
    if(editing)  if(textitem->HandleDrag(event)==vTRUE) return vTRUE;
	return res;
}

int gdListView::HandleKeyDown(vevent *event) {  
	int    r, c;
	
    if (editing){
		if((event->binding && strcmp((char *)event->binding, "Confirm") == 0)) {
			CloseEditor();
			return vTRUE;
		} else {
			key_pressed=1;    
			textitem->HandleKeyDown(event);
			return vTRUE;
		}
    } else {
		GetFocus(&r,&c);
		if(r == -1 || c == -1)  
			return vlistview::HandleKeyDown(event);    
		ListViewCell *cell = GetCellInfo(r, c);
		if(cell!=NULL && cell->check_button!=NULL) { 
			cell->check_button->HandleKeyDown(event);
		}
		if((event->binding && strcmp((char *)event->binding, "Confirm") == 0)) {
			if(cell!=NULL && !cell->read_only  && cell->popup_items.size() == 0) {
				SetFocus(r,c);
				edit_row=r;
				edit_col=c;
				CreateEditor();
				textitem->Inval(vwindowINVAL_UPDATE);
				return vTRUE;
			}
		}
		if(event->GetBinding() != NULL) {
			int res=vlistview::HandleKeyDown(event);
			GetFocus(&r,&c);
			if( CurrentRow!=r || CurrentColumn!= c ) {
				gString cmd;
				cmd.sprintf((vchar*)"%s %d %d %d %d", GetCellChangeCallback(), r, c,CurrentRow,CurrentColumn);
				gala_eval(interp(), (char*)(vchar*)cmd);
				CurrentRow    = r;
				CurrentColumn = c;
			}
			return res;
		}
	}
	return vFALSE;
}

int gdListView::HandleKeyUp(vevent *event) {
	int r,c;
	
    if (editing) {
		return textitem->HandleKeyUp(event);
	} else {
		GetFocus(&r,&c);
		if(r == -1 || c == -1)  
			return vlistview::HandleKeyUp(event);    
		ListViewCell *cell = GetCellInfo(r, c);
		if(cell == NULL)  
			return vlistview::HandleKeyUp(event);
		if(cell->check_button!=NULL) {
			cell->check_button->HandleKeyUp(event);
			vlistview::HandleKeyUp(event);
		}
		return vlistview::HandleKeyUp(event);
	}
}

void gdListView::AddPopupItem(char *mytitle, char *callback)
{
    PopupMenuItemDescPtr desc = new PopupMenuItemDesc;
    desc->title    = strdup(mytitle);
    desc->callback = strdup(callback);
    popup_items.append(desc);
}

void gdListView::HandlePopupItem(ListViewPopupMenuItem *item)
{
    char *itemtitle = (char *)item->GetTitle();
	
    if(internal_list){
		int     r, c;
		ListViewPopupMenu *menu = item->DetermineMenu();
		menu->GetRowCol(r, c);
		ListViewCell *cell = GetCellInfo(r, c);
		for(int i = 0; i < cell->popup_items.size(); i++){
			PopupMenuItemDescPtr desc = cell->popup_items[i];
			if(strcmp(desc->title, itemtitle) == 0){
				gString cmd;
				cmd.sprintf((vchar*)"%s {%s} %d %d", desc->callback, desc->title, r, c);
				gala_eval(interp(), (char*)(vchar*)cmd);
				break;
			}
		}
    } else {
		for(int i = 0; i < popup_items.size(); i++){
			PopupMenuItemDescPtr desc = popup_items[i];
			if(strcmp(desc->title, itemtitle) == 0){
				gString cmd;
				int     r, c;
				ListViewPopupMenu *menu = item->DetermineMenu();
				menu->GetRowCol(r, c);
				cmd.sprintf((vchar*)"%s %d %d", desc->callback, r, c);
				gala_eval(interp(), (char*)(vchar*)cmd);
				//            cmd = (vchar*)interp->result;
				break;
			}
		}
    }
}

void gdListView::CreateEditor()
{
    const vchar *value;
    vrect       r;
	
    value = (const vchar*)GetCellValue(edit_row, edit_col);
    textitem->SetText(value ? value : vcharGetEmptyString());
    // Resize the textitem's bounds to hold the new editing
    // value
    textitem->SelectAll();
    GetCellRect(edit_row, edit_col, &r);
    r.Inset(&r,1,1);
	r.h+=2;
	if(r.x>1) {
		r.x-=2;
		r.w+=2;
	}
    textitem->SetRect(&r);
    
    // Make sure the listview's focus it set to the edit cell
    SetFocus(edit_row, edit_col);
    
    // Black magic to display the "floating" textitem
    itemlist->Append(textitem);
    itemlist->Open();
    itemlist->Activate();
    textitem->Hide();
    textitem->Show();
    itemlist->SetDragItem(textitem);
    GetItemListIn()->SetFocusItem(this);
    itemlist->Focus();
    itemlist->SetFocusItem(textitem);
    // Show text
    editing = vTRUE;
    textitem->Open();
}

void gdListView::CloseEditor()
{
    if(editing){
		// Hide text
		editing   = FALSE;
		vstr *txt = textitem->GetTextAsString();
		PropagateItemChange(edit_row, edit_col, (char *)txt);
		vstrDestroy(txt);
		textitem->Hide();
		// Undo the black magic
		itemlist->SetFocusItem(NULL);
		textitem->Close();
		itemlist->Close();
		itemlist->Deactivate();
	}
}


// This function is designed to generate a check button and perform some
// black magic to show it correctly in the item list. Pointer to this
// check button must be saved somewere to let us remove this button in future.
gdListCheck* gdListView::CreateCheckButton(int row, int col) {
	const vchar     *value;
	vrect           r;
	gdListCheck*    check;
	
	// New check
	check=new gdListCheck(this);
	check->SetClipRect(GetRect());
    value = (const vchar*)GetCellValue(row,col);
	// set it's position
    GetCellRect(row, col, &r);
	if(value && vcharLength(value)!=0) {
		check->SetTitle(value);
		r.Inset(&r, 1, 1);
    } else {
		check->SetTitle(vcharGetEmptyString());
		r.Inset(&r, 1, 1);
		int d=r.h;
		r.x=r.x+(r.w-d)/2;
		r.w=d;
	}
    check->SetRect(&r);
	return check;
}



void gdListView::PropagateItemChange(int row, int col, char *txt) {
    gString cmd;
    cmd.sprintf((vchar*)"%s {%s} %d %d", change_callback, txt, row, col);
    gala_eval(interp(), (char*)(vchar*)cmd);
    ListViewCell *cell = GetCellInfo(row, col);
    if(cell)
		SetDataCell(row, col, txt, cell->read_only, cell->data);
    else
		SetDataCell(row, col, txt, 0, NULL);
}

void ListViewPopupMenuItem::ObserveMenuItem(vevent *event)
{
    vmenuItem::ObserveMenuItem(event);
    gdListView *gdlist = DetermineMenu()->GetListView();
    gdlist->HandlePopupItem(this);
    DetermineMenu()->DeleteLater();
}

ListViewPopupMenu *ListViewPopupMenuItem::DetermineMenu()
{
	return (ListViewPopupMenu *)vmenuItem::DetermineMenu();
}

// --------------------------------------------------------------------

vdialog *ListViewItemList::DetermineDialog()
{
	return(((gdListView*)this->GetOwner())->DetermineDialog());
}

vcursor *ListViewItemList::DetermineCursor()
{
	return(((gdListView*)this->GetOwner())->DetermineCursor());
}

vfont *ListViewItemList::DetermineFont()
{
	return(((gdListView*)this->GetOwner())->DetermineFont());
}

void ListViewItemList::DetermineColorsFrom(long theFlags,
                                           vcolor **known,
										   int *ids, int count, vcolor **colors)
{
	((gdListView*)this->GetOwner())->DetermineColorsFrom(theFlags,
		known,
		ids,
		count,
		colors);
}

void ListViewItemList::DetermineColorSchemeFrom(vcolorScheme *known,
                                                unsigned long myflags,
                                                vcolorScheme *scheme)
{
	((gdListView*)this->GetOwner())->DetermineColorSchemeFrom(known, myflags,
		scheme);
}

int ListViewItemList::DetermineLineWidth()
{
    return(((gdListView*)this->GetOwner())->DetermineLineWidth());
}

int ListViewItemList::DetermineShown()
{
    return(((gdListView*)this->GetOwner())->DetermineShown());
}

int ListViewItemList::DetermineEnabled()
{
    return(((gdListView*)this->GetOwner())->DetermineEnabled());
}

void ListViewItemList::DetermineBounds(register vrect *r)
{
    ((gdListView*)this->GetOwner())->GetItemListIn()->DetermineBounds(r);
}

void ListViewItemList::DetermineContent(vrect *r)
{
    ((gdListView*)this->GetOwner())->GetItemListIn()->DetermineContent(r);
}

vdict *ListViewItemList::GetKeyMap()
{
    return(((gdListView*)this->GetOwner())->GetKeyMap());
}

vdict *ListViewItemList::GetKeyDispatch()
{
    return(((gdListView*)this->GetOwner())->GetKeyDispatch());
}

void ListViewItemList::DrawDevice()
{
    ((gdListView*)this->GetOwner())->DrawDevice();
}

/************* gdNotebook **********************/

void gdNotebookPage::ObserveDialogItem(vevent *event)
{
    if(!strcmp((char *)event->GetBinding(), "Select")){
		SetInterpVariable((vchar*)GetTag(), (vchar*)"");
    }
}

void gdNotebookPage::Select () {
	vnotebookPage::Select();
	Update();
}

void gdNotebookPage::Update () {
    SetInterpVariable((vchar*)GetTag(), (vchar*)"");
}

void gdNotebookPage::Open()
{
    vnotebookPage::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}


/************* gdNotebook **********************/


void gdNotebook::Open()
{
    vnotebook::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}


/************* gdProgressItem **********************/


void gdProgressItem::Open()
{
    ProgressItem::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
	
    AlwaysImmediateRefresh ();
    ChangeAppearance (appearBlended);
    Blend (Viewer::GetProgressForeground(), Viewer::GetProgressBackground());
	
    if (vcharLength(GetTitle())) {
        Face().SetProgressTitle ((vchar *)GetTitle());
    }
}


/************* gdTextObserver **********************/



void gdTextObserver::InitObserver(gdWidget* my_owner)
{
    owner_ = my_owner;
}



void gdTextObserver::ObserveDelete(long pos, long delta)
{
    vtextObserver::ObserveDelete(pos, delta);
    owner_->ChangeHasOccurred();
}



void gdTextObserver::ObserveInsert(long pos, long delta)
{
    vtextObserver::ObserveInsert(pos, delta);
    owner_->ChangeHasOccurred();
}


/************* gdTextEditor **********************/

void gdTextEditor::Open()
{
    vtextview::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
	
    gdTextObserver* newObserver = new gdTextObserver;
    newObserver->InitObserver(this);
	
    //my_data->InstallObserver(newObserver);
	
    InitgdWidget((vchar*)GetTag(), this);
	
    item_has_changed      = vFALSE;
}

void gdTextEditor::Destroy(void)
{
    vtextview::Destroy();
}

void gdTextEditor::Unfocus()
{
    vtextview::Unfocus();
}

int gdTextEditor::HandleKeyDown(vevent *event)
{
    return vtextview::HandleKeyDown(event);
}

void gdTextEditor::ChangeHasOccurred()
{
    item_has_changed = 1;
}


void gdTextEditor::UpdateDisplay()
{
}

void gdTextEditor::DrawBackground(vrect *r)
{
    vtextview::DrawBackground(r);
}

void gdTextEditor::OpenFile (vchar*filename)
{
}

void gdTextEditor::SaveFile ()
{
}

void gdTextEditor::SaveFileAs (vchar* filename)
{
}

void gdTextEditor::CloseFile()
{
}

/************* gdTextItem **********************/

void gdTextItem::Init()
{
	vtextitem::Init();
	CommonInit();
}

void gdTextItem::LoadInit(vresource res)
{
	vtextitem::LoadInit(res);
	CommonInit();
}

void gdTextItem::CopyInit(vloadable * loadable)
{
	vtextitem::CopyInit(loadable);
	CommonInit();
}

void gdTextItem::CommonInit()
{
    gdTextObserver* newObserver = new gdTextObserver;
    newObserver->InitObserver(this);
    vtext* my_data = GetTextData();
    my_data->InstallObserver(newObserver);
    file_buffer           = NULL;
    file_buffer_size      = 0;
    allocated_buffer_size = 0;
    timer                 = NULL;
    item_has_changed      = vFALSE;
    if(IsOneLine())
		SetOffscreenDrawing(vTRUE);
    updateCommand_ = NULL;
	scanner = NULL;
}

void gdTextItem::Open()
{
    vtextitem::Open();
	
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
    UpdateDisplay();
}

void gdTextItem::Destroy(void)
{
    if(timer)
		StopUpdating();
    if(file_buffer)
		vmemFree(file_buffer);
	if (scanner) delete scanner;
}

void gdTextItem::Unfocus()
{
    Viewer* vr;
    View *view_;
    Viewer::GetExecutingView (vr, view_);
    if (item_has_changed && (!insideUpdate_) && (dialog_||view_)) {
        pane_ = view_ ? view_->GetPane() : NULL;
		
        if (item_->IsKindOf (&gdTextItem::Kind)) {
            gdTextItem* ti = gdTextItem::CastDown(item_);
			if (!ti->IsOneLine()) {
				vtextitem::Unfocus();
				return;
			}
		}
        vstr* result = GetTextAsString();
        SetInterpVariable((vchar*)GetTag(), result);
        vstrDestroy(result);
        item_has_changed = 0;
    }
    vtextitem::Unfocus();
}

int gdTextItem::HandleKeyDown(vevent *event)
{
    gdTextItem* ti = gdTextItem::CastDown(item_);
    if (ti->IsOneLine()) {
		const vchar* binding = event->GetBinding();
		if (binding) {
			if (vcharCompare(binding, (vchar*)"Confirm") == 0) {
				vdialog *dialog = DetermineDialog();    
				if(dialog){
					vdialogItemList *list = GetItemListIn();
                    // Change the focus. vFALSE means find next focasable item,
                    // vTRUE means find next focasable text-item,
					list->MoveFocus(1, vFALSE);
					//return TRUE;
				}
            }
		}
    }
    return vtextitem::HandleKeyDown(event);
}

void gdTextItem::ChangeHasOccurred()
{
    item_has_changed = 1;
    if (updateCommand_) {
        Tcl_SetVar(interp(), "updateTextValue", (char*)GetTextAsString(), TCL_GLOBAL_ONLY);
        gala_eval(interp(), (char*)updateCommand_);
    }
}


void gdTextItem::UpdateDisplay() {
    if (dialog_||pane_)  {
		char* newValue = NULL;
		static int doit = 0;
		newValue = Tcl_GetVar (interp(), (char*)GetTag(), TCL_GLOBAL_ONLY);
		if(newValue!=NULL) SetText((const unsigned char *)newValue);
		else               SetText((const unsigned char *)"");
    }
}

void gdTextItem::DrawBackground(vrect *r)
{
	//    printf("Draw Background called\n");
	//    fflush(stdout);
    vtextitem::DrawBackground(r);
}

void gdTextItem::HighliteLine(int line)
{
    vtext *text         = GetTextData();
    vtextSelection *sel = GetSelection();
    if(text && sel){
		text->SelectLines(sel, line, line, vtextLINE_ALL);
		ScrollToStart(vtextviewSTS_VERTICAL);
		InvalView();
    }
}

void gdTextItem::OpenFile (vchar*filename)
{
}

void gdTextItem::SaveFile ()
{
}

void gdTextItem::SaveFileAs (vchar* filename)
{
}

void gdTextItem::CloseFile()
{
}

void gdTextItem::AddScanner (const vchar* word, const vchar* cmd)
{
	if (!scanner)
		scanner = new Scanner;
	scanner->ScanPrefix (word, cmd);
}

void gdTextItem::UpdateFile(void)
{
    if(vfile == NULL)
	     return;
    vfile->Seek(0, vfsSEEK_FROM_END);
    long file_size = vfile->GetPos();

	long from;
	int diff;
	if(file_size<30000) {
		from=0;
		diff=file_size;
	} else {
		from=file_size-30000;
		diff=30000;
	}
    if (file_buffer_size >= file_size) return;
	
    if(file_buffer!=NULL) {
		vmemFree(file_buffer);
		file_buffer=NULL;
	}
	file_buffer = (char *)vmemAlloc((diff+5) * sizeof(vchar));
    vfile->Seek(from, vfsSEEK_FROM_START);
    vfile->RawRead((vbyte *)file_buffer, diff);
#ifdef _WIN32
    //We have to replace <CR><LF> pair with <LF> on Windows.
    //The file can not be opened in text (translated) mode since
    //it will break our diff mechanism
    	
	char *pCurr = file_buffer;
 	char *pCR = strchr( pCurr, '\r' );
	if(pCR!=NULL) {
       do {
        *pCR = ' ';
         pCR = strchr( pCurr, '\r' );
	   } while(pCR!=NULL);
	}
#endif

    vtext *text         = CreateTextData();
    vtextSelection *sel = GetSelection();
    if(text && sel){
	    vbool mod = IsModifiable();
	    SetModifiable(1);
	    if (scanner)
		     scanner->ScanString ((vchar*)file_buffer, interp());
	    text->AppendBlock((vchar *)file_buffer, diff, diff);
		SetTextData(text);
	    text->Calculate();
	    file_buffer_size += (file_size - file_buffer_size);

	    SetModifiable (mod);

	    int len = text->GetLength();
	    Scroll(len, 0);
    }
    item_has_changed = 1;
   
}


int gdTextItem::ShowFile(vchar *fname, int keep_updating)
{
    vfsPath vpath;
    int fileOpenError = 0;
	
    if (scanner) 
		scanner->Reset();
    
    if (timer)
		StopUpdating();
    if (file_buffer)
		delete [] file_buffer;
	
    file_buffer           = NULL;
    file_buffer_size      = 0;
    allocated_buffer_size = 0;
    if ((char)fname[0] == 0)
		return -1;

    gString filename = (vchar *)realOSPATH((char*)fname);
    vpath.Set(filename);
	
    int how;
	
    if (keep_updating)
		how = vfsOPEN_READ_WRITE;
    else
		how = vfsOPEN_READ_ONLY;
	
    vexWITH_HANDLING {
		vfile = vfsFile::Open (&vpath, how);
    } vexON_EXCEPTION {
        fileOpenError = 1;
    } vexEND_HANDLING;
    if (fileOpenError) return -1;
	
	vtext* text=CreateTextData();
	SetTextData(text);

    UpdateFile();

    if (keep_updating){
		if(timer)
			StopUpdating();
		timer = new gdTextItemUpdateTimer;
		timer->SetOwner(this);
		timer->SetRecurrent();
		//timer->SetPeriod(0, 500000000);
		//	timer->SetPeriod(0, 500000000);
		timer->SetPeriod(1, 0);
		timer->Start();
    } else {
        Scroll(0,0);
		vfile->Close();
		vfile = NULL;
    }
    return 0;
}

void gdTextItem::StopUpdating(void)
{
	if (!timer) return;
    timer->SetOwner(NULL);
    timer->Stop();
    delete timer;
    timer = NULL;
    if( vfile )
		vfile->Close();
}

void gdTextItem::TimerNotifyCallback(void)
{
    if (timer) timer->Stop();
    UpdateFile();
    if (timer) timer->Start();
}


void gdTextItem::SetUpdateCmd(vchar* updateCommand)
{
    updateCommand_ = new vchar [vcharLength(updateCommand)+1];
    vcharCopy((const vchar*)updateCommand, updateCommand_);
}
/************* gdTextItemUpdateTimer **********************/

void gdTextItemUpdateTimer::ObserveTimer(void)
{
    if(owner)
		owner->TimerNotifyCallback();
}



/************* gdQueryTextItem **********************/

void gdQueryTextItem::Open()
{
    vtextitem::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
    Tcl_TraceVar(dialog_->GetInterpreter(), (char*)GetTag(),
		TCL_TRACE_READS, gdQueryTextItem::RequestText,
		(ClientData) this);
}



char* gdQueryTextItem::RequestText(ClientData cd, Tcl_Interp*, char*, char*, int)
{
    gdQueryTextItem* self = (gdQueryTextItem*)cd;
    if (self->dialog_)  {
		vstr* result = self->GetTextAsString();
        Tcl_SetVar(self->dialog_->GetInterpreter(), (char*) self->GetTag(), (char*) result, TCL_GLOBAL_ONLY);
        vstrDestroy(result);
    }
    return NULL;
}


void gdQueryTextItem::UpdateDisplay()
{
    if (dialog_)  {
		int originalState = IsModifyable();
		SetModifyable(vTRUE);
		vchar* newValue = dialog_->GetInterpVariable((vchar*)GetTag());
        if(newValue!=NULL) SetText((const unsigned char *)newValue);
		else SetText((const unsigned char *)"");
	    SetModifyable(originalState);
    }
}



/************* gdExclusiveGroup **********************/


void gdExclusiveGroup::Open()
{
    vcontainerExclusiveGroup::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}


void gdExclusiveGroup::UpdateDisplay()
{
    if (dialog_ || pane_) {
        char* newValue =  NULL;
        newValue = (char *) GetInterpVariable((vchar*) GetTag());
		
		vdialogItem *targetItem  = FindItem(vnameInternGlobalLiteral(newValue));
		//        gdToggle* targetItem = gdToggle::CastDown(FindItem(vnameInternGlobal(newValue)));
        //        if (targetItem) targetItem->SetValue(vTRUE);
        if(targetItem) SetSelection(targetItem);
    }
}


void gdExclusiveGroup::ObserveDialogItem (vevent *event) {
    vcontainerExclusiveGroup::ObserveDialogItem (event);
	
    if ((!insideUpdate_) && (dialog_ || pane_)) {
        vdialogItem *selected = GetSelection();
        if (selected) {
            SetInterpVariable((vchar*) GetTag(), (vstr*) selected->GetTag());
        }
    }
}


/************* gdToggle **********************/


void gdToggle::Open()
{
    vcontrolToggle::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    if (IsExclusive())  {
		parent_ = gdExclusiveGroup::CastDown(GetParentObject());
		variableName_ = (vchar*)parent_->GetTag();
    }
    else  {
		variableName_ = (vchar*)GetTag();
		InitgdWidget((vchar*)GetTag(), this);
    }
}



void gdToggle::ObserveDialogItem(vevent* event)
{
    vcontrolToggle::ObserveDialogItem(event);
    if ((!insideUpdate_) && (dialog_ || pane_)) {
		
		vchar buffer[LOCAL_STRING_SIZE];
		
		if (IsExclusive())
			sprintf((char*)buffer,"%s",GetTag());
		else
			sprintf((char*)buffer,"%d",GetValue());
		
		SetInterpVariable(variableName_, buffer);
    }
}


void gdToggle::UpdateDisplay()
{
    if (dialog_ || pane_)  {
		vchar* newValue = GetInterpVariable(variableName_);
		int resultValue;
		if (IsExclusive())
			parent_->UpdateDisplay();
		else if (newValue) {
			resultValue = atoi((const char*)newValue);
			SetValue(resultValue);
		}
    }
}


/************* gdComboBox **********************/


void gdComboBox::Open()
{
    vcombo::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
	
    gdTextObserver* newObserver = new gdTextObserver;
    newObserver->InitObserver(this);
    vtext* my_data = GetTextitem()->GetTextData();
    my_data->InstallObserver(newObserver);
}


void gdComboBox::ObserveDialogItem(vevent* event)
{
    vcombo::ObserveDialogItem(event);
    if ((!insideUpdate_) && (dialog_ || pane_)) {
		vstr* result = GetTextAsString();
        SetInterpVariable((vchar*) GetTag(), result);
		vstrDestroy(result);
    }
}


void gdComboBox::UpdateDisplay()
{
    if (dialog_ || pane_)  {
		char* newValue  = NULL;
		newValue = Tcl_GetVar (interp(), (char*)GetTag(), TCL_GLOBAL_ONLY);
		if (newValue) SelectText((const vchar*)newValue);
    }
}


/************* gdPopDownComboBox **********************/


void gdPopDownComboBox::Open()
{
    vcomboPopdown::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());    
    InitgdWidget((vchar*)GetTag(), this);
    this->SetDefaultCompareFunc(CompareProc);
    this->SetAutoSort(vFALSE);
    gdTextObserver* newObserver = new gdTextObserver;
    newObserver->InitObserver(this);
    vtext* my_data = GetTextitem()->GetTextData();
    my_data->InstallObserver(newObserver);
    SetObserveComboboxProc(gdPopDownComboBoxObserver);
    //SetModifiable(vTRUE);
    SetAcceptBehavior(vcomboboxBEHAVIOR_MATCH_OR_NOTHING);
    SetSelectBehavior(vcomboboxBEHAVIOR_MATCH_OR_NOTHING);
}

void gdPopDownComboBox::SetMatchBehavior(int flag)
{
    if(flag){
		SetAcceptBehavior(vcomboboxBEHAVIOR_MATCH_OR_CLOSEST);
		SetSelectBehavior(vcomboboxBEHAVIOR_MATCH_OR_CLOSEST);
    } else {
		SetAcceptBehavior(vcomboboxBEHAVIOR_MATCH_OR_NOTHING);
		SetSelectBehavior(vcomboboxBEHAVIOR_MATCH_OR_NOTHING);
    }
}

// Pop-Down Combo Box Compare function. 
//     Always returns item-is-greater.
int gdPopDownComboBox::CompareProc(const vchar* str1, const vchar* str2)
{
    return(vcharCompare(str1, str2) );
}

void gdPopDownComboBox::gdPopDownComboBoxObserver(vcombobox *combo, vevent *event, int message)
{
    static int was_notify = FALSE;
    static gString value;
    gdPopDownComboBox *This = (gdPopDownComboBox *)combo;
#ifdef _WIN32
    if(message == vcomboboxNOTIFY_ACCEPT){
#else
		if(message == vcomboboxPopdownNOTIFY_CLOSE || message == vcomboboxNOTIFY_ACCEPT){
#endif
#if 0
			vstr* result = This->GetTextAsString();
			value = (vchar*)result;
			vstrDestroy(result);
#endif
			int index = This->GetSelectedIndex();
			if(index >= 0){
				vcomboboxValue *val = This->GetValueAt(index);
				value               = (vchar*)val->GetTitle();
				if ((!This->insideUpdate_) && (This->dialog_ || This->pane_)){
					This->SetInterpVariable((vchar*) This->GetTag(), (vchar*)value);
				}
			} else {
				if(This->GetValueCount() != 0){
					This->SelectValueAt(0);
					vcomboboxValue *val = This->GetValue();
					This->AcceptValue(val);
					val   = This->GetValue();
					value = (vchar*)val->GetTitle();
					if ((!This->insideUpdate_) && (This->dialog_ || This->pane_)){
						This->SetInterpVariable((vchar*) This->GetTag(), (vchar*)value);
					}
				} else {
					vtextitem *item = (vtextitem *)This->GetEntrybox();
					item->SetText(NULL);
				}
			}
		}
	}
	
	void gdPopDownComboBox::ObserveDialogItem(vevent* event)
	{
		vcomboPopdown::ObserveDialogItem(event);
#if 0
		if ((!insideUpdate_) && (dialog_ || pane_)){
			vstr* result = GetTextAsString();
			SetInterpVariable((vchar*)GetTag(), result);
			vstrDestroy(result);
		}
#endif
	}
	
	
	void gdPopDownComboBox::UpdateDisplay()
	{
		if (dialog_ || pane_)  {
			char* newValue  = NULL;
			static int doit = 0;
			
			newValue = Tcl_GetVar (interp(), (char*)GetTag(), TCL_GLOBAL_ONLY);
			if (newValue) SelectText((const vchar*)newValue);
		}
	}
	
	void gdPopDownComboBox::Deactivate()
	{
		vdialogItem::Deactivate();
	}
	
	
	/************* gdRuler **********************/
	
	void gdRuler::Init() {
		vruler::Init();
		CommonInit();
	}
	
	void gdRuler::LoadInit (vresource resource) {
		vruler::LoadInit(resource);
		CommonInit();
	}
	
	void gdRuler::CopyInit (vloadable *loadable) {
		vruler::CopyInit (loadable);
		CommonInit();
	}
	
	void gdRuler::CommonInit () {
		compacting = vFALSE;
	}
	
	void gdRuler::setListView (gdListView *l) {
		if(l==NULL) return;
		lview_ = l;
		l->_ruler=this;
		vrulerItem* last=GetItemAt(GetItemCount()-1);
		if(last!=NULL) {
			last->SetWidth(lview_->GetListRect()->GetWidth()-last->GetOffset()-GetItemCount());
		}
		updateListView ();
	}
	
	
	void gdRuler::ObserveRuler (vevent *event, vrulerItem *item, int message) {
		
		if (compacting) return;
		
		switch (message) {
        case vrulerITEM_MOVE_END:  // 7  Item has completed a move
			compacting  = vTRUE;
			Compact();
			compacting  = vFALSE;
			break;
        case vrulerITEM_RESIZE_END:
			compacting = vTRUE;
			Compact();
			updateListView();
			compacting = vFALSE;
			break;
		}
	}
	
	
	void gdRuler::updateListView () {
		vrulerItem* ri;
		int column;
		int width,allWidth=0;
		vrulerIterator iter;
		
		iter.Start (this);
		do {
			ri = iter.GetItem();
			if (ri) {
				column = atoi ((const char *)ri->GetTag());
				width=ri->GetWidth() + GetSpacing()-2;
				lview_->SetColumnWidth (column,width);
				allWidth+=width;
			}
		} while (iter.Next());
		allWidth-=width;
		allWidth+=2*GetItemCount();
		ri->SetWidth(lview_->GetListRect()->GetWidth()-allWidth);
		lview_->SetColumnWidth(column, lview_->GetListRect()->GetWidth()-allWidth);
	}
	
	/************* gdLabel **********************/
	
	void gdLabel::Open()
	{
		vdialogLabelItem::Open();
		dialog_ = GDialog::CastDown(DetermineDialog());
		if (GetTag())  {
			InitgdWidget((vchar*)GetTag(), this);
			//      SetInterpVariable((vchar*)GetTag(), (vchar *)GetTitle());
			UpdateDisplay();
		}
	}
	
	void gdLabel::UpdateDisplay()
	{
		vchar* newValue;
		newValue = GetInterpVariable((vchar*)GetTag());
		if(newValue) SetTitle(newValue);
		
		Inval(vwindowINVAL_OPAQUE);
	}
	
	/************* gdMultiLineLabel **********************/
	
	void gdMultiLineLabel::Open()
	{
		vcolor *color = NULL;
		vdialog *dlg = DetermineDialog();
		if( dlg )
			color = dlg->GetBackground();
		if( color )
			SetBackground( color );
		dialog_ = GDialog::CastDown(dlg);
		
		vtextitem::Open();
		
		if (GetTag())  
		{
			InitgdWidget((vchar*)GetTag(), this);
			UpdateDisplay();
		}
	}
	
	void gdMultiLineLabel::UpdateDisplay()
	{
		vchar* newValue;
		newValue = GetInterpVariable((vchar*)GetTag());
		if(newValue) SetText(newValue);
		Inval(vwindowINVAL_OPAQUE);
	}
	
	
	/************* gdSlider **********************/
	
	
	void gdSlider::Open()
	{
		vslider::Open();
		dialog_ = GDialog::CastDown(DetermineDialog());
		InitgdWidget((vchar*)GetTag(), this);
	}
	
	
	void gdSlider::UpdateDisplay()
	{
		if (dialog_ || pane_) {
			char* newValue = (char *)GetInterpVariable((vchar*)GetTag());
			SetValueScroll (atoi (newValue));
		}
	}
	
	
	void gdSlider::ObserveDialogItem (vevent *event) {
		vslider::ObserveDialogItem (event);
		
		if ((!insideUpdate_) && (dialog_ || pane_)) {
			int sliderValue = GetValue();
			char sliderValueStr[10];
			sprintf (sliderValueStr, "%d", sliderValue);
			SetInterpVariable ((vchar*) GetTag(), (vstr*)sliderValueStr);
		}
	}
	
	vspringdomObject *SpingDomObjFromItem( vspringdom *sd, vdialogItem *item )
	{
		vspringdomObject *resultObj = NULL;
		vdialogItemList* dil = item->GetItemListIn ();
		if(dil)
		{
			vcontainer *c = vcontainer::CastDown (dil->GetOwner());
			if( c )
			{
				vspringdomObject* springObj = (vspringdomObject *)sd->GetTopObject ();
				int count = c->GetItemCount ();
				for (int idx = 0;
				springObj && (idx < count);
				idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
				{
					vdialogItem* itm = c->GetItemAt (idx);
					if (itm == item)
					{
						resultObj = springObj;
						break;
					}
				}
			}
		}
		return resultObj;
	}
	
	vdialogItem *ItemFromSpingDomObj( vcontainer *c, vspringdom *sd, vspringdomObject *obj )
	{
		vdialogItem *resultItem = NULL;
		vspringdomObject* springObj = (vspringdomObject *)sd->GetTopObject ();
		int count = c->GetItemCount ();
		for (int idx = 0;
		springObj && (idx < count);
		idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
		{
			if (springObj == obj)
			{
				resultItem = c->GetItemAt (idx);;
				break;
			}
		}
		return resultItem;
	}
	
	
	void HandleResize (void *data )
	{
		static int nOldWidth = 0;
		static int nOldHeight = 0;
		int nNewAvailSize = 0;
		int fResize = 0;
		springContainer *cont = (springContainer *)data;
		if( cont->GetContainerType() == CONT_VERTICAL )
		{
			nNewAvailSize = cont->GetHeight() - cont->GetFixedSize();
		}
		else
		{
			nNewAvailSize = cont->GetWidth() - cont->GetFixedSize();
		}
		
		vspringSolution *sol = cont->GetSolution ();
		if( sol )
		{
			vspringdom* sd = vspringdom::Decompile (sol);
			if( sd )
			{
				// Find all panes.
				vspringdomObject* springObj = (vspringdomObject *)sd->GetTopObject ();
				int fChanged = 0;
				int count = cont->GetItemCount ();
				for (int idx = 0;
				springObj && (idx < count);
				idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
				{
					Pane *pane  = Pane::CastDown( cont->GetItemAt (idx) );
					//Only if dialog item is a Pane
					if( pane )
					{
						vspringdomConnection *inside = NULL;
						if( cont->GetContainerType() == CONT_VERTICAL )
							inside = sd->GetObjectConnection (springObj, vspringdomBOTTOM_INSIDE);
						else
							inside = sd->GetObjectConnection (springObj, vspringdomLEFT_INSIDE);
						if( pane->m_Ratio > 0 )
						{
							fChanged = 1;
							int nNewSize = int(float(nNewAvailSize)*pane->m_Ratio);
							sd->SetConnectionFixedBase( inside, nNewSize );
						}
						else
						{
							fChanged = 0;
							break;
						}
					}
				}
				if( fChanged )
				{
					sd->Recalculate();
					sol = sd->Compile();
					if( sol )
						cont->SetSolution (sol);
					
				}
				delete sd;
			}
		}
		
	}
	
	
	void springContainer::SetRect( const vrect *springRect )
	{
		int nWidth = GetWidth();
		int nHeight = GetHeight();
		vcontainer::SetRect( springRect );
		if( m_Type != CONT_UNDEFINED )
		{
			int nDelta;
			if( m_Type == CONT_VERTICAL )
				nDelta = GetHeight() - nHeight;
			else
				nDelta = GetWidth() - nWidth;
			//Play with connections only if rect changed
			if( nDelta )
			{
				callbackEvent *event = new callbackEvent( HandleResize, this );
				if( event )
					event->Post();
			}
		}
	}
	
	void springContainer::GetMinSize( int *w, int *h )
	{
		vcontainer::GetMinSize( w, h );
		int count = GetItemCount ();
		int nPanesSize = 0;
		int nMaxSize = 0;
		int nWidth, nHeight;
		int fRawSpring = 0;
		if( m_Type != CONT_UNDEFINED )
		{
			for (int idx = 0; idx < count && !fRawSpring; idx++) 
			{
				Pane *pane  = Pane::CastDown( GetItemAt (idx) );
				//Only if dialog item is a Pane
				if( pane )
				{
					if( pane->m_Ratio > 0 )
					{
						pane->GetMinSize( &nWidth, &nHeight );
						float base;
						if( m_Type == CONT_VERTICAL )
						{
							base = float(nHeight)/pane->m_Ratio;
							nPanesSize = nHeight;
						}
						else
						{
							base = float(nWidth)/pane->m_Ratio;
							nPanesSize = nWidth;
						}
						for (int i = 0; i < count; i++) 
						{
							if( i != idx )
							{
								pane  = Pane::CastDown( GetItemAt (i) );
								if( pane )
								{
									int nSize = int ( base * pane->m_Ratio );
									pane->GetMinSize( &nWidth, &nHeight );
									if( m_Type == CONT_VERTICAL )
										nPanesSize += MAX (nHeight, nSize);
									else
										nPanesSize += MAX (nWidth, nSize);
								}
							}
						}
						nMaxSize = MAX( nMaxSize, nPanesSize ); 
					}
					else
						fRawSpring = 1;
				}
			}
			if( !fRawSpring )
			{
				if( m_Type == CONT_VERTICAL )
					*h = nMaxSize + m_nFixedSize;
				else
					*w = nMaxSize + m_nFixedSize;
			}
		}
	}
	
	
	void springContainer::LoadInit( vresource res )
	{
		vcontainer::LoadInit(res);
		m_Type = CONT_UNDEFINED;
		m_nFixedSize = 0;
		m_fReentrance = 0;
	}
	
	int springContainer::InitWithType( SPRINGCONTAINERTYPE type)
	{
		//This function sets a container type (horizontal or vertical split)
		//and calculates non-adjustable part of the container's size
		//(this includes panes' outside connections and sashes' inside connections)
		m_Type = type;
		int nResult = 0;
		vspringSolution *sol = GetSolution ();
		if (sol)
		{
			vspringdom* sd = vspringdom::Decompile (sol);
			if (sd)
			{
				vspringdomObject* springObj = (vspringdomObject *)sd->GetTopObject ();
				int count = GetItemCount ();
				m_nFixedSize = 0;
				
				for (int idx = 0; springObj && (idx < count);
				idx++, springObj = (vspringdomObject *)sd->GetNextObjectBehind (springObj)) 
				{
					vdialogItem* itm = GetItemAt (idx);
					
					Pane *pane  = Pane::CastDown( itm );
					//Only if dialog item is a Pane
					if( pane )
					{
						vspringdomConnection *left, *right;
						if( type == CONT_VERTICAL )
						{
							left = sd->GetObjectConnection (springObj, 
								vspringdomTOP_OUTSIDE);
							right = sd->GetObjectConnection (springObj, 
								vspringdomBOTTOM_OUTSIDE);
						}
						else //assuming horizontal split
						{
							left = sd->GetObjectConnection (springObj, 
								vspringdomLEFT_OUTSIDE);
							right = sd->GetObjectConnection (springObj, 
								vspringdomRIGHT_OUTSIDE);
						}
						m_nFixedSize += sd->GetConnectionFixedBase (left);
						m_nFixedSize += sd->GetConnectionFixedBase (right);
					}
					else //assuming gdSash item
					{
						vspringdomConnection *left, *right;
						if( type == CONT_VERTICAL )
						{
							left = sd->GetObjectConnection (springObj, 
								vspringdomTOP_INSIDE);
							right = sd->GetObjectConnection (springObj, 
								vspringdomBOTTOM_INSIDE);
						}
						else
						{
							left = sd->GetObjectConnection (springObj, 
								vspringdomLEFT_INSIDE);
							right = sd->GetObjectConnection (springObj, 
								vspringdomRIGHT_INSIDE);
						}
						int nFixedLeft = sd->GetConnectionFixedBase (left);
						int nFixedRight = sd->GetConnectionFixedBase (right);
						int nWidth = MAX( nFixedLeft, nFixedRight );
						m_nFixedSize += nWidth;
					}
				}
				delete sd;
			}
		}
		return nResult;
	}
	
	//=========================================================================================================
	// This class designed to speed image loading from resource file.
	// We will use LRU strategy.
	//=========================================================================================================
	
	
	//---------------------------------------------------------------------------------------------------------
	// constructs image buffer with selected number of cells
	//---------------------------------------------------------------------------------------------------------
	imageBuffer::imageBuffer(void) {
		register i;
		
		m_Buffer =  new BufferEntryPtr[8];
		m_bufSize=8;
		for(i=0;i<m_bufSize;i++) m_Buffer[i]=NULL;
	}
	//---------------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------------
	// constructs image buffer with selected number of cells
	//---------------------------------------------------------------------------------------------------------
	imageBuffer::imageBuffer(int size) {
		register i;
		if(size < 2) size = 2;
		m_Buffer = new BufferEntryPtr [size];
		m_bufSize=size;
		for(i=0;i<m_bufSize;i++) m_Buffer[i]=NULL;
	}
	//---------------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------------
	// removes all previously selected images and image name table.
	//---------------------------------------------------------------------------------------------------------
	imageBuffer::~imageBuffer(void) {
		register i;
		
		for(i=0;i<m_bufSize;i++) {
			if(m_Buffer[i]!=NULL) {
				if(m_Buffer[i]->m_Image!=NULL) delete m_Buffer[i]->m_Image;
				if(m_Buffer[i]->m_Name!=NULL)  delete m_Buffer[i]->m_Name;
				delete m_Buffer[i];
				m_Buffer[i]=NULL;
			}
		}
		delete [] m_Buffer;
	}
	//---------------------------------------------------------------------------------------------------------
	
	
	//---------------------------------------------------------------------------------------------------------
	// loads spesified image from the resource file. If this image is already in use it will return
	// it's address and increment it's reference cound to perform LRU strategy. If this image is not
	// in the image buffer and there are free places to hold this image the function will load the 
	// image into any empty place. If there are no empty places it will first remove the image 
	// with smallest reference count. If there is no such image in the resource file this function
	// will do nothing and return NULL;
	//--------------------------------------------------------------------------------------------------------
	vimage* imageBuffer::GetImageFromResource(gString* imageName) {
		int index;
		
		// maybe image is in the buffer ?
		index = findName(imageName);
		if(index>=0) {
			if(m_Buffer[index]->m_refCount<32000)
				m_Buffer[index]->m_refCount++;
			return m_Buffer[index]->m_Image;
		}
		
		// Trying to load specified resource - doing nothing if failed.
		vresource imageResource;
		if(getResource((char *)(*imageName),&imageResource)==0) return NULL;
		
		// looking for the cell to put new image
		index = findEmptyCell(); // if there is empty cell ?
		if(index<0) { // if not, we will use LRU cell
			index= findLRUCell();
			// removing old data from the cell
			delete m_Buffer[index]->m_Image;
			delete m_Buffer[index]->m_Name;
			delete m_Buffer[index];
			m_Buffer[index]=NULL;
		}
		
		// Creating new entry in the image buffer.
		m_Buffer[index]=new BufferEntry;
		m_Buffer[index]->m_Image = new vimage;
		m_Buffer[index]->m_Name  = new gString;
		m_Buffer[index]->m_refCount=1;
		m_Buffer[index]->m_Image->Load(imageResource);
		(*m_Buffer[index]->m_Name) = *imageName;
		return 	m_Buffer[index]->m_Image;
	}
	//--------------------------------------------------------------------------------------------------------
	
	
	//--------------------------------------------------------------------------------------------------------
	// returns the index of the first empty cell or -1 if all buffer is filled
	//--------------------------------------------------------------------------------------------------------
	int imageBuffer::findEmptyCell(void) {
		register i;
		for(i=0;i<m_bufSize;i++) if(m_Buffer[i]==NULL) return i;
		return -1;
	}
	//--------------------------------------------------------------------------------------------------------
	
	//--------------------------------------------------------------------------------------------------------
	// returns the index of last recently used cell or -1 if image buffer is empty.
	//--------------------------------------------------------------------------------------------------------                
	int imageBuffer::findLRUCell(void) {
		register i;
		unsigned int minRef=32000;
		int minIndex;
		
		for(i=0;i<m_bufSize;i++) {
			if(m_Buffer[i]!=NULL && m_Buffer[i]->m_refCount<minRef) {
				minRef=m_Buffer[i]->m_refCount;
				minIndex=i;
			}
		}
		return minIndex;
	}
	//--------------------------------------------------------------------------------------------------------
	
	//--------------------------------------------------------------------------------------------------------
	// returns the index of the cell with spesified name or -1 if no such cell.
	//--------------------------------------------------------------------------------------------------------
	int imageBuffer::findName(gString* name) {
		register i;
		
		for(i=0;i<m_bufSize;i++) if(m_Buffer[i]!=NULL && *m_Buffer[i]->m_Name==*name) return i;
		return -1;
	}
	//--------------------------------------------------------------------------------------------------------
	//========================================================================================================
