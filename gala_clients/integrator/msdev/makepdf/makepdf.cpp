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
#include	"MapNet.h"

/***********************************************************************************/
//		Options
/***********************************************************************************/

typedef struct tag_OPTIONS
{
	char*				pszOptionText;
	char*				pszOptionVal;
	BOOL				bOptionOn;
}
	_OPTIONS;

typedef struct tag_OPTIONS_SUBST
{
	char*					pszOptionText;
	char*					pszOptionVal;
	char*					pszOptionStdText;
	char*					pszOptionStdVal;
}
	_OPTIONS_SUBST;

_OPTIONS_SUBST	sOptionsSubst[]= 
						 {
							{"OS",				"DOS",			"OS",						"NT"			},
							{"OS",				"NT",				"OS",						"NT"			},
							{"OS",				"UNIX",			"OS",						"UNIX"		},
							{"OS",				"SUN",			"OS",						"UNIX"		},
							{"DOS",				"",					"OS",						"NT"			},
							{"NT",				"",					"OS",						"NT"			},
							{"UNIX",			"",					"OS",						"UNIX"		},
							{"SUN",				"",					"OS",						"UNIX"		},

							{"PLATFORM",	"Win32",		"PLATFORM",			"Win32"		},
							{"Win32",			"",					"PLATFORM",			"Win32"		},
							{"P",					" ",				"PLATFORM",			" "				},

							{"TARGET",		"Debug",		"TARGET",				"Debug"		},
							{"TARGET",		"Release",	"TARGET",				"Release"	},
							{"Debug",			"",					"TARGET",				"Debug"		},
							{"DBG",				"",					"TARGET",				"Debug"		},
							{"Release",		"",					"TARGET",				"Release"	},
							{"REL",				"",					"TARGET",				"Release"	},
							{"T",					" ",				"TARGET",				" "				},

							{"OPT",				"",					"OPTIONS",			""				},
							{"OPTIONS",		"",					"OPTIONS",			""				},

							{"FILE",			" ",				"FILE",					" "				},
							{"F",					" ",				"FILE",					" "				},
							{"",					"",					"",							""				}
						 };


_OPTIONS sOptions[]= 
						 {
							 { "OS",						"NT"		, FALSE	 },
							 { "PLATFORM",			"Win32"	, FALSE	 },
							 { "TARGET",				"Debug"	, FALSE	 },
							 { "OPTIONS",				""			, FALSE	 },
							 { "FILE",					""			, FALSE	 },
							 { "",							""			, FALSE	 }
						 };


void	ParseOptions( char* pszCmdLine, _OPTIONS	*pOptDefault, _OPTIONS_SUBST *pOptSubst  );


/***********************************************************************************/
//		M A I N
/***********************************************************************************/
int main ( int argc, char* argv[] )
{
	char		szProjectFilePath	[_MAX_PATH	+1];
	char		szSourceFilePath	[_MAX_PATH	+1];
	char		szDrive						[_MAX_DRIVE	+1];
	char		szDir							[_MAX_DIR		+1];
	char		szExt							[_MAX_EXT		+1];
	char		szFname						[_MAX_PATH	+1];
	char		szOldDirectory		[_MAX_PATH	+1];
	int			i;
	_OPTIONS	*pOptTarget		= sOptions,
						*pOptPlatform	= sOptions,
						*pOptOS				= sOptions,
						*pOptFile			= sOptions,
						*pOptOpt			= sOptions;

	szProjectFilePath	[0] = '\0';
	szSourceFilePath	[0] = '\0';

	BOOL		flagPrint = TRUE;
	if ( argc < 2 )
		return 0;

	for ( i=2; i< argc; i++ )
		{
		 if ( i < argc )
			 *(argv[i]-1) = ' ';
		}
	ParseOptions( argv[1], sOptions, sOptionsSubst );
	GetCurrentDirectory( MAX_PATH, szOldDirectory );
	_splitpath( argv[1], szDrive, szDir, szFname, szExt );
	_makepath( szProjectFilePath, szDrive, szDir, szFname, szExt );
	ChangeSlash( szProjectFilePath );

	if( szProjectFilePath && ! access( szProjectFilePath, 4 ) )	// if a specified file exists
	{																														// and we have read access right

		if( strlen( szDir ) )
		{
			char		szProjectDirectory[_MAX_PATH	+1];
		
			szProjectDirectory[0] = '\0';
			strcat( szProjectDirectory, szDrive );
			strcat( szProjectDirectory, szDir );
			SetCurrentDirectory( szProjectDirectory );
		}

	FILE*	file;
	if ( flagPrint )
		{
		file= fopen(LOG_FILE_NAME, "w+t");
		fprintf(file, "%s", "MakePDF\n" );
		fclose(file);
		}

	while ( stricmp(pOptTarget	->pszOptionText, "TARGET"	 ) ) pOptTarget		++;
	while ( stricmp(pOptPlatform->pszOptionText, "PLATFORM") ) pOptPlatform	++;
	while ( stricmp(pOptOS			->pszOptionText, "OS"			 ) ) pOptOS				++;
	while ( stricmp(pOptFile		->pszOptionText, "FILE"		 ) ) pOptFile			++;
	while ( stricmp(pOptOpt			->pszOptionText, "OPTIONS" ) ) pOptOpt			++;

	if ( pOptFile->bOptionOn && *(pOptFile->pszOptionVal) )
		InternalConvertFileName( pOptFile->pszOptionVal, szSourceFilePath );

	SetCommonVar( pOptOS->pszOptionVal, pOptPlatform->pszOptionVal, pOptTarget->pszOptionVal ); 

	TargetNode		Head( szProjectFilePath, NULL );
	TargetNode*		pProjectNode;
	TargetNode*		pTarget;

	Head.ProcessNMake(TARGET_PROJECT);
	pProjectNode = Head.GetFirstChildPRJ();
	while ( pProjectNode )
		{
		 pProjectNode->ProcessNMake(TARGET_FILE_C);
		 if ( *szSourceFilePath && ( pTarget=pProjectNode->FindByNameTargetC(szSourceFilePath) ) != NULL )
				{
				 OptionNode	*pOption = pTarget->GetCompileOptions();
				 if ( pOption )
					{
					 printf(" cl -n %s", szSourceFilePath);
					 while ( pOption )
							{
							 printf(" -%s", pOption->GetOptionText() );
							 pOption= pOption->GetNext();
							}
					 printf("\n" );
					}
				 break;
				}
		 pProjectNode = pProjectNode->GetNext();
		}


	if ( !pOptFile->bOptionOn )
		{
			Head.CreateProjectTree();
			Head.RaiseUpPathC();
			Head.PrintProjectTree(0);
			Head.SetProjectPath();
			Head.ProcessFileH();
			Head.RaiseUpPathH();
			Head.RaiseUpTargetH();
			if (pOptOpt->bOptionOn)
				{
					Head.RaiseUpOptions();
					Head.WritePDFOptions();
				}
			Head.Wildcards();
			Head.PullDownPath();
			Head.PrintProjectTree(0);
			Head.ConvertFileName();
			Head.WritePDF();
		}
	SetCurrentDirectory( szOldDirectory );
	}
	
	return 0;
}

/***********************************************************************************/
//		Options
/***********************************************************************************/
void	ParseOptions( char* pszCmdLine, _OPTIONS	*pOptDefault, _OPTIONS_SUBST *pOptSubst  )
{
	if ( pszCmdLine && pOptDefault && pOptSubst )
	{
	char							*pCmdLine = pszCmdLine, *pOptText, *pOptVal;
	char							*pOptTextBeg, *pOptTextEnd, *pOptValBeg, *pOptValEnd;
	int								nOptTextLen, nOptValLen;
	_OPTIONS_SUBST		*pOptSubstLine;
	_OPTIONS					*pOptLine;

	char	*pCmdLineOld = pCmdLine;
	while ( *pCmdLine && *pCmdLine != '-' )
		pCmdLine++;

	if ( pCmdLine != pCmdLineOld && *pCmdLine )
		*pCmdLine = '\0';

	pCmdLine++;
	while ( *pCmdLine )
		{
			pOptTextEnd = pOptTextBeg = pCmdLine;
			while ( *pOptTextEnd && *pOptTextEnd != ' ' && *pOptTextEnd != ':' && *pOptTextEnd != '"' )
				pOptTextEnd++;
			
			if ( (nOptTextLen= pOptTextEnd - pOptTextBeg) == 0 )
				goto Loop;
			
			pOptSubstLine= pOptSubst;
			while( *(pOptSubstLine->pszOptionText) )
			{
				if ( (int)strlen(pOptSubstLine->pszOptionText) == nOptTextLen	&&
						 memicmp(pOptSubstLine->pszOptionText, pOptTextBeg, nOptTextLen ) == 0
					 )
					 break;
				pOptSubstLine++;
			}
			if ( *(pOptSubstLine->pszOptionText) == '\0' )
			{
				pCmdLine = pOptTextEnd;
				if ( *pCmdLine == ':' )
					pCmdLine++;
				if ( *pCmdLine == '\0' )
					break;
				if ( *pCmdLine == '"' )
				{
					pCmdLine= strchr( pCmdLine, '"');
					if ( pCmdLine )
						pCmdLine++;
					else
						break;
				}
				goto Loop;
			}	//	if ( *(pOptSubstLine->pszOptionText) == '\0' )

			pOptText = pOptSubstLine->pszOptionStdText;
			nOptTextLen = strlen( pOptText );
			if ( *pOptTextEnd == ' ' )
				while ( *(++pOptTextEnd) == ' ' );

			//--------------------
				if ( *(pOptSubstLine->pszOptionVal) )
				{
					if ( *pOptTextEnd == ':' )
					{
						pOptTextEnd++;
						if ( *pOptTextEnd == '\0' )
							break;
					}

					if ( *pOptTextEnd == '"' )
					{
					 pOptValBeg = pOptTextEnd+1;
					 if ( *pOptValBeg == '\0' )
							break;

					 pOptValEnd = strchr ( pOptValBeg, '"');
					 if ( pOptValEnd )
						 nOptValLen = pOptValEnd - pOptValBeg;
					 else
						{
						 nOptValLen = strlen( pOptValBeg );
						 pOptValEnd = pOptValBeg + nOptValLen -1;
						}
					}	// if ( *pOptEnd == '"' )
					else
					{
					 pOptValBeg = pOptTextEnd;
					 pOptValEnd = strchr ( pOptValBeg, ' ');
					 if ( pOptValEnd )
						 nOptValLen = pOptValEnd - pOptValBeg;
					 else
						{
						 nOptValLen = strlen( pOptValBeg );
						 pOptValEnd = pOptValBeg + nOptValLen -1;
						}
					}
					pCmdLine = pOptValEnd+1;

					pOptSubstLine= pOptSubst;
					while( *(pOptSubstLine->pszOptionText) )
					{
						if (		(int)strlen	(pOptSubstLine->pszOptionText) == nOptTextLen
								 &&	memicmp(pOptSubstLine->pszOptionText, pOptText, nOptTextLen ) == 0
								 &&
									(
										(	(int)strlen (pOptSubstLine->pszOptionVal ) == nOptValLen	&& 
											memicmp(pOptSubstLine->pszOptionVal, pOptValBeg, nOptValLen  ) == 0
										)
										||
											*(pOptSubstLine->pszOptionVal) == ' '
									)
							 )
							 break;
						pOptSubstLine++;
					}
					if ( *(pOptSubstLine->pszOptionText) == '\0' )
						goto Loop;

					pOptText = pOptSubstLine->pszOptionStdText;
					if ( *(pOptSubstLine->pszOptionStdVal) == ' ' )
							pOptVal = pOptValBeg;
					else
						{
							pOptVal = pOptSubstLine->pszOptionStdVal;
							nOptValLen = strlen( pOptVal );
						}

			}	// if ( pOptTableLine->pszOptionVal )

			//--------------------
			else
			{
			 pOptText	= pOptSubstLine->pszOptionStdText;
			 pOptVal	= pOptSubstLine->pszOptionStdVal;
			 nOptValLen = strlen( pOptVal );
			}


			pOptSubstLine= pOptSubst;
			while( *(pOptSubstLine->pszOptionText) )
			{
				if ( stricmp(pOptSubstLine->pszOptionText, pOptText) == 0  )
				{
					if ( *(pOptSubstLine->pszOptionVal) != ' ' )
					{
						if ( (int)strlen( pOptSubstLine->pszOptionVal ) == nOptValLen	&&
								 memicmp(pOptSubstLine->pszOptionVal, pOptVal, nOptValLen) == 0 
							 )
							{
							 pOptText	= pOptSubstLine->pszOptionStdText;
							 pOptVal	= pOptSubstLine->pszOptionStdVal;
							 break;
							}
						else
							{
							 pOptSubstLine++;
							 continue;
							}
					}
					pOptText	= pOptSubstLine->pszOptionStdText;
					break;
				}
				pOptSubstLine++;
			}	// while( *(pOptSubstLine->pszOptionText) )

			//--------------------
			pOptLine = pOptDefault;
			while ( *(pOptLine->pszOptionText) )
			{
				if ( stricmp( pOptText, pOptLine->pszOptionText ) == 0 )
				{
					pOptLine->pszOptionVal = pOptVal;
					pOptLine->bOptionOn		 = TRUE;
					break;
				}
				pOptLine++;
			}

Loop:
		while ( *pCmdLine && *pCmdLine != '/' && *pCmdLine != '-' )
			pCmdLine++;
		if ( *pCmdLine )
			pCmdLine++;
		}	// while ( *pCmdLine )
	}	// if ( pszCmdLine && pOptLine && pOptTable )
}


