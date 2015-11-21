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
#ifndef _SubChk_h
#define _SubChk_h

#include <QueryTree.h>

typedef enum {
  SC_ACTION_NONE,
  SC_ACTION_WARN,
  SC_ACTION_REJECT,
  SC_ACTION_MAX
} SC_ActionCode;

typedef enum {
  SC_REPORT_TYPE_DETAILED,
  SC_REPORT_TYPE_SUMMARY,
  SC_REPORT_TYPE_MAX         /* must be last */
} SC_ReportType;

typedef enum {
  SC_REPORT_FORMAT_TEXT,
  SC_REPORT_FORMAT_HTML,
  SC_REPORT_FORMAT_MAX       /* must be last */
} SC_ReportFormat;

class SC_AutoMail
{

  public:
                             SC_AutoMail();
                             SC_AutoMail(const SC_AutoMail &);
                             ~SC_AutoMail();

    SC_AutoMail&             operator=(const SC_AutoMail &);

    genString                recipients [SC_REPORT_TYPE_MAX][SC_REPORT_FORMAT_MAX];
    int                      sendtouser [SC_REPORT_TYPE_MAX][SC_REPORT_FORMAT_MAX];
};

class SC_RootNode : public RootNode
{

  public:

                             SC_RootNode();
                             SC_RootNode(const SC_RootNode &);
    virtual                  ~SC_RootNode();

    void                     operator=(const SC_RootNode &);

    virtual GenericTree*     CloneNode() const;

    virtual void             InstallFields();

    virtual void             Clean();

    virtual int              RunNodes(Interpreter *, QTreePtrArr &);

    virtual int              GetTotal();
    virtual void             GetHits(SC_ActionCode, symbolArr &);
    virtual SC_ActionCode    GetDisposition();

    genString                bugid;         /* bug id of live submission */
    genString                bugcomment;    /* bug comment of live submission */

    SC_AutoMail              automail;      /* automatic mailer options */

  private:

    static void              DEX_Total       (int, genString &, void *);
    static void              DEX_Warnings    (int, genString &, void *);
    static void              DEX_Rejects     (int, genString &, void *);
    static void              DEX_Disposition (int, genString &, void *);
    static void              DEX_BugID       (int, genString &, void *);
    static void              DEX_BugComment  (int, genString &, void *);

    static void              DEX_MailDTL     (int, genString &, void *);
    static void              DEX_MailSTL     (int, genString &, void *);
    static void              DEX_MailDTU     (int, genString &, void *);
    static void              DEX_MailSTU     (int, genString &, void *);

    static void              DEX_MailList    (int, genString &, SC_RootNode &, SC_ReportType, SC_ReportFormat);
    static void              DEX_MailUser    (int, genString &, SC_RootNode &, SC_ReportType, SC_ReportFormat);

};

class SC_QueryNode : public QueryNode
{

  public:

                             SC_QueryNode();
                             SC_QueryNode(const SC_QueryNode &);
    virtual                  ~SC_QueryNode();

    void                     operator=(const SC_QueryNode &);

    virtual GenericTree*     CloneNode() const;

    virtual void             InstallFields();

    virtual void             Clean();

    virtual void             GetHits(symbolArr &);
    virtual int              GetTotal();
    virtual int              GetExcessHitCount();

    int                      tolerance;   // number of hits allowed
    int                      legacy;      // let legacy code through?
    SC_ActionCode            action;      // what happens when threshold is broken
    symbolArr                cres;        // new hits
    symbolArr                pres;        // legacy hits

  private:

    static void              DEX_Tolerance  (int, genString &, void *);
    static void              DEX_Legacy     (int, genString &, void *);
    static void              DEX_Action     (int, genString &, void *);
    static void              DEX_QTag       (int, genString &, void *);
    static void              DEX_CRes       (int, genString &, void *);
    static void              DEX_CResCnt    (int, genString &, void *);
    static void              DEX_PRes       (int, genString &, void *);
    static void              DEX_PResCnt    (int, genString &, void *);
    static void              DEX_Total      (int, genString &, void *);
    static void              DEX_Hits       (int, genString &, void *);

};

class SubChkMod : public QModule
{

  public:

    virtual                  ~SubChkMod();

    externInfo*              info() const;
    static externInfo*       type_info();
    static externApp*        create(const char *, externFile* f = NULL);
    static void              InstallUserLibs();

    define_relational(SubChkMod, QModule);

  protected:

                             SubChkMod(const char *, const char *, extStatus = TEMP);

};

class SubChkCmd
{

  public:

    static int               Register();

    static int               Process(ClientData, Tcl_Interp *, int, char const **);

};

#endif /* ! _SubChk_h */
