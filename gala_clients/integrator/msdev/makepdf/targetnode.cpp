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

static	TargetNode*		gpMakeFileTarget = NULL;
static	char					gszMakeFileDir[_MAX_PATH];
/****************************************************************************
			TargetNode
*****************************************************************************/
TargetNode::TargetNode( char* pszName, TargetNode*	pParentTarget, TARGET_TYPE tType)
{
	m_pNextTarget				= NULL;
	m_pPrevTarget				= NULL;
	m_pParentTarget			= NULL;
	m_pFirstChildTarget	= NULL;

	m_pCompileOptions		= NULL;
	m_pszProjectName		= NULL;
	m_pszProjectPath		= NULL;
	m_pszFilePath				= NULL;
	m_pszFileName				= NULL;
	m_pszObjFilePath		= NULL;

	m_nPlatformLen			= 0;
	m_ExclusionFlag			= FALSE;
	m_WorkFlag					= FALSE;
	m_DeleteFlag				= FALSE;

	m_pParentTarget = pParentTarget;
	m_tType					= tType;

	switch ( m_tType )
	{
		case NODE_PROJECT:
		case NODE_FILE_C:
		case NODE_FILE_H:
		{
			m_pszProjectName	= pParentTarget->GetProjectName();
			pParentTarget->InsertNewChild( this );
			break;
		}

		case NODE_FILE:
		{
			m_pszProjectName	= pParentTarget->GetProjectName();
			pParentTarget->InsertNewChild( this );
			new TargetNode( NULL, this, NODE_FILE_H);
			new TargetNode( NULL, this, NODE_FILE_C);
			break;
		}

		case TARGET_PROJECT:
		{
		SetProjectName( pszName );
		if ( pParentTarget )
			{
			 pParentTarget->InsertNewChild( this );
			}
		else
			{
				gpMakeFileTarget = this;
				char		szDrive						[_MAX_DRIVE	+1];
				char		szDir							[_MAX_DIR		+1];
				char		szExt							[_MAX_EXT		+1];
				char		szBuffer					[_MAX_PATH	+1];

				_splitpath( GetProjectName(), szDrive, szDir, szBuffer, szExt );
				strcpy( gszMakeFileDir, szDrive );
				strcat( gszMakeFileDir, szDir );

				char* pszPlatform = GetPlatform();
				char* pszTarget		= GetTarget();

				szBuffer[0] = '\0';
				if ( pszPlatform && strlen(pszPlatform) )
					strcat( szBuffer, pszPlatform );
				else
					strcat( szBuffer, "Win32");
				strcat( szBuffer, " ");
					
				if ( pszPlatform && strlen(pszTarget) )
					strcat( szBuffer, pszTarget );
				else
					strcat( szBuffer, "Debug");
					
				if ( gpMakeFileTarget->m_pszObjFilePath )
					delete [] gpMakeFileTarget->m_pszObjFilePath;
				gpMakeFileTarget->m_pszObjFilePath = NULL;

				gpMakeFileTarget->m_nPlatformLen = strlen( szBuffer );
				if ( gpMakeFileTarget->m_nPlatformLen )
					{
					 gpMakeFileTarget->m_pszObjFilePath = new char [ gpMakeFileTarget->m_nPlatformLen +1 ];
					 if ( gpMakeFileTarget->m_pszObjFilePath )
						 strcpy( gpMakeFileTarget->m_pszObjFilePath, szBuffer );
					}

			}
		new TargetNode( NULL, this, NODE_FILE);
		new TargetNode( NULL, this, NODE_PROJECT);
		break;
		}

		case TARGET_FILE_C:
		{
			m_pszProjectName	= pParentTarget->GetProjectName();
			SetFilePath(pszName);
			pParentTarget->InsertNewChild( this );
			break;
		}

		case TARGET_FILE_H:
		{
			m_pszProjectName	= pParentTarget->GetProjectName();
			SetFilePath(pszName);
			pParentTarget->InsertNewChild( this );
		}
	}
}

//---------------------------------------------------
//
TargetNode::~TargetNode()
{
	TargetNode*	pTarget=GetFirstChildTarget();

  do {
		 delete pTarget;
     pTarget=GetFirstChildTarget();
     }
	while ( pTarget );

	TargetNode* pParentTarget = GetParent();
	if ( pParentTarget )
		 pParentTarget->RemoveChild( this );
	else
		 gpMakeFileTarget = NULL;

	if ( GetType() == TARGET_PROJECT )
	{
		if (m_pszProjectName )
			delete [] m_pszProjectName;
		m_pszProjectName = NULL;
		if (m_pszProjectPath)
			delete [] m_pszProjectPath;
		m_pszProjectPath = NULL;
	}

	if (m_pszFilePath )
		delete [] m_pszFilePath;
	m_pszFilePath		 = NULL;

	if (m_pszFileName)
		delete [] m_pszFileName;
	m_pszFileName	= NULL;

	if (m_pszObjFilePath )
		delete [] m_pszObjFilePath;
	m_pszObjFilePath = NULL;
  
	while ( m_pCompileOptions )
		 DeleteOption(m_pCompileOptions);
}


//---------------------------------------------------
//
void	TargetNode::SetAllWorkFlagsToTrue()
{
	if ( GetType() == NODE_FILE_C || GetType() == NODE_FILE_H  || GetType() == NODE_PROJECT )
		{
		 TargetNode* pTarget = GetFirstChildTarget();
		 while ( pTarget )
		 {
			 pTarget->m_WorkFlag = TRUE;
			 pTarget= pTarget->GetNext();
		 }
		}
	else
		m_WorkFlag = TRUE;
}

//---------------------------------------------------
//
char*	TargetNode::GetPlatformTarget()
{
	return gpMakeFileTarget->m_pszObjFilePath;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::GetMakeFileTarget()
{
	return gpMakeFileTarget;
}

//---------------------------------------------------
//
char*	TargetNode::GetMakeFileName()
{
	return GetMakeFileTarget()->GetProjectName();
}

//---------------------------------------------------
//
char*	TargetNode::GetMakeFileDir()
{
	return gszMakeFileDir;
}

//---------------------------------------------------
//---------------------------------------------------
//
TargetNode*	TargetNode::GetCurrentPRJ()
{
	switch ( GetType() )
		{
		 case NODE_PROJECT:
		 case NODE_FILE:
			 return GetParent();
		 case NODE_FILE_C:
		 case NODE_FILE_H:
			 return GetParent()->GetParent();
		 case TARGET_PROJECT:
			 return this;
		 case TARGET_FILE_C:
		 case TARGET_FILE_H:
			 return GetParent()->GetParent()->GetParent();
		 default:
			 return NULL;
		}
}

TargetNode*	TargetNode::GetParentPRJ()
{
	if ( GetType() != TARGET_PROJECT )
		return GetCurrentPRJ();
	else
		{
		if ( this != gpMakeFileTarget )
				return GetParent()->GetParent();
		else
			  return NULL;
		}
}

//---------------------------------------------------
//---------------------------------------------------
//
TargetNode*	TargetNode::GetNodePRJ()
{
	switch ( GetType() )
		{
		 case TARGET_PROJECT:
			 return GetFirstChildTarget();
		 case NODE_PROJECT:
			 return this;
		 case NODE_FILE:
			 return GetPrev();
		 case NODE_FILE_C:
		 case NODE_FILE_H:
			 return GetParent()->GetPrev();
		 case TARGET_FILE_C:
		 case TARGET_FILE_H:
			 return GetParent()->GetParent()->GetPrev();
		 default:
			 return NULL;
		}
}

TargetNode*	TargetNode::GetNodeFILE()
{
	switch ( GetType() )
		{
		 case TARGET_PROJECT:
			 return GetFirstChildTarget()->GetNext();
		 case NODE_PROJECT:
			 return GetNext();
		 case NODE_FILE:
			 return this;
		 case NODE_FILE_C:
		 case NODE_FILE_H:
			 return GetParent();
		 case TARGET_FILE_C:
		 case TARGET_FILE_H:
			 return GetParent()->GetParent();
		 default:
			 return NULL;
		}
}

TargetNode*	TargetNode::GetNodeC()
{
	switch ( GetType() )
		{
		 case TARGET_PROJECT:
			 return GetFirstChildTarget()->GetNext()->GetFirstChildTarget();
		 case NODE_PROJECT:
			 return GetNext()->GetFirstChildTarget();
		 case NODE_FILE:
			 return GetFirstChildTarget();
		 case NODE_FILE_C:
			 return this;
		 case NODE_FILE_H:
			 return GetPrev();
		 case TARGET_FILE_C:
			 return GetParent();
		 case TARGET_FILE_H:
			 return GetParent()->GetPrev();
		 default:
			 return NULL;
		}
}

TargetNode*	TargetNode::GetNodeH()
{
	switch ( GetType() )
		{
		 case TARGET_PROJECT:
			 return GetFirstChildTarget()->GetNext()->GetFirstChildTarget()->GetNext();
		 case NODE_PROJECT:
			 return GetNext()->GetFirstChildTarget()->GetNext();
		 case NODE_FILE:
			 return GetFirstChildTarget()->GetNext();
		 case NODE_FILE_H:
			 return this;
		 case NODE_FILE_C:
			 return GetNext();
		 case TARGET_FILE_C:
			 return GetParent()->GetNext();
		 case TARGET_FILE_H:
			 return GetParent();
		 default:
			 return NULL;
		}
}

//---------------------------------------------------
//
TargetNode*	TargetNode::GetFirst_C_PRJ()
{
	TargetNode	*pChild = GetFirstChildC();
	if ( pChild )
		return pChild;
	else
		return GetFirstChildPRJ();
}

TargetNode*	TargetNode::GetNext_C_PRJ()
{
	if ( GetNext() )
		return GetNext();
	else if ( GetType() == TARGET_FILE_C )
		return GetFirstChildPRJ();
	else
		return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::GetFirst_C_H()
{
	TargetNode	*pChild = GetFirstChildC();
	if ( pChild )
		return pChild;
	else
		return GetFirstChildH();
}

TargetNode*	TargetNode::GetNext_C_H()
{
	if ( GetNext() )
		return GetNext();
	else if ( GetType() == TARGET_FILE_C )
		return GetFirstChildH();
	else
		return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::InsertSubProject	( char* pszSubProjectName )
{
	char* pProjNameEnd = strstr( pszSubProjectName, GetPlatformTarget());
	if ( pProjNameEnd )
		{
		 *(pProjNameEnd-3) = '\0';
		 TargetNode* pNodePRJ = GetNodePRJ();
		 TargetNode* pSubProject = new TargetNode (pszSubProjectName, 
																								pNodePRJ, TARGET_PROJECT);
		 return pSubProject;
		}
	else
		return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::InsertFile_C( char* pszFileName )
{
	if ( pszFileName && strlen(pszFileName) )
		{
		TargetNode* pNodeC = GetNodeC();
		char*	pszFileNameEnd;

		if ( *pszFileName == '"' )
			{
				pszFileName++;
        pszFileNameEnd = strchr(pszFileName, '"');
				if ( pszFileNameEnd )
					*pszFileNameEnd = '\0';
			}

    pszFileNameEnd = strchr(pszFileName, '\n');
		if ( pszFileNameEnd )
			*pszFileNameEnd = '\0';

		char		szFname		[2*_MAX_PATH	+1];
		_fullpath( szFname, pszFileName, sizeof(szFname)-1 );
		TargetNode* pTarget = pNodeC->GetFirstChildTarget();
		while ( pTarget )
			{
			 if ( strlen(pTarget->GetFilePath())				&&
						stricmp(pTarget->GetFilePath(), szFname ) == 0
					)
					return NULL;

			 pTarget = pTarget->GetNext();
			}
		TargetNode* pTargetFileC = new TargetNode (szFname, pNodeC, TARGET_FILE_C);
		return pTargetFileC;
		}
	return NULL;
}

//---------------------------------------------------
//
void TargetNode::InsertNewChild( TargetNode* pNewChildTarget )
{
	if ( m_pFirstChildTarget )
		{
		 m_pFirstChildTarget->SetPrev(pNewChildTarget);
		}
	 pNewChildTarget->SetNext		(m_pFirstChildTarget);
	 pNewChildTarget->SetPrev		(NULL);
	 pNewChildTarget->SetParent	(this);
	 m_pFirstChildTarget = pNewChildTarget;
}

//---------------------------------------------------
//
void	TargetNode::RemoveChild(TargetNode* pTarget)
{
	TargetNode	*pPrev=pTarget->GetPrev(), *pNext=pTarget->GetNext();
 
	 if ( pPrev )
		 pPrev->SetNext(pNext);
	 else
		 m_pFirstChildTarget= pNext;
	 if ( pNext )
		 pNext->SetPrev(pPrev);

	 pTarget->SetNext		(NULL);
	 pTarget->SetPrev		(NULL);
	 pTarget->SetParent	(NULL);
}


//---------------------------------------------------
//
TargetNode*	TargetNode::FindTopProjectTarget()
{
	if ( this == gpMakeFileTarget )
		return NULL;

	TargetNode	*pTarget = GetCurrentPRJ(), *pHead;
	while ( (pHead= pTarget->GetParentPRJ()) != gpMakeFileTarget )
		{
		 pTarget = pHead;
		}

	return pTarget;
}


