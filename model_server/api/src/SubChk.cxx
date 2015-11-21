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
#include "SubChk.h"
#include "xxinterface.h"
#include "msg.h"

init_relational(SubChkMod, QModule);

#ifdef _WIN32  /* make the NT linker to behave */
int subchk_init_flag = 0;
#endif

//
// SC_RootNode:
//

SC_RootNode::SC_RootNode() : RootNode()
{
  appcode = QueryApp::SUBCHK;
}

SC_RootNode::SC_RootNode(const SC_RootNode &that) { *this = that; }
SC_RootNode::~SC_RootNode() {}

void SC_RootNode::operator=(const SC_RootNode &that)
{
  RootNode::operator=(that);

  bugid = that.bugid;
  bugcomment = that.bugcomment;
  automail = that.automail;
}

GenericTree *SC_RootNode::CloneNode() const { return new SC_RootNode(*this); }

SC_AutoMail::SC_AutoMail()
{
  for (int i = 0; i < SC_REPORT_TYPE_MAX; i++) {
    for (int j = 0; j < SC_REPORT_FORMAT_MAX; j++) {
      sendtouser[i][j] = 0;
    }
  }

  sendtouser[SC_REPORT_TYPE_DETAILED][SC_REPORT_FORMAT_TEXT] = 1;
}

SC_AutoMail::~SC_AutoMail() {}

SC_AutoMail::SC_AutoMail(const SC_AutoMail &that) { operator=(that); }

SC_AutoMail &SC_AutoMail::operator=(const SC_AutoMail &that)
{
  for (int i = 0; i < SC_REPORT_TYPE_MAX; i++) {
    for (int j = 0; j < SC_REPORT_FORMAT_MAX; j++) {
      recipients[i][j] = that.recipients[i][j];
      sendtouser[i][j] = that.sendtouser[i][j];
    }
  }

  return *this;
}

//
// SC_QueryNode:
//

SC_QueryNode::SC_QueryNode() : QueryNode(), tolerance(0), legacy(1), action(SC_ACTION_WARN) {}
SC_QueryNode::SC_QueryNode(const SC_QueryNode &that) { *this = that; }
SC_QueryNode::~SC_QueryNode() {}

void SC_QueryNode::operator=(const SC_QueryNode &that)
{ 
  QueryNode::operator=(that);

  tolerance = that.tolerance;
  legacy = that.legacy;
  action = that.action;
  cres = that.cres;
  pres = that.pres;
}

GenericTree *SC_QueryNode::CloneNode() const { return new SC_QueryNode(*this); }

extern const char *sc_action_names[];

inline const char *ActionToName(SC_ActionCode action)
{
  const char *name = NULL;
  if ((int)action < SC_ACTION_MAX) {
    name = sc_action_names[(int)action];
  } else {
    name = NULL;
  }
  return name;
}

inline SC_ActionCode NameToAction(const char *name)
{
  SC_ActionCode action = SC_ACTION_NONE;

  for (int i = 0; i < SC_ACTION_MAX; i++) {
    if (OSapi_strcmp((char *)sc_action_names[i], (char *)name) == 0) {
      action = (SC_ActionCode)i;
      break;
    }
  }

  return action;
}

void DEX_Action(int m, genString &s, SC_ActionCode &action)
{
  if (m == DEX_SET || m == DEX_WRITE) {
    action = NameToAction(s);
  } else {
    s = ActionToName(action);
  }
}

void SC_RootNode::InstallFields()
{
  RootNode::InstallFields();

  RegisterField("total", DEX_Total, "int", "Total Number of Violations", DEX_INVISIBLE | DEX_DERIVED);

  RegisterField("warnings",    DEX_Warnings,    "set",       "Warnings",         DEX_INVISIBLE | DEX_DERIVED);
  RegisterField("rejects",     DEX_Rejects,     "set",       "Rejects",          DEX_INVISIBLE | DEX_DERIVED);
  RegisterField("disposition", DEX_Disposition, "sc_action", "Disposition",      DEX_INVISIBLE | DEX_DERIVED);
  RegisterField("bugid",       DEX_BugID,       "string",    "Bug ID",           DEX_INVISIBLE              );
  RegisterField("comment",     DEX_BugComment,  "string",    "Bug Comment",      DEX_INVISIBLE              );

  RegisterField("mail_dtl", DEX_MailDTL, "string", "Recipients of detailed text report");
  RegisterField("mail_stl", DEX_MailSTL, "string", "Recipients of summary text report");
  RegisterField("mail_dtu", DEX_MailDTU, "bool",   "Send detailed text report to user");
  RegisterField("mail_stu", DEX_MailSTU, "bool",   "Send summary text report to user");
}

void SC_RootNode::DEX_BugID      (int m, genString &s, void *p) { DEX_String (m, s, ( (SC_RootNode *)  p)->bugid      ); }
void SC_RootNode::DEX_BugComment (int m, genString &s, void *p) { DEX_String (m, s, ( (SC_RootNode *)  p)->bugcomment ); }

void SC_RootNode::DEX_Total(int m, genString &s, void *p)
{
  SC_RootNode &root = *(SC_RootNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    int total = root.GetTotal();
    s.printf("%d", total);
  }
}

void SC_RootNode::DEX_Warnings(int m, genString &s, void *p)
{
  SC_RootNode &root = *(SC_RootNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    symbolArr warnings;
    root.GetHits(SC_ACTION_WARN, warnings);
    DEX_Symbols(m, s, warnings);
  }
}

void SC_RootNode::DEX_Rejects(int m, genString &s, void *p)
{
  SC_RootNode &root = *(SC_RootNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    symbolArr rejects;
    root.GetHits(SC_ACTION_REJECT, rejects);
    DEX_Symbols(m, s, rejects);
  }  
}

void SC_RootNode::DEX_Disposition(int m, genString &s, void *p)
{
  SC_RootNode &root = *(SC_RootNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    SC_ActionCode disposition = root.GetDisposition();
    DEX_Action(m, s, disposition);
  }
}

int SC_RootNode::GetTotal()
{
  int total = 0;

  QTreePtrArr queries;
  int n = FindActiveQueries(queries);
  for (int i = 0; i < n; i++) {
    SC_QueryNode &query = *((SC_QueryNode *)(*(queries[i])));
    total += query.GetTotal();
  }

  return total;
}

void SC_RootNode::GetHits(SC_ActionCode action, symbolArr &syms)
{
  QTreePtrArr queries;
  int n = FindActiveQueries(queries);
  for (int i = 0; i < n; i++) {
    SC_QueryNode &query = *((SC_QueryNode *)(*(queries[i])));
    if ((query.action == action) &&
       ((query.action == SC_ACTION_NONE) || (query.GetExcessHitCount() > 0))) {
      symbolArr hits;
      query.GetHits(hits);
      syms.insert_last(hits);
    }
  }
}

SC_ActionCode SC_RootNode::GetDisposition()
{
  SC_ActionCode disposition = SC_ACTION_NONE;

  QTreePtrArr queries;
  int n = FindActiveQueries(queries);
  for (int i = 0; (i < n) && (disposition != SC_ACTION_REJECT); i++) {
    SC_QueryNode &query = *((SC_QueryNode *)(*(queries[i])));
    if ((query.GetExcessHitCount() > 0) && (query.action > disposition)) {
      disposition = query.action;
    }
  }

  return disposition;
}

void SC_RootNode::DEX_MailList(int m, genString &s, SC_RootNode &root, SC_ReportType type, SC_ReportFormat format)
{
  DEX_String(m, s, root.automail.recipients[type][format]);
}

void SC_RootNode::DEX_MailUser(int m, genString &s, SC_RootNode &root, SC_ReportType type, SC_ReportFormat format)
{
  DEX_Bool(m, s, root.automail.sendtouser[type][format]);
}

void SC_RootNode::DEX_MailDTL(int m, genString &s, void *p)
{
  SC_RootNode &root = *((SC_RootNode *)p);
  DEX_MailList(m, s, root, SC_REPORT_TYPE_DETAILED, SC_REPORT_FORMAT_TEXT);
}

void SC_RootNode::DEX_MailSTL(int m, genString &s, void *p)
{
  SC_RootNode &root = *((SC_RootNode *)p);
  DEX_MailList(m, s, root, SC_REPORT_TYPE_SUMMARY, SC_REPORT_FORMAT_TEXT);
}

void SC_RootNode::DEX_MailDTU(int m, genString &s, void *p)
{
  SC_RootNode &root = *((SC_RootNode *)p);
  DEX_MailUser(m, s, root, SC_REPORT_TYPE_DETAILED, SC_REPORT_FORMAT_TEXT);
}

void SC_RootNode::DEX_MailSTU(int m, genString &s, void *p)
{
  SC_RootNode &root = *((SC_RootNode *)p);
  DEX_MailUser(m, s, root, SC_REPORT_TYPE_SUMMARY, SC_REPORT_FORMAT_TEXT);
}

void SC_QueryNode::InstallFields()
{
  QueryNode::InstallFields();

  RegisterField("total",     DEX_Total,     "int",       "Total Violations",                               DEX_DERIVED);
  RegisterField("hits",      DEX_Hits,      "set",       "Violations",                     DEX_INVISIBLE | DEX_DERIVED);
  RegisterField("tolerance", DEX_Tolerance, "int",       "Tolerance"                                                  );
  RegisterField("action",    DEX_Action,    "sc_action", "Action"                                                     );
  RegisterField("legacy",    DEX_Legacy,    "bool",      "Allow Legacy Code"                                          );

  RegisterField("cres",      DEX_CRes,      "set",       "New Violations",                 DEX_INVISIBLE              );
  RegisterField("pres",      DEX_PRes,      "set",       "Inherited Violations",           DEX_INVISIBLE              );
  RegisterField("crescnt",   DEX_CResCnt,   "int",       "Number of New Violations",       DEX_INVISIBLE | DEX_DERIVED);
  RegisterField("prescnt",   DEX_PResCnt,   "int",       "Number of Inherited Violations", DEX_INVISIBLE | DEX_DERIVED);

}

void SC_QueryNode::DEX_Tolerance (int m, genString &s, void *p) { DEX_Integer  (m, s, ( (SC_QueryNode *)  p)->tolerance ); }
void SC_QueryNode::DEX_Legacy    (int m, genString &s, void *p) { DEX_Bool     (m, s, ( (SC_QueryNode *)  p)->legacy    ); }
void SC_QueryNode::DEX_Action    (int m, genString &s, void *p) { ::DEX_Action (m, s, ( (SC_QueryNode *)  p)->action    ); }
void SC_QueryNode::DEX_CRes      (int m, genString &s, void *p) { DEX_Symbols  (m, s, ( (SC_QueryNode *)  p)->cres      ); }
void SC_QueryNode::DEX_PRes      (int m, genString &s, void *p) { DEX_Symbols  (m, s, ( (SC_QueryNode *)  p)->pres      ); }
void SC_QueryNode::DEX_CResCnt   (int m, genString &s, void *p) { DEX_SymCnt   (m, s, ( (SC_QueryNode *)  p)->cres      ); }
void SC_QueryNode::DEX_PResCnt   (int m, genString &s, void *p) { DEX_SymCnt   (m, s, ( (SC_QueryNode *)  p)->pres      ); }

void SC_QueryNode::DEX_Hits(int m, genString &s, void *p)
{
  SC_QueryNode &query = *(SC_QueryNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    symbolArr hits;
    query.GetHits(hits);
    DEX_Symbols(m, s, hits);
  }
}

void SC_QueryNode::DEX_Total(int m, genString &s, void *p)
{
  SC_QueryNode &query = *(SC_QueryNode *)p;
  if (m == DEX_GET || m == DEX_READ) {
    int total = query.GetTotal();
    s.printf("%d", total);
  }
}

void SC_QueryNode::GetHits(symbolArr &hits)
{
  hits = cres;
  if (!legacy)
    hits.insert_last(pres);
}

int SC_QueryNode::GetTotal()
{
  int total = cres.size();
  if (!legacy)
    total += pres.size();
  return total;
}

int SC_QueryNode::GetExcessHitCount()
{
  int ret = 0;

  int total = GetTotal();
  if (total > tolerance)
    ret = total - tolerance;

  return ret;    
}

void SC_RootNode::Clean()
{
  RootNode::Clean();

  bugid = "";
  bugcomment = "";
}

void SC_QueryNode::Clean()
{
  QueryNode::Clean();

  cres.removeAll();
  pres.removeAll();
}

int SC_RootNode::RunNodes(Interpreter *shell, QTreePtrArr &trees)
{
  int ret = TCL_ERROR;

  static bool has_lic = false;
  if ( ! has_lic )
    if ( _lo(LIC_SUBCHECK) )
      {
	_lm(LIC_SUBCHECK);
	return ret;
      }
    else
      has_lic = true;	

  RootNode::RunNodes(shell, trees);

  TclList list;
  for (int i = 0; i < trees.size(); i++) {
    QTree *tree = *(trees[i]);
    genString id;
    QTree::TreeToId(tree, id);
    list += id;
  }

  genString cmd;
  cmd += "subchk:run ";
  cmd += list;
  ret = shell->EvalCmd((char *)cmd);

  return ret;
}

SubChkMod::SubChkMod(const char *name, const char *fname, extStatus status) :
  QModule(name, fname, status)
{
  info()->get_apps()->insert(this);
}

SubChkMod::~SubChkMod()
{
}

externApp *SubChkMod::create(const char *name, externFile *extf)
{
  SubChkMod *module = NULL;

  externInfo *tinfo = type_info();

  if (tinfo && tinfo->enabled()) {
    char const *phys_filename;
    genString fn;

    if (extf) {
      phys_filename = extf->get_phys_filename();
    }
    else {
      tinfo->new_fname(name, fn);
      phys_filename = fn.str();
    }
    module = new SubChkMod(name, phys_filename);
    module->make_savable(extf);
    Interpreter *shell = GetActiveInterpreter();
    if (shell) {
      TclList cmd;
      cmd += "subchk:creation_hook";
      cmd += name;
      shell->EvalCmd((char *)cmd);
    }
  }

  return module;
}

externInfo *SubChkMod::info() const
{
  return type_info();
}

externInfo *SubChkMod::type_info()
{
  static externInfo *inf = new externInfo(SubChkMod::create,
                                          "sc",
                                          QueryApp::appNames[QueryApp::SUBCHK],
                                          "ext");
  return inf;
}

void SubChkMod::InstallUserLibs()
{
  const char *libs = customize::getStrPref("SubmissionCheck.Libraries");
  if (libs) {
    TclList list(libs);
    int sz = list.Size();
    for (int i = 0; i < (sz - 1); i += 2) {
      const char *name = list[i];
      const char *path = list[i + 1];
      QModule *module = QModule::Find(name);
      if (!module) {
        int ok = 0;
        QTree *tree = QTree::Read(path);
        if (tree) {
          module = QModule::Create(name, tree);
          if (module) {
            ok = 1;
	  }
	}
        if (!ok) {
          msg("STDERR: Failed to install user library: \"$1\" (\"$2\").") << name << eoarg << path << eom;
	}
      } else {
        msg("STDERR: Cannot install duplicate user library: \"$1\" (\"$2\").") << name << eoarg << path << eom;
      }
    }
  }
}

int SubChkCmd::Register()
{
  new cliCommandInfo("subchk", SubChkCmd::Process);
  externInfo::add(SubChkMod::type_info());

  return 0;
}

static int dummy = SubChkCmd::Register();

inline int streq(const char *a, const char *b) { return (OSapi_strcmp((char *)a, (char *)b) == 0); }

int SubChkCmd::Process(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  int ret = TCL_ERROR;

#if 0  // hasn't done anything since /main/1!
  Interpreter *shell = (Interpreter *)cd;

  int help = 0;

  if (argc >= 2) {

    char *subcmd = argv[1];

    help = 1;
  } else {
    help = 1;
  }  

 if (help == 1) {
   ;
 }
#endif
  return ret;
}

