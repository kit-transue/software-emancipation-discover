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

#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>

#include <fileCache.h>

#include <app.h>
#include <db_intern.h>
#include <RTL_externs.h>
#include <merge_dialog.h>

#include <customize.h>
#include <systemMessages.h>

#include <objOper.h>
#include <objRawApp.h>
#include <proj.h>
#include <feedback.h>
#include <ddKind.h>
#include <path.h>
#include <pdf_tree.h>

#ifndef _WIN32
#ifndef _targetProject_h
#include <targetProject.h>
#endif
#endif

#include <xref.h>
#include <db.h>
#include <save.h>

#include <groupHdr.h>
#include <driver_mode.h>
#include <shell_calls.h>

int outdated_pset(const char * src_file, const char *pset_file,
        const char *fn);

// does not check if file is already loaded
appPtr proj_restore_file (const char* filename, const char* ln, projNode*project)
{
    Initialize(proj_restore_file);
    app* app_head = NULL;

    if(project){
        genString pset_file = get_paraset_file_name((char*)filename, project);

        int no_restore = has_paraset_file(pset_file);
        if (no_restore )
	no_restore =  ::outdated_pset(filename, (const char *)pset_file, filename);
        if (!no_restore)
	     app_head = project->restore_file(filename, ln);
        if(app_head) return app_head;
    }

    int flag = global_cache.is_ascii((const char *)filename);

    //boris: if wrong pathname do not bother
    struct OStype_stat buf;
    int res = OSapi_stat((char*)filename, &buf);
    if ((res == -1 && errno != ENOENT) ||
	(res == -1 && errno == ENOENT && !buf.IWUSR())) {
	return NULL;
    }

    //boris: Let the Epoch handle New and non ASCII files
    app_head = new objRawApp((char*)filename, 0, 0);

    if(project)
        projectNode_put_appHeader(project, app_head);

    // New file. Mapping projects 
    if (!flag) {
	((objRawApp *)app_head)->is_new(1);
	projHeader::make_new_filename (filename);
    }
    return app_head;
}

appPtr projHeader::get_file (const char* filename)
{
    Initialize(projHeader::get_file);
    app * app_head = app::find_from_phys_name(filename);
    if(app_head) return app_head;


    genString ln;
    projNode* project = (projNode *)0;
    projHeader::fn_to_ln(filename, ln, &project);

    return proj_restore_file(filename, ln, project);
}

appPtr proj_restore_raw_file (const char* filename)
{
  return
     proj_restore_file(filename, NULL, NULL);
}

