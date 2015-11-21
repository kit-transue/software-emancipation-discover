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

/****************************************************************************
			Process .h Files Place
*****************************************************************************/
void	TargetNode::ProcessFileH()
{
	TargetNode*	pProjectNode = GetFirstChildPRJ();
	while ( pProjectNode )
		{
		 pProjectNode->ProcessNMake(TARGET_FILE_H);
		 pProjectNode->ProcessFileH();
		 pProjectNode = pProjectNode->GetNext();
		}
	
}

TargetNode*	TargetNode::FindNodeForFileH(char* pszFNameH)
{
	TargetNode	*pHead = FindTopProjectTarget();
	TargetNode	*pTargetH= pHead->FindByNameTargetH(pszFNameH);
	if ( pTargetH == NULL )
		return GetParentPRJ()->GetNodeH();

	char*	pszOwnProjectPath = this->GetParentPRJ()->GetProjectPath();
	char*	pszOldProjectPath = pTargetH->GetParentPRJ()->GetProjectPath();
	int	nCommonProjPath = CommonFilePath( pszOwnProjectPath, strlen(pszOwnProjectPath),
																				pszOldProjectPath );
	if ( (int)strlen(pszOldProjectPath) ==  nCommonProjPath )
		return NULL;

	TargetNode	*pNewTargetPRJ= pHead->FindByNameTargetPRJ( pszOwnProjectPath, nCommonProjPath);
	pTargetH->GetParent()->RemoveChild( pTargetH );
	pNewTargetPRJ->GetNodeH()->InsertNewChild( pTargetH );
	return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::FindByNameTargetPRJ(char* pszProjPath, int nProjPathLen)
{
	int	nOwnNameLen = strlen( GetProjectPath() );

	if ( nOwnNameLen < nProjPathLen )
		return NULL;
	else if ( nOwnNameLen == nProjPathLen )
		{
		 if ( memicmp( GetProjectPath(), pszProjPath, nProjPathLen) == 0 )
			 return this;
		 else
			 return NULL;
		}

	int nProjCmpLen = nProjPathLen;
	char*	pPathEnd	= strchr( pszProjPath+nOwnNameLen, '\\' );
	if ( pPathEnd )
		nProjCmpLen = pPathEnd - pszProjPath +1;
	
	TargetNode*	pTargetRes;
	TargetNode*	pTarget = GetFirstChildPRJ();
	while ( pTarget )
		{
		 if ( memicmp( pTarget->GetProjectPath(), pszProjPath, nProjCmpLen ) == 0  &&
					(pTargetRes = FindByNameTargetPRJ( pszProjPath, nProjPathLen)) != NULL
				)
				return pTargetRes;
		 else
				pTarget= pTarget->GetNext();
		}

	return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::FindByNameTargetH(char* pszFNameH)
{
	TargetNode*	pTargetH = GetFirstChildH();
	while ( pTargetH )
		{
		 if ( stricmp( pszFNameH, pTargetH->GetFilePath() ) == 0 )
			 return pTargetH;
		 pTargetH = pTargetH->GetNext();
		}

	TargetNode*	pTargetPRJ = GetFirstChildPRJ();
	while ( pTargetPRJ )
		{
		 if ( (pTargetH = pTargetPRJ->FindByNameTargetH(pszFNameH)) != NULL )
			 return pTargetH;
		 pTargetPRJ= pTargetPRJ->GetNext();
		}

	return NULL;
}

//---------------------------------------------------
//
TargetNode*	TargetNode::InsertFile_H( char* pszFileName )
{
	char*	pFileNameEnd = NULL;
	if ( pszFileName && strlen(pszFileName) )
		{
		if ( pszFileName && *pszFileName == '"' )
			{
			 pszFileName++;
			 if ( (pFileNameEnd=strchr(pszFileName, '"')) == NULL )
				 return NULL;

			 *pFileNameEnd = '\0';
			 if ( strlen(pszFileName) == 0 )
				{
				 *pFileNameEnd = '"';
				 return NULL;
				}
			}

		char		szDrive		[_MAX_DRIVE	+1];
		char		szDir			[_MAX_DIR		+1];
		char		szFname		[2*_MAX_PATH	+1];
		char		szExt			[_MAX_EXT		+1];
		_splitpath( pszFileName, szDrive, szDir, szFname, szExt );
		if ( szExt[0] == '\0' )
			{
			 if (pFileNameEnd) *pFileNameEnd = '"';
			 return NULL;
			}

		_fullpath( szFname, pszFileName, sizeof(szFname)-1 );

		TargetNode* pTargetFileH = NULL;
		TargetNode* pNodeH = FindNodeForFileH(szFname);
		if ( pNodeH )
			pTargetFileH = new TargetNode (szFname, pNodeH, TARGET_FILE_H);
		
		if (pFileNameEnd) *pFileNameEnd = '"';
		return pTargetFileH;
		}
	return NULL;
}

//---------------------------------------------------
//
void	TargetNode::RaiseUpPathH()
{
	TargetNode*	pTargetH = GetFirstChildH();

	if ( pTargetH )
	{
		char*		pszPath = GetFilePath();
		int			nPathLen =( pszPath )? nPathLen=strlen(pszPath) : NULL;

		if ( pszPath && nPathLen )
				while ( pTargetH )
				{
				 if ( strstr( pTargetH->GetFilePath(), pszPath) )
					 pTargetH->SetFileName( pTargetH->GetFilePath() + nPathLen);
				 else
					 pTargetH->SetFileName( pTargetH->GetFilePath() );
					
				 pTargetH= pTargetH->GetNext();
				}
		else
				while ( pTargetH )
				{
				 pTargetH->SetFileName( pTargetH->GetFilePath() );
				 pTargetH= pTargetH->GetNext();
				}
	}
	
	TargetNode* pTargetPRJ = GetFirstChildPRJ();
	while( pTargetPRJ )
		{
		 pTargetPRJ->RaiseUpPathH();
		 pTargetPRJ= pTargetPRJ->GetNext();
		}
}


//---------------------------------------------------
//
void	TargetNode::RaiseUpTargetH()
{
	if ( GetType() == TARGET_PROJECT )
	{
		GetNodeH()->RaiseUpTargetH();
		TargetNode	*pTarget = GetFirstChildPRJ();
		while ( pTarget )
		{
			pTarget->RaiseUpTargetH();
			pTarget= pTarget->GetNext();
		}
	}
	
	else if ( GetType() == NODE_FILE_H )
	{
	WIN32_FIND_DATA	FileFindData;
	SetAllWorkFlagsToTrue();

	while (TRUE)
	 {
		TargetNode	*pTargetH = GetFirstChildH();
		while ( pTargetH )
		{
			if ( pTargetH->GetWorkFlag() )
				break;
			pTargetH= pTargetH->GetNext();
		}
		if ( pTargetH == NULL )
			break;

		 char		szFoundDrive[_MAX_DRIVE	+1];
		 char		szFoundDir	[_MAX_DIR		+1];
		 char		szFoundFname[_MAX_PATH	+1];
		 char		szFoundExt	[_MAX_EXT		+1];
		 char		szWildcardsPath		[_MAX_PATH	+1],	szFoundFile	[_MAX_DIR		+1];
		 char		szEmpty[] = "";

		pTargetH->SetWorkFlagToFalse();
		_splitpath( pTargetH->GetFilePath(), szFoundDrive, szFoundDir, szFoundFname, szFoundExt );
		strcpy( szFoundFname, "*" );
		_makepath( szWildcardsPath, szFoundDrive, szFoundDir, szFoundFname, szFoundExt );

		HANDLE hFindFile = FindFirstFile( szWildcardsPath, &FileFindData );
		if ( hFindFile != INVALID_HANDLE_VALUE )
		do
		{
			 _makepath( szFoundFile, szFoundDrive, szFoundDir, FileFindData.cFileName, szEmpty );
			 pTargetH = GetFirstChildTarget();
			 while ( pTargetH )
			 {
				 if ( stricmp( pTargetH->GetFilePath(), szFoundFile ) == 0 )
					{
						pTargetH->SetWorkFlagToFalse();
						break;
					}
				 pTargetH= pTargetH->GetNext();
			 }
			 if ( pTargetH == NULL )	// szFoundFile is not matched to project's files
			 {
				if ( (pTargetH = GetParentPRJ()->FindByNameTargetH(szFoundFile)) != NULL )
				{
					pTargetH->GetParent()->RemoveChild( pTargetH );
					GetNodeH()->InsertNewChild( pTargetH );
					pTargetH->SetWorkFlagToFalse();

					char*		pszPath = GetParentPRJ()->GetFilePath();
					int			nPathLen = strlen( pszPath );
					if ( strstr( pTargetH->GetFilePath(), pszPath) )
					 pTargetH->SetFileName( pTargetH->GetFilePath() + nPathLen);
					else
					 pTargetH->SetFileName( pTargetH->GetFilePath() );
				}
			 }
		}
		while ( FindNextFile( hFindFile, &FileFindData ) );

	 }// while (TRUE)
	}	// else if ( getType() == NODE_FILE_H )
}

