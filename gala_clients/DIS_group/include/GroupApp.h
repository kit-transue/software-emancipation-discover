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
#ifndef _GroupApp_h
#define _GroupApp_h

//
// FILE: GroupApp.h
//
// Defines a class to manage group operations.
//

#include <symbolArr.h>

class GroupApp {

  public:

    GroupApp();
    ~GroupApp();

    typedef enum {
      INTERSECTION,
      UNION,
      IDENTITY,
      DIFF_AB,
      DIFF_BA,
      EXISTS,
      RENAME,
      DELETE,
      ADD,
      REMOVE,
      ASSIGN,
      GRABSCR,
      GRABSAV,
      GRABSUB,
      GRABSUB_FLAT,
      EXPORT,
      SAVE,
      PRINT
    } ApiCode;

    typedef enum {
      NOTMP,
      MKTMP
    } ApiFlagCode;

    typedef enum {
      EXTGROUP_RENAME,
      EXTGROUP_DELETE,
      EXTGROUP_ADD,
      EXTGROUP_REMOVE,
      EXTGROUP_ASSIGN,
      EXTGROUP_GRABSCR,
      EXTGROUP_GRABSAV,
      EXTGROUP_GRABSUB,
      EXTGROUP_GRABSUB_FLAT,
      EXTGROUP_EXPORT,
      EXTGROUP_SAVE,
      EXTGROUP_PRINT
    } UtilCode;

    typedef enum {
      UNKNOWN,
      UNARY,
      BINARY, 
      ONE_OR_MORE,
      TWO_OR_MORE
    } SelNumCode;

    typedef enum {
      NAME_INVALID,
      NAME_VALID,
      NAME_BADCHAR,
      NAME_FATAL
    } NameCode;

    typedef enum {
      SHOW_ANSWER  = 0x01,
      SHOW_TARGET  = 0x02,
      SHOW_RNAME   = 0x04,
      REVISE_RNAME = 0x08,
      RETURNS_OBJ  = 0x10,
      SEND_RNAME   = 0x20,
      ECHO_RNAME   = 0x40
    } ActionCode;

    class Action
    {
      public:
	Action();
        Action(char *, char *, SelNumCode, SelNumCode, unsigned int);
        ~Action();
        char *name;
        char *cmd;
        SelNumCode sn;
        SelNumCode rn;
        unsigned int flags;
    };

    class ActionArr
    {
      public:
	ActionArr();
	~ActionArr();
	bool Append(Action&);
	Action& operator[](int) const;
	Action& Reference(int) const;
	unsigned int size() const;
      private:
        unsigned int sz;
        Action *arr;
    };

    void Refresh(const int);
    char *GetQueryList();
    bool PerformQuery(const unsigned int, const symbolArr&, symbolArr&);
    bool PerformUtil(const unsigned int, const symbolArr&, symbolArr&);
    int GetMakeTempFlag();
    void SetMakeTempFlag(int);
    char *SnatchResultName(bool);
    char *GetResultName();
    char *GetResultPrefix();
    void SetResultName(const char *);
    bool SetResultPrefix(const char *);

  private:

    void GetGroups(symbolArr&);
    bool Perform(const Action *, const symbolArr&, symbolArr&);
    int SendCommand(const char *);
    int SendCommand(const char *, char *&);
    void SelList(const symbolArr&, genString&);
    bool CheckSel(SelNumCode, int, bool explicit_msg = true);
    void NotifyTclError();
    bool Init();
    char *DefaultResultName();
    char *DefaultResultName(const char *, int&);
    NameCode ValidName(const char *);
    bool ValidChar(const char);
    bool ValidChars(const char *, bool verbose = true);
    char *Strdup(const char *);
    void ExpandGroup(const char *, symbolArr&);
    void ExpandGroup(const symbolArr&, symbolArr&);
    void AppendQuery(char *, char *, SelNumCode, SelNumCode, unsigned int);
    void AppendUtil(char *, char *, SelNumCode, SelNumCode, unsigned int);

    static char *apiSub[];
    static char *apiFlags[];
    static const char *apiCmd;
    static ActionArr *utils;
    static ActionArr *queries;
    static genString *queryList;
    static char *debugPref;
    static bool initFlag;
    static int debugLevel;
    static bool makeTempFlag;
    static char *resultName;
    static char *resultPrefix;
    static int resultIndex;

};

#endif /* !_GroupApp_h */
