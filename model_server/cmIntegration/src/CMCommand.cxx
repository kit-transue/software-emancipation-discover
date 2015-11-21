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
#include "CMCommand.h"

CCMCommand::CCMCommand(string szDesc)
{
	string szTrimTemplate = "\n\t\" ";
	m_pAttributes = NULL;
	m_bReturns = false;
	int nStart = 0;
	int nEnd = 0;
	string szAttrRec;
	while(nStart >= 0) {
		nEnd = szDesc.find('\t',nStart);
		if(nEnd > 0)
			szAttrRec = szDesc.substr(nStart,nEnd-nStart);
		else 
			szAttrRec = szDesc.substr(nStart);
		int nDelim = szAttrRec.find('=',0);
		if(nDelim>0) {
			string szKey = szAttrRec.substr(0,nDelim);			
			string szValue = szAttrRec.substr(nDelim+1);
			
			if(szKey.compare("key")==0) m_szKey = szValue.substr(1,szValue.length()-2);
			else if(szKey.compare("display")==0) m_szName = szValue.substr(1,szValue.length()-2);
			else if(szKey.compare("return")==0) m_bReturns = true;
			else {
				if(m_pAttributes==NULL)	m_pAttributes = new TStringList();
				m_pAttributes->push_back(szValue.substr(1,szValue.length()-2));
			}
		}
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

string CCMCommand::GetAttribute(int nIdx) {
	string szValue;
	string szKey;
	if(m_pAttributes) {
		TStringList::iterator pos = m_pAttributes->begin();
		TStringList::iterator end = m_pAttributes->end();
		while(pos!=end && nIdx>=0) {
			if(!nIdx) {
				szValue = *pos;
				break;
			}
			nIdx--;
			pos++;
		}
	}
	return szValue;
}

int  CCMCommand::GetAttributesCount() {
	if(m_pAttributes) return m_pAttributes->size();
	return 0;
}

bool CCMCommand::HasAttribute(string& szAttr) {
	if(m_pAttributes) {
		TStringList::iterator pos = m_pAttributes->begin();
		TStringList::iterator end = m_pAttributes->end();
		while(pos!=end) {
			if(szAttr.compare(*pos)==0) return true;
			pos++;
		}
	}
	return false;
}

CCMCommand::~CCMCommand()
{
	if(m_pAttributes) delete m_pAttributes;
}

CCMCommandsList::CCMCommandsList(string szCmds) {
	int nStart = 0;
	int nEnd = 0;
	CCMCommand* pCommand = NULL;
	while(nStart >= 0) {
		nEnd = szCmds.find('\n',nStart);
		if(nEnd > 0)
			pCommand = new CCMCommand(szCmds.substr(nStart,nEnd-nStart));
		else 
			pCommand = new CCMCommand(szCmds.substr(nStart));
		push_back(pCommand);
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

CCMCommandsList::~CCMCommandsList() {
	CCMCommandsList::iterator pos = begin();
	CCMCommandsList::iterator iEnd = end();
	
	while(pos!=iEnd) {
		CCMCommand* pCommand = *pos;
		delete pCommand;
		pos++;
	}
}

CCMCommand* CCMCommandsList::GetCommand(int nIdx) {	
	CCMCommandsList::iterator pos = begin();
	CCMCommandsList::iterator iEnd = end();
	while(pos!=iEnd && nIdx>=0) {
		if(!nIdx) return *pos;
		nIdx--;
		pos++;
	}
	return NULL;
}

CCMCommand* CCMCommandsList::GetCommand(const char* szName) {
	CCMCommandsList::iterator pos = begin();
	CCMCommandsList::iterator iEnd = end();
	while(pos!=iEnd) {
		CCMCommand* pCommand = *pos;
		if(pCommand->GetName().compare(szName)==0) return *pos;
		pos++;
	}
	return NULL;
}

CCMCommand* CCMCommandsList::GetCommandByKey(const char* szKey) {
	CCMCommandsList::iterator pos = begin();
	CCMCommandsList::iterator iEnd = end();
	while(pos!=iEnd) {
		CCMCommand* pCommand = *pos;
		if(pCommand->GetKey().compare(szKey)==0) return *pos;
		pos++;
	}
	return NULL;
}

CCMAttributes::CCMAttributes(string szAttrs) {
	int nStart = 0;
	int nEnd = 0;
	CCMAttribute* pAttribute = NULL;
	while(nStart >= 0) {
		nEnd = szAttrs.find('\n',nStart);
		if(nEnd>0)
			pAttribute = new CCMAttribute(szAttrs.substr(nStart,nEnd-nStart));
		else 
			pAttribute = new CCMAttribute(szAttrs.substr(nStart));
		(*this)[new string(pAttribute->GetKey().c_str())]=pAttribute;
		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

CCMAttributes::~CCMAttributes() {
	CCMAttributes::iterator pos = begin();
	CCMAttributes::iterator iEnd = end();
	CCMAttribute* pAttribute;
	
	while(pos!=iEnd) {
		delete (*pos).first;
		delete (*pos).second;
		pos++;
	}
}

const int CCMAttribute::TEXT = 0; 
const int CCMAttribute::MULTILINE_TEXT = 1;
const int CCMAttribute::LIST = 2;

CCMAttribute::CCMAttribute(string szAttributes) : m_bAskFor(false) {
	string szTrimTemplate = "\n\t\" ";
	string szAttr;
	string szKey;
	string szValue;
	
	string cKeyAttr("key");
	string cDisplayAttr("display");
	string cTypeAttr("type");
	string cValueAttr("value");
	string cAskAttr("ask");

	int nStart = 0;
	int nEnd = 0;
	int nKeyEnd= 0;

	while(nStart >= 0) {
		nEnd = szAttributes.find('\t',nStart);
		if(nEnd>0)
			szAttr = szAttributes.substr(nStart,nEnd-nStart);
		else 
			szAttr = szAttributes.substr(nStart);
		nKeyEnd = szAttr.find('=');
		szKey = szAttr.substr(0,nKeyEnd);
		szValue = szAttr.substr(nKeyEnd+1);
		
		if(szKey.compare(cKeyAttr)==0) m_szKey = szValue.substr(1,szValue.length()-2);
		else if(szKey.compare(cDisplayAttr)==0) m_szName = szValue.substr(1,szValue.length()-2);
		else if(szKey.compare(cTypeAttr)==0) SetType(szValue.substr(1,szValue.length()-2));
		else if(szKey.compare(cValueAttr)==0) SetValue(szValue.substr(1,szValue.length()-2));
		else if(szKey.compare(cAskAttr)==0) SetAsk(true);

		nStart = nEnd;
		if(nStart>0) nStart++;
	}
}

void CCMAttribute::SetValue(string& szValue) {
	m_szValue = szValue;
}

int CCMAttribute::SetType(string& szType) {
	if(szType.compare("text")==0) m_nType = TEXT;
	else if(szType.compare("multiline_text")==0) m_nType = MULTILINE_TEXT;
	else if(szType[0]=='[') {
		m_nType = LIST;
		m_szValuesList = szType.substr(1,szType.length()-2);
	}
	return m_nType;
}

bool CCMAttribute::IsBoolean() {
	if(GetType()==LIST && GetListElementsCount()==2) {
		string szFirst = GetListElementValue(0);
		string szSecond = GetListElementValue(1);
		if((szFirst.compare("false")==0 || szFirst.compare("true")==0) &&
		   (szSecond.compare("false")==0 || szSecond.compare("true")==0))
			return true;
	}
	return false;
}

string CCMAttribute::GetListElementValue(int nIdx) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		string szPair;
		while(nStart >= 0 && nIdx>=0) {
			nEnd = m_szValuesList.find(';',nStart);
			if(nIdx==0) {
				if(nEnd>0)
					szPair = m_szValuesList.substr(nStart,nEnd-nStart);
				else 
					szPair = m_szValuesList.substr(nStart);
				int nDelim = szPair.find(',');
				if(nDelim>0)
					return szPair.substr(nDelim+1);
				return szPair;
			}
			nIdx--;
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

string CCMAttribute::GetListElementValue(string key) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		string szKey;
		while(nStart >= 0) {
			nEnd = m_szValuesList.find(';',nStart);
			if(nEnd>0)
				szKey = m_szValuesList.substr(nStart,nEnd-nStart);
			else 
				szKey = m_szValuesList.substr(nStart);
			int nDelim = szKey.find(',');
			if(nDelim>0) {
				string szValue = szKey.substr(nDelim+1);
				szKey = szKey.substr(0,nDelim);
				if(szKey.compare(key)==0)
					return szValue;
			}
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

int CCMAttribute::GetListElementsCount() {
	if(m_szValuesList.length()==0 || m_nType!=LIST) return 0;
	int nCount = 0;		
	int nDelim = -1;
	while((nDelim = m_szValuesList.find(';',nDelim+1)) > 0) nCount++;
	return nCount+1;
}

string CCMAttribute::GetListElementKey(int nIdx) {
	if((m_nType==TEXT || m_nType==MULTILINE_TEXT) && nIdx==0)	return m_szValue;
	else if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		string szPair;
		while(nStart >= 0 && nIdx>=0) {
			nEnd = m_szValuesList.find(';',nStart);
			if(nIdx==0) {
				if(nEnd>0)
					szPair = m_szValuesList.substr(nStart,nEnd-nStart);
				else 
					szPair = m_szValuesList.substr(nStart);
				int nDelim = szPair.find(',');
				if(nDelim>0)
					return szPair.substr(0,nDelim);
				return szPair;
			}
			nIdx--;
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

string CCMAttribute::GetListElementKey(string value) {
	if(m_nType==LIST) {
		int nStart = 0;
		int nEnd = 0;
		string szKey;
		while(nStart >= 0) {
			nEnd = m_szValuesList.find(';',nStart);
			if(nEnd>0)
				szKey = m_szValuesList.substr(nStart,nEnd-nStart);
			else 
				szKey = m_szValuesList.substr(nStart);
			int nDelim = szKey.find(',');
			if(nDelim>0) {
				string szValue = szKey.substr(nDelim+1);
				szKey = szKey.substr(0,nDelim);
				if(szValue.compare(value)==0)
					return szKey;
			}
			nStart = nEnd;
			if(nStart>0) nStart++;
		}
	}
	return "";
}

