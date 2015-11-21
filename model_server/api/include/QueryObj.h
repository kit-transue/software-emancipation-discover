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
#ifndef _QueryObj_h
#define _QueryObj_h

#include <scopeMgr.h>
#include <Interpreter.h>
#include <TclList.h>
#include <GenericTree.h>
#include <Properties.h>

class intSet;

typedef int QueryObjId;

class QueryObj : public GenericTree {

  public:

    enum { INVALID_ID = 0 };

  public:

                             QueryObj();
                             QueryObj(const QueryObj &);
                             ~QueryObj();

    QueryObj &               operator=(const QueryObj &);

    virtual GenericTree *    CloneNode() const;

    int                      Valid() const;

    void                     GetName(genString &s) const { s = name; }
    void                     SetName(const char *cp) { name = cp; }

    int                      HasAttribute(char const *);
    int                      GetAttribute(char const *, genString &);
    void                     SetAttribute(char const *, char const *);
    void                     RemoveAttribute(char const *);
    Properties &             GetAttributes() { return attributes; }

    QueryObj *               AttachRoot();
    QueryObj *               Lookup(char const * const *);

    int                      Define(TclList &);
    int                      Dump(TclList &);
    int                      Print(genString &,  char const **, int, int);

    static QueryObj *        Create();
    static QueryObj *        RootLookup(char const **);
    static QueryObj *        IdToNode(QueryObjId);
    static QueryObjId        NodeToId(const QueryObj *);

    define_relational(QueryObj, Relational);

  protected:

    static QueryObjId        NextId();
    static int               Associate(QueryObj *);

    genString                name;
    Properties               attributes;

    static intSet            idents;
    static QueryObj *        root;

};

class QueryObjCmd {

  public:

    static int               Register();

    static int               Process(ClientData, Tcl_Interp *, int, char const **);

    static int               NewCmd      (Interpreter *, int, char const **, genString &);
    static int               DeleteCmd   (Interpreter *, int, char const **, genString &);
    static int               CopyCmd     (Interpreter *, int, char const **, genString &);
    static int               InsertCmd   (Interpreter *, int, char const **, genString &);
    static int               HasAttCmd   (Interpreter *, int, char const **, genString &);
    static int               GetAttCmd   (Interpreter *, int, char const **, genString &);
    static int               SetAttCmd   (Interpreter *, int, char const **, genString &);
    static int               DelAttCmd   (Interpreter *, int, char const **, genString &);
    static int               ListAttCmd  (Interpreter *, int, char const **, genString &);
    static int               DefineCmd   (Interpreter *, int, char const **, genString &);
    static int               DumpCmd     (Interpreter *, int, char const **, genString &);
    static int               PrintCmd    (Interpreter *, int, char const **, genString &);
    static int               GetNameCmd  (Interpreter *, int, char const **, genString &);
    static int               SetNameCmd  (Interpreter *, int, char const **, genString &);
    static int               MakeRootCmd (Interpreter *, int, char const **, genString &);
    static int               LookupCmd   (Interpreter *, int, char const **, genString &);
    static int               RelationCmd (Interpreter *, int, char const **, genString &);

};

class intObj : public Relational {

  public:

    intObj(int x) { val = x; }
    ~intObj() {}

    virtual unsigned hash() const
      { return (unsigned)val; }

    int value() { return val; }
    void value(int x) { val = x; }

    operator int() { return val; }

    define_relational(intObj, Relational);

  protected:

    int val;

};

class intSet : public Set {

  public:

    intSet(unsigned size = DEFAULT_CAPACITY) : Set(size) {}

    virtual bool isEqualObjects(const Object &a, const Object &b) const {
      return (((intObj *)&a)->value() == ((intObj *)&b)->value());
    }

};

defrel_one_to_one(intObj,id,QueryObj,node);

#endif /* ! _QueryObj_h */
