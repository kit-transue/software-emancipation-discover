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
// /aset/ui/globals/ModelAliasList.C
//------------------------------------------
// description:
// 
// Pops up a dialog listing a set of entities that will be aliased for
// dormant code analysis and/or subsystem extraction and allows the user
// to cancel the operation if desired.
//------------------------------------------
// History:
//
// Tue August 27 15:42:13 1996:	Created by William M. Miller
//
//------------------------------------------
#include "genError.h"


#include "ModelAliasList.h"

#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif

#ifndef _xref_h
#include <xref.h>
#endif

#ifndef _systemMessages_h
#include <systemMessages.h>
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

#ifndef _ldrList_h
#include <ldrList.h>
#endif


//========================================
// popup_ModelAliasList
//----------------------------------------
// Description:
//
// Displays the list of entities, returning false if the user hits Cancel
// and true otherwise.
//----------------------------------------
// History:
// 
// Tue August 27 15:43:58 1996:	Created by William M. Miller
//========================================

bool popup_ModelAliasList(const symbolArr& syms) {
   Initialize(popup_ModelAliasList);

   cmd_validate("ModelAliasList-input", (symbolArr&) syms);
   genString* ents = new genString[syms.size()];
   for (size_t i = 0; i < syms.size(); i++) {
      symbolPtr sym = syms[i];
      const unsigned char *prefix = get_prefix(sym);
      if (sym.xrisnotnull()) {
	 symbolPtr dfs = sym->get_def_file();
	 if (dfs.xrisnotnull()) {
	    ents[i].printf("%s%s [%s]", prefix, sym.get_name(), dfs.get_name());
	 }
	 else ents[i].printf("%s%s", prefix, sym.get_name());
      }
      else ents[i].printf("%s%s", prefix, sym.get_name());
   }
   ModelAliasList q(ents, syms.size());
   int resp = q.ask();
   delete [] ents;
   return (resp == 1);
}


//****************************************
// Implementations for class ModelAliasList
//****************************************



//========================================
// ModelAliasList::ModelAliasList
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:07:11 1996:	Created by William M. Miller
//========================================

ModelAliasList::ModelAliasList (const genString names_and_files[], size_t sz) {
   Initialize(ModelAliasList::ModelAliasList);
#ifndef NEW_UI
   char** entries = (char**) psetmalloc(sz * sizeof(char*));
   for (size_t idx = 0; idx < sz; idx++) {
      entries[idx] = names_and_files[idx];
   }
   shell = gtDialogTemplate::create(NULL, "ModelAliasList", TXT("Aliased Entities"));
   shell->add_button("ok", TXT("OK"), OK_CB, this);
   shell->add_button("cancel", TXT("Cancel"), cancel_CB, this);
   shell->add_help_button();
   shell->help_context_name("Pset.Help.Subsystems.Extract.Name");

   gtLabel* label = gtLabel::create(shell, "label",
				    TXT("Entities in this list with the same name will be aliased together:"));
   label->alignment(gtCenter);
   gtForm::vertStack(label);
   label->manage();
   gtList* list = gtList::create((gtBase *)shell, (const char *)"entity_list", (const char *)"", (gtListStyle)gtSingle,
				 (const char **)entries, (int)sz);
   list->attach(gtTop, label, 10);
   list->attach_pos_bottom(100);
   list->attach(gtLeft);
   list->attach(gtRight);
   if (sz < 20) {
      list->num_rows(sz);
   }
   else list->num_rows(20);
   list->width(500);
   list->manage();
   psetfree(entries);
#endif
}


//========================================
// ModelAliasList::~ModelAliasList
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:15:07 1996:	Created by William M. Miller
//========================================

ModelAliasList::~ModelAliasList () {
   Initialize(ModelAliasList::~ModelAliasList);
#ifndef NEW_UI
   delete shell;
#endif
}


//========================================
// ModelAliasList::ask
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:15:31 1996:	Created by William M. Miller
//========================================

int ModelAliasList::ask () {
   Initialize(ModelAliasList::ask);
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
// ModelAliasList::OK_CB
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:16:09 1996:	Created by William M. Miller
//========================================

void ModelAliasList::OK_CB (gtPushButton*, gtEventPtr, void* cd, gtReason) {
   Initialize(ModelAliasList::OK_CB);
#ifndef NEW_UI
   ((ModelAliasList*) cd)->answer = 1;
#endif
}


//========================================
// ModelAliasList::cancel_CB
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:16:31 1996:	Created by William M. Miller
//========================================

void ModelAliasList::cancel_CB (gtPushButton*, gtEventPtr, void* cd, gtReason) {
   Initialize(ModelAliasList::cancel_CB);
#ifndef NEW_UI
   ((ModelAliasList*) cd)->answer = -1;
#endif
}




//========================================
// ModelAliasList::answer_func
//----------------------------------------
// Description:
//
//----------------------------------------
// History:
// 
// Tue August 27 16:17:34 1996:	Created by William M. Miller
//========================================

int ModelAliasList::answer_func (void* cd) {
   Initialize(ModelAliasList::answer_func);
#ifndef NEW_UI
   return ((ModelAliasList*) cd)->answer;
#else
   return 0;
#endif
}





