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
#ifndef __QUERYRESULT_HEADER_FILE
#define __QUERYRESULT_HEADER_FILE

#if defined(hp10) || defined(irix6)
   #include <iostream.h>
#else
   #include <ostream>
#endif

#include "stl.h"

typedef string TField;

typedef vector < TField* > TRecord;
typedef TRecord::iterator TFieldsIterator;

typedef list < TRecord* > TRecordSet;
typedef TRecordSet::iterator TRecordSetIterator;

class CQueryResult {
public:
	static string m_RecordDelim;
	static string m_FieldDelim;

	CQueryResult();
	~CQueryResult();
	bool parse(string& results);
	string cleanUpResult(string& results, string remove);
	int getRecordsCount();
	int getFieldsCount();
	TRecordSetIterator getStartRecord();
	TRecordSetIterator getEndRecord();
	void print(ostream& os);
	bool contains(string& filter, int nField);
	TRecord* getRecord(int idx);
	void remove(TRecord* record);
	void add(TRecord* record);
private:
	TRecordSet* m_pRecords;
};
#endif //__QUERYRESULT_HEADER_FILE
