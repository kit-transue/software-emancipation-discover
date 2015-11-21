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
// /aset/ui/include/QueryAndFetch.h
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Fri May 19 09:29:45 1995:	Created by William M. Miller
//
//------------------------------------------


#ifndef _QueryAndFetch_h
#define _QueryAndFetch_h

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif
      
#ifndef _gt_h
#include <gt.h>
#endif
      
//========================================
// Class QueryAndFetch
//	Created Fri May 19 09:29:45 1995 by William M. Miller
//----------------------------------------
// Description:
//
//========================================

class QueryAndFetch{
public:
	QueryAndFetch(const symbolArr&);
	~QueryAndFetch();
	int ask();
private:
	static void fetchCB(gtPushButton*, gtEventPtr, void*, gtReason);
	static void cancelCB(gtPushButton*, gtEventPtr, void*, gtReason);
	gtDialogTemplate* shell;
	int answer;
	static int answer_func(void*);
};


#endif



