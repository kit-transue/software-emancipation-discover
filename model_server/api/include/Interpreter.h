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
#ifndef _Interpreter_H
#define _Interpreter_H

#include "tcl.h"
#include "genString.h"
#include "genArr.h"
#include "api_internal.h"
#include "objArr.h"
#include "attribute.h"
#include "format.h"

//Remove QAR_Driver related functions as QAEngine will replace QAR_Driver.
#if 0
void QAR_deleteSession(genString& QAR_session);
#endif //#if 0

class liteScope;
class Interpreter {
  public:
    static Interpreter *Create(void);
    void ExecuteQuery(class cliQueryData *q, symbolArr&selection);
    void AnswerQuery(class projectBrowser*, int );
    int  EvalCmd(char const *cmd);
    int  EvalQuery(char const *cmd, symbolArr&sel, symbolArr&res);
    int  EvalFile(char const *fname);
    int  GetResult(genString& str);
    int  PrintResult(void);
    void PrintPrompt(void);
    void DeleteDIObject(DI_object val);
    void SaveResult(void);
    void SetOutputStream(ostream& ostr);
    ostream& GetOutputStream();
    void SetPrompt(char const *p);
    int SourceMainScript();
    int SourceStandardScript(char const *);

    Tcl_Interp  *interp;

    virtual ~Interpreter();

  private:
    Interpreter();
 
  public:
    void      SetDIResult(DI_object res);
    void      SetResult(const char* res);
    void      SetResult(const symbolArr& res);

    void SetScope(liteScope*s) {scope = s;}
    liteScope* GetScope() {return scope;}
    DI_object DecodeDIResult(void);
    bool      GetSortFlag()         {return sort_flag;}
    void      SetSortFlag(bool f)   {sort_flag = f;}
    void      OverridePutsCommand();
    void      OverrideForEachCommand();
    void      OverrideLLengthCommand();
    void      OverrideLIndexCommand();
    void      SaveOldInfoCmd(void);
    void      InitBuiltinFunctions();
    void      InitAttributes();
    void      PrintSymbolArr(symbolArr& x, int print_indexes);
    int       ReferenceObject(DI_object val);
    int       DereferenceObject(DI_object val);
    int       GetObjectReferences(DI_object val);
    int       ParseArguments(int start_pos, int argc, char const *argv[], symbolArr& res);
    int       ParseArguments(int ignore, int start_pos, int argc, char const *argv[], symbolArr& res);
    int       ParseArgString(char const *str, symbolArr& res);
    int       SetupSelector(ddSelector& sel, int argc, char const *argv[]);
  private:
    int       PerformAttributeFilter(class symbolAttribute *filter_attribute, symbolArr& arr);
    int       PerformExpressionFilter(char const *filter, symbolArr& arr);
    int       WhereDefinedCmd(int argc, char const *argv[]);
    int       WhereReferencedCmd(int argc, char const *argv[]);
    int       WhereIncludedCmd(int argc, char const *argv[]);
    int       WhereUsedCmd(int argc, char const *argv[]);
    int       WhereDeclaredCmd(int argc, char const *argv[]);

  public:
    static void TCL_TraceProc(ClientData cd, Tcl_Interp *interp, int level, char const *command, int (*)(void *, Tcl_Interp*, int, char*[]), 
			      ClientData, int argc, char const *[]);
    static char const *TraceUnset(ClientData cd, Tcl_Interp *interp, char const *name1, char const *name2, int flags);
    static int  TraceCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  PutsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ForEachCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  LLengthCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
    static int  LIndexCmd(ClientData cd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
    static int  ApplyCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  AttributeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  AddQueryCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  AddActionCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  AddAttributeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetResultCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetElementsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetSelectionCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  MessageCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  PrintCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  OutCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  OutAppendCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  PrintFormatCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SortCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SelectionCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  EmacsCommandCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);

    static int  RootsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ParentCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SubprojectsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  FilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ContentsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  FilterCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  NameCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SizeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  DefinesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  UsesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  WhereCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetIncludeFilesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetMemberFunctionsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetMemberDataCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetTemplateCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetInstantiationsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetFriendsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetGeneratedCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetMacrosCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetSuperClassesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetSubClassesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetDeclaresCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetCopyCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetAddCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetUnionCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetSubtractCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetIntersectCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  SetNilCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ChangePromptCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ModifiedEntitiesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  InstancesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  astNodesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ValidateInstArrCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  DeclarationsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  WhereDeclaredCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetInstanceScopeCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  GetInstanceSymbolCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  InstanceFromOffsetCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  ArgumentOfCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  FriendOfCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);
    static int  PackagesCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[]);

    Tcl_CmdInfo  foreach_info;
    Tcl_CmdInfo  puts_info;
    Tcl_CmdInfo  llength_info;
    Tcl_CmdInfo  lindex_info;
    Tcl_CmdInfo  orig_info_cmd;
    int          client_number;
    int          code;
    genArr(int)  object_refs;
    objArr       object_pool;
    objArr       return_object_pool;
    bool         sort_flag;
    symbolFormat *current_print_format;
    ostream      *output_stream;
    const char   *prompt;
    genString    command;
    liteScope    *scope;
#if 0
    genString    QAR_session;
#endif //#if 0
};

extern Interpreter *interpreter_instance;

#define CHECK_ARG_NUM(name,num)

struct cliCommandBase : public objDictionary{  
  cliCommandBase(const char*name);
  virtual int eval(Interpreter*i, int argc, char const **argv) = 0;
};

struct cliCommandInfo : public cliCommandBase {
  typedef int (*CB)(ClientData,Tcl_Interp*,int,char const**);
  cliCommandInfo(const char* name, CB cb) : cliCommandBase(name), _cptr(cb) {}
  cliCommandInfo(char* name, CB cb); // temp to link quickly for testing
  virtual int eval(Interpreter*i, int argc, char const **argv);
  CB _cptr;
};

struct cliCommand : public cliCommandBase {
  typedef int (*CB)(Interpreter*,int,char const**);
  cliCommand(const char* name, CB cb) : cliCommandBase(name), _cptr(cb) {}
  virtual int eval(Interpreter*i, int argc, char const **argv);
  CB _cptr;
};

struct cliQueryCommand : public cliCommandBase {
  cliQueryCommand(const char* name) : cliCommandBase(name) {}
  virtual int query(ostream&os, const symbolArr& in, symbolArr& out) = 0;
  virtual int eval(Interpreter*i, int argc, char const **argv);
};


int cli_process_input(char const *data, ostream &out_stream, Interpreter *, int do_prompt_and_print);
extern int           cli_eval_string(char const *);
extern int           parse_menu_options(int, char**, int&, int&, 
					const char**, const char**, 
					const char**);
extern void          SetActiveInterpreter(Interpreter *);
extern Interpreter*  GetActiveInterpreter(void);
extern void          release_sigchild(void);
extern void          restore_sigchild(void);
extern int	     call_cli_callback(Interpreter *, char const *, 
				       symbolArr *, symbolArr *);
extern void cli_eval_file(char const *fname, ostream& out_stream, Interpreter *interpreter);
extern int	     call_cli_callback(Interpreter *, char const *, 
				       char const *, symbolArr *, symbolArr *);
extern int cli_set_global_var(Interpreter*i, const char* name, const char*val);
#define GET_INTERPRETER(interp) (Interpreter *)Tcl_GetAssocData(interp, (char const *)"Interpreter", NULL)

#endif
