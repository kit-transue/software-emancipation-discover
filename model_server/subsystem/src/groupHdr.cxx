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
// /aset/subsystem/src/groupHdr.C
//------------------------------------------
// description:
// 
// Implementation of class groupHdr.
//------------------------------------------
// History:
//
// Tue Apr 11 16:29:45 1995:	Created by William M. Miller
//
//------------------------------------------
#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>


#include <groupHdr.h>
#include <group.h>
#include <OODT_apl_entries.h>
#include <Question.h>
#include <gtPushButton.h>
#include <subsys.h>
#include <groupTree.h>
#include <path.h>
#include <path1.h>
#include <systemMessages.h>
#include <customize.h>
#include <projectBrowser.h>
#include <xref.h>
#include <dd_or_xref_node.h>
#include <RTL_externs.h>
#include <loadedFiles.h>
#include <save_app_dialog.h>
#include <Question.h>
#include <popup_QueryAndFetch.h>
#include <messages.h>

#ifndef _WIN32
#include <dirent.h>
#else
#undef INT
#undef CHAR
#include <windows.h>
#include <io.h>
#include <stdlib.h>
#endif


