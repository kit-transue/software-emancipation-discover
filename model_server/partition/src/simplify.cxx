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
#include <Relational.h>
#include <objRelation.h>
#include <db.h>
#include <app.h>
#include <db_intern.h>
#include <ddict.h>
#include <xref.h>
#include <view.h>
#include <objOperate.h>
#include <steBuffer.h>
#include <ldrSelection.h>
#include <ldrHeader.h>
#include <OperPoint.h>
#include <stePoint.h>
#include <path.h>
#include <viewerShell.h>
#include <ste_send_message.h>
#include <top_widgets.h>
#include <machdep.h>
#include <Interpreter.h>
#include <xxinterface.h>

Interpreter *InitializeCLIInterpreter(void);
int cli_eval_string(const char* cmd, Interpreter *i);

/////////////////////////////////////////////////////////
int simplify_headers(const char* fn, const char* ln)
{
  Initialize(simplify_headers(const char*));
  
  if (!fn || !ln)
    return 0;

  if (_lo(LIC_SIMPLIFY)) {
    _lm(LIC_SIMPLIFY);
    msg("ERROR: No license is found") << eom;
    return 0;
  }

  if(OSapi_access(fn, W_OK) < 0) {
    msg("ERROR: Module $1 is read only\nCannot execute simplification on read only modules") << fn << eom;
    _li(LIC_SIMPLIFY);
    return 0;
  }

  if(!xref_not_header_file((char*)fn))
    {
      msg("ERROR: Cannot simplify header files") << eom;
      _li(LIC_SIMPLIFY);
      return 0;
    }

  push_busy_cursor();

  bool verbose = customize::getBoolPref("Simplify.Verbose");
  bool print = customize::getBoolPref("Simplify.PrintScreen");
  bool nocom = customize::getBoolPref("Simplify.NoComments");
  bool nosub = customize::getBoolPref("Simplify.NoSubstitutions");
  bool short_sub_names = customize::getBoolPref("Simplify.UseShortSubNames");
  const genString& excmd = customize::getStrPref("Simplify.VerifyExclusionsCmd");
  const genString& log = customize::getStrPref("Simplify.LogFile");		

  genString cmd("dis_simplify -modify");
  
  if (verbose)
    cmd += " -verbose";
  if (print)
    cmd += " -print";
  if (nocom)
    cmd += " -nocomments";
  if (nosub)
    cmd += " -nosub";
  if (short_sub_names)
    cmd += " -short_sub_names";
  if (log.length())
    {
      cmd += " -log ";
      cmd += log;
    }
  if (excmd.length())
    {
      cmd += " -cmd ";
      cmd += excmd;
    }
  cmd += " \"";
  cmd += ln;
  cmd += '"';

  int ret = 1;

  Interpreter* i = GetActiveInterpreter();
  if (!i)
    {
      i = InitializeCLIInterpreter();
    }

  if (i)
    {
      cli_eval_string(cmd, i);
      genString res;
      i->GetResult(res);

      if (strcmp(res, "OK") != 0)
	{
	  ret = 0;
	  msg("Simplify headers returned error $1", error_sev) << res.str() << eom;
	}
    }
  
  pop_cursor();

  _li(LIC_SIMPLIFY);

  return ret;
}

// file_is_not_viewable defined in xref.h.C
extern bool file_is_not_viewable(symbolPtr &file);

/////////////////////////////////////////////////////////////////////////////////
void collect_modules_for_simplify(projNode *pn)
{
  Initialize(collect_modules_for_simplify);
  
  pn->force_refresh(1);
  symbolArr& proj_contents = pn->rtl_contents();
  symbolPtr sym;
  ForEachS(sym, proj_contents) 
    {
      if (sym.get_kind() == DD_MODULE)
	{
	  genString ln;
	  sym.get_name(ln);
	  genString fn;
	  projHeader::ln_to_fn(ln, fn);

	  // Only get source files
	  if (!xref_not_header_file(fn) || !xref_not_txt_file(fn))
	    continue;

    	  if (file_is_not_viewable(sym))
    	    continue;  // is object file (e.g. Java .jar)

	  if(OSapi_access((char *)fn, W_OK) < 0) 
	    {
	      msg("Skipping read only module $1\nCannot execute simplification on read only modules", warning_sev) << fn.str() << eom;
	      continue;
	    }

	  msg("Simplifying module : $1", normal_sev) << (const char*) fn << eom;
	  simplify_headers(fn, ln);

	} 
      else if (sym.get_kind() == DD_PROJECT)  
	{
	  appPtr app_head = sym.get_def_app();
	  if(app_head && is_projHeader(app_head)) 
	    {
	      projNode *child_proj = projNodePtr(app_head->get_root());
	      collect_modules_for_simplify(child_proj);
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////////////////
void simplify_batch(char const *proj_name)
{
    Initialize(simplify_batch);
    
    if (_lo(LIC_SIMPLIFY)) {
	_lm(LIC_SIMPLIFY);
	return;
    }

    // add all root projects in the scan list
    projList::search_list->scan_projects("/");

    // get to project node from project name
    projNode *projnode = projNode::find_project (proj_name);
    if (!projnode) {
	msg("Error: Project \"$1\" can not be found in the PDF \"$2\".") << proj_name << eoarg << get_PDF() << eom;
	msg("The cause might be one of the following two reasons:") << eom;
	msg("1. Project name \"$1\" is not defined in this PDF.") << proj_name << eom;
	msg("2. The root directory does not exist.") << eom;
	_li(LIC_SIMPLIFY);
	return;
    }
    // set the projnode to be current project
    projNode::set_home_proj (projnode);
    collect_modules_for_simplify(projnode);
    _li(LIC_SIMPLIFY);
}

void do_interactive_simplify(genString& logical_names)
{
    Initialize(do_interactive_simplify(genString& logical_names));
    
    push_busy_cursor();
    genString cmd("dis_interactive_simplify {");
    cmd += logical_names;
    cmd += "}";
    Interpreter* i = GetActiveInterpreter();
    if (!i){
	i = InitializeCLIInterpreter();
    }
    if (i){
	cli_eval_string(cmd, i);
	genString res;
	i->GetResult(res);
	if (strcmp(res, "OK") != 0){
	    if(strcmp(res, "") == 0) {
		msg("Simplify script failed without returning a result", error_sev) << eom;
	    } else {
	  	msg("Simplify script returned error $1", error_sev) << res.str() << eom;
	    }
	}
    }
    pop_cursor();
}

void simplify_headers(symbolArr& arr)
{
    Initialize(simplify_headers(symbolArr&));

    if (_lo(LIC_SIMPLIFY)) {
	_lm(LIC_SIMPLIFY);
	msg("ERROR: No license is found") << eom;
	return;
    }
    genString logical_names;
    int num_mod = 0;
    symbolPtr sym;
    ForEachS(sym, arr){
	if (sym.get_kind()==DD_MODULE && 
	    xref_not_header_file(sym.get_name())){
	    genString ln;
	    sym.get_name(ln);
	    genString fn;
	    projNode *pr = projHeader::get_pmod_project_from_ln(ln);
	    if (pr) {
		pr->ln_to_fn(ln, fn);
	    } else {
	        projHeader::ln_to_fn(ln, fn);
	    }

	    projModule* mod;
	    projNode* prj = projHeader::ln_to_projNode(ln);	  
	    if (prj){
		mod = prj->find_module(ln);
		if (mod){
		    if (mod->is_modified()){
			db_save_or_cancel(mod->get_app(), NULL,1);
		    }	  
		    if (mod->is_loaded() && mod->get_app()){
			obj_unload(mod->get_app());
		    }
		}
	    }
	    if(OSapi_access(fn, W_OK) < 0) {
		msg("ERROR: Module $1 is read only\nCannot execute simplification on read only modules") << (char *)fn << eom;
	    } else if (file_is_not_viewable(sym)) {
		msg("ERROR: $1 is an object code module.") << (char *)fn << eom;
	    } else {
		++num_mod;
	        logical_names += " {";
		logical_names += ln;
		logical_names += "}";
	    }
       	}
    }   
    if (num_mod == 0) {
      msg("ERROR: No modules are selected") << eom;
    } else {
      do_interactive_simplify(logical_names);
    }
    _li(LIC_SIMPLIFY);
}




