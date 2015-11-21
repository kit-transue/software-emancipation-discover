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
// FunctionAccessJava.cpp: implementation of the CFunctionAccessJava class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "FunctionAccessJava.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int CFunctionAccessJava::PUBLIC = 0;
const int CFunctionAccessJava::PROTECTED = 1;
const int CFunctionAccessJava::PACKAGE = 2;
const int CFunctionAccessJava::PRIVATE = 3;
const int CFunctionAccessJava::ABSTRACT = 5;
const int CFunctionAccessJava::FINAL = 6;
const int CFunctionAccessJava::NATIVE = 7;
const int CFunctionAccessJava::STATIC = 8;
const int CFunctionAccessJava::NONE = 0; 

CFunctionAccessJava::CFunctionAccessJava()
{
	m_nPermission = NONE;
}

CFunctionAccessJava::~CFunctionAccessJava()
{
}


CString CFunctionAccessJava::GetAccessString()
{
	switch(m_nPermission) {
		case PUBLIC: return "Public";
		case PRIVATE: return "Private";
		case PACKAGE: return "";
		case PROTECTED: return "Protected";
		default: return "NONE";
	}	
}

int CFunctionAccessJava::GetAccess()
{
	return m_nPermission;
}

