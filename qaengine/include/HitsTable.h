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
// HitsTable.h: interface for the HitsTable class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#include "Hit.h"
#ifndef _QA_GENERIC_HIT_TABLE_

	typedef vector<Hit *> HitsSet;

	class HitsTable  
	{
	public:
		HitsSet compare(string query, Hit hit);
		Hit* find(int index);
		void append(string query,Hit* hit);
		HitsTable();
		virtual ~HitsTable();
		virtual int     count(string queryId,ScopeSet scope);
		virtual HitsSet search(string queryId,ScopeSet scope);
		int save(string filename);
		int load(string filename);
	private:
		int                    m_HitsCounter;
		vector<Hit *>          m_Hits;
		multimap<string,Hit *> m_QueryHash;
		map<int,Hit *>         m_IndexHash;

	};

#define _QA_GENERIC_HIT_TABLE_
#endif
