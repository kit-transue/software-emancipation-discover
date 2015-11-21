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
// AttributesAttributesQueryResult.cpp: implementation of the CAttributesAttributesQueryResult class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "AttributesQueryResult.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

TCHAR CAttributesQueryResult::m_RecordDelim = _T('\n');
TCHAR CAttributesQueryResult::m_FieldDelim = _T('\t');

CAttributesQueryResult::CAttributesQueryResult() {
        m_pRecords = new TRecordSet();
}

CAttributesQueryResult::~CAttributesQueryResult() {
	POSITION recordsIter = m_pRecords->GetHeadPosition();
	while(recordsIter!=NULL) {
		TRecord* pRecord = m_pRecords->GetNext(recordsIter);
		delete pRecord;
	}
	delete m_pRecords;
}

TRecord* CAttributesQueryResult::getNext(POSITION& pos) {
	return  m_pRecords->GetNext(pos);
}

TRecord* CAttributesQueryResult::get(POSITION pos) {
	return  m_pRecords->GetAt(pos);
}

TRecord* CAttributesQueryResult::getRecord(int idx) {
    POSITION recordsIter = m_pRecords->GetHeadPosition();
    while(recordsIter!=NULL && idx>0) {
        m_pRecords->GetNext(recordsIter);
        idx--;
    }
    if(recordsIter==NULL) return NULL;
    return m_pRecords->GetAt(recordsIter);
}

bool CAttributesQueryResult::contains(CString& filter, int nField) {
    bool bContains = find(filter,nField)!=NULL;
    return bContains;
}

POSITION CAttributesQueryResult::getStartPos() {
    return m_pRecords->GetHeadPosition();
}

int CAttributesQueryResult::getRecordsCount() {
    return m_pRecords->GetCount();
}

int CAttributesQueryResult::getFieldsCount() {
    return ((TRecord*)(m_pRecords->GetHead()))->GetSize();
}

void CAttributesQueryResult::remove(CString& filter, int nField) {
    bool bContains = false;

    POSITION recordsIter = m_pRecords->GetHeadPosition();
    while(recordsIter!=NULL) {
        TRecord* pRecord = m_pRecords->GetAt(recordsIter);
        TField* field = (*pRecord)[nField];
        if(field!=NULL && field->CompareNoCase(filter)==0) {
			m_pRecords->RemoveAt(recordsIter);
            break;
        }
		m_pRecords->GetNext(recordsIter);
    }
}

void CAttributesQueryResult::remove(TRecord* record) {
    POSITION pos = m_pRecords->Find(record);
	if(pos != NULL)	m_pRecords->RemoveAt(pos);
}

void CAttributesQueryResult::add(TRecord* record) {
    m_pRecords->AddTail(record);
}

void CAttributesQueryResult::addHead(TRecord* record) {
	m_pRecords->AddHead(record);
}

bool CAttributesQueryResult::parse(CString& results) {
    int recordStartIdx = 0;
    int recordEndIdx = 0;
    
    int nResultLength = results.GetLength();
    CString record;
    do {
		recordEndIdx = results.Find(m_RecordDelim,recordStartIdx);
		if(recordEndIdx==-1) 
				record = results.Mid(recordStartIdx);
		else
				record = results.Mid(recordStartIdx,recordEndIdx-recordStartIdx);
		if(record.GetLength()>0) {
			int fieldStartIdx = 0;
			int fieldEndIdx = 0;
                        
            CString field;

            int nRecLen = record.GetLength();
            TRecord* pRecord = new TRecord();
            add(pRecord);
			do {
				fieldEndIdx = record.Find(m_FieldDelim,fieldStartIdx);
				if(fieldEndIdx==-1) 
					field = record.Mid(fieldStartIdx);
				else
					field = record.Mid(fieldStartIdx,fieldEndIdx-fieldStartIdx);
                                
                pRecord->Add(new TField(field));

                fieldStartIdx = fieldEndIdx;
                if(fieldStartIdx!=-1)
					fieldStartIdx++;
            }while(fieldStartIdx!=-1);
        }
		recordStartIdx = recordEndIdx;
		if(recordStartIdx!=-1)
				recordStartIdx++;
    } while(recordStartIdx!=-1 && recordStartIdx<nResultLength);
    return true;
}

POSITION CAttributesQueryResult::find(CString &key, int nField)
{
    POSITION recordsIter = m_pRecords->GetHeadPosition();
    while(recordsIter!=NULL) {
        TRecord* pRecord = m_pRecords->GetAt(recordsIter);
        TField* field = (*pRecord)[nField];
        if(field!=NULL && field->CompareNoCase(key)==0) {
            return recordsIter;
        }
		m_pRecords->GetNext(recordsIter);
    }
	return NULL;
}
