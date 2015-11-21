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
// This is a -*- C++ -*- header file

#ifndef _QualityCockpitUI_h
#define _QualityCockpitUI_h

#ifndef _general_h
#include <general.h>
#endif

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

class QualityCockpitUI
{
public:

  // Interface
  static void Invoke(const char *);
  static int  Quit(void);
  ~QualityCockpitUI();
  
private:
  
  QualityCockpitUI(char const * = 0);

private:

  enum categoryType
    {
      FOLDER,
      QUERY,
      THRESH_QUERY,
      STAT_QUERY,
      UNKNOWN
    };
  
  enum session_op
    {
      NEW,
      LOAD,
      CLOSE,
      SAVE,
      SAVE_AS,
      DELETE
    };

  enum run_scope
    {
      ALL,
      SELECTED
    };

private:

  // INITIALIZATION FUNCTIONS
  void         buildInterface(char const *);
  void         initData(void);

  // CALLBACKS 
  static int   destroy_CB(void *);
  static void  browseHits_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  categoryUp_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  closeSession_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  disableQuery_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void  enableQuery_CB(gtPushButton*, gtEventPtr, void *, gtReason);
  static void  exit_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  newSession_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  openSession_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  query_dbl_click_CB(gtList*, gtEventPtr, void *, gtReason);
  static void  query_select_CB(gtList*, gtEventPtr, void *, gtReason);
  static void  remove_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  report_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  reset_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  runAll_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  runSelected_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  saveAs_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  save_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  typeInput_CB(gtPushButton*, gtEventPtr, void*, gtReason);
  static void  weightChanged_CB(gtStringEditor*, gtEventPtr, void*, gtReason);

  // UI API
  int          browseHits(void);
  int          resetWeights(void);
  int          runQueries(run_scope);
  int          runReport(void);
  int          newSession(void);
  int          loadSession(void);
  int          closeSession(void);
  int          saveSession(void);
  int          saveAsSession(void);
  int          deleteSession(void);
  int          upCategory(void);
  int          weightChanged(void);

  // UI HELPER FUNCTIONS
  genString    getSelectedQuery(void);
  int          getNewThreshold(genString const &);
  int          getReportInfo(genString &, genString &, bool &);
  int          validateInteger(genString &);
  void         retitle(void);

  // DATA DISPLAY FUNCTIONS
  int          addIcon(genString const &, genString const &, genString &);
  int          buildCategoryList(genString const &);
  int          updateFields(void);
  int          updateIcon(void);
  int          displayErrors(genString const &);

  // ACCESS API
  categoryType getType(genString const &);
  genString    getOverallScore(void);
  genString    getParent(genString const &);
  genString    getQueryHits(genString const &);
  genString    getQueryScore(genString const &);
  genString    getQueryWeight(genString const &);
  genString    getQueryThreshold(genString const &);
  genString    getSessionName(void);
  genString    selectSession(bool);
  int          browseHits(genString const &);
  int          changeWeight(genString const &, genString const &);
  int          changeThreshold(genString const &, genString const &);
  int          debuggingAccess(void);
  int          deleteSession(genString const &);
  int          disable(genString const &);
  int          enable(genString const &);
  int          enabled(genString const &);
  int          execQueryTree(genString const &);
  int          getChildren(genString const &, genString *&);
  int          getAllSessions(genString *&);
  int          getInputTypes(genString *&);
  int          loadSession(genString const &);
  int          resetTree(void);
  int          recalculateTree(genString const &);
  int          saveSession(genString const &);
  void         initAPI(void);

  // DIRECT API TO ACCESS
  int          sendCmd(genString, genString &);
  void         notifyTclError(void);
  
private:

  int                        debug_access;
  int                        invoked;
  char const                *base_title;
  char const                *full_title;
  bool                       session_modified;
  genString                  cur_input_type;
  genString                 *input_types;
  static QualityCockpitUI   *instance; 
  static int const           MIN_HEIGHT;
  static int const           MIN_WIDTH;
  static char const * const  ROOT_FOLDER;
  static char const          FOLDER_SEP;
  static char const          ENABLED_FOLDER;
  static char const          DISABLED_FOLDER;
  static char const          ENABLED_QUERY;
  static char const          DISABLED_QUERY;

private:

  // widgets we need to keep track of
  
  gtTopLevelShell *toplev;
  gtMainWindow    *main_window;
  gtList          *query_list;
  gtRadioBox      *status_select;
  gtOptionMenu    *input_menu;
  gtPushButton    *up;
  gtPushButton    *run_all_btn;
  gtPushButton    *reset_btn;  
  gtPushButton    *report_btn; 
  gtPushButton    *close_btn;  
  gtPushButton    *run_selected_btn;
  gtPushButton    *browse_hits;
  gtStringEditor  *category_txt;
  gtStringEditor  *hits_txt;
  gtStringEditor  *overall_txt;
  gtStringEditor  *score_txt;
  gtStringEditor  *weight_txt;

private:

  // flags to keep track of available licenses

  bool report_license_exists;
  bool run_license_exists;

private:

  // private class for ui for selection sessions
#ifndef CANT_FWD_DECL_NESTED
  class sessionListUI;
  class errorOutputUI;
#else
#include <sessionListUI.h>
#include <errorOutputUI.h>
#endif
};

#endif /* ! _QualityCockpitUI_h */
