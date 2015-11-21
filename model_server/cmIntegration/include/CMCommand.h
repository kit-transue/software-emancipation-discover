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
#ifndef __CMCOMMAND_HEADER_FILE
#define __CMCOMMAND_HEADER_FILE

#include "stl.h"

class CCMAttribute {
public:
	static const int TEXT; 
	static const int MULTILINE_TEXT;
	static const int LIST;

	CCMAttribute(string szAttr);
	bool IsBoolean();
	bool IsExecutable();
	int GetType() { return m_nType; };
	string& GetName() { return m_szName; };
	string& GetKey() { return m_szKey; };
	int SetType(string& szType);
	void SetValue(string& szValue);
	string& GetValue() { return m_szValue; };
	string GetListElementValue(int nIdx);
	string GetListElementValue(string szKey);
	string GetListElementKey(int nIdx);
	string GetListElementKey(string szVal);
	int GetListElementsCount();
	bool AskFor() { return m_bAskFor; };
	void SetAsk(bool state) { m_bAskFor = state; };
private:
	int m_nType;
	string m_szValue;
	string m_szName;
	string m_szKey;
	string m_szValuesList;
	bool m_bAskFor;
};

class CCMAttributes : public map<string*,CCMAttribute*> {
public:
	CCMAttributes(string szAttrs);
	virtual ~CCMAttributes();
};

class CCMAttributesList : public list<CCMAttribute*> {
};


typedef list<string> TStringList; 

class CCMCommand
{
public:
	CCMCommand(string szCmdDesc);
	virtual ~CCMCommand();
	string& GetName() { return m_szName; };
	string& GetKey() { return m_szKey; };
	bool HasAttribute(string& szAttr);
	int GetAttributesCount();
	string GetAttribute(int nIdx);
	bool Returns() { return m_bReturns; };
private:
	string m_szKey;
	string m_szName;
	TStringList* m_pAttributes;
	bool m_bReturns;
};

class CCMCommandsList : public list<CCMCommand*> {
public:
	CCMCommandsList(string szCmds);
	virtual ~CCMCommandsList();
	CCMCommand* GetCommand(int nIdx);
	CCMCommand* GetCommand(const char* szName);
	CCMCommand* GetCommandByKey(const char* szKey);
};

#endif //__CMCOMMAND_HEADER_FILE
