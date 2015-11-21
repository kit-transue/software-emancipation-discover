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

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vresourceINCLUDED
#include vresourceHEADER
#endif

#ifndef vnameINCLUDED
#include vnameHEADER
#endif

#ifndef vtextINCLUDED
#include vtextHEADER
#endif

#include vmenuHEADER

#ifndef vprintINCLUDED
#include vprintHEADER
#endif

#undef printf
#undef sprintf
#undef vsprintf

#include <viewTypes.h>
#include <symbolArr.h>
#include <view.h>
#include <viewTreeHeader.h>
#include <geditorLayer.h>
#include <ggraViewLayer.h>
#include <gglobalFuncs.h>
#include <gview.h>
#include <gviewer.h>
#include <gdDrawingArea.h>
#include <graSymbol.h>
#include <viewGraHeader.h>
#include <viewHierarchyHeader.h>
#include <viewNode.h>
#include <dialog.h>
#include <driver.h>
#include <generic-popup-menu.h>
#include <viewSelection.h>
#include <Application.h>

class vfont;
extern vfont *ui_font;
extern void  gra_display_status_message(char *str);
extern view  *gra_create_view(symbolArr& symbols, int vt, void *view_layer);

extern "C" void push_busy_cursor(void);
extern "C" void pop_cursor(void);

vdomainObject *object_to_update = NULL;
static void BringToTop();

class myDomainObserver : public vdomainObserver
{
  public:
    vkindTYPED_FULL_DECLARATION(myDomainObserver);
    vloadableINLINE_CONSTRUCTORS(myDomainObserver, vdomainObserver);

    virtual void ObserveSelect(vdomainObjectSet *obj_set, vdomainSelection selection, int selected);
    virtual void ObserveMove(vdomainObjectSet *obj_set, vpoint *delta, int positioning, int before);
    virtual void ObserveMoveLong(vdomainObjectSet *obj_set, vpointLong *delta, int positioning, int before);
};

vkindTYPED_FULL_DEFINITION(myDomainObserver, vdomainObserver, "myDomainObserver");

class popup_vmenu : public vmenu 
{
  public:
    vkindTYPED_FULL_DECLARATION(popup_vmenu);
    vloadableINLINE_CONSTRUCTORS(popup_vmenu, vmenu);

    virtual void Close();
    virtual int  HandleButtonUp(vevent *event);

    void SetPopupInfo(NodePopupInfo *pi);

    NodePopupInfo *popup_info;
};

vkindTYPED_FULL_DEFINITION(popup_vmenu, vmenu, "popup_vmenu");

class popup_vmenuItem : public vmenuItem
{
  public:
    vkindTYPED_FULL_DECLARATION(popup_vmenuItem);
    vloadableINLINE_CONSTRUCTORS(popup_vmenuItem, vmenuItem);

    void SetCallbackData(void *c, void *d);

    void *callback;
    void *data;
};

vkindTYPED_FULL_DEFINITION(popup_vmenuItem, vmenuItem, "popup_vmenuItem");

class popup_vmenuToggleItem : public vmenuToggleItem
{
  public:
    vkindTYPED_FULL_DECLARATION(popup_vmenuToggleItem);
    vloadableINLINE_CONSTRUCTORS(popup_vmenuToggleItem, vmenuToggleItem);

    void SetCallbackData(void *c, void *d);

    void *callback;
    void *data;
};

vkindTYPED_FULL_DEFINITION(popup_vmenuToggleItem, vmenuToggleItem, "popup_vmenuToggleItem");

/* ============================================================================== */

void popup_vmenuItem::SetCallbackData(void *c, void *d)
{
    callback = c;
    data     = d;

}

/* ============================================================================== */

void popup_vmenuToggleItem::SetCallbackData(void *c, void *d)
{
    callback = c;
    data     = d;
}

/* ============================================================================== */

void popup_vmenu::SetPopupInfo(NodePopupInfo *pi)
{
    popup_info = pi;
}

void popup_vmenu::Close()
{
    delete popup_info;
    vmenu::Close();
    ((_vdomainviewGlobals *)*_vdomainviewMaster)->buttonDownState = 0;
}

int popup_vmenu::HandleButtonUp(vevent *ev)
{
    int result = vmenu::HandleButtonUp(ev);
    return result;
}

/* ============================================================================== */

typedef void (*popup_callback_proc)(void *, void *, void *);
typedef void (*toggle_callback_proc)(void *, void *, int);

struct popup_menu_data {
  void *proc;
  void *data;
  int  on;
};

static void popup_toggle_callback(void *d)
{
  popup_menu_data *data     = (popup_menu_data *)d;
  toggle_callback_proc proc = (toggle_callback_proc)data->proc;
  proc(NULL, data->data, data->on);
  delete data;
}

static void popup_normal_callback(void *d)
{
  popup_menu_data *data    = (popup_menu_data *)d;
  popup_callback_proc proc = (popup_callback_proc)data->proc;
  proc(NULL, data->data, NULL);
  delete data;
}

void popup_menu_pushbutton_observe(vmenuItem *i, vevent *event)
{

    if(i->IsToggle()){
        popup_vmenuToggleItem *item = (popup_vmenuToggleItem *)i;
        toggle_callback_proc proc   = (toggle_callback_proc)item->callback;
        if(proc != NULL) {
          popup_menu_data *data  = new popup_menu_data;
          data->proc             = proc;
          data->data             = item->data;
          data->on               = i->IsOn();
          callbackEvent *event   = new callbackEvent(popup_toggle_callback, data);
          if( event )
            event->Post();
        }
    } else {
        popup_vmenuItem *item    = (popup_vmenuItem *)i;
        popup_callback_proc proc = (popup_callback_proc)item->callback;
        if(proc != NULL){
          popup_menu_data *data = new popup_menu_data;
          data->proc            = proc;
          data->data            = item->data;
          callbackEvent *event  = new callbackEvent(popup_normal_callback, data);
          if( event )
            event->Post();
        }
    }
}

/* ============================================================================== */

void myDomainObserver::ObserveSelect(vdomainObjectSet *obj_set, vdomainSelection selection, int selected)
{
    vdomainObjectSetIterator iter;
    vdomain *domain = GetOwner();
    iter.Start(obj_set);
    while(iter.Next()){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym){
            viewNode *vn = sym->get_viewnode();
            if(vn) 
                if(selected){
                    driver_instance->select_node(vn);
                    genString status_line;
                    vn->description(status_line);
                    const char *pc = status_line.str();
                    if (pc == NULL)
                        pc = "";
                    gra_display_status_message((char*)pc);
                } else
                    driver_instance->unselect(vn);
        }
    }
}

void myDomainObserver::ObserveMove(vdomainObjectSet *obj_set, vpoint *delta, int positioning, int before)
{
    vdomainObjectSetIterator iter;
    vdomain *domain = GetOwner();
    iter.Start(obj_set);
    while(iter.Next()){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym && before == 0){
            viewNode *vn = sym->get_viewnode();
            float fx, fy;
            ((viewSymbolNode *)vn)->get_position(fx, fy);
            viewGraHeaderPtr vh = (viewGraHeader *)vn->get_header();
            vh->reposition_node((viewSymbolNode *)vn, fx + delta->GetX(), fy + delta->GetY());
            break;  // reposition just one node others will become invalid after first iteration
                    // because whole view gets regenerated
        }
    }
}

void myDomainObserver::ObserveMoveLong(vdomainObjectSet *obj_set, vpointLong *delta, int positioning, int before)
{
    vdomainObjectSetIterator iter;
    vdomain *domain = GetOwner();
    iter.Start(obj_set);
    while(iter.Next()){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym && before == 0){
            viewNode *vn = sym->get_viewnode();
            float fx, fy;
            ((viewSymbolNode *)vn)->get_position(fx, fy);
            viewGraHeaderPtr vh = (viewGraHeader *)vn->get_header();
            vh->reposition_node((viewSymbolNode *)vn, fx + delta->GetX(), fy + delta->GetY());
            break;  // reposition just one node others will become invalid after first iteration
                    // because whole view gets regenerated
        }
    }
}

/* ============================================================================== */

graViewLayer::graViewLayer (symbolArr& symbols, viewtype vt, View *view, const vchar *title) {

    gInit (graViewLayer::graViewLayer);

    m_pView  = NULL; //pointer to aset's view
    dlg      = NULL;
    symbols_ = symbols;
    vt_      = vt;

// Create a pane for the new layer.

    vresource paneRes;
    Pane *pn;
    if (getResource("DialogLayers:VIEW/sw", &paneRes)) pn = new Pane(paneRes);
    else pn = new Pane;
    setPane (pn);

//  Display the layer in the specified view
    displayInView (view);

// Get the drawing area widget pointer
    myDrawingArea *gd  = (view_) ? (myDrawingArea *)view_->FindItem ((const unsigned char *)"DrawingArea") : NULL;
    ui_font            = gd->DetermineFont();
    vwindow *win       = gd->DetermineWindow();
    if(win){
        if(!win->UsesMotion())
            win->SetMotion(vTRUE);   // Will handle motion events to do highlites
    }
    gd->SetOffscreenDrawing(1);

    m_pView = gra_create_view(symbols, vt, this);
    if(m_pView != NULL){
        dlg = new dialog("vs_dialog");
        dlg->set_view(0, m_pView);
        driver_instance->add_dialog(dlg);
        myDomainObserver *observer = new myDomainObserver();
        gd->domain->InstallObserverOwned(observer);
        title_ = m_pView->get_name();
    } else {
        title_ = "No view";
    }

    gString newTitle = "DIScover - ";
    newTitle += title_.str();
    view->GetViewer()->SetTitle (newTitle);

// Update the layer list so the new layer gets in the history menu.
    updateLayerList ();
    BringToTop();
}

graViewLayer::graViewLayer (view *vp, View *view, const vchar *title){
    gInit (graViewLayer::graViewLayer);
    
    m_pView  = vp;
 
   // Set correct view type (popup menus depend on this one)
    viewGraHeader *vgh = (viewGraHeader *)vp;   
    int view_type      = vgh->get_type();
    switch(view_type){
      case Rep_Tree:
        vt_ = viewCallTree;
        break;
      case Rep_FlowChart:
        vt_ = viewFlowchart;
        break;
      case Rep_VertHierarchy:
        vt_ = viewOutline;
        break;
      case Rep_OODT_Inheritance:
        vt_ = viewInheritance;
        break;
      case Rep_ERD:
        vt_ = viewERD;
        break;
      case Rep_DataChart:
        vt_ = viewDatachart;
        break;
      case Rep_SubsystemMap:
        vt_ = viewSubsysBrowser;
        break;
      case Rep_DGraph:
        vt_ = viewGraph;
        break;
    };
    
    title_ = vp->get_name();
    // Create a pane for the new layer.
    vresource paneRes;
    Pane *pn;
    if (getResource("DialogLayers:VIEW/sw", &paneRes)) pn = new Pane(paneRes);
    else pn = new Pane;
    setPane (pn);

//  Display the layer in the specified view

    displayInView (view);

// Get the drawing area widget pointer
    myDrawingArea *gd  = (view_) ? (myDrawingArea *)view_->FindItem ((const unsigned char *)"DrawingArea") : NULL;
    ui_font            = gd->DetermineFont();
    vwindow *win       = gd->DetermineWindow();
    if(win){
        if(!win->UsesMotion())
            win->SetMotion(vTRUE);   // Will handle motion events to do highlites
    }
    gd->SetOffscreenDrawing(1);
    m_pView->open_window((long)this);
    dlg = new dialog("vs_dialog");
    dlg->set_view(0, m_pView);
    driver_instance->add_dialog(dlg);
    myDomainObserver *observer = new myDomainObserver();
    gd->domain->InstallObserverOwned(observer);

// Update the layer list so the new layer gets in the history menu.
    updateLayerList ();
    BringToTop();
}

graViewLayer::~graViewLayer () {
    gInit (graViewLayer::~graViewLayer);

    removeFromView ();
    setView (NULL);
    if (dlg)
        driver_instance->close_dialog(dlg);
}

vdomainitem *graViewLayer::GetDomainItem(void)
{
    myDrawingArea *gd = (view_) ? (myDrawingArea *)view_->FindItem ((const unsigned char *)"DrawingArea") : NULL;

    return gd;
}

void graViewLayer::getName (gString& nm) const {
    gInit (graViewLayer::getName);

    nm = title_;

    // if (viewlayer_) nm = viewlayer_->getTitle();
    // else nm = "Unknown Graphic View";
}

void graViewLayer::getResourceName (gString& nm) const {
    gInit (graViewLayer::getResourceName);

    getName(nm);
}


const vname *graViewLayer::getState (void) const {
    gInit (graViewLayer::getState);

    switch (vt_) {
      case viewInheritance:
        return vnameInternGlobalLiteral ("Inheritance");
      case viewCallTree:
        return vnameInternGlobalLiteral ("CallTree");
      case viewERD:
        return vnameInternGlobalLiteral ("Relations");
      case viewSubsysBrowser:
        return vnameInternGlobalLiteral ("SubsysBrowser");
      case viewDatachart:
        return vnameInternGlobalLiteral ("DataChart");
      default:
        return vnameInternGlobalLiteral ("GraphicalView");
    }
}


void graViewLayer::display (void) {
    gInit (graViewLayer::display);

    View *v = getView();
    if (!v) return;

    Pane *pn = getPane ();
    if (!pn) return;

    v->SetPane (pn);
}

void graViewLayer::undisplay (void) {
    gInit (graViewLayer::undisplay);
}


const char *graViewLayer::findPopupMenu (const int x, const int y) {

    gInit (graViewLayer::findPopupMenu);

        // for right now, get a resource that will have the menu in it.  This will allow
        // us to test at home.

    vresource popupRes;
    if (getResource("DialogLayers:VIEW/sw:testPopup", &popupRes)) {
        vtext *popupName = new vtext (popupRes);
        long len = popupName->GetLength();
        vchar *ptr = new vchar[len+1];
        popupName->GetTextRange (0, len, ptr, len);
        ptr[len] = '\0';
        return (const char *)ptr;
    }
    else {
        return "";
     }

}

view *dis_get_gra_view()
{
    Viewer *vr;
    View *v;
    view *pResult = NULL;
    Viewer::GetExecutingView(vr, v);
    if( v ){
        graViewLayer *pLayer = (graViewLayer *)v->GetLayer();
        if( pLayer )
            pResult = pLayer->GetGraView();
    }
    return pResult;
}

vkindTYPED_FULL_DEFINITION(myDrawingArea, vdomainitem, "myDrawingArea");

void myDrawingArea::Open()
{
    vdomainitem::Open();

    dialog_ = GDialog::CastDown(DetermineDialog());
    InitgdWidget((vchar*)GetTag(), this);
}

void myDrawingArea::Show()
{
    vwindow *win = DetermineWindow();
    if(win)
        win->SetMotion(vTRUE);   // Will handle motion events to do highlites
}

void myDrawingArea::getOffset (int *inx, int *iny) {

    int x = GetX();
    int y = GetY();

    Pane *p = pane_;
    while (p) {
        x += p->GetX();
        y += p->GetY();
        View *owner = p->GetOwner();
        View *parent = owner ? owner->GetParentView() : NULL;
        p = parent ? parent->GetPane() : NULL;
    }
   
    *inx = x;
    *iny = y;
}

int myDrawingArea::HandleMotion(vevent *event)
{
    vpointLong           pt;
    vdomainObjectSet     *objectSet;
    static vdomainObject *last_highlited = NULL;
   
    if(domain == NULL)
        return vTRUE;
    pt.SetX(event->GetX());
    pt.SetY(event->GetY());
    TranslatePointLong(&pt, vdomainviewTO_DOMAIN);

    if(last_highlited){
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(last_highlited);
        if(sym){
            sym->undo_highlight();
        }
        object_to_update = last_highlited;
        InvalObjects(vdomainObjectToSet(last_highlited), vwindowINVAL_OPAQUE);
//      Inval(vwindowINVAL_OPAQUE);
        last_highlited = NULL;
    }
    vrectLong *view_area = GetViewAreaLong();
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectsInAreaLong(view_area, sel, vFALSE, vFALSE, vdomainALL, NULL);
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    while(iter.Next()){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym){
            if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType){
                int gra_cmd_index = sym->PointNear(pt.GetX(), pt.GetY(), 5);
                if(gra_cmd_index >= 0){
                    last_highlited = obj;
                    sym->do_highlight();
                    object_to_update = obj;
                    InvalObjects(vdomainObjectToSet(obj), vwindowINVAL_OPAQUE);
//                  Inval(vwindowINVAL_OPAQUE);
                    break;
                }
            }
        }
    }
    domain->DestroySelection(sel);
    vdomainDestroyObjectSet(objectSet);
    return vTRUE;
}

vbool myDrawingArea::HandleConnectionClick(graParametricSymbol *sym, int button, int gra_cmd_index)
{
    if(button == 0){
                if(sym==NULL) return vFALSE;
            viewSymbolNodePtr viewnode = sym->get_viewnode();
                if(viewnode==NULL) return vFALSE;
        genString status_line;
        viewnode->description(status_line);
        const char *pc = status_line.str();
        if (pc == NULL) pc = "";
        gra_display_status_message((char*)pc);
        return vFALSE;
    } else if(button == 1) {
            graAttachmentPoint  attach;
            viewFuncCallNode*   vfcn;
            viewMultiNode*      vmn;
            viewNode*           vn;
            viewNode*           cn;
            int                     atn, regen_flag = 0;
                if(sym==NULL) return vFALSE;
                    // do nothing for ERD and DATA CHART
            viewSymbolNodePtr viewnode = sym->get_viewnode();
                if(viewnode==NULL) return vFALSE;
            viewGraHeader *vgh         = (viewGraHeader *)viewnode->get_header();
                if(vgh==NULL) return vFALSE;
            int view_type              = vgh->get_type();
            if (view_type == Rep_ERD || view_type == Rep_DataChart || view_type == Rep_SubsystemMap)
            return vFALSE;
        
            viewSelection selection;
            sym->find_insertion(gra_cmd_index, &selection);
        
            cn = (viewNode *)selection.connector;
            if ((vn = (viewNode *)selection.view_node) != NULL)
               vgh = (viewGraHeader *)vn->get_header();
            start_transaction() {
            if (vn != NULL && cn == NULL) {
                //-----------------------------
                // clicked branch or decoration
                //-----------------------------
                if (vn->viewIsKindOf (viewMultiType) &&
                    selection.sub_type >= graLayerNoDecorationLL &&
                    selection.sub_type <= graLayerNoDecorationUR) {
                    vmn    = (viewMultiNode *)vn;
                    atn    = selection.sub_type;
                    attach = (graAttachmentPoint) (atn - graLayerNoDecorationLL);
                    if (vmn->viewIsKindOf(viewFuncCallType)) {
                        
                        //-------------------------------------
                        // special handling for call tree nodes
                        //-------------------------------------
                        
                        regen_flag = 1;
                        vfcn       = (viewFuncCallNode *)vmn;
                        switch (attach) {
                          case graAttachLR:
                            vfcn->unprune_branches (graAttachCR);
                            regen_flag = 0;
                            break;
                          case graAttachLL:
                            vfcn->unprune_branches (graAttachCL);
                            regen_flag = 0;
                            break;
                          case graAttachCR:
                          case graAttachCL:
                            regen_flag = vfcn->toggle_display_of (attach);
                            break;
                          default:
                            break;
                        }
                    }
                    else regen_flag = vmn->toggle_display_of (attach);
                }
                if (regen_flag == 0)
                    vgh->hierarchy_control (vn);
            }
            else if ((vn != NULL) && (cn != NULL)) {
                if (vn->viewIsKindOf (viewFuncCallType)) {
                    //-----------------------------------
                    // clicked sub-branch; let's prune it
                    //-----------------------------------
                    viewFuncCallNode*   upper_vfcn;
                    viewConnectionNode* vcn;
                    objArr*                     inputs;
                    Obj*                        ob;
                    
                    vfcn        = (viewFuncCallNode *)vn;
                    vcn         = (viewConnectionNode *)cn;
                    inputs      = vcn->get_inputs ();
                    
                    //-----------------------------------------------
                    // assume sub-branch connector has only one input
                    //-----------------------------------------------
                    ForEach (ob, *inputs) {
                        if (!ob->relationalp())
                            continue;
                        if (!is_viewFuncCallNode ((Relational*) ob))
                            continue;
                        upper_vfcn = (viewFuncCallNode *)ob;
                        upper_vfcn->prune_branch (vfcn);
                        vgh->hierarchy_control (vn);
                        break;
                    }
                }
            }
        } end_transaction();
    }
    return vFALSE;
}

vbool myDrawingArea::HandleDecorationClick(graParametricSymbol *sym, int button, int decoration_clicked)
{
    viewNode *vn = sym->get_viewnode();
    if(vn){
    push_busy_cursor();
        viewGraHeader *vgh = (viewGraHeader *)vn->get_header();
        int view_type      = vgh->get_type();
        if(view_type == Rep_Tree || view_type == Rep_OODT_Inheritance){
            viewFuncCallNode *vfcn = (viewFuncCallNode *)vn;
            switch(decoration_clicked){
              case graLayerNoDecorationCR:
                vfcn->toggle_display_of(graAttachCR);
                vgh->hierarchy_control (vn);
                break;
              case graLayerNoDecorationCL:
                vfcn->toggle_display_of(graAttachCL);
                vgh->hierarchy_control (vn);
                break;
              default:
                printf("Unsupported decoration click\n");
                break;
            }
        }
        pop_cursor();
  return vFALSE;
    }
    return vTRUE;
}

typedef void (*funcptr)();
static int add_children_to_menu(vmenu *parent, NodePopupInfo *popup_info, int start_index);
static vmenu *make_submenu(NodePopupInfo *popup_info, int &index)
{
    static int subnum = 0;

    vmenu *menu = new vmenu;
    index++;
    index = add_children_to_menu(menu, popup_info, index);
    return menu;
}

static int add_children_to_menu(vmenu *parent, NodePopupInfo *popup_info, int start_index)
{
    int num = 0;

    for(int i = start_index; i < popup_info->item_number && popup_info->GetType(i) != END_SUBMENU; i++){
        vmenuItem *item = NULL;
        if(popup_info->GetType(i) == SEPARATOR){
            item = new vmenuSeparatorItem();
        } else {
            char* callback_type = NULL;

            char *label = popup_info->GetText(i);
            if(label == NULL  ||  label[0] == '\0'){
                /* Just in case someone forgot to name it. */
                label = "";
            }
            switch(popup_info->GetType(i)){
              case TOGGLE: {
                  item = new popup_vmenuToggleItem;
                  item->SetTitle((vchar *)label);
                  funcptr callback    = (funcptr)popup_info->GetCallback(i);
                  void *callback_data = popup_info->GetCallbackData();
                  ((popup_vmenuToggleItem *)item)->SetCallbackData(callback, callback_data);
                  item->SetObserveMenuItemProc(popup_menu_pushbutton_observe);
                  item->SetOn(popup_info->GetToggleState(i));
                }
                break;
              case PUSH: {
                  item = new popup_vmenuItem;
                  item->SetTitle((vchar *)label);
                  funcptr callback    = (funcptr)popup_info->GetCallback(i);
                  void *callback_data = popup_info->GetCallbackData();
                  ((popup_vmenuItem *)item)->SetCallbackData(callback, callback_data);
                  item->SetObserveMenuItemProc(popup_menu_pushbutton_observe);
                }
                break;
              case CASCADE:
                item = new vmenuSubMenuItem;
                item->SetTitle((vchar *)label);
                item->SetSubMenu(make_submenu(popup_info, i));
                break;
            }
        }
        if(item)
            parent->AppendItem(item);
    }
    return i;
}

graParametricSymbol *myDrawingArea::FindConnectionSymbol(vdomainviewButtonDownEvent *event, int &gra_cmd_index)
{
    graParametricSymbol *ret = NULL;
    vdomainObjectSet    *objectSet;

    vpoint pos           = event->position;
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->DestroySelection(sel);
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    int x = pos.GetX();
    int y = pos.GetY();
    while(iter.Next() && ret == NULL){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType){
            gra_cmd_index = sym->PointNear(x, y, 5);
            if(gra_cmd_index >= 0)
                ret = sym;
        }
    }
    return ret;
}

vbool myDrawingArea::HandleButtonDownEvent(vdomainviewButtonDownEvent *event)
{
    int button = event->event->GetButton();
    if(button == 2){
            // For all connection symbols (except outline view) simulate middle button click (button == 1)
            NodePopupInfo *popup_info = NULL;
            if(event->clickedOnObject != NULL){
                graParametricSymbol *sym  = (graParametricSymbol *)domain->GetObjectData(event->clickedOnObject);
                if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType){
                        int gra_cmd_index;
                        sym = FindConnectionSymbol(event, gra_cmd_index);
                        if(sym){
                            view *view_header = sym->get_viewnode()->get_header();
                            if(is_viewHierarchyHeader(view_header))
                                popup_info = sym->generate_popup();
                            else { // simulate middle button
                                    return HandleConnectionClick(sym, 1, gra_cmd_index);
                                        }
                                }
                        } else
                        popup_info = sym->generate_popup();
                } else {
                int gra_cmd_index;
                graParametricSymbol *sym = FindConnectionSymbol(event, gra_cmd_index);
                if(sym){
                        view *view_header = sym->get_viewnode()->get_header();
                                if(view_header) {
                            if(is_viewHierarchyHeader(view_header))
                                popup_info = sym->generate_popup();
                            else { // simulate middle button
                                return HandleConnectionClick(sym, 1, gra_cmd_index);
                                        }
                                }
                        }
                }
            if(popup_info){
                popup_vmenu *menu = new popup_vmenu;
                menu->SetPopupInfo(popup_info);
                add_children_to_menu(menu, popup_info, 0);
                vpoint pt(event->position);
                TranslatePoint(&pt, vdomainviewTO_DIALOG);
                vrect rect;
                rect.x = pt.GetX();
                rect.y = pt.GetY();
                rect.w = 1;
                rect.h = 1;
                int pane_x, pane_y;
                getOffset(&pane_x, &pane_y);
                rect.x += pane_x;
                rect.y += pane_y;
                menu->PlaceRight(DetermineWindow(), &rect);
                //              menu->Open();
                menu->Popup(event->event);
                return vFALSE;
                }
            return vTRUE;
    }
    vpoint pos = event->position;
    if(event->clickedOnObject != NULL){
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(event->clickedOnObject);
        if(sym && (sym->SymbolType() == graParametricSymbolType)) {
            int decoration_clicked = sym->DecorationPoint(pos.GetX(), pos.GetY());
            if(decoration_clicked == -1)
                     return vFALSE;
            if(decoration_clicked == graLayerNoNormal || decoration_clicked == graLayerNoNote || 
               decoration_clicked == 0xFFF)
                     return vTRUE; // Go ahead with normal selection
            return HandleDecorationClick(sym, button, decoration_clicked);
        }
    }
    
    int gra_cmd_index;
    graParametricSymbol *sym = FindConnectionSymbol(event, gra_cmd_index);
    if(sym)
             return HandleConnectionClick(sym, button, gra_cmd_index);

    return vTRUE; // process further events
}

void myDrawingArea::DrawObjects(void)
{
    vdomainObjectSet *objectSet;
    vdomain          *theDomain = GetDomain();

    if(domain == NULL)
        return;

#if 0
    if(object_to_update){
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(object_to_update);
        if(sym){
            int cx, cy, w, h;
            sym->Draw(cx, cy, w, h, TRUE);
        }
        object_to_update = NULL;
    } else {
        vrectLong *view_area = GetViewAreaLong();
        vdomainSelection sel = theDomain->CreateSelection();
        objectSet            = theDomain->GetObjectsInAreaLong(view_area, sel, vFALSE, vFALSE, vdomainALL, NULL);
        vdomainObjectSetIterator iter;
        iter.Start(objectSet);
        while(iter.Next()){
            vdomainObject *obj       = iter.GetObject();
            graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
            if(sym){
                int cx, cy, w, h;
                sym->Draw(cx, cy, w, h, TRUE);
            }
        }
        //    vdraw::MoveTo(view_area->x + 100, view_area->y + 50);
        //    vdraw::LineTo(view_area->x + 200, view_area->y + 50);
        //    vdraw::Stroke();
        theDomain->DestroySelection(sel);
        vdomainDestroyObjectSet(objectSet);
    }
#endif

    vdraw::SetLineWidth(1);
    vrectLong *view_area = GetViewAreaLong();
    vdomainSelection sel = theDomain->CreateSelection();
    objectSet            = theDomain->GetObjectsInAreaLong(view_area, sel, vFALSE, vFALSE, vdomainALL, NULL);
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    while(iter.Next()){
        vdomainObject *obj       = iter.GetObject();
        graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
        if(sym){
            int cx, cy, w, h;
            sym->Draw(cx, cy, w, h, TRUE);
        }
    }
    //    vdraw::MoveTo(view_area->x + 100, view_area->y + 50);
    //    vdraw::LineTo(view_area->x + 200, view_area->y + 50);
    //    vdraw::Stroke();
    theDomain->DestroySelection(sel);
    vdomainDestroyObjectSet(objectSet);
}

void myDrawingArea::Print(vprint *printer)
{
        Viewer *vr;
    View *v;
    Viewer::GetExecutingView(vr, v);
        MyDialog *dialog = vr->GetDialog();
        Pane *pane = v->GetPane();

        vdomain *theDomain = GetDomain();
        vrectLong bound; 
        bound.Copy(theDomain->GetBoundsLong());

        vmatrix *matrix = GetMatrix();
        float AA,DD;
        AA=DD=1.0;
        if(matrix) {
                AA = matrix->GetA();
                DD = matrix->GetD();
        }

        vrect paper_size;
        paper_size.CopyFloat(printer->GetPaper());

        int page_w,page_h;
        page_w = paper_size.GetWidth()-20;
        page_h = paper_size.GetHeight()-50;

        vpointLong *pt = GetScrollTranslation();;
        int orgX = pt->GetX();
        int orgY = pt->GetY();

        vrectLong newBound;
        long newBound_w,newBound_h;
        newBound_w = bound.GetWidth()+page_w/AA;
        newBound_h = bound.GetHeight()+(page_h+30)/DD;
        newBound.Set(bound.GetX()-12/AA,bound.GetY()-40/DD,newBound_w,newBound_h);
        vbool bVBound = theDomain->GetVirtualBounds();
        theDomain->SetVirtualBounds(0);
        theDomain->SetBoundsLong(&newBound);

        int page_x,page_y;
        page_x = newBound_w*AA/page_w;
        page_y = newBound_h*DD/page_h;
        
        const vrect *vr_rect=dialog->GetRect();
        vr->ResizeWindow(0/*dummy*/,0/*dummy*/,page_h+30,page_w+10);

        vbool bHorzBar = HasHorzBar();
        vbool bVertBar = HasVertBar();
        SetHorzBar(FALSE);
        SetVertBar(FALSE);
        
        for(int i=0; i<page_y; i++) {
                for(int j=0; j<page_x; j++) {
                        printer->OpenPage();
                        Scroll((j*page_w-12)/AA,(i*page_h-12)/DD,vdomainABSOLUTE);
                        pane->Draw();
                        printer->ClosePage();
                }
        }

        SetHorzBar(bHorzBar);
        SetVertBar(bVertBar);

        vr->ResizeWindow(0/*dummy*/,0/*dummy*/,vr_rect->GetHeight(),vr_rect->GetWidth());
        theDomain->SetBoundsLong(&bound);
        Scroll(orgX,orgY,vdomainABSOLUTE);

        theDomain->SetVirtualBounds(bVBound);
}



void BringToTop()
{
  Viewer *pViewer = Viewer::GetFirstViewer();
  if( pViewer )
    pViewer->bring_to_top();

}
