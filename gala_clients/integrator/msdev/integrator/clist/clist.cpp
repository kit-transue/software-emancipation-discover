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
#include "CList.h"

Node::Node (void *pNewData)
{
	m_pData = pNewData;
	m_pClockWise = (Node*)0;
	m_pAClockWise = (Node*)0;
}

//-------------------------------------------------------------------

CircleList::~CircleList()
{
	Node *pEntry = GetEntryPoint();
	if( pEntry )
	{
		Node *pCurrent = pEntry->GetNextClockWise();
		while( pCurrent != pEntry )
		{
			pCurrent = pCurrent->GetNextClockWise();
			delete pCurrent;
		}
		delete pEntry;
	}
}


int CircleList::AddNode( void *pData )
{
	Node *pNewNode = new Node( pData );
	if( !pNewNode )
		return 0;
	Node *pEntry = GetEntryPoint();
	if( pEntry )
	{
		Node *pAClock = pEntry->GetNextAClockWise();
		if( !pAClock )
			return 0;
		pAClock->SetNextClockWise( pNewNode );
		pNewNode->SetNextAClockWise( pAClock );
		pNewNode->SetNextClockWise( pEntry ); 
		pEntry->SetNextAClockWise( pNewNode );
	}

	else
	{
		SetEntryPoint( pNewNode );
		pNewNode->SetNextAClockWise( pNewNode );
		pNewNode->SetNextClockWise( pNewNode );
	}

	return 1;
}

int CircleList::RemoveNode( Node *pNode )
{
	if( !pNode )
		return 0;
	Node *pEntry = GetEntryPoint();
	if( !pEntry )
		return 0;
	if( pNode == pEntry ) //we are deleting an entry point
	{
		Node *pNewEntry = pEntry->GetNextClockWise();
		if( pNewEntry != pEntry )
			SetEntryPoint( pNewEntry );
		else
			SetEntryPoint( (Node*)0 ); //we are deleting a last node
	}
	//Make sure that the node belongs to a list
	else
	{
		Node *pCurrent = pEntry->GetNextClockWise(); 
		while( (pCurrent != pEntry) && (pCurrent != pNode) )
			pCurrent = pCurrent->GetNextClockWise();
		if( pCurrent!= pNode ) //the node being deleted does't belong to a list
			return 0;
	}

	pNode->GetNextClockWise()->SetNextAClockWise( pNode->GetNextAClockWise() );
	pNode->GetNextAClockWise()->SetNextClockWise( pNode->GetNextClockWise() );

	delete pNode;
	return 1;

}

		



	


