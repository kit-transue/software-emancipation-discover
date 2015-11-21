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
#include "QueryResult.h"

string CQueryResult::m_RecordDelim = "\n";
string CQueryResult::m_FieldDelim = "\t";

CQueryResult::CQueryResult() {
        m_pRecords = new TRecordSet();
}

CQueryResult::~CQueryResult() {
        TRecordSetIterator recordsIter = m_pRecords->begin();
        while(recordsIter!=m_pRecords->end()) {
                TRecord* pRecord = *recordsIter;
                TFieldsIterator fieldsIterator = pRecord->begin();      
                while(fieldsIterator!=pRecord->end()) {
                        TField* field = *fieldsIterator;
                        delete field;
                        fieldsIterator++;
                }
                delete pRecord;
                recordsIter++;
        }
        delete m_pRecords;
}

TRecord* CQueryResult::getRecord(int idx) {
        TRecordSetIterator recordsIter = m_pRecords->begin();
        while(recordsIter!=m_pRecords->end() && idx>0) {
                recordsIter++;
                idx--;
        }
        if(recordsIter==m_pRecords->end()) return NULL;
        return *recordsIter;
}

bool CQueryResult::contains(string& filter, int nField) {
        bool bContains = false;

        TRecordSetIterator recordsIter = m_pRecords->begin();
        while(recordsIter!=m_pRecords->end()) {
                TRecord* pRecord = *recordsIter;
                TField* field = (*pRecord)[nField];
                if(field!=NULL && field->compare(filter)==0) {
                        bContains = true;
                        break;
                }
                recordsIter++;
        }
        return bContains;
}

TRecordSetIterator CQueryResult::getStartRecord() {
        return m_pRecords->begin();
}

TRecordSetIterator CQueryResult::getEndRecord() {
        return m_pRecords->end();
}

int CQueryResult::getRecordsCount() {
        return m_pRecords->size();
}

int CQueryResult::getFieldsCount() {
        return ((TRecord*)(m_pRecords->front()))->size();
}

void CQueryResult::remove(TRecord* record) {
        m_pRecords->remove(record);
}

void CQueryResult::add(TRecord* record) {
        m_pRecords->push_back(record);
}

string CQueryResult::cleanUpResult(string &result, string remove) {
	if (result.size() <= 2) {
		return string("");
	}
	string cleanString(" ");
	int removeLength = remove.size();
	
	int firstPos = result.find(remove, 0);
	if (firstPos == -1) {
		return cleanString.append(result.substr(2, result.size()));
	}
	int nextPos = result.find(remove, firstPos + 1);
	while (nextPos != -1) {
		cleanString.append(result.substr(
					firstPos + removeLength, 
					nextPos - (firstPos + removeLength)));
		firstPos = nextPos;
		nextPos = result.find(remove, nextPos+1);
	} 			
	cleanString.append(result.substr(firstPos + removeLength));
	return cleanString;
}

bool CQueryResult::parse(string& results) {
        string::size_type recordStartIdx = 0;
        string::size_type recordEndIdx = 0;
        string SpaceDelim = " ";
        
        int nResultLength = results.length();
        string record;
        do {
                recordEndIdx = results.find(m_RecordDelim,recordStartIdx);
                if(recordEndIdx==string::npos) 
                        record = results.substr(recordStartIdx);
                else
                        record = results.substr(recordStartIdx,recordEndIdx-recordStartIdx);
                if(record.length()>0) {
                        string::size_type fieldStartIdx = 0;
                        string::size_type fieldEndIdx = 0;
                        
                        string field;

                        int nRecLen = record.length();
                        TRecord* pRecord = new TRecord();
                        add(pRecord);
                        string* pDelim = &SpaceDelim;
                        do {
                                fieldEndIdx = record.find(*pDelim,fieldStartIdx);
                                if(pDelim==&SpaceDelim) pDelim = &m_FieldDelim;
                                if(fieldEndIdx==string::npos) 
                                        field = record.substr(fieldStartIdx);
                                else
                                        field = record.substr(fieldStartIdx,fieldEndIdx-fieldStartIdx);
                                
                                pRecord->push_back(new TField(field));

                                fieldStartIdx = fieldEndIdx;
                                if(fieldStartIdx!=string::npos)
                                        fieldStartIdx++;
                        }while(fieldStartIdx!=string::npos);
                }
                recordStartIdx = recordEndIdx;
                if(recordStartIdx!=string::npos)
                        recordStartIdx++;
        } while(recordStartIdx!=string::npos && recordStartIdx<nResultLength);
        return true;
}

void CQueryResult::print(ostream& os) {
        string fieldDelim = "\t";
        TRecordSetIterator recordsIter = m_pRecords->begin();
        while(recordsIter!=m_pRecords->end()) {
                TRecord* pRecord = *recordsIter;
                TFieldsIterator fieldsIterator = pRecord->begin();      
                while(fieldsIterator!=pRecord->end()) {
                        TField* field = *fieldsIterator;
                        os << *field;
                        fieldsIterator++;
                        if(fieldsIterator!=pRecord->end())
                                os << m_FieldDelim;
                }
                recordsIter++;
                os << m_RecordDelim;
        }
}

