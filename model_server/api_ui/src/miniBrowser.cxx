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
#include "tcl.h"
#include <Interpreter.h>
#include <cliUI.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <viewerShell.h>
#include <miniBrowser.h>
#include <cLibraryFunctions.h>
#include <objArr.h>
#include <gtPushButton.h>
#include <gtBaseXm.h>
#include <gtRTL.h>
#include <gtForm.h>

#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>


class cliListController : public miniBrowserRTL_API {
  public:
    cliListController(char *cproc, char *dcproc, objArr& buttons);
    // Function get called when selecting in rtl
    virtual void selection_callback(RTL* rtl, symbolArr& selected, miniBrowser *); 
    // Function get called when selection in rtl is made
    // should return 0 if there are no need in default symbol action processing
    // non 0 otherwise
    virtual int  action_callback(RTL* rtl, symbolArr& selected, miniBrowser *); 
    // Function gets called when rtl goes out of history list & not dispayed anywhere
    // in minibrowser
    virtual void rtl_out_of_scope(RTL *rtl);
    virtual char *get_sort_spec(RTL *rtl);
    // Function should return non 0 if needs set of special buttons (vcr controls)
    // under the list (buttonst are next, prev, current)
    virtual int  need_vcr_control(RTL *rtl);
    // Function called to let api create it\'s own buttons
    // for list control form. (Should use own add_button method)
    // parent should be used as a parent for buttons
    virtual void make_own_buttons(gtBase *parent, gtRTL *, RTL *rtl);
    // Call this function from make_own_buttons to add custom button
    // to the list control form (list supports up to 7 buttons)
//  virtual void add_button(gtPrimitive *);


    static void button_callback(gtPushButton *b, gtEvent*, void* data, gtReason);

    genString   click_proc;
    genString   dclick_proc;
    objArr      buttons;
    int         draw_vcr;
//    Interpreter *interp;   Need to add it here!
};

class miniBrowserButton : public Obj {
  public:
    miniBrowserButton(char *b_name, char *b_script, char *b_script_data, Interpreter *i);
    ~miniBrowserButton(void);

    genString         name;
    genString         script;
    genString         script_data;
    cliListController *list_controller;
    gtRTL             *gt_rtl;
    Interpreter       *interpreter;
};

enum ItemType { ITEM_TEXT, ITEM_ENUM };

class miniBrowserItem;

class miniBrowserItemList {
  public:
    miniBrowserItemList(Interpreter *i, objArr& items, int notify, miniBrowser *mb, char *callback_proc, char *callback_data);
    ~miniBrowserItemList();
    
    int Change_CB(miniBrowserItem *item);
    int Cancel_CB(void);

    genString   callback_proc;
    genString   callback_data;
    int         instant_notify;
    miniBrowser *mini_browser;
    objArr      items;
    Interpreter *interpreter;
};

class optionMenuButton : public Obj {
  public:
    optionMenuButton(Widget w) { widg = w; };
    Widget widg;
};

class miniBrowserItem : public Obj {
  public:
    miniBrowserItem(Widget w, char *n, ItemType t);
    ~miniBrowserItem();

    static void OptionButton_CB(Widget w, XtPointer client_data, XtPointer call_data);
    static void EditField_CB(Widget w, XtPointer client_data, XtPointer call_data);
    
    char *GetText(void);
    void SetText(char *txt);

    miniBrowserItemList *parent;
    genString           name;
    Widget              widg;
    ItemType            type;
    objArr              buttons;
};

extern "C" int TclFindElement(Tcl_Interp *interp, char *list, int listLength, char **elementPtr, char **nextPtr, int *sizePtr,
			      int *bracePtr);

Interpreter* GetActiveInterpreter();
void SetActiveInterpreter(Interpreter *i);

//--------------------------------------------------------------------------------------------------

miniBrowserItemList::miniBrowserItemList(Interpreter *i, objArr& _items, int notify, miniBrowser *mb, char *cb_proc, char *cb_data)
{
    this->callback_proc  = cb_proc;
    this->callback_data  = cb_data;
    this->mini_browser   = mb;
    this->items          = _items;
    this->interpreter    = i;
    this->instant_notify = notify;
    for(int ii = 0; ii < items.size(); ii++){
	miniBrowserItem *item = (miniBrowserItem *)items[ii];
	item->parent          = this;
    }
}

miniBrowserItemList::~miniBrowserItemList(void)
{
    for(int i = 0; i < items.size(); i++)
	delete items[i];
}

int miniBrowserItemList::Change_CB(miniBrowserItem *item)
{
    genString result;
    result = "ok { ";
    if(item){
	if(!instant_notify)
	    return 0;
	genString tmp;
	tmp.printf("{ \"%s\" \"%s\" } ", (char *)item->name, item->GetText());
	result += tmp;
    } else {
	for(int i = 0; i < items.size(); i++){
	    miniBrowserItem *item = (miniBrowserItem *)items[i];
	    genString tmp;
	    tmp.printf("{ \"%s\" \"%s\" } ", (char *)item->name, item->GetText());
	    result += tmp;
	}
    }
    result += "} ";
    result += '{';
    result += callback_data;
    result += '}';
    int do_close_or_advance = call_cli_callback(interpreter, callback_proc, (const char *)result, NULL, NULL);
    if(do_close_or_advance && !instant_notify)
	mini_browser->restore_list();
    return do_close_or_advance;
}

int miniBrowserItemList::Cancel_CB(void)
{
    genString result;
    result = "cancel { ";
    for(int i = 0; i < items.size(); i++){
	miniBrowserItem *item = (miniBrowserItem *)items[i];
	genString tmp;
	tmp.printf("\"%s\" ", item->GetText());  // Probably will need more attention
	result += tmp;
    }
    result += "} ";
    result += '{';
    result += callback_data;
    result += '}';
    int do_close = call_cli_callback(interpreter, callback_proc, (const char *)result, NULL, NULL);
    if(do_close)
	mini_browser->restore_list();
    return do_close;
}

//--------------------------------------------------------------------------------------------------

miniBrowserItem::miniBrowserItem(Widget w, char *n, ItemType t)
{
    widg = w;
    type = t;
    name = n;
}

miniBrowserItem::~miniBrowserItem()
{
    for(int i = 0; i < buttons.size(); i++){
	delete buttons[i];
    }
}

char *miniBrowserItem::GetText(void)
{
    static genString text;

    text = "";
    if(type == ITEM_TEXT){
	char *txt = XmTextGetString(widg);
	if(txt){
	    text = txt;
	    XtFree(txt);
	}
    } else if(type == ITEM_ENUM){
	Widget pb;
	XtVaGetValues(widg, XmNmenuHistory, &pb, NULL);
	// check for "Not Set" option. It is always a first button
	if(buttons.size() != 0){
	    optionMenuButton *ob = (optionMenuButton *)buttons[0];
	    if(pb == ob->widg)
		text = ""; // May be it shoud be some special symbol
	    else
		text = XtName(pb);
	} else
	    text = XtName(pb);
    }
    return (char *)text;
}

void miniBrowserItem::SetText(char *txt)
{
    if(type == ITEM_TEXT){
	XmTextSetString(widg, txt);
    } else if(type == ITEM_ENUM){
	int i;
	for(i = 0; i < buttons.size(); i++){
	    optionMenuButton *omb = (optionMenuButton *)buttons[i];
	    if(strcmp(XtName(omb->widg), txt) == 0){
		Arg args[1];
		XtSetArg(args[0], XmNmenuHistory, omb->widg);
		XtSetValues(widg, args, 1);
		break;
	    }
	}
	if(i == buttons.size()){
	    optionMenuButton *omb = (optionMenuButton *)buttons[0];
	    Arg args[1];
	    XtSetArg(args[0], XmNmenuHistory, omb->widg);
	    XtSetValues(widg, args, 1);
	}
    }
}

void miniBrowserItem::OptionButton_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    miniBrowserItem *item = (miniBrowserItem *)client_data;
    item->parent->Change_CB(item);
}

void miniBrowserItem::EditField_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    miniBrowserItem *item = (miniBrowserItem *)client_data;
    if(item->parent->Change_CB(item)){
	XmProcessTraversal(item->widg, XmTRAVERSE_NEXT_TAB_GROUP);
    }
}

//--------------------------------------------------------------------------------------------------

miniBrowserButton::miniBrowserButton(char *b_name, char *b_script, char *b_script_data, Interpreter *i)
{
    name        = b_name;
    script      = b_script;
    script_data = b_script_data;
    interpreter = i;
}

miniBrowserButton::~miniBrowserButton(void)
{
}

//--------------------------------------------------------------------------------------------------

cliListController::cliListController(char *cproc, char *dcproc, objArr& b)
{
    if(cproc)
	click_proc = cproc;
    if(dcproc)
	dclick_proc = dcproc;
    buttons  = b;
    draw_vcr = 0;
}

void cliListController::selection_callback(RTL *, symbolArr& selected, miniBrowser *)
{
    if(click_proc.length() == 0)
	return;
    call_cli_callback(GetActiveInterpreter(), (const char *)click_proc, "", &selected, NULL);
}

int cliListController::action_callback(RTL *, symbolArr& selected, miniBrowser *)
{
    if(dclick_proc.length() == 0)
	return 1;
    int ret_val = call_cli_callback(GetActiveInterpreter(), (const char *)dclick_proc, "", &selected, NULL);
    return ret_val;
}

char *cliListController::get_sort_spec(RTL *)
{
    return (char *)"ns"; // Non sorted list
}

int cliListController::need_vcr_control(RTL *)
{
    return draw_vcr;
}

void cliListController::make_own_buttons(gtBase *parent, gtRTL *gt_rtl, RTL *)
{
    for(int i = 0; i < buttons.size(); i++){
	genString b_label;
	b_label.printf("_%d_", i);
	miniBrowserButton *b = (miniBrowserButton *)buttons[i];
	b->list_controller   = this;
	b->gt_rtl            = gt_rtl;
	gtPushButton *button = gtPushButton::create(parent, (char *)b_label, (char *)b->name, 
						    cliListController::button_callback, b);
	add_button(button);
    }
}

void cliListController::button_callback(gtPushButton *gt_button, gtEvent*, void* data, gtReason)
{
    miniBrowserButton *b  = (miniBrowserButton *)data;
    cliListController *lc = b->list_controller;
    
    symbolArr selected;
    b->gt_rtl->app_nodes(&selected);
    RTL *rtl           = b->gt_rtl->RTL();
    RTLNodePtr node    = (RTLNode *)rtl->get_root ();
    symbolArr contents = node->rtl_contents();
    call_cli_callback(b->interpreter, b->script, b->script_data, &contents, &selected);
}

void cliListController::rtl_out_of_scope(RTL *)
{
    for(int i = 0; i < buttons.size(); i++){
	miniBrowserButton *button = (miniBrowserButton *)buttons[i];
	delete button;
    }
    buttons.removeAll();
    obj_delete(this);
}

//--------------------------------------------------------------------------------------------------

int cli_set_minibrowser(const char* title, int window, int bring_on_top, int draw_vcr, 
			char *click_proc, char *dclick_proc, symbolArr& arr, objArr& buttons)
{
    static int ind = 0;
    static char buf[30];

    Initialize(cli_set_minibrowser);
    
    if(!title){
	OSapi_sprintf(buf, "list%d", ind++);
	title = buf;
    }
    RTL *rtl                 = new RTL((char*)title);	
    RTLNode* rtl_node        = checked_cast(RTLNode, rtl->get_root());
    rtl_node->rtl_contents() = arr;
    
    cliListController *lc = new cliListController(click_proc, dclick_proc, buttons);
    lc->draw_vcr          = draw_vcr;
    APIhandler_put_rtl(lc, rtl);

    viewerShell * vs = viewerShell::get_current_vs(-1);
    miniBrowser* mb  = vs->get_mini_browser();
    if(window >= 0)
	mb->set_output_window(window);
    mb->browse(rtl);
    if(bring_on_top)
	vs->bring_to_top();
    return TCL_OK;
}

static void add_button(objArr& buttons, Interpreter *i, char *b_name, char *s_name, char *s_data)
{
    miniBrowserButton *b = new miniBrowserButton(b_name, s_name, s_data, i);
    buttons.insert_last(b);
}

static int set_miniCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{   
    Interpreter *i = (Interpreter *)cd;
    symbolArr arr;
    char *clk        = NULL;
    char *dclk       = NULL;
    char *title      = NULL;
    int  pos         = 1;
    int window       = -1;
    int draw_vcr     = 0;
    int bring_on_top = 0;
    objArr buttons;
    if(argc > 1){
	while(pos < argc && argv[pos][0] == '-'){
	    if(strcmp(argv[pos], "-title") == 0)
		title = argv[++pos];
	    else if(strcmp(argv[pos], "-popup") == 0)
		bring_on_top = 1;
	    else if(strcmp(argv[pos], "-vcr") == 0)
		draw_vcr = 1;
	    else if(strcmp(argv[pos], "-click") == 0)
		clk = argv[++pos];
	    else if(strcmp(argv[pos], "-dclick") == 0)
		dclk = argv[++pos];
	    else if(strcmp(argv[pos], "-button") == 0){
		if(argc <= pos + 3){
		    Tcl_AppendResult(interp, argv[0], ": wrong number of -button parameters\n",
				     "shoud be -button button_name proc proc_data\n", NULL);
		    return TCL_ERROR;
		}
		add_button(buttons, i, argv[pos + 1], argv[pos + 2], argv[pos + 3]);
		pos += 3;
	    }
	    else if(strcmp(argv[pos], "-top") == 0)
		window = 0;
	    else if(strcmp(argv[pos], "-bottom") == 0)
		window = 1;
	    pos++;
	}
    }
    if(i->ParseArguments(pos, argc, argv, arr) != 0)
	return TCL_ERROR;
    int retval = cli_set_minibrowser(title, window, bring_on_top, draw_vcr, clk, dclk, arr, buttons); 
    return retval;
}

static int create_text_row(Interpreter *i, gtForm *form, Widget *prev_widget, char *label, char *def_val, 
			   int read_only, Widget *ret_widget, objArr &items_list)
{
    Widget fw = form->rep()->widget();
    Widget i0;
    if(prev_widget == NULL) {
	Widget l0 = XtVaCreateManagedWidget(label, xmLabelGadgetClass, fw,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_FORM,
					    XmNtopOffset, 200,
					    NULL);
	i0 = XtVaCreateManagedWidget("TEXT", xmTextWidgetClass, fw,
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, l0,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     NULL);
	Widget s0 = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, fw,
					    XmNorientation, XmHORIZONTAL,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNrightAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, i0,
					    NULL);
	*ret_widget = s0;
    } else {
	Widget l0 = XtVaCreateManagedWidget(label, xmLabelGadgetClass, fw,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, *prev_widget,
					    XmNtopOffset, 200,
					    NULL);
	i0 = XtVaCreateManagedWidget("TEXT", xmTextWidgetClass, fw,
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, l0,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, *prev_widget,
				     XmNrightAttachment, XmATTACH_FORM,
				     NULL);
	Widget s0 = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, fw,
					    XmNorientation, XmHORIZONTAL,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNrightAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, i0,
					    NULL);
	*ret_widget = s0;
    }
    XmTextSetString(i0, def_val);
    if(read_only)
	XtVaSetValues(i0, XmNeditable, False, NULL);
    miniBrowserItem *item = new miniBrowserItem(i0, label, ITEM_TEXT);
    items_list.insert_last(item);
    XtAddCallback(i0, XmNactivateCallback, miniBrowserItem::EditField_CB, (char*)item);
    return TCL_OK;
}

 
static int create_enum_row(Interpreter *i, gtForm *form, char *label, char *def_val, char *elems, 
			   Widget *prev_widget, Widget *ret_widget, objArr& items_list)
{
    char *p, *element, *next;
    int  size, parenthesized, result;

    Widget menu_choice = NULL;
    Widget fw          = form->rep()->widget();
    Widget pdm         = XmCreatePulldownMenu(fw, (char *)"pull_down", NULL, 0);
    p                  = elems;
    objArr option_buttons;
    miniBrowserItem *item = new miniBrowserItem(NULL, label, ITEM_ENUM);
    Widget button         = XtVaCreateManagedWidget("Not Set", xmPushButtonWidgetClass, pdm, NULL); 
    XtAddCallback(button, XmNactivateCallback, miniBrowserItem::OptionButton_CB, (char*)item);
    optionMenuButton *omb = new optionMenuButton(button);
    option_buttons.insert_last(omb);
    if(def_val != NULL && def_val[0] == 0)
	menu_choice = button;
    do{
	int llen = strlen(p);
	result   = TclFindElement(i->interp, p, llen, &element, &next, &size,
				  &parenthesized);
	if (result != TCL_OK) {
	    return result;
	}
	if(*element == 0)
	    break;
	char *str = new char[size + 1];
	strncpy(str, element, size);
	str[size]     = 0;
	Widget button = XtVaCreateManagedWidget(str, xmPushButtonWidgetClass, pdm, NULL);
	XtAddCallback(button, XmNactivateCallback, miniBrowserItem::OptionButton_CB, (char *)item);
	if(menu_choice == NULL || strcmp(str, def_val) == 0)
	    menu_choice = button;
	optionMenuButton *omb = new optionMenuButton(button);
	option_buttons.insert_last(omb);
	delete [] str;
	p = next;
    } while(*p != 0);
    XmString menu_title = XmStringCreateSimple((char *)"");
    Widget i0;
    if(prev_widget == NULL) {
	Widget l0 = XtVaCreateManagedWidget(label, xmLabelGadgetClass, fw,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_FORM,
					    XmNtopOffset, 200,
					    NULL);
	Arg args[7];
	XtSetArg(args[0], XmNlabelString, menu_title);
	XtSetArg(args[1], XmNsubMenuId, pdm);
	XtSetArg(args[2], XmNtopAttachment, XmATTACH_FORM);
	XtSetArg(args[3], XmNrightAttachment, XmATTACH_FORM);
	XtSetArg(args[4], XmNleftAttachment, XmATTACH_WIDGET);
	XtSetArg(args[5], XmNleftWidget, l0);
	XtSetArg(args[6], XmNmenuHistory, menu_choice);
	i0 = XmCreateOptionMenu(fw, (char *)"op_menu", args, 7);
	XtManageChild(i0);
	Widget s0 = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, fw,
					    XmNorientation, XmHORIZONTAL,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNrightAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, i0,
					    NULL);
	
	*ret_widget = s0;
    } else {
	Widget l0 = XtVaCreateManagedWidget(label, xmLabelGadgetClass, fw,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, *prev_widget,
					    XmNtopOffset, 200,
					    NULL);
	Arg args[8];
	XtSetArg(args[0], XmNlabelString, menu_title);
	XtSetArg(args[1], XmNsubMenuId, pdm);
	XtSetArg(args[2], XmNtopAttachment, XmATTACH_WIDGET);
	XtSetArg(args[3], XmNtopWidget, *prev_widget);
	XtSetArg(args[4], XmNrightAttachment, XmATTACH_FORM);
	XtSetArg(args[5], XmNleftAttachment, XmATTACH_WIDGET);
	XtSetArg(args[6], XmNleftWidget, l0);
	XtSetArg(args[7], XmNmenuHistory, menu_choice);
	i0 = XmCreateOptionMenu(fw, (char *)"op_menu", args, 8);
	XtManageChild(i0);
	Widget s0 = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, fw,
					    XmNorientation, XmHORIZONTAL,
					    XmNleftAttachment, XmATTACH_FORM,
					    XmNrightAttachment, XmATTACH_FORM,
					    XmNtopAttachment, XmATTACH_WIDGET,
					    XmNtopWidget, i0,
					    NULL);
	*ret_widget = s0;
    }
    XmStringFree(menu_title);    
    item->widg            = i0;
    item->buttons         = option_buttons;
    items_list.insert_last(item);
    return TCL_OK;
}

static int parse_row_def(Interpreter *i, gtForm *form, char *layout, objArr &items_list, Widget *prev_widget, Widget *ret_widget)
{
    char *p, *element, *next;
    int  size, parenthesized, result;

    int llen = strlen(layout);
    p        = layout;
    result   = TclFindElement(i->interp, p, llen, &element, &next, &size,
			      &parenthesized);
    if (result != TCL_OK) 
	return result;
    if(*element == 0){
	Tcl_AppendResult(i->interp, "missing label", NULL);
	return TCL_ERROR;
    }
    char *label_name = new char[size + 1];
    strncpy(label_name, element, size);
    label_name[size] = 0;
    p                = next;
    llen             = strlen(p);
    result           = TclFindElement(i->interp, p, llen, &element, &next, &size,
				      &parenthesized);
    if (result != TCL_OK || *element == 0){
	Tcl_AppendResult(i->interp, "missing default value for label ", label_name, NULL);
	delete [] label_name;
	return TCL_ERROR;
    }
    char *def_val = new char[size + 1];
    strncpy(def_val, element, size);
    def_val[size] = 0;
    p             = next;
    llen          = strlen(p);
    result        = TclFindElement(i->interp, p, llen, &element, &next, &size,
				   &parenthesized);
    if (result != TCL_OK || *element == 0){
	Tcl_AppendResult(i->interp, "missing type for label ", label_name, NULL);
	delete [] label_name;
	delete [] def_val;
	return TCL_ERROR;
    }
    char *type_name = new char[size + 1];
    strncpy(type_name, element, size);
    type_name[size] = 0;
    if(strcmp(type_name, "text") == 0)
	create_text_row(i, form, prev_widget, label_name, def_val, 0, ret_widget, items_list);
    else if(strcmp(type_name, "rotext") == 0)
	create_text_row(i, form, prev_widget, label_name, def_val, 1, ret_widget, items_list);
    else if(strcmp(type_name, "enum") == 0) {
	if(create_enum_row(i, form, label_name, def_val, next, prev_widget, ret_widget, items_list) != TCL_OK){
	    delete [] type_name;
	    delete [] label_name;
	    delete [] def_val;
	    return TCL_ERROR;
	}
    } else {
	Tcl_AppendResult(i->interp, "invalid cell type", NULL);
	delete [] type_name;
	delete [] label_name;
	delete [] def_val;
	return TCL_ERROR;
    }
    delete [] type_name;
    delete [] label_name;
    delete [] def_val;
    return TCL_OK;
}

static int parse_layout(Interpreter *i, gtForm *form, objArr& items_list, char *layout)
{
    char *p, *element, *next;
    int  size, parenthesized, result;
    Widget *prev_widget = NULL;
    Widget ret_widget;

    p = layout;
    do{
	int llen = strlen(p);
	result   = TclFindElement(i->interp, p, llen, &element, &next, &size,
				  &parenthesized);
	if (result != TCL_OK) {
	    return result;
	}
	if(*element == 0)
	    return TCL_OK;
	char *row_def = new char[size + 1];
	strncpy(row_def, element, size);
	row_def[size] = 0;
	if(parse_row_def(i, form, row_def, items_list, prev_widget, &ret_widget) != TCL_OK){
	    delete [] row_def;
	    return TCL_ERROR;
	}
	delete [] row_def;
	prev_widget = &ret_widget;
	p           = next;
    } while(*p != 0);
    return TCL_OK;
}

static void cancel_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    miniBrowserItemList *il = (miniBrowserItemList *)client_data;
    if(il->Cancel_CB()){
	il->mini_browser->set_list_cleanup_form(NULL, NULL); // reset this to null to indicate that no more attribute window present
	il->mini_browser->restore_list();
	delete il;
    }
}

static void OK_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    miniBrowserItemList *il = (miniBrowserItemList *)client_data;
    if(il->Change_CB(NULL)){
	il->mini_browser->set_list_cleanup_form(NULL, NULL); // reset this to null to indicate that no more attribute window present
	delete il;
    }
}

static void create_update_cancel_buttons(miniBrowser *mb, gtForm *form, miniBrowserItemList *il)
{
    Widget fw = form->rep()->widget();
    Widget bform = XtVaCreateManagedWidget("button_form", xmFormWidgetClass, fw,
					   XmNfractionBase, 25,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNtopAttachment, XmATTACH_OPPOSITE_FORM,
					   XmNtopOffset, -950,
					   NULL);
    Widget okbut = XtVaCreateManagedWidget("Update",  xmPushButtonWidgetClass, bform,
					   XmNtopAttachment, XmATTACH_POSITION,
					   XmNtopPosition, 1,
					   XmNleftAttachment, XmATTACH_POSITION,
					   XmNleftPosition, 1,
					   XmNrightAttachment, XmATTACH_POSITION,
					   XmNrightPosition, 12,
					   NULL);
    Widget cbut = XtVaCreateManagedWidget("Close",  xmPushButtonWidgetClass, bform,
					  XmNtopAttachment, XmATTACH_POSITION,
					  XmNtopPosition, 1,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, 13, 
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, 24, 
					  NULL);
    XtAddCallback(cbut, XmNactivateCallback, cancel_CB, (char *)il);
    XtAddCallback(okbut, XmNactivateCallback, OK_CB, (char *)il);
    XtUnmanageChild(bform);
    XtManageChild(bform);
}

static void create_cancel_button(miniBrowser *mb, gtForm *form, miniBrowserItemList *il)
{
    Widget fw = form->rep()->widget();
    Widget bform = XtVaCreateManagedWidget("button_form", xmFormWidgetClass, fw,
					   XmNfractionBase, 25,
					   XmNleftAttachment, XmATTACH_FORM,
					   XmNbottomAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNtopAttachment, XmATTACH_OPPOSITE_FORM,
					   XmNtopOffset, -950,
					   NULL);
    Widget cbut = XtVaCreateManagedWidget("Close",  xmPushButtonWidgetClass, bform,
					  XmNtopAttachment, XmATTACH_POSITION,
					  XmNtopPosition, 1,
					  XmNleftAttachment, XmATTACH_POSITION,
					  XmNleftPosition, 5, 
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, 20, 
					  NULL);
    XtAddCallback(cbut, XmNactivateCallback, cancel_CB, (char *)il);
    XtUnmanageChild(bform);
    XtManageChild(bform);
}

static void attribute_editor_cleanup(miniBrowser *mb, void *data)
{
    int window = mb->restore_list();
    if(window >= 0){
	miniBrowserItemList *il = (miniBrowserItemList *)data;
	if(il){
	    il->mini_browser->set_list_cleanup_form(NULL, NULL); // reset this to null to indicate that no more attribute window present
	    delete il;
	}
    }
}

static int mini_restoreCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    viewerShell * vs = viewerShell::get_current_vs(-1);
    miniBrowser* mb  = vs->get_mini_browser();
    attribute_editor_cleanup(mb, NULL);
#if 0
    int window       = mb->restore_list();
    if(window >= 0){
	miniBrowserItemList *il = mini_windows[window];
	if(il)
	    delete il;
	mini_windows[window] = NULL;
    }
#endif
    return TCL_OK;
}

static int mini_createCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    Interpreter *i = (Interpreter *)cd;
    if(argc < 3){
	Tcl_AppendResult(interp, "wrong # args: should be ", argv[0],
			 " layout ok_callback ok_data -option(s) ...", (char *) NULL);
	return TCL_ERROR;
    }
    char *title              = NULL;
    const char *ok_button_name     = "Ok";
    const char *cancel_button_name = "Cancel";
    int  pos                 = 4;
    int window               = -1;
    int instant_notify       = 0;
    int bring_on_top         = 0;
    objArr buttons;
    if(argc > 3){
	while(pos < argc && argv[pos][0] == '-'){
	    if(strcmp(argv[pos], "-title") == 0)
		title = argv[++pos];
	    else if(strcmp(argv[pos], "-popup") == 0)
		bring_on_top = 1;
	    else if(strcmp(argv[pos], "-notify") == 0)
		instant_notify = 1;
	    else if(strcmp(argv[pos], "-ok") == 0)
		ok_button_name = argv[++pos];
	    else if(strcmp(argv[pos], "-cancel") == 0)
		cancel_button_name = argv[++pos];
	    else if(strcmp(argv[pos], "-top") == 0)
		window = 0;
	    else if(strcmp(argv[pos], "-bottom") == 0)
		window = 1;
	    else {
		Tcl_AppendResult(interp, "incorrect option for ", argv[0], NULL);
		return TCL_ERROR;
	    }
	    pos++;
	}
    }
    gtForm *new_form;
    viewerShell * vs = viewerShell::get_current_vs(-1);
    miniBrowser* mb  = vs->get_mini_browser();
    if(bring_on_top)
	vs->bring_to_top();
    mb->restore_list();
    if(window < 0)
	window  = mb->get_next_window();
    new_form = mb->unmanage_list(window);
    objArr items_list;
    int result = parse_layout(i, new_form, items_list, argv[1]);
    if(result != TCL_OK)
	return result;
    miniBrowserItemList *il = new miniBrowserItemList(i, items_list, instant_notify, mb, argv[2], argv[3]);
    mb->set_list_cleanup_form(attribute_editor_cleanup, il);
    if(instant_notify)
	create_cancel_button(mb, new_form, il);
    else
	create_update_cancel_buttons(mb, new_form, il);
    genString res_window;
    res_window.printf("%d", window);
    Tcl_AppendResult(i->interp, (char *)res_window, NULL);
    return result;
}

static int parse_att_def_val(Interpreter *i, char *attr_def, miniBrowserItemList *il)
{
    char *p, *element, *next;
    int  size, parenthesized, result, returnLast;
    
    p        = attr_def;
    int llen = strlen(p);
    result   = TclFindElement(i->interp, p, llen, &element, &next, &size,
			      &parenthesized);
    if (result != TCL_OK) {
	return result;
    }
    if(*element == 0){
	Tcl_AppendResult(i->interp, "'", attr_def, "'", " - missing attribute name", NULL);
	return TCL_ERROR;
    }
    char *attr_name = new char[size + 1];
    strncpy(attr_name, element, size);
    attr_name[size] = 0;
    p               = next;
    llen            = strlen(p);
    result          = TclFindElement(i->interp, p, llen, &element, &next, &size,
				     &parenthesized);
    if (result != TCL_OK) {
	return result;
    }
    if(*element == 0){
	Tcl_AppendResult(i->interp, attr_name, ": missing attribute value", NULL);
	delete [] attr_name;
	return TCL_ERROR;
    }
    char *attr_val = new char[size + 1];
    strncpy(attr_val, element, size);
    attr_val[size] = 0;
    for(int ii = 0; ii < il->items.size(); ii++){
	miniBrowserItem *item = (miniBrowserItem *)il->items[ii];
	if(strcmp((char *)item->name, attr_name) == 0){
	    item->SetText(attr_val);
	    break;
	}
    }
    delete [] attr_name;
    delete [] attr_val;
    return TCL_OK;
}

static int mini_fillCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    char *p, *element, *next;
    int  size, parenthesized, result;

    Interpreter *i = (Interpreter *)cd;
    if(argc != 2){
	Tcl_AppendResult(interp, "wrong # args: should be ", argv[0],
			 " window values", (char *) NULL);
	return TCL_ERROR;
    }
    viewerShell * vs        = viewerShell::get_current_vs(-1);
    miniBrowser* mb         = vs->get_mini_browser();
    miniBrowserItemList *il = (miniBrowserItemList *)mb->get_cleanup_form_data();
    if(il == NULL){
	Tcl_AppendResult(interp, "window is not initialized", NULL);
	return TCL_ERROR;
    }
    p = argv[1];
    do{
	int llen = strlen(p);
	result   = TclFindElement(i->interp, p, llen, &element, &next, &size,
				  &parenthesized);
	if (result != TCL_OK) {
	    return result;
	}
	if(*element == 0)
	    return TCL_OK;
	char *attr_def = new char[size + 1];
	strncpy(attr_def, element, size);
	attr_def[size] = 0;
	if(parse_att_def_val(i, attr_def, il) != TCL_OK){
	    delete [] attr_def;
	    return TCL_ERROR;
	}
	delete [] attr_def;
	p  = next;
    } while(*p != 0);
    return TCL_OK;
}

static int mini_windowAliveCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    if(argc != 1){
	Tcl_AppendResult(interp, argv[0], ": does not require any argument", (char *) NULL);
	return TCL_ERROR;
    }
    viewerShell * vs        = viewerShell::get_current_vs(-1);
    miniBrowser* mb         = vs->get_mini_browser();
    miniBrowserItemList *il = (miniBrowserItemList *)mb->get_cleanup_form_data();
    if(il != NULL)
	Tcl_AppendResult(interp, "1", NULL);
    else
	Tcl_AppendResult(interp, "0", NULL);
    return TCL_OK;
}

static int mini_refreshCmd(ClientData cd, Tcl_Interp *interp, int argc, char **argv)
{
    viewerShell * vs = viewerShell::get_current_vs(-1);
    if(vs == NULL)
	return TCL_OK;
    miniBrowser* mb = vs->get_mini_browser();
    mb->refresh();
    return TCL_OK;
}

static int add_commands()
{
    new cliCommandInfo("set_mini", set_miniCmd);
    new cliCommandInfo("mini_create", mini_createCmd);
    new cliCommandInfo("mini_restore", mini_restoreCmd);
    new cliCommandInfo("mini_fill", mini_fillCmd);
    new cliCommandInfo("mini_refresh", mini_refreshCmd);
    new cliCommandInfo("mini_window_alive", mini_windowAliveCmd);
    return 0;
}

static int add_commands_dummy = add_commands();
