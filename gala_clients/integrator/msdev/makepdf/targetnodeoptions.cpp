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
#include "MakePDF.h"


static char *pszNotUsedOptions[] =
{ 
	"nologo",
	"YX",
	"FR",
	"Fp",
	"Fo",
	"Fd",
	"Zi",
	"c",
	"W",
	"w",
	"Gm",
	"Z7",
	""};

/****************************************************************************
			OptionNode
*****************************************************************************/
OptionNode::OptionNode( OptionNode* pHeadOption, char* pszOptionText, int nOptionTextLen)
{
	m_pszOptionText = NULL;
	if ( pszOptionText && nOptionTextLen )
		m_pszOptionText = new char [ nOptionTextLen +1 ];
	if ( m_pszOptionText )
		strcpy( m_pszOptionText, pszOptionText);

	SetPrev(NULL);
	SetNext(pHeadOption);
	if ( pHeadOption )
		pHeadOption->SetPrev(this);
}

//---------------------------------------------------
//
OptionNode::~OptionNode()
{
	if ( m_pszOptionText )
		delete [] m_pszOptionText;
	m_pszOptionText = NULL;
}

//---------------------------------------------------
//
void OptionNode::RemoveOptionNode()
{
	OptionNode	*pPrev= GetPrev(), *pNext= GetNext();
	if ( pPrev )
		pPrev->SetNext( pNext );

	if ( pNext )
		pNext->SetPrev( pPrev);

	SetNext(NULL);
	SetPrev(NULL);
}

/****************************************************************************
			Process Options
*****************************************************************************/
void	TargetNode::ProcessOptions	( char* pszMakeLine )
{
	char	szBuffer[MAKE_LINE_LEN];
	char	*pOpt = strstr(pszMakeLine, " /"), *pOptNext;
	
	if ( pOpt )
		{
		szBuffer[0] = '\0';
		pOpt+=2;
		while ( *pOpt )
			{
       pOptNext = strstr(pOpt, " /");
			 if ( pOptNext )
				 *pOptNext = '\0';
			 else
				 pOptNext = pszMakeLine + strlen(pszMakeLine) -2;
			 
			 BOOL fOption = TRUE;
			 for ( int i=0; i < sizeof(pszNotUsedOptions)/sizeof(char*); i++ )
					{
					  if ( *(pszNotUsedOptions[i]) == '\0')
							break;
						if ( memcmp(pOpt, pszNotUsedOptions[i], strlen(pszNotUsedOptions[i])) == 0 )
						{
						 fOption = FALSE;
						 if ( memcmp( pOpt, "Fo", 2) == 0 )
							 SetObjFilePath(pOpt+3);
						 break;
						}
					}
			 if ( fOption)
				 InsertOption(pOpt);

			 pOpt = pOptNext+2;

			}	// while ( *pOpt )
		}		// if ( pOpt )
}

//---------------------------------------------------
//
OptionNode*	TargetNode::InsertOption(char* pszOptionText)
{
	int					nCompileOptionLen;
	OptionNode*	pCompileOptions = NULL;

  if ( pszOptionText == NULL )
  	 return NULL;

  nCompileOptionLen=strlen(pszOptionText);
	if ( nCompileOptionLen )
		{
		 pCompileOptions = new OptionNode(m_pCompileOptions, pszOptionText, nCompileOptionLen );
		 if ( pCompileOptions )
			{
			 if ( pCompileOptions->GetOptionText() )
					m_pCompileOptions = pCompileOptions;
			 else
				 {
				  delete pCompileOptions;
					pCompileOptions = NULL;
				 }
			}
		}
	return pCompileOptions;
}

//---------------------------------------------------
//
void TargetNode::DeleteOption(OptionNode* pOptNode)
{
	if ( pOptNode )
	{
		if ( m_pCompileOptions == pOptNode )
			 m_pCompileOptions = pOptNode->GetNext();
		pOptNode->RemoveOptionNode();
		delete pOptNode;
	}
}

//---------------------------------------------------
//
void	TargetNode::RaiseUpOptions()
{
	OptionNode	*pOwnOption, *pOwnOptionNext, *pChildOption;
	TargetNode	*pTarget;

	pTarget = GetFirstChildPRJ();
	while ( pTarget )
		{
		 pTarget->RaiseUpOptions();
		 pTarget= pTarget->GetNext();
		}

	if ( this != GetMakeFileTarget() )
		{
		pTarget= GetFirstChildC();
		if ( pTarget && (pChildOption= pTarget->GetCompileOptions()) != NULL )
		 {
			 while ( pChildOption )		// Copy all options from the 1st child 
			 {
			  InsertOption(pChildOption->GetOptionText());
				pChildOption= pChildOption->GetNext();
			 }
			 
			 while ( (pTarget= pTarget->GetNext()) != NULL )	// Look through other children
			 {
				if ( pTarget->GetCompileOptions() == NULL )
					{
					while ( m_pCompileOptions )
						 DeleteOption(m_pCompileOptions);
					 break;
					}
				if ( (pOwnOption= GetCompileOptions()) == NULL )
					break;																// No own options = no common options
				while ( pOwnOption )										// Check own options
				{
					pChildOption = pTarget->FindOptions( pOwnOption->GetOptionText() );
					pOwnOptionNext = pOwnOption->GetNext();
					if ( pChildOption == NULL )							// if own option is not common
						DeleteOption( pOwnOption );
					pOwnOption = pOwnOptionNext;
				
				}	// while ( pOwnOption )
			 }	// while ( (pTarget= pTarget->GetNext()) )

      pOwnOption= GetCompileOptions();
			if ( pOwnOption )													// there are common options
			{
				while ( pOwnOption )										// look through all common options
				{
					pTarget= GetFirstChildC();
					while ( pTarget )											// and trough all child target
					{
						pTarget->DeleteOption( pTarget->FindOptions( pOwnOption->GetOptionText() ) );
						pTarget= pTarget->GetNext();
					}
					pOwnOption= pOwnOption->GetNext();
				}
			}

		 }//	if ( pTarget && (pChildOption= pTarget->GetOptionText()) )
		}	//	if ( this != GetMakeFileTarget() )

}

//---------------------------------------------------
//
OptionNode*	TargetNode::FindOptions(char* pszOptionText)
{
	OptionNode*	pChildOption = GetCompileOptions();
	while ( pChildOption )
	{
		if ( strcmp(pszOptionText, pChildOption->GetOptionText()) == 0 )
			break;
		pChildOption = pChildOption->GetNext();
	}
	return pChildOption;
}
