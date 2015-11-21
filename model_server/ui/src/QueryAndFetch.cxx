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
// /aset/ui/globals/QueryAndFetch.C
//------------------------------------------
// description:
// 
// Pops up a dialog listing a set of files, informing the user that they
// need to be fetched before the requested operation can succeed, and
// allowing the user to check out the files or abort the operation.
//------------------------------------------
// History:
//
// Fri May 19 09:29:45 1995:	Created by William M. Miller
//
//------------------------------------------
#include "genError.h"
#include "machdep.h"

#include "QueryAndFetch.h"
#include <messages.h>

#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif

#ifndef _xref_h
#include <xref.h>
#endif

#ifndef _gtLabel_h
#include <gtLabel.h>
#endif

#ifndef _gtForm_h
#include <gtForm.h>
#endif

#ifndef _gtList_h
#include <gtList.h>
#endif

#ifndef _ste_interface_h
#include <ste_interface.h>
#endif

#ifndef _top_widgets_h
#include <top_widgets.h>
#endif

#ifndef __psetmem_h
#include <psetmem.h>
#endif

#ifndef _cmd_h
#include <cmd.h>
#endif


//======================================================================
// popup_QueryAndFetch - Moved to interface/src/popup_QueryAndFetch.C
//======================================================================



//****************************************
// Implementations for class QueryAndFetch
//****************************************



//========================================
// QueryAndFetch::QueryAndFetch
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 10:14:35 1995:	Created by William M. Miller
//========================================

 QueryAndFetch::QueryAndFetch (const symbolArr& files) {
   Initialize(QueryAndFetch::QueryAndFetch);

   size_t sz = files.size();
   {
      char** entries = (char**) psetmalloc(sz * sizeof(char*));
      for (size_t idx = 0; idx < sz; idx++) {
	 entries[idx] = files[idx].get_name();
      }
#ifndef NEW_UI
      shell = gtDialogTemplate::create(NULL, "QueryAndFetch", TXT("Files to Fetch"));
      shell->add_button("fetch", TXT("Fetch"), fetchCB, this);
      shell->add_button("cancel", TXT("Cancel"), cancelCB, this);
      shell->add_help_button();
      shell->help_context_name("Pset.Designer.Help.FetchFiles");

      gtLabel* label = gtLabel::create(shell, "label",
			       TXT("Must fetch the following files") );
      label->alignment(gtCenter);
      gtForm::vertStack(label);
      label->manage();
      gtList* list = gtList::create((gtBase *)shell, (const char *)"file_list", (const char *)"", (gtListStyle)gtSingle,
				    (const char **)entries, (int)sz);
      list->attach(gtTop, label, 10);
      list->attach_pos_bottom(100);
      list->attach(gtLeft);
      list->attach(gtRight);
      list->num_rows(6);
      list->width(300);
      list->manage();
#endif
      psetfree(entries);
   }
}


//========================================
// QueryAndFetch::~QueryAndFetch
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 10:14:57 1995:	Created by William M. Miller
//========================================

QueryAndFetch::~QueryAndFetch () {
   Initialize(QueryAndFetch::~QueryAndFetch);
#ifndef NEW_UI
   delete shell;
#endif
}


//========================================
// QueryAndFetch::ask
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 10:15:34 1995:	Created by William M. Miller
//========================================

int QueryAndFetch::ask () {
   Initialize(QueryAndFetch::ask);
#ifndef NEW_UI
   answer = 0;
   shell->popup(1);
   shell->take_control_top(answer_func, this);
   shell->popdown();
   return answer;
#else
   return 0;
#endif
}


//========================================
// QueryAndFetch::fetchCB
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 10:17:34 1995:	Created by William M. Miller
//========================================

void QueryAndFetch::fetchCB (gtPushButton*, gtEventPtr, void* cd, gtReason) {
   Initialize(QueryAndFetch::fetchCB);
#ifndef NEW_UI
   ((QueryAndFetch*) cd)->answer = 1;
#endif
}


//========================================
// QueryAndFetch::cancelCB
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 10:18:39 1995:	Created by William M. Miller
//========================================

void QueryAndFetch::cancelCB (gtPushButton*, gtEventPtr, void* cd, gtReason) {
   Initialize(QueryAndFetch::cancelCB);
#ifndef NEW_UI
   ((QueryAndFetch*) cd)->answer = -1;
#endif
}




//========================================
// QueryAndFetch::answer_func
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Fri May 19 12:21:55 1995:	Created by William M. Miller
//========================================

int QueryAndFetch::answer_func (void* cd) {
   Initialize(QueryAndFetch::answer_func);
#ifndef NEW_UI
   return ((QueryAndFetch*) cd)->answer;
#else
   return 0;
#endif
}





