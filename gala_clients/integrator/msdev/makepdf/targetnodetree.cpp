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
			ProjectTree
*****************************************************************************/
void	TargetNode::PrintProjectTree(int nTabs)
{
	FILE*		file;
	char*		pszBuffer = new char[MAKE_LINE_LEN];
	char*		pszTabs = new char [ nTabs +PDF_FILE_INDENT+1];
	memset( pszTabs,' ', nTabs );
	*(pszTabs+nTabs) = '\0';

	if ( this == GetMakeFileTarget() )
		sprintf( pszBuffer, "========================================\n%s.mak File: %s\n Platform: \"%s\"",
						 pszTabs, GetMakeFileName(), GetPlatformTarget() );
	else if ( GetType() == TARGET_PROJECT )
		{
		if ( GetParentPRJ() == GetMakeFileTarget() )
			sprintf( pszBuffer, "\n  ------------\n%s  PROJECT: %s", pszTabs, GetProjectName() );
		else
			sprintf( pszBuffer, "\n%s  PROJECT: %s", pszTabs, GetProjectName() );
		}
	else if ( GetType() == TARGET_FILE_C )
		{
		if ( GetFileName() )
			sprintf( pszBuffer, "%s   .c: %s", pszTabs, GetFileName() );
		else
			sprintf( pszBuffer, "%s   .c: %s", pszTabs, GetFilePath() );
		}
	else if ( GetType() == TARGET_FILE_H )
		{
		if ( GetFileName() )
			sprintf( pszBuffer, "%s   .h: %s", pszTabs, GetFileName() );
		else
			sprintf( pszBuffer, "%s   .h: %s", pszTabs, GetFilePath() );
		}

	OptionNode*	pOption = GetCompileOptions();
	if ( pOption )
		{
		 strcat( pszBuffer, "  :");
		 while ( pOption )
		 {
			 strcat( pszBuffer, " /");
			 strcat( pszBuffer, pOption->GetOptionText() );
			 pOption = pOption->GetNext();
		 }
		}
	strcat( pszBuffer, "\n" );

	if ( GetType() == TARGET_PROJECT )
		{
		memset( pszTabs,' ', nTabs+PDF_FILE_INDENT );
		*(pszTabs+nTabs+PDF_FILE_INDENT) = '\0';
		if ( GetFilePath() )
			{
			strcat( pszBuffer, pszTabs );
			strcat( pszBuffer, "   Phis: ");
			strcat( pszBuffer, GetFilePath() );
			strcat( pszBuffer, "\n" );
			}
		if ( GetProjectPath() )
			{
			strcat( pszBuffer, pszTabs );
			strcat( pszBuffer, "    Log: ");
			strcat( pszBuffer, GetProjectPath() );
			strcat( pszBuffer, "\n" );
			}
		strcat( pszBuffer, pszTabs );
		strcat( pszBuffer, "{\n" );
		}

	file = fopen(LOG_FILE_NAME, "a+t");
	fprintf(file, "%s", pszBuffer );
	fclose(file);
	delete [] pszBuffer;
	delete [] pszTabs;

	if ( GetType() == TARGET_PROJECT )
		{
		TargetNode*	pSubTarget = GetFirstChildH();
		while ( pSubTarget )
			{
			 pSubTarget->PrintProjectTree(nTabs +PDF_FILE_INDENT);
			 pSubTarget = pSubTarget->GetNext();
			}
		pSubTarget = GetFirstChildC();
		while ( pSubTarget )
			{
			 pSubTarget->PrintProjectTree(nTabs +PDF_FILE_INDENT);
			 pSubTarget = pSubTarget->GetNext();
			}
		pSubTarget = GetFirstChildPRJ();
		while ( pSubTarget )
			{
			 pSubTarget->PrintProjectTree(nTabs +PDF_FILE_INDENT);
			 pSubTarget = pSubTarget->GetNext();
			}

		pszTabs = new char [ nTabs+PDF_FILE_INDENT+4];
		memset( pszTabs,' ', nTabs+PDF_FILE_INDENT );
		*(pszTabs+nTabs+PDF_FILE_INDENT) = '\0';
		strcat( pszTabs, "}\n" );
		file = fopen(LOG_FILE_NAME, "a+t");
		fprintf(file, "%s", pszTabs );
		fclose(file);
		delete [] pszTabs;
		}

}

//---------------------------------------------------
//
void	TargetNode::CreateProjectTree()
{
	TargetNode* pTarget;
	TargetNode* pTargetEmpty;
	TargetNode* pTargetParent;
	char*				pszEmptyProjectName;

	pTarget = GetFirstChildPRJ();
	while ( pTarget )
		{
		 if ( pTarget->GetFirst_C_PRJ() == NULL )
			{
			 pTargetEmpty = pTarget;
			 pTarget = pTarget->GetNext();
			 delete pTargetEmpty;
			}
		 else
			 pTarget = pTarget->GetNext();
		}

	while ( (pTargetEmpty=FindEmptyProject()) != NULL )
		{
		 pszEmptyProjectName = pTargetEmpty->GetProjectName();
		 if ( pszEmptyProjectName == NULL )
			 delete pTargetEmpty;
		 else
		 {
			 pTarget = GetFirstChildPRJ();
			 while ( pTarget )
			 {
				 if ( strlen(pTarget->GetProjectName())  && 
							stricmp(pszEmptyProjectName, pTarget->GetProjectName()) == 0
						)
						{
						 pTarget->GetParent()->RemoveChild(pTarget);
						 pTargetParent = pTargetEmpty->GetParent();
						 delete pTargetEmpty;
						 pTargetParent->InsertNewChild( pTarget );
						 break;
						}
				 else
					 pTarget = pTarget->GetNext();
			 }	// while ( pTarget )

			 if ( pTarget == NULL )
				 delete pTargetEmpty;
		 
		 }	// if ( pszEmptyProjectName != NULL )

		}		// while ( (pTargetEmpty=FindEmptyProject()) )
}

//---------------------------------------------------
//

TargetNode* TargetNode::FindEmptyProject()
{
	TargetNode* pTarget = GetFirstChildPRJ();
	TargetNode* pTargetEmpty = NULL;
	while ( pTarget )
		{
		 if ( pTarget->GetFirst_C_PRJ() )
			{
       pTargetEmpty= pTarget->FindEmptyProject();
			 if ( pTargetEmpty )
					return pTargetEmpty;
			}
		 else	// if ( ( pTarget->GetFirst_C_PRJ() ) == NULL )
			 return pTarget;
		 
		 pTarget = pTarget->GetNext();
		}	// while ( pTarget )
	
	return pTargetEmpty;
}
