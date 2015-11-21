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
#include "AccessCatalog.h"

AccessCatalog::Hook::Hook(HookCmd _cmd, HookMP _mp) : cmd(_cmd), mp(_mp) {}

bool AccessCatalog::InitHooks()
{
  AppendHook ("new",    &AccessCatalog::AppendItemCmd);
  AppendHook ("exists", &AccessCatalog::FindItemCmd  );
  AppendHook ("info",   &AccessCatalog::ListAllCmd   );
  AppendHook ("delete", &AccessCatalog::RemoveItemCmd);

  return true;
}

bool AccessCatalog::AppendHook(HookCmd cmd, HookMP mp)
{
  Hook *h = new Hook(cmd, mp);   // ignore purify error here
  hooks.append(h);

  return true;
}

int AccessCatalog::ProcessCommand(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  int ret = TCL_ERROR;

  Interpreter *i = (Interpreter *)cd;
  genString result = "";

  if (argc >= 2) {
    int sz = hooks.size();
    bool match = false;
    for (int ii = 0; ii < sz; ii++) {
      if (!strcmp(hooks[ii]->cmd, argv[1])) {
        HookMP mp = hooks[ii]->mp;
	interpreter = i;
        ret = (this->*mp)(argc, argv, result);
	interpreter = NULL;
        match = true;
        break;
      }
    }        
    if (!match)
      MainUsage(result, argv[0]);
  }
  else
    MainUsage(result, argv[0]);

  if (ret != TCL_OK)
    Tcl_AppendResult(i->interp, (char *)result, NULL);
  else
    Tcl_SetResult(i->interp, (char *)result, TCL_VOLATILE);

  return ret;
}

bool AccessCatalog::MainUsage(genString &usage, char const *topcmd)
{
  usage = "";

  int sz = hooks.size();
  if (sz > 0) {
    usage.printf("%s: commands: ", topcmd);
    for (int ii = 0; ii < sz; ii++) {
      usage += hooks[ii]->cmd;
      if (ii < sz - 1)
        usage += " ";
    }
  }

  return true;
}

int AccessCatalog::AppendItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    if (!Find(argv[2])) {
      objDictionary *od = new objDictionary(argv[2]);
      Append(od);
      ret = TCL_OK;
    }
    else
      result.printf("%s: error: name already in use: \"%s\".", argv[0], argv[2]);
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int AccessCatalog::FindItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    if (Find(argv[2]))
      result = "1";
    else
      result = "0";
    ret = TCL_OK;
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

int AccessCatalog::ListAllCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 2) {
    Tcl_DString list;
    Tcl_DStringInit(&list);
    Iterator it(set);
    Object *obj;
    while (obj = it.operator++()) {
      char const *name = ((objDictionary *)obj)->get_name();
      Tcl_DStringAppendElement(&list, name);
    }
    char const *s = Tcl_DStringValue(&list);
    if (s) result = s;
    Tcl_DStringFree(&list);
    ret = TCL_OK;
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "");

  return ret;
}

int AccessCatalog::RemoveItemCmd(int argc, char const *argv[], genString &result)
{
  int ret = TCL_ERROR;

  result = "";

  if (argc == 3) {
    if (!Remove(argv[2]))
      result.printf("%s: error: item not found: \"%s\".", argv[0], argv[2]);
    else
      ret = TCL_OK;
  }
  else
    result.printf("usage: %s %s %s", argv[0], argv[1], "tag");

  return ret;
}

bool AccessCatalog::Append(Relational *od)
{
  set.insert(od);
  return true;
}

Relational *AccessCatalog::Find(char const *tag)
{
  return (Relational *)(set.lookup(tag));
}

bool AccessCatalog::Remove(char const *tag)
{
  bool ret = false;

  Obj *obj = Find(tag);
  if (obj) {
    set.remove(*obj);
    delete obj;
    ret = true;
  }

  return ret;
}

bool AccessCatalog::FillArr(objArr &arr, char const *tag)
{
  bool ret = false;

  Obj *obj = Find(tag);
  if (obj) {
    arr.insert_last(obj);
    ret = true;
  }

  return ret;
}

bool AccessCatalog::FillArr(AccessCatalog *catalog, objArr &arr, char const *tag)
{
  bool ret = false;

  if (catalog)
    ret = catalog->FillArr(arr, tag);

  return ret;
}

int AccessCatalog::FillArr(AccessCatalog *catalog, objArr &arr, genArrCharPtr &tags, genArrCharPtr &bad, bool die)
{
  int ret = 0;

  int sz = tags.size();
  for (int ii = 0; ii < sz; ii++) {
    if (FillArr(catalog, arr, *(tags[ii]))) {
      ++ret;
    }
    else {
      bad.append(tags[ii]);
      if (die)
        break;
    }
  }

  return ret;
}
