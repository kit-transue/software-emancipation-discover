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
#include <objOper.h>

#include "representation.h"
#include <genError.h>
#include <objArr.h>
#include <objRawApp.h>
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <smt.h>
#include <xref.h>
#include <fileCache.h>
#include <sys/param.h>
#include <ste_send_message.h>
#include <systemMessages.h>
#include <ste_interface.h>
#include <machdep.h>
#include <steHeader.h>
#include <path.h>
#include <top_widgets.h>
#include <cmd.h>
#include <driver_mode.h>

extern void  gt_oneshot(void (*fun)(void*), void*data, int ping);

extern void ste_interface_toggle_app( objRawApp *appl, app*h );

defrel_one_to_one(objRawApp,raw,app,smt);
init_rel_or_ptr(objRawApp,raw,0,app,smt,0);

struct appRef : public Relational
{
  define_relational(appRef,Relational);
  genString filename;
    appRef(app*);
    ~appRef();
};
init_relational(appRef,Relational);
defrel_one_to_one(appRef,ref,app,app);
init_rel_or_ptr(appRef,ref,0,app,app,0);

static int LOAD_DEBUG = OSapi_getenv("LOAD_DEBUG") != NULL;
extern "C" void gen_count();
appRef::appRef(app*hh)
{
  filename = hh->get_name();
  app_put_ref(hh, this);
  if(LOAD_DEBUG){
    msg("create appRef $1\n") << (char*) filename << eom;
    gen_count();
  }
}
appRef::~appRef()
{
  if(LOAD_DEBUG){
    msg("delete appRef $1\n") << (char*) filename << eom;
    gen_count();
  }
}


extern app *proj_path_find_app (const char *fname, int no_raw = 0);
extern void proj_path_report_app (app *, projNode *prj = NULL);
extern app *proj_path_convert_app (genString& path, genString& real_path, projNode *prj = NULL);

appPtr proj_restore_raw_file (const char* filename);

extern int gt_process_events();
static int procevents(void * raw_ref)
{
  gt_process_events();
  objRawApp* rapp = (objRawApp*) ref_get_app((appRef*) raw_ref);
  return rapp ? 0 : 1;
}

extern app *proj_restore_file(projNode*that, const char *fn, const char *ln, 
      projModule * mod, int(*)(void*), void*);

inline const char * get_suffix (const char *f_name)
{
    return (f_name) ? strrchr (f_name, '.') : NULL;
}

app* proj_load_model(const char * f_name, appRef * raw_ref)
{
    Initialize(proj_load_model);

    genString lnm;
    projNode *prj = NULL;
    appPtr ah = NULL;

    projHeader::fn_to_ln (f_name, lnm, &prj, 0, 1);

    if (prj && lnm.length()) {
	genString filename;
	prj->ln_to_fn (lnm, filename);
	ah = proj_restore_file(prj, filename, lnm, NULL, 
			       raw_ref ? procevents : NULL, raw_ref);
    }
    return ah;
}


static app* proj_load_model(appRef * raw_ref)
{
  
  objRawApp* old_ah = (objRawApp*) ref_get_app(raw_ref);
  if(!old_ah)
    return NULL;	

  const char* f_name = old_ah->get_name();
  return
        proj_load_model(f_name, raw_ref);
}
int proj_loading_in_progress(objRawApp*old_ah)
{
  if (old_ah->view_kind() == 2){
    ste_print_message("Loading of file in progress ...");
    return 1;
  }
  return 0;
}

app* proj_load_model(objRawApp* old_ah)
{
  if(!old_ah)
	return NULL;	
  if(proj_loading_in_progress(old_ah))
    return NULL;
  
  const char* f_name = old_ah->get_name();
  return
        proj_load_model(f_name, NULL);
}

static int async_load_up = 0;

static void toggle_apps (void *dt)
{
   Initialize(static toggle_apps);

   Assert(dt);
   Assert(async_load_up);

   app *new_app = ref_get_app(checked_cast(appRef,dt)); 
   delete RelationalPtr(dt);
   if(!new_app) return;

   objRawApp *raw_h = smt_get_raw(new_app);
   if (raw_h) {
	push_busy_cursor ();
	ste_finalize ((viewPtr)NULL);
	raw_h->view_kind(1);
        ste_interface_toggle_app (raw_h, new_app);
	new_app->add_to_prop_rtl();
	pop_cursor();
   }

   async_load_up = 0;

   genString msg;
   msg.printf (TXT("Loaded Model for %s"), new_app->get_name());
   ste_print_message ((char *)msg);
}

extern void ste_interface_set_readwrite (objRawApp *ah);

//void gt_delete_pusher();
//void gt_setup_pusher(int i, int j);

static void toggle_apps_pre(void*data)
{
 gt_oneshot (toggle_apps, data, 0);
}

static int no_loading_in_progress = 0;
int proj_no_delayed_loads() {return no_loading_in_progress;}

static void load_model (void * void_ah)
{

    if (async_load_up) {
	pset_send_own_callback (load_model, void_ah);
	return;
    }

    appRef* raw_ref = (appRef*) void_ah;
    objRawApp *ah;

    if(procevents(void_ah)){
       goto end;
    }

    ah = (objRawApp*) ref_get_app(raw_ref);

  app* new_app;

//  gt_setup_pusher(1000, 3);
  async_load_up = 1;
  genError*err;

  genTry{
     new_app = proj_load_model (raw_ref);
  } genCatch(err) {
     new_app = NULL;
  } genEndtry;

//  gt_delete_pusher();
  ah = (objRawApp*)ref_get_app(raw_ref);

  if(!new_app) {
    async_load_up = 0;
    if(ah)
      ste_interface_set_readwrite (ah);
    ste_print_message (TXT("Model does not exist."));
  } else {
    if(ah)
      smt_put_raw(new_app,ah);
    async_load_up = 1;

    appRef * ref = new appRef(new_app);
//    app_put_ref(new_app, ref);
//    pset_send_own_callback (toggle_apps, (void *)ref);
//    pset_send_own_callback (toggle_apps_pre, (void *)ref);
    
    if(! procevents(raw_ref))
//	gt_oneshot (toggle_apps_pre, (void *)ref, 0);
//	gt_oneshot (toggle_apps, (void *)ref, 0);
        pset_send_own_callback (toggle_apps, (void *)ref);


  }
 end:
  delete raw_ref;
  if(no_loading_in_progress > 0)
    -- no_loading_in_progress;
// cout << "load count: " << proj_no_delayed_loads() << endl;

}

extern int get_RTL_from_file(char *filename, symbolArr& sel);

appPtr proj_create_raw_app_by_name(genString& filename)
{
    Initialize(proj_create_raw_app_by_name);
    genString lnm, proj_filename;
    projNode *prj = NULL;

    projHeader::fn_to_ln (filename, lnm, &prj, 0, 1);

    if (!prj || !lnm.length())
	prj = NULL;
    else
    {
	prj->ln_to_fn (lnm, proj_filename);
        if (!proj_filename.length())
        {
            prj = 0;
            projHeader::ln_to_fn (lnm, proj_filename);
            if (proj_filename.length())
                projHeader::fn_to_ln (proj_filename, lnm, &prj, 0, 1);
        }
    }

    genString realname;         // for realpath resolved file name
    app *ah = NULL;
    if (prj) {
	ah = proj_path_convert_app (proj_filename, realname, prj);
    } else
	ah = proj_path_convert_app (filename, realname, prj);
    
    if (!ah){
	if (prj)
	    ah = proj_restore_raw_file(proj_filename);
	else
	    ah = proj_restore_raw_file(realname);
      }
    
    if(ah)
       proj_path_report_app (ah, prj);

    return ah;
}

static void load_model_pre (void * rap)
{
   pset_send_own_callback (load_model, rap);
}

static app* get_existing_app(const char *f_name, genString& filename)
{
    project_convert_filename((char *) f_name, filename);

    app* ah = proj_path_find_app ((const char *)filename);

    if (ah && is_smtHeader(ah)) {
	smtHeader* sh = (smtHeader*)ah;
	sh->get_root();
	if (!sh->srcbuf) {
	    obj_unload(sh);
	    ah = NULL;
	}
    }
    return ah;
}

static appPtr proj_get_raw_app_opt(const char* f_name, int force_model, 
                           int check_existing)
{
    genString filename;
    app* ah;
    if(check_existing){
        ah = get_existing_app(f_name, filename);
        if(ah) return ah;
        if(filename.length())
           f_name = filename;
        else
           return NULL;
    } else {
       filename = f_name;
    }
    ah = proj_create_raw_app_by_name(filename);
    if(ah && is_objRawApp(ah)){
      if(force_model){
	((objRawApp *)ah)->view_kind(2);    // flag "do-not-toggle"
	ah->set_read_only(1);

    appRef * ref = new appRef(ah);
//    app_put_ref(ah, ref);

        ++ no_loading_in_progress;

	pset_send_own_callback (load_model_pre, (void*)ref);
	ste_print_message (TXT("Looking for Model ..."));
      } else {
        projModule *mod = appHeader_get_projectModule(ah);
	genString pset_file = get_paraset_file_name((const char*)filename, mod);
	int has_pset = pset_file.str() ? has_paraset_file(pset_file) : 0;
	if(has_pset){
	  ((objRawApp *)ah)->view_kind(1);
	  ah->set_read_only(1);
	}
      }
    }
    return ah;
}

appPtr proj_get_raw_app(const char* f_name)
{
  return
    proj_get_raw_app_opt(f_name, 0, 1);
  
}


appPtr proj_get_app_asinch (const char* f_name)
{  
  return 
    proj_get_raw_app_opt (f_name, 1, 1);
}

static char* delayed_disabled = OSapi_getenv("PSET_DISABLE_DELAYED_MODEL");

static int asinch_mode(const char *f_name )
{
  int no_delayed = (delayed_disabled) ? 1 : 0;

  if(no_delayed || is_model_build())
    return 0;

  VALIDATE {
     return 0;
  }

  // Should be an ascii file
  
  const char *suffix = get_suffix (f_name);
  if ( suffix && (strcmp(suffix, ".pset") == 0) )
      return 0;

  if (global_cache.is_ascii (f_name))
      return 1;

  return 0;
}
void check_auto_save_copy(char *fn);
appPtr get_create_app_by_name (const char* f_name)
{
    if (!(f_name && f_name[0]))
        return NULL;

    static genString filename;
    app* ah = get_existing_app(f_name, filename);
    if(ah) 
       return ah;
    if(filename.length())
       f_name = filename;

    check_auto_save_copy((char*) f_name);

    if(asinch_mode(f_name))
       return  proj_get_raw_app_opt (f_name, 1, 0);


    if(!ah)
        ah = proj_load_model(filename, NULL);

    if(!ah)
        ah = proj_create_raw_app_by_name(filename);

    return ah;
}

/*
   START-LOG-------------------------------------------

   $Log: get_create_app.C  $
   Revision 1.16 2001/07/25 20:45:47EDT Sudha Kallem (sudha) 
   Changes to support model_server.

   END-LOG---------------------------------------------
*/








