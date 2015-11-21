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
// graAction.C
//------------------------------------------
// synopsis:
// Implements X Intrinsics Actions for graphics window
//------------------------------------------

// INCLUDE FILES

// NOTE: The order of the following include files is CRITICAL because of
// macros used by InterViews to control name-space conflicts.
//  (actually, the order should no longer matter because of the
//   InterViews scoping fixup. -jef)
#include <general.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <x11_intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Object.h>

#include <rubberGraphic.h>

#include <graWindow.h>
#include <graSymbol.h>

#include <InterViews/enter-scope.h>
#include <InterViews/rubline.h>
#include <InterViews/perspective.h>
#include <InterViews/transformer.h>
#include <InterViews/event.h>
#include <IV-X11/xevent.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>
#include <InterViews/painter.h>

// The order of the remaining include files is less critical.

#include <genError.h>
#include <viewNode.h>
#include <graInterface.h>
#include <gra_ivInterface.h>
#include <viewGraHeader.h>
#include <viewERDHeader.h>
#include <viewMulticonnHeader.h>
#include <viewSelection.h>
#include <driver.h>
#include <button.h>
#include <ui.h>
#include <viewERDClassNode.h>
#include <viewerShell.h>
#include <ddict.h>
#include <oodt_ui_decls.h>
#include <systemMessages.h>
#include <dd_or_xref_node.h>

#include <generic-popup-menu.h>

#include <cmd.h>
#include <cmd_enums.h>

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#ifndef _groupTree_h
#include <groupTree.h>
#endif

#ifndef _popup_QueryAndFetch_h
#include <popup_QueryAndFetch.h>
#endif

#include <customize.h>

// definitions stolen from Motif header files:

//#define XmNtranslations		"translations"
//#define XmNuserData		"userData"


// definitions stolen from UIM/X header files:

extern Display* UxDisplay;
extern XtAppContext UxAppContext;


// EXTERNAL FUNCTIONS

extern int      cmd_gra_set_cur_view(Widget);
extern int      gra_type();

graParametricSymbol* gra_find_selection(graWindow*, int x, int y, viewSelection&);
extern "C" int getmagfact();
void gra_drag_symbol(viewSelection*, iv3(Event)&);
void gra_line_select(iv3(Event)&, iv2(Coord)&, iv2(Coord)&, iv2(Coord)&, iv2(Coord)&);
void gra_rect_select(iv3(Event)&, iv2(Coord)&, iv2(Coord)&, iv2(Coord)&, iv2(Coord)&);


// CLASS STATIC VARIABLES

rubberGraphic*	graWindow::xor_box;


// FILE STATIC VARIABLES

static graWindow* highlight_window;
static graWindow* focus_window;

static graWindow* pending_focus_window = NULL;
static graParametricSymbol* pending_focus_symbol= NULL;

static graWindow* user_data;	// Used by widget_to_this().
static Arg arg_user_data = { XmNuserData, XtArgVal(&user_data) };

static viewSelection pickup_sel; // Used by pickup/carry/drop.
static iv3(Rubberband)* carry_feedback;
static float carry_x_offset;
static float carry_y_offset;

static char default_translations[] = "\
    None	<Btn1Down>:		modal_click()		\n\
    Shift	<Btn1Down>:		select(add)		\n\
    Ctrl	<Btn1Down>:		select(toggle)		\n\
		<Btn2Down>:		collapse_expand()	\n\
    Ctrl	<Btn3Down>:		recenter()		\n\
		<Btn3Down>:		symbol_menu()		\n\
    None	<Btn1Motion>:		begin_connection()	\n\
    None	<Btn2Motion>:		pickup()		\n\
    Ctrl	<Btn2Motion>:		zoom_rectangle()	\n\
		<Motion>:		set_highlight()		\n\
		<Enter>:		set_highlight()		\n\
		<Leave>:		clear_highlight()	\n\
";

static char default_translations_no_Btn3[] = "\
    None	<Btn1Down>:		modal_click()		\n\
    Shift	<Btn1Down>:		select(add)		\n\
    Ctrl	<Btn1Down>:		select(toggle)		\n\
		<Btn2Down>:		collapse_expand()	\n\
    None	<Btn1Motion>:		begin_connection()	\n\
    None	<Btn2Motion>:		pickup()		\n\
    Ctrl	<Btn2Motion>:		zoom_rectangle()	\n\
		<Motion>:		set_highlight()		\n\
		<Enter>:		set_highlight()		\n\
		<Leave>:		clear_highlight()	\n\
";


// FUNCTION DEFINITIONS

objTree * cmd_get_child(objTree *t, int ind)
{
    if(!t)
	return 0;
    if(ind == -99)
	return t;
    if(ind < 0)
	return 0;
    objTree * cur = t->get_first();
    if (!cur)
	return 0;
    for(int ii=1; ii<=ind; ++ii)
    {
	cur = cur->get_next();
	if(!cur)
	    break;
    }
    return cur;
}


int cmd_get_ind(objTree * t)
{
    Initialize(cmd_get_ind);

    if(!t)
	return -1;

    objTree * root = t->get_parent();
    if(!root)
	return -99;
    Assert(! root->get_parent());

    int ind = -1;
    while(t)
    {
	t = t->get_prev();
	++ ind;
    }
    return ind;
}


static     viewSelection cur_view_sel;
static     XEvent        cur_ev;

extern "C" void cmd_moveto_ev(XEvent * e);


static   graParametricSymbol * update_cur_sel();



//------------------------------------------
// widget_to_this
//------------------------------------------

graWindow* widget_to_this(Widget widget)
{
    Initialize(widget_to_this);

    XtGetValues(widget, &arg_user_data, 1);

    return(user_data);
}

graParametricSymbol* cmd_find_select(
    graWindow*gw, XEvent*e, viewSelection& sel)
{
    Initialize(cmd_find_select);

    graParametricSymbol * gs;

    if(e != &cur_ev)
    {
	int ymax = gw->getymax();
	gs = gra_find_selection(gw, e->xmotion.x, ymax - e->xmotion.y, sel);

    }
    else
    {
        gs = update_cur_sel();
	sel = cur_view_sel;
    }

    return gs;
}

//static viewer * cur_viewer;
extern viewer * cur_viewer;

static struct {
 int view_node;
 int connector;
}  cmd_sel_index;

viewTree * cmd_get_node(objTree * root, int ind)
{
  return
     viewTreePtr(cmd_get_child(root, ind));
}

static   graParametricSymbol * update_cur_sel()
{
    view * cur_view = cur_viewer->get_view_ptr();
    objTree * root = cur_view->get_root();

    cur_view_sel.view_node = cmd_get_node(root, cmd_sel_index.view_node);
    cur_view_sel.connector = cmd_get_node(root, cmd_sel_index.connector);

    graParametricSymbol * gs=0;
    if(cur_view_sel.view_node)
    {
	if(is_viewSymbolNode(cur_view_sel.view_node))
	    gs = viewSymbolNodePtr(cur_view_sel.view_node) ->get_picture();
    }
    else if(cur_view_sel.connector)
    {
//	if(is_viewConnectionNode(cur_view_sel.connector))
//	    gs = viewConnectionNodePtr(cur_view_sel.connector) ->get_picture();
    }

    return gs;
}

void cmd_compute_delta(graWindow * gw, XEvent *e, viewTree* view_node, 
		       viewTree * connector, float& dx, float& dy)
{
    Initialize(cmd_compute_delta);

    graParametricSymbol * gs=0;
    if(view_node && (view_node->get_id() > 0))
    {
	if(is_viewSymbolNode(view_node))
	    gs = viewSymbolNodePtr(view_node) ->get_picture();
    }
    else if(connector && connector->get_id() > 0)
    {
	if(is_viewConnectionNode(connector))
	    gs = viewConnectionNodePtr(connector) ->get_picture();
    }

    int l,b,r,t;
    if(gs)
    {
	gs->GetBox(l, b, r, t);

	dx = (e->xmotion.x - l) / (float)(r - l);
	dy = (gw->getymax() - b - e->xmotion.y) / (float) (t - b) ;
    }
    else
    {
	dx = 0;
	dy = 0;
	l = b = r = t = 0;
    }

    DBG
    {
	msg(" Box lbrt $1 $2 $3 $4 dx $5 dy $6\n") << l << eoarg << b << eoarg << r << eoarg << t << eoarg << dx << eoarg << dy << eom;
    }
}     

extern int cmd_rerecording;
static int find_description(int is_rerecording, viewSymbolNodePtr vn, genString& descr)
{

    if(!vn) 
      return 0;

    if(is_rerecording || cmd_available_p(cmdGraDescription)){
      vn->description(descr);

      char * dd = (char*) descr;
      if(dd)  {
	while(dd[0] == ' ') ++ dd;
	if(dd[0] && dd[0] != ' ') {
	  descr = dd;
	  return 1;
	}
      }
    }

    ostrstream os;
    vn->send_string(os);
    os << ends;
    char*txt = os.str();

    if(!txt)
       return 0;

    descr = txt;
    delete txt;
    return 2;
}

static void cmd_record_action(int is_rerecording, const char *action, 
      const char *arg, int node_ind, int con_ind, int sub, float dx, 
      float dy, viewSymbolNode* vn)
{
    genString descr;
    find_description(is_rerecording, vn, descr);

    char * txt = (char*)descr;
    char buf[1024];
   
    if(arg)
	OSapi_sprintf(buf, "gra_%s(%d, %d, %d, %2.3f, %2.3f, \"%s\"", 
		action, node_ind, con_ind, sub, dx, dy, arg);
    else
	OSapi_sprintf(buf, "gra_%s(%d, %d, %d, %2.3f, %2.3f, 0", 
		action, node_ind, con_ind, sub, dx, dy);

    char * bbb = buf + strlen(buf);
    if(txt){
        if(strlen(txt) > 128)
               txt[128] = '\0';
	OSapi_sprintf(bbb, ", \"%s\");", txt);
    } else {
	OSapi_sprintf(bbb, ", 0);");
    }
    cmd_gen_write_force(buf);

}

static void cmd_gra_before_action(const char* action, graWindow * gw, 
     Widget w, XEvent* e, char** p, int)
{
    Initialize(cmd_gra_before_action);

    char * arg = p ? *p : 0;

    if((e->type != ButtonPress) &&
       (e->type != ButtonRelease) &&
       strcmp(action,"pickup") && strcmp(action, "begin_connection")
       && strcmp(action,"end_connection"))
	return;

    cmd_gra_set_cur_view(w);

    viewSelection view_sel;
    cmd_find_select(gw, e, view_sel);

    viewSymbolNode * vn = viewSymbolNodePtr(view_sel.view_node);
    
    viewTree * vc = view_sel.connector;
    int sub = view_sel.sub_type;
   
    int node_ind = cmd_get_ind(vn);
    int con_ind = cmd_get_ind(vc);

    float dx, dy;

    cmd_compute_delta(gw, e, vn, vc, dx, dy);
 
    cmd_record_action(0, action, arg, node_ind, con_ind, sub, dx, dy, vn);
  }


static int verify_ajust_sn(int sn, char* txt, viewPtr cur_view)
{

 if(sn < 0) 
    return sn;

 Initialize(verify_ajust_sn);

 objTree * root = cur_view->get_root();
 Assert(root);

 viewTreePtr view_node = cmd_get_node(root, sn);
 genString err_txt;
 if(!view_node || ! is_viewSymbolNode(view_node)){
   err_txt.printf ("Wrong selection at sn = %d", sn);
   cmd_validation_error ((char *)err_txt);
   return sn;
 }

 if (!(cmd_validate_p() && cmd_available_p(cmdGraValidation)))
    return sn;

 if(! txt)
    return sn;

 viewSymbolNodePtr vn = viewSymbolNodePtr(view_node);
 genString descr;
 find_description(0, vn, descr);
 
 const char *new_txt = descr.str();


 if (strcmp(txt, new_txt)){
   int found = 0;
   if(cmd_rerecording){
     viewSymbolNodePtr vnarr[2];
     vnarr[0] = vnarr[1] = vn;
     for(int ii=0; !found &&(ii<20); ++ii) {
       if(vnarr[0]) vnarr[0] = viewSymbolNodePtr(vnarr[0]->get_prev());
       if(vnarr[1]) vnarr[1] = viewSymbolNodePtr(vnarr[1]->get_next());
       if(!vnarr[0] && !vnarr[1]) break;
       for(int jj=0; jj<2; ++jj){
	 viewSymbolNodePtr prev = vnarr[jj];
	 if(prev && is_viewSymbolNode(prev)){
	   genString pdescr;
	   find_description(0, prev, pdescr);
	   const char*ptxt = pdescr.str();
	   if(!strcmp(txt, ptxt)){
	     sn = cmd_get_ind(prev);
	     found = 1;
	     break;
	   }
	 }
       }
     }
   }
   if(!found){
     err_txt.printf ("Wrong selection. Expected: %s; Real: %s", txt, new_txt);
     cmd_validation_error ((char *)err_txt);
   }
 }
 return sn;
}

static graWindow * cmd_gra_selection(const char *action, const char *arg,
                       int sn, int cn, int sub, float dx, float dy, char *txt)
{
    Initialize(cmd_gra_selection);

    if(!cmd_available_p(cmdGraValidation))
      txt = NULL;
    view * cur_view = cur_viewer->get_view_ptr();
    Assert(cur_view);
    viewGraHeader * gv = checked_cast(viewGraHeader, cur_view);
    graWindow * gw     = ((gra_ivInterface *)gv->get_gra_interface())->GetWindow();

    sn = verify_ajust_sn(sn, txt, cur_view);
    cmd_sel_index.view_node = sn;
    cmd_sel_index.connector = cn;
        
    cur_view_sel.sub_type = sub;

    graParametricSymbol * gs = update_cur_sel();
    Widget wdg = gw->get_widget();


    if(cmd_rerecording){
      viewSymbolNodePtr vn = viewSymbolNodePtr(cur_view_sel.view_node);
      cmd_record_action(1, action, arg, sn, cn, sub, dx, dy, vn);
    }

    if(gs) {
	int l,b,r,t;
	gs->GetBox(l, b, r, t);

	Position x_root, y_root;

	cur_ev.xmotion.x = (int) (dx * (r - l) + l);
	cur_ev.xmotion.y = (int) ((gw->getymax() - b) - dy * (t - b));

        // translate window coords to screen coords
	XtTranslateCoords(wdg, cur_ev.xmotion.x,
			  cur_ev.xmotion.y, &x_root, &y_root);

        cur_ev.xmotion.x_root = (int) x_root;
        cur_ev.xmotion.y_root = (int) y_root;
    }

    Window wnd = XtWindow(wdg);
    Window wroot;
    Window parent;
    Window *children;
    unsigned int n;

    IF (!XQueryTree (UxDisplay, wnd, &wroot, &parent, &children, &n))
	return gw;

    if(n==1)
        wnd = children[0];

    cur_ev.xbutton.window = wnd;

    cmd_moveto_ev(&cur_ev);

    return gw;
  }


// Generic action-function macros
#define GRA_ACTION(action) paste(gra_action_,action)
#define CMD_ACTION(action) paste(gra_,action)

#define BEFORE_ACTION(action,gw,w,e,p,n) \
VALIDATE cmd_gra_before_action(quote(action),gw, w, e, p, *n)

#define DEFINE_GRA_ACTION(action) \
extern "C" void CMD_ACTION(action)\
(int sn, int cn, int sub, double dx, double dy, char*arg, char *txt){ \
 if(sn==-1 && cn==-1) return; \
 cmd_gra_selection(quote(action),arg, sn,cn,sub,dx,dy,txt)->action(&cur_ev, &arg, arg?1:0);  \
 cmd_flush(); \
}\
static void GRA_ACTION(action) (Widget w, XEvent* e, String* p, Cardinal* n)\
{ graWindow * gw = widget_to_this(w); \
  if(cmd_begin())\
     BEFORE_ACTION(action,gw,w,e,p,n);\
  gw->action(e, p, *n);\
  cmd_end();\
}

#define GRA_ACTIONS_REC(action) { (char *)quote(action), GRA_ACTION(action) }

// function definitions

void graWindow::set_widget(unsigned long x_window)
{
    Initialize(graWindow::set_widget);
    
    Arg arg[2];
    
    Widget w = (x_window ? XtWindowToWidget(UxDisplay, x_window) : NULL);
    if(w)
    {
	static XtTranslations translations;
	if(translations == NULL)
	{
	    uiValue value;
	    const char* type =
		ui_get_resource("aset.graphics.translations",
				"AlphaSet.Graphics.Translations",
				&value);
	    if(type)
	    {
		if(!strcmp(type, XtRTranslationTable))
		    translations = XtTranslations(value.addr);
		else if(!strcmp(type, XtRString))
		    translations = XtParseTranslationTable((String)value.addr);
	    }
	    if(translations == NULL)
	    {
		if (customize::getBoolPref("DIS_viewer.Btn3_Disabled")) {
		    translations = XtParseTranslationTable(default_translations_no_Btn3);
		}
		else
		{
		    translations = XtParseTranslationTable(default_translations);
		}
	    }
	}
	
	// Set values on new widget.
	XtSetArg(arg[0], XmNuserData, this);
	XtSetArg(arg[1], XmNtranslations, translations);
	XtSetValues(w, arg, XtNumber(arg));
    }
    else if(widget)
    {
	static XtTranslations empty_translations =
	    XtParseTranslationTable("");
	
	// Clear values on old widget.
	XtSetArg(arg[0], XmNuserData, NULL);
	XtSetArg(arg[1], XmNtranslations, empty_translations);
	XtSetValues(widget, arg, XtNumber(arg));
    }
    widget = w;
}

//------------------------------------------
// find_symbol
//------------------------------------------

graParametricSymbol* graWindow::find_symbol(XEvent* e)
{
    Initialize(graWindow::find_symbol);

    viewSelection view_sel;
    graParametricSymbol* symbol =
	cmd_find_select(this, e, view_sel);

    if(symbol && !view_sel.connector)
	return(symbol);
    else if (!symbol && view_sel.connector)
        return (checked_cast(viewConnectionNode,view_sel.connector)->get_picture());
    else if (symbol && view_sel.connector) {
	if (is_viewFuncCallNode (view_sel.view_node))
	    return symbol;
    }

    return(NULL);
}

//------------------------------------------
// focus_in
//------------------------------------------

DEFINE_GRA_ACTION(focus_in)

void graWindow::focus_in(XEvent*, char**, int) {}

//------------------------------------------
// focus_out
//------------------------------------------

DEFINE_GRA_ACTION(focus_out)

void graWindow::focus_out(XEvent*, char**, int) {}

//------------------------------------------
// set_highlight
//------------------------------------------

DEFINE_GRA_ACTION(set_highlight)

void graWindow::set_highlight(XEvent* e, char**, int)
{
    Initialize(graWindow::set_highlight);

//    if(e == &cur_ev) return;

    graParametricSymbol* symbol = find_symbol(e);
    if(symbol)
	highlight(symbol);
    else if(highlight_window)
        highlight_window->unhighlight();
}

//------------------------------------------
// clear_highlight
//------------------------------------------

DEFINE_GRA_ACTION(clear_highlight)

void graWindow::clear_highlight(XEvent*, char**, int)
{
    Initialize(graWindow::clear_highlight);

//    if(e == &cur_ev) return;

    if(highlight_window)
	highlight_window->unhighlight();
}

//------------------------------------------
// open_node
//------------------------------------------

DEFINE_GRA_ACTION(open_node)

void graWindow::open_node(XEvent* e, char**, int)
{
    Initialize(graWindow::open_node);

    graParametricSymbol* symbol = find_symbol(e);
    if(symbol)
	symbol->open_node(this);
}

//------------------------------------------
// collapse_expand
//------------------------------------------

DEFINE_GRA_ACTION(collapse_expand)

void graWindow::collapse_expand (XEvent* e, char**, int)
{
    Initialize (graWindow::collapse_expand);

    clear_highlight(0,0,0);

    graAttachmentPoint	attach;
    viewFuncCallNode*	vfcn;
    viewMultiNode*	vmn;
    viewGraHeader*	vgh;
    viewSelection	vs;
    viewNode*		vn;
    viewNode*		cn;
    int			atn, regen_flag = 0;

    // do nothing for ERD and DATA CHART
    int vt = gra_type();
    if (vt == Rep_ERD || vt == Rep_DataChart || vt == Rep_SubsystemMap)
        return;

    cmd_find_select (this, e, vs);

    cn = checked_cast (viewNode, vs.connector);
    if ((vn = checked_cast (viewNode, vs.view_node)) != NULL)
        vgh = checked_cast (viewGraHeader, vn->get_header());

    start_transaction() {

        if (vn != NULL && cn == NULL) {

	    //-----------------------------
	    // clicked branch or decoration
	    //-----------------------------
		
	    if (vn->viewIsKindOf (viewMultiType) &&
		    vs.sub_type >= graLayerNoDecorationLL &&
		    vs.sub_type <= graLayerNoDecorationUR) {
		vmn = checked_cast (viewMultiNode, vn);
	        atn = vs.sub_type;
		attach = (graAttachmentPoint) (atn - graLayerNoDecorationLL);
	        if (vmn->viewIsKindOf(viewFuncCallType)) {

		    //-------------------------------------
		    // special handling for call tree nodes
		    //-------------------------------------

		    regen_flag = 1;
		    vfcn = checked_cast (viewFuncCallNode, vmn);
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

	        viewFuncCallNode*	upper_vfcn;
	        viewConnectionNode*	vcn;
	        objArr*			inputs;
	        Obj*			ob;

	        vfcn 	= checked_cast (viewFuncCallNode, vn);
	        vcn 	= checked_cast (viewConnectionNode, cn);
	        inputs	= vcn->get_inputs ();

		//-----------------------------------------------
		// assume sub-branch connector has only one input
		//-----------------------------------------------
    
	        ForEach (ob, *inputs) {
	            if (!ob->relationalp())
	    	        continue;
		    if (!is_viewFuncCallNode ((Relational*) ob))
	                continue;
		    upper_vfcn = checked_cast (viewFuncCallNode, ob);
		    upper_vfcn->prune_branch (vfcn);
		    vgh->hierarchy_control (vn);
		    break;
	        }
	    }
        }
    } end_transaction();
}

//------------------------------------------
// drop
//------------------------------------------

DEFINE_GRA_ACTION(drop)

void graWindow::drop(XEvent* e, char**, int)
{
    Initialize(graWindow::drop);

    if(carry_feedback)
    {
	carry_feedback->Erase();
	carry_feedback->unref();
	carry_feedback = NULL;
    }

    if(pickup_sel.view_node == NULL)
	return;

    viewSelection drop_sel;
    cmd_find_select(this, e, drop_sel);

    if(drop_sel.view_node == NULL) {
        iv3(Transformer)* xfmp = GetGraphic()->GetTransformer();
	float x = e->xmotion.x;
	float y = ymax - e->xmotion.y;
	xfmp->inverse_transform(x, y);
        viewGraHeaderPtr vh =
              checked_cast(viewGraHeader,pickup_sel.view_node->get_header());
	vh->reposition_node(checked_cast(viewSymbolNode,pickup_sel.view_node), x +
              carry_x_offset, y + carry_y_offset);
	return;
    }

    if(pickup_sel.view_node != drop_sel.view_node ||
       pickup_sel.connector != drop_sel.connector)
    {
	viewNodePtr from_node = checked_cast(viewNode,pickup_sel.view_node);
	viewGraHeaderPtr view = checked_cast(viewGraHeader,from_node->get_header());

	button DUMMY(eventType_Undefined, eventMod_None);

	driver_instance->put_selection(*view, DUMMY, pickup_sel);
	driver_instance->addselect();

	viewNodePtr onto_node = checked_cast(viewNode,drop_sel.view_node);
	view = checked_cast(viewGraHeader,onto_node->get_header());

	driver_instance->put_selection(*view, DUMMY, drop_sel);
	driver_instance->addselect();

	driver_instance->move_node();
    }
    else
	collapse_expand(e, NULL, 0);

    pickup_sel.view_node = NULL;
    pickup_sel.connector = NULL;

    set_highlight(e, NULL, 0);
}

//------------------------------------------
// pickup
//------------------------------------------

DEFINE_GRA_ACTION(pickup)

void graWindow::pickup(XEvent* e, char**, int)
{
    Initialize(graWindow::pickup);

    if (gra_type() == Rep_OODT_Inheritance)
	return;

    if(carry_feedback)
    {
	carry_feedback->unref();
	carry_feedback = NULL;
    }

    graParametricSymbol* symbol = cmd_find_select(this, e, pickup_sel);

    // Don't drag connectors around (yet).
    if(!pickup_sel.view_node || pickup_sel.connector)
    {
	pickup_sel.view_node = NULL;
	return;
    }

    // Disable draggin in source outline views (for now) -jef
    viewSymbolNodePtr viewnode = symbol->get_viewnode();
    viewPtr viewheader= checked_cast(view, viewnode->get_header());
    if (! is_viewERDHeader(viewheader) && ! is_viewMulticonnHeader(viewheader))
    {
	pickup_sel.view_node = NULL;
	return;
    }


    // unhighlight current element during dragging
    clear_highlight (NULL, 0, 0);

    // Disable dragging during journal playback
    if (!cmd_execute_journal_name) {
	iv3(Event) ie;
	ie.target = this;
	ie.x = e->xmotion.x;
	ie.y = ymax - e->xmotion.y;
	gra_drag_symbol (&pickup_sel, ie);
	
	Cardinal count = 0;
	BEFORE_ACTION (drop,this,get_widget(),&ie.rep()->xevent_,0,&count);
	drop (&ie.rep()->xevent_, 0, count);
    }
}

//------------------------------------------
// carry
//------------------------------------------

DEFINE_GRA_ACTION(carry)

void graWindow::carry(XEvent* e, char**, int)
{
    Initialize(graWindow::carry);

    if(carry_feedback)
    {
	carry_feedback->Erase();

	auto_scroll(e->xmotion.x, ymax - e->xmotion.y);

	carry_feedback->SetCanvas(canvas);
	carry_feedback->Track(e->xmotion.x, ymax - e->xmotion.y);
	carry_feedback->Draw();
    }
}

void graWindow::set_mode (graWindow_Mode new_mode)
{
    Initialize (graWindow::set_mode);

    modal_value = new_mode;
}


//------------------------------------------
// end_connection
//------------------------------------------


DEFINE_GRA_ACTION(end_connection)

void graWindow::end_connection(XEvent* e, char**, int)
{
    Initialize(graWindow::end_connection);

    // This function used to allow the user to define new relations;
    // now it does nothing.  It should probably be removed :)

    if (carry_feedback) {
       carry_feedback->Erase();
       carry_feedback->unref();
       carry_feedback = NULL;
    }
}

//------------------------------------------
//begin_connection
//------------------------------------------

DEFINE_GRA_ACTION(begin_connection)

void graWindow::begin_connection(XEvent* e, char** params, int num_params)
{
    Initialize(graWindow::begin_connection);

    switch (modal_value) {
      case in_zoom_modal:
	zoom_rectangle (e, params, num_params);
	break;

      case in_1_1_relation:
      case in_1_many_relation:
      case in_many_1_relation:
      case in_many_many_relation:
	break;

      case in_pan_modal:
      case in_select_modal:
      default:
	break;
    }
}

//------------------------------------------
// modal_click
//------------------------------------------

static void timer_callback (XtPointer ptr, XtIntervalId*)
{
    * (bool*) ptr = true;
}

DEFINE_GRA_ACTION(modal_click)

void graWindow::modal_click (XEvent* e, char** params, int num_params)
{
    Initialize (graWindow::modal_click);

    static unsigned long mct = 0UL;
    if (mct == 0UL)
	mct = (unsigned long) XtGetMultiClickTime (UxDisplay);

    //-------------------------
    // highlight clicked symbol
    //-------------------------

    graParametricSymbol* symbol = find_symbol (e);
    if (symbol)
	clear_highlight (NULL, NULL, 0);	
    select (e, params, num_params);
    if (symbol)
	highlight (symbol);

    //--------------------------------------------------------
    // check for a double click using a small local event loop
    //--------------------------------------------------------

    bool	double_click = false;
    XEvent	event;

    if (!cmd_execute_journal_name) {

        XtGrabPointer (widget, True, (unsigned int) ButtonPressMask,
		    GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

        bool timed_out = false;
	XtInputMask im, nxim = XtIMTimer | XtIMAlternateInput;
        XtIntervalId xid = XtAppAddTimeOut (UxAppContext, mct,
				timer_callback, (XtPointer) &timed_out);

        while (!timed_out) {
	    if ((im = XtAppPending (UxAppContext)) & nxim)
		XtAppProcessEvent (UxAppContext, nxim);
	    else if (im & XtIMXEvent) {
	        XtAppNextEvent (UxAppContext, &event);
	        if (event.type == ButtonPress) {
		    XtRemoveTimeOut (xid);
	            double_click = true;
	            break;
	        }
		XtDispatchEvent (&event);
	    }
        }

        XtUngrabPointer (widget, CurrentTime);
    }

    //-------------------------------------------
    // perform double-click action if appropriate
    //-------------------------------------------

    if (double_click) {
	int num = 0;
	BEFORE_ACTION (open_node, this, widget, &event, NULL, &num);
	open_node (&event, NULL, 0);
    }
}

//------------------------------------------
// zoom_rectangle
//------------------------------------------

DEFINE_GRA_ACTION(zoom_rectangle)

void graWindow::zoom_rectangle(XEvent* e, char**, int)
{
    Initialize(graWindow::zoom_rectangle);

    static const int MIN_ZOOM_SIZE = 5;
    static const float MAX_MAGNIFICATION = 100.0;

    iv3(Perspective) np(*perspective);

    // The assumption is that the user just pressed a mouse button.
    // The following function drags a rubberband rectangle until
    // an UpEvent is received.

    iv3(Event) event;
    event.x = e->xmotion.x;
    event.y = ymax - e->xmotion.y;
    event.target = this;

    iv2(Coord) xx0, yy0, x1, y1;
    gra_rect_select(event, xx0, yy0, x1, y1);

    if(x1 - x0 < MIN_ZOOM_SIZE || y1 - y0 < MIN_ZOOM_SIZE)
	Return

    x0 += np.curx;	// Convert to the Graphic's coordinate system.
    x1 += np.curx;
    y0 += np.cury;
    y1 += np.cury;

    float factor = max(float(x1 - x0) / float(np.curwidth),
		       float(y1 - y0) / float(np.curheight));

    if(mag / factor > MAX_MAGNIFICATION)
	factor = mag / MAX_MAGNIFICATION;

    np.curwidth = round(np.curwidth * factor);
    np.curheight = round(np.curheight * factor);

    iv2(Coord) cx = (x0 + x1) / 2;
    iv2(Coord) cy = (y0 + y1) / 2;

    np.curx = cx - np.curwidth / 2;
    np.cury = cy - np.curheight / 2;

    Adjust(np);
}

//------------------------------------------
// recenter
//------------------------------------------

DEFINE_GRA_ACTION(recenter)

void graWindow::recenter(XEvent* e, char**, int)
{
    Initialize(graWindow::recenter);

    recenter(e->xmotion.x, ymax - e->xmotion.y);
}

void graWindow::recenter(int x, int y)
{
    Initialize(graWindow::recenter);

    iv3(Perspective) s(*perspective);
    s.curx += x - (s.curwidth/2);
    s.cury += y - (s.curheight/2);
    Adjust(s);
}

//------------------------------------------
// select
//------------------------------------------

DEFINE_GRA_ACTION(select)

void graWindow::select(XEvent* e, char** params, int num_params)
{
    Initialize(graWindow::select);

    viewSelection sel;
    graParametricSymbol* symbol = cmd_find_select(this, e, sel);

    if (symbol && symbol->has_text_focus())
    {
	symbol->set_text_focus(0);
	symbol = cmd_find_select(this, e, sel);
    }

    enum { SELECT_SINGLE, SELECT_ADD, SELECT_TOGGLE } mode;
    if(num_params <= 0)
	mode = SELECT_SINGLE;
    else if(!strcmp(params[0], "add"))
	mode = SELECT_ADD;
    else if(!strcmp(params[0], "toggle"))
	mode = SELECT_TOGGLE;
    else // invalid selection mode.
	return;

    if(sel.view_node)
    {
	if((mode == SELECT_TOGGLE) &&
	   driver_instance->is_selected(sel.view_node))
	{
	    driver_instance->unselect(sel.view_node);
	    return;
	}
    }
    else if(mode == SELECT_ADD || mode == SELECT_TOGGLE)
    {
	// Do nothing if clicking on nothing in "add" or "toggle" mode.
	return;
    }
    
    if(symbol)
    {
        set_text_focus(symbol);
        if(symbol->has_text_focus())
	{
	    int pos = symbol->locate_text_position(
		e->xmotion.x, ymax - e->xmotion.y);
	    if(pos >= 0)
		symbol->shift_cursor(pos);
	}
    }

    viewGraHeaderPtr viewHeader =
	(sel.view_node ? checked_cast(viewGraHeader,sel.view_node->get_header()) : NULL);

    button DUMMY(eventType_Undefined, eventMod_None);

    driver_instance->put_selection(*viewHeader, DUMMY, sel);
    if(mode == SELECT_SINGLE)
	driver_instance->select();
    else
	driver_instance->addselect();
}


//-----------------------------------------
// symbol_menu
//-----------------------------------------

void cmd_gra_popup_begin(Widget wg);
void cmd_gra_popup_end();
void gra_destroy_menu(Widget);

DEFINE_GRA_ACTION(symbol_menu)

typedef void (*funcptr)();
static int add_children_to_menu(Widget parent, NodePopupInfo *popup_info, int start_index);

static Widget make_submenu(Widget parent, NodePopupInfo *popup_info, int &index)
{
    static int subnum = 0;

    Widget sub;
    char subname[50];
    sprintf(subname, "submenu%d", subnum++);
    sub = XmCreatePulldownMenu(parent, subname, NULL, 0);
    index++;
    index = add_children_to_menu(sub, popup_info, index);

    return sub;
}

static int add_children_to_menu(Widget parent, NodePopupInfo *popup_info, int start_index)
{
    int num = 0;
    int i;
    for(i = start_index; i < popup_info->item_number && popup_info->GetType(i) != END_SUBMENU; i++){
        Widget wij_child = NULL;
	if(popup_info->GetType(i) == SEPARATOR){
	    wij_child = XmCreateSeparator(parent, "sep", NULL, 0);
	} else {
            Arg         xt_args[2];
            char        name[128];
            XmString    xmstr         = NULL;
            char*       callback_type = NULL;

	    char *label = popup_info->GetText(i);
	    if(label == NULL  ||  label[0] == '\0'){
                /* Just in case someone forgot to name it. */
                sprintf(name, "item%d", num++);
                label = name;
            } else {
		int j;
                for(j = 0; label[j]  &&  (j < sizeof(name) - 1); ++j)
                {
                    /* Translate "bad" characters in widget name. */
                    switch(label[j])
                    {
                      case '.': name[j] = ','; break;
                      case '*': name[j] = '+'; break;
                      case ':': name[j] = ';'; break;
                      case '?': name[j] = '/'; break;
                      case '!': name[j] = '|'; break;
                      default:  name[j] = label[j]; break;
                    }
                }
                name[j] = '\0';
            }
            xmstr = XmStringCreateSimple((char *)gettext(label));
            XtSetArg(xt_args[0], XmNlabelString, xmstr);
            switch(popup_info->GetType(i)){
              case TOGGLE:
                wij_child     = XmCreateToggleButton(parent, name, xt_args, 1);
                callback_type = XmNvalueChangedCallback;
                break;
	      case PUSH:
                wij_child     = XmCreatePushButton(parent, name, xt_args, 1);
                callback_type = XmNactivateCallback;
                break;
	      case CASCADE:
                XtSetArg(xt_args[1], XmNsubMenuId,
                         make_submenu(parent, popup_info, i));
                wij_child = XmCreateCascadeButton(parent, name, xt_args, 2);
                break;
            }
            if(callback_type)
            {
                funcptr callback    = (funcptr)popup_info->GetCallback(i);
		void *callback_data = popup_info->GetCallbackData();
                if(callback)
                    XtAddCallback(
                        wij_child, callback_type, (XtCallbackProc)callback, callback_data);
            }
            XmStringFree(xmstr);
        }
        if(wij_child)
            XtManageChild(wij_child);
    }
    return i;
}

void graWindow::symbol_menu(XEvent* e, char**, int)
{
    Initialize(graWindow::symbol_menu);

    graParametricSymbol* symbol = find_symbol(e);
    if(symbol)
    {
	NodePopupInfo *popup_info    = symbol->generate_popup();
	if(popup_info){
	    viewGraHeaderPtr viewHeader = checked_cast(viewGraHeader,symbol->get_viewnode()->get_header());
	    dialogPtr dlg = driver_instance->find_dialog_for_view(viewHeader);
	    if (!dlg)
		return;
	    Window parent_window = (Window)dlg->window_of_view(viewHeader);
	    Widget parent_widget = XtWindowToWidget(UxDisplay, parent_window);
	    Widget menu_parent = XtVaCreateManagedWidget("popup_muffler",
							 xmDrawingAreaWidgetClass, parent_widget,
							 XmNwidth, 5,
							 XmNheight, 5,
							 XmNmappedWhenManaged, 0,
							 NULL);
	    if (!menu_parent)
		return;
	    XtRealizeWidget(menu_parent);

	    Widget popup_menu = XmCreatePopupMenu(menu_parent, "context_pop_men", NULL, 0);
	    if (popup_menu != NULL)
	    {
		add_children_to_menu(popup_menu, popup_info, 0);
		cmd_gra_popup_begin(popup_menu);
		
		XtAddCallback(popup_menu, XmNunmapCallback,
			      XtCallbackProc(gra_destroy_menu), NULL);
		popup_context_menu(e->xmotion.x_root, e->xmotion.y_root,
				   popup_menu);
		
		cmd_gra_popup_end();
	    }
	    delete popup_info;
	}
    }
}

void gra_destroy_menu(Widget menu)
{
    Widget parent = XtParent(XtParent(menu));
    XtDestroyWidget(parent);
}

//------------------------------------------
// gra_init_actions
// 
// This function registers application level Actions with the Intrinsics
// Translation Manager.  This associates the action names used in
// Translation resource strings with pointers to functions.  There is one
// entry in the actions[] array for each Action defined above.  This is
// only called from the graWindow ctor.
// 
// See also graWindow::set_widget.
//------------------------------------------

void gra_init_actions()
{
    Initialize(gra_init_actions);

    static XtActionsRec actions[] =
    {
	GRA_ACTIONS_REC(focus_in),
	GRA_ACTIONS_REC(focus_out),
	GRA_ACTIONS_REC(set_highlight),
	GRA_ACTIONS_REC(clear_highlight),
	GRA_ACTIONS_REC(open_node),
	GRA_ACTIONS_REC(collapse_expand),
	GRA_ACTIONS_REC(pickup),
	GRA_ACTIONS_REC(carry),
	GRA_ACTIONS_REC(drop),
	GRA_ACTIONS_REC(zoom_rectangle),
	GRA_ACTIONS_REC(recenter),
	GRA_ACTIONS_REC(select),
	GRA_ACTIONS_REC(symbol_menu),
	GRA_ACTIONS_REC(modal_click),
        GRA_ACTIONS_REC(begin_connection),
        GRA_ACTIONS_REC(end_connection),
    };
    static init;

    if(!init)
    {
	XtAppAddActions(UxAppContext, actions, XtNumber(actions));
	init = 1;
    }
}

//------------------------------------------
// highlight_box
//
// Change the location of the highlighting box.
// 
// This is a private function that is used only by highlight and
// unhighlight.
//------------------------------------------

void graWindow::highlight_box(graParametricSymbol* symbol)
{
    Initialize(graWindow::highlight_box);

    if(xor_box == NULL)	// Create xor_box the first time we need it.
    {
	xor_box = new rubberGraphic(output, canvas);
	if(xor_box == NULL)
	    Return		// allocation failed!
    }
    else
	xor_box->Erase();

    if(symbol)
    {
        highlight_symbol = symbol;
	symbol->do_highlight(canvas, xor_box);
	highlight_window = this;
    }
    else {
        if (highlight_symbol)
            highlight_symbol->undo_highlight(canvas);
	highlight_symbol = NULL;
	highlight_window = NULL;
    }
}

//------------------------------------------
// highlight
//
// Set the highlight on a given symbol in this window.
//------------------------------------------

void graWindow::highlight(graParametricSymbol* symbol)
{
    Initialize(graWindow::highlight);

    if(this != highlight_window || symbol != highlight_symbol)
    {
	if(highlight_window)
	    highlight_window->unhighlight();
	highlight_box(symbol);

	if(symbol)
	{
	    viewSymbolNodePtr viewnode = symbol->get_viewnode();
	    genString status_line;
	    viewnode->description(status_line);
	    const char *pc = status_line.str();
	    if (pc == NULL)
		pc = "";
	    msg("highlight:graAction.h.C", normal_sev) << pc << eom;
	}
    }
}

//------------------------------------------
// unhighlight
//
// If this window has the highlight_box on any symbol,
// then remove it.
//------------------------------------------

void graWindow::unhighlight()
{
    Initialize(graWindow::unhighlight);

    if(this == highlight_window)
	highlight_box(NULL);
}

//------------------------------------------
// unhighlight
//
// If this window has the highlight on the specified symbol,
// then remove it.
//------------------------------------------

void graWindow::unhighlight(graParametricSymbol* symbol)
{
    Initialize(graWindow::unhighlight);

    if(this == highlight_window && symbol == highlight_symbol)
	highlight_box(NULL);
}

//------------------------------------------
// gra_handle_window_delete
//------------------------------------------

void gra_handle_window_delete(graWindow *window)
{
    Initialize(gra_handle_window_delete);

    if(window)
    {
	window->set_widget(NULL);

	if(window == focus_window)
	    window->remove_text_focus();

	window->unhighlight();
    }
}

//------------------------------------------
// gra_handle_symbol_delete
//------------------------------------------

void gra_handle_symbol_delete(graWindow* window, graParametricSymbol *symbol)
{
    Initialize(gra_handle_symbol_delete);

    if(window)
	window->unhighlight(symbol);

    if(symbol)
    {
	if(symbol->get_viewnode())
	    driver_instance->unselect(symbol->get_viewnode());
	if(window && symbol->has_text_focus())
	    window->remove_text_focus();
    }
}

//------------------------------------------
// graWindow::set_text_focus
//------------------------------------------

void graWindow::set_text_focus(graParametricSymbol* symbol)
{
    Initialize(graWindow::set_text_focus);

    if(focus_window)
	focus_window->remove_text_focus();

    if(symbol)
    {
	damage_area(symbol);
	symbol->set_text_focus(1);
	damage_area(symbol);

        if(symbol->has_text_focus())
	{
	    focus_symbol = symbol;
	    focus_window = this;
	}
	else
	{
	    symbol->set_text_focus(0);
	    damage_area(symbol);
	}
    }

    Return
}

//------------------------------------------
extern void gra_finalize(void)
{
    if (focus_window)
	focus_window->remove_text_focus();
}

//------------------------------------------
// graWindow::remove_text_focus
//------------------------------------------

void graWindow::remove_text_focus()
{
    Initialize(graWindow::remove_text_focus);

    if(this == focus_window)
    {
	if(focus_symbol)
	{
	    damage_area(focus_symbol);
	    focus_symbol->set_text_focus(0);
	    if (focus_symbol) // the previous call may have nulled focus_symbol
		damage_area(focus_symbol);

	    focus_symbol = NULL;
	}
	focus_window = NULL;
    }

    Return
}

void gra_focus_symbol(graWindow *window, graParametricSymbol *symbol)
//
// This function shifts the current window (if necessary) to make
// sure that a given symbol is visible on the screen.  It dosn't try
// to center the symbol or put the symbol in any given location, it
// moves the window in the direction of the off screen symbol just
// enough to display the entire symbol in the window, or if the symbol
// is larger then the window, it displays as much of the top-left
// of the symbol as it can. If we have to move the window, and if the
// window can be adjusted back to zero on the x-axis while still
// displaying all of the symbol, then we snap the window back to the
// x-axis (this is to make things act better for the hierarchical view).  -jef
//
{
    if (!window || !symbol)
	return;

    // if the window's canvas doesn't exist yet, defer the focus until
    // it's there
    if (window->GetCanvas() == NULL)
    {
	pending_focus_window = window;
	pending_focus_symbol = symbol;
	return;
    }

    iv3(Perspective) ps = *window->GetPerspective();

    int newx = ps.curx;
    int newy = ps.cury;

    int win_top = ps.cury + ps.curheight;
    int win_bottom = ps.cury;
    int win_left = ps.curx;
    int win_right = ps.curx + ps.curwidth;

    float left, bottom, right, top;
    symbol->GetBounds(left, bottom, right, top);

    int symbol_margin = 5;
    int sym_top = round(top) + ps.cury + symbol_margin;
    int sym_bottom = round(bottom) + ps.cury - symbol_margin;
    int sym_left = round(left) + ps.curx - symbol_margin;
    int sym_right = round(right) + ps.curx + symbol_margin;

    if (sym_bottom < win_bottom)
    {
	newy = sym_bottom;
	if (newy > sym_bottom) // Adjust so top edge is always visible
	    newy = sym_bottom;
    }
    else if (sym_top > win_top)
    {
	newy = sym_top - ps.curheight;
    } // otherwise do nothing because it is already visible on this axis

    if (sym_left < win_left)
    {
	if (sym_right < ps.curwidth) // Snap back to the x-axis
	    newx = 0;
	else
	    newx = sym_left;
    }
    else if (sym_right > win_right)
    {
	newx = sym_right - ps.curwidth;
	if (newx > sym_left) // Adjust so left edge is always visible
	    newx = sym_left;
    } // otherwise do nothing because it is already visible on this axis

    if (ps.cury != newy || ps.curx != newx)
    {
	// Move the window
	ps.curx = newx;
	ps.cury = newy;
	window->Adjust(ps);
    }
}


//------------------------------------------
// gra_focus_pending
//
// execute a gra_focus_symbol call that had been deferred
// because the window's canvas didn't exist
//------------------------------------------

void gra_focus_pending()
{
    if (pending_focus_window && pending_focus_symbol)
    {
	gra_focus_symbol(pending_focus_window, pending_focus_symbol);
	pending_focus_symbol = NULL;
	pending_focus_window = NULL;
    }
}

//------------------------------------------
// Handle
//
// Subvert the X Intrinsics event dispatching mechanism to process a
// translation table for the graWindow object, which is not a Widget.
// 
// The translation table is actually owned by the Motif DrawingArea widget
// that contains the top-level InterViews Scene.  This function overwrites
// the window member of the Event structure so that it appears to be
// directed at the DrawingArea widget.  All the translations for this
// widget refer to the C functions below this one, and they simply convert
// the action into a member function.
//------------------------------------------

extern "C" viewerShell_set_current_view (void *);

void graWindow::Handle(iv3(Event)& e)
{
    Initialize (graWindow::Handle);

    // set current view in viewershell
    if (e.eventType == DownEvent) {
	if (view_header)
	    viewerShell_set_current_view (view_header);
    }

    if(widget)
    {
	if(e.eventType != KeyEvent)
	{
	    XEvent* x_event = &e.rep()->xevent_;
	    x_event->xany.window = XtWindow(widget);
	    XtDispatchEvent(x_event);
	}
    }

    Return
}

//------------------------------------------
// Redraw
// 
// Draw the portion of the window's image within the given coordinates.
//------------------------------------------

void graWindow::Redraw(iv2(Coord) l, iv2(Coord) b, iv2(Coord) r, iv2(Coord) t)
{
    Initialize(graWindow::Redraw);

    GraphicBlock::Redraw(l, b, r, t);
    if(highlight_symbol)
    {
	xor_box->SetGraphic(highlight_symbol);
	xor_box->GetPainter()->Clip(canvas, l, b, r, t);
	highlight_symbol->do_highlight(canvas, xor_box);
	xor_box->GetPainter()->NoClip();
    }

    Return
}

/*------------------------------------------
$Log: graAction.h.C  $
Revision 1.39 2001/07/25 20:41:58EDT Sudha Kallem (sudha) 
Changes to support model_server.
Revision 1.2.1.34  1994/07/22  14:01:22  mg
Bug track: NA
gaphical clicking in white space bug fix in scripting

Revision 1.2.1.33  1994/07/14  23:02:34  aharlap
bug # 7856

Revision 1.2.1.32  1994/07/01  22:23:04  jerry
Bug track: 7704
Smit bears full responsibility.

Revision 1.2.1.31  1994/02/24  18:02:13  aharlap
Port

Revision 1.2.1.30  1994/02/09  22:56:41  pero
bug 6267: Ctrl-Button1 for multiple selection was in conflict
with Ctrl-Button1 for Zoom Selection; Shifted Button2->Button3
and Button1->Button2

Revision 1.2.1.29  1994/01/28  21:16:06  davea
bug 6197:
In cmd_get_child, handle the case where there
are no children at all.

Revision 1.2.1.28  1994/01/05  17:22:31  andrea
Bug track: 5847
I changed the rubber line for MMB to be a sliding rect.  The LMB stays a rubber line

Revision 1.2.1.27  1993/12/31  19:37:50  boris
Bug track: Test Validation
Test Validation

Revision 1.2.1.26  1993/12/07  13:31:43  andrea
Bug track: n/a
I changed the sliding rect to a rubber line when dragging and dropping relations
with the LMB

Revision 1.2.1.25  1993/11/01  21:24:32  andrea
Bug track: 5081
I added an extra check in cmd_get_cur_view

Revision 1.2.1.24  1993/09/27  20:27:02  kws
Bug track: Port
Port changes

Revision 1.2.1.23  1993/09/23  15:35:39  mg
Bug track: NA
record string

Revision 1.2.1.22  1993/08/27  00:16:56  jon
bug 3642 - Added extra widget as parent to popup menus and deleted
the widget when the menu is unmapped to avoid a server grab the next
time button 3 is pressed.

Revision 1.2.1.21  1993/08/26  13:26:27  wmm
Fix bug 4298 (restore sliding rectangle for drag&drop relation creation, at
least until a rubberband connector can be implemented).

Revision 1.2.1.20  1993/08/25  13:19:45  wmm
Fix bug 4516.

Revision 1.2.1.19  1993/08/15  21:52:24  jon
Disabled dragging with the middle mouse button in all views execept
ERD and Multiconn (bug 4025)

Revision 1.2.1.18  1993/08/12  13:25:41  andrea
fixed bug 4275

Revision 1.2.1.17  1993/08/09  22:01:25  andrea
fixed bug 4008

Revision 1.2.1.16  1993/07/22  21:40:54  andrea
*** empty log message ***

Revision 1.2.1.15  1993/05/25  20:19:40  mg
update_cur_sel

Revision 1.2.1.14  1993/05/17  17:47:39  mg
*** empty log message ***

Revision 1.2.1.13  1993/05/14  20:13:32  mg
*** empty log message ***

Revision 1.2.1.12  1993/05/13  22:56:02  glenn
Put back a "return" that was accidentally deleted in graWindow::drop.
This may fix bug #3783.

Revision 1.2.1.11  1993/05/13  13:50:12  mg
*** empty log message ***

Revision 1.2.1.10  1993/05/11  21:23:19  mg
cmd recording

Revision 1.2.1.9  1993/03/30  22:56:46  wmm
Fix pointer conversion error discovered in investigating bugs
2634, 2745, and 2914.

Revision 1.2.1.8  1993/02/17  00:12:42  jon
Rewrote gra_focus_symbol to be a little smarter (bug 2354)

Revision 1.2.1.7  1992/11/23  18:12:55  wmm
typesafe casts.

Revision 1.2.1.6  1992/11/17  19:41:41  jon
Bug 2003

Revision 1.2.1.5  1992/11/10  22:53:16  jon
Disabled editing in graphical views (see graWindow::select)

Revision 1.2.1.4  1992/11/06  21:59:32  jon
Fixed crash introduced by previous fix.

Revision 1.2.1.3  1992/11/05  18:42:06  jon
Fixed problem with clicking on a symbol that was being edited
Bug #1689

Revision 1.2.1.2  92/10/09  18:36:47  builder
fixed rcs header

Revision 1.2.1.1  92/10/07  20:42:23  smit
*** empty log message ***

Revision 1.2  92/10/07  20:42:22  smit
*** empty log message ***

Revision 1.1  92/10/07  18:20:33  smit
Initial revision

Revision 1.1  92/10/07  17:56:12  smit
Initial revision

// Revision 1.31  92/10/01  13:32:23  jon
// Bug #422
// 
// Revision 1.30  92/08/28  10:35:31  glenn
// Remove graWindow::destroy_popup.  Use XtDestroyWidget instead.
// 
// Revision 1.29  92/08/26  09:27:06  jon
// Added unmap callback to the popup menus so they could be
// destroyed correctly.  Bug #639
// 
// Revision 1.28  92/07/30  11:31:34  jon
// Cleanup of InterViews scope fix
// 
// Revision 1.27  92/07/29  23:44:08  jon
// InterViews scope fix
// 
// Revision 1.26  92/07/09  14:50:45  glenn
// Call auto_scroll in carry.
// 
// Revision 1.25  92/06/29  16:50:41  glenn
// Pass graSymbol's description to status-line during highlight.
// 
// Revision 1.24  92/06/19  12:15:48  jont
// add relations of different cardinalities
// 
// Revision 1.23  92/06/18  20:53:42  jont
// added click_relation
// 
// Revision 1.22  92/06/07  10:59:53  smit
// change some include files.
// 
// Revision 1.21  92/05/23  19:54:53  sergey
// Changed toggle_display interface to avoid double routing in Call Trees.
// 
// Revision 1.20  92/05/22  10:07:05  smit
// Fix text focusing and selection problems.
// 
// Revision 1.19  92/05/21  17:04:11  smit
// Unhilite hilighted tokens during unselect.
// 
// Revision 1.18  92/05/21  13:07:11  smit
// Make unselect_symbol work without window.
// 
// Revision 1.17  92/05/15  18:38:07  wmm
// Remove erroneous assumption in drop() that source and target views are the
// same.
// 
// Revision 1.16  92/05/13  18:52:05  jont
// fixed gra_select_symbol to work with null window;
// implemented gra_focus_pending
// 
// Revision 1.15  92/04/17  14:54:59  smit
// Added support to make view current by clicking.
// 
// Revision 1.14  92/04/16  08:23:35  glenn
// Fix problems with include files.
// 
// Revision 1.13  92/04/15  10:06:58  wmm
// Fix problem in which graFlexConn highlighting still occasionally resulted in
// use of the rubber band box.
// 
// Revision 1.12  92/04/15  08:02:23  wmm
// Implement ability to move graphical symbols.
// 
// Revision 1.11  92/04/09  18:51:53  wmm
// Change highlighting to use graParametricSymbol::do_highlight() and
// undo_highlight() if available (for highlighting of graFlexConn connectors).
// 
// Revision 1.10  92/04/06  18:59:03  glenn
// Use old gra_focus_symbol method.
// 
// Revision 1.9  92/04/05  13:35:11  smit
// Make move work.
// 
// Revision 1.8  92/03/31  17:24:10  wmm
// Allow connectors to be selected.
// 
// Revision 1.7  92/03/30  17:09:40  smit
// modified translation table to make zoom work.
// Added actions for modal values.
// 
// Revision 1.6  92/03/20  16:28:20  glenn
// Add more checks for window and symbol ptrs in C interface functions.
// 
// Revision 1.5  92/03/19  09:23:33  builder
// Fixed unreasonable nesting problem.
// 
// Revision 1.4  92/03/10  17:45:15  wmm
// Make OODT browser graphics available to call tree browser.
// 
// Revision 1.3  92/03/06  17:36:33  glenn
// Implement "add" and "toggle" modes for select Action.
// Bind new selection modes to Shift-Btn1Down and Ctrl-Btn1Down.
// 
// Revision 1.2  92/03/04  13:50:45  glenn
// Rename move_begin/move_tracking/move_end to pickup/carry/drop.
// Rename move_begin/move_tracking/move_end to pickup/carry/drop.
// Allow carrying between windows.
// 
// Revision 1.1  92/03/03  18:30:04  glenn
// Initial revision
// 
------------------------------------------*/
