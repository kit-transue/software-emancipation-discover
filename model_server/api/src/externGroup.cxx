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
#include "Interpreter.h"
#include "externApp.h"
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genString.h"
#include <sys/stat.h>
#include "machdep.h"
#include "scopeMgr.h"
#include <tcl.h>
#include "symbolArr.h"
#include "symbolSet.h"
#include "cLibraryFunctions.h"
#include "msg.h"
#include <TclList.h>

init_relational(extGroupApp,externApp);
void (*gmgr_refresh_hook) ();

static void app_delete(externApp*ap)
{
  externInfo* ti = ap->info();
  ti->app_delete(ap);
}


objNameSet&
extGroupApp::get_all(void)
{
  return
   * extGroupApp::type_info()->get_apps();
}

extGroupApp*
extGroupApp::rename(char const* new_name)
{
  extGroupApp* new_app = get(new_name);
  if(new_app)
    return NULL;

  objNameSet& apps = get_all();   
  apps.remove(*this);

  externInfo* ti = type_info();
  genString new_phys_fname;		
  char const *old_phys_fname = get_phys_name();
  if (old_phys_fname) {
      genString extern_dir;
      ti->get_extdir_from_physname(get_name(), old_phys_fname, extern_dir);
      ti->fname(new_name, extern_dir.str(), new_phys_fname);
  }
  else {
      ti->new_fname(new_name, new_phys_fname);
  }
  
  this->title = new_name;
  this->set_filename(new_phys_fname);
  this->set_phys_name(new_phys_fname);

  apps.insert(this);
  return this;
}

extGroupApp::~extGroupApp()
{
  if(arr) delete arr;
  objNameSet& apps = get_all();   
  apps.remove(*this);
}

symbolArr* extGroupApp::get_sym_arr()
{
  return arr;
}

void extGroupApp::set_sym_arr(symbolArr* symarr)
{
  arr = symarr;
}


#define COPY_CHAR    1
#define REPLACE_CHAR 2

bool extern_canonic_name(char const* name, genString& new_name)
{
//    char* special_chars  = "/*()^$|&[]";
    int  state           = COPY_CHAR;
    bool changed         = false;
    
    char const *p  = name;
    new_name       = "";
    int ch;
    while((ch = *p) != 0){
	if(!((ch=='_') || isalnum(ch))){
	    if(state == COPY_CHAR){
		new_name += '_';
		state     = REPLACE_CHAR;
		changed   = true;
	    } else if(state == REPLACE_CHAR){
		/* do nothing */
	    }
	} else {
#ifdef _WIN32
	    int lowch = tolower(ch);
	    if(lowch != ch){
		changed   = true;
		new_name += (char)lowch;
	    } else
		new_name += (char)ch;
#else
	    new_name += *p;
#endif	   
	    if(state == REPLACE_CHAR)
		state = COPY_CHAR;
	}
	p++;
    }
    return changed;
}

extGroupApp*
extGroupApp::get(char const *name)
{
  objNameSet&apps = extGroupApp::get_all();
  extGroupApp* ap = (extGroupApp*) apps.lookup(name);
  if(!ap){
    genString new_name;
    bool changed_name = extern_canonic_name(name, new_name);
    if(changed_name)
      ap = (extGroupApp*) apps.lookup(new_name);
  }
  return ap;
}

symbolArr*
extGroupApp::find(char const*name)
{
  extGroupApp* ap = get(name);

  symbolArr*array = NULL;
  if(ap)
    array = ap->load();
  return array;
}


// required because function pointers don't adjust for covariant return types:
static externApp*
extGroupApp_create_adaptor(char const *c, externFile *extf)
{
  return extGroupApp::create(c, extf);
}

externInfo*
extGroupApp::type_info()
{
  static externInfo* inf = new externInfo(extGroupApp_create_adaptor, "grp", "group", "ext");
  return inf;
}

extGroupApp::extGroupApp(char const *name, char const *fname , extStatus st) :
	externApp(name, fname, st),
	arr(NULL)
{
  set_phys_name(fname);
  info()->get_apps()->insert(this);
}

externInfo*
extGroupApp::info() const
{
  return type_info();
}

extern char const* ATT_etag(symbolPtr&);

bool
extGroupApp::externSave()
{
  Initialize(extGroupApp::externSave);

  bool ret_value = false;
  char const* name = get_name();
  char const* fname = OSPATH(get_phys_name());

  if (fname && fname[0]){
      // If the specified group hasn't been "loaded," we crash:
      (void)find(name);
      Assert(arr);

      if(ext_stat==TEMP) {
          make_savable();
      }

      msg("saving group $1: $2\n", normal_sev) << name << eoarg << fname << eom;
      if (write_group(fname,arr) == 0) {
          set_status(SAVED);
          ret_value = true;
      }
  }
  else {
      msg("Invalid physical file name for group $1.", error_sev) << name << eom; 
  }

  return ret_value;
}

int write_group (char const* file_name, symbolArr* arr) 
// returns: 0 if successful
{
  ofstream os(file_name, ios::out);
  if(os.bad()){
    msg("Error while accessing directory:\n$1", error_sev) << file_name << eom;
    return -1;
  }
  int sz = arr->size();  
  for(int ii =0; ii<sz; ++ii){
    char const* etag = ATT_etag((*arr)[ii]);
    os << etag << '\n';
  }
  os << flush;
  return 0;
}

extGroupApp*
extGroupApp::create(char const* name, symbolArr*arr)
{
  extGroupApp* ap = extGroupApp::create(name, (externFile *)NULL);
  ap->arr = arr;
  ap->set_status(MODIFIED);

  return ap;
}

extGroupApp*
extGroupApp::create(char const* name, externFile *extf)
{
  char const *physname;
  genString pn;

  if (extf) {
    physname = extf->get_phys_filename();
  }
  else {
    externInfo* ti = type_info();
    ti->new_fname(name, pn);
    physname = pn.str(); 
  }

  extGroupApp * ap = new extGroupApp(name, physname);

  ap->make_savable(extf);
  return ap;
}

extGroupApp*
extGroupApp::create_tmp(char const* name, symbolArr*arr)
{
  genString physname;
  externInfo* ti = type_info();
  ti->new_fname(name, physname);
  
  extGroupApp * ap = new extGroupApp(name, physname, TEMP);

  ap->arr = arr;
  return ap;
}

extGroupApp* extGroupApp::current;

Interpreter*  GetActiveInterpreter();
int parse_selection(char const *buf, symbolArr& result);
int parse_selection_with_nulls(char const *buf, symbolArr& result);

symbolArr*
extGroupApp::load()
{
  if(arr) 
    return arr;

  char const*name = get_name();
  char const*fname = OSPATH(get_phys_name());
  
  if(!(fname && fname[0])) {
    msg("Invalid physical file name for group $1", error_sev) << name << eom; 
    return NULL;
  }

  msg("Loading group $1: $2...", normal_sev) << name << eoarg << fname << eom;
  arr = new symbolArr;

  int read_err = read_group(fname,arr);
  if (read_err) return NULL;

  msg(" done\n", normal_sev) << eom;
  return arr;
}

int read_group (char const* file_name, 
		symbolArr* arr, 
		char delim, 
		boolean skip_first_line,
		boolean include_null_symbols) 
// read_group reads in a file (specified by file_name) and parses
//  its contents to convert it to an array of discover entities.
//  If a delimiter is specified, all characters between the first
//  occurance of the delimiter on a line and the end of that line
//  are ignored.
//  The symbol array is filled with symbols in the same order as 
//  their names appear in the file.
//  If add_nulls is specified then a null symbol is added to the 
//  when a symbol in the file cannot be parsed.
//
// returns: 0 if successful
{
  struct stat st;
  stat(file_name, &st);

  FILE* fl = fopen(file_name, "r");

  if(!fl){
    msg("Failed to open $1", error_sev) << file_name << eom;
    return -1;
  }

  int  fsize = (int) st.st_size;
  char* buf = new char[fsize + 1];
  char* buf2 = NULL;
  int sz = fread(buf, 1, fsize, fl);
  buf[fsize] = '\0';

  char* text = buf;

  // ignore text after delimiter.
  if (delim != '\0') {
    char del[2];
    del[0] = delim;
    del[1] = '\0';
    buf2 = new char[fsize + 1];
    int count = 0;
    int count2 = 0;
    while (count < fsize) {
      int len = strcspn(buf+count,del);
      strncpy(buf2 + count2, buf + count, len);
      count += len;
      count2 += len;
      count += strcspn(buf+count,"\n");
    }
    buf2[count2] = '\0';
    text = buf2;
  }

  if (skip_first_line) {
    text += strcspn(text,"\n") + 1;
  }

  if (include_null_symbols) {
    parse_selection_with_nulls(text, *arr);
  } else {
    parse_selection(text, *arr);
  }
  
  fclose(fl);
  delete buf;
  if (buf2) delete buf2;
  return 0;
}

extern int (*groupFunct)(char const* name, symbolArr* arr);

int (*oldGroupFunct)(char* name, symbolArr* arr);

static int group_get_by_name(char const* name, symbolArr*ret)
{
  int retval = 1;
  symbolArr* arr = extGroupApp::find(name);
  if(arr){
    *ret = *arr;
    return 0;
  }
  if(oldGroupFunct)
    retval = (*oldGroupFunct)((char*)name, ret);
  return retval;
}

static groupType group_find(Interpreter* i, char const* name, symbolArr**ret)
{
  symbolArr* arr = extGroupApp::find(name);
  if(arr) {
    *ret = arr;
    return GR_NEW;
  }
  *ret = 0;
  return GR_NONE;
}

static bool rename_group(extGroupApp *app, char const *new_name)
{
  bool retval = false;
  if (app) {
    symbolArr* arr = app->load();
    externInfo* ti = app->type_info();

    genString new_phys_fname;		
    char const *old_phys_fname = app->get_phys_name();
    if (old_phys_fname) {
      genString extern_dir;
      ti->get_extdir_from_physname(app->get_name(), old_phys_fname, extern_dir);
      ti->fname(new_name, extern_dir.str(), new_phys_fname);
    }
    else {
      ti->new_fname(new_name, new_phys_fname);
    }

    symbolArr *new_arr = new symbolArr;
    externFile *extf = ti->new_extfile(new_name, new_phys_fname);
    extGroupApp* new_app = extGroupApp::create(new_name, extf);
    new_app->set_sym_arr(new_arr);
    *new_arr = *arr;

    new_app->externSave();
     
    app_delete(app);
    retval = true;
  }
  return retval;
}

void array_decompose(symbolArr&src, symbolArr&trg, symbolArr&in, symbolArr&out)
{
  in.removeAll();
  out.removeAll();
  int src_sz = src.size();
  int trg_sz = trg.size();
  if(trg_sz == 0) {
    out = src;
  } else if (src_sz > 0) {
    symbolSet set;
    set = trg;
    for(int ii=0; ii<src_sz; ++ii){
      symbolPtr& cur = src[ii];
      if(set.includes(cur))
	in.insert_last(cur);
      else
	out.insert_last(cur);
    }
  }
}

bool str2group(char const *selstr, char const *prefix, int &counter, genString &gname)
{
  bool ret = false;

  Interpreter *i = GetActiveInterpreter();
  symbolArr selarr;
  if (i && (i->ParseArgString(selstr, selarr) == 0)) {
    groupType gtype;
    do {
      gname.printf("%s%d", prefix, counter++);
      genString proper;
      if (extern_canonic_name(gname, proper))
        gname = proper;
      symbolArr *arr = NULL;
      gtype = group_find(i, gname, &arr);
    } while ((gtype != GR_NONE) && (counter > 0));

    if (counter > 0) {
      symbolArr *arr = new symbolArr;
      extGroupApp *app = extGroupApp::create(gname, arr);
      *arr = selarr;
      app->set_status(externApp::NEW);
      if (app->externSave())
        ret = true;
    }
  }

  return ret;
}

void attribute_info_list(Interpreter*i, objNameSet& list);

int group_infoCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;
  if(argc == 1) {
    genString list;
    attribute_info_list(i, extGroupApp::get_all());
    return TCL_OK;
  } else {
    return TCL_ERROR;
  }
}

int group_existsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * name = argv[1];
  if(argc > 3){
    Tcl_AppendResult(i->interp, "group exists ", name,  " ", argv[3], " no extra args allowed", NULL);
    return TCL_ERROR;
  }

  groupType tp;
  extGroupApp*ap = extGroupApp::get(name);
  if(ap) {
    tp = GR_NEW;
  } else {
    tp = GR_NONE;
  }
  genString ret;
  ret.printf("%d", (int) tp);
  Tcl_AppendResult(i->interp, (char*)ret, NULL);
  return TCL_OK;
}

int group_sizeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * name = argv[1];

  if(argc > 3){
    Tcl_AppendResult(i->interp, "group size ", name, " ", argv[3], " no extra args allowed", NULL);
    return TCL_ERROR;
  }
  symbolArr* arr = extGroupApp::find(name);

  if(!arr) {
     Tcl_AppendResult(i->interp, "group ", name, " does not exist", NULL);
     return TCL_ERROR;
  }
  int sz = arr->size();
  char buf[12];
  OSapi_sprintf(buf, "%d", sz);
  Tcl_AppendResult(i->interp, buf, NULL);
  return TCL_OK;
}

int group_getCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * name = argv[1];

  if(argc > 3){
    Tcl_AppendResult(i->interp, "group get ", name, " ", argv[3], " no extra args allowed", NULL);
    return TCL_ERROR;
  }
  symbolArr* arr = NULL;
  groupType tp = group_find(i, name, &arr);

  if(tp == GR_NONE) {
     Tcl_AppendResult(i->interp, "group ", name, " does not exist", NULL);
     return TCL_ERROR;
  }

  DI_object target;
  DI_object_create(&target);
  symbolArr * trg_arr =  get_DI_symarr(target);
  group_get_by_name(name, trg_arr);
  i->SetDIResult(target); 
  return TCL_OK;
}

int group_saveCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;
  char const * name = argv[1];

  if(argc > 3){
    Tcl_AppendResult(i->interp, "group save ", name, " ", argv[3], " no extra args allowed", NULL);
     return TCL_ERROR;
  }    

  extGroupApp* app = extGroupApp::get(name);

  if(!app){
    Tcl_AppendResult(i->interp, "group ", name, " does not exist", NULL);
    return TCL_ERROR;
  }
  app->externSave();
  return TCL_OK;
} 

int group_deleteCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;
  char const * name = argv[1];

  if(argc > 3){
    Tcl_AppendResult(i->interp, "group delete ", name, " ", argv[3], " no extra args allowed", NULL);
     return TCL_ERROR;
  }    

  extGroupApp* app = extGroupApp::get(name);

  if(!app){
    Tcl_AppendResult(i->interp, "group ", name, " does not exist", NULL);
    return TCL_ERROR;
  }

  app_delete(app);

  if(gmgr_refresh_hook)
    (*gmgr_refresh_hook)();

  return TCL_OK;
} 
static void group_copy_array(const symbolArr& src, symbolArr& trg)
{
  int sz = src.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = src[ii];
    if(sym.relationalp()){
      Relational * obj = sym;
      if(is_smtTree(obj) || is_ddSymbol(obj)){
	symbolPtr xsym = sym.get_xrefSymbol();
	if(xsym.isnotnull())
	  trg.insert_last(xsym);
	continue;
      }
    } 

    trg.insert_last(sym);
  }
}

int group_realnameCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;

  char const *name = argv[1];
  genString new_name;
  bool changed = extern_canonic_name(name, new_name);
  if (changed)
    name = new_name;
  Tcl_SetResult(i->interp, (char*)name, TCL_VOLATILE);
  ret = TCL_OK; 

  return ret;
}

int group_newCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * name = argv[1];
  genString new_name;
  bool changed = extern_canonic_name(name, new_name);
  if(changed){
    name = new_name;
  }
  symbolArr* arr = NULL;
  groupType tp = group_find(i, name, &arr);

  if(tp != GR_NONE) {
    Tcl_AppendResult(i->interp, "group ", name, " already  exists", NULL);
    return TCL_ERROR;
  }
   
  symbolArr res;
  if(argc > 2 && i->ParseArguments(2, argc, argv, res) != 0)
    return TCL_ERROR;


  arr = new symbolArr;
  extGroupApp* app = extGroupApp::create(name, arr);

  group_copy_array(res, *arr);
//  *arr = res;

  app->set_status(externApp::NEW);

  if(gmgr_refresh_hook)
    (*gmgr_refresh_hook)();

  Tcl_SetResult(i->interp, (char*)name, TCL_VOLATILE);
  return TCL_OK;

}

int group_tmpCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * name = argv[1];
  genString new_name;
  bool changed = extern_canonic_name(name, new_name);
  if(changed){
    name = new_name;
  }

  symbolArr* arr = NULL;
  groupType tp = group_find(i, name, &arr);

  if(tp != GR_NONE) {
    Tcl_AppendResult(i->interp, "group ", name, " already  exists", NULL);
    return TCL_ERROR;
  }
   
  symbolArr res;
  if(argc > 2 && i->ParseArguments(2, argc, argv, res) != 0)
    return TCL_ERROR;

  arr = new symbolArr;
  extGroupApp* app = extGroupApp::create_tmp(name, arr);

  group_copy_array(res, *arr);
//  *arr = res;

  app->set_status(externApp::TEMP);

  if(gmgr_refresh_hook)
    (*gmgr_refresh_hook)();

  Tcl_SetResult(i->interp, (char*)name, TCL_VOLATILE);
  return TCL_OK;
}

int group_saveasCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const * oname = argv[1];

  if(argc != 3) {
    Tcl_AppendResult(i->interp, "group saveas", oname, " needs new name", NULL);
    return TCL_ERROR;
  }
   
  char const * name = argv[2];

  symbolArr* arr = NULL;
  groupType tp = group_find(i, name, &arr);

  if(tp != GR_NONE) {
    Tcl_AppendResult(i->interp, "group ", name, " already  exists", NULL);
    return TCL_ERROR;
  }
   
  symbolArr* res = NULL;
  groupType otp = group_find(i, oname, &res);

  if(otp == GR_NONE) {
     Tcl_AppendResult(i->interp, "group ", oname, " does not exist", NULL);
     return TCL_ERROR;
  }
  arr = new symbolArr;
  extGroupApp* app = extGroupApp::create(name, arr);
  *arr = *res;
  app->externSave();

  if(gmgr_refresh_hook)
    (*gmgr_refresh_hook)();

  return TCL_OK;
}

int group_renameCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  char const* oname = argv[1];
  if(argc != 3) {
    Tcl_AppendResult(i->interp, "group rename ", oname, " needs new name", NULL);
    return TCL_ERROR;
  }
  char const* name = argv[2];
  extGroupApp* app = extGroupApp::get(oname);
  extGroupApp* new_app = extGroupApp::get(name);

  if(new_app) {
    Tcl_AppendResult(i->interp, "group rename ", name, " already exists", NULL);
    return TCL_ERROR;
  }

  if (!app) {
    Tcl_AppendResult(i->interp, "group rename ", oname, " does not exist", NULL);
    return TCL_ERROR;
  }

  int retval = TCL_OK;
  if(app->get_status() == externApp::TEMP){
    new_app =  app->rename(name);
    if( !new_app )
      retval = TCL_ERROR;
  } else {
    retval = (rename_group(app, name)) ? TCL_OK : TCL_ERROR;
  }

  if((retval == TCL_OK) && gmgr_refresh_hook)
    (*gmgr_refresh_hook)();

  return retval;
}


#define if_equal(nm) if(OSapi_strcmp((char*)cmd,quote(nm))==0)
#define subcommand(nm) if_equal(nm) return paste3(group_,nm,Cmd)(cd,interp,argc-1,argv+1)

int groupCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  Interpreter *i     = (Interpreter *)cd;

  externInfo::load();

  if(argc<2) {
    Tcl_AppendResult(i->interp, "group commands:\n  info kind new tmp save saveas rename set get size exists\n  add unique append intersect subtract", NULL);
    return TCL_OK;
  } 
  char const* cmd = argv[1];

  subcommand(info);

  if(argc == 2) {
    Tcl_AppendResult(i->interp, cmd , ": group name required", NULL);
    return TCL_ERROR;
  }

  char const* name = argv[2];

  subcommand(save);
  subcommand(get);
  subcommand(size);
  subcommand(exists);
  subcommand(new);
  subcommand(tmp);
  subcommand(rename);
  subcommand(saveas);
  subcommand(delete);
  subcommand(realname);

  extGroupApp* app = extGroupApp::get(name);

  symbolArr* arr = NULL;
  groupType tp = group_find(i, name, &arr);

  if(tp == GR_NONE) {
     Tcl_AppendResult(i->interp, "group ", name, " does not exist", NULL);
     return TCL_ERROR;
  }

  symbolArr res;

  if(i->ParseArguments(3, argc, argv, res) != 0)
      return TCL_ERROR;
  
  if_equal(set) {
    if(!arr){
      Tcl_AppendResult(i->interp, "group ", name, " is not settable", NULL);
      return TCL_ERROR;
    }
    arr->removeAll();
    group_copy_array(res, *arr);
//  *arr = res;

    if(app) app->set_status(externApp::MODIFIED);
  } else if_equal(unique) {
    if(argc > 3){
     Tcl_AppendResult(i->interp, "group unique ", name, argv[3], " no extra args allowed", NULL);
     return TCL_ERROR;
    }
    arr->unsort();
    if(app) app->set_status(externApp::MODIFIED);
  } else if_equal(append) {
    arr->insert_last(res);
    if(app) app->set_status(externApp::MODIFIED);
  } else if_equal(add) {
    arr->insert_last(res);
    arr->unsort();      
    if(app) app->set_status(externApp::MODIFIED);
  } else if_equal(subtract) {
    symbolArr in, out;
    array_decompose(*arr, res, in, out);
    *arr = out;
    if(app) app->set_status(externApp::MODIFIED);
  } else if_equal(intersect) {
    symbolArr in, out;
    array_decompose(*arr, res, in, out);
    *arr = in;
    if(app) app->set_status(externApp::MODIFIED);
  } else  if_equal(kind) {
	  if(app) {
		  if(app->get_symbolPtr_kind()==DD_EXTGROUP_TMP) 
			  Tcl_AppendResult(i->interp, "temporary",NULL);
		  else
			  Tcl_AppendResult(i->interp, "persistant",NULL);
          return TCL_OK;
	  } else {
		  Tcl_AppendResult(i->interp, "unknown group",NULL);
		  return TCL_ERROR;
	  }
  } else  { 
    Tcl_AppendResult(i->interp, "group ", cmd, ": wrong cmd", NULL);
    return TCL_ERROR;
  }

  Tcl_AppendResult(i->interp, name, NULL);
  return TCL_OK;
}  

static int parse_selectionCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc != 2){
    return TCL_ERROR;
  }
  DI_object target;
  DI_object_create(&target);
  symbolArr * trg_arr =  get_DI_symarr(target);
  parse_selection(argv[1], *trg_arr);
  i->SetDIResult(target); 
  return TCL_OK;
}
int api_print_attribute(symbolArr& arr, char const*exp, ostream& os);
static int print_attributeCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc < 2){
    return TCL_ERROR;
  }
  char const* exp = argv[1];
  symbolArr arr;
  if(i->ParseArguments(2, argc, argv, arr) != 0)
    return TCL_ERROR;
  ostream& os = i->GetOutputStream();
  int status = api_print_attribute(arr, exp, os);
  return status ? TCL_ERROR : TCL_OK;
}

static int print_fileCmd(Interpreter *i, int argc, char const *argv[])
{
  if(argc != 2){
    return TCL_ERROR;
  }
  char const* fn = argv[1];
  ostream& os = i->GetOutputStream();
  FILE* f = fopen(fn, "r");
  if(!f){
    os << "file " << fn << " does not exist" << endl;
    return TCL_ERROR;
  }
  int ch;
  while((ch = getc(f)) != EOF)
    os << (char)ch;

  os << flush;
  fclose(f);
  return TCL_OK;
}
projModule * symbol_get_module( symbolPtr& sym);
int db_read_src_from_pset (char const *pset_fname, genString&res);
static int print_psetCmd(Interpreter *i, int argc, char const *argv[])
{
  ostream& os = i->GetOutputStream();
  symbolArr res;
  if(i->ParseArguments(1, argc, argv, res) != 0)
    return TCL_ERROR;
  if(res.size() != 1)
    return TCL_ERROR;
  projModule*mod = symbol_get_module(res[0]);
  if(!mod)
    return TCL_ERROR;
  char const* pset = mod->paraset_file_name();
  if(!pset)
    return TCL_ERROR;
  genString buf;
  int sz =   db_read_src_from_pset(pset, buf);
  if(sz < 0)
    return TCL_ERROR;
  os << (char*) buf << flush;
  return TCL_OK;
}

// This differs from the previous command in that
// you can specify the actual physical path to the
// pset file you're interested in:

static int print_pset_fileCmd(Interpreter *shell, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  if (argc == 2) {
    char const *file = argv[1];
    genString buf = "";
    int sz = db_read_src_from_pset(file, buf);
    if (sz >= 0) {
      ostream &os = shell->GetOutputStream();
      os << (char *)buf << flush;
      ret = TCL_OK;
    } else {
      genString err;
      err.printf("%s: failed to read pset file: \"%s\"", argv[0], file);
    }
  } else {
    genString usage;
    usage.printf("%s: usage: <psetfile>", argv[0]);
    Tcl_SetResult(shell->interp, (char *)usage, TCL_VOLATILE);
  }

  return ret;
}

static int init()
{
  groupFunct = group_get_by_name;

  externInfo::add(extGroupApp::type_info());

  new cliCommandInfo("group", groupCmd);
  new cliCommand("print_attribute", print_attributeCmd);
  new cliCommand("print_file", print_fileCmd);
  new cliCommand("print_pset", print_psetCmd);
  new cliCommand("print_pset_file", print_pset_fileCmd);
  new cliCommand("parse_selection", parse_selectionCmd);

  return 0;
}
static int init_it = init();

//
// Big hack to get groups to behave like more familiar
// data types. Gala/pset_server do not understand these
// external apps; the very idea of processing anything
// besides a symbolPtr is abhorrent to our client/server
// design.
//
// Either groups need to start behaving like every other
// entity type in the model (use traditional data structures
// to represent groups) or make our client/server
// apparatus more flexible.
//
// [05/28/97, abover]
//

void
extGroupApp::get_all(symbolArr& arr)
{
  objNameSet& list = extGroupApp::get_all();
  Iterator it(list);
  Object *obj;
  while (obj = it.operator++()) {
    extGroupApp *app = (extGroupApp *)obj;
    arr.insert_last(app);
  }
}

int
extGroupApp::is_symbolPtr_def() const { return 1; }

ddKind
extGroupApp::get_symbolPtr_kind() const
{ 
  if (ext_stat==TEMP)
    return DD_EXTGROUP_TMP;
  else
    return DD_EXTGROUP;
}

