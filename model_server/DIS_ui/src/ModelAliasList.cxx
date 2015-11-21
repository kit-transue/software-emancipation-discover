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
// ModelAliasList.C
//------------------------------------------
// description:
// 
// Pops up a dialog listing a set of entities that will be aliased for
// dormant code analysis and/or subsystem extraction and allows the user
// to cancel the operation if desired.
//------------------------------------------
#ifndef vportINCLUDED
   #include <vport.h>
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif
#ifndef vapplicationINCLUDED
#include vapplicationHEADER
#endif

// Discover includes.
// Need to adjust some Galaxy defs.
#undef printf    // Galaxy has its own printf macro.

#include <cLibraryFunctions.h>
#ifndef _genError_h
#include <genError.h>
#endif

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "Application.h"
#include "../../DIS_ui/interface.h"
#undef printf
extern Application* DISui_app;

#ifndef _ModelAliasList_h
#include "ModelAliasList.h"
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _xref_h
#include <xref.h>
#endif

#ifndef _systemMessages_h
#include <systemMessages.h>
#endif

#ifndef _ste_interface_h
#include <ste_interface.h>
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
//========================================

bool popup_ModelAliasList(const symbolArr& syms) {
   Initialize(popup_ModelAliasList);

   genString* ents = new genString[syms.size()];
   for (size_t i = 0; i < syms.size(); i++) {
      symbolPtr sym = syms[i];
      const unsigned char *prefix = get_prefix(sym);
      if (sym.xrisnotnull()) {
	 symbolPtr dfs = sym->get_def_file();
	 if (dfs.xrisnotnull()) {
	    ents[i].printf("{%s%s [%s]} ", prefix, sym.get_name(), dfs.get_name());
	 }
	 else ents[i].printf("{%s%s} ", prefix, sym.get_name());
      }
      else ents[i].printf("{%s%s} ", prefix, sym.get_name());
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
//========================================

ModelAliasList::ModelAliasList (const genString names_and_files[], size_t sz) {
   Initialize(ModelAliasList::ModelAliasList);

   answer = 0;
   vstr *Aliaslist = vstrCloneScribed(vcharScribeLiteral(names_and_files[0].str()));
   for(int i = 1; i < sz; i++)
      Aliaslist = vstrAppendScribed(Aliaslist, vcharScribeLiteral(names_and_files[i].str()));

   genString command;

   command.printf ("dis_setvar -global MODELALIAS_value {%s}; set returnValue [dis_prompt {ModelAliasDialog} {%s}]; if { $returnValue == \"Yes\" } then { return {1}} else {return {-1}}",
           (vchar *)Aliaslist, "Aliased Entities");
   vchar *retValue = (vchar *)rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
   answer = atoi((char *)retValue);
}


//========================================
// ModelAliasList::~ModelAliasList
//========================================

ModelAliasList::~ModelAliasList () {
   Initialize(ModelAliasList::~ModelAliasList);
}


//========================================
// ModelAliasList::ask
//========================================

int ModelAliasList::ask () {
   Initialize(ModelAliasList::ask);

   return answer;
}
