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
class Node
{
	protected:
		Node*		m_pClockWise;
		Node*		m_pAClockWise;
		void*		m_pData;
	public:
		Node(void * pData=(Node*)0);

		Node*		GetNextClockWise	()							{ return m_pClockWise; };
		Node*		GetNextAClockWise	()							{ return m_pAClockWise; };
		void		SetNextClockWise	( Node *pNew )	{ m_pClockWise = pNew; };
		void		SetNextAClockWise	( Node *pNew )	{ m_pAClockWise = pNew; };

		void*		GetData()		                      { return m_pData; };
    void    SetData(void* pData)              { m_pData = pData; }
};


class CircleList
{
	protected:
		Node*		m_pEntry;
	public:
		CircleList	() { m_pEntry = (Node *)0; }
		~CircleList	();

		int		AddNode				( void * );
		int		RemoveNode		( Node * );
		Node* GetEntryPoint	()							{ return m_pEntry; };
		void	SetEntryPoint	( Node *pNew )	{ m_pEntry = pNew; };
};

CircleList* GetQueueList();
