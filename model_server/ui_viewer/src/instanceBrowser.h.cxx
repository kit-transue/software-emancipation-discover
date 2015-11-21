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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <systemMessages.h>

#include <gtForm.h>
#include <gtBase.h>
#include <gtPushButton.h>

#include <objOper.h>
#include <RTL.h>
#include <proj.h>
#include <ddict.h>
#include <genArr.h>
#include <path.h>

#include <viewerShell.h>
#include <view_creation.h>
#include <customize.h>
#include <ste_interface.h>
#include <symbolScope.h>
#include <projList.h>
#include <ldrList.h>
#include <miniBrowser.h>
#include <instanceBrowser.h>

#include <cliUI.h>

Interpreter* GetActiveInterpreter();

class cliInstance : public InstanceController {
  public:
    cliInstance(symbolArr& inst, char *title, char *cscript, miniBrowser *browser = NULL);
    
    virtual int        use_own_icons() { return 0; };
    virtual int        use_own_names() { return 0; };
    virtual symbolPtr  get_symbol(int ind);
    virtual char       *get_sort_spec(RTL *);
    virtual void       selection_callback(RTL*, symbolArr& selected, miniBrowser *);
  private:
    genString click_script;
};

class smtInstance : public InstanceController {
  public:
    smtInstance(symbolArr& inst, char *title, char *full_name, miniBrowser *browser = NULL);
    
    virtual int        use_own_icons() { return 0; };
    virtual int        use_own_names() { return 1; };
    virtual symbolPtr  get_symbol(int ind);
    virtual char       *get_sym_name(int ind);
    virtual char       *get_sort_spec(RTL *);
    virtual int        need_vcr_control(RTL *) { return 1; };
    virtual void       make_own_buttons(gtBase *, gtRTL *, RTL *);
    virtual void       handle_remove_rep(int ind);
  private:
    static void DoneButton(gtPushButton* b, gtEvent*, void* r, gtReason);
};

init_relational(InstanceRep,appTree);
defrel_many_to_one(InstanceRep,Instance,Relational,object);
init_rel_or_ptr(InstanceRep,Instance, relationMode::D,Relational,object, 0);

//-----------------------------------------------------------------------------------------------

cliInstance::cliInstance(symbolArr& inst, char *s_title, char *cscript, miniBrowser *m_browser) : 
           InstanceController(inst, s_title, s_title, m_browser)
{
    click_script  = cscript;
    if(inst.size() != 0){
	symbolPtr sym = inst[0];
	if(click_script.length() != 0){
	    symbolArr syms;
	    syms.insert_last(sym);
	    call_cli_callback(GetActiveInterpreter(), (char *)click_script, "", &syms, NULL);
	} else
	    view_create(sym, Rep_UNKNOWN, 1);
    }
}

symbolPtr cliInstance::get_symbol(int ind)
{
    return InstanceController::get_symbol(ind);
}

char *cliInstance::get_sort_spec(RTL *)
{
    return "ns"; // Non sorted list
}

void cliInstance::selection_callback(RTL*, symbolArr& selected, miniBrowser *)
{
    Initialize(cliInstance::selection_callback);

    symbolPtr sym      = selected[0];
    int       i        = representation_get_index(sym);
    symbolPtr real_sym = instances[i];
    if(click_script.length() != 0){
	symbolArr syms;
	syms.insert_last(real_sym);
	call_cli_callback(GetActiveInterpreter(), (char *)click_script, "", &syms, NULL);
    } else
	view_create(real_sym, Rep_UNKNOWN, 1);
}

//-----------------------------------------------------------------------------------------------

int  focus_on_node(appTreePtr, int, int);

smtInstance::smtInstance(symbolArr& inst, char *s_title, char *f_name, miniBrowser *m_browser) : 
           InstanceController(inst, s_title, f_name, m_browser)
{
    // Immideatly focus on the first item in list
    if(inst.size() != 0){
	ste_finalize();
	RelationalPtr obj = inst[0];
	if(obj && is_smtTree(obj)){
	    smtTree *smt = (smtTree *)obj;
	    focus_on_node(smt, 0, 1);
	}
    }
}

void smtInstance::make_own_buttons(gtBase *parent, gtRTL *, RTL *)
{
    gtPushButton *done = gtPushButton::create(parent, "done_button", "Done", DoneButton, this);
    add_button(done);
}

void smtInstance::DoneButton(gtPushButton*, gtEvent*, void* data, gtReason)
{
    smtInstance *instance = (smtInstance *)data;
    // get All rtls controlled by this controller
    objSet rtls = APIhandler_get_rtls(instance);
    ObjPtr ob;
    ForEach(ob, rtls){ // Delete all rtls from minibrowser
	instance->browser->remove((RTL *)ob);
    }
}

void smtInstance::handle_remove_rep(int ind)
{
    miniBrowser::clear_selections();
    InstanceController::handle_remove_rep(ind);
    // If no more instances (empty list) just delete itself & rtls
    if(instances.size() == 0){
	// get All rtls controlled by this controller
	objSet rtls = APIhandler_get_rtls(this);
	ObjPtr ob;
	ForEach(ob, rtls){ // Delete all rtls from minibrowser
	    browser->remove((RTL *)ob);
	}
    }
}


//unsigned char smtInstance::get_sym_icon(int ind)
//{
//    return PIX_XHARDASSOC;
//}

char *smtInstance::get_sym_name(int ind)
{
    static genString label;
    
    RelationalPtr obj = InstanceController::get_symbol(ind);
    smtTree *smt      = (smtTree *)obj;
    label.printf("%s : %d", (char *)smt->get_name(), smt->src_line_num);
    return label;
}

symbolPtr smtInstance::get_symbol(int ind)
{
// Called to determen icon of symbol. return symbol connected to smt & let ldr 
// to decide which icon to use.
    RelationalPtr obj = InstanceController::get_symbol(ind);
    smtTree *smt      = (smtTree *)obj;
    ddElement *dd     = (ddElement *)get_relation(ref_dd_of_smt, smt);
    if(dd)
	return dd;
    else
	return obj;
}

char *smtInstance::get_sort_spec(RTL *)
{
    return "ns"; // Non sorted list
}

InstanceRep::InstanceRep(symbolPtr sym, InstanceController *c, int i)
{
    Initialize(InstanceRep::InstanceRep);

    if(!sym.is_instance() && sym.relationalp()){  // do not try to convert instance (will restore the file)
	RelationalPtr obj = RelationalPtr(sym);
	Instance_put_object(this, obj);
    }
    controller = c;
    ind        = i;
}

InstanceRep::~InstanceRep()
{
    Initialize(InstanceRep::~InstanceRep);
    
    if(controller)
	controller->handle_remove_rep(ind);
}

void InstanceRep::print(ostream&os, int) const
{
    os << "Index :: " << ind << endl;
}

void InstanceRep::notify(int action_type, Relational* obj,
			 objPropagator*, RelType* rel)
{
    Initialize(InstanceRep::notify);

    if(action_type & relationMode::M){
	if(rel == Instance_of_object || rel == object_of_Instance){
	    if(controller)
		controller->handle_modification(ind, this);
	}
    }
}

void InstanceRep::handle_remove(void)
{
    Initialize(InstanceRep::handle_remove);

    //   if(controller)
//	controller->handle_remove_rep(ind);
//    obj_delete(this);
}

char *InstanceRep::get_name() const
{
    if(controller)
	return controller->get_sym_name(ind);
    else
	return "<unknown>";
}

unsigned char InstanceRep::get_icon()
{
    if(controller)
	return controller->get_sym_icon(ind);
    else
	return PIX_XUNKNOWN;
}

symbolPtr InstanceRep::get_associated_symbol(void)
{
    Initialize(InstanceRep::get_associated_symbol);
    
    if(controller)
	return controller->get_symbol(ind);
    Assert(controller);
    symbolPtr null;
    return null;
}

int InstanceRep::use_own_icon(void)
{
    Initialize(InstanceRep::use_own_icon);

    if(controller)
	return controller->use_own_icons();
    else
	return 1;
}

int InstanceRep::use_own_name(void)
{
    Initialize(InstanceRep::use_own_name);

    if(controller)
	return controller->use_own_names();
    else
	return 0;
}

InstanceController::InstanceController(symbolArr& inst, char *s_title, char *f_name, miniBrowser *m_browser)
{
    Initialize(InstanceController::InstanceController);
    
    in_destructor = 0;
    instances     = inst;
    for(int i = 0; i < instances.size(); i++){
	symbolPtr sym    = instances[i];
	InstanceRep *rep = new InstanceRep(sym, this, i);
	representation.insert_last(rep);
    }
    this->full_name = f_name;
    this->title     = s_title;
    this->browser   = m_browser;
}

InstanceController::~InstanceController()
{
    Initialize(InstanceController::~InstanceController);

    in_destructor = 1;
    for(int i = 0; i < representation.size(); i++){
	symbolPtr sym     = representation[i];
	RelationalPtr obj = RelationalPtr(sym);
	delete obj;
    }
//    printf("Controller dying\n");
//    OSapi_fflush(stdout);
}

int InstanceController::use_own_icons(void)
{
    Initialize(InstanceController::use_own_icons);

    return 1;
}

symbolPtr InstanceController::get_symbol(int ind)
{
    Initialize(InstanceController::get_symbol);

    return instances[ind];
}

unsigned char InstanceController::get_sym_icon(int)
{
    Initialize(InstanceController::get_icon);
    
    return PIX_XUNKNOWN;
}

int InstanceController::use_own_names(void)
{
    Initialize(InstanceController::use_own_icons);

    return 0;
}

char *InstanceController::get_sym_name(int)
{
    Initialize(InstanceController::get_sym_name);

    return "<UNDEFINED>";
}

symbolArr& InstanceController::get_representation(void)
{
    Initialize(InstanceController::get_representation);
    
    return representation;
}

void InstanceController::selection_callback(RTL*, symbolArr& selected, miniBrowser *)
{
    Initialize(InstanceController::selection_callback);

    RelationalPtr obj = representation_get_real(selected[0]);
    ste_finalize();
    if(obj && is_smtTree(obj)){
	smtTree *smt = (smtTree *)obj;
	focus_on_node(smt, 0, 1);
    }
}

Relational* InstanceController::representation_get_real(symbolPtr sym)
{
    Initialize(InstanceController::representation_get_real);
    
    RelationalPtr rp = sym;
    if(rp && is_InstanceRep(rp)){
	InstanceRep *rep  = (InstanceRep *)rp;
	RelationalPtr obj = Instance_get_object(rep);
	return obj;
    }
    return NULL;
}

int InstanceController::representation_get_index(symbolPtr sym)
{
    Initialize(InstanceController::representation_get_real);
    
    RelationalPtr rp = sym;
    if(rp && is_InstanceRep(rp)){
	InstanceRep *rep = (InstanceRep *)rp;
	return rep->get_index();
    }
    return 0;
}

int InstanceController::rtl_dying(RTL *)
{
    Initialize(InstanceController::rtl_dying);
    
    return 1;
}

void InstanceController::handle_modification(int, InstanceRep *rep)
{
    Initialize(InstanceController::handle_modification);
    
    // Update all rtls
    objSet rtls = APIhandler_get_rtls(this);
    ObjPtr ob;
    ForEach(ob, rtls){
	RTL *rtl = (RTL *)ob;
	rtl->notify(relationMode::M, rep, NULL, NULL);
    }
}

void InstanceController::handle_remove_rep(int ind)
{
    Initialize(InstanceController::handle_delete_rep);
    
    if(!in_destructor){
	instances.remove_index(ind);
	representation.remove_index(ind);
    }
    // Re-index
    for(int i = ind; i < representation.size(); i++){
	symbolPtr sym     = representation[i];
	RelationalPtr obj = RelationalPtr(sym);
	InstanceRep *rep  = (InstanceRep *)obj;
	rep->set_index(i);
    }
}

void InstanceController::browse(void)
{
    Initialize(InstanceController::browse);
    
    symbolArr &rep      = get_representation();
    RTLPtr new_rtl      = new RTL(title);
    RTLNodePtr rtl_node = checked_cast(RTLNode, new_rtl->get_root());
    new_rtl->set_phys_name(full_name);
    for(int i = 0; i < rep.size(); i++){
	rtl_node->rtl_insert(rep[i]);
    }
    APIhandler_put_rtl(this, new_rtl);
    if(browser)
	browser->browse(rtl_node);
    else{
	viewerShell* vsh = view_target_viewer()->get_shell();
	vsh->get_mini_browser()->browse(rtl_node);
    }
}

InstanceAPI::InstanceAPI(symbolArr& domain)
{
    Initialize(InstanceAPI::InstanceAPI);
    
    domain_selection = domain;
}

InstanceAPI::~InstanceAPI()
{
    Initialize(InstanceAPI::~InstanceAPI);
//    printf("APIhadler dying\n");
//    OSapi_fflush(stdout);
}

int InstanceAPI::create_instance_array(symbolPtr selection)
{
    Initialize(InstanceAPI::create_instance_array);
    
    int ret_val = 1;   // Tell minibrowser to select on the node
    instances.removeAll();
    symbolPtr xselection = selection.get_xrefSymbol();
    if(xselection.xrisnull())
	return ret_val;
    if(!xselection.is_loaded_or_cheap())
	return ret_val;
    appPtr selection_app = xselection.get_def_app();
    if(!selection_app)
	return ret_val;
    smtTree *selection_smt = NULL;
    if(xselection.get_kind() == DD_FUNC_DECL || xselection.get_kind() == DD_CLASS){
	ddElement *xdd = xselection->get_def_dd();
	if(xdd){
	    selection_smt = (smtTree *)get_relation(def_smt_of_dd, xdd);
	    ret_val       = 0;  // Will not select on clicked item, later InstanceController will
	    // point to correct instance
	}
    }
    for(int i = 0; i < domain_selection.size(); i++){
	symbolPtr domain = domain_selection[i];
	symbolPtr xsym   = domain.get_xrefSymbol();
	if (xsym.xrisnull())
	    continue;
	ddElement *dd = xsym.get_dds(selection_app);
	if(!dd)
	    continue;
	objSet smt_set;
	smt_set = get_relation(ref_smt_of_dd, dd);
	ObjPtr  ob;
	smtTree *smt;
	ForEach(ob, smt_set){
	    smt = checked_cast(smtTree, ob);
	    if(selection_smt){
		if(smt->subtree_of(selection_smt))
		    instances.insert_last(smt);
	    }else
		instances.insert_last(smt);
	    ret_val = 0;
	}
    }
    return ret_val;
}

void InstanceAPI::create_list_name(symbolArr& array, genString& name)
{
    Initialize(InstanceAPI::create_list_name);

    symbolPtr selection = array[0];
    genString tmp;
    tmp = "Instances of ";
    if(selection.relationalp()){
	RelationalPtr obj = selection;
	if(obj)
	    tmp += obj->get_name();
	else
	    tmp += "<noname>";
    }else{
	symbolPtr xsym = selection.get_xrefSymbol();
	if(xsym.isnotnull())
	    tmp += xsym.get_name();
	else
	    tmp += "<noname>";
    }
    if(strlen((char *)tmp) >= 25){
	char *str = (char *)tmp;
	str[25]   = 0;
    }
    if(array.size() > 1)
	tmp += ",...";
    name = tmp;
}

int InstanceAPI::rtl_dying(RTL *)
{
    Initialize(InstanceAPI::rtl_dying);
    
    return 1;
}

InstanceController *InstanceAPI::create_instance_controller(symbolArr& inst, char *name, char *full_name,
							    symbolPtr, miniBrowser *browser)
{
    Initialize(InstanceAPI::create_instance_controller);
    
//    return new InstanceController(inst);
    return new smtInstance(inst, name, full_name, browser);
}

static int compare_smts(void *x, void *y)
{
    smtTree *smt1 = *(smtTree **)x;
    smtTree *smt2 = *(smtTree **)y;
    
    if(smt1->src_line_num > smt2->src_line_num)
	return 1;
    else
	if(smt1->src_line_num < smt2->src_line_num)
	    return -1;
	else
	    return 0;
}

void InstanceAPI::sort_instances(void)
{  
    Initialize(InstanceAPI::sort_instances);
    // Copy instances to obj array, sort them based on line number
    // and put them back into instance array
    objArr objinst;
    int i;
    for(i = 0; i < instances.size(); i++){
	Relational *obj = instances[i];
	objinst.insert_last(obj);
    }
    objinst.sort((sortfunctionPtr)compare_smts);
    for(i = 0; i < objinst.size(); i++)
	instances[i] = objinst[i];
}

int InstanceAPI::action_callback(RTL*, symbolArr& selected, miniBrowser *browser)
{
    Initialize(InstanceAPI::action_callback);

    InstanceController *controller = NULL;
    symbolPtr selection            = selected[0];
    symbolArr cli_results;
    genString cli_title;
    genString cli_click_cb;
    int       ret_val;
    if(cli_instance_hook(selection, domain_selection, cli_results, cli_title, cli_click_cb)){
	if(cli_results.size() == 0){
	    msg("No_instances_found:instanceBrowser.h.C", normal_sev) << eom;
	    return 0;
	}
	if(cli_title.length() == 0){
	    genString name, full_name, tmp;
	    create_list_name(domain_selection, name);
	    tmp        = selection.get_name();
	    if(tmp.length() > 40){
		char *p = tmp;
		p[40]   = 0;
	    }
	    full_name  = name;
	    full_name += " (";
	    full_name += tmp;
	    full_name += ")";	    
	    cli_title  = full_name;
	}
	controller = new cliInstance(cli_results, cli_title, cli_click_cb, browser);
	ret_val    = 0;
    } else {
	ret_val = create_instance_array(selection);
	if(instances.size() == 0){
	    msg("No_instances_found:instanceBrowser.h.C", normal_sev) << eom;
	    return ret_val;
	}
	genString name, full_name, tmp;
	create_list_name(domain_selection, name);
	tmp        = selection.get_name();
	if(tmp.length() > 40){
	    char *p = tmp;
	    p[40]   = 0;
	}
	full_name  = name;
	full_name += " (";
	full_name += tmp;
	full_name += ")";
	sort_instances();
	controller = create_instance_controller(instances, name, full_name, selection, browser);
    }
    controller->browse();
    return ret_val;
}

