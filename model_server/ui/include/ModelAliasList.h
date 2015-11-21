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
#ifndef _ModelAliasList_h
#define _ModelAliasList_h

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef NEW_UI
#ifndef _gt_h
#include <gt.h>
#endif
#endif

bool popup_ModelAliasList(const symbolArr&);

//========================================
// Class ModelAliasList
//	Created Tue Aug 27 15:12:56 1996 by William M. Miller
//----------------------------------------
// Description:
//
// Show the user a list of entities that will be aliased together for
// dormant code analysis and subsystem extraction and let him/her pull
// the plug if he/she doesn't like it.
//========================================

class ModelAliasList {
public:
   ModelAliasList(const genString names_and_files[], size_t count);
   ~ModelAliasList();
   int ask();
private:
#ifndef NEW_UI
   static void OK_CB(gtPushButton*, gtEventPtr, void*, gtReason);
   static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);
   gtDialogTemplate* shell;
   static int answer_func(void*);
#endif
   int answer;
};


#endif
