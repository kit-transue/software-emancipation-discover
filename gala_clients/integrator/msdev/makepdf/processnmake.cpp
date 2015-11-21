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
#include <process.h>


/****************************************************************************
			Process NMake
*****************************************************************************/

static TargetNode* pTargetFileC = NULL;

void	TargetNode::ProcessNMakeProjectLine	( char* pszMakeLine, TARGET_TYPE tType )
{
	char				*pProjName, *pProjNameEnd, *pFileName, *pFileNameEnd;
	TargetNode	*pNewTarget;

	//------------------------------
	if ( tType == TARGET_PROJECT )
	{
	pProjName = &(pszMakeLine[1]);
	if( pszMakeLine[0] == '"' && pProjName )
		{
    pProjNameEnd = strchr( pProjName, '"' );
    if ( pProjNameEnd )
    	{
			*pProjNameEnd = '\0';
			InsertSubProject(pProjName);
      }
		}
	}
	//------------------------------
	else if ( tType == TARGET_FILE_C )
	{
		if ( strnicmp(pszMakeLine, NMAKE_PROJLINE_BEG, sizeof(NMAKE_PROJLINE_BEG)-1)  == 0)
		{
      pProjName = strstr(pszMakeLine, NMAKE_CFG);
			if ( pProjName )
			{
			pProjName += sizeof(NMAKE_CFG)-1;
      pProjNameEnd = strchr( pProjName, '"' );
			if ( pProjNameEnd )
				{
				 *pProjNameEnd = '\0';
				 InsertSubProject(pProjName);
				}
			}
		}
		else if ( strnicmp(pszMakeLine, NMAKE_FILELINE_BEG, sizeof(NMAKE_FILELINE_BEG)-1)  == 0)
		{
			pFileName = strstr(pszMakeLine, NMAKE_FILENAME_BEG);
      if ( pFileName )
			{
       pNewTarget = InsertFile_C(pFileName+sizeof(NMAKE_FILENAME_BEG)-1 );
			 if ( pNewTarget )
				 pNewTarget->ProcessOptions( pszMakeLine);
			}
		}

	}		 // else if ( tType == TARGET_FILE_C )
	
	//------------------------------
	else if ( tType == TARGET_FILE_H )
	{
		char	*pszDirName, *pszDirNameEnd, *pszInclude, *pszIncludeEnd;
		char	*psz1, *psz2;
		char	szFilePath[_MAX_PATH];

		if ( pTargetFileC == NULL && *pszMakeLine == '"'  &&
    	 (pFileNameEnd=strchr( pszMakeLine+1, '"')) != NULL )
		{
			*pFileNameEnd = '\0';
			char	szFullName[_MAX_PATH];
			_fullpath( szFullName, pszMakeLine+1, _MAX_PATH);

			pTargetFileC= GetFirstChildC();
			while ( pTargetFileC )
			{
				if ( stricmp(szFullName, pTargetFileC->GetObjFilePath()) == 0 )
				{
					pFileNameEnd += 1 +strlen(NMAKE_OBJNAME_REST);
					psz1 = strchr( pFileNameEnd, '{');
					psz2 = strchr( pFileNameEnd, '"');
					if ( psz1 )
						{
						 if ( psz2 )
							 pszMakeLine = min( psz1, psz2);
						 else
							 pszMakeLine = psz1;
						 break;
						}
					else if ( psz2 )
						{
						 pszMakeLine = psz2;
						 break;
						}
					else
						pszMakeLine = strchr( pFileNameEnd, '\n');
				}
				pTargetFileC= pTargetFileC->GetNext();
			}
		}
		else if ( pTargetFileC && *pszMakeLine == '\n' )
		{
			pTargetFileC = NULL;
			return;
		}

		if ( pTargetFileC )
		{
			while ( *pszMakeLine != '\n' )
			{
				while(*pszMakeLine == '\t' || *pszMakeLine == ' ') pszMakeLine++;
				if ( *pszMakeLine == '\n' )
					break;
				else if ( *pszMakeLine == '"' )
				{
					pFileName			= pszMakeLine+1;
					pFileNameEnd	= strchr( pFileName, '"');
					*pFileNameEnd = '\0';
					pszMakeLine		= pFileNameEnd+1;
					pTargetFileC->InsertFile_H( pFileName );
				}
				else if ( *pszMakeLine == '{' )		// Multiply choise for .h files
				{																	// {Dir1;Dir2;...}"File.h"
					pszInclude= pszMakeLine+1;
					pszIncludeEnd = strchr( pszInclude, '}');
					if ( pszIncludeEnd )
					{
						*pszIncludeEnd = '\0';
						pFileName= pszIncludeEnd+2;
						pFileNameEnd	= strchr( pFileName, '"');
						*pFileNameEnd = '\0';
						pszMakeLine		= pFileNameEnd+1;
						pszDirName		= pszInclude;
						while ( pszDirName < pszIncludeEnd )
						{
							pszDirNameEnd = strchr( pszDirName, ';' );
							if ( pszDirNameEnd )
								*pszDirNameEnd= '\0';
							else
								pszDirNameEnd= pszIncludeEnd;
							if ( *pszDirName != '%' )
							{
								strcpy( szFilePath, pszDirName );
								strcat( szFilePath, pFileName  );
								if ( !access( szFilePath, 4 ) )
									pTargetFileC->InsertFile_H( szFilePath );
							}
							pszDirName = pszDirNameEnd+1;
						}	//while ( pszDirName < pszIncludeEnd )

					}	// if ( pszIncludeEnd )
				}	// else if ( *pszMakeLine == '{' )

				else
					while(*pszMakeLine != '\t' && *pszMakeLine != ' ' && *pszMakeLine != '\n') pszMakeLine++;
			}	// while ( *pszMakeLine != '\n' )
		}	// if ( pTargetFileC )


	}	// else if ( tType == TARGET_FILE_H )

}


//---------------------------------------------------
//
int	TargetNode::ProcessNMake(TARGET_TYPE tType)
{
	int									nResult = SUCCESS;
	BOOL								fStarted;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO         siStartInfo;
	SECURITY_ATTRIBUTES saAttr;
	HANDLE							hSaveStdout;
	HANDLE							hSaveStderr;
	HANDLE							hChildStdoutRd;
	HANDLE							hChildStdoutWr;
	char								szCommand[2*_MAX_PATH+ sizeof(NMAKE_EXE_FILE_H) +1];

	pTargetFileC								= NULL;
	saAttr.nLength							= sizeof( SECURITY_ATTRIBUTES );
	saAttr.bInheritHandle				= TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	hSaveStdout = GetStdHandle( STD_OUTPUT_HANDLE );
	hSaveStderr = GetStdHandle( STD_ERROR_HANDLE );
	// Create pipe for redirection of child's standard output
	if ( CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0) )
		{
		if ( SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr) &&
				 SetStdHandle(STD_ERROR_HANDLE, hChildStdoutWr) )
		{
			/* Set up members of STARTUPINFO structure. */

			ZeroMemory( (void*)&siStartInfo, sizeof(siStartInfo) );
			ZeroMemory( (void*)&piProcInfo,  sizeof(piProcInfo)  );
      siStartInfo.cb = sizeof(STARTUPINFO);
			siStartInfo.lpReserved	= NULL;
			siStartInfo.lpReserved2 = NULL; 
			siStartInfo.cbReserved2 = 0;
			siStartInfo.lpDesktop		= NULL; 
			siStartInfo.dwFlags			= 0;
			siStartInfo.wShowWindow = SW_HIDE;

			/* Create the child process. */ 

			if ( tType == TARGET_FILE_H )
				strcpy( szCommand, NMAKE_EXE_FILE_H );
			else
				strcpy( szCommand, NMAKE_EXE_PROJECT );
			strcat( szCommand, "\"" );
			strcat( szCommand, GetMakeFileName() );
			strcat( szCommand, "\"" );
			strcat( szCommand, NMAKE_CFG );
			if( GetType() == TARGET_PROJECT && this != GetMakeFileTarget() )
				{
				 strcat( szCommand, GetProjectName() );
				 strcat( szCommand, " - " );
				 strcat( szCommand, GetPlatformTarget() );
				}
			strcat( szCommand, "\"" );

			fStarted = CreateProcess(NULL, 
					szCommand,		 /* command line                       */ 
					NULL,          /* process security attributes        */
					NULL,          /* primary thread security attributes */ 
					TRUE,          /* handles are inherited              */ 
					0,             /* creation flags                     */ 
					NULL,          /* use parent's environment           */ 
					NULL,          /* use parent's current directory     */ 
					&siStartInfo,  /* STARTUPINFO pointer                */ 
					&piProcInfo);  /* receives PROCESS_INFORMATION       */

			// After child process is started, we can restore standard output
			// and error handles
			SetStdHandle( STD_OUTPUT_HANDLE, hSaveStdout );
			SetStdHandle( STD_ERROR_HANDLE, hSaveStderr );

			if( fStarted )
			{
				//WaitForSingleObject( piProcInfo.hProcess, 2000/*INFINITE*/ );
				// After child is terminated, we are closing a write pipe handle
				CloseHandle(hChildStdoutWr);
				int nHandle = _open_osfhandle( (long)hChildStdoutRd, O_RDONLY| O_TEXT );
				if( nHandle != -1 )
				{
					FILE *pFile = fdopen( nHandle, "r" );
					if( pFile )
					{
					//---------------------------------------------------------------------------------
						char	szMakeLine[MAKE_LINE_LEN+1];

						while( fgets( szMakeLine, MAKE_LINE_LEN, pFile ) )
						{
							if( strlen( szMakeLine )	)
								ProcessNMakeProjectLine( szMakeLine, tType );
						}

					//---------------------------------------------------------------------------------
						fclose( pFile );
					}	// if( pFile )
					else
					{
						close( nHandle );
						nResult = ERROR_FILE_POINTER;
					}

				}	// if( nHandle != -1 )
				else
				{
					nResult = ERROR_CRUNTIME_HANDLE;;
					CloseHandle( hChildStdoutRd );
				}

			}	// if ( fStarted )
			else
				nResult = ERROR_CREATEPROCESS;

		}		// if ( SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr)
		else
			nResult = ERROR_HANDLEREDIRECT;

	 }		// if ( CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0) )
   else
			nResult = ERROR_PIPECREATE;

	return nResult;
}
