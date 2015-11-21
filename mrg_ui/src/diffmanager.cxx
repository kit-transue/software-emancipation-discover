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
#include "stdio.h"
#include "genStringPlus.h"
#include "genString.h"

#ifndef MTK_GSTRING
#define MTK_GSTRING
typedef genString gString;
#endif

#include "diffmanager.h"
#include "difffile.h"
#include "mtkdiffmanager.h"
#include "servercomm.h"
#include "mtkargs.h"

//----------------------------------------------------------------------------------

DiffInfo::DiffInfo(char const *nm) :
	name(nm)
{
}

//----------------------------------------------------------------------------------

SrcInfo::SrcInfo(char const *nm) :
	name(nm)
{
}

//----------------------------------------------------------------------------------

DiffManager::DiffManager()
{
	gString data_file(startup_args->getAssoc("data", 0));
	if (data_file.length() != 0) {
		ReadDataFile((char*)data_file);
	} else {
		RequestDiffsFromServer();
	}
}

//----------------------------------------------------------------------------------

void DiffManager::AddSourceFile(char const *name)
{
    gString files;
    SrcInfo *src = new SrcInfo(name);
    gString cmd;
    cmd.printf("get_diff_files {%s}", name);
    int result = SendSyncCommand((char *)cmd, files);
    if(result == 0) {
	return;
    } else {
	char *p     = (char *)files;
	char *start = p;
	while(*p != 0){
	    while(*p != '\n' && *p != 0)
		p++;
	    if(*p == '\n'){
		int ch = *p;
		*p     = 0;
		AddDiffFile(src, start);
		*p = ch;
		p++;
		start = p;
	    }
	}
    }
    sources.append(&src);
}

//----------------------------------------------------------------------------------

void DiffManager::RequestDiffsFromServer()
{
    gString sources;

    int result = SendSyncCommand("get_selected_diff", sources);
    if(result == 0) {
	//MtkDiffManager::DisplayNotice("Unable to communicate with server.");
	return;
    } 
    char *p     = (char *)sources;
    if (p != NULL) {
	char *start = p;
	while(*p != 0){
	    while(*p != '\n' && *p != 0)
		p++;
	    if(*p == '\n'){
		int ch = *p;
		*p     = 0;
		if(*start=='{') start++;
		if(*(p-1)=='}') *(p-1)=0;
		AddSourceFile(start);
		*p = ch;
		p++;
		start = p;
	    }
	}
    }
}

//----------------------------------------------------------------------------------

void DiffManager::AddDiffFile(SrcInfoPtr src, DiffInfo *fl)
{
    src->diffs.append(&fl);
}

//----------------------------------------------------------------------------------

void DiffManager::AddDiffFile(SrcInfoPtr src, char const *name)
{
    DiffInfo *fl = new DiffInfo(name);
    AddDiffFile(src, fl);
}

//----------------------------------------------------------------------------------

int DiffManager::ReadDataFile(char const *data_file)
{
	char buf[4096];

	FILE *fl = fopen(data_file, "r");
	if(fl == NULL){
		fclose(fl);
		return -1;
	}
	if(fgets(buf, 4096, fl) == NULL){
		fclose(fl);
		return -1;
	}
	char *p = strrchr(buf, '\n');
	if(p != NULL)
	    *p = 0;
	SrcInfo *src = new SrcInfo(buf);
	sources.append(&src);
	while(!feof(fl)) {
		if(fgets(buf, 4096, fl) != NULL){
			p = strrchr(buf, '\n');
			if(p != NULL)
				*p = 0;
			AddDiffFile(src, buf);
		}
	}
	fclose(fl);
	return 0;
}

//----------------------------------------------------------------------------------

void DiffManager::GetMergeResultName(char const *source_name, gString& result)
{
    gString new_file_command;
    new_file_command.printf("diff_result_name {%s}", source_name);
    int res = SendSyncCommand((char *)new_file_command, result);
}

//----------------------------------------------------------------------------------

void DiffManager::ReportFilesWithConflicts()
{
    genStringPlus cmd;

    cmd += "diff_files_with_conflicts {";
    for(int i = 0; i < sources_with_conflicts.size(); i++){
	SrcInfo *si = *(SrcInfo **)sources_with_conflicts[i];
	cmd        += "{";
	cmd        += (char *)si->name;
	cmd        += "} ";
    }
    cmd += "}";
    gString reply;
    int res = SendSyncCommand((char *)cmd, reply);
}

//----------------------------------------------------------------------------------

void DiffManager::BatchMerge()
{
    sources_with_conflicts.reset();
    for(int i = 0; i < sources.size(); i++){
	SrcInfo *si          = *(SrcInfo **)sources[i];
	DiffFile *diff_file  = new DiffFile((char *)si->name);
	for(int j = 0; j < si->diffs.size(); j++){
	    DiffInfo *diff = *(DiffInfo **)si->diffs[j];
	    diff_file->LoadChangeFile(diff->name);
	}
	diff_file->Sort();
	diff_file->RemoveSimilarChanges();
	if(diff_file->TestConflicts()){
	    sources_with_conflicts.append(&si);
	} else {
	    gString diff_result_name;
	    GetMergeResultName((char *)si->name, diff_result_name); 
	    // following two calls should be in reverse order, but then it tends to delete "diff.0" after creating it
	    diff_file->RemoveDeltaFiles();
	    diff_file->SaveChangeFile((char *)diff_result_name);
	}
    }
    ReportFilesWithConflicts();
}

//----------------------------------------------------------------------------------
