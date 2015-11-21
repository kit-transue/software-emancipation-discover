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
#include "QueryApp.h"
#include "QueryTree.h"
#include "SubChk.h"
#include <TclList.h>
#include "machdep.h"
#include "cLibraryFunctions.h"
#include "msg.h"

init_relational(QModule, externApp);
init_rel_or_ptr(QModule, module, 0, QTree, tree, 0);

const char *QueryApp::appNames[] = {
  "subchk"
};

QueryApp::AppCode QueryApp::NameToCode(const char *name)
{
  AppCode code = INVALID;

  if (name) {
    int sz = sizeof(appNames) / sizeof(const char *);
    for (int i = 0; i < sz && code == INVALID; i++) {
      if (OSapi_strcmp((char *)name, (char *)appNames[i]) == 0) {
        code = (AppCode)i;
      }
    }
  }

  return code;
}

const char *QueryApp::CodeToName(AppCode code)
{
  const char *name = NULL;

  if (code != INVALID) {
    name = appNames[code];
  }

  return name;
}

// Node factory -- used by the parser and the client app
// (this intermediate QueryApp stuff may disappear in
// favor of something more direct, but we will still
// need creation routines):

QTree *QueryApp::CreateNode(int treecode, int appcode)
{
  QTree *node = NULL;

  switch (appcode)
  {
    case SUBCHK: switch (treecode)
                 {
                   case QTree::ROOT:   node = new SC_RootNode;   break;
		   case QTree::QUERY:  node = new SC_QueryNode;  break;
                   /* ok */
		 }
    /* ok */
  }

  if (!node)
  {
    switch (treecode)
    {
      case QTree::ROOT:   node = new RootNode;      break;
      case QTree::FOLDER: node = new FolderNode;    break;
      case QTree::QUERY:  node = new QueryNode;     break;
      default:            node = new QTree;         break;
    }
  }

  return node;
}

//
// QModule:
//

QueryApp::AppCode QModule::defaultAppCode = QueryApp::SUBCHK;

QModule::QModule(const char *name, const char *fname, extStatus status) :
  externApp(name, fname, status)
{
}

QModule::~QModule()
{
  module_rem_tree(this);
}

bool QModule::externSave()
{
  bool ret = false;

  QTree *tree = module_get_tree(this);

  if (tree) {
    const char *name = get_name();
    const char *fname = OSPATH(get_phys_name());
    msg("Saving queries: $1...", normal_sev) << name << eom;
    if (fname && *fname) {
      if (tree->Write(fname) == 0) {
        set_status(externApp::SAVED);
        ret = true;
      }
    } else {
      msg("Invalid physical file name for query $1.", error_sev) << name << eom;
    }
  }

  return ret;
}

QTree *QModule::Open()
{
  QTree *tree = NULL;

  tree = module_get_tree(this);
  if (!tree) {
    genString fname = OSPATH(get_phys_name());
    if (fname.length() > 0) {
      msg("Loading queries: $1...", normal_sev) << get_name() << eom;
      if (fileref.length() > 0) {
        struct OStype_stat st;
        if ((OSapi_stat((char *)fname, &st)) != 0 || (st.st_size <= 0)) {
          fname = fileref;
        }
      }
      tree = QTree::Read(fname);
      if (tree) {
        module_put_tree(this, tree);
      } 
    }
    else {
      msg("Invalid physical file name for query $1.", error_sev) << get_name() << eom;
    }
  }
  return tree;
}

bool QModule::Save()
{
  return externSave();
}

bool QModule::Close()
{
  QTree *tree = module_get_tree(this);
  if (tree) {
    module_put_tree(this, NULL);
    delete tree;
  }

  const char *fname = OSPATH(get_phys_name());  
  if (fname && *fname) {
    struct OStype_stat st;
    if ((OSapi_stat((char *)fname, &st) != 0) || (st.st_size <= 0)) {
      Remove(get_name());
    }
  }

  return true;
}

QModule *QModule::Create(const char *name, QTree *tree)
{
  QModule *module;

  RootNode *root = (RootNode *)tree->GetRoot();
  QueryApp::AppCode appcode = root->appcode;

  module = Find(name, appcode);

  if (!module) {
    genString basename;
    GetUnusedName(basename, name, appcode);
    switch (appcode) {
      case QueryApp::SUBCHK : module = (QModule *)SubChkMod::create(basename); break;
      default               : module = NULL;                                   break;
    }
  }

  if (module) {
    module_put_tree(module, tree);
  }

  return module;
}

void QModule::Remove(const char *name, const char *app)
{
  if (!app)
    app = QueryApp::CodeToName(defaultAppCode);

  QModule *module = Find(name, app);

  if (module) {
    externInfo *extinf;
    if (app && (extinf = (externInfo *)externInfo::find(app))) {
      extinf->get_apps()->remove(*module);
      extinf->delete_extfile(name);
      //delete module;
    }
  }
}

QModule *QModule::Find(const char *name, const char *app)
{
  QModule *module = NULL;

  if (!app)
    app = QueryApp::CodeToName(defaultAppCode);

  externInfo *extinf;
  if (app && (extinf = (externInfo *)externInfo::find(app))) {
    objNameSet *apps = extinf->get_apps();
    if (apps) {
      Iterator it(*apps);
      Object *ob;
      while (ob = it.operator++()) {
        QModule *m = (QModule *)ob;
        if (OSapi_strcasecmp((char *)name, (char *)m->get_name()) == 0) {
          module = m;
          break;
	}
      }
    }
  }

  return module;
}

QModule *QModule::Find(const char *name, QueryApp::AppCode app)
{
  return QModule::Find(name, QueryApp::CodeToName(app));
}

bool QModule::List(TclList &list, const char *app)
{
  bool ret = false;

  if (!app)
    app = QueryApp::CodeToName(defaultAppCode);

  externInfo *extinf;
  if (app && (extinf = (externInfo *)externInfo::find(app))) {
    objNameSet *apps = extinf->get_apps();
    if (apps) {
      Iterator it(*apps);
      Object *ob;
      while (ob = it.operator++()) {
        char const *name = ((objDictionary *)ob)->get_name();
        list += name;
      }
      ret = true;
    }
  }

  return ret;
}

void QModule::GetUnusedName(genString &name, const char *prefix, QueryApp::AppCode appcode)
{
  unsigned long counter = 1;

  if (!(prefix && *prefix))
    prefix = "Untitled";
  
  name = prefix;

  const char *app = QueryApp::CodeToName(appcode);

  while (Find(name, app)) {
    name.printf("%s%d", prefix, counter++);
  }
}

void QModule::LoadDefaults()
{
  const char *name = "BaseLibrary";
  if (SubChkMod::Find(name) == NULL) {
    const char *home = OSapi_getenv("PSETHOME");
    if (home) {
      SubChkMod *module = (SubChkMod *)SubChkMod::create(name);
      if (module) {
        module->fileref = home;
        module->fileref += "/lib/baselib.sc";
      }
    }
  }
  SubChkMod::InstallUserLibs();
}

