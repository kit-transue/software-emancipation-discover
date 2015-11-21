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
#include vbuttonHEADER
#include vcontrolHEADER
#include vcontainerHEADER
#include vnotebookHEADER
#include vstdlibHEADER
#include veventHEADER
#include vsignalHEADER

#include <Application.h>
#include <gDrawingArea.h>
#include <tipwin.h>
#include <ggenDialog.h>
#include <ginterp.h>
#include <gString.h>
#include <gglobalFuncs.h>
#include <gdraw.h>
#include <cLibraryFunctions.h>
#include <machdep.h>

#define X_MARGIN 5
#define Y_MARGIN 10

#define POINTER1 1
#define POINTER2 2

extern vresource systemResources;

static vimage *pointers[2] = { NULL, NULL };

class gdDomainObserver : public vdomainObserver
{
  public:
    vkindTYPED_FULL_DECLARATION(gdDomainObserver);
    vloadableINLINE_CONSTRUCTORS(gdDomainObserver, vdomainObserver);

    void SetDomainItem(gdDrawingArea *domitem);

    gdDrawingArea *item;

//    virtual void ObserveSelect(vdomainObjectSet *obj_set, vdomainSelection selection, int selected);
    virtual void ObserveMove(vdomainObjectSet *obj_set, vpoint *delta, int positioning, int before);
};

vkindTYPED_FULL_DEFINITION(gdDomainObserver, vdomainObserver, "gdDomainObserver");
vkindTYPED_FULL_DEFINITION(gdDrawingArea, vdomainitem, "gdDrawingArea");

//-----------------------------------------------------------------

DrawAreaItemDesc::~DrawAreaItemDesc()
{
    if(image)
	image->DeleteLater();
    if(title)
	free(title);
    if(tipwin_text)
	free(tipwin_text);
}

//-----------------------------------------------------------------

void gdDomainObserver::SetDomainItem(gdDrawingArea *domitem)
{
    item = domitem;
}

void gdDomainObserver::ObserveMove(vdomainObjectSet *obj_set, vpoint *delta, int positioning, int before)
{
    vdomainObjectSetIterator iter;
    vdomain *domain = GetOwner();
    iter.Start(obj_set);
    while(iter.Next()){
	vdomainObject *obj     = iter.GetObject();
	DrawAreaItemDesc *desc = (DrawAreaItemDesc *)domain->GetObjectData(obj);
	if(desc && before == 0){
	    vpoint pt;
	    int image_w = desc->image->GetWidth();
	    int image_h = desc->image->GetHeight();
	    pt.SetX(desc->x + image_w / 2);
	    pt.SetY(desc->y + image_h / 2);
	    int from_row, from_col, from_cell;
	    item->FindRowColumnCell(pt, from_row, from_col, from_cell);
	    int repositioned = 0;
	    if(from_cell != -1){
		vpoint new_pos;
		pt.Add(delta, &new_pos);
		if(new_pos.GetX() - image_w / 2 > X_MARGIN && new_pos.GetX() + image_w / 2 < item->current_w - X_MARGIN
		   && new_pos.GetY() - image_h / 2 > Y_MARGIN && new_pos.GetY() + image_w / 2 < item->current_h - Y_MARGIN){
		    int to_row, to_col, to_cell;
		    item->FindRowColumnCell(new_pos, to_row, to_col, to_cell);
		    if(to_cell != -1) {
			vrect rect;
			int x, y;
			int do_move = 1;
			if(to_cell > item->items.size() - 1)
			    to_cell = item->items.size() - 1;
			if(item->move_command && to_cell != from_cell){
			    gString full_cmd;
			    full_cmd.sprintf((const vchar *)"%s %d %d %d %d %d %d", item->move_command, from_row, from_col, from_cell,
					     to_row, to_col, to_cell);
			    if(gala_eval(item->interp(), (char *)full_cmd) == TCL_OK){
				if(Tcl_GetStringResult(item->interp())[0] == '0')
				    do_move = 0;
			    }
			}
			if(do_move){
			    item->GetItemPosition(x, y, image_w, image_h, to_cell);
			    rect.Set(x, y, image_w, image_h);
			    domain->SetObjectBounds(vdomainObjectToSet(obj),
						    &rect);
			    desc->x      = x;
			    desc->y      = y;
			    repositioned = 1;
			    item->MoveInsertItem(desc, from_cell, to_cell);
			}
		    }
		}
	    }
	    if(!repositioned){
		vrect rect;
		int x, y;
		
		item->GetItemPosition(x, y, image_w, image_h, from_cell);
		rect.Set(x, y, image_w, image_h);
		domain->SetObjectBounds(vdomainObjectToSet(obj),
					&rect);
	    }
	}    
    }
}

//-----------------------------------------------------------------

void gdDrawingArea::CommonInit()
{
    vresource res;

    if(pointers[0] == NULL){
	if(getResource(systemResources, "Images", &res)){
	    int num_images = vresourceCountComponents(res);
	    for(int i = 0; i < num_images; i++) {
		const vname *tag = vresourceGetNthTag(res, i);
		if(strcmp((char *)tag, "Pointer1") == 0){
		    vresource image_res = vresourceGet(res, tag);
		    pointers[0]         = new vimage(vloadableDONT_LOAD);
		    pointers[0]->Load(image_res);
		} else if(strcmp((char *)tag, "Pointer2") == 0){
		    vresource image_res = vresourceGet(res, tag);
		    pointers[1]         = new vimage(vloadableDONT_LOAD);
		    pointers[1]->Load(image_res);
		}
	    }
	}
    }
    vertical     = 0;
    grid_w       = 20;
    grid_h       = 20;
    current_rows = 0;
    current_cols = 0;
    was_opened   = 0;
}

void gdDrawingArea::LoadInit(vresource resource)
{
    vdomainitem::LoadInit(resource);
    CommonInit();
}

void gdDrawingArea::Destroy()
{
    vdomainitem::Destroy();
}

void gdDrawingArea::Init()
{
    vdomainitem::Init();
    CommonInit();
}

//-----------------------------------------------------------------

void gdDrawingArea::Open()
{
    vdomainObjectSet *objectSet;
    vrect            *bounds;
    vrect            rect;
    vdomainObject    *object;
    vcolorSpec       spec;
    vcolor           *color;
    vdomain          *theDomain;

    vdomainitem::Open();
    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
    if(was_opened == 0){
      SetFocusable(vTRUE);
      SetDragMethod(vdomainviewEXTERNAL_DRAG);
      SetOffscreenDrawing(vTRUE);
      
      gdDomainObserver *observer = new gdDomainObserver();
      theDomain                  = GetDomain();
      bounds                     = (vrect*)GetRect();
      observer->SetDomainItem(this);
      theDomain->InstallObserverOwned(observer);
      theDomain->Resize(bounds->w, bounds->h);
      current_w = bounds->w;
      current_h = bounds->h;
      CalcRowsAndColumns();
      theDomain->SetOrientation(vdomainORIENT_BOTTOM_LEFT);
      was_opened = 1;
      InvalView();
    }
}

void gdDrawingArea::Show()
{
    vwindow *win = DetermineWindow();
//    if(win)
//	win->SetMotion(vTRUE);   // Will handle motion events to do highlites
}

void gdDrawingArea::DrawObject(vdomainObject *object)
{
    vrect   *bounds;
    vdomain *theDomain;
    vrect   src;
  
    theDomain              = GetDomain();
    DrawAreaItemDesc *desc = (DrawAreaItemDesc *)theDomain->GetObjectData(object);
    bounds                 = theDomain->GetObjectBounds(object);
    src.x                  = 0;
    src.y                  = 0;
    src.w                  = desc->image->GetWidth();
    src.h                  = desc->image->GetHeight();
    if(desc->selected != 0){
	vdraw::MoveTo(bounds->GetX(), bounds->GetY());
	vdraw::RectImageComposite(desc->image, &src, bounds);
	if((desc->selected & POINTER1) != 0 && pointers[0] != NULL)
	    vdraw::ImageCompositeIdent(pointers[0]);
	if((desc->selected & POINTER2) != 0 && pointers[1] != NULL)
	    vdraw::ImageCompositeIdent(pointers[1]);
    } else
	vdraw::RectImage(desc->image, &src, bounds);
    if(desc->title){
	vdrawGSave();
	vfont *font      = DetermineFont();
	int string_width = font->StringWidthX((const vchar *)desc->title);
	vrect rect;
	rect.x = bounds->GetX();
	rect.y = bounds->GetY();
	rect.w = desc->image->GetWidth();
	rect.h = desc->image->GetHeight();
	if(rect.w < string_width - 2){
	    float scale_factor = (float)rect.w / string_width;
	    vdraw::SetFont(font->FScale(font->GetFace(), font->GetSize() * scale_factor));
	}
	vdraw::RectShowCenter((const vchar *)desc->title, &rect);
	vdrawGRestore();
    }
}

// select flag 1 - add pointer 1, 2 - add pointer 2, -1 - remove pointer 1, -2 - remove pointer 2, 0 - remove both
void gdDrawingArea::SelectItem(int item, int select_flag)
{
    if(item < 0 || item >= items.size())
	return;
    DrawAreaItemDesc *desc = items[item];
    if(select_flag > 0)
	desc->selected |= select_flag;
    else
	if(select_flag < 0)
	    desc->selected &= ~(-select_flag);
	else
	    desc->selected = 0;
    InvalObjects(vdomainObjectToSet(desc->object), vwindowINVAL_IMMEDIATE);
}

vbool gdDrawingArea::HandleDoubleClickEvent(vdomainviewDblClickEvent *event)
{
    int button = event->event->GetButton();
    if(button == 0){
	vpoint pt(event->position);
	int row, col, cell;
	FindRowColumnCell(pt, row, col, cell);
	if (cell != -1 && click_command){
	    gString full_cmd;
	    full_cmd.sprintf((const vchar *)"%s %d %d %d dblclick", click_command, row, col, cell);
	    gala_eval(interp(), (char *)full_cmd);
	    return vTRUE;
	}
    }
    return vFALSE;
}

gString cc;
int     crow; 
int     ccol;
int     ccell;

#ifdef _WIN32
static void HandleButton(void *data)
{
    gdDrawingArea *obj = (gdDrawingArea *)data;
    gString full_cmd;
    full_cmd.sprintf((const vchar *)"%s %d %d %d click", (char *)cc, crow, ccol, ccell);
    gala_eval(obj->interp(), (char *)full_cmd);
}

#else

static void HandleButton(void *data)
{
    static sigset_t old_mask;
    sigset_t mask;

    gdDrawingArea *obj = (gdDrawingArea *)data;

    sigemptyset (&mask);
    sigaddset (&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);
    gString full_cmd;
    full_cmd.sprintf((const vchar *)"%s %d %d %d click", (char *)cc, crow, ccol, ccell);
    gala_eval(obj->interp(), (char *)full_cmd);
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
}
#endif

vbool gdDrawingArea::HandleButtonDownEvent(vdomainviewButtonDownEvent *event)
{
    int button = event->event->GetButton();
    if(button == 0){
#if 0
	vpoint pt(event->position);
	int row, col, cell;
	FindRowColumnCell(pt, row, col, cell);
	if (cell != -1 && click_command){
	    gString full_cmd;
	    full_cmd.sprintf((const vchar *)"%s %d %d %d click", click_command, row, col, cell);
	    gala_eval(interp(), (char *)full_cmd);
	    return vTRUE;
	}
#endif
	vpoint pt(event->position);
	int row, col, cell;
	FindRowColumnCell(pt, row, col, cell);
	if (cell != -1 && click_command){
	    button_down_row = row;
	    button_down_col = col;
	    return vTRUE;
	}
    }
    return vFALSE;
}

vbool gdDrawingArea::HandleButtonUpEvent(vdomainviewButtonUpEvent *event)
{
    int button = event->event->GetButton();
    if(button == 0){
	vpoint pt(event->position);
	int row, col, cell;
	FindRowColumnCell(pt, row, col, cell);
	if (cell != -1 && click_command && row == button_down_row && col == button_down_col){
#if 0
	    gString full_cmd;
	    full_cmd.sprintf((const vchar *)"%s %d %d %d click", click_command, row, col, cell);
	    gala_eval(interp(), (char *)full_cmd);
	    return vTRUE;
#endif
	    cc    = click_command;
	    crow  = row;
	    ccol  = col;
	    ccell = cell;
	    callbackEvent *event = new callbackEvent( HandleButton, this );
	    if( event )
		event->PostFuture(0, 1000000);
	}
    }
    return vFALSE;
}

int gdDrawingArea::HandleMotion(vevent *event)
{
    vpoint pt, pt_orig;

    pt.SetX(event->GetX());
    pt.SetY(event->GetY());
    pt_orig = pt;
    TranslatePoint(&pt, vdomainviewTO_DOMAIN);
    int x, y;
    DetermineDialog()->GetPointer( &x, &y );
    int diff_x = pt_orig.x - x;
    int diff_y = pt_orig.y - y;
    int row, col, cell;
    FindRowColumnCell(pt, row, col, cell);
    if(cell < 0 || cell >= items.size()){
	tipWindow::Display(NULL, DetermineDialog(), -1, 0, 0, NULL);
	return vTRUE;
    }
    DrawAreaItemDesc *desc = items[cell];
    if(desc->tipwin_text)
	tipWindow::Display (this, DetermineDialog(), cell, diff_x, diff_y, (const vchar *)desc->tipwin_text);
    else
	tipWindow::Display (this, DetermineDialog(), cell, diff_x, diff_y, (const vchar *)desc->title);
    return vTRUE;
}


void gdDrawingArea::FindRowColumnCell(vpoint &pt, int &row, int &col, int &cell)
{
    int x = pt.GetX();
    int y = pt.GetY();
    if(x > X_MARGIN && x < X_MARGIN + grid_w * current_cols &&
       y > Y_MARGIN && y < current_h - Y_MARGIN) {
	y   += Y_MARGIN;
	x   -= X_MARGIN;
	row  = (current_h - y) / grid_h;
	col  = x / grid_w;
	cell = row * current_cols + col;
    } else 
	cell = -1;
}

void gdDrawingArea::SetClickCmd(char *cmd)
{
    click_command = strdup(cmd);
}

void gdDrawingArea::SetMoveCmd(char *cmd)
{
    move_command = strdup(cmd);
}

void gdDrawingArea::SetGrid(int w, int h, char *orientation)
{
    grid_w = w;
    grid_h = h;
    if(*orientation == 'V' || *orientation == 'v')
	vertical = 1;
    else
	vertical = 0;
    CalcRowsAndColumns();
}

void gdDrawingArea::SetGridDimensions(int rows, int cols)
{
    vdomain *theDomain;
    
    if(rows > current_rows || cols > current_cols){
	current_cols = cols;
	current_rows = rows;
	current_w    = X_MARGIN + X_MARGIN + current_cols * grid_w;
	current_h    = Y_MARGIN + Y_MARGIN + current_rows * grid_h;
	theDomain    = GetDomain();
	theDomain->Resize(current_w, current_h);
	Scroll(0, current_h, vdomainABSOLUTE);
    }
}

void gdDrawingArea::Clear()
{
    vdomain *theDomain = GetDomain();
    for(int i = 0; i < items.size(); i++){
	DrawAreaItemDesc *desc = items[i];
	theDomain->RemoveObjects(vdomainObjectToSet(desc->object));
	delete desc;
    }
    items.reset();
    vrect *bounds = (vrect*)GetRect();
    theDomain->Resize(bounds->w, bounds->h);
    current_w = bounds->w;
    current_h = bounds->h;
}

void gdDrawingArea::SetIconTitle(int item_num, char *title)
{
    if(item_num >= items.size() || item_num < 0)
	return;
    DrawAreaItemDesc *desc = items[item_num];
    desc->title            = strdup(title);
}

void gdDrawingArea::SetIconTip(int item_num, char *tip)
{
    if(item_num >= items.size() || item_num < 0)
	return;
    DrawAreaItemDesc *desc = items[item_num];
    desc->tipwin_text      = strdup(tip);
}

void gdDrawingArea::RemoveItem(int item_num)
{
    if(item_num >= items.size() || item_num < 0)
	return;
    DrawAreaItemDesc *desc = items[item_num];
    domain->RemoveObjects(vdomainObjectToSet(desc->object));
    delete desc;
    ShiftItems(item_num);
}

int gdDrawingArea::AddItem(char *image_name)
{
    vdomainObject *object;
    vdomain	  *theDomain;
    vrect	  rect;
    vimage        *image = NULL;
    vresource     res;

    if(getResource(systemResources, "Images", &res)){
	int num_images = vresourceCountComponents(res);
	for(int i = 0; image == NULL && i < num_images; i++) {
	    const vname *tag = vresourceGetNthTag(res, i);
	    if(strcmp((char *)tag, image_name) == 0){
		vresource image_res = vresourceGet(res, tag);
		image               = new vimage(vloadableDONT_LOAD);
		image->Load(image_res);
	    }
	}
    }
    if(image == NULL)
	return -1;
    DrawAreaItemDesc *desc = new DrawAreaItemDesc;
    desc->number           = items.size();
    desc->image            = image;
    desc->selected         = 0;
    desc->title            = NULL;
    desc->tipwin_text      = NULL;

    theDomain    = GetDomain();
    int image_w = image->GetWidth();
    int image_h = image->GetHeight();
    int x, y;
    GetItemPosition(x, y, image_w, image_h, desc->number);
    desc->x = x;
    desc->y = y;
    // Extend domain bounds if new item lies outside current bounds
//    if(x + image_w > current_w)
//	current_w = x + image_w;
    if(y < 0){
	int diff   = -y + Y_MARGIN;
	current_h += diff;
	desc->y    = y = Y_MARGIN;
	theDomain->Resize(current_w, current_h);
	// Shift all existing items
	for(int i = 0; i < items.size(); i++){
	    DrawAreaItemDesc *tmp = items[i];
	    int x, y;
	    vrect rect;
	    GetItemPosition(x, y, tmp->image->GetWidth(), tmp->image->GetHeight(), i);
	    tmp->x = x;
	    tmp->y = y;
	    rect.Set(x, y, tmp->image->GetWidth(), tmp->image->GetHeight());
	    vdomainObject *object = tmp->object;
	    theDomain->SetObjectBounds(vdomainObjectToSet(object),
				       &rect);
	}
	Scroll(0, 0, vdomainABSOLUTE);
    }
    rect.Set(x, y, image_w, image_h);
    items.append(desc);
    object       = theDomain->AddObject();
    desc->object = object;
    theDomain->SetObjectBounds(vdomainObjectToSet(object), &rect);
    theDomain->SetObjectData(vdomainObjectToSet(object), desc);
    vdomainObjectSet *objectSet = vdomainCreateObjectSet(NULL);
    vdomainAddToObjectSet(&objectSet, object);
    InvalView();
    theDomain->ShowObjects(objectSet);
    vdomainDestroyObjectSet(objectSet);
    return desc->number;
}

void gdDrawingArea::ShiftItems(int to_pos)
{
    vdomain *theDomain = GetDomain();
    for(int i = to_pos; i < items.size() - 1; i++){
	DrawAreaItemDesc *tmp = items[i + 1];
	items[i]              = tmp;
	tmp->number           = i;
	int x, y;
	vrect rect;
	GetItemPosition(x, y, tmp->image->GetWidth(), tmp->image->GetHeight(), i);
	rect.Set(x, y, tmp->image->GetWidth(), tmp->image->GetHeight());
	vdomainObject *object = tmp->object;
	theDomain->SetObjectBounds(vdomainObjectToSet(object),
				   &rect);
	tmp->x = x;
	tmp->y = y;
    }
    items.remove(items.size() - 1);
}

void gdDrawingArea::MoveInsertItem(DrawAreaItemDesc *desc, int from_pos, int to_pos)
{
    vdomain *theDomain = GetDomain();
    if(from_pos != items.size() - 1){
	ShiftItems(from_pos);
	items.append(desc);
    }
    for(int i = items.size() - 2; i >= to_pos; i--){
	DrawAreaItemDesc *tmp = items[i];
	items[i + 1]          = tmp;
	tmp->number           = i + 1;
	int x, y;
	vrect rect;
	GetItemPosition(x, y, tmp->image->GetWidth(), tmp->image->GetHeight(), i + 1);
	rect.Set(x, y, tmp->image->GetWidth(), tmp->image->GetHeight());
	vdomainObject *object = tmp->object;
	theDomain->SetObjectBounds(vdomainObjectToSet(object),
				   &rect);
	tmp->x = x;
	tmp->y = y;
    }
    items[to_pos] = desc;
    desc->number  = to_pos;
}

void gdDrawingArea::CalcRowsAndColumns()
{
    if(current_w - X_MARGIN * 2 < grid_w)
	current_cols = 1;
    else
	current_cols = (current_w - X_MARGIN * 2) / grid_w;
    if(current_h - Y_MARGIN * 2 < grid_h)
	current_rows = 1;
    else
	current_rows = (current_h - Y_MARGIN * 2) / grid_h;
}

void gdDrawingArea::GetItemPosition(int &x, int &y, int w, int h, int number)
{
    y = (number / current_cols) * grid_h;
    x = (number % current_cols) * grid_w;
    x += X_MARGIN;
    y  = current_h - y - grid_h - Y_MARGIN;
}
