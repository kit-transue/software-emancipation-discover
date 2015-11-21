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

#ifndef _MAKEPDF_H
#define _MAKEPDF_H

#include <io.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>				
#include <stdio.h>
#include <fcntl.h>
#include <windows.h>
#include "DisRegistry.h"

#define	NMAKE_EXE_PROJECT			"nmake /ANCL /F "
#define	NMAKE_EXE_FILE_H			"nmake /PANCL /F "
#define NMAKE_CFG							" CFG=\""
#define NMAKE_PROJLINE_BEG		"\tNMAKE /ANCL "
#define NMAKE_FILELINE_BEG		"\tcl.exe "
#define NMAKE_FILENAME_BEG		" /c "
#define	NMAKE_OBJNAME_REST		": -n "

#define	LOG_FILE_NAME					"c:\\Integrator\\MakePDF.Log"
#define	PDF_FILE_INDENT				2

#define	MAKE_LINE_LEN						1024

#define	SUCCESS									 0
#define ERROR_NUMARGUMENTS			-1
#define ERROR_NOMAKEFILE				-2
#define ERROR_PIPECREATE				-3
#define ERROR_HANDLEREDIRECT		-4
#define ERROR_CREATEPROCESS			-5
#define ERROR_CRUNTIME_HANDLE		-6
#define ERROR_FILE_POINTER			-7


enum TARGET_TYPE
	{
	NODE_PROJECT,
	NODE_FILE,
	NODE_FILE_C,
	NODE_FILE_H,
	TARGET_PROJECT,
	TARGET_FILE_C,
	TARGET_FILE_H
	};


//---------------------------------------------------
class OptionNode
{
protected:
	OptionNode*		m_pPrevOptionNode;
	OptionNode*		m_pNextOptionNode;
	char*					m_pszOptionText;

	void					SetPrev(OptionNode* pNode)	{m_pPrevOptionNode=pNode;}
	void					SetNext(OptionNode* pNode)	{m_pNextOptionNode=pNode;}

public:
	OptionNode( OptionNode* pHeadOption, char* pszOptionText, int nOptionTextLen);
 ~OptionNode();

	OptionNode*		GetPrev()				{return m_pPrevOptionNode;}
	OptionNode*		GetNext()				{return m_pNextOptionNode;}
	char*					GetOptionText()	{return m_pszOptionText;}
	void					RemoveOptionNode();
};

//---------------------------------------------------
class TargetNode
{
protected:
	TargetNode*		m_pPrevTarget;
	TargetNode*		m_pNextTarget;
	TargetNode*		m_pParentTarget;
	TargetNode*		m_pFirstChildTarget;

	TARGET_TYPE		m_tType;
	OptionNode*		m_pCompileOptions;
	char*					m_pszProjectName;
	char*					m_pszProjectPath;
	char*					m_pszFileName;
	char*					m_pszFilePath;
	char*					m_pszObjFilePath;
	int						m_nPlatformLen;
	BOOL					m_ExclusionFlag;
	BOOL					m_DeleteFlag;
	BOOL					m_WorkFlag;

	void					SetPrev						(TargetNode* pTarget) { m_pPrevTarget		= pTarget; }
	void					SetNext						(TargetNode* pTarget) { m_pNextTarget		= pTarget; }
	void					SetParent					(TargetNode* pTarget) { m_pParentTarget = pTarget; }
	void					RemoveChild				(TargetNode* pTarget);
	void					InsertNewChild		(TargetNode* pTarget);
	void					ShiftFirstChild		()	{ m_pFirstChildTarget = m_pFirstChildTarget->GetNext(); }
	TargetNode*		InsertSubProject	( char* pszSubProjectName );
	TargetNode*		InsertFile_C			( char* pszFileName );
	TargetNode*		InsertFile_H			( char* pszFileName );

	void					SetProjectName		(char*);
	void					SetFilePath				(char*);
	void					SetFileName				(char*);
	void					SetObjFilePath		(char*);
	void					DeleteFileName		();
	void					DeleteFilePath		();

	void					ProcessNMakeProjectLine	( char* pszMakeLine, TARGET_TYPE );
	void					ProcessOptions					( char* pszMakeLine );
	TargetNode*		FindEmptyProject				();

	void					DeleteOption			(OptionNode*);
	OptionNode*		InsertOption			(char*);
	OptionNode*		FindOptions				(char*);

public:
	TargetNode(char* szName, TargetNode*	pParentTarget=NULL, TARGET_TYPE = TARGET_PROJECT);
 ~TargetNode();

	BOOL					GetWorkFlag							()			{ return m_WorkFlag; }
	void					SetWorkFlagToTrue				()			{ m_WorkFlag = TRUE; }
	void					SetWorkFlagToFalse			()			{ m_WorkFlag = FALSE; }
	void					SetAllWorkFlagsToTrue		();
	BOOL					GetExclusionFlag				()			{ return m_ExclusionFlag; }
	void					SetExclusionFlag				()			{ m_ExclusionFlag = TRUE; }
	BOOL					GetDeleteFlag						()			{ return m_DeleteFlag; }
	void					SetDeleteFlag						()			{ m_DeleteFlag = TRUE; }

	TargetNode*		GetMakeFileTarget		();
	TargetNode*		FindTopProjectTarget();

  TargetNode*		GetNext							()			{ return m_pNextTarget; }
	TargetNode*		GetPrev							()			{ return m_pPrevTarget; }
	TargetNode*		GetParent						()			{ return m_pParentTarget; }
	TargetNode*		GetFirstChildTarget	()			{ return m_pFirstChildTarget; }

	TargetNode*		GetParentPRJ				();
	TargetNode*		GetCurrentPRJ				();

	TargetNode*		GetNodePRJ					();
	TargetNode*		GetNodeFILE					();
	TargetNode*		GetNodeC						();
	TargetNode*		GetNodeH						();

	TargetNode*		GetFirstChildPRJ		()		{return GetNodePRJ()->GetFirstChildTarget();}
	TargetNode*		GetFirstChildC			()		{return GetNodeC  ()->GetFirstChildTarget();}
	TargetNode*		GetFirstChildH			()		{return GetNodeH  ()->GetFirstChildTarget();}

	TargetNode*		GetNext_C_PRJ				();
	TargetNode*		GetNext_C_H					();
	TargetNode*		GetFirst_C_PRJ			();
	TargetNode*		GetFirst_C_H				();


  TARGET_TYPE		GetType							()			{ return m_tType; }
	char*					GetMakeFileName			();
	char*					GetMakeFileDir			();
	char*					GetPlatformTarget		();

	char*					GetProjectPath			()			{ return m_pszProjectPath; }
	char*					GetProjectName			()			{ return m_pszProjectName; }
	char*					GetFilePath					()			{ return m_pszFilePath; }
	char*					GetFileName					()			{ return m_pszFileName; }
	char*					GetObjFilePath			()			{ return m_pszObjFilePath; }
	OptionNode*		GetCompileOptions		()			{ return m_pCompileOptions; }

	TargetNode*		FindByNameTargetH				(char* pszFNameH);
	TargetNode*		FindNodeForFileH				(char* pszFNameH);
	TargetNode*		FindByNameTargetPRJ			(char* pszProjPath, int nProjPathLen);
	TargetNode*		FindByNameTargetC				(char* pszFNameC);

	void					PrintProjectTree		(int nTabs=0);
	int						ProcessNMake				(TARGET_TYPE=TARGET_PROJECT);
	void					CreateProjectTree		();
	void					RaiseUpOptions			();
	void					SetProjectPath			();
	char*					RaiseUpPathC				();
	void					ProcessFileH				();
	void					RaiseUpPathH				();
	void					RaiseUpTargetH			();
	void					Wildcards						();
	void					PullDownPath				();

	void					ConvertFileName			();
	void					WritePDFOptions			();
	void					WritePDFFileOptions	();
	void					WritePDF						();
	void					WritePDFTree				(int);
};

//-------------------------------------
//
void		SetCommonVar						(char* pszOS, char* pszPlatform, char* pszTarget);
char*		GetThisComputerName			();
char*		GetPlatform							();
char*		GetTarget								();
char*		GetTargetOS							();
char		GetSlash								();
int			CommonFilePath					(char* pszFName1, int nFName1Len, char* pszFName2 );
void		InternalConvertFileName	(char*, char*);

#endif	// #ifndef _MAKEPDF_H
