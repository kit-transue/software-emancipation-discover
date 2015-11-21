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
// Argument.h: interface for the CArgument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARGUMENT_H__4D8ACE24_7668_4346_8421_5F1E69907251__INCLUDED_)
#define AFX_ARGUMENT_H__4D8ACE24_7668_4346_8421_5F1E69907251__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CArgument {
public:
	void SetType(CString& szType);
	void SetName(CString& szName);
	CArgument(CString& type,CString& name) {m_szType = type;m_szName=name;};
	CString& GetType() {return m_szType;};
	CString& GetName() {return m_szName;};

 private:
	CString m_szType;
	CString m_szName;
};

typedef CList<CArgument*,CArgument*> TArgumentsList;
class CArguments : public TArgumentsList {
public:
	virtual ~CArguments();
	Parse(CString& szArgs);
};

#endif // !defined(AFX_ARGUMENT_H__4D8ACE24_7668_4346_8421_5F1E69907251__INCLUDED_)
