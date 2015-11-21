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
// CMCommand.cpp: implementation of the CCMCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "discovermdi.h"
#include "CMCommand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCMCommand::CCMCommand(CString szDesc)
{
	CString szTrimTemplate = "\n\t\" ";
	m_pAttributes = NULL;
	m_bReturns = false;
	int nStart = 0;
	int nEnd = 0;
	CString szAttrRec;
	while(nStart >= 0) {
		nEnd = szDesc.Find('\t',nStart);
		if(nEnd > 0)
			szAttrRec = szDesc.Mid(nStart,nEnd-nStart);
		else 
			szAttrRec = szDesc.Mid(nStart);
		int nDelim = szAttrRec.Find('=',0);
		if(nDelim>0) {
			CString szKey = szAttrRec.Mid(0,nDelim);
			
			CString szValue = szAttrRec.Mid(nDelim+1);
			szValue.TrimLeft(szTrimTemplate);
			szValue.TrimRight(szTrimTemplate);
			
			if(szKey.CompareNoCase("key")==0) m_szKey = szValue;
			else if(szKey.CompareNoCase("display")==0) m_szName = szValue;
			else if(szKey.CompareNoCase("return")==0) m_bReturns = true;
			else {
				if(m_pAttributes==NULL)	m_pAttributes = new CStringList();
				m_pAttributes->AddTail(szValue);
			}
		}
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

CString CCMCommand::GetAttribute(int nIdx) {
	CString szValue;
	CString szKey;
	if(m_pAttributes) {
		CString szTmp;
		POSITION pos = m_pAttributes->GetHeadPosition();
		while(pos!=NULL && nIdx>=0) {
			szTmp = m_pAttributes->GetNext(pos);
			if(!nIdx) {
				szValue = szTmp;
				break;
			}
			nIdx--;
		}
	}
	return szValue;
}

int  CCMCommand::GetAttributesCount() {
	if(m_pAttributes) return m_pAttributes->GetCount();
	return 0;
}

bool CCMCommand::HasAttribute(CString& szAttr) {
	if(m_pAttributes)
		return m_pAttributes->Find(szAttr)!=NULL;
	return false;
}

CCMCommand::~CCMCommand()
{
	if(m_pAttributes) delete m_pAttributes;
}

CCMCommandsList::CCMCommandsList(CString szCmds) {
	int nStart = 0;
	int nEnd = 0;
	CCMCommand* pCommand = NULL;
	while(nStart >= 0) {
		nEnd = szCmds.Find('\n',nStart);
		if(nEnd > 0)
			pCommand = new CCMCommand(szCmds.Mid(nStart,nEnd-nStart));
		else 
			pCommand = new CCMCommand(szCmds.Mid(nStart));
		AddTail(pCommand);
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

CCMCommandsList::~CCMCommandsList() {
	POSITION pos = GetHeadPosition();
	while(pos!=NULL) {
		CCMCommand* pCommand = GetNext(pos);
		delete pCommand;
	}
}

CCMCommand* CCMCommandsList::GetCommand(int nIdx) {	
	POSITION pos = GetHeadPosition();
	while(pos!=NULL && nIdx>=0) {
		CCMCommand* pCommand = GetNext(pos);
		if(!nIdx) return pCommand;
		nIdx--;
	}
	return NULL;
}

CCMCommand* CCMCommandsList::GetCommandByName(CString& szName) {
	POSITION pos = GetHeadPosition();
	while(pos!=NULL) {
		CCMCommand* pCommand = GetNext(pos);
		if(szName.Compare(pCommand->GetName()) == 0) return pCommand;
	}
	return NULL;
}

CCMCommand* CCMCommandsList::GetCommandByKey(CString& szKey) {
	POSITION pos = GetHeadPosition();
	while(pos!=NULL) {
		CCMCommand* pCommand = GetNext(pos);
		if(szKey.Compare(pCommand->GetKey()) == 0) return pCommand;
	}
	return NULL;
}

CCMAttributes::CCMAttributes(CString szAttrs) {
	int nStart = 0;
	int nEnd = 0;
	CCMAttribute* pAttribute = NULL;
	while(nStart >= 0) {
		nEnd = szAttrs.Find('\n',nStart);
		if(nEnd>0)
			pAttribute = new CCMAttribute(szAttrs.Mid(nStart,nEnd-nStart));
		else 
			pAttribute = new CCMAttribute(szAttrs.Mid(nStart));
		(*this)[pAttribute->GetKey()]=pAttribute;
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

CCMAttributes::~CCMAttributes() {
	POSITION pos = GetStartPosition();
	CString key;
	CCMAttribute* pAttribute;

	while(pos!=NULL) {
		GetNextAssoc( pos, key, (CCMAttribute*&)pAttribute);
		delete pAttribute;
	}
}

const int CCMAttribute::TEXT = 0; 
const int CCMAttribute::MULTILINE_TEXT = 1;
const int CCMAttribute::LIST = 2;

CCMAttribute::CCMAttribute(CString szAttributes) : m_bAskFor(false) {
	CString szTrimTemplate = "\n\t\" ";
	CString szAttr;
	CString szKey;
	CString szValue;
	int nStart = 0;
	int nEnd = 0;
	int nKeyEnd= 0;

	while(nStart >= 0) {
		nEnd = szAttributes.Find('\t',nStart);
		if(nEnd>0)
			szAttr = szAttributes.Mid(nStart,nEnd-nStart);
		else 
			szAttr = szAttributes.Mid(nStart);
		nKeyEnd = szAttr.Find('=');
		szKey = szAttr.Mid(0,nKeyEnd);
		szValue = szAttr.Mid(nKeyEnd+1);
		szValue.TrimLeft(szTrimTemplate);
		szValue.TrimRight(szTrimTemplate);
		
		if(szKey=="key") m_szKey = szValue;
		else if(szKey=="display") m_szName = szValue;
		else if(szKey=="type") SetType(szValue);
		else if(szKey=="value") SetValue(szValue);
		else if(szKey=="ask") SetAsk(true);

		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

void CCMAttribute::SetValue(CString& szValue) {
	m_szValue = szValue;
}

int CCMAttribute::SetType(CString& szType) {
	if(szType.CollateNoCase("TEXT")==0) m_nType = TEXT;
	else if(szType.CollateNoCase("MULTILINE_TEXT")==0) m_nType = MULTILINE_TEXT;
	else if(szType.GetLength()>0 && szType[0]=='[') {
		m_nType = LIST;
		m_szValuesList = szType;
		m_szValuesList.TrimLeft('[');
		m_szValuesList.TrimRight(']');
	}
	return m_nType;
}

bool CCMAttribute::IsBoolean() {
	if(GetType()==LIST && GetListElementsCount()==2) {
		CString szFirst = GetListElementValue(0);
		CString szSecond = GetListElementValue(1);
		if((szFirst.CompareNoCase("false") == 0 && szSecond.CompareNoCase("true") == 0) ||
		   (szFirst.CompareNoCase("true") == 0 && szSecond.CompareNoCase("false") == 0))
			return true;
	}
	return false;
}

CString CCMAttribute::GetListElementValue(int nIdx) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		CString szPair;
		while(nStart >= 0 && nIdx>=0) {
			nEnd = m_szValuesList.Find(';',nStart);
			if(nIdx==0) {
				if(nEnd>0)
					szPair = m_szValuesList.Mid(nStart,nEnd-nStart);
				else 
					szPair = m_szValuesList.Mid(nStart);
				int nDelim = szPair.Find(',');
				if(nDelim>0)
					return szPair.Mid(nDelim+1);
				return szPair;
			}
			nIdx--;
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

CString CCMAttribute::GetListElementValue(CString key) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		CString szKey;
		while(nStart >= 0) {
			nEnd = m_szValuesList.Find(';',nStart);
			if(nEnd>0)
				szKey = m_szValuesList.Mid(nStart,nEnd-nStart);
			else 
				szKey = m_szValuesList.Mid(nStart);
			int nDelim = szKey.Find(',');
			if(nDelim>0) {
				CString szValue = szKey.Mid(nDelim+1);
				szKey = szKey.Left(nDelim);
				if(szKey.CompareNoCase(key)==0)
					return szValue;
			}
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

int CCMAttribute::GetListElementsCount() {
	int nCount = 0;
	if(m_nType==LIST) {
		int nDelim = -1;
		while((nDelim = m_szValuesList.Find(';',nDelim+1)) > 0) nCount++;
	}
	return nCount+1; // for all other types (not list) this value will be 1, 
					 // because nCount will equal 0
}

CString CCMAttribute::GetListElementKey(int nIdx) {
	if((m_nType==TEXT || m_nType==MULTILINE_TEXT) && nIdx==0)	return m_szValue;
	else if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		CString szPair;
		while(nStart >= 0 && nIdx>=0) {
			nEnd = m_szValuesList.Find(';',nStart);
			if(nIdx==0) {
				if(nEnd>0)
					szPair = m_szValuesList.Mid(nStart,nEnd-nStart);
				else 
					szPair = m_szValuesList.Mid(nStart);
				int nDelim = szPair.Find(',');
				if(nDelim>0)
					return szPair.Left(nDelim);
				return szPair;
			}
			nIdx--;
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

CString CCMAttribute::GetListElementKey(CString value) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		CString szKey;
		while(nStart >= 0) {
			nEnd = m_szValuesList.Find(';',nStart);
			if(nEnd>0)
				szKey = m_szValuesList.Mid(nStart,nEnd-nStart);
			else 
				szKey = m_szValuesList.Mid(nStart);
			int nDelim = szKey.Find(',');
			if(nDelim>0) {
				CString szValue = szKey.Mid(nDelim+1);
				szKey = szKey.Left(nDelim);
				if(szValue.CompareNoCase(value)==0)
					return szKey;
			}
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}
