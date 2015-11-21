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

//---------------------------------------------------
//
void	TargetNode::SetProjectName(char* pszProjectName )
{
	char* pszNewProjectName;
	int		nProjectNameLen;
	if ( pszProjectName == NULL )
  	 return;

  nProjectNameLen = strlen(pszProjectName);
	if ( nProjectNameLen )
		{
		 pszNewProjectName = new char [nProjectNameLen+1];
		 strcpy( pszNewProjectName, pszProjectName );
		 if ( m_pszProjectName )
				delete [] m_pszProjectName;

		 m_pszProjectName = pszNewProjectName;
		}
}

//---------------------------------------------------
//
void	TargetNode::SetFilePath(char* pszFilePath )
{
	char* pszNewFilePath;
	int		nFilePathLen = strlen(pszFilePath);
	if ( pszFilePath && nFilePathLen )
		{
		 pszNewFilePath = new char [nFilePathLen+1];
		 strcpy( pszNewFilePath, pszFilePath );
		 
		 if ( m_pszFilePath )
				delete [] m_pszFilePath;

		 m_pszFilePath = pszNewFilePath;
		}
}

//---------------------------------------------------
//
void	TargetNode::SetFileName(char* pszFileName )
{
	char* pszNewFileName;
	if ( pszFileName && strlen(pszFileName) )
		{
		 pszNewFileName = new char [strlen(pszFileName)+1];
		 strcpy( pszNewFileName, pszFileName );
		 
		 if ( m_pszFileName )
				delete [] m_pszFileName;

		 m_pszFileName = pszNewFileName;
		}
}

//---------------------------------------------------
//
void	TargetNode::DeleteFileName()
{
	 if ( m_pszFileName )
			delete [] m_pszFileName;
	 m_pszFileName = NULL;
}

//---------------------------------------------------
//
void	TargetNode::DeleteFilePath()
{
	 if ( m_pszFilePath )
			delete [] m_pszFilePath;
	 m_pszFilePath = NULL;
}

//---------------------------------------------------
//
void	TargetNode::ConvertFileName()
{
	char	*pszName = GetFileName(), szNewName[2*_MAX_PATH+1];

	if ( pszName )
		{
		 InternalConvertFileName(pszName, szNewName);
		 DeleteFileName();
		 SetFileName(szNewName );
		}
	pszName = GetFilePath();
	if ( pszName )
		{
		 InternalConvertFileName(pszName, szNewName);
		 DeleteFilePath();
		 SetFilePath(szNewName );
		}

	TargetNode* pTarget = GetFirstChildTarget();
	while ( pTarget )
		{
		 pTarget->ConvertFileName();
		 pTarget= pTarget->GetNext();
		}
}

//---------------------------------------------------
//
void	TargetNode::SetObjFilePath( char* pszObjFilePath )
{
	if ( pszObjFilePath == NULL )
  	return;
	int	nObjFilePathLen = strlen(pszObjFilePath);
  if ( nObjFilePathLen == 0 )
  	return;

	if ( m_pszObjFilePath )
		delete [] m_pszObjFilePath;
	m_pszObjFilePath = NULL;

	char		szDrive						[_MAX_DRIVE	+1];
	char		szDir							[_MAX_DIR		+1];
	char		szFname						[_MAX_PATH	+1];
	char		szExt							[_MAX_EXT		+1];
	char		szRelObjFilePath	[_MAX_PATH	+1];
	char		*pszObjFilePathEnd = strchr( pszObjFilePath, '"');
	if ( pszObjFilePathEnd )
		{
		 nObjFilePathLen = pszObjFilePathEnd - pszObjFilePath-1;
		 if ( nObjFilePathLen )
		 	{
			_splitpath( m_pszFilePath, szDrive, szDir, szFname, szExt );

			 memcpy( szRelObjFilePath, pszObjFilePath, nObjFilePathLen );
			 *(szRelObjFilePath +nObjFilePathLen) = '\0';

			 strcat( szRelObjFilePath, "\\" );
			 strcat( szRelObjFilePath, szFname );
			 strcat( szRelObjFilePath, ".obj" );
			_fullpath( szFname, szRelObjFilePath, _MAX_PATH);
			 if ( m_pszObjFilePath )
					 delete [] m_pszObjFilePath;

			 m_pszObjFilePath = new char [ strlen(szFname) +1 ];
			 if ( m_pszObjFilePath )
					strcpy( m_pszObjFilePath, szFname);
			}
    }
}


//---------------------------------------------------
//
TargetNode*	TargetNode::FindByNameTargetC(char* pszFNameC)
{
	char	szFileName[2*MAX_PATH+1];

	TargetNode	*pTargetC	= GetFirstChildC();
	while ( pTargetC )
		{
		 InternalConvertFileName(pTargetC->GetFilePath(), szFileName);
		 if ( stricmp(szFileName, pszFNameC) == 0 )
			 break;
		 pTargetC= pTargetC->GetNext();
		}
	return pTargetC;
}

/****************************************************************************
			Process Project Path
*****************************************************************************/

void	TargetNode::SetProjectPath()
{
	if ( GetType() != TARGET_PROJECT )
		return;

	if ( this == GetMakeFileTarget() )
	{
	 m_pszProjectPath = new char [3] ;
	 strcpy( m_pszProjectPath, ".\\" );
	}
	else
	{
	char*		pParentPath = GetParentPRJ()->GetProjectPath();
	char*		pProjectTextBeg = GetProjectName();
	char*		pProjectTextEnd = strchr( pProjectTextBeg, ' ');
	int			nProjectTextLen, nParentTextLen = strlen(pParentPath);

	if ( pProjectTextEnd )
		 nProjectTextLen = pProjectTextEnd - pProjectTextBeg;
	else
		 nProjectTextLen = strlen( pProjectTextBeg );

	m_pszProjectPath = new char [ nParentTextLen + 1 + nProjectTextLen +1 ];
	if ( m_pszProjectPath )
		{
		 strcpy( m_pszProjectPath, pParentPath );
		 memcpy( m_pszProjectPath+ nParentTextLen, pProjectTextBeg, nProjectTextLen);
		 *(m_pszProjectPath+ nParentTextLen + nProjectTextLen ) = '\0';
		 strcat( m_pszProjectPath, "\\" );
		}

	}	// else //if ( this == GetMakeFileTarget() )

	//---------------------------------------------
	TargetNode* pTarget = GetFirstChildPRJ();
	while ( pTarget )
		{
		 pTarget->SetProjectPath();
		 pTarget = pTarget->GetNext();
		}
}

/****************************************************************************
			Process FilePath
*****************************************************************************/

char*	TargetNode::RaiseUpPathC()
{
	TargetNode	*pTarget;
	char				*pszFPath1, *pszFPath2;
  int					nCommonFPathLen;

	if ( GetType() == TARGET_PROJECT )
		{
		pTarget= GetFirst_C_PRJ();
		while ( pTarget )
			{
			 pTarget->RaiseUpPathC();
			 pTarget= pTarget->GetNext_C_PRJ();
			}

      pTarget= GetFirst_C_PRJ();
		if ( pTarget && (pszFPath1= pTarget->GetFilePath()) != NULL )
		 {
			 nCommonFPathLen = CommonFilePath( pszFPath1, strlen(pszFPath1), pszFPath1 );
			 while ( (pTarget= pTarget->GetNext_C_PRJ()) != NULL )	// Look through other children
			 {
				if ( (pszFPath2= pTarget->RaiseUpPathC()) == NULL )
					{
					 nCommonFPathLen = 0;
					 break;
					}

				if ( (nCommonFPathLen = CommonFilePath( pszFPath1, nCommonFPathLen, pszFPath2 )) == 0 )
					break;

			 }	// while ( (pTarget= pTarget->GetNext()) )

			 if ( nCommonFPathLen && (m_pszFilePath = new char [ nCommonFPathLen+1 ]) != NULL )
			 {
				 memcpy( m_pszFilePath, pszFPath1, nCommonFPathLen );
				 *(m_pszFilePath +nCommonFPathLen) = '\0';
			 }

			 pTarget= GetFirst_C_PRJ();
			 while ( pTarget )
			 {
				 pTarget->SetFileName( (pTarget->GetFilePath()) + nCommonFPathLen );
				 pTarget= pTarget->GetNext_C_PRJ();
			 }

			}		// if ( (pTarget= GetFirst_C_PRJ()) && (pszFPath1= pTarget->RaiseUpPath()) )
		}			// if ( GetType() == TARGET_PROJECT )

	return GetFilePath();
}

//--------------------------------------------------------------
//
void	TargetNode::PullDownPath()
{
	TargetNode	*pTarget;
	char*	pszCommonName =  GetMakeFileDir();
	int		nCommonLen = strlen( pszCommonName );
	int		nLen;

	SetFileName( pszCommonName );
	SetFilePath( pszCommonName );

	pTarget= GetFirstChildC();
	while ( pTarget )
		{
		 nLen = CommonFilePath( pszCommonName, nCommonLen, pTarget->GetFileName() );
		 if ( nLen )
			 pTarget->SetFileName( (pTarget->GetFilePath()) + nLen );
		 pTarget= pTarget->GetNext();
		}

	pTarget= GetFirstChildH();
	while ( pTarget )
		{
		 nLen = CommonFilePath( pszCommonName, nCommonLen, pTarget->GetFileName() );
		 if ( nLen )
			 pTarget->SetFileName( (pTarget->GetFilePath()) + nLen );
		 pTarget= pTarget->GetNext();
		}

	pTarget= GetFirstChildPRJ();
	while ( pTarget )
		{
		 if ( pTarget->GetFileName() == NULL )
				pTarget->PullDownPath();
		 else
			{
			 nLen = CommonFilePath( pszCommonName, nCommonLen, pTarget->GetFileName() );
			 if ( nLen )
				 pTarget->SetFileName( (pTarget->GetFileName()) + nLen );
			}
		 pTarget= pTarget->GetNext();
		}
}

/****************************************************************************
			Wildcards
*****************************************************************************/

void	TargetNode::Wildcards()
{
	TargetNode	*pTarget, *pTargetFile;
	if ( GetType() == TARGET_PROJECT )
		{
		GetNodeC()->Wildcards();
		GetNodeH()->Wildcards();
		pTarget = GetFirstChildPRJ();
		while ( pTarget )
			{
			 pTarget->Wildcards();
			 pTarget= pTarget->GetNext();
			}
		}
	else if ( GetType() == NODE_FILE_C || GetType() == NODE_FILE_H )
		{
		 char		szDrive						[_MAX_DRIVE	+1],	szFoundDrive[_MAX_DRIVE	+1];
		 char		szDir							[_MAX_DIR		+1],	szFoundDir	[_MAX_DIR		+1];
		 char		szFname						[_MAX_PATH	+1],	szFoundFname[_MAX_PATH	+1];
		 char		szExt							[_MAX_EXT		+1],	szFoundExt	[_MAX_EXT		+1];
		 char		szWildcardsPath		[_MAX_PATH	+1],	szFoundFile	[_MAX_DIR		+1];
		 char		szEmpty[] = "";

		 WIN32_FIND_DATA	FileFindData;

		 SetAllWorkFlagsToTrue();
		 while ( TRUE )
			{
			 pTargetFile = GetFirstChildTarget();
			 while ( pTargetFile )
			 {
				if ( pTargetFile-> GetWorkFlag() )
					{
					 _splitpath( pTargetFile->GetFileName(), szDrive, szDir, szFname, szExt );
					 if ( szFname[0] != '*' )
							break;
					}
				pTargetFile= pTargetFile->GetNext();
			 }
			 if ( pTargetFile == NULL )
				 break;

			 pTargetFile->SetWorkFlagToFalse();
			 _splitpath( pTargetFile->GetFilePath(), szFoundDrive, szFoundDir, szFoundFname, szFoundExt );
			 strcpy( szFoundFname, "*" );
			 _makepath( szWildcardsPath, szFoundDrive, szFoundDir, szFoundFname, szFoundExt );

			 int		nMatchedFiles=0, nNoMatchedFiles = 0;
			 HANDLE hFindFile = FindFirstFile( szWildcardsPath, &FileFindData );
			 if ( hFindFile != INVALID_HANDLE_VALUE )
			 do
			 	{
					 _makepath( szFoundFile, szFoundDrive, szFoundDir, FileFindData.cFileName, szEmpty );
					 pTarget = GetFirstChildTarget();
					 while ( pTarget )
					 {
						 if ( stricmp( pTarget->GetFilePath(), szFoundFile ) == 0 )
								break;
						 pTarget= pTarget->GetNext();
					 }
					 if ( pTarget )
						{
						 nMatchedFiles++;
						 pTarget->SetWorkFlagToFalse();
						}
					 else
						 nNoMatchedFiles++;
				}
			 while ( FindNextFile( hFindFile, &FileFindData ) );

			 if ( nMatchedFiles && (nNoMatchedFiles==0 || (nNoMatchedFiles*100)/nMatchedFiles<=50) )
				{
				 hFindFile = FindFirstFile( szWildcardsPath, &FileFindData );
				 do
					{
					 _makepath( szFoundFile, szFoundDrive, szFoundDir, FileFindData.cFileName, szEmpty );
					 pTarget = GetFirstChildTarget();
					 while ( pTarget )
					 {
						 if ( stricmp( pTarget->GetFilePath(), szFoundFile ) == 0 )
								break;
						 pTarget= pTarget->GetNext();
					 }
					 if ( pTarget )
							pTarget->SetDeleteFlag();
					 else	// szFoundFile is not matched to project's files, insert new one
						{
						 char*		pszPath = GetParentPRJ()->GetFilePath();
						 int			nPathLen = strlen( pszPath );

						 pTargetFile = new TargetNode (szFoundFile, this, GetFirstChildTarget()->GetType());
						 if ( strstr( pTargetFile->GetFilePath(), pszPath) )
							 pTargetFile->SetFileName( pTargetFile->GetFilePath() + nPathLen);
						 else
							 pTargetFile->SetFileName( pTargetFile->GetFilePath() );
						 pTargetFile->SetExclusionFlag();
						 pTargetFile->SetWorkFlagToFalse();
						}
					}
					while ( FindNextFile( hFindFile, &FileFindData ) );

					// ----------------- delete matched files
					pTargetFile = GetFirstChildTarget();
					BOOL	bFirstFlag = TRUE;
					while ( pTargetFile )
					{
						pTarget = pTargetFile->GetNext();
						if ( pTargetFile-> GetDeleteFlag() )
							{
							 if ( bFirstFlag )
							 {
								 _splitpath( pTargetFile->GetFileName(), szDrive, szDir, szFname, szExt );
								 strcpy( szFname, "*" );
								 _makepath( pTargetFile->GetFileName(), szDrive, szDir, szFname, szExt );
								 _splitpath( pTargetFile->GetFilePath(), szDrive, szDir, szFname, szExt );
								 strcpy( szFname, "*" );
								 _makepath( pTargetFile->GetFilePath(), szDrive, szDir, szFname, szExt );
								 bFirstFlag = FALSE;
							 }
							 else
								 delete pTargetFile;
							}
						pTargetFile= pTarget;
					}

				}	// if ( bFindAllFilesFlag )
			}	// while (TRUE )
	}	// else if ( GetType() == NODE_FILE_C || GetType() == NODE_FILE_H )
}

