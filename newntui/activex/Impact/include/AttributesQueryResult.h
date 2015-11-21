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
// AttributesAttributesQueryResult.h: interface for the CAttributesAttributesQueryResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATTRIBUTESAttributesQueryResult_H__09542058_658A_49E1_BB4A_E8D864648120__INCLUDED_)
#define AFX_ATTRIBUTESAttributesQueryResult_H__09542058_658A_49E1_BB4A_E8D864648120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef CString TField;

class TRecord : public CArray < TField*,TField* > {
public:
	~TRecord() {
		int nCount = GetSize();
		for(int fieldsIterator=0;fieldsIterator<nCount;fieldsIterator++) {
			TField* field = (*this)[fieldsIterator];
			delete field;
		}
	}
};

typedef CList < TRecord*,TRecord* > TRecordSet;

class CAttributesQueryResult {
public:
	POSITION find(CString& key,int field);
	static TCHAR m_RecordDelim;
	static TCHAR m_FieldDelim;

	CAttributesQueryResult();
	~CAttributesQueryResult();
	bool parse(CString& results);
	int getRecordsCount();
	int getFieldsCount();
	POSITION getStartPos();
	TRecord* get(POSITION pos);
	TRecord* getNext(POSITION& pos);
	bool contains(CString& filter, int nField);
	TRecord* getRecord(int idx);
	void remove(TRecord* record);
	void remove(CString& filter, int nField);
	void add(TRecord* record);
	void addHead(TRecord* record);
private:
	TRecordSet* m_pRecords; 
};

#endif // !defined(AFX_ATTRIBUTESAttributesQueryResult_H__09542058_658A_49E1_BB4A_E8D864648120__INCLUDED_)
