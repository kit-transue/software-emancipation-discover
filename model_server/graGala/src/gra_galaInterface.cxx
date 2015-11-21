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
// INCLUDE FILES

#include <vport.h>
#include <vevent.h>
#include vapplicationHEADER
#include vwindowHEADER
#include <graInterface.h>
#include <gra_galaInterface.h>
#include <ggraViewLayer.h>
#include <gviewer.h>
#define  xxxPrintf printf
#undef   printf

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <graEnums.h>
#include <graSymbol.h>
#include <graResources.h>
#include <viewError.h>
#include <viewNode.h>
#include <viewGraHeader.h>
#include <viewFn.h>
#include <viewSelection.h>
#include <viewFlowchartHeader.h>
#include <viewTreeHeader.h>
#include <driver.h>
#include <perspective.h>
#include <graWorld.h>
#include <systemMessages.h>

#define printf xxxPrintf

extern "C" isServer();

extern int is_view_node_visible(viewTree *v);

extern long gala_current_window;;
extern long current_root_window;

class gravdomainLong : public vdomainLong { // this class is create to ovveride vdomain::SelectAll function which 
                                            // generates array bounds write error.
  public:
    vkindTYPED_FULL_DECLARATION(gravdomainLong);
    vloadableINLINE_CONSTRUCTORS(gravdomainLong, vdomainLong);
    
    virtual void SelectAll(vdomainSelection selection, int select, int selectHidden);
};

vkindTYPED_FULL_DEFINITION(gravdomainLong, vdomainLong, "gravdomainLong");

class CallbackEvent : public veventClientEvent
{
  public:
    virtual int Handle(void);

    void *func;
    void *data;
};

/* ---------------------------------------------------------------------- */

int CallbackEvent::Handle(void)
{
    void (*fn)(void *);

    fn       = (void (*)(void *))func;
    void *dt = data;
    if (fn) 
	(*fn)(dt);
    return vTRUE;
}

/* ---------------------------------------------------------------------- */

void gravdomainLong::SelectAll(vdomainSelection selection, int select, int selectHidden)
{
    vdomainObjectSet *objectSet = GetObjectSet(selection, vdomainALL, NULL);
    if(select)
	SelectObjects(selection, objectSet);
    else
	UnselectObjects(selection, objectSet);
    vdomainDestroyObjectSet(objectSet);
}

/* ---------------------------------------------------------------------- */

graInterface::graInterface()
{
}

graInterface::~graInterface()
{
}

graInterface *graInterface::Create(void)
{
    return(new gra_galaInterface);
}

/* -------------------------------------------------------------------------- */

gra_galaInterface::gra_galaInterface()
{
    domain     = NULL;
    domainitem = NULL;
}

graInterface *gra_galaInterface::Copy()
{
    return NULL;
}

void gra_galaInterface::CloseWin()
{
}

void gra_galaInterface::DeletePicture()
{
}

void gra_galaInterface::CreatePicture()
{
    domain = new gravdomainLong;
}

void gra_galaInterface::ResetPositions(void)
{
    vdomainObjectSet  *connection_objects;
    vdomainObjectSet  *objectSet;
    vrect             rect;

    if(domain == NULL || domainitem == NULL)
	return;
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->DestroySelection(sel);
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    while(iter.Next()){
	vdomainObject *obj       = iter.GetObject();
	graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
	// reset original position first
	viewSymbolNode *vn       = (viewSymbolNode *)sym->get_viewnode();
	if(vn){
	    float fx, fy;
	    vn->get_position(fx, fy);
	    sym->set_position(fx, fy);
	}
    }
    vdomainDestroyObjectSet(objectSet);
}

void gra_galaInterface::RepositionObjects(void)
{
    vdomainObjectSet  *connection_objects;
    vdomainObjectSet  *objectSet;
    vrectLong         rect;
    int               width;
    int               height;

    if(domain == NULL || domainitem == NULL)
	return;
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->DestroySelection(sel);
    domain->CalcBoundingRectLong(&rect, objectSet);
    height = domainitem->GetHeight() - 20; // 20 is the size of scroll bar 
    width  = domainitem->GetWidth();
    int domain_w, domain_h;
    if(height > rect.h)
	domain_h = height;
    else
	domain_h = rect.h + 20;
    if(width > rect.w)
	domain_w = width;
    else
	domain_w = rect.w + 30;
    domain->Resize(domain_w, domain_h);
    int dx = 0        - rect.x;
    int dy = domain_h - rect.y - rect.h;
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    while(iter.Next()){
	vdomainObject *obj       = iter.GetObject();
	graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
	if(dx != 0)
	    sym->MoveDX(dx);
	if(dy != 0)
	    sym->MoveDY(dy);
    }
    domain->ShowObjects(objectSet);
//    domainitem->SetScrollTranslation(rect.x, rect.y, vdomainABSOLUTE);

//  Move all connection objects to the back
    connection_objects = vdomainCreateObjectSet(NULL);
    iter.Start(objectSet);
    while(iter.Next()){
	vdomainObject *obj       = iter.GetObject();
	graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
	if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType)
	    vdomainAddToObjectSet(&connection_objects, obj);
    }
    domain->LayerObjects(connection_objects, vdomainLAYER_TOBACK);
//    domain->LayerObjects(connection_objects, vdomainLAYER_TOFRONT);
    vdomainDestroyObjectSet(connection_objects);
    vdomainDestroyObjectSet(objectSet);
}

vdomainitem *ui_get_domain_item(void);  // Should be removed at the end of testing
void gra_galaInterface::CreateWin(viewGraHeader *header, long parent_window)
{
    if(parent_window == 0)
	domainitem = ui_get_domain_item();
    else
	domainitem = ((graViewLayer *)parent_window)->GetDomainItem();
    domainitem->SetDomain(domain);
    RepositionObjects();
    int view_type = header->get_type();
    domainitem->SetDragProhibited(view_type != Rep_ERD);
    domainitem->SetLiveSelection(vFALSE);
}

void gra_galaInterface::ManipulatePrespective(int op, int i1, int i2, int i3, int i4)
{
    vdomainObjectSet  *connection_objects;
    vdomainObjectSet  *objectSet;
    vrectLong         rect;
    int               width;
    int               height;
    connection_objects   = vdomainCreateObjectSet(NULL);
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->DestroySelection(sel);
    domain->CalcBoundingRectLong(&rect, objectSet);
    height = domainitem->GetHeight();
    width  = domainitem->GetWidth();
    if(op == VIEW_ZOOM_TO_FIT){
//	domainitem->SetOffscreenDrawing(0);
	vmatrix *scaleMatrix;
	vmatrix *matrix;
	width        -= 30;
	height       -= 20;
	float   rel_x = (float)width / (float)rect.w;
	float   rel_y = (float)height / (float)rect.h;
	if(rel_x > 1)
	    rel_x = 1;
	if(rel_y > 1)
	    rel_y = 1;
	if(rel_x < 0.0001)
	    rel_x = 0.01;
	if(rel_y < 0.0001)
	    rel_y = 0.01;
	matrix        = (class vmatrix *)vmatrix::GetIdent();
	scaleMatrix   = new vmatrix;
	matrix->Scale(rel_x, rel_y, scaleMatrix);
	domainitem->SetMatrix(scaleMatrix);
    } else if(op == VIEW_RESET) {
	vmatrix *scaleMatrix;
	vmatrix *matrix;
	matrix        = (class vmatrix *)vmatrix::GetIdent();
	scaleMatrix   = new vmatrix;
	matrix->Scale(1, 1, scaleMatrix);
	domainitem->SetMatrix(scaleMatrix);
    } else if(op == VIEW_ZOOM_XY) {
	vmatrix *scaleMatrix;
	vmatrix *matrix;
	matrix = domainitem->GetMatrix();
	if(matrix == NULL)
	    matrix = (class vmatrix *)vmatrix::GetIdent();
	scaleMatrix = new vmatrix;
	if(i1 > 0)
	    matrix->Scale(2, 2, scaleMatrix);
	else
	    matrix->Scale(0.5, 0.5, scaleMatrix);
	domainitem->SetMatrix(scaleMatrix);
    }
}

int gra_galaInterface::GetSliderStyle()
{
    return 0;
}

void gra_galaInterface::SetSliderStyle(int style)
{
}

int gra_galaInterface::WindowExists()
{
    return 0;
}

graParametricSymbol *gra_galaInterface::GetTextFocus()
{
    return NULL;
}

void gra_galaInterface::DamageArea(graParametricSymbol *sym)
{
    if(domainitem)
	domainitem->InvalView();
}

void gra_galaInterface::InvalidatePicture()
{
    RepositionObjects();
    if(domainitem)
	domainitem->InvalView();
}

void gra_galaInterface::RepaintView()
{
    ResetPositions();
    RepositionObjects();
}

void gra_galaInterface::RefreshWindow()
{
    vdomainObjectSet  *objectSet;
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->ShowObjects(objectSet);
    domain->DestroySelection(sel);
    vdomainDestroyObjectSet(objectSet);
}

void gra_galaInterface::Unhighlight()
{
}

void gra_galaInterface::InsertSymbol(graParametricSymbol *sym)
{
    vdomainObjectSet  *objectSet;
    vrectLong         rect;
    vdomainObject     *object;
    vcolorSpec	      spec;
    vcolor	      *color;
    int               x, y, w, h;

    sym->GetPosition(x, y, w, h);
    rect.x = x; rect.y = y; rect.w = w; rect.h = h;
    object = domain->AddObject();
    domain->SetObjectBoundsLong(vdomainObjectToSet(object),
			        &rect);
    domain->SetObjectData(vdomainObjectToSet(object), sym);
    sym->SetDomainAndObject(domain, object);
 
    if(domainitem){
	// Preserve selection 
	int layers = sym->get_layers();
	if((layers & graSymbolModeSelected) != 0){
	    // Disable all observers
	    vdomainObserverIterator iter;
	    iter.Start(domain);
	    while(iter.Next()){
		vdomainObserver *observer = iter.GetObserver();
		observer->Disable();
	    }
	    vdomainSelection sel = domainitem->GetSelection();
	    domain->SelectObjects(sel, vdomainObjectToSet(object));
	    // Enable observers
	    iter.Start(domain);
	    while(iter.Next()){
		vdomainObserver *observer = iter.GetObserver();
		observer->Enable();
	    }
	}
	domainitem->InvalView();
    }
}

void gra_galaInterface::RemoveSymbol(graParametricSymbol *sym)
{
    vdomainObject *object;

    if(sym->get_viewnode())
	driver_instance->unselect(sym->get_viewnode());
    object = sym->GetDomainObject();
    if(object)
	domain->RemoveObjects(vdomainObjectToSet(object));
}

void gra_galaInterface::RemovePictures(void)
{
}

int gra_galaInterface::GetWindowSize(int& width, int& height)
{
    return 0;
}

void gra_galaInterface::SelectSymbol(graParametricSymbol* symbol)
{
    if (symbol){
	int layers = symbol->get_layers();
	if((layers & graSymbolModeSelected) == 0){ // check that it is not selected already, to avoid recursive update
	    symbol->add_layers(graSymbolModeSelected);
	    symbol->hilite_modification_text();
	    
	    if(domain && domainitem){
		vdomainObject *obj = symbol->GetDomainObject();
		if(obj){
		  // Disable all observers
		       vdomainObserverIterator iter;
		  iter.Start(domain);
		  while(iter.Next()){
		    vdomainObserver *observer = iter.GetObserver();
		    observer->Disable();
		  }
		  vdomainSelection sel = domainitem->GetSelection();
		  domain->SelectObjects(sel, vdomainObjectToSet(obj));
		  domainitem->InvalObjects(vdomainObjectToSet(obj), vwindowINVAL_IMMEDIATE);
		  // Enable observers
		       iter.Start(domain);
		  while(iter.Next()){
		    vdomainObserver *observer = iter.GetObserver();
		    observer->Enable();
		  }
		}
	    }
	}
    }
}

void gra_galaInterface::UnselectSymbol(graParametricSymbol* symbol)
{
    if (symbol){
	symbol->unhilite_all();
	symbol->rem_layers(graSymbolModeSelected);
	if(domain && domainitem){
	    vdomainObject *obj = symbol->GetDomainObject();
	    if(obj)
		domainitem->InvalObjects(vdomainObjectToSet(obj), vwindowINVAL_IMMEDIATE);
	}
    }
}

void gra_galaInterface::HighliteSymbol(graParametricSymbol* symbol)
{
}

void gra_galaInterface::UnhighliteSymbol(graParametricSymbol* symbol)
{
}

void gra_galaInterface::FocusSymbol(graParametricSymbol* symbol)
{
}

void gra_galaInterface::ComputePagesToPrint(int& rows, int& cols)
{
}

float gra_galaInterface::ScaleToFitToPage()
{
    return 1.0;
}

int gra_galaInterface::SendPostscript(ostream& ostr)
{
    return 0;
}

/* ---------------------------------------------------------------------- */

NodePopupInfo::NodePopupInfo(void *cb_data)
{
    item_number   = 0;
    callback_data = cb_data;
}

NodePopupInfo::~NodePopupInfo()
{
    for(int i = 0; i < item_number; i++){
	if(row_text[i] != NULL)
	    free(row_text[i]);
    }
}

void *NodePopupInfo::GetCallbackData(void)
{
    return callback_data;
}

void NodePopupInfo::AddRow(int type, const char *text, void *callback)
{
    if(item_number < 50){
	row_text[item_number]     = text ? strdup(text) : NULL;
	row_type[item_number]     = type;
	row_callback[item_number] = callback;
	toggle_state[item_number] = 0;
	item_number++;
    }
}

void NodePopupInfo::SetToggleState(int state)
{
    if(item_number > 0){
	toggle_state[item_number - 1] = state;
    }
}

int NodePopupInfo::GetToggleState(int index)
{
    return toggle_state[index];
}

int NodePopupInfo::GetType(int index)
{
    return row_type[index];
}

char *NodePopupInfo::GetText(int index)
{
    return row_text[index];
}

void *NodePopupInfo::GetCallback(int index)
{
    return row_callback[index];
}

/* ---------------------------------------------------------------------- */

void gra_display_status_message(char *str)
{ 
    Viewer *vr;
    View   *v;
    Viewer::GetExecutingView (vr, v);
    if (vr && v)
	vr->DisplayMessage ((vchar*)str);
}

/* ---------------------------------------------------------------------- */

void pset_send_own_callback (void (*fn)(void *), void *dt)
{
    CallbackEvent *ev = new CallbackEvent;
    ev->func          = fn;
    ev->data          = dt;
    ev->SetPriority(veventPRIORITY_MIN);
    ev->Post();
}

/* ---------------------------------------------------------------------- */

int get_erd_builtin_rel_mask( int nCheckBox )
{
    int nMask = 0;
    switch( nCheckBox )
    {
      case 0:
	nMask = contains_rel;
	break;
      case 1:
	nMask = points_to_rel;
	break;
      case 2:
	nMask = refers_to_rel;
	break;
      case 3:
	nMask = method_type_rel;
	break;
      case 4:
	nMask = method_argument_rel;
	break;
      case 5:
	nMask = has_friend_rel;
	break;
      case 6:
	nMask = nested_in_rel;
	break;
      case 7:
	nMask = contained_in_rel;
	break;
      case 8:
	nMask = pointed_to_by_rel;
	break;
      case 9:
	nMask = referred_to_by_rel;
	break;
      case 10:
	nMask = returned_by_function_rel;
	break;
      case 11:
	nMask = is_argument_rel;
	break;
      case 12:
	nMask = is_friend_rel;
	break;
      case 13:
	nMask = container_of_rel;
	break;
    }
    return nMask;
}


int get_erd_members_mask( int nCheckBox )
{
    int nMask = 0;
    
    switch( nCheckBox )
    {
      case 0:
	nMask = show_methods;
	break;
      case 1:
	nMask = show_data;
	break;
      case 2:
	nMask = show_nested_types;
	break;
      case 3:
	nMask = show_class_members;
	break;
      case 4:
	nMask = show_instance_members;
	break;
      case 5:
	nMask = show_virtual_members;
	break;
      case 6:
	nMask = show_public_members;
	break;
      case 7:
	nMask = show_protected_members;
	break;
      case 8:
	nMask = show_private_members;
	break;
      case 9:
	nMask = show_inherited;
	break;
      case 10:
	nMask = show_arguments; 
	break;
      case 11:
	nMask = show_package_prot_members; 
	break;

    }
    return nMask;
}


int get_dc_builtin_rel_mask( int nCheckBox )
{
    int nMask = 0;
    switch( nCheckBox )
    {
      case 0:
	nMask = contains_rel;
	break;
      case 1:
	nMask = points_to_rel;
	break;
      case 2:
	nMask = contained_in_rel;
	break;
      case 3:
	nMask = pointed_to_by_rel;
	break;
    }
    return nMask;
}

#ifdef NEW_UI
// This implementation of push_busy_cursor(), pop_cursor() and reset_cursor()
// handles setting and resetting a busy cursor only.

static int nBlockCount = 0;
static void reset_busy_cursor()
{
  vapplication *app = vapplication::GetCurrent();
  if( app )
  {
    vapplicationWindowIterator iterator;               
    iterator.Start(app);
    while (iterator.Next())
    {
      vwindow *aWindow = iterator.GetWindow();
      if( aWindow->IsBlocked() )
        aWindow->Unblock();
    }
    iterator.Finish();
  }
}

extern "C" void push_busy_cursor()
{
  if( !nBlockCount )
  {
    vapplication *app = vapplication::GetCurrent();
    if( app )
    {
      vapplicationWindowIterator iterator;               
      iterator.Start(app);
      while (iterator.Next())
      {
        vwindow *aWindow = iterator.GetWindow();
        if( !aWindow->IsBlocked() )
          aWindow->Block();
      }
      iterator.Finish();
    }
  }
  nBlockCount++;
}

extern "C" void pop_cursor()
{
  if( nBlockCount == 1 )
    reset_busy_cursor();
  if( nBlockCount >=1 )
    nBlockCount--;
}

extern "C" void reset_cursor()
{
  nBlockCount = 0;
  reset_busy_cursor();
}
#endif /*NEW_UI*/
