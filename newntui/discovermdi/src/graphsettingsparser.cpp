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
// GraphSettingsParser.cpp: implementation of the CGraphSettingsParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "discovermdi.h"
#include "GraphSettingsParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGraphSettingsParser::CGraphSettingsParser()
{

}

CGraphSettingsParser::~CGraphSettingsParser()
{

}

void CGraphSettingsParser::ParseSettings(CString& file) {
register i;
CFile settingsFile;
CString textLine;
CString tagStr;
CString valStr;
char ch;

    if(settingsFile.Open(file,CFile::modeRead)) {
		while(settingsFile.Read(&ch,1)==1) {
			if(ch!='\r' && ch!='\n') {
				textLine+=ch;
			} else {
				if(textLine.GetLength()>0) {
			        textLine.TrimLeft();
			        textLine.TrimRight();
			        if(textLine[0]!='#') {
				        int count = 0;
				        tagStr="";
				        valStr="";
				        for(i=0;i<textLine.GetLength();i++) {
					        if(textLine[i]==':') {
						        count++;
						        continue;
							}
					        if(count==0) tagStr+=textLine[i];
					        if(count==1) valStr+=textLine[i];
						}
				        tagStr.TrimLeft();
				        tagStr.TrimRight();
				        valStr.TrimLeft();
				        valStr.TrimRight();
				        CParserEntry entry;
				        entry.m_Tag=tagStr;
				        entry.m_Val=valStr;
				        m_List.AddTail(entry);
					}
				}
			    textLine="";
			}
		}
	}

}

CString CGraphSettingsParser::GetValue(CString &tag) {
	POSITION pos;
	pos=m_List.GetHeadPosition();
	while(pos) {
		if(m_List.GetAt(pos).m_Tag==tag) {
			return m_List.GetAt(pos).m_Val;
		}
		m_List.GetNext(pos);
	}
	return CString("");
}
