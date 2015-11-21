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
#include <time.h>
#include "MakePDF.h"

static	FILE*	filePDF;
static	char	gszOptFileName[_MAX_PATH];

//---------------------------------------------------
//
void	TargetNode::WritePDFOptions()
{
	strcpy( gszOptFileName, GetMakeFileName() );
	char*	pszExt = strrchr( gszOptFileName, '.');
	if ( pszExt )
		memcpy( pszExt, ".pdf.opt", 9);
	else
		strcat( gszOptFileName, ".pdf.opt");

	if ( GetSlash() == '\\' )
		filePDF = fopen( gszOptFileName, "w+t" );
	else
		filePDF = fopen( gszOptFileName, "w+b" );
	if ( !filePDF )
		{
		 MessageBox(NULL, "Open file error", "MakePDF", MB_OK);
		 return;
		}

	TargetNode	*pTarget = GetFirstChildPRJ();
	while ( pTarget )
		{
		 pTarget->WritePDFFileOptions();
		 pTarget= pTarget->GetNext();
		}
	fclose( filePDF );
}

//---------------------------------------------------
//
void	TargetNode::WritePDFFileOptions()
{
	OptionNode	*pOption = GetCompileOptions();
	if ( GetType() == TARGET_PROJECT )
		{
		fprintf(filePDF, "* %s", GetProjectName() );
		while ( pOption )
			{
			 fprintf(filePDF, " -%s", pOption->GetOptionText() );
			 pOption= pOption->GetNext();
			}
		fprintf( filePDF, "\n");

		TargetNode	*pTarget = GetFirstChildC();
		while ( pTarget )
			{
			 pTarget->WritePDFFileOptions();
			 pTarget= pTarget->GetNext();
			}
		pTarget = GetFirstChildPRJ();
		while ( pTarget )
			{
			 pTarget->WritePDFFileOptions();
			 pTarget= pTarget->GetNext();
			}
		}

	else if ( GetType() == TARGET_FILE_C  &&  pOption )
		{
		 char				szFilePath[_MAX_PATH];
		 
		 InternalConvertFileName( GetFilePath(), szFilePath );
		 fprintf(filePDF, "%s", szFilePath );
		 while ( pOption )
				{
				 fprintf(filePDF, " -%s", pOption->GetOptionText() );
				 pOption= pOption->GetNext();
				}
		 fprintf( filePDF, "\n");
		}
}

//******************************************************************************************
//
void	TargetNode::WritePDF()
{
	char	szFileNamePDF[_MAX_PATH];
	strcpy( szFileNamePDF, GetMakeFileName() );
	char*	pszExt = strrchr( szFileNamePDF, '.');
	if ( pszExt )
		memcpy( pszExt+1, "pdf", 5);
	else
		strcat( szFileNamePDF, ".pdf");

	if ( GetSlash() == '\\' )
		filePDF = fopen( szFileNamePDF, "w+t" );
	else
		filePDF = fopen( szFileNamePDF, "w+b" );
	if ( !filePDF )
		{
		 MessageBox(NULL, "Open file error", "MakePDF", MB_OK);
		 return;
		}

	char	szTimeBuff[40], szDateBuff[40];
	char	szBuffer[_MAX_PATH+1];

	ZeroMemory( (void*)&szBuffer, sizeof(szBuffer) );
	memset( szBuffer, '#', 80 );

//	_tzset();
	_strtime( szTimeBuff );
	_strdate( szDateBuff );

	fprintf( filePDF, "%s", szBuffer );
	fprintf( filePDF, "\n");
	fprintf( filePDF, "##\tFile was created by MakePDF %s %s", szDateBuff, szTimeBuff );
	fprintf( filePDF, "\n");
	fprintf( filePDF, "##\tMakeFile\t: %s", GetMakeFileName() );
	fprintf( filePDF, "\n");
	fprintf( filePDF, "##\tPlatform\t: %s", GetPlatformTarget() );
	fprintf( filePDF, "\n");
	fprintf( filePDF, "##\tTargetOS\t: %s", GetTargetOS() );
	fprintf( filePDF, "\n");
	fprintf( filePDF, "%s", szBuffer );
	fprintf( filePDF, "\n"); fprintf( filePDF, "\n");

	TargetNode*	pTargetPRJ = GetFirstChildPRJ();
	while ( pTargetPRJ )
		{
		 pTargetPRJ->WritePDFTree(0);
		 pTargetPRJ = pTargetPRJ->GetNext();
		}

	fclose( filePDF );
}

//---------------------------------------------------
//
void	TargetNode::WritePDFTree(int nTabs)
{
	char*		pszTabs;
	int			nTabsNew;
	char		szOptFileName[_MAX_PATH+1], szCmdPath[_MAX_PATH+1];

	if ( GetType() == TARGET_PROJECT )
		{
		pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
		memset( pszTabs,' ', nTabs );
		*(pszTabs+nTabs) = '\0';

		fprintf( filePDF, "\n");
		if ( GetParentPRJ() == GetMakeFileTarget() )
			fprintf( filePDF, "%s%s\t: %s\t<->\t/%s  {", pszTabs, GetProjectName(), GetFilePath(),
																										GetProjectName() );
		else
			 fprintf( filePDF, "%s%s\t: %s\t<->\t%s  {", pszTabs, GetProjectName(),
																										GetFileName(), GetProjectName() );
		
		fprintf( filePDF, "\n");
		GetDisRegistryStr("DiscoverDir", szOptFileName, sizeof(szOptFileName), "c:\\");
		InternalConvertFileName( szOptFileName, szCmdPath );
		InternalConvertFileName( gszOptFileName, szOptFileName );
		if ( GetSlash() == '\\' )
		{
		fprintf( filePDF, "%s  : (**)/(*).(c*) /%%/.make", pszTabs );
		fprintf( filePDF, " => \"PDFOpt %s %s (1)/(2).(3)\" ", szOptFileName, GetProjectName() );
		}
		else
		{
		fprintf( filePDF, "%s  : /(**)/(*).(c*) /%%/.make", pszTabs );
		fprintf( filePDF, " => \"PDFOpt %s %s /(1)/(2).(3)\" ", szOptFileName, GetProjectName() );
		}
		fprintf( filePDF, "\n");
		delete [] pszTabs;

		GetNodeFILE()->WritePDFTree(nTabs+PDF_FILE_INDENT);

		TargetNode*	pTargetPRJ = GetFirstChildPRJ();
		while ( pTargetPRJ )
			{
			 pTargetPRJ->WritePDFTree( nTabs+PDF_FILE_INDENT );
			 pTargetPRJ = pTargetPRJ->GetNext();
			}
		
		pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
		memset( pszTabs,' ', nTabs );
		*(pszTabs+nTabs) = '\0';
		fprintf( filePDF, "%s}", pszTabs);
		fprintf( filePDF, "\n");
		delete [] pszTabs;
		}

	//---------------------------------------------------------
	else if ( GetType() == NODE_FILE || GetType() == NODE_FILE_C || GetType() == NODE_FILE_H )
		{
		if ( GetFileName() )
			{
			pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
			memset( pszTabs,' ', nTabs );
			*(pszTabs+nTabs) = '\0';
			fprintf ( filePDF, "%s%s\n%s{", pszTabs, GetFileName(), pszTabs );
			fprintf( filePDF, "\n");
			nTabsNew = nTabs + 2;
			delete [] pszTabs;
			}
		else 
			nTabsNew = nTabs;

		if ( GetType() == NODE_FILE )
			{
			GetNodeH()->WritePDFTree(nTabsNew);
			GetNodeC()->WritePDFTree(nTabsNew);
			}
		else
			{
			pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
			memset( pszTabs,' ', nTabs );
			*(pszTabs+nTabs) = '\0';
			TargetNode*	pTarget = GetFirstChildTarget();
			while ( pTarget )
				{
				 if ( pTarget->GetExclusionFlag() )
						{
						 fprintf( filePDF, "%s  ^ %s", pszTabs, pTarget->GetFileName());
						 fprintf( filePDF, "\n");
						}
				 pTarget= pTarget->GetNext();
				}
			pTarget = GetFirstChildTarget();
			while ( pTarget )
				{
				 if ( !pTarget->GetExclusionFlag() )
						{
						 fprintf( filePDF, "%s    %s", pszTabs, pTarget->GetFileName());
						 fprintf( filePDF, "\n");
						}
				 pTarget= pTarget->GetNext();
				}
			}

		if ( GetFileName() )
			{
			pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
			memset( pszTabs,' ', nTabs );
			*(pszTabs+nTabs) = '\0';
			fprintf ( filePDF, "%s}\n", pszTabs );
			delete [] pszTabs;
			}
		}

}
