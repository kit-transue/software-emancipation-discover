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
#ifndef  vportINCLUDED
#include <vport.h>
#endif

#include vtextitemHEADER
#include vlistitemHEADER
#include vstdlibHEADER
#include vlistHEADER
#include vnameHEADER
#include veventHEADER
#include vtextHEADER
#include vrectHEADER
#include vcharHEADER
#include vstrHEADER
#include vnumHEADER
#include vdragHEADER
#include vmenuHEADER
#include vresourceHEADER
#include vfsHEADER

#ifndef  vlistviewINCLUDED
#include <vlistview.h>
#endif

#include <ggenDialog.h>
#include <gString.h>
#include <gglobalFuncs.h>
#include <gAttributeEditor.h>

vkindTYPED_FULL_DEFINITION(gdAttributeEditor, vlistview, "gdAttributeEditor");
vkindTYPED_FULL_DEFINITION(AttributeEditorTextItem, vtextitem, NULL);
vkindTYPED_FULL_DEFINITION(AttributeEditorItemList, vdialogItemList, NULL);
vkindTYPED_FULL_DEFINITION(AttributePopupMenu, vmenu, "AttributePopupMenu");
vkindTYPED_FULL_DEFINITION(AttributePopupMenuItem, vmenuItem, "AttributePopupMenuItem");


// --------------------------------------------------------------------

// --------------------------------------------------------------------

void gdAttributeEditor::CommonInit()
{
    // Create floating textitem
    textitem = CreateTextItem();
    textitem->SetAutoScroll(vFALSE);
    textitem->SetOneLine(vTRUE);

    attribute_list = NULL;
    itemlist = new AttributeEditorItemList;
    itemlist->SetOwner(this);
    itemlist->Append(textitem);
    editing = vFALSE;
}

void gdAttributeEditor::LoadInit(vresource resource)
{
    vlistview::LoadInit(resource);
    CommonInit();
    InitgdWidget((vchar*)GetTag(), this);
    vlist *list = new vlist;
    SetList (list);
}

void gdAttributeEditor::Destroy()
{
    if (textitem)
	delete textitem;
    vlist *list = GetList();
    if(list)
	delete list;
    vlistview::Destroy();
}

void gdAttributeEditor::Init()
{
    // initialize inherited data
    vlistview::Init();
    CommonInit();
    InitgdWidget((vchar*)GetTag(), this);
    vlist *list = new vlist;
    SetList (list);
}

void gdAttributeEditor::Open()
{
    InitgdWidget((vchar*)GetTag(), this);
    vlistview::Open();
    vlist *list = GetList();
    list->SetExtendRowSelection (GetSelection(), vFALSE);
    list->SetSelectMethod (GetSelection(), vlistNO_SELECTION);
    list->SetSelectionImmutable(GetSelection(), vFALSE);
    SetVertBar(vTRUE);
    SetVertGrid(vTRUE);
    SetHorzGrid(vTRUE);
    SetDrawingSelection(vFALSE);

    const vrect *rect = vlistview::GetRect();
    int w = rect->GetWidth() - 25;
    SetColumnWidth(0, w / 2);
    SetColumnWidth(1, w / 2);

//    SetGrabbingHand(vTRUE);
//    SetRearrange(vlistviewROW_REARRANGE);
//    SetRowRearrange(vTRUE);
//    TriggerAcceptOnEnter(TRUE, FALSE);
//    TriggerAcceptOnReturn(TRUE, FALSE);
}

AttributeEditorTextItem *gdAttributeEditor::CreateTextItem()
{
    AttributeEditorTextItem *theTextitem = new AttributeEditorTextItem;
    theTextitem->SetParentView(this);
    return theTextitem;
}

void gdAttributeEditor::Reset()
{
    CloseEditor();
    vlist *list = GetList();
    list->SetRowCount(0);
    AttributeInfo *ptr;
    while(attribute_list != NULL){
	ptr = attribute_list->next;
	delete attribute_list;
	attribute_list = ptr;
    };
}

void gdAttributeEditor::AddAttribute(char *name, char *type, char *script)
{
    AttributeType tp = ATT_STRING;

    if(strcmp(type, "string") == 0)
	tp = ATT_STRING;
    else if(strcmp(type, "int") == 0)
	tp = ATT_INT;
    else if(strcmp(type, "enum") == 0)
	tp = ATT_ENUM;

    if(attribute_list == NULL){
	attribute_list = new AttributeInfo(name, tp, script);
    } else {
	AttributeInfo *ptr = attribute_list;
	while(ptr->next != NULL)
	    ptr = ptr->next;
	ptr->next = new AttributeInfo(name, tp, script);
    }
    UpdateList();
}

// this function will add new enum value to the existing enum attribute
void gdAttributeEditor::AddEnumAttributeValue(char *name, char *enum_val)
{
    AttributeInfo *ptr = attribute_list;
    while(ptr != NULL){
	if(strcmp(ptr->name, name) == 0){
	    if(ptr->enum_vals == NULL){
		ptr->enum_vals = new AttributeEnumVal(enum_val);
		break;
	    } else {
		AttributeEnumVal *vptr = ptr->enum_vals;
		while(vptr->next != NULL){
		    vptr = vptr->next;
		}
		vptr->next = new AttributeEnumVal(enum_val);
	    } 
	}
	ptr = ptr->next;
    };
}

void gdAttributeEditor::SetAttributeValue(char *name, char *value)
{
    vlist *list        = GetList();
    AttributeInfo *ptr = attribute_list;
    int row            = 0;
    while(ptr != NULL){
	if(strcmp(ptr->name, name) == 0){
	    ptr->value = value;
	    list->SetCellValue(row, 1, (vchar *)ptr->value);
	    break;
	}
	ptr = ptr->next;
	row++;
    };
}

void gdAttributeEditor::UpdateList(void)
{
    vlist *list        = GetList();
    int row            = 0;
    AttributeInfo *ptr = attribute_list;
    while(ptr != NULL){
	list->SetCellValue(row, 0, (vchar *)ptr->name);
	ptr = ptr->next;
	row++;
    };
    if(list->GetRowCount() == 1){
      const vrect *rect = vlistview::GetRect();
      int w             = rect->GetWidth() - 25;
      SetColumnWidth(0, w / 2);
      SetColumnWidth(1, w / 2);
    }
}

AttributeInfo *gdAttributeEditor::GetAttribute(int r)
{
    AttributeInfo *ptr = attribute_list;
    while(ptr != NULL){
	if(r == 0){
	    return ptr;
	}
	ptr = ptr->next;
	r--;
    };
    return NULL;
}

AttributeType gdAttributeEditor::GetAttributeType(int r)
{
    AttributeInfo *ptr = GetAttribute(r);
    if(ptr)
	return ptr->type;
    return ATT_STRING;
}

void gdAttributeEditor::PopupEnum(int r, vevent *event)
{
    AttributeInfo *ptr = GetAttribute(r);
    if(ptr){
	AttributePopupMenu *menu = new AttributePopupMenu;
	menu->SetAttributeEditor(this);
	AttributeEnumVal *vptr = ptr->enum_vals;
	while(vptr != NULL){
	    AttributePopupMenuItem *item = new AttributePopupMenuItem;
	    item->SetTitle((vchar *)vptr->name);
	    menu->AppendItem(item);
	    vptr = vptr->next;
	}
	int x, y;
	vwindow *vwd = DetermineWindow();
	menu->SetBackground(vwd->GetBackground());
	vwd->GetPointer(&x, &y);
	vrect rect;
	rect.x = x;
	rect.y = y;
	rect.w = 1;
	rect.h = 1;
	menu->PlaceRight(DetermineWindow(), &rect);
	menu->Popup(event);        
    }
}

int gdAttributeEditor::HandleButtonDown(vevent *event)
{
    vpoint point;
    int    r, c;
    point.x = event->GetX(); point.y = event->GetY();
    MapPointToCell(&point, &r, &c);
    if(r == -1 || c == -1)
      return vTRUE;
    int button = event->GetButton();
    if(button == 2){
    } else if(button == 0){
	int doEditing = vTRUE;
	if (HasHorzBar()){
	    vcontrol      *scrl;
	    scrl = this->hscroll;   // !!! hack alert !!!
	    if (scrl && scrl->ContainsPoint(point.x, point.y))
		doEditing = vFALSE;
	}
	if (doEditing && HasVertBar()){
	    vcontrol      *scrl;
	    scrl = this->vscroll;  // !!! hack alert !!!
	    if (scrl && scrl->ContainsPoint(point.x, point.y))
		doEditing = vFALSE;
	}
	if(GetAttributeType(r) == ATT_ENUM){
	    if(editing)
		CloseEditor();
//	    SetFocus(r, c);
//	    vrect rect;
//	    GetCellRect(r, c, &rect);
//	    DrawCellFocus(r, c, &rect);
	    
	    if(c == 1){
		edit_row = r;
		edit_col = c;
		PopupEnum(r, event);
	    }
	    return vTRUE;
	} 
	vlistview::HandleButtonDown(event);
	vrect rect;
//	GetCellRect(r, c, &rect);
//	DrawCellFocus(r, c, &rect);
	if(doEditing){
	    if(editing){
		if (textitem->GetRect()->ContainsPoint(point.x, point.y))
		    return textitem->HandleButtonDown(event);
		else { // close editor and move it to new position
		    CloseEditor();
		    edit_row = r;
		    edit_col = c;
		    CreateEditor();
		}
	    } else {
		edit_row = r;
		edit_col = c;
		CreateEditor();
	    }
	} else {
	    CloseEditor();
	    return vTRUE;
	}
    }
    return vTRUE;
}

int gdAttributeEditor::HandleButtonUp(vevent *event)
{
    if(editing){
	return textitem->HandleButtonUp(event);
    } else 
	return vlistview::HandleButtonUp(event);
}

int gdAttributeEditor::HandleKeyDown(vevent *event)
{
    int ret = vTRUE;
    
    vpoint point;
    int    r, c;
    point.x = event->GetX(); point.y = event->GetY();
    MapPointToCell(&point, &r, &c);
    if (editing){
	if(event->binding && strcmp((char *)event->binding, "Confirm") == 0){
	    CloseEditor();
	    ret = vTRUE;
	} else 
	    ret = textitem->HandleKeyDown(event);
//	if (ret)
//	    textitem->Touch(vTRUE);
    } else {
	if (event->GetBinding() != NULL)
	    ret = vlistview::HandleKeyDown(event);
    }
    return ret;
}

int gdAttributeEditor::HandleKeyUp(vevent *event)
{
    int ret = vTRUE;
    if (editing)
	ret = textitem->HandleKeyUp(event);
    else
	ret = vlistview::HandleKeyUp(event);
    return ret;
}

void gdAttributeEditor::CreateEditor()
{
    const vchar   *value;
    vrect          r;

    if(edit_col == 0)
	return;
    value = (const vchar*)GetList()->GetCellValue(edit_row, edit_col);
    textitem->SetText(value ? value : vcharGetEmptyString());
    // Resize the textitem's bounds to hold the new editing
    // value
    textitem->SelectAll();
//    if ( (editRow!=edlistINVALID_INDEX) && (editRow >= 0) &&
//	 (editCol!=edlistINVALID_INDEX) && (editCol >= 0)){
    GetCellRect(edit_row, edit_col, &r);
    int inset = (int)GetDefaultSelectionInset();
    r.Inset(&r, -inset, -inset);
//    }
    textitem->SetRect(&r);
    
    // Make sure the listview's focus it set to the edit cell
    SetFocus(edit_row, edit_col);
    
    // Black magic to display the "floating" textitem
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

void gdAttributeEditor::SetRect(const vrect *rect)
{
    CloseEditor();
    vlistview::SetRect(rect);
    int w = rect->GetWidth() - 25;
    SetColumnWidth(0, w / 2);
    SetColumnWidth(1, w / 2);
}

void gdAttributeEditor::CloseEditor()
{
    if(editing){
	// Hide text
	editing   = FALSE;
	vstr *txt = textitem->GetTextAsString();
	PropagateItemChange(edit_row, (char *)txt);
	vstrDestroy(txt);
	textitem->Hide();
	// Undo the black magic
	this->itemlist->SetFocusItem(NULL);
	textitem->Close();
	this->itemlist->Close();
	this->itemlist->Deactivate();
    }
}

void gdAttributeEditor::PropagateItemChange(int row, char *txt)
{
    vlist *list        = GetList();
    AttributeInfo *ptr = attribute_list;
    int pos            = 0;
    while(ptr != NULL){
	if(pos == row){
	    ptr->value = txt;
	    list->SetCellValue(row, 1, (vchar *)ptr->value);
	    gString cmd;
	    cmd.sprintf((vchar *)"%s {%s} {%s}", (char *)ptr->script, (char *)ptr->name, (char *)ptr->value);
            Tcl_Interp* i = gdWidget::interp();
            gala_eval(i, (char*)(vchar*)cmd);
	    break;
	}
	ptr = ptr->next;
	pos++;
    };
}

void gdAttributeEditor::SetEnumValue(char *txt)
{
    PropagateItemChange(edit_row, txt);
}

void gdAttributeEditor::Draw()
{
    // Draw the listview first!
    vlistview::Draw();
    // If editing draw the textitem
    if (editing){
	vdrawGSave();
	Clip();
	itemlist->Draw();
	vdrawGRestore();
    }
}

// --------------------------------------------------------------------

vdialog *AttributeEditorItemList::DetermineDialog()
{
  return(((gdAttributeEditor*)this->GetOwner())->DetermineDialog());
}

vcursor *AttributeEditorItemList::DetermineCursor()
{
  return(((gdAttributeEditor*)this->GetOwner())->DetermineCursor());
}

vfont *AttributeEditorItemList::DetermineFont()
{
  return(((gdAttributeEditor*)this->GetOwner())->DetermineFont());
}

void AttributeEditorItemList::DetermineColorsFrom(long theFlags,
                                           vcolor **known,
                                         int *ids, int count, vcolor **colors)
{
  ((gdAttributeEditor*)this->GetOwner())->DetermineColorsFrom(theFlags,
                                                      known,
                                                      ids,
                                                      count,
                                                      colors);
}

void AttributeEditorItemList::DetermineColorSchemeFrom(vcolorScheme *known,
                                                unsigned long myflags,
                                                vcolorScheme *scheme)
{
  ((gdAttributeEditor*)this->GetOwner())->DetermineColorSchemeFrom(known, myflags,
								   scheme);
}

int AttributeEditorItemList::DetermineLineWidth()
{
    return(((gdAttributeEditor*)this->GetOwner())->DetermineLineWidth());
}

int AttributeEditorItemList::DetermineShown()
{
    return(((gdAttributeEditor*)this->GetOwner())->DetermineShown());
}

int AttributeEditorItemList::DetermineEnabled()
{
    return(((gdAttributeEditor*)this->GetOwner())->DetermineEnabled());
}

void AttributeEditorItemList::DetermineBounds(register vrect *r)
{
    ((gdAttributeEditor*)this->GetOwner())->GetItemListIn()->DetermineBounds(r);
}

void AttributeEditorItemList::DetermineContent(vrect *r)
{
    ((gdAttributeEditor*)this->GetOwner())->GetItemListIn()->DetermineContent(r);
}

vdict *AttributeEditorItemList::GetKeyMap()
{
    return(((gdAttributeEditor*)this->GetOwner())->GetKeyMap());
}

vdict *AttributeEditorItemList::GetKeyDispatch()
{
    return(((gdAttributeEditor*)this->GetOwner())->GetKeyDispatch());
}

void AttributeEditorItemList::DrawDevice()
{
    ((gdAttributeEditor*)this->GetOwner())->DrawDevice();
}

// --------------------------------------------------------------------

void AttributePopupMenuItem::ObserveMenuItem(vevent *event)
{
    vmenuItem::ObserveMenuItem(event);
    AttributePopupMenu *menu = DetermineMenu();
    gdAttributeEditor *ae    = menu->GetAttributeEditor();
    char *itemtitle          = (char *)GetTitle();
    ae->SetEnumValue(itemtitle);
    menu->DeleteLater();
}

AttributePopupMenu *AttributePopupMenuItem::DetermineMenu()
{
   return (AttributePopupMenu *)vmenuItem::DetermineMenu();
}

// --------------------------------------------------------------------

AttributeInfo::~AttributeInfo()
{
  AttributeEnumVal *ptr = enum_vals;
  while(ptr != NULL){
    enum_vals = enum_vals->next;
    delete ptr;
    ptr = enum_vals;
  }
}

// --------------------------------------------------------------------

void InitAttributeEditor(gdAttributeEditor *ae)
{
    vlist *list;

    list = ae->GetList();
    list->SetRowCount(0);
    list->SetColumnCount(2);
    ae->SetColumnWidth(0, 90); 
    ae->SetColumnWidth(1, 90);
}
