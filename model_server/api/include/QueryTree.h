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
#ifndef _QueryTree_h
#define _QueryTree_h

#include <GenericTree.h>
#include <DataExchange.h>
#include <QueryApp.h>
#include <genString.h>
#include <genArr.h>
#include <Interpreter.h>
#include <scopeMgr.h>

class QTreePtrArr;

class QTree : public GenericTree
{

  public:

    typedef enum {
      GENERIC = 0x01,
      ROOT    = 0x02,
      FOLDER  = 0x04,
      QUERY   = 0x08
    } TreeCode;

    typedef enum {
      DUMP_VERBOSE    = 0x01,
      DUMP_PERSISTENT = 0x02
    } DumpCode;

  public:

                             QTree();
                             QTree(const QTree &);
    virtual                  ~QTree();

    void                     operator=(const QTree &);   // does not return *this

    static QTree*            Create(TreeCode, QueryApp::AppCode);
    virtual GenericTree*     CloneNode() const;

    virtual QTree*           Find(int, int = 1) const;
    virtual int              Find(int, QTreePtrArr &, int = 1) const;
    virtual int              FindActiveQueries(QTreePtrArr &, int = -1) const;

    virtual int              Build();
    virtual void             Clean();
    virtual void             DeepClean();

    virtual QTree*           PerformInsert(QTree *, int = -1);

    virtual DataField*       FindField(char const *);
    virtual int              GetField(char const *, genString &);
    virtual int              SetField(char const *, char const *, int = 0);
    virtual void             GetAllFields(objArr &);

    virtual DataField*       RegisterField(char const *, DataField::DataFieldFunc,
                                           char const *, char const *, int = 0);
    virtual void             InstallFields();  // called once for each node type to define the fields
    virtual DataExchange*    GetExchange();    // get the exchange for this node type
    static objNameSet        exchangeSet;      // set of exchanges (one for each node type)

    int                      Dump(ostream &, int, int = 0);
    static QTree*            Load(istream &);
    int                      Write(char const *);
    static QTree*            Read(char const *);
    int                      Print(ostream &, int, char const **, int = 0);

    static int               SourceLib(Interpreter *);

    static void              TreeToId(QTree *, genString &);
    static QTree*            IdToTree(char const *);

    TreeCode                 treecode;

    static char const*       tags[];
    static const int         allows[];
    static char const*       dislib[];

};

typedef QTree* QTreePtr;
genArr(QTreePtr);

defrel_one_to_one(QModule, module, QTree, tree);

class RootNode : public QTree
{

  public:

                             RootNode();
                             RootNode(const RootNode &);
    virtual                  ~RootNode();

    void                     operator=(const RootNode &);

    virtual GenericTree*     CloneNode() const;

    virtual void             InstallFields();

    virtual int              Run(Interpreter *, QTree *);
    virtual int              RunNodes(Interpreter *, QTreePtrArr &);
    virtual void             SetDomain(symbolArr &);

    QueryApp::AppCode        appcode;

    symbolArr                domain;

  private:

    static void              DEX_Domain(int, genString &, void *);

};

class HeaderNode : public QTree
{

  public:

                             HeaderNode();
                             HeaderNode(const HeaderNode &);
    virtual                  ~HeaderNode();

    void                     operator=(const HeaderNode &);

    virtual void             InstallFields();

    int                      enabled;
    int                      visible;
    genString                name;
    genString                info;
    genString                tag;

  private:

    static void              DEX_Enabled(int, genString &, void *);
    static void              DEX_Visible(int, genString &, void *);
    static void              DEX_Name(int, genString &, void *);
    static void              DEX_Info(int, genString &, void *);
    static void              DEX_Tag(int, genString &, void *);

};

class FolderNode : public HeaderNode
{

  public:

                             FolderNode();
                             FolderNode(const FolderNode &);
    virtual                  ~FolderNode();

    void                     operator=(const FolderNode &);

    virtual GenericTree*     CloneNode() const;

};

class QueryNode : public HeaderNode
{

  public:

                             QueryNode();
                             QueryNode(const QueryNode &);
    virtual                  ~QueryNode();

    void                     operator=(const QueryNode &);

    virtual GenericTree*     CloneNode() const;

    virtual void             InstallFields();

    virtual void             Clean();

    genString                command;
    int                      exec;

  private:

    static void              DEX_Command(int, genString &, void *);
    static void              DEX_Exec(int, genString &, void *);

};

class QTreeCmd
{

  public:

    static int               Register();

    static int               Process(ClientData, Tcl_Interp *, int, char const **);

    static int               NewCmd                   (Interpreter *, int, char const **);
    static int               CopyCmd                  (Interpreter *, int, char const **);
    static int               DeleteCmd                (Interpreter *, int, char const **);
    static int               InsertCmd                (Interpreter *, int, char const **);

    static int               TypeCmd                  (Interpreter *, int, char const **);
    static int               FindCmd                  (Interpreter *, int, char const **);
    static int               GetRootCmd               (Interpreter *, int, char const **);

    static int               InspectCmd               (Interpreter *, int, char const **);
    static int               DumpCmd                  (Interpreter *, int, char const **);
    static int               LoadCmd                  (Interpreter *, int, char const **);
    static int               WriteCmd                 (Interpreter *, int, char const **);
    static int               ReadCmd                  (Interpreter *, int, char const **);
    static int               PrintCmd                 (Interpreter *, int, char const **);

    static int               GetFieldCmd              (Interpreter *, int, char const **);
    static int               SetFieldCmd              (Interpreter *, int, char const **);
    static int               GetFieldsCmd             (Interpreter *, int, char const **);
    static int               GetFieldTypeCmd          (Interpreter *, int, char const **);

    static int               BindCmd                  (Interpreter *, int, char const **);
    static int               CreateCmd                (Interpreter *, int, char const **);
    static int               OpenCmd                  (Interpreter *, int, char const **);
    static int               CloseCmd                 (Interpreter *, int, char const **);
    static int               SaveCmd                  (Interpreter *, int, char const **);
    static int               SaveAsCmd                (Interpreter *, int, char const **);
    static int               RemoveCmd                (Interpreter *, int, char const **);
    static int               ListCmd                  (Interpreter *, int, char const **);
    static int               ModuleOfCmd              (Interpreter *, int, char const **);

    static int               InitCmd                  (Interpreter *, int, char const **);
    static int               SourceCmd                (Interpreter *, int, char const **);
    static int               RunCmd                   (Interpreter *, int, char const **);
    static int               RunnersCmd               (Interpreter *, int, char const **);
    static int               RelationCmd              (Interpreter *, int, char const **);

};

#endif /* ! _QueryTree_h */
