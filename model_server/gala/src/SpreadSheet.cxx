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

#include "gSpreadSheet.h"
#include "gdialog.h"
#include "ginterp.h"
#include <cLibraryFunctions.h>

inline int ABS(int x) { return x > 0 ? x : -x; }
#define FILT_VER_STR "@@filter_version_2"
#define MAX_LEN  1024

typedef struct {
  int  row;
  int  criteria;
  char basestring[1024];
  int  sensitive;
  int  entities;
  int  strings;
} PatternClipBoard;

static PatternClipBoard p_clipboard[200];
static int PatternClipBoard_num = 0;

typedef struct {
  int  row;
  int  criteria;
  char type[1024];
  char size[1024];
} TypeClipBoard;

static TypeClipBoard t_clipboard[200];
static int TypeClipBoard_num = 0;

char *PopupList[] = { "Cut", "Copy", "Paste", "Insert", "Delete" };
int Popup_num = 5;

extern vresource systemResources;

vkindTYPED_FULL_DEFINITION(gdSpreadSheet, vlistview, "gdSpreadSheet");


void InitSpreadSheet(gdSpreadSheet *ssheet, int cols)
{
    vlist *list;
    int       j;

  list = ssheet->GetList();

  list->SetRowCount(1);
  list->SetColumnCount(cols);

  ssheet->InsertOneEmptyRow(0, 0);

#ifndef _WIN32
  for(j = 0; j < cols; j++) 
     if( j == 0) {
        if(cols == 4) ssheet->SetColumnWidth(j, 28);
        else if(cols ==6) ssheet->SetColumnWidth(j, 28);
     } else if(j == 1) {
        if(cols == 4) ssheet->SetColumnWidth(j, 94);
        else if(cols == 6) ssheet->SetColumnWidth(j, 99);
     } else if(j == 2) {
        if(cols == 4) ssheet->SetColumnWidth(j, 129);
        else if(cols == 6) ssheet->SetColumnWidth(j, 234);
     } else if(cols == 4 && j == 3)
           ssheet->SetColumnWidth(j, 100);
     else
        ssheet->SetColumnWidth(j, 99);
#else
  for(j = 0; j < cols; j++)
     if( j == 0) {
        ssheet->SetColumnWidth(j, 30);
     } else if(j == 1) {
        ssheet->SetColumnWidth(j, 82);
     } else if(j == 2) {
        if(cols == 4) ssheet->SetColumnWidth(j, 99);
        else if(cols == 6) ssheet->SetColumnWidth(j, 139);
     } else 
        ssheet->SetColumnWidth(j, 94);
#endif
  ssheet->SetRowHeight(vlistviewALL_ROWS, 20);

}    

void gdSpreadSheet::CommonInit()
{
  // Blank attributes
  editRow    = editCol = vlistNO_CELLS_SELECTED;
  lastX      = lastY = -1;
  textitem   = NULL;
  itemlist   = NULL;
  editing    = FALSE;
  krtn       = FALSE;
  kent       = FALSE;
  conf       = FALSE;
  inside     = TRUE;
  modified   = 0;


  // Create floating textitem
  textitem = CreateTextItem();
  textitem->SetAutoScroll(vFALSE);
  textitem->SetOneLine(vTRUE);

  itemlist = new SpreadSheetItemList;
  itemlist->SetOwner(this);
  itemlist->Append(textitem);
}

void gdSpreadSheet::Init()
{
  // initialize inherited data
  vlistview::Init();

  CommonInit();
  InitgdWidget((vchar*)GetTag(), this);
  vlist *list = new vlist;
  SetList (list);
}

void gdSpreadSheet::LoadInit(vresource resource)
{
  // initialize inherited data
  vlistview::LoadInit(resource);

  CommonInit();
  InitgdWidget((vchar*)GetTag(), this);
  vlist *list = new vlist;
  SetList (list);
}


void gdSpreadSheet::Open()
{
    vlistview::Open();
    InitgdWidget((vchar*)GetTag(), this);

    vlist *list = GetList();
    list->SetExtendRowSelection (GetSelection(), vTRUE);
    list->SetSelectMethod (GetSelection(), vlistRECTANGULAR_SELECTION);
    list->SetSelectionImmutable(GetSelection(), vFALSE);
    SetVertBar(TRUE);
    SetDrawingSelection(vFALSE);
    SetRearrange(vlistviewROW_REARRANGE);
    SetRowRearrange(vTRUE);
    TriggerAcceptOnEnter(TRUE, FALSE);
    TriggerAcceptOnReturn(TRUE, FALSE);
}

void gdSpreadSheet::Destroy()
{
    if(itemlist) {
    itemlist->Delete(textitem);
    delete itemlist;
    itemlist = NULL;
  }

  if (textitem) {
    delete textitem;
    textitem = NULL;
  }

  vlist *list = GetList();
  if(list)
     delete list;

  vlistview::Destroy();
}


int gdSpreadSheet::GetSelectedCell(int *row, int *col)
{
  int    gotOne;

  // Gotta have a vlist attached...
  vlist *list = GetList();
  if (!list)
     return FALSE;


  // This only works for a single cell selection
  int n = list->CountCells(GetSelection());
  if (n==1)
  {
     list->GetSelectedCell(GetSelection(), row, col);
     gotOne = TRUE;
  } else {
      *row = vlistNO_CELLS_SELECTED;
      *col = vlistNO_CELLS_SELECTED;
      gotOne = FALSE;
  }

  return gotOne;
}

void gdSpreadSheet::CleanUp()
{
  InsertOneEmptyRow(0,1);
  int row = GetList()->GetRowCount();

  if(row <= 1) return;

  GetList()->RemoveRange(1, row-1, vlistROW);
}


void gdSpreadSheet::AcceptEdits()
{
  const vchar *input;

  if (IsEditing())
    {
      // Store the current display text to the list
      if (textitem->IsDirty())
      {
          input = textitem->ToBuffer();
          PutDownValue(input);
          textitem->Touch(FALSE);
      }

      // Finish editing cell

      StopEdit();
    }
}

void gdSpreadSheet::RejectEdits()
{
  // Phffht

  if (IsEditing())
  {
     StopEdit();
     textitem->Touch(FALSE);
  }
}


int gdSpreadSheet::CanQueryFocusAsTarget()
{
  int   ok = FALSE;

  vlist *list = GetList();
  vlistSelection *theSelection = GetSelection();

  switch (list->GetSelectMethod(theSelection))
    {
      case vlistONE_ONLY_SELECTION:
        {
          ok = TRUE;
          break;
        }
      case vlistFREE_SELECTION:
      case vlistRECTANGULAR_SELECTION:
        {
          ok = list->CountCells(theSelection) == 1;
          break;
        }
    }
  return ok;
}



SpreadSheetTextItem *gdSpreadSheet::CreateTextItem()
{
  SpreadSheetTextItem *theTextitem = new SpreadSheetTextItem;
  theTextitem->SetParentView(this);
  return theTextitem;
}


const vchar* gdSpreadSheet::PickUpValue()
{
  const vchar *str;

  if ( (editRow != vlistNO_CELLS_SELECTED) &&
       (editCol != vlistNO_CELLS_SELECTED) )

    // !!!
    // Warning: we assume the contents of the list's cells
    // are character pointers
    // !!!
  
    str = (const vchar*)GetList()->GetCellValue(editRow, editCol);
  else
    str = NULL;

  return str;
}


void gdSpreadSheet::PutDownValue(const vchar *value)
{
  vstr *str;

  if ( (editRow != vlistNO_CELLS_SELECTED) &&
       (editCol != vlistNO_CELLS_SELECTED) )
    {
      // !!!
      // Warning: we assume the contents of the list's cells
      // are dynamic string pointers (vstrs)
      // !!!

      str = (vstr*)GetList()->GetCellValue(editRow, editCol);
      if (str)
        vstrDestroy(str);

      str = value ? vstrClone(value) : NULL;

      GetList()->SetCellValue(editRow, editCol, str);
    }
}


int gdSpreadSheet::EditCell(int row, int col)
{
  int            ok=vTRUE;

  // If we're editing something, validate the current value
  // and commit changes

  if (IsEditing())
    {
        AcceptEdits();
    }


  // Move to cell if can

   GetList()->SelectCell(GetSelection(), row, col);
   editRow = row, editCol = col;
   Edit(TRUE);

  // done

  return ok;
}


int gdSpreadSheet::LoseIfOutsideScrolls(vevent *event)
{
  vcontrol      *scrl;


  int doFinishEditing = TRUE;

  int x = event->GetX();
  int y = event->GetY();

  if (HasHorzBar())
    {
      scrl = this->hscroll;   // !!! hack alert !!!
      if (scrl && scrl->ContainsPoint(x, y))
        doFinishEditing = FALSE;
    }

  if (doFinishEditing && HasVertBar())
    {
      scrl = this->vscroll;  // !!! hack alert !!!
      if (scrl && scrl->ContainsPoint(x, y))
        doFinishEditing = FALSE;
    }


  return doFinishEditing;
}

int gdSpreadSheet::TriggerAccept(vevent *event)
{
  int           tryIt;
  vkey          key;
  vkeyModifiers mkey;

  #define isMod(m) ((m) == vkeyMODIFIER_KEY_CTRL)

  tryIt = FALSE;

  if (krtn || kent)
    {
      key  = vkeyGetStrokeKey(event->GetKeyStroke());
      mkey = vkeyGetStrokeModifiers(event->GetKeyStroke());

      if ( krtn      &&
           key==vkeyRETURN &&
          (!conf || (conf && isMod(mkey))) )

        tryIt = TRUE;

      else
      if  ( kent     &&
           key==vkeyENTER  &&
          (!conf || (conf && isMod(mkey))) )

        tryIt = TRUE;

      else
      if  ( kent     &&
           key==vkeyENTER  &&
          (!conf || (conf && isMod(mkey))) )

        tryIt = TRUE;

    }

  #undef isMod
  return tryIt;
}

int gdSpreadSheet::HandleButtonDown(vevent *event)
{
  int ret = TRUE;
  int keepActive = FALSE;

  if (LoseIfOutsideScrolls(event))  {
     vpoint        point;

     point.x = event->GetX(); point.y = event->GetY();

     if (!GetRect()->ContainsPoint(point.GetX(), point.GetY())) 
         return  vlistview::HandleButtonDown(event);

     int           r, c;
     MapPointToCell(&point, &r, &c);

     // If this is drag, remember the start point of the drag 

     if (event->GetBinding() == vname_Drag) {
         startPt.Set(event->GetX(), event->GetY()); //get start point in vlistview coordinate system
         int x, y;
         vwindow *wd = DetermineWindow();
         wd->GetPointer(&x, &y);  //get start point in window system
         startPt_wd.Set(x,y);

         vwindow::SyncEvents();
         return (vTRUE);
     }

     // If right mouse button is pressed (button = 2), popup menu comes out

     int button = event->GetButton();
     if(button == 2){
        if((c == 0) && (GetList()->IsCellSelected(GetSelection(), r, c))) {
           MyPopupMenu *menu = new MyPopupMenu;
           menu->SetSpreadSheet(this);
           MyPopupMenuItem *item;
           int i;
           for(i = 0; i < Popup_num; i++) {
             item = new MyPopupMenuItem;
             item->SetTitle((vchar *)PopupList[i]);
             menu->AppendItem(item);
           }
           int x,y;
           vwindow *vwd = DetermineWindow();
           menu->SetBackground(vwd->GetBackground());
           vwd->GetPointer(&x, &y);
           vrect myrect;
           myrect.x = x;
           myrect.y = y;
           myrect.w = 1;
           myrect.h = 1;
           menu->PlaceRight(DetermineWindow(), &myrect);
           menu->Popup(event);        
        }
        ret = vlistview::HandleButtonDown(event);
        return ret;
     }

     GetList()->UnselectAll(GetSelection());
     if(c == 0) {  // If the first column is selected, the whole row will be selected as well.
        GetList()->SelectRow(GetSelection(), r);

        ret = vlistview::HandleButtonDown(event);
        return ret;
     }
     int col = GetList()->GetColumnCount();
     if(col == 6 && c!= 2) {  // Column 1, 3, 4 and 5 are two value toggle
        const vchar *str;

        str = (const vchar*)GetList()->GetCellValue(r,c);
 
        if (strcmp((char *)str, "")==0) { // This is the first time that this row is selected and  
                                          // column 1, 3, 4 and 5 should be initialized with default value.
           str = vstrCloneScribed (vcharScribeLiteral("Include"));
           GetList()->SetCellValue(r,1, str);
           str = vstrCloneScribed (vcharScribeLiteral("Yes"));
           GetList()->SetCellValue(r, 3, str);
           GetList()->SetCellValue(r, 4, str);
           GetList()->SetCellValue(r, 5, str);

           AcceptEdits();
           editRow = r;  editCol = 2; // Set editing focus on column 2 
           Edit(TRUE);
        } else {
           if(strcmp((char *)str, "Include")==0)
              str = vstrCloneScribed (vcharScribeLiteral("Exclude"));
           else if (strcmp((char *)str, "Exclude")==0)
              str = vstrCloneScribed (vcharScribeLiteral("Include"));
           else if (strcmp((char *)str, "Yes")==0)
              str = vstrCloneScribed (vcharScribeLiteral("No"));
           else if (strcmp((char *)str, "No")==0)
              str = vstrCloneScribed (vcharScribeLiteral("Yes"));
           GetList()->SetCellValue(r,c, str);
        }
        SetModified(1);   // This spreadsheet has been modified.
        return ret;
     } else if (col == 4 && c == 1) {
        const vchar *str;

        str = (const vchar*)GetList()->GetCellValue(r,c);

        if (strcmp((char *)str, "")==0) { //This is the first time that this row is selected and
                                          //column 1 should be initialized with default value.
           str = vstrCloneScribed (vcharScribeLiteral("Include"));
           GetList()->SetCellValue(r,c, str);
           AcceptEdits();
           editRow = r;  editCol = 2;  // Set editing focus on column 2
           Edit(TRUE);
           SetModified(1);
           return ret;
        }
        if(strcmp((char *)str, "Include")==0)
           str = vstrCloneScribed (vcharScribeLiteral("Exclude"));
        else if (strcmp((char *)str, "Exclude")==0)
           str = vstrCloneScribed (vcharScribeLiteral("Include"));

        GetList()->SetCellValue(r,c, str);
        SetModified(1);
        return ret;
     } else if (col == 6 && c == 2) {  // This is the first time that this row is selected.
        const vchar *str;
        str = (const vchar*)GetList()->GetCellValue(r,1);
        if(strcmp((char *)str, "")==0) {
           str = vstrCloneScribed (vcharScribeLiteral("Include"));
           GetList()->SetCellValue(r,1, str);
           str = vstrCloneScribed (vcharScribeLiteral("Yes"));
           GetList()->SetCellValue(r, 3, str);
           GetList()->SetCellValue(r, 4, str);
           GetList()->SetCellValue(r, 5, str);
           SetModified(1);
        } 
    } else if (col == 4 && (c == 2 || c == 3)) { // This is the first time that this row is selected.
        const vchar *str;
        str = (const vchar*)GetList()->GetCellValue(r,1);
        if (strcmp((char *)str, "")==0) {
           str = vstrCloneScribed (vcharScribeLiteral("Include"));
           GetList()->SetCellValue(r,1, str);
           SetModified(1);
        }
    }   
   }

  // If already editing pass the event to the textitem, else
  // do some "special case" processing, else send to listview

  if (IsEditing())
    {
      if (GetTextItem()->GetRect()->ContainsPoint(event->GetX(),
                                                  event->GetY()))
        {

          this->itemlist->SetDragItem(textitem);

          ret = textitem->HandleButtonDown(event);
        }
      else
      if (event->GetBinding() != vname_Grab)
        {
          if (LoseIfOutsideScrolls(event))
                AcceptEdits();
          else
            keepActive = TRUE;
        }
    }


  // If not editing pass along automatically pass to inherited. Remember
  // where the button down took place st. if the button up occurs within
  // a "slop" square area, we can begin editing the underlying cell.

  if (!IsEditing() || keepActive)
    {

      lastX = event->GetX();
      lastY = event->GetY();

      ret = vlistview::HandleButtonDown(event);
    }

  // finished

  return ret;
}

int gdSpreadSheet::HandleButtonUp(vevent *event)
{
  int ret = TRUE;

  if (LoseIfOutsideScrolls(event))  {

     if (event->GetBinding() == vname_Drag)
        return ret;

     vpoint        point;
     int           r, c;

     point.x = event->GetX(); point.y = event->GetY();

     MapPointToCell(&point, &r, &c);

     if(r == (GetList()->GetRowCount()-1)) // Every time the last row is selected, a new row will be
        InsertOneEmptyRow(r+1, 1);         // automatically appended to the list.

     int col = GetList()->GetColumnCount();
     if((col == 6 && c!=2) || (col == 4 && c != 2 && c != 3)) { // If it is not editing column, just return
        vlistview::HandleButtonUp(event);
        return ret;
     }
  }

  // If editing, pass along to textitem

  if (IsEditing())
    {
      ret = textitem->HandleButtonUp(event);
    }

  // Else pass along to listview

  else
    {
      ret = vlistview::HandleButtonUp(event);

      // If within a "slop" area of the last button down, begin
      // editing the cell underneath -- iff the cell is editable!
      if (   (lastX > event->GetX()-editingSLOP)
          && (lastX < event->GetX()+editingSLOP)
          && (lastY > event->GetY()-editingSLOP)
          && (lastY < event->GetY()+editingSLOP)
          && event->GetBinding() != vname_Grab)
        {
          vpoint        point;
          int           r, c;

          point.x = lastX, point.y = lastY;
          MapPointToCell(&point, &r, &c);

          if ( (event->GetBinding()==vname_Select) ||
               (event->GetBinding()==vname_Adjust &&
                IsSelectedCell(r, c)) ) {
             editRow = r, editCol = c;
             Edit(vTRUE);
          }
        }
    }


  // Done

  return ret;
}


int gdSpreadSheet::HandleKeyDown(vevent *event)
{
  int ret = TRUE;

  vpoint        point;
  int           r, c;

  point.x = event->GetX(); point.y = event->GetY();
  MapPointToCell(&point, &r, &c);

  int col = GetList()->GetColumnCount();
  if((col == 6 && c!=2) || (col == 4 && c != 2 && c != 3)) 
     return ret;

  // Pass along...

  if(IsEditing()) {
     ret = textitem->HandleKeyDown(event);
     if (ret)
        textitem->Touch(TRUE);

     if ( !ret && TriggerAccept(event))
        AcceptEdits();
  } else if (event->GetBinding()!=NULL)
     ret = vlistview::HandleKeyDown(event);

  // Done

  return ret;
}


int gdSpreadSheet::HandleKeyUp(vevent *event)
{
  int ret = TRUE;

  vpoint        point;
  int           r, c;

  point.x = event->GetX(); point.y = event->GetY();
  MapPointToCell(&point, &r, &c);

  int col = GetList()->GetColumnCount();
  if((col == 6 && c!=2) || (col == 4 && c != 2 && c != 3))
     return ret;

  // Textitem or Listview...to be or not to be

  if (IsEditing())
      ret = textitem->HandleKeyUp(event);
  else
      ret = vlistview::HandleKeyUp(event);

  // Done

  return ret;
}

int gdSpreadSheet::HandleDrag(vevent *event)
{
    // If binding is not drag then let super class handle it
    if (event->GetBinding() != vname_Drag)
        return vlistview::HandleDrag(event);

    int r,c;
    vpoint        point;
    point.x = event->GetX(); point.y = event->GetY();
    MapPointToCell(&point, &r, &c);

    // If we are dragging and break hysterisis then start drag
    int row, column, numDown, numAcross;
    GetList()->DetermineSelectionBounds(GetSelection(), &row, &column, &numDown, &numAcross);
    int height = GetRowHeight(row);
    vpoint pt;
    GetStartDragPt(&pt);
    if ((row != -1) && (ABS(event->GetY() - pt.GetY()) > height/4))
    {
        // Get list cell rectangle
        vrect* myrect = new vrect;
        vlistSelection *myselection = GetSelection();
        vrectLong *longrt = myselection->rects;
        myrect->SetX(longrt->GetX());
        myrect->SetY(longrt->GetY());
        myrect->SetWidth(longrt->GetWidth());
        myrect->SetHeight(longrt->GetHeight());

        // Create drag and initialize
        MyDrag *drag = new MyDrag;
        drag->SetSpreadSheet(this);
        drag->SetMaxSize(myrect->GetWidth(), myrect->GetHeight());

        // Load cursors from vr file
        vresource res;
        vcursor  *goodCursor, *badCursor;
        if(getResource(systemResources, "Cursors", &res)) {
           goodCursor = new vcursor(vresourceGet(res, vnameInternGlobalLiteral("GoodDrop")));
           badCursor = new vcursor(vresourceGet(res, vnameInternGlobalLiteral("BadDrop")));
        } else {
           goodCursor = badCursor = NULL;
        }

        // Set up cursors
        drag->SetOriginalCursor(DetermineWindow()->DetermineCursor());
        drag->SetGoodCursor(goodCursor);
        drag->SetBadCursor(badCursor);

        // Start drag
        drag->SetOffset(event->GetX() - myrect->GetX(), event->GetY() - myrect->GetY());
        drag->Start(event);
    }

    return (vTRUE);
}

int gdSpreadSheet::IssueSelector(int selector, vdict *context)
{
  int   ret = TRUE, check;
  int   row, col;


  // Remember which (if any) cell we're editing

  int prev_row = editRow; int prev_col = editCol;


  // Do what we must do for selector - we're not interested in changing
  // the default class behavior for selectors

  ret = vlistview::IssueSelector(selector, context);

  // BUT, if the selector involved movement of the selected cell, we will
  // switch to edit mode if we can

  check = ret;

  if (check)
    switch (selector)
      {
        case vdialogITEM_SELECT_RIGHT:
        case vdialogITEM_SELECT_LEFT:
        case vdialogITEM_SELECT_UP:
        case vdialogITEM_SELECT_PAGE_LEFT:
        case vdialogITEM_SELECT_PAGE_RIGHT:
        case vdialogITEM_SELECT_PAGE_UP:
        case vdialogITEM_SELECT_PAGE_DOWN:
        case vdialogITEM_SELECT_BEGIN:
        case vdialogITEM_SELECT_END:
          {
            if (CanQueryFocusAsTarget())
              {
                GetFocus(&row, &col);
                if ( (row != vlistNO_CELLS_SELECTED)  &&
                     (col != vlistNO_CELLS_SELECTED)  &&
                     (row != prev_row || col != prev_col) )
                  //   CanEditCell(row, col) )
                  {
                    editRow = row, editCol = col;
                    Edit(TRUE);
                  }
              }/*can-use-focus*/
          }/*move-selector*/
      }/*switch*/

  return ret;
}

void gdSpreadSheet::Focus()
{
  if (IsEditing())
    this->itemlist->Focus();

  else
    vlistview::Focus();
}



void gdSpreadSheet::UnFocus()
{
  if (IsEditing())
    this->itemlist->Unfocus();

  else
    vlistview::Unfocus();
}


void gdSpreadSheet::Draw()
{

  // Draw the listview first!

  vlistview::Draw();


  // If editing draw the textitem

  if (IsEditing() && EditingHappensInside())
    {
      vdrawGSave();
      Clip();
      itemlist->Draw();
      vdrawGRestore();
    }
}

void gdSpreadSheet::ForceDrawTextItem()
{
  if (IsEditing())
    {
      vdrawGSave();
      if (EditingHappensInside())
        {
          Clip();
        }
      else
        {
          vrect r;
          r = *(GetTextItem()->GetRect());
          vdrawRectClip(r.x, r.y+10, r.w, r.h);
        }
      itemlist->Draw();
      vdrawGRestore();
    }
}

void gdSpreadSheet::DetermineEditingRect(vrect *r)
{
  int inset;

  *r = *vrectNull();

  if ( (editRow!=vlistNO_CELLS_SELECTED) && (editRow >= 0) &&
       (editCol!=vlistNO_CELLS_SELECTED) && (editCol >= 0))
    {
      GetCellRect(editRow, editCol, r);
      inset = (int)GetDefaultSelectionInset();
      r->Inset(r, -inset, -inset);
    }
}

void gdSpreadSheet::SetRect(const vrect *theRect)
{
  vrect r;
  int   oldH, newH;

  r = *GetRect();

  vlistview::SetRect(theRect);


  // Adjust the floating textitem to the new listview
  // dimensions -- we know this is called by vdialogArrange (ala
  // the itemlist.

  if (IsEditing() &&
      !r.IsEqual(GetRect()))
    {

      oldH = r.h;
      newH = GetHeight();

      DetermineEditingRect(&r);

      if(textitem)
	  textitem->SetRect((const vrect *)&r);
    }

}

int gdSpreadSheet::DetermineMinRowH()
{
  int    w, rowheight;

  // Assuming "stringized" cell contents, calculate the minimum height
  // as the textitem's natural height _minus_ the laf-dependent border
  // dimension

  textitem->GetNaturalSize(&w, &rowheight);
  rowheight -= 2*GetDefaultSelectionInset();
  return rowheight;
}

void gdSpreadSheet::ObserveScroll(int h, int v)
{
  vlistview::ObserveScroll(h, v);

  if (IsEditing())
    {
      vrect r;
      DetermineEditingRect(&r);

      if (EditingHappensInside())
        r.Offset(&r, h, v);

      textitem->SetRect(&r);
    }
}


void gdSpreadSheet::DrawBackground(int r, int c, int selected, vrect *theRect)
{
  if ( !IsEditing() ||
       !EditingHappensInside() ||
      (r != editRow) || (c != editCol) )

    vlistview::DrawCellBackground(r,c, selected, theRect);
}

void gdSpreadSheet::DrawSelection(int r, int c, vrect *theRect)
{
  if ( !IsEditing() ||
       !EditingHappensInside() ||
       (r != editRow) || (c != editCol) )

    vlistview::DrawCellSelection(r, c, theRect);
}

void gdSpreadSheet::Edit(int turnOn)
{
  const vchar   *value;
  vrect          r;

  // If turn on editing

  if ( turnOn && !IsEditing() )
    {
      // Load in the current value in displayable string
      // format

      value = PickUpValue();
      textitem->SetText(value ? value : vcharGetEmptyString());

      // Resize the textitem's bounds to hold the new editing
      // value

      textitem->SelectAll();
      DetermineEditingRect(&r);
      textitem->SetRect(&r);

      // Make sure the listview's focus it set to the edit cell

      SetFocus(editRow, editCol);

      // Black magic to display the "floating" textitem
      this->itemlist->Open();
      this->itemlist->Activate();
      textitem->Hide();
      textitem->Show();
      this->itemlist->SetDragItem(textitem);
      GetItemListIn()->SetFocusItem(this);
      this->itemlist->Focus();
      this->itemlist->SetFocusItem(textitem);

      // Show text

      editing = TRUE;
      textitem->Open();

    }

  // If turn off editing

  else
  if ( !turnOn && IsEditing() )
    {
      // Hide text
      editing = FALSE;
      textitem->Hide();

      // Undo the black magic
      this->itemlist->SetFocusItem(NULL);
      textitem->Close();
      this->itemlist->Close();
      this->itemlist->Deactivate();

    }

  return;
}


int gdSpreadSheet::CellIsCompletelyVisible(int row, int col)
{
  vrect viewrect, cellrect;

  viewrect = *(GetListRect());
  GetCellRect(row, col, &cellrect);
  return (viewrect.InlineEncloses(&cellrect));
}

int gdSpreadSheet::Hasmodified()
{
  int m = textitem->IsDirty();
  m = modified || m;
  return m;
}


void gdSpreadSheet::ClearEditingText()
{
  textitem->SetText(vcharGetEmptyString());
}


char *gdSpreadSheet::GetEditingValue(int row, int col)
{
     char *value;
     if((editRow == row) && (editCol == col) && IsEditing()) {
        vchar *tmp = textitem->ToBuffer();
        value = (tmp && *tmp != vcharNULL)? (char *)tmp:(char *)NULL;
     } else
        value = (char *)GetList()->GetCellValue(row, col);
     return value;
}

void gdSpreadSheet::SetEditingCellValue(int row, int col, vstr *val)
{
     if(row == editRow && col == editCol && IsEditing())
        textitem->SetText(val? (const vchar *)val : vcharGetEmptyString());
     else
        GetList()->SetCellValue(row, col, val);
}

void gdSpreadSheet::InvalFirstCol()
{
    int total_r = GetList()->GetRowCount();
    int i;
    vstr *str;
    for( i = 0; i < total_r; i++) {
       char ss[100];
       OSapi_sprintf(ss, "%d", i+1);
       str = vstrCloneScribed (vcharScribeLiteral(ss));
       GetList()->SetCellValue(i, 0, str);
    }
}

void gdSpreadSheet::InsertOneEmptyRow(int row, int flag)
{
     if(flag)
        GetList()->InsertRow(row);

     int total_c = GetList()->GetColumnCount();
     int i;
     vstr *str;
     for( i = 0; i < total_c; i++) {
           char ss[1024];
           if(i == 0)
                OSapi_sprintf(ss, "%d", row+1);
           else
                strcpy(ss, "");
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           GetList()->SetCellValue(row, i, str);
     }
     SetRowHeight(row, 20);
}

//============ The end of gdSpreadSheet =======================================

//*****************************************************************************
//* SpreadSheetItemList
//*****************************************************************************

// overrides from vkindTyped
vkindTYPED_FULL_DEFINITION(SpreadSheetItemList, vdialogItemList, NULL);

vdialog *SpreadSheetItemList::DetermineDialog()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineDialog());
}


vcursor *SpreadSheetItemList::DetermineCursor()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineCursor());
}


vfont *SpreadSheetItemList::DetermineFont()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineFont());
}


void SpreadSheetItemList::DetermineColorsFrom(long theFlags,
                                           vcolor **known,
                                         int *ids, int count, vcolor **colors)
{
  ((gdSpreadSheet *)this->GetOwner())->DetermineColorsFrom(theFlags,
                                                      known,
                                                      ids,
                                                      count,
                                                      colors);
}


void SpreadSheetItemList::DetermineColorSchemeFrom(vcolorScheme *known,
                                                unsigned long myflags,
                                                vcolorScheme *scheme)
{
  ((gdSpreadSheet *)this->GetOwner())->DetermineColorSchemeFrom(known, myflags,
                                                           scheme);
}


int SpreadSheetItemList::DetermineLineWidth()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineLineWidth());
}

int SpreadSheetItemList::DetermineShown()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineShown());
}


int SpreadSheetItemList::DetermineEnabled()
{
  return(((gdSpreadSheet *)this->GetOwner())->DetermineEnabled());
}


void SpreadSheetItemList::DetermineBounds(register vrect *r)
{
  ((gdSpreadSheet *)this->GetOwner())->GetItemListIn()->DetermineBounds(r);
}


void SpreadSheetItemList::DetermineContent(vrect *r)
{
  ((gdSpreadSheet *)this->GetOwner())->GetItemListIn()->DetermineContent(r);
}


vdict *SpreadSheetItemList::GetKeyMap()
{
  return(((gdSpreadSheet *)this->GetOwner())->GetKeyMap());
}

vdict *SpreadSheetItemList::GetKeyDispatch()
{
  return(((gdSpreadSheet *)this->GetOwner())->GetKeyDispatch());
}

void SpreadSheetItemList::DrawDevice()
{
  ((gdSpreadSheet *)this->GetOwner())->DrawDevice();
}

//============= The end of SpreadSheetItemList ================================

//*****************************************************************************
//* SpreadSheetTextItem
//*****************************************************************************

// overrides from vkindTyped
vkindTYPED_FULL_DEFINITION(SpreadSheetTextItem, vtextitem, NULL);


void SpreadSheetTextItem::CommonInit()
{
  this->parentview = NULL;
  this->dirty      = FALSE;
  this->allowAny   = FALSE;

  SetOneLine(TRUE);
  SetEditable();
}

void SpreadSheetTextItem::Copy(vloadable *original)
{
  vtextitem::Copy(original);

  this->parentview = NULL;
}


void SpreadSheetTextItem::Touch(int dirtyBool)
{
  dirty = dirtyBool;
}


void SpreadSheetTextItem::Init()
{
  vtextitem::Init();

  CommonInit();
}


void SpreadSheetTextItem::CopyInit(vloadable *original)
{
  vtextitem::CopyInit(original);

  CommonInit();
}

vchar *SpreadSheetTextItem::ToBuffer()
{
  vchar         *cp;

  vtext *text = GetTextData();
  size_t len  = text->GetLength();

  cp = vstrCreateSized(len + 1);

  GetText(cp, len+1);
  cp[len] = vcharNULL;

  return cp;
}


void SpreadSheetTextItem::SelectAll()
{
  vtext *text = GetTextData();

  text->SelectRange(GetSelection(), 0, text->GetLength());
}

//============= The end of SpreadSheetTextItem ===============================


//*****************************************************************************
//* MyDrag
//*****************************************************************************

vkindTYPED_FULL_DEFINITION(MyDrag, vdrag, "MyDrag");


/*
 * MyDrag::Init()
 *
 */
void MyDrag::Init()
{
    vdrag::Init();

    sslist = NULL;
}

void MyDrag::ObserveDrag(vwindow *myfrom, vwindow *to, vevent *event)
{
    int row, column, numDown, numAcross;
    sslist->GetList()->DetermineSelectionBounds(sslist->GetSelection(), &row, &column, &numDown, &numAcross);

    // Call list drop notify
    int          nrow;

    int x,y;
    sslist->GetDifffromRoot(&x, &y);
    // If drop is in same dialog and in listitem then just move row
    if (to == myfrom &&
        sslist->GetRect()->ContainsPoint(event->GetX()+x, event->GetY()+y))
    {
        vpoint stPt;
        sslist->GetStartDragPt(&stPt);
        int ht = sslist->GetRowHeight(0);
        nrow = row + (stPt.GetY() - event->GetY())/ht + numDown -1; // Calculate the row no. to drag to

        sslist->AcceptEdits();

        // Move row
        if ((nrow < row) || (nrow > row+numDown-1)) {
            sslist->GetList()->MoveRange(row, nrow, numDown, vlistROW);
            sslist->SetModified(1);
        }
    }

    // Restore original window cursor
    vapplication::GetCurrent()->SetCursor(GetOriginalCursor());

    sslist->InvalFirstCol();   //Renumber all rows

    // Destroy this drag !!!
    DeleteLater();
}

// Notify procedure for drag to give feedback about potential drop site.
 
int MyDrag::HandleDetail(vwindow *myfrom, vwindow *to, vevent *event)
{
    vcursor     *cursor;

    if (to == myfrom &&
        sslist->GetRect()->ContainsPoint(event->GetX(), event->GetY()))
        cursor = GetGoodCursor();
    else
        cursor = GetBadCursor();
    vapplication::GetCurrent()->SetCursor(cursor);

    return (0);
}


void MyDrag::Draw(vrect *rect)
{

    vdraw::GSave();
    vdraw::SetLineWidth(1);

    // Outline list cell rect
    vdraw::RectsStrokeInside(rect, 1);

    // Draw list cell
    vlistIterator iterator;
    int row, column;
    iterator.StartWithSelection(sslist->GetSelection());
    while (iterator.Next()) {
       row = iterator.GetRow();
       column = iterator.GetColumn();
       vrect rt;
       sslist->GetCellRect(row, column, &rt);
       sslist->DrawCellContents(row, column, vTRUE, &rt);
    }
    iterator.Finish();
    vdraw::GRestore();
}

//============ The end of MyDrag  =======================================

//*****************************************************************************
//* MyPopupMenu & MyPopupMenuItem
//*****************************************************************************

vkindTYPED_FULL_DEFINITION(MyPopupMenu, vmenu, "MyPopupMenu");
vkindTYPED_FULL_DEFINITION(MyPopupMenuItem, vmenuItem, "MyPopupMenuItem");

void SelectionToClipBoard(gdSpreadSheet *gdlist);
void SetFromClipBoard(gdSpreadSheet *sheet, int row);

void MyPopupMenuItem::ObserveMenuItem(vevent *event)
{
    vmenuItem::ObserveMenuItem(event);
    gdSpreadSheet *gdlist = DetermineMenu()->GetSpreadSheet();
    gdlist->AcceptEdits();
    int edrow = gdlist->GetEditingRow();
    char *itemtitle = (char *)GetTitle();
    if(strcmp(itemtitle, "Cut") == 0) {
        SelectionToClipBoard(gdlist);
        int row, column, numDown, numAcross;
        gdlist->GetList()->DetermineSelectionBounds(gdlist->GetSelection(), &row, &column, &numDown, &numAcross);
        gdlist->GetList()->RemoveRange(row, numDown, vlistROW);
    } else if(strcmp(itemtitle, "Copy") == 0) {
        SelectionToClipBoard(gdlist);
    } else if(strcmp(itemtitle, "Paste") == 0) {
        int row, column, numDown, numAcross;
        gdlist->GetList()->DetermineSelectionBounds(gdlist->GetSelection(), 
                                                       &row, &column, &numDown, &numAcross);
	SetFromClipBoard(gdlist, row);
    } else if(strcmp(itemtitle, "Insert") == 0) {
        int row, column, numDown, numAcross;
        gdlist->GetList()->DetermineSelectionBounds(gdlist->GetSelection(), &row, &column, &numDown,&numAcross);
        gdlist->InsertOneEmptyRow(row, 1);
    } else if(strcmp(itemtitle, "Delete") == 0) {
        int row, column, numDown, numAcross;
        gdlist->GetList()->DetermineSelectionBounds(gdlist->GetSelection(), &row, &column, &numDown, &numAcross);
        gdlist->GetList()->RemoveRange(row, numDown, vlistROW);
    }
    if(strcmp(itemtitle, "Copy"))
        gdlist->InvalFirstCol();
    gdlist->SetModified(1);
}

MyPopupMenu *MyPopupMenuItem::DetermineMenu()
{
   return (MyPopupMenu *)vmenuItem::DetermineMenu();
}

//=========== The end of MyPopupMenu and MyPopupMenuItem ======================

//*****************************************************************************
//* Other non-member functions
//*****************************************************************************

// Save the contents of selected rows to clipboard

void  SelectionToClipBoard(gdSpreadSheet *sheet)
{
     int row, column, numDown, numAcross;
     sheet->GetList()->DetermineSelectionBounds(sheet->GetSelection(), &row, &column, &numDown, &numAcross);
     int i,j;
     int col =sheet->GetList()->GetColumnCount();
     if (col == 6) {
        PatternClipBoard_num = 0;
        for(i = row; i < row+numDown; i++) {
           j = 1;
           p_clipboard[PatternClipBoard_num].row = i;
           char *value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(!value || strcmp(value, "")==0) 
               p_clipboard[PatternClipBoard_num].criteria = -1;
           else
               p_clipboard[PatternClipBoard_num].criteria = (strcmp(value, "Include")==0? 1:0);

           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(value)
              strcpy(p_clipboard[PatternClipBoard_num].basestring, value);
           else
              strcpy(p_clipboard[PatternClipBoard_num].basestring, "");

           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(!value || strcmp(value, "")==0)
              p_clipboard[PatternClipBoard_num].sensitive = -1;
           else
              p_clipboard[PatternClipBoard_num].sensitive = (strcmp(value, "Yes")==0? 1:0);

           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(!value || strcmp(value, "")==0)
              p_clipboard[PatternClipBoard_num].entities = -1;
           else
              p_clipboard[PatternClipBoard_num].entities = (strcmp(value, "Yes")==0? 1:0);
 
           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(!value || strcmp(value, "")==0)
              p_clipboard[PatternClipBoard_num].strings = -1;
           else
              p_clipboard[PatternClipBoard_num].strings = (strcmp(value, "Yes")==0? 1:0);        
           PatternClipBoard_num++;
	}
     } else if (col == 4) {
	TypeClipBoard_num = 0;
        for(i = row; i < row+numDown; i++) {
           j = 1;
           t_clipboard[TypeClipBoard_num].row = i;
           char *value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if(!value || strcmp(value, "")==0)
              t_clipboard[TypeClipBoard_num].criteria = -1;
           else
              t_clipboard[TypeClipBoard_num].criteria = (strcmp(value, "Include")==0? 1:0);

           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if (value)
              strcpy(t_clipboard[TypeClipBoard_num].type, value);
           else
              strcpy(t_clipboard[TypeClipBoard_num].type, "");

           value = (char *)sheet->GetList()->GetCellValue(i, j++);
           if (value)
              strcpy(t_clipboard[TypeClipBoard_num].size, value);
           else 
              strcpy(t_clipboard[TypeClipBoard_num].size, "");
           TypeClipBoard_num++;
        }
     }
}

//Restore the row contents from clipboard

void  SetFromClipBoard(gdSpreadSheet *gdlist, int row)
{
     int i, j;
     vstr  *str;
	 int col =gdlist->GetList()->GetColumnCount();
     if (col == 6) {
	if (PatternClipBoard_num == 0)  return;
        gdlist->GetList()->InsertRange(row, PatternClipBoard_num, vlistROW);
        for(i = 0; i < PatternClipBoard_num; i++) {
           j = 1;
           char ss[100];
           if (p_clipboard[i].criteria == -1)
              strcpy(ss, "");
           else
              strcpy(ss, (p_clipboard[i].criteria == 1? "Include":"Exclude"));
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           strcpy(ss, p_clipboard[i].basestring);
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           if (p_clipboard[i].sensitive == -1)
              strcpy(ss, "");
           else
              strcpy(ss, (p_clipboard[i].sensitive == 1? "Yes":"No"));
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           if (p_clipboard[i].entities == -1)
              strcpy(ss, "");
           else
              strcpy(ss, (p_clipboard[i].entities == 1? "Yes":"No"));
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           if (p_clipboard[i].strings == -1)
              strcpy(ss, "");
           else
              strcpy(ss, (p_clipboard[i].strings == 1? "Yes":"No"));
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           gdlist->SetRowHeight(row, 20);
           row++;
        }
     } else if (col == 4) {
	if (TypeClipBoard_num == 0)  return;
        gdlist->GetList()->InsertRange(row, TypeClipBoard_num, vlistROW);
        for(i = 0; i < TypeClipBoard_num; i++) {
           j = 1;
           char ss[100];
           if (t_clipboard[i].criteria == -1)
              strcpy(ss, "");
           else
              strcpy(ss, (t_clipboard[i].criteria == 1? "Include":"Exclude"));
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           strcpy(ss, t_clipboard[i].type);
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           strcpy(ss, t_clipboard[i].size);
           str = vstrCloneScribed (vcharScribeLiteral(ss));
           gdlist->GetList()->SetCellValue(row, j++, str);

           gdlist->SetRowHeight(row, 20);
           row++;
        }
     }
}


typedef struct {
  int criteria;
  char basestring[1024];
  int sensitive;
  int entities;
  int strings;
} PATTERN;

typedef struct {
  int criteria;
  char type[1024];
  char size[1024];
} TYPE;


int SendToServer_sync(char *msg, gString& result);
int SendToServer_asyn(char *msg);
 
// On Unix, filtername is passed in without path info. and aset
// has to be asked to obtain the full path of the filter.
// On NT, the full path of the filter name is passed in.

// return 1 ----  save filter successfully.
//        0 ----  filter exists, but error occurs when it is saved.
//       -1 ----  filter does not exist.

int SaveOneFilter(gdSpreadSheet *plist, gdSpreadSheet *tlist, char *filtername)
{
    gString filename;
#ifndef _WIN32
     char buf[1024]; 

     OSapi_sprintf(buf, "filter_save %s", filtername);
     int ret = SendToServer_sync(buf, filename);
     if(!ret)  return 0;

    if(filename.length() == 0) return -1;
#else
    filename = filtername;
#endif

     FILE *fd;
     if(!(fd = OSapi_fopen(filename, "w"))) {
         printf("Error: open file failed.\n");
         return 0;
     }
     OSapi_fprintf(fd, "%s\n", FILT_VER_STR);
     PATTERN pattern;
     TYPE    type;

     vlist *list = plist->GetList();
     int row = list->GetRowCount();
     int column = list->GetColumnCount();

     char *filterlist = "# Pattern Matching\n\n";
     fwrite(filterlist, 1, strlen(filterlist), fd);

     int i, j;
     char *value;
     for(i = 0; i < row; i++) {
        j = 1;
        value = (char *)list->GetCellValue(i,j++);
        if(strcmp(value, "")==0)  continue;
        pattern.criteria = (strcmp(value, "Include")==0)? 1:0;

        value = plist->GetEditingValue(i,j++);
        if(value && strcmp(value, "")!=0 )
	  strcpy(pattern.basestring, value);
        else
           strcpy(pattern.basestring, "NULL");

        pattern.sensitive = (strcmp((char *)list->GetCellValue(i,j++), "Yes")==0)? 1:0;
        pattern.entities = (strcmp((char *)list->GetCellValue(i,j++), "Yes")==0)? 1:0;
        pattern.strings = (strcmp((char *)list->GetCellValue(i,j++), "Yes")==0)? 1:0;
	
	gString tmp;
	tmp.sprintf((vchar *)"!%5d  %1d  %1d  %1d  %-40s\n\n", pattern.criteria, 
		    pattern.sensitive, pattern.entities, pattern.strings, pattern.basestring);
	fwrite((char *)tmp, 1, strlen((char *)tmp), fd);
     }   
     filterlist = "\n\n";
     fwrite(filterlist, 1, strlen(filterlist), fd);

     list   = tlist->GetList();
     row    = list->GetRowCount();
     column = list->GetColumnCount();

     filterlist = "# Type Matching\n\n";
     fwrite(filterlist, 1, strlen(filterlist), fd);

     for(i = 0; i < row; i++) {
        j = 1;
        value = (char *)list->GetCellValue(i,j++);
        if(strcmp(value, "")==0)  continue;
        type.criteria = (strcmp(value, "Include")==0)? 1:0;

        value = tlist->GetEditingValue(i,j++);
        if(value && strcmp(value, "")!=0 )
           strcpy(type.type, value);
        else
           strcpy(type.type, "NULL");

        value = tlist->GetEditingValue(i,j++);
        if(value && strcmp(value, "")!=0 )
           strcpy(type.size, value);
        else
	  strcpy(type.size, "NULL");

	gString tmp;
	tmp.sprintf((vchar *)"!%5d  %-40s %-20s\n\n", type.criteria, type.type, type.size);
	fwrite((char *)tmp, 1, strlen((char *)tmp), fd);

     }
     OSapi_fclose(fd);
     plist->AcceptEdits();
     tlist->AcceptEdits();
     plist->SetModified(0);
     tlist->SetModified(0);
     return 1;
}
     


// return 1 ----  load filter successfully.
//        0 ----  filter exists, but error occurs when it is loaded.
//       -1 ----  filter does not exist.

// On Unix, filtername is passed in without path info. and aset
// has to be asked to obtain the full path of the filter.
// On NT, the full path of the filter name is passed in.

int LoadOneFilter(gdSpreadSheet *plist, gdSpreadSheet *tlist, char *filtername)
{
    gString filename;

     if(strcmp(filtername, "New")==0) {
        plist->AcceptEdits();
        plist->CleanUp();
        tlist->AcceptEdits();
        tlist->CleanUp();
        plist->SetModified(0);
        tlist->SetModified(0);
        return 1;
     }

#ifndef _WIN32
     char buf[1024];
     if(strcmp(filtername, "Default")==0) {
        char *path = OSapi_getenv("PSETHOME");
        if(!path)  return 0;
	filename  = path;
	filename += "/lib/Default.flt";
     } else {
        OSapi_sprintf(buf, "filter_load %s", filtername);
        int ret = SendToServer_sync(buf, filename);
        if(!ret)  return 0;
	if(filename.length() == 0) return -1;
     }
#else
    filename = filtername;
#endif

     FILE *fd;
     if(!(fd = OSapi_fopen(filename, "r"))) {
         printf("Error: open file failed.\n");
         return 0;
     }

     PATTERN pattern;
     TYPE    type;

     int row, old_filter = 1;
     vstr *str;
    
     char version_line[MAX_LEN];
     fscanf(fd, "%s", version_line);
     if (!strcmp(version_line, FILT_VER_STR))
       old_filter = 0;
     fseek(fd, 0L, SEEK_SET);
     char ch = OSapi_fgetc(fd);
     while((ch != '#') && (ch != EOF))
        ch = OSapi_fgetc(fd);
     if(ch == EOF) {  //If the file is empty, clean up the spreadsheet.
        OSapi_fclose(fd);
        plist->AcceptEdits();
        plist->CleanUp();
        tlist->AcceptEdits();
        tlist->CleanUp();
        plist->SetModified(0);
        tlist->SetModified(0);
        return 1;
     }

     plist->AcceptEdits();
     plist->CleanUp();
     row = 0;
     while(((ch=OSapi_fgetc(fd)) != EOF) && (ch!= '#')) {

        while((ch != EOF) && (ch != '#') && (ch != '!'))
           ch = OSapi_fgetc(fd);
        if(ch == EOF) {
           OSapi_fclose(fd);
           plist->SetModified(0);
           tlist->SetModified(0);
           return 1;
        }
        if(ch == '#')  break;

	//sudha:03/16/98, changes to fix bug# 14752	
	int ret, num_read;
	if (old_filter) {
	  num_read = 5;
	  ret = OSapi_fscanf(fd, "%5d  %40s  %1d %1d  %1d", &pattern.criteria, pattern.basestring, 
		  &pattern.sensitive, &pattern.entities, &pattern.strings); 
	}
	else {
	  num_read = 4;
	  ret = OSapi_fscanf(fd, "%5d  %1d %1d  %1d", &pattern.criteria, 
	             &pattern.sensitive, &pattern.entities, &pattern.strings);
	}
        if(ret != num_read || ret == EOF || ret == 0) {
	  printf("Error: read error.\n");
	  OSapi_fclose(fd);
	  return 0;
        }
	char value[MAX_LEN];
	strcpy(value, "");
	if (!old_filter) {
	  char c, *ptr = value;
	  int len = 0;

          // strip leading spaces
	  while( ((c=fgetc(fd)) == ' ') || (c == '\t') );
	  
	  while(c != EOF && c != '\n' && c != '\015') {
	    *(ptr++) = c;

	    len++;
	    if (len > MAX_LEN) {
              printf("Error: Regular expression is longer that %d chars.\n", MAX_LEN);
	      OSapi_fclose(fd);
	      return 0;
            }

	    c = fgetc(fd);
	  }
	  *ptr = '\0';

	  if (value[0] == 0) {
            printf("Error: Could not read regular expression.\n");
	    OSapi_fclose(fd);
	    return 0;
          } else {
	    // strip following spaces 
	    ptr = value + strlen(value) - 1;
	    while((*ptr == ' ') || (*ptr == '\t'))
	      *(ptr--) = '\0';
	  }
	  strcpy(pattern.basestring, value);
	}
	int col = 0;
        OSapi_sprintf(value, "%d", row+1);
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, (pattern.criteria? "Include":"Exclude"));  
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, pattern.basestring);
        if(strcmp(value, "NULL")==0)
            strcpy(value, "");
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->SetEditingCellValue(row, col++, str);
    
        strcpy(value, (pattern.sensitive? "Yes":"No"));
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, (pattern.entities? "Yes":"No"));
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, (pattern.strings? "Yes":"No"));
        str = vstrCloneScribed (vcharScribeLiteral(value));
        plist->GetList()->SetCellValue(row, col++, str);
        
        plist->SetRowHeight(row, 20);

        row++;
     }

     if(ch == EOF) {
        OSapi_fclose(fd);
        plist->SetModified(0);
        tlist->SetModified(0);
        return 1;
     }
    
     tlist->AcceptEdits();
     tlist->CleanUp();
     row = 0;
     while(((ch=OSapi_fgetc(fd)) != EOF) && (ch!= '#')) {

        while((ch != EOF) && (ch != '#') && (ch != '!'))
           ch = OSapi_fgetc(fd);
        if(ch == EOF)  {
           OSapi_fclose(fd);
           plist->SetModified(0);
           tlist->SetModified(0);
           return 1;
        }
        if(ch == '#')  break;

        int ret = OSapi_fscanf(fd, "%5d  %40s  %20s", &type.criteria, type.type, type.size);
        if(ret != 3 || ret == EOF || ret == 0) {
            printf("Error: read error.\n");
            OSapi_fclose(fd);
            return 0;
        }
        int col = 0;
        char value[1024];
        OSapi_sprintf(value, "%d", row+1);
        str = vstrCloneScribed (vcharScribeLiteral(value));
        tlist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, (type.criteria? "Include":"Exclude"));
        str = vstrCloneScribed (vcharScribeLiteral(value));
        tlist->GetList()->SetCellValue(row, col++, str);

        strcpy(value, type.type);
        if(strcmp(value, "NULL")==0)
            strcpy(value, "");
        str = vstrCloneScribed (vcharScribeLiteral(value));
        tlist->SetEditingCellValue(row, col++, str);

        strcpy(value, type.size);
        if(strcmp(value, "NULL")==0)
            strcpy(value, "");
        str = vstrCloneScribed (vcharScribeLiteral(value));
        tlist->SetEditingCellValue(row, col++, str);
        tlist->SetRowHeight(row, 20);

        row++;
    }
    OSapi_fclose(fd);
    plist->SetModified(0);
    tlist->SetModified(0);
    return 1;
}

// This function is only used by Unix version.

void GetAllFilterName(gString& filternames)
{
    gString buffer;
    int ret      = SendToServer_sync("filter_list", buffer);
    filternames  = (vchar *)"Default ";
    filternames += buffer;
}

// Send access command to aset asynchronically. This function is only used by Unix version.

int SendToServer_asyn(char *msg)
{
    int fd = 4;
    int len;
    char head[5];
    head[0] = '\3';
    if(write(fd, head, 1) != 1)
        return 0;

    len = strlen(msg);
    if(write(fd, msg, len + 1) == len + 1) 
        return 1;
    else
        return 0;
}

// Send access command to aset synchronically. This function is only used by Unix version.

int SendToServer_sync(char *msg, gString& retval)
{
    // For standard I/O:  fdread = 0, fdwrite = 1
    // For 2waypipe:      fdread = 3, fdwrite = 4
    int fdread = 3;
    int fdwrite = 4;
    int len;
    char head[5];
    
    head[0] = '\4';
    if(write(fdwrite, head, 1) != 1) {
        printf("Error: write head failed\n");
        return 0;
    }

    int ok = 1;
    len = strlen(msg);
    if(write(fdwrite, msg, len + 1) == len + 1) {
	char buffer[1025];
        int nbyte, msglen = 0, recvlen = 0, gotsize = 0;
        do {
            do {
	      nbyte = read(fdread, buffer, sizeof(buffer) - 1);
	    } while ((errno == EINTR) && (nbyte < 0));

            if (nbyte >= 0) {
              buffer[nbyte] = '\0';
              if (!gotsize) {
                int headlen = strlen(buffer);
                if ((headlen > 0) && (headlen < (sizeof(buffer) - 1))) {
                  msglen = atoi(buffer);
                  if (msglen >= 0) {
                    recvlen += (nbyte - headlen) - 1;
                    retval += buffer + headlen + 1;
                    gotsize = 1;
		  } else {
                    printf("Error: message header from server contains invalid length\n");
                    ok = 0;
		  }
	        } else {
                  printf("Error: message header from server is corrupt\n");
                  ok = 0;
	        }
	      } else {
                recvlen += nbyte;
                retval += buffer;
	      }
	    } else {
              printf("Error: failed to read from server\n");
              ok = 0;
	    }
	} while ((recvlen < msglen) && ok);
    } else {
        printf("Error: write command error\n");
        ok = 0;
    }

    return ok;
}
