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
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <Xm/Text.h>

#include "ChangeArgumentsAction.h"

CChangeArgumentsAction::CChangeArgumentsAction():CImpactAction("Change Arguments"),m_pTable(NULL){
}

void CChangeArgumentsAction::Clean(){
	TArgumentsIterator i;
	for(i=m_Arguments.begin();i!=m_Arguments.end();i++) {
		delete *i;
	}
	m_Arguments.clear();	
}

CChangeArgumentsAction::~CChangeArgumentsAction(){
	Clean();
	if(m_pTable) {
		delete m_pTable;
		m_pTable = NULL;
	}
}


void CChangeArgumentsAction::parseArgs(string& args){
	// Parse string of arguments...
	// Init parser: where to start, where to finish, etc.
	int roundBracesLevel = 0; // level of "( )" braces nesting,
                               //     it is used for extracting sequential args
	int sharpBracesLevel = 0; // level of "< >" braces nesting,
                               //     it is used for extracting sequential args
	int begNdx = args.find('('); // begin of arguments, index of first symbol
	int endNdx = (begNdx >= 0) ? args.rfind(')') : string::npos; // index of last symbol
	begNdx = (begNdx == string::npos) ? 0 : begNdx + 1;
	endNdx = (endNdx == string::npos) ? args.length() - 1 : endNdx - 1;
	int begArg = begNdx; // begin of current argument

	for(int ndx = begNdx; ndx <= endNdx; ndx++) {
		char ch = args[ndx];

		if(ch=='(') roundBracesLevel++;
		else if(ch==')') roundBracesLevel--;
		else if(ch=='<') sharpBracesLevel++;
		else if(ch=='>') sharpBracesLevel--;
		
		if((ch == ',' && roundBracesLevel == 0 && sharpBracesLevel == 0) || (ndx == endNdx)) {
             		// It is delimeter of arguments
			int nLen = ((ndx == endNdx) ? ndx + 1 : ndx)-begArg;
             		string arg = args.substr(begArg, nLen);
			m_Arguments.push_back(new CArgument(arg,""));
             		begArg = ndx + 1;
         	} //if ch
     } //for ndx
}

Widget CChangeArgumentsAction::GetAttributesArea(Widget parent, symbolPtr* pSym){
	if(m_pTable) delete m_pTable;
	m_pTable = new CArgumentsTable(parent,350,300);
	
	setWait(true);
	Clean();
	string result;
	string command = "args " + prepareQuery(CEntityInfo::etag(pSym));
	CEntityInfo::exec(command,result);
	if(result.length()>0) parseArgs(result);
	
	TArgumentsIterator i;
	for(i=m_Arguments.begin();i!=m_Arguments.end();i++)
		m_pTable->Add((CArgument*)*i);
	m_pTable->Add(NULL);

	setWait(false);

	return m_pTable->GetWidget();
}

bool CChangeArgumentsAction::ActionPerformed(symbolPtr* pSym) {
  CImpactAction::ActionPerformed(pSym);
  
  string szArguments = "(";
  int nRows = m_pTable->GetRowsCount();
  for(int i=0;i<nRows;i++) {
	CArgument arg = m_pTable->Get(i);
	szArguments += arg.GetType();
	if(i!=nRows-1) szArguments += ","; 
  }
  szArguments += ")";

  SetDescription(GetName()+" to " + szArguments);
  string command = "Impact:ChangeFunctionArguments " + prepareQuery(CEntityInfo::etag(pSym)) + " " + szArguments;

  string results;
  CEntityInfo::exec(command,results);
  parseResult(results);
  return true;
}




