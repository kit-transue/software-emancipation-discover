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
//
// FILE: GroupApp.C
//
// Class definitions for GroupApp. (Provides group management routines.)
//

#ifndef ISO_CPP_HEADERS
#include <limits.h>
#else /* ISO_CPP_HEADERS */
#include <limits>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "../include/GroupApp.h"
#include "../../DIS_main/interface.h"
#include <gRTListServer.h>
#include <externApp.h>
#include <scopeMgr.h>
#include <messages.h>
#include <tcl.h>
#include <Interpreter.h>
#include <cLibraryFunctions.h>

//
// Initialize static members:
//

char *GroupApp::apiSub[] = {
  "intersection",
  "union",
  "identity",
  "diff_ab",
  "diff_ba",
  "exists",
  "rename",
  "delete",
  "add nosel",
  "remove nosel",
  "assign nosel",
  "grabscr",
  "grabsav",
  "grabsub",
  "grabsubflat",
  "export",
  "save",
  "print"
};

char *GroupApp::apiFlags[] = {
  "-notmp",
  "-mktmp"
};

GroupApp::ActionArr *GroupApp::queries = NULL;
genString *GroupApp::queryList = NULL;
GroupApp::ActionArr *GroupApp::utils = NULL;
const char *GroupApp::apiCmd = "_smgr";
char *GroupApp::debugPref = "DIS_internal.Smgr_DebugLevel";
bool GroupApp::initFlag = false;
int GroupApp::debugLevel = -1;
bool GroupApp::makeTempFlag = false;
char *GroupApp::resultName = NULL;
char *GroupApp::resultPrefix = NULL;
int GroupApp::resultIndex = 0;

extern Interpreter *interpreter_instance;

extern "C" {
  void ArrToDobjName(const symbolArr&, genString&);
  void DobjNameToArr(const char *, symbolArr&);
}

extern int UnusedSubsysName(const char *);


//
// Initialize the group application:
//
GroupApp::GroupApp()
{
  Initialize(GroupApp::GroupApp);

  Init();
}

//
// De-allocate the group application:
//
GroupApp::~GroupApp()
{
  Initialize(GroupApp::~GroupApp);

  if (resultName)
    delete[] resultName;
  if (resultPrefix)
    delete[] resultPrefix;
  if (queries)
    delete queries;
  if (queryList)
    delete queryList;
  if (utils)
    delete utils;

  initFlag = false;
}

bool GroupApp::Init()
{
  Initialize(GroupApp::Init);

  if (!initFlag) {

    debugLevel = customize::getIntPref(debugPref);

    queries = new ActionArr;
    utils = new ActionArr;

    //
    // Specification of the valid group actions:
    //
    // The fields are as follows:
    //
    //     visible name
    //     api subcommand
    //     source selection code
    //     result selection code
    //     flags
    //
    // NOTE: Only queries have visible names. The other names are
    //       hardcoded in the menus data file.
    //
    // NOTE: Braindead HP compiler won't let us initialize this
    //       statically with an aggregate-list, so it's done here.
    //
    // NOTE: Order here matters! Queries correspond to the ask list
    //       in the group manager; Utils correspond to the UtilCode
    //       enum and are typically invoked from the menu.
    //

    //------------------------------- Queries ------------------------------>

    AppendQuery("Intersection", apiSub[INTERSECTION], TWO_OR_MORE, UNKNOWN,     SHOW_ANSWER|RETURNS_OBJ);
    AppendQuery("Union",        apiSub[UNION],        TWO_OR_MORE, UNKNOWN,     SHOW_ANSWER|RETURNS_OBJ);
    AppendQuery("Identity",     apiSub[IDENTITY],     UNARY,       UNKNOWN,     SHOW_ANSWER|RETURNS_OBJ);
    AppendQuery("A - B",        apiSub[DIFF_AB],      BINARY,      UNKNOWN,     SHOW_ANSWER|RETURNS_OBJ);
    AppendQuery("B - A",        apiSub[DIFF_BA],      BINARY,      UNKNOWN,     SHOW_ANSWER|RETURNS_OBJ);

    //------------------------------- Utils -------------------------------->

    AppendUtil(NULL,            apiSub[RENAME],       UNARY,       UNKNOWN,     SEND_RNAME|REVISE_RNAME);
    AppendUtil(NULL,            apiSub[DELETE],       ONE_OR_MORE, UNKNOWN,     0);
    AppendUtil(NULL,            apiSub[ADD],          ONE_OR_MORE, ONE_OR_MORE, 0);
    AppendUtil(NULL,            apiSub[REMOVE],       ONE_OR_MORE, ONE_OR_MORE, 0);
    AppendUtil(NULL,            apiSub[ASSIGN],       ONE_OR_MORE, ONE_OR_MORE, 0);
    AppendUtil(NULL,            apiSub[GRABSCR],      UNKNOWN,     ONE_OR_MORE, SEND_RNAME|REVISE_RNAME|ECHO_RNAME);
    AppendUtil(NULL,            apiSub[GRABSAV],      UNKNOWN,     ONE_OR_MORE, SEND_RNAME|REVISE_RNAME|ECHO_RNAME);
    AppendUtil(NULL,            apiSub[GRABSUB],      UNKNOWN,     ONE_OR_MORE, SEND_RNAME|REVISE_RNAME);
    AppendUtil(NULL,            apiSub[GRABSUB_FLAT],      UNKNOWN,     ONE_OR_MORE, SEND_RNAME|REVISE_RNAME);
    AppendUtil(NULL,            apiSub[EXPORT],       ONE_OR_MORE, UNKNOWN,     0);
    AppendUtil(NULL,            apiSub[SAVE],         ONE_OR_MORE, UNKNOWN,     0);
    AppendUtil(NULL,            apiSub[PRINT],        ONE_OR_MORE, UNKNOWN,     0);

    initFlag = true;
  }

  return initFlag;
}

//
// Sends the list of active groups to some rtl:
//
void GroupApp::Refresh(const int rtl_id)
{
  Initialize(GroupApp::Refresh);

  if (rtl_id > 0) {
    RTListServer *rtl = RTListServer::find(rtl_id);
    if (rtl) {
      symbolArr sel;
      rtl->getSelArr(sel);
      rtl->clear();
      symbolArr groups;
      GetGroups(groups);
      rtl->insert(groups);
      rtl->select(sel);
      if (debugLevel > 0)
	 OSapi_printf("GroupApp: Refreshed RTL: %d.\n", rtl_id);
    }
  }
}

//
// Returns a well-formed Tcl list (in string
// form) containing all of the valid queries:
//
char *GroupApp::GetQueryList()
{
  Initialize(GroupApp::GetQueryList);

  if (!queryList) {
    queryList = new genString;
    *queryList = "";
    unsigned int sz = queries->size();
    for (int i = 0; i < sz; i++) {
      *queryList += "{";
      *queryList += queries->Reference(i).name;
      *queryList += "} ";
    }
  }

  return (char *)(*queryList);
}

//
// Stuff an array of symbols with group objects:
//
void GroupApp::GetGroups(symbolArr& arr)
{
  Initialize(GroupApp::GetGroups);

  extGroupApp::get_all(arr);
}

//
// Turn a symbolArr into a Tcl list:
//
void GroupApp::SelList(const symbolArr& src, genString& list)
{
  Initialize(GroupApp::SelList);

  symbolPtr sym;
  ForEachS(sym, src) {
    list += "{";
    genString name;
    sym.get_name(name);
    list += (char *)name;
    list += "} ";
  }
}

//
// Send a command to the interpreter:
//
int GroupApp::SendCommand(const char *command)
{
  Initialize(GroupApp::SendCommand);

  char *result;
  return SendCommand(command, result);
}

//
// Send a command to the interpreter and store the result:
//
int GroupApp::SendCommand(const char *command, char *&result)
{
  Initialize(GroupApp::SendCommand);

  genString fullCommand;
  fullCommand.printf("%s {%s}", apiCmd, command);

  if (debugLevel > 0)
     OSapi_printf("GroupApp: Sending command: \"%s\".\n", (char *)fullCommand);

  call_cli_callback(NULL, (const char *)fullCommand, NULL, NULL);
  result = Tcl_GetStringResult(interpreter_instance->interp);
  if(interpreter_instance->code == TCL_ERROR)
    NotifyTclError();
  
//  int code = dis_access_eval((char *)fullCommand, result);
//  if (code == TCL_ERROR)
//    NotifyTclError();

  return interpreter_instance->code;
}

//
// Notify the user of an error encountered by the interpreter:
//
void GroupApp::NotifyTclError()
{
  Initialize(GroupApp::NotifyTclError);

  char *err = Tcl_GetVar(interpreter_instance->interp, "errorInfo", TCL_GLOBAL_ONLY);
  dis_message(NULL, MSG_ERROR, "M_SETS_TCLERR", err);
}

//
// Check to see if we have the right number of operands:
//
bool GroupApp::CheckSel(SelNumCode s, int n, bool explicit_msg)
{
  bool ret = false;

  char *err = NULL;

  switch (s)
  {
    case UNARY:         // One operand:

                        if (n != 1)
                          err = "M_SETS_UNARY_SEL";
                        else
                          ret = true;
                        break;

    case BINARY:        // Two operands:

                        if (n != 2)
	                  err = "M_SETS_BINARY_SEL";
			else
			  ret = true;
			break;

    case ONE_OR_MORE:   // One or more operands:

			if (n < 1)
			  err = "M_SETS_ONE_OR_MORE_SEL";
			else
			  ret = true;
			break;

    case TWO_OR_MORE:   // Two or more operands:

                        if (n < 2)
	                  err = "M_SETS_TWO_OR_MORE_SEL";
			else
			  ret = true;
			break;

    default:            break;
  }

  if (ret == false) {
    if (explicit_msg) {
      if (err)
        dis_message(NULL, MSG_ERROR, err);
    }
    else
      dis_message(NULL, MSG_ERROR, "M_SETS_BADSEL");
  }

  return ret;
}

//
// Perform a query:
//
bool GroupApp::PerformQuery(const unsigned int id, const symbolArr& src, symbolArr& res)
{
  Initialize(GroupApp::PerformQuery);

  bool ret = false;

  unsigned int sz = queries->size();

  if (id < sz) {
    Action *action = &(queries->Reference(id));
    ret = Perform(action, src, res);
  }

  return ret;
}

//
// Perform a utility function:
//
bool GroupApp::PerformUtil(const unsigned int id, const symbolArr& src, symbolArr& res)
{
  Initialize(GroupApp::PerformUtil);

  bool ret = false;

  unsigned int sz = utils->size();

  if (id < sz) {
    Action *action = &(utils->Reference(id));
    ret = Perform(action, src, res);
  }

  return ret;
}

//
// Perform an action:
//
bool GroupApp::Perform(const Action *action, const symbolArr& src, symbolArr& res)
{
  Initialize(GroupApp::Perform);

  bool ret = false;

  genString command;
  genString glist;
  genString objname;
  genString rname;
  DI_object dobj = NULL;

  int flags = action->flags;

  //
  // Form command:
  //

  // Subcommand:

  if (action->cmd) {
    command += action->cmd;
    command += " ";
  }

  // Side-effect:

  if (flags & RETURNS_OBJ) {
    if (GetMakeTempFlag()) {
      command += apiFlags[MKTMP];
      command += " ";
      if (!(flags & SEND_RNAME)) {
	rname = SnatchResultName(true);
	if (rname.not_null()) {
	  command += rname;
	  command += " ";
        }
	else {
	  return ret;
        }
      }
    }
    else {
      command += apiFlags[NOTMP];
      command += " ";
    }
  }

  // Result name:

  if (flags & SEND_RNAME) {
    rname = SnatchResultName(false);
    if (rname.not_null()) {
      command += rname;
      command += " ";
    }
    else {
      return ret;
    }
  }

  // Selection:

  SelNumCode sn = action->sn;
  SelNumCode rn = action->rn;

  bool explicit_msg = true;

  if ((sn != UNKNOWN) && (rn != UNKNOWN))
    explicit_msg = false;

  if (rn != UNKNOWN) {
    if (CheckSel(rn, res.size(), explicit_msg)) {
	DI_object_create(&dobj);
	symbolArr *symarr = get_DI_symarr(dobj);
	symarr->insert_last(res);
	objname.printf("_DI_%d", GET_DI_SET_INDEX(dobj)); // Not quite understand this whole function. This is needs testing!
	command += objname;
	command += " ";
    }
    else
      return ret;
  }

  if (sn != UNKNOWN) {
    if (CheckSel(sn, src.size(), explicit_msg)) {
      SelList(src, glist);
      command += glist;
      command += " ";
    }
    else
      return ret;
  }

  //
  // Send command:
  //

  if (command.not_null()) {

    char *result;

    int code = SendCommand((char *)command, result);

    if (code != TCL_ERROR) {

      // Display:

      if (flags & SHOW_ANSWER) {
        if (flags & RETURNS_OBJ) {
          symbolArr arr(0);
          DobjNameToArr(result, arr);
	  res = arr;
	}
      }
      else if (flags & SHOW_TARGET) {
	symbolArr nullArr(0);
	res = nullArr;
	ExpandGroup(src, res);
      }
      else if (flags & SHOW_RNAME) {
	symbolArr nullArr(0);
	res = nullArr;
        ExpandGroup(GetResultName(), res);
      }

      // Aftermath:

      if ((flags & REVISE_RNAME) || (GetMakeTempFlag())) {
        SetResultName(DefaultResultName());
      }

      if (flags & ECHO_RNAME) {
        char *rn = (char *)rname;
        dis_message(NULL, MSG_DIAG, "M_SETS_USINGNAME", rn ? rn : "-");
      }

      ret = true;
    }
  }

  if (dobj != NULL)
      interpreter_instance->DeleteDIObject(dobj);

  return ret;
}

//
// Fill the specified symbolArr with the members of the specified group:
//
void GroupApp::ExpandGroup(const char *name, symbolArr& res)
{
  Initialize(GroupApp::ExpandGroup);

  symbolArr *arr = extGroupApp::find(name);

  if (arr)
    res.insert_last(*arr);
}

//
// Fill a symbolArr with one or more groups:
//
void GroupApp::ExpandGroup(const symbolArr& arr, symbolArr& res)
{
  symbolPtr group;
  ForEachS(group, arr) {
    genString name;
    group.get_name(name);
    ExpandGroup((char *)name, res);
  }
}

//
// Get the value of the temporary group creation flag:
//
int GroupApp::GetMakeTempFlag()
{
  Initialize(GroupApp::GetMakeTempFlag);

  int ret = (int)makeTempFlag;

  return ret;
}

void GroupApp::SetMakeTempFlag(int flag)
{
  Initialize(GroupApp::SetMakeTempFlag);

  if (flag == 0)
    makeTempFlag = false;
  else
    makeTempFlag = true;
}

//
// Get the current name, and check its validity:
//
char *GroupApp::SnatchResultName(bool search)
{
  Initialize(GroupApp::SnatchResultName);

  char *ret = NULL;

  char *name = GetResultName();

  switch (ValidName(name))
  {
    case NAME_VALID:   ret = name;
		       break;

    case NAME_INVALID: if (search) {
			 SetResultName(DefaultResultName());
			 ret = SnatchResultName(false);      // Recurse
		       }
		       else {
	                 dis_message(NULL, MSG_ERROR, "M_SETS_BADNAME");
			 SetResultName(DefaultResultName());
		       }
		       break;

    default:           SetResultName(DefaultResultName());
		       break;
  }

  return ret;
}

//
// Check to see if the specified name is already taken:
//
GroupApp::NameCode GroupApp::ValidName(const char *name)
{
  Initialize(GroupApp::ValidName);

  NameCode ret = NAME_INVALID;

  if (name) {
    if (*name) {
      if (ValidChars(name)) {
        if (UnusedSubsysName(name)) {
          genString command;
          command.printf("%s %s", apiSub[EXISTS], (char *)name);

          char *result = NULL;

          int code = SendCommand((char *)command, result);

          if (code != TCL_ERROR) {
            if (result)
              if (!strcmp(result, "0"))          // No such group
                ret = NAME_VALID;
          }
          else {
            dis_message(NULL, MSG_ERROR, "M_SETS_GETNAME_FATAL");
	    ret = NAME_FATAL;
          }
        }
      }
      else
        ret = NAME_BADCHAR;
    }
  }

  return ret;
}

//
// Check to see if the specified character is legal:
//
bool GroupApp::ValidChar(const char c)
{
  Initialize(GroupApp::ValidChar);

  bool ret = false;

  if (isalnum(c))
    ret = true;
  else {
    static char *vc = "-_=+,";
    static int len = OSapi_strlen(vc);
    for (int i = 0; i < len; i++) {
      if (c == vc[i]) {
        ret = true;
	break;
      }
    }
  }

  return ret;
}

//
// 
//
bool GroupApp::ValidChars(const char *name, bool verbose)
{
  Initialize(GroupApp::ValidChars);

  bool ret = false;

  if (name) {
    const char *p = name;
    bool valid = true;
    for (; *p && valid; p++) {
      if (!ValidChar(*p))
	valid = false;
    }
    if (valid)
      ret = true;
    else
      dis_message(NULL, MSG_ERROR, "M_SETS_BADCHAR");
  }

  return ret;
}

//
// Get the result prefix:
//
char *GroupApp::GetResultPrefix()
{
  Initialize(GroupApp::GetResultPrefix);

  char *ret = "";

  static char *defaultPrefix = "result_";

  if (!resultPrefix)
    SetResultPrefix(defaultPrefix);

  if (resultPrefix)
    ret = resultPrefix;

  return ret;
}

//
// Set the result prefix:
//
bool GroupApp::SetResultPrefix(const char *prefix)
{
  Initialize(GroupApp::SetResultPrefix);

  bool ret = false;

  if (prefix) {

    bool reset = false;

    if (resultPrefix) {
      if (strcmp(resultPrefix, prefix)) {
        delete[] resultPrefix;
        resultPrefix = NULL;
	reset = true;
      }
    }
    else
      reset = true;

    if (reset) {
      if (ValidChars(prefix)) {
        resultPrefix = Strdup(prefix);
	resultIndex = 1;
	SetResultName(DefaultResultName());
	ret = true;
      }
    }
    else
      ret = true;
  }

  return ret;
}

//
// Get the result name:
//
char *GroupApp::GetResultName()
{
  Initialize(GroupApp::GetResultName);

  char *ret = "";

  if (!resultName) {
    char *name = DefaultResultName();
    SetResultName(name);
    delete name; 
  } 

  if (resultName)
    ret = resultName;

  return ret;
}

//
// Set the result name:
//
void GroupApp::SetResultName(const char *name)
{
  Initialize(GroupApp::SetResultName);

  if (name) {
    if (resultName) {
      delete[] resultName;
      resultName = NULL;
    }
    resultName = Strdup(name);
  }  
}

//
// Conjure up an available group name:
//
char *GroupApp::DefaultResultName(const char *prefix, int &index)
{
  Initialize(GroupApp::DefaultResultName);

  char *ret = NULL;

  static int max = INT_MAX - 1;

  bool valid = false;
  genString name;
  NameCode nc;

  while ((index < max) && (valid == false)) {
    name.printf("%s%d", prefix, index);
    nc = ValidName((char *)name);
    if (nc == NAME_VALID)
      valid = true;
    else if (nc == NAME_INVALID)
      ++index;
    else
      break;
  }

  if (valid == true)
    ret = Strdup((char *)name);
  else if (index >= max)
    dis_message(NULL, MSG_ERROR, "M_SETS_MAXIDX");

  return ret;
}

//
// Wrapper function:
//
char *GroupApp::DefaultResultName()
{
  Initialize(GroupApp::DefaultResultName);

  char *ret = DefaultResultName(GetResultPrefix(), resultIndex);

  return ret;
}

//
// Generic allocation function:
//
char *GroupApp::Strdup(const char *str)
{
  Initialize(GroupApp::Strdup);

  char *ret = NULL;

  if (str) {
    char *target = new char[strlen(str) + 1];
    int i;
    for (i = 0; str[i]; i++)
      target[i] = str[i];
    target[i] = '\0';
    ret = target;
  }

  return ret;
}

//
// Add a query:
//
void GroupApp::AppendQuery(char *name, char *cmd, SelNumCode sn, SelNumCode rn, unsigned int flags)
{
  Action action(name, cmd, sn, rn, flags);
  queries->Append(action);
}

//
// Add a utility:
//
void GroupApp::AppendUtil(char *name, char *cmd, SelNumCode sn, SelNumCode rn, unsigned int flags)
{
  Action action(name, cmd, sn, rn, flags);
  utils->Append(action);
}

//
// Action storage routines:
//

GroupApp::Action::Action(char *_name, char *_cmd, SelNumCode _sn, SelNumCode _rn, unsigned int _flags)
{
  name = _name;
  cmd = _cmd;
  sn = _sn;
  rn = _rn;
  flags = _flags;
}

GroupApp::Action::Action() {}
GroupApp::Action::~Action() {}

GroupApp::ActionArr::ActionArr() { sz = 0; arr = NULL; }
GroupApp::ActionArr::~ActionArr() { if (arr) delete[] arr; }

GroupApp::Action& GroupApp::ActionArr::operator[](int i) const { return Reference(i); }
GroupApp::Action& GroupApp::ActionArr::Reference(int i) const { return arr[i]; }
unsigned int GroupApp::ActionArr::size() const { return sz; }

bool GroupApp::ActionArr::Append(Action& item)
{
  Initialize(GroupApp::ActionArr);

  bool ret = false;

  Action *_arr = new Action[sz + 1];

  if (_arr) {
    if (arr) {
      for (int i = 0; i < sz; i++) {
        _arr[i] = arr[i];
      }
      delete[] arr;
    }
    _arr[sz] = item;
    arr = _arr;
    
    ++sz;

    ret = true;
  }

  return ret;
}
