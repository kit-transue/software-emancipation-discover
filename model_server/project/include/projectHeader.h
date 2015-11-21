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
/*
    START-LOG-------------------------------

    $Log: projectHeader.h  $
    Revision 1.3 1998/08/10 18:21:19EDT pero 
    port to VC 5.0: removal of typename, or, etc.
 * Revision 1.2.1.8  1992/11/10  16:08:40  oak
 * Added map_file_to_project.
 *
 * Revision 1.2.1.7  1992/11/07  19:43:53  sharris
 * declare two new methods "find_all_modules" & "module_Disk_File_Altered"
 *
 * Revision 1.2.1.6  1992/11/02  14:16:52  kws
 * Added invalidate_cache for module node
 *
 * Revision 1.2.1.5  92/10/30  19:18:02  trung
 * add new static method: load_project_saved_file
 * 
 * Revision 1.2.1.4  1992/10/18  14:46:23  kws
 * Added static method save_all_projects
 *
 * Revision 1.2.1.3  92/10/16  09:59:23  kws
 * Added method find_rmt_file
 * 
 * Revision 1.2.1.2  92/10/09  18:22:56  builder
 * *** empty log message ***
 * 

    END-LOG---------------------------------
*/

#ifndef _projectHeader_h
#define _projectHeader_h

RelClass(projectNode);
RelClass(moduleNode);
RelClass(projectHeader);
RelClass(RTLNode);
RelClass(appTree);
class objArr;
class fileXref;

#include "projectCmds.h"

//class xrefTable;

class projectHeader : public app {

public:
    define_relational (projectHeader,app);
    projectHeader (char *name, int create_root);
    projectHeader (char *name);
    projectHeader (char *name, char *source_path);
    ~projectHeader ();
    projectHeader (const projectHeader& ) {};
    copy_member(projectHeader);
    register_project(projectNodePtr);

    static make (appTreePtr, char *target);
    static cfgmgr (const char *command, const char *file);
    static handle_file(CFG_Function, appTreePtr, RTLNodePtr, int first_flag);

    virtual void insert_obj(objInserter *oi, objInserter *ni);
    virtual void remove_obj(objRemover *, objRemover *nr);
    virtual save (void *buffer, int maxlength);
    virtual objArr* search(commonTreePtr root, void* constraints,
        int options, int load_flag);
    static Relational* restore (void *rec, app *h, char *text = 0);

    static projectNodePtr find_root_project (char *fname);

    static char* get_global_xref_name();
    static char* get_user_xref_name();

    static char *map_file (const char *fname, int write_flag);
    static char *map_file_to_project (const char *fname, int write_flag, 
			   const char *project_name);
    static char *translate_file (const char *, int &);
    static int checkout_files(objArr& filenames, int lock = 0);
    static int checkin_files(objArr& filenames);

    static app* project_get_header (char *filename);

    static int get_lock_status (char* filename, char** locker);

    static char *quote_file(const char* filename);

// ----------------

    static int is_system_file (char *);
    static int is_user_file (char *);
    static int is_loaded (const char*);
    static int is_locked (const char*);
    static int is_modified (const char *);
    static int is_parsed (const char *);

    static int language (const char *);
    static int module_type (const char *);

    static void remove_project (const char*);

    static appPtr find_file (const char*);
    static appPtr find_rmt_file (const char*);
    static appPtr load_file (const char *);
    static appPtr load_project_saved_file (const char *);
    static int unload_file (const char *);

    static int lock_file (const char* fname);
    static int unlock_file (const char* fname);
    static int commit_file (const char* fname);
    static int save_file (const char* fname);
    static int remove_file (const char* fname);
    static int move_file (const char* fname, const char* dest, int multi_move);

    static int import_file (const char* fname);

    static void report_change (const char* fname);
    static void report_import (app *);

    static void get_makefile (projectHeader*, const char*);
    static moduleNode* make_module (const char*);
    static int module_Disk_File_Altered( moduleNodePtr module );
    static void find_all_modules( objArr &array ); 
    static moduleNode* find_module (char*);
    static moduleNode* find_module (char *proj, char*);
    static moduleNode* find_module (projectNode*, char*);
    static void invalidate_cache(moduleNode*);



    static int make_path (const char *);

    static const char * current_project_directory ();
    static projectHeader* current_project_header ();
    static void current_project (const char*);
    static char * current_project ();

    static void save_all_projects();

    fileXref *get_fileXref() {return file_xref;}
    //    xrefTable* get_xref(int global = 0);

private:

    //xrefTable *xref;
    fileXref *file_xref;
    //xrefTable *gxref;
};

generate_descriptor(projectHeader,app);

int project_load_id (char *id);
extern "C" int project_set_project (char* system, char* user);
extern "C" void project_get_project (char** system, char** user);

#endif
