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
//**********
//
// ui_cm_put.h - Configuration Management Put Operation User Interface class.
//
// class ui_cm_put
//
//**********

#ifndef _ui_cm_put_h
#define _ui_cm_put_h



class projModule;
class RTListServer;




// Base UI CM Operation Class
class ui_cm_put : public ui_cm_oper {

public:
	ui_cm_put(const char* oper_name, RTListServer* fileRTL);
	virtual ~ui_cm_put();

	static bool PutAlreadyInProgress;
	static symbolArr FailedModules;
	static symbolArr LockedFailedModules;
	static symbolArr MergeFailedModules;
	static int GetFailureType(const symbolArr& syms, genString& failureType);

	// When the put fails, we need to save the original RTL 
	// so that it can be used later if the user does an unlock or unget.
	static RTListServer* HomeProjectRTL;

	virtual bool CheckPreConditions(const symbolArr& file_syms);

protected:
	virtual bool NeedsMoreInfo(projModule*);
	virtual bool GetMoreInfo(projModule*);
	virtual bool OperateOnModule(projModule*);
	virtual bool ProcessFailedModules(const symbolArr& failures);

	bool needs_comment_string_;
	bool has_comment_string_;
	genString comment_string_;

private:
};





#endif    // _ui_cm_put_h
