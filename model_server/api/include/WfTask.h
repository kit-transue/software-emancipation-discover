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
#ifndef _WfTask_h
#define _WfTask_h

#include <AccessCatalog.h>
#include <genString.h>
#include <Interpreter.h>
#include <WfNode.h>
#include <externApp.h>

class WfTaskCatalog : public AccessCatalog
{
  public:
    virtual int AppendItemCmd(int, char const *[], genString &);
    virtual int RunCmd(int, char const *[], genString &);
    virtual int InsertCmd(int, char const *[], genString &);
    virtual int RemoveCmd(int, char const *[], genString &);
    virtual int WhereCmd(int, char const *[], genString &);
    virtual int PrintCmd(int, char const *[], genString &);
    virtual int SetParamCmd(int, char const *[], genString &);
    virtual int GetParamCmd(int, char const *[], genString &);
    virtual int SaveCmd(int, char const *[], genString &);
    virtual int DumpCmd(int, char const *[], genString &);
    virtual int MoveCmd(int, char const *[], genString &);
    virtual int ValidateCmd(int, char const *[], genString &);
    virtual int ReviveCmd(int, char const *[], genString &);
    virtual int ErrorStatusCmd(int, char const *[], genString &);

    virtual Relational *Find(char const *);    
};

class WfTaskApp : public externApp
{
  public:

    define_relational(WfTaskApp, externApp);

    virtual bool externSave();
    virtual externInfo *info() const;
    static externInfo *type_info();
    static externApp *create(char const *, externFile *fn = NULL);
  protected:

    WfTaskApp(char const *, char const *, extStatus = TEMP);

};

class WfTask : public objDictionary {

  public:

    WfTask(char const *, objArr &, bool = false, WfTaskApp * = NULL);
    ~WfTask();

    static bool Init();
    static int LoadMainScript();

    static WfTaskCatalog *catalog;

    int Run(Interpreter *);
    int Run(Interpreter *, int, int);
    int RunFrom(Interpreter *, int);
    int RunFor(Interpreter *, int);
    void AttachFrom(int);
    void AttachParams(int);
    WfParamInstance *MatchParam(WfParamInstance *, int);
    void Insert(int, objArr &);
    void Remove(int);
    int CurrentPosition();
    bool ValidNodeIndex(int);
    void Print(TclList &, genArrOf(int) &, WfPrintCode, bool = false);
    void Print(TclList &, WfPrintCode, bool = false);
    void GetNodeParam(int, char const *, objArr &);
    bool SetNodeParam(int, char const *, char const *);
    bool PrintNodeParam(int, char const *, TclList &);
    void Move(int, int);
    void Revive();
    bool Validate();
    int NormalizePos(int);
    void Consolidate(WfTaskApp *ap);
    void CycleVals(genArrCharPtr &, bool);
    void Nullify();
    void SearchAndReplace(WfValueType::CODE, char const *, genString &, objArr &);
    static bool Save(WfTaskApp *);
    static bool Load(WfTaskApp *);
    static bool LoadNodeDefs(objArr &, TclList &);
    static bool LoadParamDefs(objArr &, TclList &);
    void NotifyApp();
    void GetErrorString(genString &str);
    void GetErrorInfo(genString &str);

    define_relational(WfTask, objDictionary);
    virtual void print(ostream& = cout, int = 0) const;

  private:

    int       curnode;        // index of the current node instance
    objArr    ninst;          // node instances
    genString error_string;
    genString error_info;

    static char const *main_script;  // basename of workflow initialization script
};

defrel_one_to_one(WfTask,task,WfTaskApp,app);

#endif /* !_WfTask_h */
