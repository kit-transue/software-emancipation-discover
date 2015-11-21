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
// Argument.cpp: implementation of the CArgument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Impact.h"
#include "Argument.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArguments::~CArguments() {
	POSITION nPos = GetHeadPosition();
	while(nPos!=NULL) {
		delete GetNext(nPos);
	}
}

CArguments::Parse(CString& szArgs)
{
	// Parse string of arguments...
	// Init parser: where to start, where to finish, etc.
	int roundBracesLevel = 0; // level of "( )" braces nesting,
                               //     it is used for extracting sequential szArgs
	int sharpBracesLevel = 0; // level of "< >" braces nesting,
                               //     it is used for extracting sequential szArgs
	int begNdx = szArgs.Find('('); // begin of arguments, index of first symbol
	int endNdx = (begNdx >= 0) ? szArgs.ReverseFind(')') : -1; // index of last symbol
	begNdx = (begNdx == -1) ? 0 : begNdx + 1;
	endNdx = (endNdx == -1) ? szArgs.GetLength() - 1 : endNdx - 1;
	int begArg = begNdx; // begin of current argument

	for(int ndx = begNdx; ndx <= endNdx; ndx++) {
		char ch = szArgs[ndx];

		if(ch=='(') roundBracesLevel++;
		else if(ch==')') roundBracesLevel--;
		else if(ch=='<') sharpBracesLevel++;
		else if(ch=='>') sharpBracesLevel--;
		
		// It is delimeter of arguments
		if((ch == ',' && roundBracesLevel == 0 && sharpBracesLevel == 0) || (ndx == endNdx)) {
			int nLen = ((ndx == endNdx) ? ndx + 1 : ndx)-begArg;
            CString arg = szArgs.Mid(begArg, nLen);
			AddTail(new CArgument(arg,CString("")));
            begArg = ndx + 1;
		} //if ch
	} //for ndx
}

void CArgument::SetName(CString &szName)
{
	m_szName = szName;
}

void CArgument::SetType(CString &szType)
{
	m_szType = szType;
}
