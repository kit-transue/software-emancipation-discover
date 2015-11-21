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
#ifndef _QueryApp_h
#define _QueryApp_h

#include <externApp.h>
#include <Interpreter.h>

class QTree;
class QueryNode;
class TclList;

class QueryApp
{

  public:

    typedef enum {
      INVALID = -1,
      SUBCHK  =  0
    } AppCode;

  public:

    static QTree*            CreateNode(int, int);

    static AppCode           NameToCode(const char *);
    static const char*       CodeToName(AppCode);

    static const char*       appNames[];

};


class QModule : public externApp
{

  public:


                             QModule(const char *, const char *, extStatus = TEMP);
    virtual                  ~QModule();

    virtual bool             externSave();

    QTree*                   Open();
    bool                     Close();
    bool                     Save();

    genString                fileref;

    // Each of the following static routines has a matching
    // function that doesn't require the caller to specify
    // an application name or code. These convenience routines
    // operate on the default application which is defined
    // to be submission check.

    static QModule*          Create(const char *, QTree *);
    static void              Remove(const char *, const char * = NULL);
    static QModule*          Find(const char *, const char * = NULL);
    static QModule*          Find(const char *, QueryApp::AppCode);
    static bool              List(TclList &, const char * = NULL);
    static void              GetUnusedName(genString &, const char *, QueryApp::AppCode);
    static void              LoadDefaults();

    static QueryApp::AppCode defaultAppCode;

    define_relational(QModule, externApp);

};

#endif /* ! _QueryApp_h */
