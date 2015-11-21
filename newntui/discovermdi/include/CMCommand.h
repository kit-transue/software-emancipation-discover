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
// CMCommand.h: interface for the CCMCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CMCOMMAND_H__701126B4_87D1_4136_B411_C818D8BD1B0F__INCLUDED_)
#define AFX_CMCOMMAND_H__701126B4_87D1_4136_B411_C818D8BD1B0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CCMAttribute {
public:
	static const int TEXT; 
	static const int MULTILINE_TEXT;
	static const int LIST;

	CCMAttribute(CString szAttr);
	bool IsBoolean();
	bool IsExecutable();
	int GetType() { return m_nType; };
	CString& GetName() { return m_szName; };
	CString& GetKey() { return m_szKey; };
	int SetType(CString& szType);
	void SetValue(CString& szValue);
	CString& GetValue() { return m_szValue; };
	CString GetListElementValue(int nIdx);
	CString GetListElementValue(CString szKey);
	CString GetListElementKey(int nIdx);
	CString GetListElementKey(CString szVal);
	int GetListElementsCount();
	bool AskFor() { return m_bAskFor; };
	void SetAsk(bool state) { m_bAskFor = state; };
private:
	int m_nType;
	CString m_szValue;
	CString m_szName;
	CString m_szKey;
	CString m_szValuesList;
	bool m_bAskFor;
};

class CCMAttributes : public CTypedPtrMap<CMapStringToPtr,CString,CCMAttribute*> {
public:
	CCMAttributes(CString szAttrs);
	virtual ~CCMAttributes();
};

class CCMAttributesList : public CTypedPtrList<CPtrList,CCMAttribute*> {
};

class CCMCommand
{
public:
	CCMCommand(CString szCmdDesc);
	virtual ~CCMCommand();
	CString& GetName() { return m_szName; };
	CString& GetKey() { return m_szKey; };
	bool HasAttribute(CString& szAttr);
	int GetAttributesCount();
	CString GetAttribute(int nIdx);
	bool Returns() { return m_bReturns; };
private:
	CString m_szKey;
	CString m_szName;
	CStringList* m_pAttributes;
	bool m_bReturns;
};

class CCMCommandsList : public CTypedPtrList<CPtrList, CCMCommand*> {
public:
	CCMCommandsList(CString szCmds);
	virtual ~CCMCommandsList();
	CCMCommand* GetCommand(int nIdx);
	CCMCommand* GetCommandByName(CString& szName);
	CCMCommand* GetCommandByKey(CString& szKey);
};

#endif // !defined(AFX_CMCOMMAND_H__701126B4_87D1_4136_B411_C818D8BD1B0F__INCLUDED_)
